//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI topology filer (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_PUISDHANDLERFILER_HPP
#define C_PUISDHANDLERFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QDir>
#include <QColor>
#include <QList>
#include <QFont>
#include "C_OscXmlParser.hpp"
#include "C_PuiSdNode.hpp"
#include "C_PuiSdBus.hpp"
#include "C_PuiBsTextElement.hpp"
#include "C_PuiSdTextElementBus.hpp"
#include "C_PuiBsBoundary.hpp"
#include "C_PuiBsImage.hpp"
#include "C_PuiBsElements.hpp"
#include "C_PuiBsLineArrow.hpp"
#include "C_PuiSdSharedDatapools.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_PuiSdLastKnownHalElementId.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_PuiSdHandlerFiler
{
public:
   C_PuiSdHandlerFiler(void);

   //Data pool
   static int32_t h_LoadDataPools(QList<C_PuiSdNodeDataPool> & orc_DataPools,
                                  stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                  const QDir * const opc_BasePath,
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
   static int32_t h_SaveDataPools(const QList<C_PuiSdNodeDataPool> & orc_UiDataPools,
                                  const QList<stw::opensyde_core::C_OscNodeDataPool> * const opc_OscDataPools,
                                  const QDir * const opc_BasePath,
                                  stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_SaveDataPoolFile(const C_PuiSdNodeDataPool & orc_DataPool, const QString & orc_FilePath);
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

   // Shared DatapoolConfiguration
   static int32_t h_LoadSharedDatapoolsFile(const QString & orc_FilePath, C_PuiSdSharedDatapools & orc_SharedDatapools);
   static void h_LoadSharedDatapoolsGroups(C_PuiSdSharedDatapools & orc_SharedDatapools,
                                           stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_LoadSharedDatapoolsGroup(QList<stw::opensyde_core::C_OscNodeDataPoolId> & orc_Group,
                                          stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_SaveSharedDatapoolsFile(const QString & orc_FilePath,
                                            const C_PuiSdSharedDatapools & orc_SharedDatapools);
   static void h_SaveSharedDatapoolsGroups(const C_PuiSdSharedDatapools & orc_SharedDatapools,
                                           stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);

   //Com
   static int32_t h_LoadCanProtocols(QList<C_PuiSdNodeCanProtocol> & orc_CanProtocols,
                                     stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                     const QDir * const opc_BasePath);
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
   static int32_t h_SaveCanProtocols(const QList<C_PuiSdNodeCanProtocol> & orc_UiCanProtocols,
                                     const QList<stw::opensyde_core::C_OscCanProtocol> * const opc_OscCanProtocols, const QList<stw::opensyde_core::C_OscNodeDataPool> * const opc_OscDatapools, const QDir * const opc_BasePath, stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_SaveCanProtocolFile(const C_PuiSdNodeCanProtocol & orc_CanProtocol, const QString & orc_FilePath);
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
                              stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser, const QDir * const opc_BasePath,
                              QList<stw::opensyde_core::C_OscNode> * const opc_OscNodes);
   static int32_t h_LoadNodeFile(C_PuiSdNode & orc_Node, const QString & orc_FilePath, const QDir * const opc_BasePath,
                                 stw::opensyde_core::C_OscNode * const opc_OscNode);
   static void h_SaveNodes(const QList<C_PuiSdNode> & orc_Nodes,
                           stw::opensyde_core::C_OscXmlParser & orc_XmlParser);
   static int32_t h_LoadBuses(QList<C_PuiSdBus> & orc_Buses,
                              stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveBuses(const QList<C_PuiSdBus> & orc_Buses,
                           stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadBusTextElements(QList<C_PuiSdTextElementBus> & orc_BusTextElements,
                                        stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveBusTextElements(const QList<C_PuiSdTextElementBus> & orc_BusTextElements,
                                     stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static int32_t h_LoadLastKnownHalcCrcs(std::map<stw::opensyde_core::C_OscNodeDataPoolListElementOptArrayId,
                                                   C_PuiSdLastKnownHalElementId> & orc_Crcs,
                                          stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveLastKnownHalcCrcs(const std::map<stw::opensyde_core::C_OscNodeDataPoolListElementOptArrayId,
                                                      C_PuiSdLastKnownHalElementId> & orc_Crcs,
                                       stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);

   //Complete
   static int32_t h_SaveSystemDefinitionUiFile(const QString & orc_FilePath,
                                               const stw::opensyde_core::C_OscSystemDefinition & orc_OscSystemDefinition, const QList<C_PuiSdNode> & orc_UiNodes, const QList<C_PuiSdBus> & orc_UiBuses, const QList<C_PuiSdTextElementBus> & orc_BusTextElements, const stw::opensyde_gui_logic::C_PuiBsElements & orc_Elements, const std::map<stw::opensyde_core::C_OscNodeDataPoolListElementOptArrayId,
                                                                                                                                                                                                                                                                                                                                                                   C_PuiSdLastKnownHalElementId> & orc_LastKnownHalcCrcs);
   static int32_t h_LoadSystemDefinitionUiFile(const QString & orc_FilePath, QList<C_PuiSdNode> & orc_UiNodes,
                                               QList<C_PuiSdBus> & orc_UiBuses,
                                               QList<C_PuiSdTextElementBus> & orc_BusTextElements,
                                               stw::opensyde_gui_logic::C_PuiBsElements & orc_Elements,
                                               std::map<stw::opensyde_core::C_OscNodeDataPoolListElementOptArrayId,
                                                        C_PuiSdLastKnownHalElementId> & orc_LastKnownHalcCrcs,
                                               QList<stw::opensyde_core::C_OscNode> * const opc_OscNodes);

   //File names
   static QString h_GetNodeUiFileName(void);
   static QString h_GetCommUiFileName(const QString & orc_DatapoolName);
   static QString h_GetDatapoolUiFileName(const QString & orc_DatapoolName);
   static QString h_GetSystemDefinitionUiFilePath(const QString & orc_SystemDefinitionCoreFilePath);
   static QString h_GetSharedDatapoolUiFilePath(const QString & orc_SystemDefinitionCoreFilePath);

private:
   static int32_t mh_LoadDatapoolFile(C_PuiSdNodeDataPool & orc_DataPool, const QString & orc_FilePath,
                                      stw::opensyde_core::C_OscNodeDataPool * const opc_OscDataPool);
   static int32_t mh_LoadCommFile(C_PuiSdNodeCanProtocol & orc_UiCanProtocol, const QString & orc_FilePath);
   static int32_t mh_LoadNode(C_PuiSdNode & orc_Node, stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                              const QDir * const opc_BasePath, stw::opensyde_core::C_OscNode * const opc_OscNode);
   static int32_t mh_SaveNodeFile(const C_PuiSdNode & orc_UiNode, const stw::opensyde_core::C_OscNode & orc_OscNode,
                                  const QString & orc_FilePath, const QDir * const opc_BasePath);
   static int32_t mh_SaveNode(const C_PuiSdNode & orc_UiNode, const stw::opensyde_core::C_OscNode * const opc_OscNode,
                              const QDir * const opc_BasePath, stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser);
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
