# minimum 3.24 for FetchContent, FILE_SET, and modern CMake policies
cmake_minimum_required(VERSION 3.24)

if(POLICY CMP0177)
   cmake_policy(SET CMP0177 NEW)
endif()
if(POLICY CMP0100)
   cmake_policy(SET CMP0100 NEW)
endif()

project(SYDEflash LANGUAGES CXX)

#include script to generate PC-lint configuration files
include(lint_config.cmake)

# Sets CMAKE_AUTOUIC and CMAKE_AUTOMOC to ON for ui files compiling
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
   enable_language("RC")
   set(CMAKE_RC_INCLUDE_PATH
      ${PROJECT_ROOT}/src/syde_flash
   )
   set(CMAKE_RC_COMPILE_OBJECT "${CMAKE_RC_COMPILER} -O coff -I${CMAKE_RC_INCLUDE_PATH} <SOURCE> <OBJECT>")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)

#signals emit etc. are in conflict with the DBC library
add_definitions(-DQT_NO_KEYWORDS)
if(WIN32)
   add_definitions(-DUNICODE -D_UNICODE)
   add_definitions(-DQT_STATICPLUGIN)
endif()

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/..)
set(OPENSYDE_GUI_ROOT ${PROJECT_ROOT}/../libraries/opensyde_gui)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Svg)
find_package(Qt6 REQUIRED COMPONENTS SvgWidgets)

# --- PROJECT INCLUDE PATHS --------------------------------------------------------------------------------------------

include_directories()
set (OPENSYDE_INCLUDES
   ${OPENSYDE_GUI_ROOT}/src
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/line_edit
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/push_button
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/spin_box
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/text_browser
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/check_box
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/label
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/menu
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/spin_box
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/util
   ${OPENSYDE_GUI_ROOT}/src/help_engine
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/check_box
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/frame
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/menu
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/radio_button
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/scroll_area
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/splitter
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_browser
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/tool_button
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget
   ${PROJECT_ROOT}/src/precompiled_headers
   ${OPENSYDE_GUI_ROOT}/src/precompiled_headers
   ${OPENSYDE_GUI_ROOT}/src/project_operations
   ${PROJECT_ROOT}/src/syde_flash
   ${PROJECT_ROOT}/src/syde_flash/bottom_bar
   ${PROJECT_ROOT}/src/syde_flash/configure_node
   ${PROJECT_ROOT}/src/syde_flash/properties
   ${PROJECT_ROOT}/src/syde_flash/search_nodes
   ${PROJECT_ROOT}/src/syde_flash/settings
   ${PROJECT_ROOT}/src/syde_flash/update
   ${PROJECT_ROOT}/src/syde_flash/user_settings
   ${OPENSYDE_GUI_ROOT}/src/user_settings
   ${PROJECT_ROOT}/src/syde_flash/util
   ${OPENSYDE_GUI_ROOT}/src/system_definition
   ${OPENSYDE_GUI_ROOT}/src/system_definition/node_edit/datapools
   ${OPENSYDE_GUI_ROOT}/src/system_views/communication
   ${OPENSYDE_GUI_ROOT}/src/system_views/device_configuration
   ${OPENSYDE_GUI_ROOT}/src/table_base
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base
   ${OPENSYDE_GUI_ROOT}/src/util
)

# Include path with library modules
set (OPENSYDE_LIB_INCLUDES
)

# --- PROJECT SOURCE FILE PATHS ----------------------------------------------------------------------------------------

set(OPENSYDE_SOURCES
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxDark.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTable.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTableSmall.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxWhite.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubDarkBrowse.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariablesBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubProjOp.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter/C_CamOgeSpi.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button/C_CamOgeTobRecentProjects.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingsBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSpinBoxGroupTable.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_AdapterBrowser.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosSectionPopup.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/util/C_CamUtiGeneric.cpp
   ${OPENSYDE_GUI_ROOT}/src/help_engine/C_HeHandler.cpp
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagAboutDialog.cpp
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagToolTip.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeContextMenu.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeOverlay.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgePopUpContentBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTitleBarWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTransparentScrollArea.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTreeToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconDelegate.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconOnly.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxMultiSelect.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxResizingView.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTableBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxText.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxHover.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxMouseMove.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabContextMenuBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabElided.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericNoPaddingNoMargins.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpTitle.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabSvgOnly.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeContextMenuBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePathBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeProperties.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/menu/C_OgeMuSections.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconOnly.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconText.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubProjAction.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStandard.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSvgIconOnly.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabProperties.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaMain.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAllBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAutoFixBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFix.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFixCustomTrigger.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleDynamicDecimalsBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64AutoFix.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64ToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebContextMenuBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedContextMenuBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedPropertiesComment.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipContent.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/tool_button/C_OgeTobToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiProgressBar.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiOnlyBackground.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiUtil.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiWithToolTip.cpp
   ${OPENSYDE_GUI_ROOT}/src/project_operations/C_PopErrorHandling.cpp
   ${PROJECT_ROOT}/src/syde_flash/C_FlaMainWindow.cpp
   ${PROJECT_ROOT}/src/syde_flash/C_FlaTitleBarWidget.cpp
   ${PROJECT_ROOT}/src/syde_flash/bottom_bar/C_FlaBottomBar.cpp
   ${PROJECT_ROOT}/src/syde_flash/configure_node/C_FlaConNodeConfigPopup.cpp
   ${PROJECT_ROOT}/src/syde_flash/fla_main.cpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/line_edit/C_FlaOgeLeFilePath.cpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/push_button/C_FlaOgePubPathVariables.cpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/spin_box/C_FlaOgeSpxDark.cpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/text_browser/C_FlaOgeTebProgressLog.cpp
   ${PROJECT_ROOT}/src/syde_flash/properties/C_FlaPropWidget.cpp
   ${PROJECT_ROOT}/src/syde_flash/search_nodes/C_FlaSenDcBasicSequences.cpp
   ${PROJECT_ROOT}/src/syde_flash/search_nodes/C_FlaSenSearchNodePopup.cpp
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
   ${PROJECT_ROOT}/src/syde_flash/util/C_FlaUti.cpp
   ${PROJECT_ROOT}/src/syde_flash/util/C_FlaUtiStyleSheets.cpp
   ${OPENSYDE_GUI_ROOT}/src/system_definition/C_SdTooltipUtil.cpp
   ${OPENSYDE_GUI_ROOT}/src/system_definition/node_edit/datapools/C_SdNdeDpContentUtil.cpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/communication/C_SyvComDriverThread.cpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/device_configuration/C_SyvDcConnectedNodeList.cpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/device_configuration/C_SyvDcConnectedNodeWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblDelegate.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblEditLineEditBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblModelAction.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewInteraction.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewScroll.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreDataElementSortFilter.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreItem.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreModel.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreSimpleItem.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreSimpleModel.cpp
   ${OPENSYDE_GUI_ROOT}/src/util/C_Uti.cpp
   ${OPENSYDE_GUI_ROOT}/src/util/C_UtiClipBoardHelper.cpp
)

set(OPENSYDE_HEADERS
   ${PROJECT_ROOT}/src/precompiled_headers/precomp_headers.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxDark.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTable.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTableSmall.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxWhite.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubDarkBrowse.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariablesBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubProjOp.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter/C_CamOgeSpi.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button/C_CamOgeTobRecentProjects.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingsBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSpinBoxGroupTable.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_AdapterBrowser.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosSectionPopup.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/util/C_CamUtiGeneric.hpp
   ${OPENSYDE_GUI_ROOT}/src/constants.hpp
   ${OPENSYDE_GUI_ROOT}/src/help_engine/C_HeHandler.hpp
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagAboutDialog.hpp
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagToolTip.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeContextMenu.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeOverlay.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgePopUpContentBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTitleBarWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTransparentScrollArea.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTreeToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconDelegate.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconOnly.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxMultiSelect.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxResizingView.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTableBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxText.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxHover.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxMouseMove.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabContextMenuBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabElided.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericNoPaddingNoMargins.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpTitle.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabSvgOnly.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeContextMenuBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePathBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeProperties.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/menu/C_OgeMuSections.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconOnly.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconText.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubProjAction.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStandard.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSvgIconOnly.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabProperties.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaMain.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAllBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAutoFixBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFix.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFixCustomTrigger.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleDynamicDecimalsBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64AutoFix.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64ToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebContextMenuBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedContextMenuBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedPropertiesComment.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipContent.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/tool_button/C_OgeTobToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiProgressBar.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiOnlyBackground.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiUtil.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiWithToolTip.hpp
   ${OPENSYDE_GUI_ROOT}/src/project_operations/C_PopErrorHandling.hpp
   ${PROJECT_ROOT}/src/syde_flash/C_FlaMainWindow.hpp
   ${PROJECT_ROOT}/src/syde_flash/C_FlaTitleBarWidget.hpp
   ${PROJECT_ROOT}/src/syde_flash/bottom_bar/C_FlaBottomBar.hpp
   ${PROJECT_ROOT}/src/syde_flash/configure_node/C_FlaConNodeConfigPopup.hpp
   ${PROJECT_ROOT}/src/syde_flash/fla_constants.hpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/line_edit/C_FlaOgeLeFilePath.hpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/push_button/C_FlaOgePubPathVariables.hpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/spin_box/C_FlaOgeSpxDark.hpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/text_browser/C_FlaOgeTebProgressLog.hpp
   ${PROJECT_ROOT}/src/syde_flash/properties/C_FlaPropWidget.hpp
   ${PROJECT_ROOT}/src/syde_flash/search_nodes/C_FlaSenDcBasicSequences.hpp
   ${PROJECT_ROOT}/src/syde_flash/search_nodes/C_FlaSenSearchNodePopup.hpp
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
   ${PROJECT_ROOT}/src/syde_flash/util/C_FlaUti.hpp
   ${PROJECT_ROOT}/src/syde_flash/util/C_FlaUtiStyleSheets.hpp
   ${OPENSYDE_GUI_ROOT}/src/system_definition/C_SdTooltipUtil.hpp
   ${OPENSYDE_GUI_ROOT}/src/system_definition/node_edit/datapools/C_SdNdeDpContentUtil.hpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/communication/C_SyvComDriverThread.hpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/device_configuration/C_SyvDcConnectedNodeList.hpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/device_configuration/C_SyvDcConnectedNodeWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblDelegate.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblEditLineEditBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblModelAction.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewInteraction.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewScroll.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreDataElementSortFilter.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreItem.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreModel.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreSimpleItem.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreSimpleModel.hpp
   ${OPENSYDE_GUI_ROOT}/src/util/C_Uti.hpp
   ${OPENSYDE_GUI_ROOT}/src/util/C_UtiClipBoardHelper.hpp
)

set(OPENSYDE_FORMS
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.ui
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagAboutDialog.ui
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagToolTip.ui
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.ui
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.ui
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.ui
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.ui
   ${PROJECT_ROOT}/src/syde_flash/C_FlaMainWindow.ui
   ${PROJECT_ROOT}/src/syde_flash/C_FlaTitleBarWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/bottom_bar/C_FlaBottomBar.ui
   ${PROJECT_ROOT}/src/syde_flash/configure_node/C_FlaConNodeConfigPopup.ui
   ${PROJECT_ROOT}/src/syde_flash/properties/C_FlaPropWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/search_nodes/C_FlaSenSearchNodePopup.ui
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetAdvancedPropertiesWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetProgressWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/settings/C_FlaSetWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpHexFileView.ui
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpListItemWidget.ui
   ${PROJECT_ROOT}/src/syde_flash/update/C_FlaUpProperties.ui
   ${OPENSYDE_GUI_ROOT}/src/system_views/device_configuration/C_SyvDcConnectedNodeWidget.ui
)

# --- COMPILER FLAGS AND LIBRARIES -------------------------------------------------------------------------------------

# OPENSYDE CORE
# only include relevant modules (include directories and source files)
set(OPENSYDE_CORE_SKIP_ZIPPING                     1)
set(OPENSYDE_CORE_SKIP_IMPORTS                     1)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             1)
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            0)
set(OPENSYDE_CORE_SKIP_PARAM_SET                   1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON     0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING            1)
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         1)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   1)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     1)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          1)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           1)
else()
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          0)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           0)
endif()
set(OPENSYDE_CORE_SKIP_SECURITY                    1)

# Only pull in our own copy of opensyde_core when nothing has provided it already.
# Building this tool on its own (the historical path, and what build.sh does) takes
# this branch and is unchanged. The root CMakeLists.txt adds one core configured with
# every subsystem enabled before adding the tools, so a whole-repo build shares a
# single core archive instead of compiling it once per tool.
if(NOT TARGET opensyde_core)
   add_subdirectory(${PROJECT_ROOT}/../libraries/opensyde_core ${CMAKE_BINARY_DIR}/opensyde_core)
endif()

# SET LIBRARIES
set(OPENSYDE_LINK_LIBRARIES
   Qt6::Core
   Qt6::Gui
   Qt6::Widgets
   Qt6::Svg
   Qt6::SvgWidgets
   opensyde_core
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   find_package(OpenSSL REQUIRED)
   list(APPEND OPENSYDE_LINK_LIBRARIES
      OpenSSL::SSL
      OpenSSL::Crypto
   )
else()
   list(APPEND OPENSYDE_LINK_LIBRARIES
      version
      ws2_32   #WinSock
      Iphlpapi #IP helper API
      ${PROJECT_ROOT}/../libraries/openssl/openssl64/libssl.a
      ${PROJECT_ROOT}/../libraries/openssl/openssl64/libcrypto.a
   )
endif()

# COMPILER SWITCHES
# use same compiler switches that gcc uses when compiling openSYDE core from osc_moduletest.pro file
# TODO
add_compile_options(-Wno-deprecated-declarations -Wall -Wextra)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# do not warn about unused parameters in miniz.c; this library is not owned by us
set_source_files_properties(${PROJECT_ROOT}/../libraries/opensyde_core/miniz/miniz.c PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)

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

target_precompile_headers(SYDEflash PRIVATE ${PROJECT_ROOT}/src/precompiled_headers/precomp_headers.hpp)

include(GNUInstallDirs)

# create PC-lint configuration files
if(COMMAND add_pc_lint)
   add_pc_lint(SYDEflash ${OPENSYDE_SOURCES} ${OPENSYDE_HEADERS} ${OPENSYDE_FORMS})
endif(COMMAND add_pc_lint)

# --- INSTALL RESULTS --------------------------------------------------------------------------------------------------
set(REPO_ROOT ${PROJECT_ROOT}/..)
install(TARGETS SYDEflash DESTINATION ${REPO_ROOT}/result/${CMAKE_BUILD_TYPE}/SYDEflash)

# --- WARNINGS AS ERRORS -----------------------------------------------------------------------------------------------
# opensyde_core has built with -Wall -Wextra -Werror and zero diagnostics since the
# core warning sweep. These trees already compiled with -Wall -Wextra -Wpedantic but
# without -Werror, and every line we own here is warning-free -- so this only pins
# what is already true.
#
# It matters because the -Wparentheses that would have caught C_OscViewFiler's nine
# inverted security-option comparisons was emitted by *these* builds, and read by
# nobody.
#
# Deliberately target-scoped rather than appended to CMAKE_CXX_FLAGS or
# add_compile_options: those are directory-scoped and propagate into
# add_subdirectory targets, including the vendored can-libraries submodule, whose
# flex-generated Vector_DBC/Scanner.cpp emits two -Woverloaded-virtual warnings we
# neither own nor can fix. Target scope keeps -Werror on our sources only.
#
# if(TARGET ...) because some of these are defined conditionally.
if(NOT MSVC)
   foreach(OSY_WERROR_TARGET SYDEflash)
      if(TARGET ${OSY_WERROR_TARGET})
         target_compile_options(${OSY_WERROR_TARGET} PRIVATE -Werror)
      endif()
   endforeach()
endif()
