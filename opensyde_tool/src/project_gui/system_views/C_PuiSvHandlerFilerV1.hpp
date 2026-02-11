//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System view filer (V1 format) (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_PUISVHANDLERFILERV1_HPP
#define C_PUISVHANDLERFILERV1_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QList>
#include <vector>
#include "stwtypes.hpp"
#include "C_OscNode.hpp"
#include "C_PuiSvData.hpp"
#include "C_OscXmlParser.hpp"
#include "C_PuiSvDbWidgetBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_PuiSvHandlerFilerV1
{
public:
   C_PuiSvHandlerFilerV1(void);

   static int32_t h_LoadViews(QList<C_PuiSvData> & orc_Views,
                              stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveViews(const QList<C_PuiSvData> & orc_Views,
                           stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadDashboard(C_PuiSvDashboard & orc_Dashboard,
                                  stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                  const bool oq_IgnoreMostErrorCases = false);
   static void h_SaveDashboard(const C_PuiSvDashboard & orc_Dashboard,
                               stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadReadRails(QMap<stw::opensyde_core::C_OscNodeDataPoolListElementId,
                                       C_PuiSvReadDataConfiguration> & orc_Rails,
                                  stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveReadRails(const QMap<stw::opensyde_core::C_OscNodeDataPoolListElementId,
                                          C_PuiSvReadDataConfiguration> & orc_Rails,
                               stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);

private:
   static int32_t mh_LoadNodeActiveFlags(QByteArray & orc_NodeActiveFlags,
                                         stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadNodeUpdateInformation(
      QList<stw::opensyde_core::C_OscViewNodeUpdate> & orc_NodeUpdateInformation,
      stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadOneNodeUpdateInformation(stw::opensyde_core::C_OscViewNodeUpdate & orc_NodeUpdateInformation,
                                                  stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                                  const stw::opensyde_core::C_OscNode & orc_Node);
   static int32_t mh_LoadDashboards(QList<C_PuiSvDashboard> & orc_Dashboards,
                                    stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadView(C_PuiSvData & orc_View, stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadPc(opensyde_core::C_OscViewPc & orc_OscPc, C_PuiSvPc & orc_PuiPc,
                            stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadCharts(QList<C_PuiSvDbChart> & orc_Widgets,
                                stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadLabels(QList<C_PuiSvDbLabel> & orc_Widgets,
                                stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadParams(QList<C_PuiSvDbParam> & orc_Widgets,
                                stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_LoadParamExpandedItems(QList<C_PuiSvDbExpandedTreeIndex> & orc_Items,
                                         stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_LoadParamColumnPositionIndices(QList<int32_t> & orc_Items,
                                                 stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadPieCharts(QList<C_PuiSvDbPieChart> & orc_Widgets,
                                   stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadSpinBoxes(QList<C_PuiSvDbSpinBox> & orc_Widgets,
                                   stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadTables(QList<C_PuiSvDbTable> & orc_Widgets,
                                stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadSliders(QList<C_PuiSvDbSlider> & orc_Widgets,
                                 stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadProgressBars(QList<C_PuiSvDbProgressBar> & orc_Widgets,
                                      stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadToggles(QList<C_PuiSvDbToggle> & orc_Widgets,
                                 stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadWidgetBase(C_PuiSvDbWidgetBase & orc_Widget,
                                    stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadWriteWidgetBase(C_PuiSvDbWriteWidgetBase & orc_WriteWidget,
                                         stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_LoadUiIndex(C_PuiSvDbNodeDataPoolListElementId & orc_Id,
                              stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadParamDataSetIndices(QList<int32_t> & orc_Values,
                                             stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadParamValues(QList<stw::opensyde_core::C_OscNodeDataPoolContent> & orc_Values,
                                     stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadParamTables(QList<QList<int32_t> > & orc_Values,
                                     stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadParamColumns(QList<int32_t> & orc_Values,
                                      stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_LoadDataElement(stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                                  const stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_StringToWriteMode(const QString & orc_String, C_PuiSvDbWriteWidgetBase::E_WriteMode & ore_Mode);
   static int32_t mh_StringToDisplayStyle(const QString & orc_String, C_PuiSvDbWidgetBase::E_Style & ore_Style);
   static int32_t mh_StringToToggleType(const QString & orc_String, C_PuiSvDbToggle::E_Type & ore_Type);
   static int32_t mh_StringToSpinBoxType(const QString & orc_String, C_PuiSvDbSpinBox::E_Type & ore_Type);
   static int32_t mh_StringToLabelType(const QString & orc_String, C_PuiSvDbLabel::E_Type & ore_Type);
   static int32_t mh_StringToSliderType(const QString & orc_String, C_PuiSvDbSlider::E_Type & ore_Type);
   static int32_t mh_StringToProgressBarType(const QString & orc_String, C_PuiSvDbProgressBar::E_Type & ore_Type);
   static int32_t mh_StringToProgressBarAlignmentType(const QString & orc_String,
                                                      C_PuiSvDbProgressBar::E_Alignment & ore_Alignment);
   static int32_t mh_StringToTransmissionMode(const QString & orc_String,
                                              C_PuiSvReadDataConfiguration::E_TransmissionMode & ore_Mode);
   static int32_t mh_StringToSourceType(const QString & orc_String,
                                        C_PuiSvDbNodeDataPoolListElementId::E_Type & ore_Type);
   static void mh_SaveNodeActiveFlags(const QByteArray & orc_NodeActiveFlags,
                                      stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveNodeUpdateInformation(
      const QList<stw::opensyde_core::C_OscViewNodeUpdate> & orc_NodeUpdateInformation,
      stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveDashboards(const QList<C_PuiSvDashboard> & orc_Dashboards,
                                 stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveView(const C_PuiSvData & orc_View, stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SavePc(const stw::opensyde_core::C_OscViewPc & orc_OscPc, const C_PuiSvPc & orc_PuiPc,
                         stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveCharts(const QList<C_PuiSvDbChart> & orc_Widgets,
                             stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveLabels(const QList<C_PuiSvDbLabel> & orc_Widgets,
                             stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveParams(const QList<C_PuiSvDbParam> & orc_Widgets,
                             stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveParamExpandedItems(const QList<C_PuiSvDbExpandedTreeIndex> & orc_Items,
                                         stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveParamColumnPositionIndices(const QList<int32_t> & orc_Items,
                                                 stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SavePieCharts(const QList<C_PuiSvDbPieChart> & orc_Widgets,
                                stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveSpinBoxes(const QList<C_PuiSvDbSpinBox> & orc_Widgets,
                                stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveTables(const QList<C_PuiSvDbTable> & orc_Widgets,
                             stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveSliders(const QList<C_PuiSvDbSlider> & orc_Widgets,
                              stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveProgressBars(const QList<C_PuiSvDbProgressBar> & orc_Widgets,
                                   stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveToggles(const QList<C_PuiSvDbToggle> & orc_Widgets,
                              stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveWidgetBase(const C_PuiSvDbWidgetBase & orc_Widget,
                                 stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveWriteWidgetBase(const C_PuiSvDbWriteWidgetBase & orc_WriteWidget,
                                      stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveUiIndex(const C_PuiSvDbNodeDataPoolListElementId & orc_Id,
                              stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveDataElement(const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                                  stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static QString mh_DeviceConfigModeToString(const C_PuiSvData::E_DeviceConfigurationMode oe_Mode);
   static QString mh_WriteModeToString(const C_PuiSvDbWriteWidgetBase::E_WriteMode oe_Mode);
   static QString mh_DisplayStyleToString(const C_PuiSvDbWidgetBase::E_Style oe_Style);
   static QString mh_ToggleTypeToString(const C_PuiSvDbToggle::E_Type oe_Type);
   static QString mh_SpinBoxTypeToString(const C_PuiSvDbSpinBox::E_Type oe_Type);
   static QString mh_LabelTypeToString(const C_PuiSvDbLabel::E_Type oe_Type);
   static QString mh_SliderTypeToString(const C_PuiSvDbSlider::E_Type oe_Type);
   static QString mh_ProgressBarTypeToString(const C_PuiSvDbProgressBar::E_Type oe_Type);
   static QString mh_ProgressBarAlignmentTypeToString(const C_PuiSvDbProgressBar::E_Alignment oe_Alignment);
   static QString mh_TransmissionModeToString(const C_PuiSvReadDataConfiguration::E_TransmissionMode oe_Mode);
   static QString mh_SourceTypeToString(const C_PuiSvDbNodeDataPoolListElementId::E_Type oe_Type);
   static C_PuiSvData::E_DeviceConfigurationMode mh_StringToDeviceConfigMode(const QString & orc_Input);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
