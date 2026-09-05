// Nova Engine public export definitions.
// SPDX-License-Identifier: MIT
#pragma once

#if defined(_MSC_VER)
    #ifdef NOVA_SHARED
        #ifdef NOVA_EXPORTS
            #define NOVA_API __declspec(dllexport)
        #else
            #define NOVA_API __declspec(dllimport)
        #endif
    #else
        #define NOVA_API
    #endif
#else
    #define NOVA_API
#endif
