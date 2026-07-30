//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief      STW specific standard defines and types

   Standard typedefs in compliance with STW C Coding Rules.
   The header file is intended to be portable to different compilers.
   This should be compatible with 32bit and 64bit compilers with all data models.

   Note that depending on the platform the native types (uintn, sintn)
    might be the same as the fixed width types (uint32, int32).
    So on these platforms the compiler or static checker cannot distinguish between them.
    See #35853 for issue related to this.

   \copyright   Copyright 2007 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef STWTYPESH
#define STWTYPESH

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <limits.h>

//Do *not* add a check to prevent building with a C++ compiler.
//C sources can be used in C++ application (but not the other way around).
//So we might have a scenario where a C++ application needs to include the header of a C module

/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
typedef unsigned char uint8; ///< data type  8bit unsigned
typedef signed char sint8;   ///< data type  8bit signed

//16-bit
#if (USHRT_MAX == 65535)
typedef unsigned short uint16;
typedef signed short sint16;
#else
//stwtypes.h type for 16bit integer not known
extern T_stwtypes_h_undefined_type gt_stwtypes_h_FailBuild;
#endif

//32-bit: check for long first: if this is true then we can have definitions that
// can be distinguished from native int types.
#if (LONG_MAX == 2147483647)
typedef unsigned long uint32;
typedef signed long sint32;
#elif (INT_MAX == 2147483647)
//in this case we will have uint32 and sint32 that are not distinguishable from native int types.
typedef unsigned int uint32;
typedef signed int sint32;
#else
//stwtypes.h type for 32bit integer not known
extern T_stwtypes_h_undefined_type gt_stwtypes_h_FailBuild;
#endif

//64-bit: not standard in C90, use compiler extensions
#if defined(_MSC_VER) || defined __BORLANDC__
typedef unsigned __int64 uint64;
typedef signed __int64 sint64;
#elif defined(__GNUC__) || defined(__clang__)
typedef unsigned long long uint64;
typedef signed long long sint64;
#else
//stwtypes.h type for 64bit integer not known
extern T_stwtypes_h_undefined_type gt_stwtypes_h_FailBuild;
#endif

typedef float float32;  ///< data type IEEE 32bit float
typedef double float64; ///< data type IEEE 64bit float

// native data types
typedef unsigned int uintn; ///< data type native unsigned int
typedef signed int sintn;   ///< data type native signed int
typedef char charn;         ///< data type native char

// portable compile-time size checks
typedef char stw_types_assert_uint8_is_1[(sizeof(uint8) == 1U) ? 1 : -1];
typedef char stw_types_assert_uint16_is_2[(sizeof(uint16) == 2U) ? 1 : -1];
typedef char stw_types_assert_uint32_is_4[(sizeof(uint32) == 4U) ? 1 : -1];
typedef char stw_types_assert_uint64_is_8[(sizeof(uint64) == 8U) ? 1 : -1];
typedef char stw_types_assert_sint8_is_1[(sizeof(sint8) == 1U) ? 1 : -1];
typedef char stw_types_assert_sint16_is_2[(sizeof(sint16) == 2U) ? 1 : -1];
typedef char stw_types_assert_sint32_is_4[(sizeof(sint32) == 4U) ? 1 : -1];
typedef char stw_types_assert_sint64_is_8[(sizeof(sint64) == 8U) ? 1 : -1];
typedef char stw_types_assert_float32_is_4[(sizeof(float32) == 4U) ? 1 : -1];
typedef char stw_types_assert_float64_is_8[(sizeof(float64) == 8U) ? 1 : -1];

#endif
