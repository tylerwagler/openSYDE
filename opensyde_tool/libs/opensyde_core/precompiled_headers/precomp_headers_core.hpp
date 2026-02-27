//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Core precompiled header (header)

   To use this precompiled-header in a Qt project add the following to your .pro
   file:

   CONFIG   += precompile_header
   PRECOMPILED_HEADER = ../src/precomp_headers_core.h

   Contains a list of header files to be pre-compiled.
   see http://doc.qt.io/qt-5/qmake-precompiledheaders.html for details

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef PRECOMP_HEADERS_CORE_HPP
#define PRECOMP_HEADERS_CORE_HPP

// lint -esym(766,"precomp_headers_core.h")   effectively not used in lint
// "builds"; but that's exactly what we want
#ifndef _lint // speed up linting: don't include all of the headers for each
              // linted .cpp file

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */

/* Add C includes here */

#if defined __cplusplus
/* Add C++ includes here */
#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#endif
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <vector>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QtGlobal>

#include "C_OscUtils.hpp"
#include <QString>

#endif

#endif

#if __cplusplus >= 201103L // C++11 ?
#define TGL_UTIL_FUNC_ID __func__
#else
#ifdef __BORLANDC__
#define TGL_UTIL_FUNC_ID __FUNC__
#else
#define TGL_UTIL_FUNC_ID __FUNCTION__
#endif
#endif

#endif
