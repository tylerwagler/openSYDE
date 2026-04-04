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
   ${PROJECT_ROOT}/src/can_monitor
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/check_box
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/menu
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/spin_box
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings
   ${PROJECT_ROOT}/src/can_monitor/project
   ${PROJECT_ROOT}/src/help_engine
   ${PROJECT_ROOT}/src/navigable_gui
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
   ${PROJECT_ROOT}/src/opensyde_gui_elements/scroll_area
   ${PROJECT_ROOT}/src/opensyde_gui_elements/slider
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box
   ${PROJECT_ROOT}/src/opensyde_gui_elements/splitter
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tab_widget
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_browser
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tool_button
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget
   ${PROJECT_ROOT}/src/project_operations
   ${PROJECT_ROOT}/src/syde_flash
   ${PROJECT_ROOT}/src/util
   ${PROJECT_ROOT}/src/syde_flash/bottom_bar
   ${PROJECT_ROOT}/src/syde_flash/configure_node
   ${PROJECT_ROOT}/src/syde_flash/gui_elements
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/line_edit
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/push_button
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/spin_box
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/text_browser
   ${PROJECT_ROOT}/src/syde_flash/properties
   ${PROJECT_ROOT}/src/syde_flash/search_nodes
   ${PROJECT_ROOT}/src/syde_flash/settings
   ${PROJECT_ROOT}/src/syde_flash/update
   ${PROJECT_ROOT}/src/syde_flash/user_settings
   ${PROJECT_ROOT}/src/syde_flash/util
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
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/check_box/C_CamOgeChxSettings.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box/C_CamOgeGbxMessageGeneratorBackground.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box/C_CamOgeGbxMessageSignalsBorder.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit/C_CamOgeLeIdPopUp.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit/C_CamOgeLeTableEdit.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/menu/C_CamOgeMuRecentProjects.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariables.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariablesBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubProjOp.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/spin_box/C_CamOgeSpxProperties.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter/C_CamOgeSpi.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button/C_CamOgeTobRecentProjects.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingsBase.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSpinBoxGroupTable.cpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosSectionPopup.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeContextMenu.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeDarkScrollArea.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeHorizontalListWidget.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeListViewToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeListWidgetToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeOverlay.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeSmoothScrollArea.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTabBar.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTableWidgetComIf.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTransparentScrollArea.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewCheckable.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewToolTipBaseCheckable.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeWidgetToolBarSearch.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeWidgetToolBarSearchItemDelegate.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeWidgetToolBarSearchItemWidget.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeWidgetToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxDefaultSmall.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxTristateBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxUnified.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconDelegate.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxMultiSelect.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxMultiSelectTableHalc.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxResizingView.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTable.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTableBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTableHalc.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxText.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxUnified.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/frame/C_OgeFraGeneric.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/frame/C_OgeFraStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxMouseMove.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxNavigationSection.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxNavigationSectionSelected.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxNavigationTab.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxTransparentDashboard.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabAdaptiveSize.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabContextMenuBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabDashboardDefault.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabDashboardLabelCaption.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabDashboardLabelValue.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabDoubleClick.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabElided.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabExternalLink.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabFrameError.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericBubble.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericNoPaddingNoMargins.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericWithContextMenu.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpSubTitle.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpTitle.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyledContextMenu.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyledToolTip.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabSvgOnly.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabUnified.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeComboBox.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeContextMenuBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePath.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePathBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeSearch.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeUnified.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/menu/C_OgeMuSections.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/menu/C_OgeMuTabChartFit.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubAddChartDataElement.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubBackNavigation.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubColor.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubDashboard.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubGenericTextWithBackground.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconChart.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconEvents.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconOnly.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubNavigationHover.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubOpen.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubOptions.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubPathVariables.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubPrevNavigation.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubProjAction.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStyledToolTip.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSvgIconOnly.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSvgIconWithText.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSvgIconWithTextBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSystemCommissioningEdit.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubTabClose.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubToolBar.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubTreeWidgetRoot.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubUnified.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubUpdate.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubUseCase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubUseCaseNav.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaMain.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAllBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAutoFixBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFix.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFixCustomTrigger.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleDynamicDecimalsBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleProperties.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxEditProperties.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxFactor.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxFactorProperties.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxFactorStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxFactorTable.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64AutoFix.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64Factor.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64FactorTable.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64Properties.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64Styled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64Table.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64ToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxNumber.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxTable.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxTableComIf.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxTableDouble.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxTableHalc.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiHorizontalNavigation.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiStandard.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tab_widget/C_OgeTawAlternative.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tab_widget/C_OgeTawStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tab_widget/C_OgeTawToolBox.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tab_widget/C_OgeTawToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebContextMenuBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebStyled.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedContextMenuBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedElided.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedPropertiesComment.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedTable.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipContent.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tool_button/C_OgeTobToolTipBase.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiBopperle.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiBorder.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiEditBackground.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiFixPosition.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiFontConfig.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiHover.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiNavigationTab.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiOnlyBackground.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiPieChart.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiProgressBar.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiProgressBopperle.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiProgressLog.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroupProperties.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiUtil.cpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiWithToolTip.cpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/line_edit/C_FlaOgeLeFilePath.cpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/push_button/C_FlaOgePubPathVariables.cpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/spin_box/C_FlaOgeSpxDark.cpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/text_browser/C_FlaOgeTebProgressLog.cpp
   ${PROJECT_ROOT}/src/syde_flash/util/C_FlaUti.cpp
   ${PROJECT_ROOT}/src/syde_flash/util/C_FlaUtiStyleSheets.cpp
   ${PROJECT_ROOT}/src/util/C_Uti.cpp
   ${PROJECT_ROOT}/src/util/C_UtiClipBoardHelper.cpp
   ${PROJECT_ROOT}/src/util/C_UtiFindNameHelper.cpp
   ${PROJECT_ROOT}/src/util/C_UtiStyleSheets.cpp
   ${PROJECT_ROOT}/src/util/C_UtiUndoStack.cpp
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
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/check_box/C_CamOgeChxSettings.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box/C_CamOgeGbxMessageGeneratorBackground.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/group_box/C_CamOgeGbxMessageSignalsBorder.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit/C_CamOgeLeIdPopUp.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/line_edit/C_CamOgeLeTableEdit.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/menu/C_CamOgeMuRecentProjects.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariables.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubPathVariablesBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/push_button/C_CamOgePubProjOp.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/spin_box/C_CamOgeSpxProperties.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/splitter/C_CamOgeSpi.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/tool_button/C_CamOgeTobRecentProjects.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingsBase.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSpinBoxGroupTable.hpp
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_settings/C_CamMosSectionPopup.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeContextMenu.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeDarkScrollArea.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeHorizontalListWidget.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeListViewToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeListWidgetToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeOverlay.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeSmoothScrollArea.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTabBar.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTableWidgetComIf.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTransparentScrollArea.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewCheckable.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeViewToolTipBaseCheckable.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeWidgetToolBarSearch.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeWidgetToolBarSearchItemDelegate.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeWidgetToolBarSearchItemWidget.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeWidgetToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxDefaultSmall.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxTristateBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/check_box/C_OgeChxUnified.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxIconDelegate.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxMultiSelect.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxMultiSelectTableHalc.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxResizingView.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTable.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTableBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxTableHalc.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxText.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/combo_box/C_OgeCbxUnified.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/frame/C_OgeFraGeneric.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/frame/C_OgeFraStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxMouseMove.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxNavigationSection.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxNavigationSectionSelected.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxNavigationTab.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/group_box/C_OgeGbxTransparentDashboard.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabAdaptiveSize.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabContextMenuBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabDashboardDefault.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabDashboardLabelCaption.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabDashboardLabelValue.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabDoubleClick.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabElided.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabExternalLink.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabFrameError.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericBubble.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericNoPaddingNoMargins.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabGenericWithContextMenu.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpSubTitle.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabPopUpTitle.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyledContextMenu.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabStyledToolTip.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabSvgOnly.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/label/C_OgeLabUnified.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeComboBox.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeContextMenuBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePath.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeFilePathBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeSearch.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/line_edit/C_OgeLeUnified.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/menu/C_OgeMuSections.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/menu/C_OgeMuTabChartFit.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubAddChartDataElement.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubBackNavigation.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubColor.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubDashboard.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubGenericTextWithBackground.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconChart.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconEvents.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubIconOnly.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubNavigationHover.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubOpen.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubOptions.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubPathVariables.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubPrevNavigation.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubProjAction.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubStyledToolTip.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSvgIconOnly.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSvgIconWithText.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSvgIconWithTextBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubSystemCommissioningEdit.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubTabClose.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubToolBar.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubTreeWidgetRoot.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubUnified.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubUpdate.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubUseCase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/push_button/C_OgePubUseCaseNav.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/radio_button/C_OgeRabToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/scroll_area/C_OgeSaMain.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAllBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxAutoFixBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFix.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleAutoFixCustomTrigger.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleDynamicDecimalsBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleProperties.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxDoubleToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxEditProperties.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxFactor.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxFactorProperties.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxFactorStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxFactorTable.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64AutoFix.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64Factor.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64FactorTable.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64Properties.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64Styled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64Table.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxInt64ToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxNumber.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxTable.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxTableComIf.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxTableDouble.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxTableHalc.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/spin_box/C_OgeSpxToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiHorizontalNavigation.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/splitter/C_OgeSpiStandard.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tab_widget/C_OgeTawAlternative.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tab_widget/C_OgeTawStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tab_widget/C_OgeTawToolBox.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tab_widget/C_OgeTawToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebContextMenuBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_browser/C_OgeTebStyled.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedContextMenuBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedElided.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedPropertiesComment.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedTable.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/text_edit/C_OgeTedToolTipContent.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/tool_button/C_OgeTobToolTipBase.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiBopperle.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiBorder.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiEditBackground.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiFixPosition.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiFontConfig.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiHover.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiNavigationTab.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiOnlyBackground.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiPieChart.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiProgressBar.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiProgressBopperle.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiProgressLog.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroupProperties.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiUtil.hpp
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiWithToolTip.hpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/line_edit/C_FlaOgeLeFilePath.hpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/push_button/C_FlaOgePubPathVariables.hpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/spin_box/C_FlaOgeSpxDark.hpp
   ${PROJECT_ROOT}/src/syde_flash/gui_elements/text_browser/C_FlaOgeTebProgressLog.hpp
   ${PROJECT_ROOT}/src/syde_flash/util/C_FlaUti.hpp
   ${PROJECT_ROOT}/src/syde_flash/util/C_FlaUtiStyleSheets.hpp
   ${PROJECT_ROOT}/src/util/C_Uti.hpp
   ${PROJECT_ROOT}/src/util/C_UtiClipBoardHelper.hpp
   ${PROJECT_ROOT}/src/util/C_UtiFindNameHelper.hpp
   ${PROJECT_ROOT}/src/util/C_UtiStyleSheets.hpp
   ${PROJECT_ROOT}/src/util/C_UtiUndoStack.hpp
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
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSectionHeader.ui
   ${PROJECT_ROOT}/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingSubSection.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgePopUpDialog.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/C_OgeTreeWidgetToolBarSearchItemWidget.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiError.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiFixPosition.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiFontConfig.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiHover.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiNavigationTab.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiPieChart.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiProgressLog.ui
   ${PROJECT_ROOT}/src/opensyde_gui_elements/widget/C_OgeWiSpinBoxGroup.ui
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
install(TARGETS SYDEflash DESTINATION ${PROJECT_ROOT}/result/utilities/SYDEflash)
