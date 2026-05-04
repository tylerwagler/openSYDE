//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Version configuration (header)

   Single source of truth for the openSYDE GUI application version. Consumed by
   resources.rc (Windows file-info block) and by main.cpp (Qt application version,
   surfaced in the About dialog on every platform).
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef VERSION_CONFIG_HPP
#define VERSION_CONFIG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

/* -- Global Constants ---------------------------------------------------------------------------------------------- */
#define PROJECT_VERSION_MAJOR 1
#define PROJECT_VERSION_MINOR 90
#define PROJECT_VERSION_RELEASE 1
#define PROJECT_VERSION_BUILD 0

#endif // VERSION_CONFIG_HPP
