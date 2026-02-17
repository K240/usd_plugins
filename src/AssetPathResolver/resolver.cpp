#include "resolver.h"

#include "pxr/base/arch/fileSystem.h"
#include "pxr/base/arch/systemInfo.h"
#include "pxr/base/js/json.h"
#include "pxr/base/js/value.h"
#include "pxr/base/tf/fileUtils.h"
#include "pxr/base/tf/pathUtils.h"
#include "pxr/base/tf/staticTokens.h"
#include "pxr/usd/ar/defineResolver.h"
#include "pxr/usd/ar/filesystemAsset.h"
#include "pxr/usd/ar/filesystemWritableAsset.h"
#include "pxr/usd/ar/notice.h"

#include <cstdlib>
#include <fstream>
#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

AR_DEFINE_RESOLVER(AssetPathResolver, ArResolver);

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static const std::string _assetScheme = "asset:";

std::pair<std::string, std::string>
AssetPathResolver::ParseAssetUri(const std::string& assetPath)
{
    if (assetPath.size() <= _assetScheme.size() ||
        assetPath.compare(0, _assetScheme.size(), _assetScheme) != 0)
    {
        return {"", ""};
    }

    std::string rest = assetPath.substr(_assetScheme.size());
    std::string name;
    std::string version = "latest";

    auto qpos = rest.find('?');
    if (qpos != std::string::npos) {
        name = rest.substr(0, qpos);
        std::string query = rest.substr(qpos + 1);
        // Parse "v=<number>"
        if (query.size() > 2 && query.substr(0, 2) == "v=") {
            version = "v" + query.substr(2);
        }
    } else {
        name = rest;
    }

    return {name, version};
}

/// Build the candidate file path from asset name and version.
/// Pattern: {assetRoot}/{name}/{version}/{name}.usd
static std::string
_BuildCandidatePath(const std::string& assetRoot,
                    const std::string& name,
                    const std::string& version)
{
    if (assetRoot.empty() || name.empty()) {
        return {};
    }
    std::string path = assetRoot;
    // Ensure trailing separator
    if (path.back() != '/' && path.back() != '\\') {
        path += '/';
    }
    path += name + "/" + version + "/" + name + ".usd";
    return TfAbsPath(path);
}

/// Read {assetRoot}/{name}/versions.json and return the version string
/// mapped to "latest" (e.g. "v10"). Returns empty string on failure.
static std::string
_ResolveLatestVersion(const std::string& assetRoot,
                      const std::string& name)
{
    if (assetRoot.empty() || name.empty()) {
        return {};
    }
    std::string jsonPath = assetRoot;
    if (jsonPath.back() != '/' && jsonPath.back() != '\\') {
        jsonPath += '/';
    }
    jsonPath += name + "/versions.json";

    std::ifstream ifs(jsonPath);
    if (!ifs.is_open()) {
        TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
            "_ResolveLatestVersion - cannot open '%s'\n", jsonPath.c_str());
        return {};
    }

    JsParseError error;
    JsValue root = JsParseStream(ifs, &error);
    if (root.IsNull() || !root.IsObject()) {
        TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
            "_ResolveLatestVersion - parse error in '%s': %s\n",
            jsonPath.c_str(), error.reason.c_str());
        return {};
    }

    const JsObject& obj = root.GetJsObject();
    auto it = obj.find("latest");
    if (it == obj.end() || !it->second.IsString()) {
        TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
            "_ResolveLatestVersion - 'latest' key not found in '%s'\n",
            jsonPath.c_str());
        return {};
    }

    std::string version = it->second.GetString();
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "_ResolveLatestVersion('%s') -> '%s'\n", name.c_str(), version.c_str());
    return version;
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

AssetPathResolver::AssetPathResolver()
{
    const char* root = std::getenv("USD_ASSET_ROOT");
    if (root) {
        _assetRoot = root;
    }
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::AssetPathResolver() - USD_ASSET_ROOT='%s'\n",
        _assetRoot.c_str());
}

AssetPathResolver::~AssetPathResolver() = default;

// ---------------------------------------------------------------------------
// Identifier
// ---------------------------------------------------------------------------

std::string
AssetPathResolver::_CreateIdentifier(
    const std::string& assetPath,
    const ArResolvedPath& anchorAssetPath) const
{
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_CreateIdentifier('%s', '%s')\n",
        assetPath.c_str(), anchorAssetPath.GetPathString().c_str());

    if (assetPath.empty()) {
        return assetPath;
    }
    // For asset: URIs, the identifier is the URI itself (no anchoring).
    return assetPath;
}

std::string
AssetPathResolver::_CreateIdentifierForNewAsset(
    const std::string& assetPath,
    const ArResolvedPath& anchorAssetPath) const
{
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_CreateIdentifierForNewAsset('%s', '%s')\n",
        assetPath.c_str(), anchorAssetPath.GetPathString().c_str());

    if (assetPath.empty()) {
        return assetPath;
    }
    return assetPath;
}

// ---------------------------------------------------------------------------
// Resolve
// ---------------------------------------------------------------------------

ArResolvedPath
AssetPathResolver::_Resolve(
    const std::string& assetPath) const
{
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_Resolve('%s')\n", assetPath.c_str());

    if (assetPath.empty()) {
        return ArResolvedPath();
    }

    auto [name, version] = ParseAssetUri(assetPath);
    if (name.empty()) {
        return ArResolvedPath();
    }

    if (version == "latest") {
        version = _ResolveLatestVersion(_assetRoot, name);
        if (version.empty()) {
            return ArResolvedPath();
        }
    }

    std::string candidate = _BuildCandidatePath(_assetRoot, name, version);
    if (candidate.empty()) {
        return ArResolvedPath();
    }

    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_Resolve - candidate='%s'\n", candidate.c_str());

    if (TfPathExists(candidate)) {
        return ArResolvedPath(candidate);
    }

    return ArResolvedPath();
}

ArResolvedPath
AssetPathResolver::_ResolveForNewAsset(
    const std::string& assetPath) const
{
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_ResolveForNewAsset('%s')\n", assetPath.c_str());

    if (assetPath.empty()) {
        return ArResolvedPath();
    }

    auto [name, version] = ParseAssetUri(assetPath);
    if (name.empty()) {
        return ArResolvedPath();
    }

    if (version == "latest") {
        version = _ResolveLatestVersion(_assetRoot, name);
        if (version.empty()) {
            return ArResolvedPath();
        }
    }

    std::string candidate = _BuildCandidatePath(_assetRoot, name, version);
    if (candidate.empty()) {
        return ArResolvedPath();
    }

    // For new assets, return the path even if it doesn't exist yet.
    return ArResolvedPath(candidate);
}

// ---------------------------------------------------------------------------
// Context
// ---------------------------------------------------------------------------

ArResolverContext
AssetPathResolver::_CreateDefaultContext() const
{
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_CreateDefaultContext()\n");
    return ArResolverContext();
}

ArResolverContext
AssetPathResolver::_CreateDefaultContextForAsset(
    const std::string& assetPath) const
{
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_CreateDefaultContextForAsset('%s')\n",
        assetPath.c_str());
    return ArResolverContext();
}

bool
AssetPathResolver::_IsContextDependentPath(
    const std::string& assetPath) const
{
    return false;
}

void
AssetPathResolver::_RefreshContext(
    const ArResolverContext& context)
{
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_RefreshContext()\n");
}

// ---------------------------------------------------------------------------
// Asset access
// ---------------------------------------------------------------------------

ArTimestamp
AssetPathResolver::_GetModificationTimestamp(
    const std::string& assetPath,
    const ArResolvedPath& resolvedPath) const
{
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_GetModificationTimestamp('%s', '%s')\n",
        assetPath.c_str(), resolvedPath.GetPathString().c_str());

    const std::string& filePath = resolvedPath.GetPathString();
    double mtime;
    if (ArchGetModificationTime(filePath.c_str(), &mtime)) {
        return ArTimestamp(mtime);
    }
    return ArTimestamp();
}

std::shared_ptr<ArAsset>
AssetPathResolver::_OpenAsset(
    const ArResolvedPath& resolvedPath) const
{
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_OpenAsset('%s')\n",
        resolvedPath.GetPathString().c_str());

    return ArFilesystemAsset::Open(resolvedPath);
}

std::shared_ptr<ArWritableAsset>
AssetPathResolver::_OpenAssetForWrite(
    const ArResolvedPath& resolvedPath,
    WriteMode writeMode) const
{
    TF_DEBUG(ASSETPATHRESOLVER_RESOLVER).Msg(
        "AssetPathResolver::_OpenAssetForWrite('%s', %d)\n",
        resolvedPath.GetPathString().c_str(),
        static_cast<int>(writeMode));

    return ArFilesystemWritableAsset::Create(resolvedPath, writeMode);
}

PXR_NAMESPACE_CLOSE_SCOPE
