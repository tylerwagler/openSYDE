//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Parameter set file reader/writer base (Multi-Format Implementation)

   Base class providing utility methods for parameter set filers using the
   Qt-native serialization framework.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscParamSetFilerBase_New.hpp"
#include "C_OscParamSetFilerBase.hpp"
#include "C_OscLoggingHandler.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------------
 */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */
uint16_t C_OscParamSetFilerBase_New::mhu16_FileVersion = 1;

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * -------------------------------------------------------------------------------------------------------
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
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscParamSetFilerBase_New::C_OscParamSetFilerBase_New(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add CRC to file

   \param[in]      orc_Path              File path

   \return
   C_NO_ERR   CRC added
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetFilerBase_New::h_AddCrc(const QString &orc_Path) {
   // This is a legacy method - delegate to original implementation
   return C_OscParamSetFilerBase::h_AddCrc(orc_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check file version

   \param[in,out]  orc_Device            Device to read from

   \return
   C_NO_ERR   valid version
   C_CONFIG   invalid version
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetFilerBase_New::h_CheckFileVersion(QIODevice &orc_Device) {
   QDataStream in(&orc_Device);
   in.setVersion(QDataStream::Qt_6_0);
   
   uint16_t u16_Version;
   in >> u16_Version;
   
   if (in.status() == QDataStream::Ok && u16_Version == mhu16_FileVersion) {
      return C_NO_ERR;
   } else {
      return C_CONFIG;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save file version

   \param[in,out]  orc_Device            Device to write to
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetFilerBase_New::h_SaveFileVersion(QIODevice &orc_Device) {
   QDataStream out(&orc_Device);
   out.setVersion(QDataStream::Qt_6_0);
   out << mhu16_FileVersion;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save file info

   \param[in,out]  orc_Device            Device to write to
   \param[in]      orc_FileInfo          File info to save
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetFilerBase_New::h_SaveFileInfo(QIODevice &orc_Device,
                                                 const C_OscParamSetInterpretedFileInfoData &orc_FileInfo) {
   QDataStream out(&orc_Device);
   out.setVersion(QDataStream::Qt_6_0);
   out << static_cast<qint32>(orc_FileInfo.c_FileInfo.size());
   for (const auto &c_Info : orc_FileInfo.c_FileInfo) {
      out << c_Info;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load file info

   \param[in,out]  orc_Device            Device to read from
   \param[out]     orc_FileInfo          File info to load
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetFilerBase_New::h_LoadFileInfo(QIODevice &orc_Device,
                                                    C_OscParamSetInterpretedFileInfoData &orc_FileInfo,
                                                    bool &orq_MissingOptionalContent) {
   Q_UNUSED(orq_MissingOptionalContent);
   
   QDataStream in(&orc_Device);
   in.setVersion(QDataStream::Qt_6_0);
   
   qint32 s_Size;
   in >> s_Size;
   
   orc_FileInfo.c_FileInfo.clear();
   for (qint32 s_I = 0; s_I < s_Size; ++s_I) {
      QString c_Info;
      in >> c_Info;
      orc_FileInfo.c_FileInfo.append(c_Info);
   }
   
   return (in.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load data pool infos

   \param[out]     orc_DataPoolInfos     Data pool infos to load
   \param[in,out]  orc_Device            Device to read from
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetFilerBase_New::h_LoadDataPoolInfos(QList<C_OscParamSetDataPoolInfo> &orc_DataPoolInfos,
                                                        QIODevice &orc_Device,
                                                        bool &orq_MissingOptionalContent) {
   Q_UNUSED(orq_MissingOptionalContent);
   
   QDataStream in(&orc_Device);
   in.setVersion(QDataStream::Qt_6_0);
   
   qint32 s_Size;
   in >> s_Size;
   
   orc_DataPoolInfos.clear();
   for (qint32 s_I = 0; s_I < s_Size; ++s_I) {
      C_OscParamSetDataPoolInfo c_Info;
      c_Info.FromQDataStream(in);
      orc_DataPoolInfos.append(c_Info);
   }
   
   return (in.status() == QDataStream::Ok) ? C_NO_ERR : C_CONFIG;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data pool infos

   \param[in]      orc_DataPoolInfos     Data pool infos to save
   \param[in,out]  orc_Device            Device to write to
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetFilerBase_New::h_SaveDataPoolInfos(const QList<C_OscParamSetDataPoolInfo> &orc_DataPoolInfos,
                                                      QIODevice &orc_Device) {
   QDataStream out(&orc_Device);
   out.setVersion(QDataStream::Qt_6_0);
   
   out << static_cast<qint32>(orc_DataPoolInfos.size());
   for (const auto &c_Info : orc_DataPoolInfos) {
      c_Info.ToQDataStream(out);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load data pool info

   \param[out]     orc_DataPoolInfo      Data pool info to load
   \param[in,out]  orc_Device            Device to read from
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
   C_CONFIG   invalid format
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetFilerBase_New::h_LoadDataPoolInfo(C_OscParamSetDataPoolInfo &orc_DataPoolInfo,
                                                        QIODevice &orc_Device,
                                                        bool &orq_MissingOptionalContent) {
   Q_UNUSED(orq_MissingOptionalContent);
   
   orc_DataPoolInfo.FromQDataStream(QDataStream(&orc_Device));
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save data pool info

   \param[in]      orc_DataPoolInfo      Data pool info to save
   \param[in,out]  orc_Device            Device to write to
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetFilerBase_New::h_SaveDataPoolInfo(const C_OscParamSetDataPoolInfo &orc_DataPoolInfo,
                                                     QIODevice &orc_Device) {
   const_cast<C_OscParamSetDataPoolInfo &>(orc_DataPoolInfo).ToQDataStream(QDataStream(&orc_Device));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Load node name (deprecated)

   \param[out]     orc_Name              Node name
   \param[in]      orc_XmlParser         XML parser

   \return
   C_NO_ERR   name loaded
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetFilerBase_New::mh_LoadNodeName(QString &orc_Name, C_OscXmlParserBase &orc_XmlParser) {
   return C_OscParamSetFilerBase::mh_LoadNodeName(orc_Name, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Save node name (deprecated)

   \param[in]      orc_Name              Node name
   \param[in]      orc_XmlParser         XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetFilerBase_New::mh_SaveNodeName(const QString &orc_Name, C_OscXmlParserBase &orc_XmlParser) {
   C_OscParamSetFilerBase::mh_SaveNodeName(orc_Name, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Load data pool infos (deprecated)

   \param[out]     orc_DataPoolInfos     Data pool infos
   \param[in]      orc_XmlParser         XML parser
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetFilerBase_New::mh_LoadDataPoolInfos(QList<C_OscParamSetDataPoolInfo> &orc_DataPoolInfos,
                                                          C_OscXmlParserBase &orc_XmlParser,
                                                          bool &orq_MissingOptionalContent) {
   return C_OscParamSetFilerBase::mh_LoadDataPoolInfos(orc_DataPoolInfos, orc_XmlParser, orq_MissingOptionalContent);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Save data pool infos (deprecated)

   \param[in]      orc_DataPoolInfos     Data pool infos
   \param[in]      orc_XmlParser         XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetFilerBase_New::mh_SaveDataPoolInfos(const QList<C_OscParamSetDataPoolInfo> &orc_DataPoolInfos,
                                                       C_OscXmlParserBase &orc_XmlParser) {
   C_OscParamSetFilerBase::mh_SaveDataPoolInfos(orc_DataPoolInfos, orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Load data pool info (deprecated)

   \param[out]     orc_DataPoolInfo      Data pool info
   \param[in]      orc_XmlParser         XML parser
   \param[out]     orq_MissingOptionalContent  Flag for missing optional content

   \return
   C_NO_ERR   data loaded
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetFilerBase_New::mh_LoadDataPoolInfo(C_OscParamSetDataPoolInfo &orc_DataPoolInfo,
                                                         C_OscXmlParserBase &orc_XmlParser,
                                                         bool &orq_MissingOptionalContent) {
   return C_OscParamSetFilerBase::mh_LoadDataPoolInfo(orc_DataPoolInfo, orc_XmlParser, orq_MissingOptionalContent);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Legacy compatibility - Save data pool info (deprecated)

   \param[in]      orc_DataPoolInfo      Data pool info
   \param[in]      orc_XmlParser         XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetFilerBase_New::mh_SaveDataPoolInfo(const C_OscParamSetDataPoolInfo &orc_DataPoolInfo,
                                                      C_OscXmlParserBase &orc_XmlParser) {
   C_OscParamSetFilerBase::mh_SaveDataPoolInfo(orc_DataPoolInfo, orc_XmlParser);
}
