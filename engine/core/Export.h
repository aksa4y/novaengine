// Nova Engine legacy export bridge.
// SPDX-License-Identifier: MIT
#pragma once

#include "NovaExport.h"

// The implementation is still compiled through the legacy compatibility
// boundary. Keep these names as aliases so old translation units continue to
// compile while all new public code uses NOVA_API / NOVA_SHARED.
#if !defined(DORIAX_SHARED) && defined(NOVA_SHARED)
    #define DORIAX_SHARED NOVA_SHARED
#endif

#if !defined(DORIAX_EXPORTS) && defined(NOVA_EXPORTS)
    #define DORIAX_EXPORTS NOVA_EXPORTS
#endif

#ifndef DORIAX_API
    #define DORIAX_API NOVA_API
#endif
