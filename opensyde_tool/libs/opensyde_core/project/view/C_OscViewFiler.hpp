//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       View filer (core parts)
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCVIEWFILER_HPP
#define C_OSCVIEWFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QList>
#include "C_OscXmlParser.hpp"
#include "C_OscNode.hpp"
#include "C_OscViewData.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscViewFiler
{
public:
   C_OscViewFiler(void);

   // Load
   static int32_t h_LoadSystemViewsFile(QList<C_OscViewData> & orc_Views,
                                        const QString & orc_PathSystemViews,
                                        const QList<C_OscNode> & orc_OscNodes);
   static int32_t h_LoadViewsOsc(QList<C_OscViewData> & orc_Views, const QList<C_OscNode> & orc_OscNodes,
                                 C_OscXmlParserBase & orc_XmlParser, const QString & orc_BasePath);
   static int32_t h_LoadViewOsc(C_OscViewData & orc_View, stw::opensyde_core::C_OscXmlParserBase & orc_XmlParser,
                                const QList<stw::opensyde_core::C_OscNode> & orc_OscNodes);

   // Save
   static void h_SaveNodeActiveFlags(const QByteArray & orc_NodeActiveFlags,
                                     C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveNodeUpdateInformation(const QList<C_OscViewNodeUpdate> & orc_NodeUpdateInformation,
                                           C_OscXmlParserBase & orc_XmlParser);
   static void h_SavePc(const opensyde_core::C_OscViewPc & orc_OscPc, C_OscXmlParserBase & orc_XmlParser);

   // Utilities
   static int32_t h_StringToPemFileStateDebugger(const QString & orc_String,
                                                 C_OscViewNodeUpdate::E_StateDebugger & ore_State);
   static int32_t h_StringToPemFileStateSecurity(const QString & orc_String,
                                                 C_OscViewNodeUpdate::E_StateSecurity & ore_State);
   static QString h_PemFileStateDebuggerToString(const C_OscViewNodeUpdate::E_StateDebugger oe_State);
   static QString h_PemFileStateSecurityToString(const C_OscViewNodeUpdate::E_StateSecurity oe_State);

private:
   // Load
   static int32_t mh_LoadNodeActiveFlags(QByteArray & orc_NodeActiveFlags,
                                         C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadNodeUpdateInformation(QList<C_OscViewNodeUpdate> & orc_NodeUpdateInformation,
                                               C_OscXmlParserBase & orc_XmlParser,
                                               const QList<C_OscNode> & orc_OscNodes);
    static void mh_LoadPc(opensyde_core::C_OscViewPc & orc_OscPc, const C_OscXmlParserBase & orc_XmlParser);
    static void mh_LoadNodeUpdateInformationPaths(QStringList & orc_Paths,
                                                  const QString & orc_XmlTagBaseName,
                                                  C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadOneNodeUpdateInformation(C_OscViewNodeUpdate & orc_NodeUpdateInformation,
                                                  C_OscXmlParserBase & orc_XmlParser, const C_OscNode & orc_Node);
   static void mh_LoadNodeUpdateInformationParam(QList<C_OscViewNodeUpdateParamInfo> & orc_Info,
                                                 C_OscXmlParserBase & orc_XmlParser);
   static void mh_LoadNodeUpdateInformationSkipUpdateOfFiles(QList<bool> & orc_Flags,
                                                             C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadNodeUpdateInformationPem(C_OscViewNodeUpdate & orc_NodeUpdateInformation,
                                                  C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadNodeUpdateInformationPemStates(C_OscViewNodeUpdate & orc_NodeUpdateInformation,
                                                        C_OscXmlParserBase & orc_XmlParser);
   static int32_t mh_LoadViewFileOsc(C_OscViewData & orc_View, const QString & orc_FilePath,
                                     const QList<stw::opensyde_core::C_OscNode> & orc_OscNodes);

    // Save
    static void mh_SaveNodeUpdateInformationPaths(const QStringList & orc_Paths,
                                                  const QString & orc_XmlTagBaseName,
                                                  C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveNodeUpdateInformationParamInfo(const QList<C_OscViewNodeUpdateParamInfo> & orc_Info,
                                                     C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveNodeUpdateInformationSkipUpdateOfFiles(const QList<bool> & orc_Flags,
                                                             C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveNodeUpdateInformationPem(const C_OscViewNodeUpdate & orc_NodeUpdateInformation,
                                               C_OscXmlParserBase & orc_XmlParser);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
