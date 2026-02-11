//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI topology filer (V2 format) (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_PUISDHANDLERFILERV2_HPP
#define C_PUISDHANDLERFILERV2_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QColor>
#include <QFont>
#include <QList>
#include "C_OscXmlParser.hpp"
#include "C_PuiSdNode.hpp"
#include "C_PuiSdBus.hpp"
#include "C_PuiBsTextElement.hpp"
#include "C_PuiSdTextElementBus.hpp"
#include "C_PuiBsBoundary.hpp"
#include "C_PuiBsImage.hpp"
#include "C_PuiBsLineArrow.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_PuiSdHandlerFilerV2
{
public:
   C_PuiSdHandlerFilerV2(void);

   //Data pool
   static int32_t h_LoadDataPools(QList<C_PuiSdNodeDataPool> & orc_DataPools,
                                  stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                  QList<stw::opensyde_core::C_OscNodeDataPool> * const opc_OscDataPools);
   static int32_t h_LoadDataPool(C_PuiSdNodeDataPool & orc_DataPool,
                                 stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                 stw::opensyde_core::C_OscNodeDataPool * const opc_OscDataPool);
   static int32_t h_LoadDataPoolLists(QList<C_PuiSdNodeDataPoolList> & orc_DataPoolLists,
                                      stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                      QList<stw::opensyde_core::C_OscNodeDataPoolList> * const opc_OscLists);
   static int32_t h_LoadDataPoolList(C_PuiSdNodeDataPoolList & orc_DataPoolList,
                                     stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                     stw::opensyde_core::C_OscNodeDataPoolList * const opc_OscList);
   static int32_t h_LoadDataPoolListElements(QList<C_PuiSdNodeDataPoolListElement> & orc_DataPoolListElements,
                                             stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                             QList<stw::opensyde_core::C_OscNodeDataPoolListElement> * const opc_OscElements);
   static void h_LoadDataPoolListElement(C_PuiSdNodeDataPoolListElement & orc_DataPoolListElement,
                                         const stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                         stw::opensyde_core::C_OscNodeDataPoolListElement * const opc_OscElement);
   static void h_SaveDataPools(const QList<C_PuiSdNodeDataPool> & orc_DataPools,
                               stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveDataPool(const C_PuiSdNodeDataPool & orc_DataPool,
                              stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveDataPoolLists(const QList<C_PuiSdNodeDataPoolList> & orc_DataPoolLists,
                                   stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveDataPoolList(const C_PuiSdNodeDataPoolList & orc_DataPoolList,
                                  stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveDataPoolListElements(const QList<C_PuiSdNodeDataPoolListElement> & orc_DataPoolListElements,
                                          stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveDataPoolListElement(const C_PuiSdNodeDataPoolListElement & orc_DataPoolListElement,
                                         stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);

   //Com
   static int32_t h_LoadCanProtocols(QList<C_PuiSdNodeCanProtocol> & orc_CanProtocols,
                                     stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadCanProtocol(C_PuiSdNodeCanProtocol & orc_CanProtocol,
                                    stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadCanMessageContainers(QList<C_PuiSdNodeCanMessageContainer> & orc_CanMessageContainers,
                                             stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadCanMessageContainer(C_PuiSdNodeCanMessageContainer & orc_CanMessageContainer,
                                            stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadCanMessages(QList<C_PuiSdNodeCanMessage> & orc_CanMessages,
                                    stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadCanMessage(C_PuiSdNodeCanMessage & orc_CanMessage,
                                   stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_LoadCanSignals(QList<C_PuiSdNodeCanSignal> & orc_CanSignals,
                                stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_LoadCanSignal(C_PuiSdNodeCanSignal & orc_CanSignal,
                               const stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveCanProtocols(const QList<C_PuiSdNodeCanProtocol> & orc_CanProtocols,
                                  stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveCanProtocol(const C_PuiSdNodeCanProtocol & orc_CanProtocol,
                                 stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveCanMessageContainers(const QList<C_PuiSdNodeCanMessageContainer> & orc_CanMessageContainers,
                                          stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveCanMessageContainer(const C_PuiSdNodeCanMessageContainer & orc_CanMessageContainer,
                                         stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveCanMessages(const QList<C_PuiSdNodeCanMessage> & orc_CanMessages,
                                 stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveCanMessage(const C_PuiSdNodeCanMessage & orc_CanMessage,
                                stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveCanSignals(const QList<C_PuiSdNodeCanSignal> & orc_CanSignals,
                                stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveCanSignal(const C_PuiSdNodeCanSignal & orc_CanSignal,
                               stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);

   // Else
   static int32_t h_LoadNodes(QList<C_PuiSdNode> & orc_Nodes,
                              stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                              QList<stw::opensyde_core::C_OscNode> * const opc_OscNodes);
   static void h_SaveNodes(const QList<C_PuiSdNode> & orc_Nodes,
                           stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadBuses(QList<C_PuiSdBus> & orc_Buses,
                              stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveBuses(const QList<C_PuiSdBus> & orc_Buses,
                           stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadBusTextElements(QList<C_PuiSdTextElementBus> & orc_BusTextElements,
                                        stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveBusTextElements(const QList<C_PuiSdTextElementBus> & orc_BusTextElements,
                                     stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);

private:
   static int32_t mh_LoadNode(C_PuiSdNode & orc_Node, stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                              stw::opensyde_core::C_OscNode * const opc_OscNode);
   static void mh_SaveNode(const C_PuiSdNode & orc_Node, stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadBus(C_PuiSdBus & orc_Bus, stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveBus(const C_PuiSdBus & orc_Bus, stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadTextElement(C_PuiBsTextElement * const opc_TextElement,
                                     stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveTextElement(const C_PuiBsTextElement * const opc_TextElement,
                                  stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   template <typename T>
   static T * mh_GetArrayElemIfAvailable(QList<T> * const opc_Vector, const uint32_t ou32_Index);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
