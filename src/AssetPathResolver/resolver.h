#ifndef AR_ASSETPATHRESOLVER_RESOLVER_H
#define AR_ASSETPATHRESOLVER_RESOLVER_H

#include "api.h"
#include "debugCodes.h"

#include "pxr/pxr.h"
#include "pxr/usd/ar/resolver.h"

#include <memory>
#include <string>
#include <utility>

PXR_NAMESPACE_OPEN_SCOPE

/// \class AssetPathResolver
/// URI resolver for the "asset:" scheme.
///
/// Resolves paths of the form:
///   asset:chair_0       -> {USD_ASSET_ROOT}/chair_0/latest/chair_0.usd
///   asset:chair_0?v=10  -> {USD_ASSET_ROOT}/chair_0/v10/chair_0.usd
///
class AssetPathResolver final : public ArResolver
{
public:
    AR_ASSETPATHRESOLVER_API
    AssetPathResolver();

    AR_ASSETPATHRESOLVER_API
    virtual ~AssetPathResolver();

    /// Parse an "asset:" URI into (name, version) pair.
    /// Returns ("", "") if the path is not an asset URI.
    static std::pair<std::string, std::string>
    ParseAssetUri(const std::string& assetPath);

protected:
    AR_ASSETPATHRESOLVER_API
    std::string _CreateIdentifier(
        const std::string& assetPath,
        const ArResolvedPath& anchorAssetPath) const final;

    AR_ASSETPATHRESOLVER_API
    std::string _CreateIdentifierForNewAsset(
        const std::string& assetPath,
        const ArResolvedPath& anchorAssetPath) const final;

    AR_ASSETPATHRESOLVER_API
    ArResolvedPath _Resolve(
        const std::string& assetPath) const final;

    AR_ASSETPATHRESOLVER_API
    ArResolvedPath _ResolveForNewAsset(
        const std::string& assetPath) const final;

    AR_ASSETPATHRESOLVER_API
    ArResolverContext _CreateDefaultContext() const final;

    AR_ASSETPATHRESOLVER_API
    ArResolverContext _CreateDefaultContextForAsset(
        const std::string& assetPath) const final;

    AR_ASSETPATHRESOLVER_API
    bool _IsContextDependentPath(
        const std::string& assetPath) const final;

    AR_ASSETPATHRESOLVER_API
    void _RefreshContext(
        const ArResolverContext& context) final;

    AR_ASSETPATHRESOLVER_API
    ArTimestamp _GetModificationTimestamp(
        const std::string& assetPath,
        const ArResolvedPath& resolvedPath) const final;

    AR_ASSETPATHRESOLVER_API
    std::shared_ptr<ArAsset> _OpenAsset(
        const ArResolvedPath& resolvedPath) const final;

    AR_ASSETPATHRESOLVER_API
    std::shared_ptr<ArWritableAsset> _OpenAssetForWrite(
        const ArResolvedPath& resolvedPath,
        WriteMode writeMode) const final;

private:
    std::string _assetRoot;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // AR_ASSETPATHRESOLVER_RESOLVER_H
