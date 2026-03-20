# minimum 3.16 for precompiled headers
cmake_minimum_required(VERSION 3.16)

project(SYDEflash LANGUAGES CXX)

#include script to generate PC-lint configuration files
include(../lint_config.cmake)

# Sets CMAKE_AUTOUIC and CMAKE_AUTOMOC to ON for ui files compiling
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# For supporting windows .rc files
if(WIN32)
   enable_language("RC")
   # Special configuration of RC file compiling with windres necessary. Without change, the command line call is to long
   # for windres with all include paths forwarded by CMake
   set(CMAKE_RC_INCLUDE_PATH
      ${PROJECT_ROOT}/src/syde_flash
   )
   set(CMAKE_RC_COMPILE_OBJECT "${CMAKE_RC_COMPILER} -O coff -I${CMAKE_RC_INCLUDE_PATH} <SOURCE> <OBJECT>")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)

#signals emit etc. are in conflict with the DBC library
add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DQT_STATICPLUGIN)

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/../..)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Svg)
find_package(Qt6 REQUIRED COMPONENTS SvgWidgets)
find_package(Qt6 REQUIRED COMPONENTS Xml)
find_package(Qt6 REQUIRED COMPONENTS SerialBus Network)

# --- PROJECT INCLUDE PATHS --------------------------------------------------------------------------------------------

include_directories()
set (OPENSYDE_INCLUDES
   ${PROJECT_ROOT}/src
   ${PROJECT_ROOT}/src/help_engine
   ${PROJECT_ROOT}/src/navigable_gui
   ${PROJECT_ROOT}/src/project_operations
   ${PROJECT_ROOT}/src/syde_flash
   ${PROJECT_ROOT}/src/syde_flash/bottom_bar
   ${PROJECT_ROOT}/src/syde_flash/configure_node
   ${PROJECT_ROOT}/src/syde_flash/properties
   ${PROJECT_ROOT}/src/syde_flash/search_nodes
   ${PROJECT_ROOT}/src/syde_flash/settings
   ${PROJECT_ROOT}/src/syde_flash/update
   ${PROJECT_ROOT}/src/syde_flash/user_settings
   ${PROJECT_ROOT}/src/system_definition
   ${PROJECT_ROOT}/src/system_definition/node_edit/datapools
   ${PROJECT_ROOT}/src/system_views/communication
   ${PROJECT_ROOT}/src/system_views/device_configuration
   ${PROJECT_ROOT}/src/table_base
   ${PROJECT_ROOT}/src/table_base/tree_base
)

# Include path with library modules
set (OPENSYDE_LIB_INCLUDES
)

# --- PROJECT SOURCE FILE PATHS ----------------------------------------------------------------------------------------

set(OPENSYDE_SOURCES

   ${PROJECT_ROOT}/src/help_engine/C_HeHandler.cpp
   ${PROJECT_ROOT}/src/navigable_gui/C_NagAboutDialog.cpp
   ${PROJECT_ROOT}/src/navigable_gui/C_NagToolTip.cpp
   ${PROJECT_ROOT}/src/project_operations/C_PopErrorHandling.cpp
   ${PROJECT_ROOT}/src/syde_flash/C_FlaMainWindow.cpp
   ${PROJECT_ROOT}/src/syde_flash/C_FlaTitleBarWidget.cpp
   ${PROJECT_ROOT}/src/syde_flash/bottom_bar/C_FlaBottomBar.cpp
   ${PROJECT_ROOT}/src/syde_flash/configure_node/C_FlaConNodeConfigPopup.cpp
   ${PROJECT_ROOT}/src/syde_flash/fla_main.cpp
   ${PROJECT_ROOT}/src/syde_flash/properties/C_FlaPropWidget.cpp
   ${PROJECT_ROOT}/src/syde_flash/search_nodes/C_FlaSenDcBasicSequences.cpp
   ${PROJECT_ROOT}/src/syde_flash/search_nodes/C_FlaSenSearchNodePopup.cpp
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaBitrateWidget.cpp
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetAdvancedPropertiesWidget.cpp
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetProgressWidget.cpp
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetWidget.cpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpHexFileInfo.cpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpHexFileView.cpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpListItemWidget.cpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpListWidget.cpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpProperties.cpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpSequences.cpp
   ${PROJECT_ROOT}/src/syde_flash/user_settings/C_UsFiler.cpp
   ${PROJECT_ROOT}/src/syde_flash/user_settings/C_UsHandler.cpp
   ${PROJECT_ROOT}/src/system_definition/C_SdTooltipUtil.cpp
   ${PROJECT_ROOT}/src/system_definition/node_edit/datapools/C_SdNdeDpContentUtil.cpp
   ${PROJECT_ROOT}/src/system_views/communication/C_SyvComDriverThread.cpp
   ${PROJECT_ROOT}/src/system_views/device_configuration/C_SyvDcConnectedNodeList.cpp
   ${PROJECT_ROOT}/src/system_views/device_configuration/C_SyvDcConnectedNodeWidget.cpp
   ${PROJECT_ROOT}/src/table_base/C_TblDelegate.cpp
   ${PROJECT_ROOT}/src/table_base/C_TblEditLineEditBase.cpp
   ${PROJECT_ROOT}/src/table_base/C_TblModelAction.cpp
   ${PROJECT_ROOT}/src/table_base/C_TblViewInteraction.cpp
   ${PROJECT_ROOT}/src/table_base/C_TblViewScroll.cpp
   ${PROJECT_ROOT}/src/table_base/C_TblViewToolTipBase.cpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreDataElementSortFilter.cpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreItem.cpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreModel.cpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreSimpleItem.cpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreSimpleModel.cpp
)

set(OPENSYDE_HEADERS

   ${PROJECT_ROOT}/src/constants.hpp
   ${PROJECT_ROOT}/src/help_engine/C_HeHandler.hpp
   ${PROJECT_ROOT}/src/navigable_gui/C_NagAboutDialog.hpp
   ${PROJECT_ROOT}/src/navigable_gui/C_NagToolTip.hpp
   ${PROJECT_ROOT}/src/project_operations/C_PopErrorHandling.hpp
   ${PROJECT_ROOT}/src/syde_flash/C_FlaMainWindow.hpp
   ${PROJECT_ROOT}/src/syde_flash/C_FlaTitleBarWidget.hpp
   ${PROJECT_ROOT}/src/syde_flash/bottom_bar/C_FlaBottomBar.hpp
   ${PROJECT_ROOT}/src/syde_flash/configure_node/C_FlaConNodeConfigPopup.hpp
   ${PROJECT_ROOT}/src/syde_flash/fla_constants.hpp
   ${PROJECT_ROOT}/src/syde_flash/properties/C_FlaPropWidget.hpp
   ${PROJECT_ROOT}/src/syde_flash/search_nodes/C_FlaSenDcBasicSequences.hpp
   ${PROJECT_ROOT}/src/syde_flash/search_nodes/C_FlaSenSearchNodePopup.hpp
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaBitrateWidget.hpp
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetAdvancedPropertiesWidget.hpp
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetProgressWidget.hpp
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetWidget.hpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpHexFileInfo.hpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpHexFileView.hpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpListItemWidget.hpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpListWidget.hpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpProperties.hpp
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpSequences.hpp
   ${PROJECT_ROOT}/src/syde_flash/user_settings/C_UsFiler.hpp
   ${PROJECT_ROOT}/src/syde_flash/user_settings/C_UsHandler.hpp
   ${PROJECT_ROOT}/src/system_definition/C_SdTooltipUtil.hpp
   ${PROJECT_ROOT}/src/system_definition/node_edit/datapools/C_SdNdeDpContentUtil.hpp
   ${PROJECT_ROOT}/src/system_views/communication/C_SyvComDriverThread.hpp
   ${PROJECT_ROOT}/src/system_views/device_configuration/C_SyvDcConnectedNodeList.hpp
   ${PROJECT_ROOT}/src/system_views/device_configuration/C_SyvDcConnectedNodeWidget.hpp
   ${PROJECT_ROOT}/src/table_base/C_TblDelegate.hpp
   ${PROJECT_ROOT}/src/table_base/C_TblEditLineEditBase.hpp
   ${PROJECT_ROOT}/src/table_base/C_TblModelAction.hpp
   ${PROJECT_ROOT}/src/table_base/C_TblViewInteraction.hpp
   ${PROJECT_ROOT}/src/table_base/C_TblViewScroll.hpp
   ${PROJECT_ROOT}/src/table_base/C_TblViewToolTipBase.hpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreDataElementSortFilter.hpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreItem.hpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreModel.hpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreSimpleItem.hpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreSimpleModel.hpp
)

set(OPENSYDE_FORMS

   ${PROJECT_ROOT}/src/navigable_gui/C_NagAboutDialog.ui
   ${PROJECT_ROOT}/src/navigable_gui/C_NagToolTip.ui
   ${PROJECT_ROOT}/src/syde_flash/C_FlaMainWindow.ui
   ${PROJECT_ROOT}/src/syde_flash/C_FlaTitleBarWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/bottom_bar/C_FlaBottomBar.ui
   ${PROJECT_ROOT}/src/syde_flash/configure_node/C_FlaConNodeConfigPopup.ui
   ${PROJECT_ROOT}/src/syde_flash/properties/C_FlaPropWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/search_nodes/C_FlaSenSearchNodePopup.ui
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetAdvancedPropertiesWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaBitrateWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetProgressWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpHexFileView.ui
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpListItemWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpProperties.ui
   ${PROJECT_ROOT}/src/system_views/device_configuration/C_SyvDcConnectedNodeWidget.ui
)

# --- COMPILER FLAGS AND LIBRARIES -------------------------------------------------------------------------------------

# OPENSYDE CORE
# only include relevant modules (include directories and source files)
set(OPENSYDE_CORE_SKIP_ZIPPING                     1)
set(OPENSYDE_CORE_SKIP_IMPORTS                     1)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             1)
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON     0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING            0)
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         1)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     1)
set(OPENSYDE_CORE_SKIP_XAPP_BASIC_FILE_HANDLING    1)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   1)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          1)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           1)
else()
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          0)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           0)
endif()

add_subdirectory(${PROJECT_ROOT}/libs/opensyde_core ${CMAKE_BINARY_DIR}/opensyde_core)

# SET LIBRARIES
set(OPENSYDE_LINK_LIBRARIES
   Qt6::Core
   Qt6::Gui
   Qt6::Widgets
   Qt6::Svg
   Qt6::SvgWidgets
   opensyde_core
)
if(WIN32)
   list(APPEND OPENSYDE_LINK_LIBRARIES
      version
      ws2_32   #WinSock
      Iphlpapi #IP helper API
   )
endif()

# COMPILER SWITCHES
# use same compiler switches that gcc uses when compiling openSYDE core from osc_moduletest.pro file
# TODO
add_compile_options(-Wno-deprecated-declarations -Wall -Wextra)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# do not warn about unused parameters in miniz.c; this library is not owned by us
set_source_files_properties(${PROJECT_ROOT}/libs/opensyde_core/miniz/miniz.c PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)

# --- BUILD EXECUTABLE -------------------------------------------------------------------------------------------------

# WIN32 is necessary to suppress console opening in background
if(WIN32)
   set(PLATFORM_RC_FILE ${PROJECT_ROOT}/src/syde_flash/fla_resources.rc)
else()
   set(PLATFORM_RC_FILE "")
endif()
qt_add_executable(SYDEflash
   WIN32
   ${PLATFORM_RC_FILE}
   ${PROJECT_ROOT}/src/syde_flash/fla_application.qrc
   ${OPENSYDE_SOURCES} ${OPENSYDE_HEADERS} ${OPENSYDE_FORMS})

# Set application include paths:
# marking library modules with the keyword "SYSTEM" to suppress its warnings
target_include_directories(SYDEflash PRIVATE ${OPENSYDE_INCLUDES})
target_include_directories(SYDEflash SYSTEM PRIVATE ${OPENSYDE_LIB_INCLUDES})

target_link_libraries(SYDEflash PRIVATE ${OPENSYDE_LINK_LIBRARIES})

target_precompile_headers(SYDEflash PRIVATE ${PROJECT_ROOT}/src/precompiled_headers/precomp_headers_gui.hpp)

include(GNUInstallDirs)

# create PC-lint configuration files
if(COMMAND add_pc_lint)
   add_pc_lint(SYDEflash ${OPENSYDE_SOURCES} ${OPENSYDE_HEADERS} ${OPENSYDE_FORMS})
endif(COMMAND add_pc_lint)

# --- INSTALL RESULTS --------------------------------------------------------------------------------------------------
