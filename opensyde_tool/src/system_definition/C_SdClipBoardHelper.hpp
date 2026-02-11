//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle clipboard load and save (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDCLIPBOARDHELPER_HPP
#define C_SDCLIPBOARDHELPER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <QList>
#include <QString>
#include <QStringList>
#include <QModelIndexList>
#include "stwtypes.hpp"
#include "C_OscNodeDataPool.hpp"
#include "C_PuiSdNodeDataPool.hpp"
#include "C_OscNodeDataPoolList.hpp"
#include "C_PuiSdNodeDataPoolList.hpp"
#include "C_OscCanMessage.hpp"
#include "C_PuiSdNodeCanSignal.hpp"
#include "C_OscCanMessageIdentificationIndices.hpp"
#include "C_SdTopologyDataSnapshot.hpp"
#include "C_OscHalcConfigStandalone.hpp"
#include "C_UtiClipBoardHelper.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdClipBoardHelper :
   public C_UtiClipBoardHelper
{
public:
   static void h_StoreDataPool(const stw::opensyde_core::C_OscNodeDataPool & orc_OscContent,
                               const stw::opensyde_gui_logic::C_PuiSdNodeDataPool & orc_UiContent);
   static int32_t h_LoadToDataPool(stw::opensyde_core::C_OscNodeDataPool & orc_OscContent,
                                   stw::opensyde_gui_logic::C_PuiSdNodeDataPool & orc_UiContent);
   static void h_StoreDataPoolLists(const QList<stw::opensyde_core::C_OscNodeDataPoolList> & orc_OscContent,
                                    const QList<C_PuiSdNodeDataPoolList> & orc_UiContent,
                                    const stw::opensyde_core::C_OscNodeDataPool::E_Type & ore_Type);
   static int32_t h_LoadToDataPoolLists(QList<stw::opensyde_core::C_OscNodeDataPoolList> & orc_OscContent,
                                        QList<C_PuiSdNodeDataPoolList> & orc_UiContent,
                                        stw::opensyde_core::C_OscNodeDataPool::E_Type & ore_Type);
   static void h_StoreDataPoolListElementsToClipBoard(
      const QList<stw::opensyde_core::C_OscNodeDataPoolListElement> & orc_OscContent,
      const QList<C_PuiSdNodeDataPoolListElement> & orc_UiContent,
      const stw::opensyde_core::C_OscNodeDataPool::E_Type oe_DatapoolType);
   static int32_t h_LoadToDataPoolListElementsFromClipBoard(
      QList<stw::opensyde_core::C_OscNodeDataPoolListElement> & orc_OscContent,
      QList<C_PuiSdNodeDataPoolListElement> & orc_UiContent);
   static void h_StoreDataPoolListElementsToString(
      const QList<stw::opensyde_core::C_OscNodeDataPoolListElement> & orc_OscContent,
      const QList<C_PuiSdNodeDataPoolListElement> & orc_UiContent,
      const stw::opensyde_core::C_OscNodeDataPool::E_Type oe_DatapoolType, QString & orc_Output);
   static int32_t h_LoadToDataPoolListElementsFromString(
      QList<stw::opensyde_core::C_OscNodeDataPoolListElement> & orc_OscContent,
      QList<C_PuiSdNodeDataPoolListElement> & orc_UiContent, const QString & orc_Input);
   static void h_StoreIndicesToString(const QList<uint32_t> & orc_Indices, QString & orc_Output);
   static int32_t h_LoadIndicesFromString(QList<uint32_t> & orc_Indices, const QString & orc_Input);
   static void h_StoreDataPoolListDataSetsToClipBoard(
      const QList<stw::opensyde_core::C_OscNodeDataPoolDataSet> & orc_OscNames,
      const QList<QList<stw::opensyde_core::C_OscNodeDataPoolContent> > & orc_OscDataSetValues);
   static int32_t h_LoadToDataPoolListDataSetsFromClipBoard(
      QList<stw::opensyde_core::C_OscNodeDataPoolDataSet> & orc_OscNames,
      QList<QList<stw::opensyde_core::C_OscNodeDataPoolContent> > & orc_OscDataSetValues);
   static void h_StoreDataPoolListDataSetsToString(
      const QList<stw::opensyde_core::C_OscNodeDataPoolDataSet> & orc_OscNames,
      const QList<QList<stw::opensyde_core::C_OscNodeDataPoolContent> > & orc_OscDataSetValues,
      QString & orc_Output);
   static int32_t h_LoadToDataPoolListDataSetsFromString(
      QList<stw::opensyde_core::C_OscNodeDataPoolDataSet> & orc_OscNames,
      QList<QList<stw::opensyde_core::C_OscNodeDataPoolContent> > & orc_OscDataSetValues,
      const QString & orc_Input);
   static void h_StoreMessages(const QList<stw::opensyde_core::C_OscCanMessage> & orc_Messages,
                               const QList<QList<stw::opensyde_core::
                                                  C_OscNodeDataPoolListElement> > & orc_OscSignalCommons,
                               const QList<QList<C_PuiSdNodeDataPoolListElement> > & orc_UiSignalCommons,
                               const QList<C_PuiSdNodeCanMessage> & orc_UiMessages,
                               const QList<QStringList> & orc_OwnerNodeName,
                               const QList<QList<uint32_t> > & orc_OwnerNodeInterfaceIndex,
                               const QList<QList<uint32_t> > & orc_OwnerNodeDatapoolIndex,
                               const QList<QList<bool> > & orc_OwnerIsTxFlag,
                               const opensyde_core::C_OscCanProtocol::E_Type oe_ProtocolType);
   static int32_t h_LoadMessages(QList<stw::opensyde_core::C_OscCanMessage> & orc_Messages,
                                 QList<QList<stw::opensyde_core::
                                                    C_OscNodeDataPoolListElement> > & orc_OscSignalCommons,
                                 QList<QList<
                                           C_PuiSdNodeDataPoolListElement> > & orc_UiSignalCommons,
                                 QList<C_PuiSdNodeCanMessage> & orc_UiMessages,
                                 QList<QStringList> & orc_OwnerNodeName,
                                 QList<QList<uint32_t> > & orc_OwnerNodeInterfaceIndex,
                                 QList<QList<uint32_t> > & orc_OwnerNodeDatapoolIndex,
                                 QList<QList<bool> > & orc_OwnerIsTxFlag);
   static void h_StoreSignalsToClipboard(const QList<stw::opensyde_core::C_OscCanSignal> & orc_Signals,
                                         const QList<stw::opensyde_core::C_OscNodeDataPoolListElement> & orc_OscSignalCommons, const QList<C_PuiSdNodeDataPoolListElement> & orc_UiSignalCommons, const QList<C_PuiSdNodeCanSignal> & orc_UiSignals, const opensyde_core::C_OscCanProtocol::E_Type oe_ProtocolType);
   static int32_t h_LoadSignalsFromClipboard(QList<stw::opensyde_core::C_OscCanSignal> & orc_Signals,
                                             QList<stw::opensyde_core::C_OscNodeDataPoolListElement> & orc_OscSignalCommons, QList<C_PuiSdNodeDataPoolListElement> & orc_UiSignalCommons, QList<C_PuiSdNodeCanSignal> & orc_UiSignals);
   static void h_StoreMessageIndexToString(
      const QList<stw::opensyde_core::C_OscCanMessageIdentificationIndices> & orc_MessageIds,
      QString & orc_Output);
   static int32_t h_LoadMessageIndexFromString(const QString & orc_Input,
                                               QList<stw::opensyde_core::C_OscCanMessageIdentificationIndices> & orc_MessageIds);
   static void h_StoreDataSnapShotToClipboard(const C_SdTopologyDataSnapshot & orc_Data);
   static int32_t h_LoadDataSnapShotFromClipboard(C_SdTopologyDataSnapshot & orc_Data);

   static void h_StoreHalcItemConfigToClipboard(const stw::opensyde_core::C_OscHalcConfigStandalone & orc_Data);
   static int32_t h_LoadHalcItemConfigFromClipboard(stw::opensyde_core::C_OscHalcConfigStandalone & orc_Data);

private:
   C_SdClipBoardHelper(void);

   static void mh_StoreSignalsToString(const QList<stw::opensyde_core::C_OscCanSignal> & orc_Signals,
                                       const QList<stw::opensyde_core::C_OscNodeDataPoolListElement> & orc_OscSignalCommons, const QList<C_PuiSdNodeDataPoolListElement> & orc_UiSignalCommons, const QList<C_PuiSdNodeCanSignal> & orc_UiSignals, const opensyde_core::C_OscCanProtocol::E_Type oe_ProtocolType, QString & orc_Output);
   static int32_t mh_LoadSignalsFromString(const QString & orc_Input,
                                           QList<stw::opensyde_core::C_OscCanSignal> & orc_Signals,
                                           QList<stw::opensyde_core::C_OscNodeDataPoolListElement> & orc_OscSignalCommons, QList<C_PuiSdNodeDataPoolListElement> & orc_UiSignalCommons, QList<C_PuiSdNodeCanSignal> & orc_UiSignals);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
