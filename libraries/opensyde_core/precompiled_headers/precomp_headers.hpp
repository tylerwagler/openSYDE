//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Core precompiled header (header)

   Use target_precompile_headers() in CMake (requires CMake >= 3.16):
      target_precompile_headers(<target> PRIVATE precomp_headers.hpp)

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef PRECOMP_HEADERS_HPP
#define PRECOMP_HEADERS_HPP

//lint -esym(766,"precomp_headers.h")   effectively not used in lint "builds"; but that's exactly what we want
#ifndef _lint //speed up linting: don't include all of the headers for each linted .cpp file

/* -- Includes ------------------------------------------------------------------------------------------------------ */

/* Add C includes here */

#if defined __cplusplus
/* Add C++ includes here */
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <cstdio>
#include <cstring>
#include <vector>
#include <map>
#include <list>
#include <iostream>
#include <fstream>

#include "C_SclString.hpp"
#include "C_OscUtils.hpp"

#endif

#endif

#endif
