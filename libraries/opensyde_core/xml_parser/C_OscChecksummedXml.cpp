//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Parse / write XML file with CRC protection

   see .h file header for details

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers
#include <cstdint>
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscChecksummedXml.hpp"

#include <sstream>
#include <iomanip>
#include <string>
#include "C_SclChecksums.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::scl;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
static const std::string mc_NAME_CRC_ATTRIBUTE = "file_crc";

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

namespace {
/// Helper: integer to hexadecimal string (zero-padded, uppercase, no "0x" prefix)
template <typename T>
std::string mh_IntToHex(const T orc_Value, const uint32_t ou32_Digits)
{
   std::stringstream c_Stream;
   c_Stream << std::hex << std::uppercase << std::setw(ou32_Digits) << std::setfill('0') << orc_Value;
   return c_Stream.str();
}
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor

   Set up class
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscChecksummedXml::C_OscChecksummedXml(void) :
   C_OscXmlParser(),
   mu16_CrcDepth(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Open XML data from file

   Open XML file.
   If the file could not be opened the function will return an error and prepare an empty XML structure.
   After the data was opened the function will check the CRC

   The checksum value is expected in the attribute "c_NAME_CRC_ATTRIBUTE" of the root name.

   \param[in]   orc_FileName   path to XML file to open

   \return
   Errc::success    data was read from file
   Errc::noact      could not load from file; invalid XML
   Errc::rd_wr      file was read but checksum entry not found at defined position
   Errc::checksum   data was read but CRC is not correct
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscChecksummedXml::LoadFromFile(const std::string & orc_FileName)
{
   std::error_code c_Return = C_OscXmlParser::LoadFromFile(orc_FileName);

   if (!c_Return)
   {
       const std::string c_Text = this->SelectRoot();
      if (c_Text == "")
      {
         c_Return = Errc::rd_wr;
      }
      else
      {
         if (this->AttributeExists(mc_NAME_CRC_ATTRIBUTE) == false)
         {
            c_Return = Errc::rd_wr;
         }
         else
         {
            const uint16_t u16_CrcFromFile = static_cast<uint16_t>(this->GetAttributeUint32(mc_NAME_CRC_ATTRIBUTE));
            const uint16_t u16_CrcCalc = this->m_CalcXmlCrc();

            this->SelectRoot(); //be defensive: set defined start state
            c_Return = (u16_CrcCalc == u16_CrcFromFile) ? Errc::success : Errc::checksum;
         }
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Write XML data to file

   Update CRC and write data to file.
   After calculation the checksum value is written to the attribute "c_NAME_CRC_ATTRIBUTE" of the root name.
   A pre-existing file will be replaced.

   Will set the active node to "root".

   \param[in]   orc_FileName   path to XML file to write to

   \return
   Errc::success   data was written to file
   Errc::noact     could not write data from file
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscChecksummedXml::SaveToFile(const std::string & orc_FileName)
{
   std::error_code c_Return = Errc::success;
   const uint16_t u16_CrcCalc = this->m_CalcXmlCrc();

   if (this->SelectRoot() == "")
   {
      c_Return = Errc::noact;
   }
   else
   {
      this->SetAttributeString(mc_NAME_CRC_ATTRIBUTE, "0x" + mh_IntToHex(u16_CrcCalc, 4));

      c_Return = C_OscXmlParser::SaveToFile(orc_FileName);
   }
   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------

void C_OscChecksummedXml::m_CalcXmlCrcNode(uint16_t & oru16_Crc)
{
    std::string c_NodeLv1;
    std::string c_Text;

   c_NodeLv1 = this->SelectNodeChild();

   mu16_CrcDepth++;
   C_SclChecksums::CalcCRC16(&mu16_CrcDepth, 2U, oru16_Crc);
   while (c_NodeLv1 != "")
   {
      std::vector<C_OscXmlAttribute> c_Attributes;
      C_SclChecksums::CalcCRC16(c_NodeLv1.c_str(), c_NodeLv1.length(), oru16_Crc);
      c_Attributes = this->GetAttributes();
      for (uint32_t u32_Index = 0U; u32_Index < c_Attributes.size(); u32_Index++)
      {
         C_SclChecksums::CalcCRC16(c_Attributes[u32_Index].c_Name.c_str(),
                                   c_Attributes[u32_Index].c_Name.length(), oru16_Crc);
         C_SclChecksums::CalcCRC16(c_Attributes[u32_Index].c_Value.c_str(),
                                   c_Attributes[u32_Index].c_Value.length(), oru16_Crc);
      }
      c_Text = this->GetNodeContent();
      C_SclChecksums::CalcCRC16(c_Text.c_str(), c_Text.length(), oru16_Crc);

      //sub-nodes ?
      m_CalcXmlCrcNode(oru16_Crc);

      c_NodeLv1 = this->SelectNodeNext();
      if (c_NodeLv1 == "")
      {
         //go back to parent, we are finished here ...
         this->SelectNodeParent();
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------

uint16_t C_OscChecksummedXml::m_CalcXmlCrc(void)
{
   std::vector<C_OscXmlAttribute> c_Attributes;
    std::string c_Text;
   uint16_t u16_Crc = 0x1D0FU; //set CCITT25 start value

   c_Text = this->SelectRoot();

   C_SclChecksums::CalcCRC16(c_Text.c_str(), c_Text.length(), u16_Crc);
   c_Attributes = this->GetAttributes();
   for (uint32_t u32_Index = 0U; u32_Index < c_Attributes.size(); u32_Index++)
   {
      if (c_Attributes[u32_Index].c_Name != mc_NAME_CRC_ATTRIBUTE) //skip CRC value
      {
         C_SclChecksums::CalcCRC16(c_Attributes[u32_Index].c_Name.c_str(),
                                   c_Attributes[u32_Index].c_Name.length(), u16_Crc);
         C_SclChecksums::CalcCRC16(c_Attributes[u32_Index].c_Value.c_str(),
                                   c_Attributes[u32_Index].c_Value.length(), u16_Crc);
      }
   }

   mu16_CrcDepth = 1U;
   c_Text = this->GetNodeContent();
   C_SclChecksums::CalcCRC16(c_Text.c_str(), c_Text.length(), u16_Crc);
   C_SclChecksums::CalcCRC16(&mu16_CrcDepth, 2U, u16_Crc);

   //subnodes:
   m_CalcXmlCrcNode(u16_Crc);

   return u16_Crc;
}

//----------------------------------------------------------------------------------------------------------------------
