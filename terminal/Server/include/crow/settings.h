/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


#pragma once
// settings for crow
// TODO(ipkn) replace with runtime config. libucl?

/* #ifdef - enables debug mode */
//#define CROW_ENABLE_DEBUG

/* #ifdef - enables logging */
#define CROW_ENABLE_LOGGING

/* #ifdef - enforces section 5.2 and 6.1 of RFC6455 (only accepting masked messages from clients) */
//#define CROW_ENFORCE_WS_SPEC

/* #define - specifies log level */
/*
    Debug       = 0
    Info        = 1
    Warning     = 2
    Error       = 3
    Critical    = 4

    default to INFO
*/
#ifndef CROW_LOG_LEVEL
#define CROW_LOG_LEVEL 1
#endif

#ifndef CROW_STATIC_DIRECTORY
#define CROW_STATIC_DIRECTORY "static/"
#endif
#ifndef CROW_STATIC_ENDPOINT
#define CROW_STATIC_ENDPOINT "/static/<path>"
#endif

// compiler flags

#if defined(_MSC_VER)
#if _MSC_VER < 1900
#define CROW_MSVC_WORKAROUND
#define constexpr const
#define noexcept throw()
#endif
#endif
