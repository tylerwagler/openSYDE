//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       File handler for parameter set raw node data

   File handler for parameter set raw node data with both legacy XML and Qt-native
   serialization support (binary, JSON, XML).

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETRAWNODEFILER_HPP
#define C_OSCPARAMSETRAWNODEFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscParamSetFilerBase.hpp"
#include "C_OscParamSetRawNode.hpp"
#include "stwtypes.hpp"

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/// File handler for parameter set raw node data with legacy and Qt-native serialization
class C_OscParamSetRawNodeFiler : public C_OscParamSetFilerBase {
public:
   // Legacy XML-based methods (used by C_OscParamSetHandler)
   static int32_t h_LoadRawNode(C_OscParamSetRawNode &orc_Node,
                                C_OscXmlParserBase &orc_XmlParser,
                                bool &orq_MissingOptionalContent);
   static void h_SaveRawNode(const C_OscParamSetRawNode &orc_Node,
                             C_OscXmlParserBase &orc_XmlParser);

   // New Qt-native multi-format methods
   static int32_t h_LoadFile(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node);
   static int32_t h_SaveFile(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node);
   static int32_t h_LoadBinary(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node);
   static int32_t h_SaveBinary(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node);
   static int32_t h_LoadJson(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node);
   static int32_t h_SaveJson(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node);
   static int32_t h_LoadXml(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node);
   static int32_t h_SaveXml(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node);

private:
   C_OscParamSetRawNodeFiler(void);

   static int32_t mh_LoadEntries(QList<C_OscParamSetRawEntry> &orc_Entries,
                                 C_OscXmlParserBase &orc_XmlParser);
   static void mh_SaveEntries(const QList<C_OscParamSetRawEntry> &orc_Entries,
                              C_OscXmlParserBase &orc_XmlParser);
   static int32_t mh_LoadEntry(C_OscParamSetRawEntry &orc_Entry,
                               C_OscXmlParserBase &orc_XmlParser);
   static void mh_SaveEntry(const C_OscParamSetRawEntry &orc_Entry,
                            C_OscXmlParserBase &orc_XmlParser);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
