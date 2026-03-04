//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       File handler for parameter set raw node data

   File handler for parameter set raw node data with both legacy XML and Qt-native
   serialization support (binary, JSON, XML).

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include <cstdio>

#include "C_OscLoggingHandler.hpp"
#include "C_OscParamSetRawNodeFiler.hpp"
#include "stwerrors.hpp"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTextStream>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load parameter set node from legacy XML

   Load parameter set node data from XML file
   pre-condition: the passed XML parser has the active node set to "node"
   post-condition: the passed XML parser has the active node set to the same "node"

   \param[out]    orc_Node                   data storage
   \param[in,out] orc_XmlParser              XML with specified node active
   \param[in,out] orq_MissingOptionalContent Flag for indication of optional content missing

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_LoadRawNode(C_OscParamSetRawNode &orc_Node,
                                                  C_OscXmlParserBase &orc_XmlParser,
                                                  bool &orq_MissingOptionalContent) {
   int32_t s32_Retval = C_OscParamSetFilerBase::mh_LoadNodeName(orc_Node.c_Name, orc_XmlParser);

   if (s32_Retval == C_NO_ERR) {
      s32_Retval = C_OscParamSetFilerBase::mh_LoadDataPoolInfos(
          orc_Node.c_DataPools, orc_XmlParser, orq_MissingOptionalContent);
      if (s32_Retval == C_NO_ERR) {
         s32_Retval = C_OscParamSetRawNodeFiler::mh_LoadEntries(orc_Node.c_Entries, orc_XmlParser);
      }
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save parameter set node to legacy XML

   Save parameter set node to XML file
   pre-condition: the passed XML parser has the active node set to "node"
   post-condition: the passed XML parser has the active node set to the same "node"

   \param[in]     orc_Node      data storage
   \param[in,out] orc_XmlParser XML with specified node active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetRawNodeFiler::h_SaveRawNode(const C_OscParamSetRawNode &orc_Node,
                                               C_OscXmlParserBase &orc_XmlParser) {
   C_OscParamSetFilerBase::mh_SaveNodeName(orc_Node.c_Name, orc_XmlParser);
   C_OscParamSetFilerBase::mh_SaveDataPoolInfos(orc_Node.c_DataPools, orc_XmlParser);
   C_OscParamSetRawNodeFiler::mh_SaveEntries(orc_Node.c_Entries, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscParamSetRawNodeFiler::C_OscParamSetRawNodeFiler(void)
    : C_OscParamSetFilerBase() {
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load parameter set entries from legacy XML

   \param[out]    orc_Entries   data storage
   \param[in,out] orc_XmlParser XML with specified node active

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::mh_LoadEntries(QList<C_OscParamSetRawEntry> &orc_Entries,
                                                    C_OscXmlParserBase &orc_XmlParser) {
   int32_t s32_Retval = C_NO_ERR;

   orc_Entries.clear();
   if (orc_XmlParser.SelectNodeChild("raw") == "raw") {
      QString c_SelectedNode = orc_XmlParser.SelectNodeChild("raw-entry");

      if (c_SelectedNode == "raw-entry") {
         do {
            C_OscParamSetRawEntry c_Item;
            s32_Retval = C_OscParamSetRawNodeFiler::mh_LoadEntry(c_Item, orc_XmlParser);
            if (s32_Retval == C_NO_ERR) {
               orc_Entries.push_back(c_Item);
            }
            c_SelectedNode = orc_XmlParser.SelectNodeNext("raw-entry");
         } while ((c_SelectedNode == "raw-entry") && (s32_Retval == C_NO_ERR));
         // Return
         Q_ASSERT(orc_XmlParser.SelectNodeParent() == "raw");
      } else {
         osc_write_log_error("Loading Dataset data",
                             "Could not find \"node\".\"raw\".\"raw-entry\" node.");
         s32_Retval = C_CONFIG;
      }
      // Return
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "node");
   } else {
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save parameter set entries to legacy XML

   \param[in]     orc_Entries   data storage
   \param[in,out] orc_XmlParser XML with specified node active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetRawNodeFiler::mh_SaveEntries(const QList<C_OscParamSetRawEntry> &orc_Entries,
                                                 C_OscXmlParserBase &orc_XmlParser) {
   // Clean start
   if (orc_XmlParser.SelectNodeChild("raw") == "raw") {
      Q_ASSERT(orc_XmlParser.DeleteNode() == "raw");
      Q_ASSERT(orc_XmlParser.SelectRoot() == "opensyde-parameter-sets");
   }
   Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("raw") == "raw");
   for (uint32_t u32_Index = 0U; u32_Index < orc_Entries.size(); u32_Index++) {
      Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("raw-entry") == "raw-entry");
      C_OscParamSetRawNodeFiler::mh_SaveEntry(orc_Entries[u32_Index], orc_XmlParser);
      // Return
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "raw");
   }
   // Return
   Q_ASSERT(orc_XmlParser.SelectNodeParent() == "node");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load parameter set entry from legacy XML

   \param[out]    orc_Entry     data storage
   \param[in,out] orc_XmlParser XML with specified node active

   \return
   C_NO_ERR   data read
   C_CONFIG   content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::mh_LoadEntry(C_OscParamSetRawEntry &orc_Entry,
                                                  C_OscXmlParserBase &orc_XmlParser) {
   int32_t s32_Retval = C_NO_ERR;

   if (orc_XmlParser.SelectNodeChild("address") == "address") {
      try {
         orc_Entry.u32_StartAddress =
             static_cast<uint32_t>(orc_XmlParser.GetNodeContent().toLongLong());
      } catch (...) {
         osc_write_log_error("Loading Dataset data",
                             "Node \"node\".\"raw\".\"raw-entry\".\"address\" "
                             "contains non-integer value (" +
                                 orc_XmlParser.GetNodeContent() + ").");
         s32_Retval = C_CONFIG;
      }
      // Return
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "raw-entry");
   } else {
      osc_write_log_error("Loading Dataset data",
                          "Could not find \"node\".\"raw\".\"raw-entry\".\"address\" node.");
      s32_Retval = C_CONFIG;
   }
   if (s32_Retval == C_NO_ERR) {
      if (orc_XmlParser.SelectNodeChild("value") == "value") {
         const QString c_Content = orc_XmlParser.GetNodeContent();
         QStringList c_Tokens = c_Content.split(";");
         orc_Entry.c_Bytes.reserve(c_Tokens.size());
         for (int32_t s32_It = 0;
              (s32_It < c_Tokens.size()) && (s32_Retval == C_NO_ERR); ++s32_It) {
            const QString &rc_Token = c_Tokens[s32_It];
            try {
               orc_Entry.c_Bytes.push_back(static_cast<uint8_t>(rc_Token.toInt()));
            } catch (...) {
               osc_write_log_error("Loading Dataset data",
                                   "Node \"node\".\"raw\".\"raw-entry\".\"value\" "
                                   "contains non-integer value (" +
                                       rc_Token + ").");
               s32_Retval = C_CONFIG;
            }
         }
         // Return
         Q_ASSERT(orc_XmlParser.SelectNodeParent() == "raw-entry");
      } else {
         osc_write_log_error("Loading Dataset data",
                             "Could not find \"node\".\"raw\".\"raw-entry\".\"value\" node.");
         s32_Retval = C_CONFIG;
      }
   }
   if (s32_Retval == C_NO_ERR) {
      if (orc_XmlParser.SelectNodeChild("size") == "size") {
         uint32_t u32_Size = 0;
         try {
            u32_Size =
                static_cast<uint32_t>(orc_XmlParser.GetNodeContent().toLongLong());
         } catch (...) {
            s32_Retval = C_CONFIG;
         }
         if (s32_Retval == C_NO_ERR) {
            if (u32_Size != orc_Entry.c_Bytes.size()) {
               s32_Retval = C_CONFIG;
            }
         }
         // Return
         Q_ASSERT(orc_XmlParser.SelectNodeParent() == "raw-entry");
      } else {
         osc_write_log_error("Loading Dataset data",
                             "Could not find \"node\".\"raw\".\"raw-entry\".\"size\" node.");
         s32_Retval = C_CONFIG;
      }
   }

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save parameter set entry to legacy XML

   \param[in]     orc_Entry     data storage
   \param[in,out] orc_XmlParser XML with specified node active
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetRawNodeFiler::mh_SaveEntry(const C_OscParamSetRawEntry &orc_Entry,
                                              C_OscXmlParserBase &orc_XmlParser) {
   QString c_Bytes;

   orc_XmlParser.CreateNodeChild("address", QString::number(orc_Entry.u32_StartAddress));
   orc_XmlParser.CreateNodeChild("size", QString::number(orc_Entry.c_Bytes.size()));
   if (orc_Entry.c_Bytes.size() > 0) {
      c_Bytes = QString::number(orc_Entry.c_Bytes[0]);
      for (uint32_t u32_It = 1; u32_It < orc_Entry.c_Bytes.size(); ++u32_It) {
         c_Bytes += ';';
         c_Bytes += QString::number(orc_Entry.c_Bytes[u32_It]);
      }
   } else {
      c_Bytes = "";
   }
   orc_XmlParser.CreateNodeChild("value", c_Bytes);
}

//----------------------------------------------------------------------------------------------------------------------
// New Qt-native multi-format methods
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load parameter set raw node from file (auto-detect format)

   \param[in] c_FilePath Path to the file to load
   \param[out] rc_Node    Loaded node data

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_LoadFile(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node) {
   QString c_LowerPath = c_FilePath.toLower();

   if (c_LowerPath.endsWith(".bin")) {
      return h_LoadBinary(c_FilePath, rc_Node);
   } else if (c_LowerPath.endsWith(".json")) {
      return h_LoadJson(c_FilePath, rc_Node);
   } else if (c_LowerPath.endsWith(".xml")) {
      return h_LoadXml(c_FilePath, rc_Node);
   } else {
      return h_LoadBinary(c_FilePath, rc_Node);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save parameter set raw node to file (auto-detect format)

   \param[in] c_FilePath Path to the file to save
   \param[in] rc_Node    Node data to save

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_SaveFile(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node) {
   QString c_LowerPath = c_FilePath.toLower();

   if (c_LowerPath.endsWith(".bin")) {
      return h_SaveBinary(c_FilePath, rc_Node);
   } else if (c_LowerPath.endsWith(".json")) {
      return h_SaveJson(c_FilePath, rc_Node);
   } else if (c_LowerPath.endsWith(".xml")) {
      return h_SaveXml(c_FilePath, rc_Node);
   } else {
      return h_SaveBinary(c_FilePath, rc_Node);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load parameter set raw node from binary file

   \param[in] c_FilePath Path to the binary file to load
   \param[out] rc_Node    Loaded node data

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_LoadBinary(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node) {
   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }

   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);

   int32_t i32_Result = rc_Node.FromQDataStream(c_Stream);

   c_File.close();

   return i32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save parameter set raw node to binary file

   \param[in] c_FilePath Path to the binary file to save
   \param[in] rc_Node    Node data to save

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_SaveBinary(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node) {
   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }

   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);

   int32_t i32_Result = rc_Node.ToQDataStream(c_Stream);

   c_File.close();

   return i32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load parameter set raw node from JSON file

   \param[in] c_FilePath Path to the JSON file to load
   \param[out] rc_Node    Loaded node data

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_LoadJson(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node) {
   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return C_RD_WR;
   }

   QTextStream c_Stream(&c_File);
   QString c_Content = c_Stream.readAll();
   c_File.close();

   QJsonParseError c_ParseError;
   QJsonDocument c_Doc = QJsonDocument::fromJson(c_Content.toUtf8(), &c_ParseError);

   if (c_ParseError.error != QJsonParseError::NoError) {
      return C_CONFIG;
   }

   if (!c_Doc.isObject()) {
      return C_CONFIG;
   }

   return rc_Node.FromJsonObject(c_Doc.object());
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save parameter set raw node to JSON file

   \param[in] c_FilePath Path to the JSON file to save
   \param[in] rc_Node    Node data to save

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_SaveJson(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node) {
   QJsonObject c_Object = rc_Node.ToJsonObject();
   QJsonDocument c_Doc(c_Object);

   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
      return C_RD_WR;
   }

   QTextStream c_Stream(&c_File);
   c_Stream << c_Doc.toJson(QJsonDocument::Compact);
   c_File.close();

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load parameter set raw node from XML file

   \param[in] c_FilePath Path to the XML file to load
   \param[out] rc_Node    Loaded node data

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_LoadXml(const QString &c_FilePath, C_OscParamSetRawNode &rc_Node) {
   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }

   QDomDocument c_Doc;
   QString c_ErrorMsg;
   int32_t i32_LineNumber;

   if (!c_Doc.setContent(&c_File, &c_ErrorMsg, &i32_LineNumber)) {
      c_File.close();
      return C_CONFIG;
   }

   c_File.close();

   return rc_Node.FromQDomElement(c_Doc.documentElement());
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save parameter set raw node to XML file

   \param[in] c_FilePath Path to the XML file to save
   \param[in] rc_Node    Node data to save

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler::h_SaveXml(const QString &c_FilePath, const C_OscParamSetRawNode &rc_Node) {
   QDomDocument c_Doc("param-set-raw-node");

   QDomElement c_RootElement = rc_Node.ToQDomDocument(c_Doc, "param-set-raw-node");
   c_Doc.appendChild(c_RootElement);

   QFile c_File(c_FilePath);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }

   QTextStream c_Stream(&c_File);
   c_Stream << c_Doc.toString();
   c_File.close();

   return C_NO_ERR;
}
