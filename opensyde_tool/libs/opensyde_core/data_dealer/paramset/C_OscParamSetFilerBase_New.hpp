//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Parameter set file reader/writer base (Multi-Format)

   Base class providing utility methods for parameter set filers using the
   Qt-native serialization framework.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETFILERBASE_NEW_HPP
#define C_OSCPARAMSETFILERBASE_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscParamSetDataPoolInfo.hpp"
#include "C_OscParamSetInterpretedFileInfoData.hpp"
#include "stwtypes.hpp"
#include <QList>
#include <QString>

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

class C_OscParamSetFilerBase_New {
public:
   // --------------------------------------------------------------------------
   // File Operations
   // --------------------------------------------------------------------------
   static int32_t h_AddCrc(const QString &orc_Path);
   static int32_t h_CheckFileVersion(QIODevice &orc_Device);
   static void h_SaveFileVersion(QIODevice &orc_Device);
   static void h_SaveFileInfo(QIODevice &orc_Device,
                              const C_OscParamSetInterpretedFileInfoData &orc_FileInfo);
   static int32_t h_LoadFileInfo(QIODevice &orc_Device,
                                 C_OscParamSetInterpretedFileInfoData &orc_FileInfo,
                                 bool &orq_MissingOptionalContent);

   // --------------------------------------------------------------------------
   // Data Pool Info Operations
   // --------------------------------------------------------------------------
   static int32_t h_LoadDataPoolInfos(QList<C_OscParamSetDataPoolInfo> &orc_DataPoolInfos,
                                      QIODevice &orc_Device,
                                      bool &orq_MissingOptionalContent);
   static void h_SaveDataPoolInfos(const QList<C_OscParamSetDataPoolInfo> &orc_DataPoolInfos,
                                   QIODevice &orc_Device);
   static int32_t h_LoadDataPoolInfo(C_OscParamSetDataPoolInfo &orc_DataPoolInfo,
                                     QIODevice &orc_Device,
                                     bool &orq_MissingOptionalContent);
   static void h_SaveDataPoolInfo(const C_OscParamSetDataPoolInfo &orc_DataPoolInfo,
                                  QIODevice &orc_Device);

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated)
   // --------------------------------------------------------------------------
   [[deprecated("Use Qt-native serialization methods instead")]]
   static int32_t mh_LoadNodeName(QString &orc_Name, C_OscXmlParserBase &orc_XmlParser);
   [[deprecated("Use Qt-native serialization methods instead")]]
   static void mh_SaveNodeName(const QString &orc_Name, C_OscXmlParserBase &orc_XmlParser);
   [[deprecated("Use Qt-native serialization methods instead")]]
   static int32_t mh_LoadDataPoolInfos(QList<C_OscParamSetDataPoolInfo> &orc_DataPoolInfos,
                                       C_OscXmlParserBase &orc_XmlParser,
                                       bool &orq_MissingOptionalContent);
   [[deprecated("Use Qt-native serialization methods instead")]]
   static void mh_SaveDataPoolInfos(const QList<C_OscParamSetDataPoolInfo> &orc_DataPoolInfos,
                                    C_OscXmlParserBase &orc_XmlParser);
   [[deprecated("Use Qt-native serialization methods instead")]]
   static int32_t mh_LoadDataPoolInfo(C_OscParamSetDataPoolInfo &orc_DataPoolInfo,
                                      C_OscXmlParserBase &orc_XmlParser,
                                      bool &orq_MissingOptionalContent);
   [[deprecated("Use Qt-native serialization methods instead")]]
   static void mh_SaveDataPoolInfo(const C_OscParamSetDataPoolInfo &orc_DataPoolInfo,
                                   C_OscXmlParserBase &orc_XmlParser);

protected:
   C_OscParamSetFilerBase_New(void);

private:
   static uint16_t mhu16_FileVersion;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
