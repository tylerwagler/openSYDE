# minimum 3.16 for precompiled headers
cmake_minimum_required(VERSION 3.16)

project(openSYDE_CAN_Monitor LANGUAGES CXX)

#include script to generate PC-lint configuration files
include(../lint_config.cmake)

# Sets CMAKE_AUTOUIC and CMAKE_AUTOMOC to ON for ui files compiling
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# For supporting windows .rc files - only enable on Windows
if(WIN32)
   enable_language("RC")
   # Special configuration of RC file compiling with windres necessary. Without change, the command line call is to long
   # for windres with all include paths forwarded by CMake
   set(CMAKE_RC_INCLUDE_PATH
      ${PROJECT_ROOT}/src/can_monitor
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

find_package(Qt6 REQUIRED COMPONENTS Core Xml)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 COMPONENTS Svg QUIET)
find_package(Qt6 COMPONENTS SvgWidgets QUIET)
find_package(Qt6 REQUIRED COMPONENTS SerialBus Network)

# --- PROJECT INCLUDE PATHS --------------------------------------------------------------------------------------------

include_directories()
set (OPENSYDE_INCLUDES
   ${PROJECT_ROOT}/src
   ${PROJECT_ROOT}/src/can_monitor
   ${PROJECT_ROOT}/src/can_monitor/data_base
   ${PROJECT_ROOT}/src/can_monitor/project
   ${PROJECT_ROOT}/src/can_monitor/user_settings
   ${PROJECT_ROOT}/src/can_monitor/message_trace
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings
   ${PROJECT_ROOT}/src/can_monitor/message_generator
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/check_box
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/label
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/menu
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/spin_box
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget
   ${PROJECT_ROOT}/src/can_monitor/table_base
   ${PROJECT_ROOT}/src/can_monitor/util
   ${PROJECT_ROOT}/src/help_engine
   ${PROJECT_ROOT}/src/opensyde_gui_elements
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box
   ${PROJECT_ROOT}/src/opensyde_gui_elements/frame
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit
   ${PROJECT_ROOT}/src/opensyde_gui_elements/menu
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button
   ${PROJECT_ROOT}/src/opensyde_gui_elements/radio_button
   ${PROJECT_ROOT}/src/opensyde_gui_elements/splitter
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box
   ${PROJECT_ROOT}/src/opensyde_gui_elements/scroll_area
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_browser
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tool_button
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget
   ${PROJECT_ROOT}/src/navigable_gui
   ${PROJECT_ROOT}/src/com_import_export
   ${PROJECT_ROOT}/src/table_base
   ${PROJECT_ROOT}/src/table_base/tree_base
   ${PROJECT_ROOT}/src/system_definition
   ${PROJECT_ROOT}/src/system_definition/node_edit/datapools
   ${PROJECT_ROOT}/src/system_views/communication
   ${PROJECT_ROOT}/src/project_operations/
   ${PROJECT_ROOT}/src/util
   ${PROJECT_ROOT}/libs/flexlexer
   ${PROJECT_ROOT}/libs/dbc_driver_library/src
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector
)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
   list(APPEND OPENSYDE_INCLUDES
      ${PROJECT_ROOT}/libs/zlib
   )
endif()

# Include path with library modules
set (OPENSYDE_LIB_INCLUDES
   ${PROJECT_ROOT}/libs/flexlexer
)

# --- PROJECT SOURCE FILE PATHS ----------------------------------------------------------------------------------------

set(OPENSYDE_SOURCES
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariablesBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingsBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosBitrateWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/util/C_CamUtiGeneric.cpp
   ${PROJECT_ROOT}/src/util/C_Uti.cpp
   ${PROJECT_ROOT}/src/system_views/communication/C_SyvComMessageMonitor.cpp
   ${PROJECT_ROOT}/src/system_views/communication/C_SyvComDriverThread.cpp
   ${PROJECT_ROOT}/src/com_import_export/C_CieImportDbc.cpp
   ${PROJECT_ROOT}/src/com_import_export/C_CieConverter.cpp
   ${PROJECT_ROOT}/src/system_definition/node_edit/datapools/C_SdNdeDpContentUtil.cpp
   ${PROJECT_ROOT}/src/can_monitor/cam_main.cpp
   ${PROJECT_ROOT}/src/project_operations/C_PopErrorHandling.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/AttributeDefinition.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/AttributeRelation.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/AttributeValueType.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/BitTiming.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/EnvironmentVariable.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Message.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Network.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Parser.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Signal.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/SignalGroup.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/SignalType.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/ValueTable.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/platform.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/frame/C_OgeFraStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeOverlay.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxMouseMove.cpp
   ${PROJECT_ROOT}/src/navigable_gui/C_NagToolTip.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiUtil.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpTitle.cpp
   ${PROJECT_ROOT}/src/help_engine/C_HeHandler.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabContextMenuBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpSubTitle.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebContextMenuBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipContent.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeContextMenu.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStyledToolTip.cpp
   ${PROJECT_ROOT}/src/can_monitor/C_CamMainWindow.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiOnlyBackground.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiWithToolTip.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeDelegate.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeGuiBuffer.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeModel.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeView.cpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreItem.cpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreModel.cpp
   ${PROJECT_ROOT}/src/can_monitor/user_settings/C_UsHandler.cpp
   ${PROJECT_ROOT}/src/can_monitor/user_settings/C_UsFiler.cpp
   ${PROJECT_ROOT}/src/navigable_gui/C_NagAboutDialog.cpp
${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabToolTipBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyled.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyledContextMenu.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabDashboardDefault.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePath.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconOnly.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyledToolTip.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyled.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabSvgOnly.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabUnified.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxUnified.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeUnified.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyledToolTip.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyledContextMenu.cpp
   ${PROJECT_ROOT}/src/can_monitor/C_CamComDriverBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetStatusBarWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetControlBarWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosBitrateWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosLoggingWidget.cpp
${PROJECT_ROOT}/src/can_monitor/C_CamTitleBarWidget.cpp
    ${PROJECT_ROOT}/src/can_monitor/util/C_CamUtiStyleSheets.cpp
    ${PROJECT_ROOT}/src/util/C_UtiClipBoardHelper.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetClipBoardHelper.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter/C_CamOgeSpi.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiBase.cpp
${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericNoPaddingNoMargins.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabToolTipBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubProjAction.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubUnified.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxToolTipBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxUnified.cpp
    ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableView.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableModel.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeContextMenuBase.cpp
   ${PROJECT_ROOT}/src/util/C_UtiFindNameHelper.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenMessagesWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProHandler.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProHandlerFiler.cpp
${PROJECT_ROOT}/src/can_monitor/project/C_CamProMessageData.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubProjOp.cpp
    ${PROJECT_ROOT}/src/can_monitor/project/C_CamProClipBoardHelper.cpp
    ${PROJECT_ROOT}/src/table_base/C_TblModelAction.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterItemWidget.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTransparentScrollArea.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/check_box/C_CamOgeChxSettings.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxToolTipBase.cpp
   ${PROJECT_ROOT}/src/table_base/C_TblViewScroll.cpp
${PROJECT_ROOT}/src/table_base/C_TblViewToolTipBase.cpp
    ${PROJECT_ROOT}/src/table_base/C_TblDelegate.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeToolTipBase.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit/C_CamOgeLeTableEdit.cpp
   ${PROJECT_ROOT}/src/table_base/C_TblEditLineEditBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconDelegate.cpp
${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxResizingView.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box/C_CamOgeGbxMessageGeneratorBackground.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterPopup.cpp
${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedPropertiesComment.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedContextMenuBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit/C_CamOgeLeIdPopUp.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxText.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProFilterData.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaMain.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterTableView.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterTableModel.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableView.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableModel.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigWidget.cpp
   ${PROJECT_ROOT}/src/table_base/C_TblViewInteraction.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeLoggerData.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box/C_CamOgeGbxMessageSignalsBorder.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseItemWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProDatabaseData.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseBusSelectionPopup.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosSectionPopup.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableDelegate.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenKeySelect.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/spin_box/C_CamOgeSpxProperties.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAllBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAutoFixBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFix.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64AutoFix.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64ToolTipBase.cpp
${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleDynamicDecimalsBase.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSpinBoxGroupTable.cpp
    ${PROJECT_ROOT}/src/can_monitor/project/C_CamProLoggingData.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbc.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbcMessageId.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbHandler.cpp
${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsy.cpp
    ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigUtil.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/menu/C_CamOgeMuRecentProjects.cpp
    ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreDataElementSortFilter.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewToolTipBase.cpp
${PROJECT_ROOT}/src/system_definition/C_SdTooltipUtil.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionModel.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionPopup.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionView.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button/C_CamOgeTobRecentProjects.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tool_button/C_OgeTobToolTipBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/util/C_CamUti.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableDelegate.cpp
   ${PROJECT_ROOT}/src/table_base/C_TblDelegateUtil.cpp
${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeToolTipBase.cpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariables.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePathBase.cpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/menu/C_OgeMuSections.cpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreSimpleItem.cpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreSimpleModel.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTableBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeLoggerDataGreyOutInformation.cpp
   ${PROJECT_ROOT}/src/can_monitor/table_base/C_CamTblDelegate.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetUtil.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetSettingsPopup.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxMultiSelect.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabElided.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFixCustomTrigger.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsyMessageId.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsyListId.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbcUnmappedMessageId.cpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Scanner.cpp
)

set(OPENSYDE_HEADERS
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariablesBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingsBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosBitrateWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/util/C_CamUtiGeneric.hpp
   ${PROJECT_ROOT}/src/util/C_Uti.hpp
   ${PROJECT_ROOT}/src/system_views/communication/C_SyvComMessageMonitor.hpp
   ${PROJECT_ROOT}/src/system_views/communication/C_SyvComDriverThread.hpp
   ${PROJECT_ROOT}/libs/flexlexer/FlexLexer.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Attribute.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/AttributeDefinition.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/AttributeObjectType.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/AttributeRelation.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/AttributeValueType.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/BitTiming.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/ByteOrder.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/EnvironmentVariable.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/ExtendedMultiplexor.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Message.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Network.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Node.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Parser.hpp
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Scanner.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Signal.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/SignalGroup.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/SignalType.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/ValueDescriptions.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/ValueTable.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/ValueType.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/location.hh
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/platform.h
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/position.hh
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/stack.hh
   ${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/vector_dbc_export.h
   ${PROJECT_ROOT}/src/com_import_export/C_CieImportDbc.hpp
   ${PROJECT_ROOT}/src/com_import_export/C_CieConverter.hpp
   ${PROJECT_ROOT}/src/system_definition/node_edit/datapools/C_SdNdeDpContentUtil.hpp
   ${PROJECT_ROOT}/src/project_operations/C_PopErrorHandling.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/frame/C_OgeFraStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeOverlay.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxMouseMove.hpp
   ${PROJECT_ROOT}/src/navigable_gui/C_NagToolTip.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiUtil.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpTitle.hpp
   ${PROJECT_ROOT}/src/help_engine/C_HeHandler.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabContextMenuBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpSubTitle.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebContextMenuBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipContent.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeContextMenu.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStyledToolTip.hpp
   ${PROJECT_ROOT}/src/can_monitor/C_CamMainWindow.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiOnlyBackground.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiWithToolTip.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeDelegate.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeGuiBuffer.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeModel.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeView.hpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreItem.hpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreModel.hpp
   ${PROJECT_ROOT}/src/can_monitor/cam_constants.hpp
   ${PROJECT_ROOT}/src/can_monitor/user_settings/C_UsHandler.hpp
   ${PROJECT_ROOT}/src/can_monitor/user_settings/C_UsFiler.hpp
   ${PROJECT_ROOT}/src/navigable_gui/C_NagAboutDialog.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyledToolTip.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyledContextMenu.hpp
   ${PROJECT_ROOT}/src/can_monitor/C_CamComDriverBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetStatusBarWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetControlBarWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosBitrateWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosLoggingWidget.hpp
${PROJECT_ROOT}/src/can_monitor/C_CamTitleBarWidget.hpp
    ${PROJECT_ROOT}/src/can_monitor/util/C_CamUtiStyleSheets.hpp
    ${PROJECT_ROOT}/src/constants.hpp
   ${PROJECT_ROOT}/src/util/C_UtiClipBoardHelper.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetClipBoardHelper.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter/C_CamOgeSpi.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiBase.hpp
${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericNoPaddingNoMargins.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabToolTipBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubProjAction.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubUnified.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxToolTipBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxUnified.hpp
    ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableView.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableModel.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeContextMenuBase.hpp
   ${PROJECT_ROOT}/src/util/C_UtiFindNameHelper.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenMessagesWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProHandler.hpp
${PROJECT_ROOT}/src/can_monitor/project/C_CamProHandlerFiler.hpp
    ${PROJECT_ROOT}/src/can_monitor/project/C_CamProMessageData.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubProjOp.hpp
    ${PROJECT_ROOT}/src/can_monitor/project/C_CamProClipBoardHelper.hpp
    ${PROJECT_ROOT}/src/table_base/C_TblModelAction.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterItemWidget.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTransparentScrollArea.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/check_box/C_CamOgeChxSettings.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxToolTipBase.hpp
${PROJECT_ROOT}/src/table_base/C_TblViewScroll.hpp
    ${PROJECT_ROOT}/src/table_base/C_TblViewToolTipBase.hpp
    ${PROJECT_ROOT}/src/table_base/C_TblDelegate.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeToolTipBase.hpp
${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit/C_CamOgeLeTableEdit.hpp
    ${PROJECT_ROOT}/src/table_base/C_TblEditLineEditBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconDelegate.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxResizingView.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box/C_CamOgeGbxMessageGeneratorBackground.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterPopup.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedPropertiesComment.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedContextMenuBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit/C_CamOgeLeIdPopUp.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxText.hpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProFilterData.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconOnly.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaMain.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterTableView.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterTableModel.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableView.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableModel.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigWidget.hpp
   ${PROJECT_ROOT}/src/table_base/C_TblViewInteraction.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeLoggerData.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box/C_CamOgeGbxMessageSignalsBorder.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseItemWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProDatabaseData.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseBusSelectionPopup.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosSectionPopup.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableDelegate.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenKeySelect.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/spin_box/C_CamOgeSpxProperties.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAllBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAutoFixBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFix.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64.hpp
${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64AutoFix.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleToolTipBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64ToolTipBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleDynamicDecimalsBase.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSpinBoxGroupTable.hpp
    ${PROJECT_ROOT}/src/can_monitor/project/C_CamProLoggingData.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbc.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbcMessageId.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbHandler.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsy.hpp
${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigUtil.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/menu/C_CamOgeMuRecentProjects.hpp
    ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreDataElementSortFilter.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewToolTipBase.hpp
${PROJECT_ROOT}/src/system_definition/C_SdTooltipUtil.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionModel.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionPopup.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionView.hpp
    ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button/C_CamOgeTobRecentProjects.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tool_button/C_OgeTobToolTipBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/util/C_CamUti.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableDelegate.hpp
   ${PROJECT_ROOT}/src/table_base/C_TblDelegateUtil.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeToolTipBase.hpp
${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariables.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePathBase.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePath.hpp
    ${PROJECT_ROOT}/src/opensyde_gui_elements/menu/C_OgeMuSections.hpp
    ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreSimpleItem.hpp
   ${PROJECT_ROOT}/src/table_base/tree_base/C_TblTreSimpleModel.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTableBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeLoggerDataGreyOutInformation.hpp
   ${PROJECT_ROOT}/src/can_monitor/table_base/C_CamTblDelegate.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetUtil.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetSettingsPopup.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxMultiSelect.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabElided.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFixCustomTrigger.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsyMessageId.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsyListId.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbcUnmappedMessageId.hpp
)

set(OPENSYDE_FORMS
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.ui
   ${PROJECT_ROOT}/src/navigable_gui/C_NagToolTip.ui
   ${PROJECT_ROOT}/src/can_monitor/C_CamMainWindow.ui
   ${PROJECT_ROOT}/src/navigable_gui/C_NagAboutDialog.ui
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetStatusBarWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetControlBarWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosBitrateWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosLoggingWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/C_CamTitleBarWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.ui
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenMessagesWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterItemWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterPopup.ui
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseItemWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseBusSelectionPopup.ui
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenKeySelect.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionPopup.ui
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetSettingsPopup.ui
)

# --- COMPILER FLAGS AND LIBRARIES -------------------------------------------------------------------------------------

# OPENSYDE CORE
# only include relevant modules (include directories and source files)
set(OPENSYDE_CORE_SKIP_ZIPPING                     1)
set(OPENSYDE_CORE_SKIP_IMPORTS                     1)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             1)
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    0)
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
   Qt6::Xml
   Qt6::Gui
   Qt6::Widgets
   Qt6::Svg
   Qt6::SvgWidgets
   Qt6::SerialBus
   Qt6::Network
   opensyde_core
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   find_package(ZLIB REQUIRED)
   list(APPEND OPENSYDE_LINK_LIBRARIES
      ZLIB::ZLIB
   )
else()
   list(APPEND OPENSYDE_LINK_LIBRARIES
      ${PROJECT_ROOT}/libs/zlib/libzlibstatic.a
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
#TODO
#set_source_files_properties(${PROJECT_ROOT}/libs/dbc_driver_library/src/Vector/DBC/Scanner.cpp PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)

# --- BUILD EXECUTABLE -------------------------------------------------------------------------------------------------

# WIN32 is necessary to suppress console opening in background
if(WIN32)
   set(PLATFORM_RC_FILE ${PROJECT_ROOT}/src/can_monitor/cam_resources.rc)
else()
   set(PLATFORM_RC_FILE "")
endif()
qt_add_executable(openSYDE_CAN_Monitor
   WIN32
   ${PLATFORM_RC_FILE}
   ${PROJECT_ROOT}/src/can_monitor/cam_application.qrc
   ${OPENSYDE_SOURCES} ${OPENSYDE_HEADERS} ${OPENSYDE_FORMS})

# Set application include paths:
# marking library modules with the keyword "SYSTEM" to suppress its warnings
target_include_directories(openSYDE_CAN_Monitor PRIVATE ${OPENSYDE_INCLUDES})
target_include_directories(openSYDE_CAN_Monitor SYSTEM PRIVATE ${OPENSYDE_LIB_INCLUDES})

target_link_libraries(openSYDE_CAN_Monitor PRIVATE ${OPENSYDE_LINK_LIBRARIES})

target_precompile_headers(openSYDE_CAN_Monitor PRIVATE ${PROJECT_ROOT}/src/precompiled_headers/precomp_headers_gui.hpp)

include(GNUInstallDirs)

# create PC-lint configuration files
if(COMMAND add_pc_lint)
   add_pc_lint(openSYDE_CAN_Monitor ${OPENSYDE_SOURCES} ${OPENSYDE_HEADERS} ${OPENSYDE_FORMS})
endif(COMMAND add_pc_lint)

# --- INSTALL RESULTS --------------------------------------------------------------------------------------------------
install(TARGETS openSYDE_CAN_Monitor DESTINATION ${PROJECT_ROOT}/result/tool/CAN_Monitor)


