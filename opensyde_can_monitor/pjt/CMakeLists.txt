# minimum 3.16 for precompiled headers
cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0177)
   cmake_policy(SET CMP0177 NEW)
endif()
if(POLICY CMP0100)
   cmake_policy(SET CMP0100 NEW)
endif()

project(openSYDE_CAN_Monitor LANGUAGES CXX)

#include script to generate PC-lint configuration files
include(lint_config.cmake)

# Sets CMAKE_AUTOUIC and CMAKE_AUTOMOC to ON for ui files compiling
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# For supporting windows .rc files - only enable on Windows
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
   enable_language("RC")
   set(CMAKE_RC_INCLUDE_PATH
      ${PROJECT_ROOT}/src/can_monitor
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

set(CMAKE_CXX_STANDARD 17)
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
   ${PROJECT_ROOT}/src/can_monitor
   ${PROJECT_ROOT}/src/can_monitor/data_base
   ${PROJECT_ROOT}/src/can_monitor/project
   ${PROJECT_ROOT}/src/can_monitor/user_settings
   ${PROJECT_ROOT}/src/can_monitor/message_trace
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings
   ${PROJECT_ROOT}/src/can_monitor/message_generator
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/check_box
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/label
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/menu
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/spin_box
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget
   ${PROJECT_ROOT}/src/can_monitor/table_base
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/util
   ${OPENSYDE_GUI_ROOT}/src/help_engine
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
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/splitter
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/scroll_area
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_browser
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/tool_button
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui
   ${OPENSYDE_GUI_ROOT}/src/com_import_export
   ${OPENSYDE_GUI_ROOT}/src/table_base
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base
   ${OPENSYDE_GUI_ROOT}/src/system_definition
   ${OPENSYDE_GUI_ROOT}/src/system_definition/node_edit/datapools
   ${OPENSYDE_GUI_ROOT}/src/system_views/communication
   ${OPENSYDE_GUI_ROOT}/src/project_operations/
   ${OPENSYDE_GUI_ROOT}/src/gettext
   ${OPENSYDE_GUI_ROOT}/src/util
   ${PROJECT_ROOT}/src/precompiled_headers
   ${PROJECT_ROOT}/../libraries/flexlexer
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector
   ${PROJECT_ROOT}/../libraries/vector_blf/src/Vector
)

# Include path with library modules
set (OPENSYDE_LIB_INCLUDES
   ${PROJECT_ROOT}/../libraries/flexlexer
)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
   list(APPEND OPENSYDE_LIB_INCLUDES ${PROJECT_ROOT}/../libraries/gettext)
endif()

# --- PROJECT SOURCE FILE PATHS ----------------------------------------------------------------------------------------

set(OPENSYDE_SOURCES
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariablesBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingsBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDllWidgetLogic.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/util/C_CamUtiGeneric.cpp
   ${OPENSYDE_GUI_ROOT}/src/gettext/C_GtGetText.cpp
   ${OPENSYDE_GUI_ROOT}/src/util/C_Uti.cpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/communication/C_SyvComMessageMonitor.cpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/communication/C_SyvComMessageLoggerFileBlf.cpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/communication/C_SyvComDriverThread.cpp
   ${OPENSYDE_GUI_ROOT}/src/com_import_export/C_CieImportDbc.cpp
   ${OPENSYDE_GUI_ROOT}/src/com_import_export/C_CieConverter.cpp
   ${OPENSYDE_GUI_ROOT}/src/system_definition/node_edit/datapools/C_SdNdeDpContentUtil.cpp
   ${PROJECT_ROOT}/src/can_monitor/cam_main.cpp
   ${OPENSYDE_GUI_ROOT}/src/project_operations/C_PopErrorHandling.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/AttributeDefinition.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/AttributeRelation.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/AttributeValueType.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/BitTiming.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/EnvironmentVariable.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Message.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Network.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Parser.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Signal.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/SignalGroup.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/SignalType.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/ValueTable.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/platform.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgePopUpContentBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTitleBarWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeOverlay.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxMouseMove.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconText.cpp
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagToolTip.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiUtil.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpTitle.cpp
   ${OPENSYDE_GUI_ROOT}/src/help_engine/C_HeHandler.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxHover.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabContextMenuBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStandard.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebContextMenuBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipContent.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeContextMenu.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubToolTipBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/C_CamMainWindow.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiOnlyBackground.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiWithToolTip.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeDelegate.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeGuiBuffer.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeModel.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeView.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreItem.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreModel.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSvgIconOnly.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabSvgOnly.cpp
   ${PROJECT_ROOT}/src/can_monitor/user_settings/C_UsHandler.cpp
   ${PROJECT_ROOT}/src/can_monitor/user_settings/C_UsFiler.cpp
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagAboutDialog.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabToolTipBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/C_CamComDriverBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetStatusBarWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetControlBarWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDllWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosLoggingWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/C_CamTitleBarWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/util/C_CamUtiStyleSheets.cpp
   ${OPENSYDE_GUI_ROOT}/src/util/C_UtiClipBoardHelper.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetClipBoardHelper.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter/C_CamOgeSpi.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericNoPaddingNoMargins.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabProperties.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubProjAction.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxWhite.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableView.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableModel.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeContextMenuBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/util/C_UtiFindNameHelper.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenMessagesWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProHandler.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProHandlerFiler.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProMessageData.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubProjOp.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProClipBoardHelper.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblModelAction.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterItemWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTransparentScrollArea.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewScroll.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblDelegate.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTable.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblEditLineEditBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconDelegate.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxResizingView.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterPopup.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedPropertiesComment.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedContextMenuBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxText.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconOnly.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProFilterData.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconOnly.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaMain.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterTableView.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterTableModel.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableView.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableModel.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigWidget.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewInteraction.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeLoggerData.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseItemWidget.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProDatabaseData.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseBusSelectionPopup.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosSectionPopup.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableDelegate.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenKeySelect.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAllBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAutoFixBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFix.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64AutoFix.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64ToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleDynamicDecimalsBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSpinBoxGroupTable.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxDark.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubDarkBrowse.cpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProLoggingData.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbc.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbcMessageId.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbHandler.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsy.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigUtil.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeProperties.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreDataElementSortFilter.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/system_definition/C_SdTooltipUtil.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionModel.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionPopup.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionView.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTableSmall.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button/C_CamOgeTobRecentProjects.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/tool_button/C_OgeTobToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/util/C_CamUti.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableDelegate.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblDelegateUtil.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTreeToolTipBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariables.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePathBase.cpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit/C_CamOgeLeFilePath.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/menu/C_OgeMuSections.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreSimpleItem.cpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreSimpleModel.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTableBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeLoggerDataGreyOutInformation.cpp
   ${PROJECT_ROOT}/src/can_monitor/table_base/C_CamTblDelegate.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetUtil.cpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetSettingsPopup.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxMultiSelect.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabElided.cpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFixCustomTrigger.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsyMessageId.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsyListId.cpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbcUnmappedMessageId.cpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Scanner.cpp
)

set(OPENSYDE_HEADERS
   ${PROJECT_ROOT}/src/precompiled_headers/precomp_headers.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariablesBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingsBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDllWidgetLogic.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/util/C_CamUtiGeneric.hpp
   ${OPENSYDE_GUI_ROOT}/src/gettext/C_GtGetText.hpp
   ${OPENSYDE_GUI_ROOT}/src/util/C_Uti.hpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/communication/C_SyvComMessageMonitor.hpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/communication/C_SyvComMessageLoggerFileBlf.hpp
   ${OPENSYDE_GUI_ROOT}/src/system_views/communication/C_SyvComDriverThread.hpp
   ${PROJECT_ROOT}/../libraries/flexlexer/FlexLexer.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Attribute.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/AttributeDefinition.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/AttributeObjectType.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/AttributeRelation.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/AttributeValueType.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/BitTiming.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/ByteOrder.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/EnvironmentVariable.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/ExtendedMultiplexor.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Message.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Network.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Node.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Parser.hpp
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Scanner.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Signal.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/SignalGroup.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/SignalType.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/ValueDescriptions.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/ValueTable.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/ValueType.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/location.hh
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/platform.h
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/position.hh
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/stack.hh
   ${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/vector_dbc_export.h
   ${OPENSYDE_GUI_ROOT}/src/com_import_export/C_CieImportDbc.hpp
   ${OPENSYDE_GUI_ROOT}/src/com_import_export/C_CieConverter.hpp
   ${OPENSYDE_GUI_ROOT}/src/system_definition/node_edit/datapools/C_SdNdeDpContentUtil.hpp
   ${OPENSYDE_GUI_ROOT}/src/project_operations/C_PopErrorHandling.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgePopUpContentBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTitleBarWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeOverlay.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxMouseMove.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconText.hpp
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagToolTip.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiUtil.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpTitle.hpp
   ${OPENSYDE_GUI_ROOT}/src/help_engine/C_HeHandler.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxHover.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabContextMenuBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStandard.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebContextMenuBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipContent.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeContextMenu.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubToolTipBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/C_CamMainWindow.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiOnlyBackground.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiWithToolTip.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeDelegate.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeGuiBuffer.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeModel.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeView.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreItem.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreModel.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSvgIconOnly.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabSvgOnly.hpp
   ${PROJECT_ROOT}/src/can_monitor/cam_constants.hpp
   ${PROJECT_ROOT}/src/can_monitor/user_settings/C_UsHandler.hpp
   ${PROJECT_ROOT}/src/can_monitor/user_settings/C_UsFiler.hpp
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagAboutDialog.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabToolTipBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/C_CamComDriverBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetStatusBarWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetControlBarWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDllWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosLoggingWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/C_CamTitleBarWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/util/C_CamUtiStyleSheets.hpp
   ${OPENSYDE_GUI_ROOT}/src/constants.hpp
   ${OPENSYDE_GUI_ROOT}/src/util/C_UtiClipBoardHelper.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetClipBoardHelper.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter/C_CamOgeSpi.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericNoPaddingNoMargins.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabProperties.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubProjAction.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxWhite.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableView.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableModel.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeContextMenuBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/util/C_UtiFindNameHelper.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenMessagesWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProHandler.hpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProHandlerFiler.hpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProMessageData.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubProjOp.hpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProClipBoardHelper.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblModelAction.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterItemWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTransparentScrollArea.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewScroll.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblDelegate.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTable.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblEditLineEditBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconDelegate.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxResizingView.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterPopup.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedPropertiesComment.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedContextMenuBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxText.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconOnly.hpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProFilterData.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconOnly.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaMain.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterTableView.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterTableModel.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableView.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableModel.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigWidget.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblViewInteraction.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeLoggerData.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseItemWidget.hpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProDatabaseData.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseBusSelectionPopup.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosSectionPopup.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigTableDelegate.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenKeySelect.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAllBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAutoFixBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFix.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64AutoFix.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64ToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleDynamicDecimalsBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSpinBoxGroupTable.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxDark.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubDarkBrowse.hpp
   ${PROJECT_ROOT}/src/can_monitor/project/C_CamProLoggingData.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbc.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbcMessageId.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbHandler.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsy.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigUtil.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeProperties.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreDataElementSortFilter.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/system_definition/C_SdTooltipUtil.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionModel.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionPopup.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionView.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTableSmall.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button/C_CamOgeTobRecentProjects.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/tool_button/C_OgeTobToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/util/C_CamUti.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenTableDelegate.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/C_TblDelegateUtil.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgeTreeToolTipBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariables.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePathBase.hpp
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit/C_CamOgeLeFilePath.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/menu/C_OgeMuSections.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreSimpleItem.hpp
   ${OPENSYDE_GUI_ROOT}/src/table_base/tree_base/C_TblTreSimpleModel.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTableBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetTreeLoggerDataGreyOutInformation.hpp
   ${PROJECT_ROOT}/src/can_monitor/table_base/C_CamTblDelegate.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetUtil.hpp
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetSettingsPopup.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxMultiSelect.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/label/C_OgeLabElided.hpp
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFixCustomTrigger.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsyMessageId.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbOsyListId.hpp
   ${PROJECT_ROOT}/src/can_monitor/data_base/C_CamDbDbcUnmappedMessageId.hpp
)

set(OPENSYDE_FORMS
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.ui
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.ui
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.ui
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagToolTip.ui
   ${PROJECT_ROOT}/src/can_monitor/C_CamMainWindow.ui
   ${OPENSYDE_GUI_ROOT}/src/navigable_gui/C_NagAboutDialog.ui
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetStatusBarWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetControlBarWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenWidget.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosWidget.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDllWidget.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterWidget.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseWidget.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosLoggingWidget.ui
   ${PROJECT_ROOT}/src/can_monitor/C_CamTitleBarWidget.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.ui
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenMessagesWidget.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterItemWidget.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosFilterPopup.ui
   ${PROJECT_ROOT}/src/can_monitor/message_generator/signals/C_CamGenSigWidget.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseItemWidget.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseBusSelectionPopup.ui
   ${PROJECT_ROOT}/src/can_monitor/message_generator/C_CamGenKeySelect.ui
   ${OPENSYDE_GUI_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.ui
   ${OPENSYDE_GUI_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionPopup.ui
   ${PROJECT_ROOT}/src/can_monitor/message_trace/C_CamMetSettingsPopup.ui
)

# --- COMPILER FLAGS AND LIBRARIES -------------------------------------------------------------------------------------

# OPENSYDE CORE
# only include relevant modules (include directories and source files)
set(OPENSYDE_CORE_SKIP_ZIPPING                     1)
set(OPENSYDE_CORE_SKIP_IMPORTS                     1)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             1)
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            0)
set(OPENSYDE_CORE_SKIP_PARAM_SET                   1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON     0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING            0)
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

add_subdirectory(${PROJECT_ROOT}/../libraries/opensyde_core ${CMAKE_BINARY_DIR}/opensyde_core)

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
   find_package(PkgConfig REQUIRED)
   find_package(ZLIB REQUIRED)
   # Build vector_blf as a static library (upstream hardcodes SHARED)
   add_subdirectory(${PROJECT_ROOT}/../cmake/vector_blf_static ${CMAKE_BINARY_DIR}/vector_blf EXCLUDE_FROM_ALL)
   list(APPEND OPENSYDE_LINK_LIBRARIES
      Vector_BLF
      ZLIB::ZLIB
   )
else()
   list(APPEND OPENSYDE_LINK_LIBRARIES
      version
      ws2_32   #WinSock
      Iphlpapi #IP helper API
      ${PROJECT_ROOT}/../libraries/gettext/libintl.a
      ${PROJECT_ROOT}/../libraries/blf_driver_library/libvector_blf.a
      ${PROJECT_ROOT}/../libraries/zlib/libzlibstatic.a
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
#TODO
#set_source_files_properties(${PROJECT_ROOT}/../libraries/dbc_driver_library/src/Vector/DBC/Scanner.cpp PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)

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

target_precompile_headers(openSYDE_CAN_Monitor PRIVATE ${PROJECT_ROOT}/src/precompiled_headers/precomp_headers.hpp)

include(GNUInstallDirs)

# create PC-lint configuration files
if(COMMAND add_pc_lint)
   add_pc_lint(openSYDE_CAN_Monitor ${OPENSYDE_SOURCES} ${OPENSYDE_HEADERS} ${OPENSYDE_FORMS})
endif(COMMAND add_pc_lint)

# --- INSTALL RESULTS --------------------------------------------------------------------------------------------------
set(REPO_ROOT ${PROJECT_ROOT}/..)
install(TARGETS openSYDE_CAN_Monitor DESTINATION ${REPO_ROOT}/result/${CMAKE_BUILD_TYPE}/openSYDE_CAN_Monitor)
