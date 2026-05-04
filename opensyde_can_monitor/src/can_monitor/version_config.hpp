//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Version configuration (header)

   Single source of truth for the openSYDE CAN Monitor application version. Consumed by
   cam_resources.rc (Windows file-info block) and by cam_main.cpp (Qt application version,
   surfaced in the About dialog on every platform).
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef VERSION_CONFIG_HPP
#define VERSION_CONFIG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

/* -- Global Constants ---------------------------------------------------------------------------------------------- */
#define PROJECT_VERSION_MAJOR 1
#define PROJECT_VERSION_MINOR 86
#define PROJECT_VERSION_RELEASE 0
#define PROJECT_VERSION_BUILD 0

#endif // VERSION_CONFIG_HPP
