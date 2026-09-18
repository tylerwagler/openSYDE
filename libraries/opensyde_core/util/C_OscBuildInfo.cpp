//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Build identification of the running binary

   The per-tool version numbers in version_config.hpp are hand-edited and say nothing about which
   commit a binary came from. This class does: the values come from osy_build_info.hpp, which
   osy_build_info.cmake stamps from the git checkout before every build of opensyde_core.

   Every tool prints the summary next to its version and MD5 checksum (banner, log, About dialog),
   so a binary in the field can be tied back to a tag or a commit.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OscBuildInfo.hpp"
#include "osy_build_info.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get the version control identification of the source

   Output of `git describe --tags --always --dirty`: a tag name on a tagged commit ("v2026.09.18"),
   the tag plus distance and short hash after it ("v2026.09.18-3-g1a2b3c4"), or just the short
   hash when no tag is reachable. A "-dirty" suffix means the tree had uncommitted changes.

   \return
   Identification string; "unknown" when the build did not come from a git checkout
*/
//----------------------------------------------------------------------------------------------------------------------
const char * C_OscBuildInfo::h_GetVersionControlId(void)
{
   return OSY_BUILD_GIT_DESCRIBE;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get the committer date of the source

   Strict ISO 8601 with offset, e.g. "2026-09-18T12:14:00+02:00". This is the date of the commit,
   not of the build: it identifies the source and does not change between builds of the same commit.

   \return
   Date string; "unknown" when the build did not come from a git checkout
*/
//----------------------------------------------------------------------------------------------------------------------
const char * C_OscBuildInfo::h_GetSourceDate(void)
{
   return OSY_BUILD_SOURCE_DATE;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get both values in one line for banners and logs

   \return
   "<version control id>, source dated <date>"
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscBuildInfo::h_GetSummary(void)
{
   return std::string(h_GetVersionControlId()) + ", source dated " + h_GetSourceDate();
}
