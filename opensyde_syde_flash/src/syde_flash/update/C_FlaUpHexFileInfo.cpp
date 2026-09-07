//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for Data Block file details (implementation)

   Widget for Data Block file details that displays HEX file information.

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>
#include <QFileInfo>
#include <QDateTime>
#include <QCryptographicHash>

#include "precomp_headers.hpp"
#include "stwtypes.hpp"
#include "C_FlaUpHexFileInfo.hpp"
#include "C_OscHexFile.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
 *
 *  \param[in,out]  orc_File    Hex file to load
*/
//----------------------------------------------------------------------------------------------------------------------
C_FlaUpHexFileInfo::C_FlaUpHexFileInfo(const QString & orc_File) :
   s32_CurrentHexFileIndex(0)
{
   SetHexFileInfo(orc_File);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set hex file information

   \param[in]     orc_File    HEX file path
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaUpHexFileInfo::SetHexFileInfo(const QString & orc_File)
{
   const QFileInfo c_FileInfo(orc_File);
   C_OscHexFile c_HexFile;

   std::vector<stw::opensyde_core::C_OscApplicationInfoBlock> c_InfoBlocks;

   c_BlockInfo.resize(0);
   c_HexFileInfo.s32_NumberOfBlocks = 0;
   c_HexFileInfo.c_FileName = c_FileInfo.fileName();

   const std::error_code c_Result = c_HexFile.LoadFromFile(orc_File.toStdString().c_str());
   c_HexFile.GetApplicationInformationBlocks(c_InfoBlocks, 0UL, false, false, false);

   if (!c_Result)
   {
      uint32_t u32_Crc;
      c_HexFileInfo.c_TimeStamp = c_FileInfo.lastModified().toString("dd.MM.yyyy HH:mm:ss");
      c_HexFileInfo.c_Size = static_cast<QString>("%1 bytes").arg(c_FileInfo.size());
      c_HexFileInfo.c_Checksum = mh_GetMd5Hex(orc_File);
      c_HexFileInfo.c_NumberOfBytes = QString::number(c_HexFile.ByteCount());
      //the checksum is the out-parameter; the return value only says whether it could be calculated
      if (!c_HexFile.CalcFileChecksum(u32_Crc))
      {
         c_HexFileInfo.c_BitCrc = static_cast<QString>("0x%1").arg(u32_Crc, 0, 16);
      }
      else
      {
         c_HexFileInfo.c_BitCrc = "";
      }
      if (c_InfoBlocks.size() > 0)
      {
         c_HexFileInfo.s32_NumberOfBlocks = c_InfoBlocks.size();
      }
      else
      {
         c_HexFileInfo.s32_NumberOfBlocks = 0;
      }
      if (c_InfoBlocks.size() > 0)
      {
         c_BlockInfo.resize(c_InfoBlocks.size());
         for (size_t u32_ItAppl = 0UL; u32_ItAppl < c_InfoBlocks.size(); ++u32_ItAppl)
         {
            const stw::opensyde_core::C_OscApplicationInfoBlock & rc_CurInfo = c_InfoBlocks[u32_ItAppl];
            c_BlockInfo.at(u32_ItAppl).c_BlockType = rc_CurInfo.GetInfoLevelAsString().c_str();
            c_BlockInfo.at(u32_ItAppl).c_Version = QString::number(rc_CurInfo.u8_StructVersion);
            if (rc_CurInfo.ContainsDeviceID() == true)
            {
               c_BlockInfo.at(u32_ItAppl).c_DeviceId = rc_CurInfo.GetDeviceID().c_str();
            }
            else
            {
               c_BlockInfo.at(u32_ItAppl).c_DeviceId = "Unknown";
            }
            if (rc_CurInfo.ContainsDateAndTime() == true)
            {
               c_BlockInfo.at(u32_ItAppl).c_Date = rc_CurInfo.GetDate().c_str();
            }
            else
            {
               c_BlockInfo.at(u32_ItAppl).c_Date = "Unknown";
            }
            if (rc_CurInfo.ContainsDateAndTime() == true)
            {
               c_BlockInfo.at(u32_ItAppl).c_Time = rc_CurInfo.GetTime().c_str();
            }
            else
            {
               c_BlockInfo.at(u32_ItAppl).c_Time = "Unknown";
            }
            if (rc_CurInfo.ContainsProjectName() == true)
            {
               c_BlockInfo.at(u32_ItAppl).c_ProjectName = rc_CurInfo.GetProjectName().c_str();
            }
            else
            {
               c_BlockInfo.at(u32_ItAppl).c_ProjectName = "Unknown";
            }
            if (rc_CurInfo.ContainsProjectVersion() == true)
            {
               c_BlockInfo.at(u32_ItAppl).c_ProjectVersion = rc_CurInfo.GetProjectVersion().c_str();
            }
            else
            {
               c_BlockInfo.at(u32_ItAppl).c_ProjectVersion = "Unknown";
            }
            if (rc_CurInfo.ContainsAdditionalInfo() == true)
            {
               c_BlockInfo.at(u32_ItAppl).c_AdditionalInfo = rc_CurInfo.GetAdditionalInfo().c_str();
            }
            else
            {
               c_BlockInfo.at(u32_ItAppl).c_AdditionalInfo = "Unknown";
            }
         }
      }
   }
   else
   {
      c_HexFileInfo.c_TimeStamp = "No information available";
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get MD5 checksum for file encoded in hex

   \param[in] orc_Path File path

   \return
   MD5 checksum for file encoded in hex
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_FlaUpHexFileInfo::mh_GetMd5Hex(const QString & orc_Path)
{
   QString c_Retval;
   QFile c_File(orc_Path);

   if (c_File.open(QFile::ReadOnly))
   {
      QCryptographicHash c_Md5Algorithm(QCryptographicHash::Md5);
      if (c_Md5Algorithm.addData(&c_File))
      {
         c_Retval = c_Md5Algorithm.result().toHex();
      }
      c_File.close();
   }
   return c_Retval;
}
