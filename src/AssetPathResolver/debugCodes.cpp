#include "debugCodes.h"

#include "pxr/base/tf/registryManager.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfDebug)
{
    TF_DEBUG_ENVIRONMENT_SYMBOL(
        ASSETPATHRESOLVER_RESOLVER,
        "Asset URI Resolver debug output");
}

PXR_NAMESPACE_CLOSE_SCOPE
