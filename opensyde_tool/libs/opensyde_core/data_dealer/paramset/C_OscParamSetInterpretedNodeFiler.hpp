//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for interpreted parameter set node files (Multi-Format)

   Load / save interpreted parameter set node data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETINTERPRETEDNODEFILER_NEW_HPP
#define C_OSCPARAMSETINTERPRETEDNODEFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscFilerUtil.hpp"
#include "C_OscXmlParser.hpp"
#include "C_OscParamSetInterpretedNode.hpp"
#include "stwtypes.hpp"
#include <QList>

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

class C_OscParamSetInterpretedNodeFiler_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadInterpretedNodeFile(C_OscParamSetInterpretedNode &orc_Node,
                                            const QString &orc_FilePath,
                                            bool &orq_MissingOptionalContent);
   static int32_t h_SaveInterpretedNodeFile(const C_OscParamSetInterpretedNode &orc_Node,
                                            const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscParamSetInterpretedNode &orc_Node,
                               const QString &orc_FilePath,
                               bool &orq_MissingOptionalContent);
   static int32_t h_SaveBinary(const C_OscParamSetInterpretedNode &orc_Node,
                               const QString &orc_FilePath);
   static int32_t h_LoadFromMemoryBinary(C_OscParamSetInterpretedNode &orc_Node,
                                         const QByteArray &orc_Data,
                                         bool &orq_MissingOptionalContent);
   QByteArray h_SaveToMemoryBinary(const C_OscParamSetInterpretedNode &orc_Node) const;

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscParamSetInterpretedNode &orc_Node,
                             const QString &orc_FilePath,
                             bool &orq_MissingOptionalContent);
   static int32_t h_SaveJson(const C_OscParamSetInterpretedNode &orc_Node,
                             const QString &orc_FilePath);
   static int32_t h_LoadFromMemoryJson(C_OscParamSetInterpretedNode &orc_Node,
                                       const QJsonObject &orc_Object,
                                       bool &orq_MissingOptionalContent);
   QJsonObject h_SaveToMemoryJson(const C_OscParamSetInterpretedNode &orc_Node) const;

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscParamSetInterpretedNode &orc_Node,
                            const QString &orc_FilePath,
                            bool &orq_MissingOptionalContent);
   static int32_t h_SaveXml(const C_OscParamSetInterpretedNode &orc_Node,
                            const QString &orc_FilePath);
   static int32_t h_LoadFromMemoryXml(C_OscParamSetInterpretedNode &orc_Node,
                                      const QDomElement &orc_Element,
                                      bool &orq_MissingOptionalContent);
   QDomElement h_SaveToMemoryXml(const C_OscParamSetInterpretedNode &orc_Node,
                                 QDomDocument &ro_Doc) const;

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated - for backward compatibility only)
   // --------------------------------------------------------------------------
   [[deprecated("Use h_LoadInterpretedNodeFile/h_SaveInterpretedNodeFile with format detection, or format-specific methods")]]
   static int32_t h_LoadInterpretedNode(C_OscParamSetInterpretedNode &orc_Node,
                                        C_OscXmlParserBase &orc_XmlParser,
                                        bool &orq_MissingOptionalContent);
   [[deprecated("Use h_LoadInterpretedNodeFile/h_SaveInterpretedNodeFile with format detection, or format-specific methods")]]
   static void h_SaveInterpretedNode(const C_OscParamSetInterpretedNode &orc_Node,
                                     C_OscXmlParserBase &orc_XmlParser);

private:
   // Helper for format detection
   static int32_t mh_DetectAndLoad(C_OscParamSetInterpretedNode &orc_Node,
                                   const QString &orc_FilePath,
                                   bool &orq_MissingOptionalContent);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
