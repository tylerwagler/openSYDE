//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       CANopen EDS / DCF file handler

   Provides functions to read information from CANopen EDS / DCF file.

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers
#include <QFileInfo>

#include <sstream>   //for std::istringstream
#include <algorithm> //for std::sort
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_SclChecksums.hpp"
#include "C_OscCanOpenObjectDictionary.hpp"
#include <QString>
#include <QFile>
#include <QTextStream>
/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_TextAndSize
{
public:
   QString c_Text;
   uint8_t u8_Size;

   bool q_IsInteger;
   bool q_IsUnsigned;
   bool q_IsFloat;
   bool q_IsString;
};

//----------------------------------------------------------------------------------------------------------------------



/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
static const uint8_t mu8_NUM_DATA_TYPES = 0x24U;

static const C_TextAndSize mac_TextAndSizeTable[mu8_NUM_DATA_TYPES] =
{
   //name            size   sint,  uint,  float, string
   { "",                0,  false, false, false, false },            // 0
   { "BOOLEAN",         1,  false, true,  false, false },            // 1
   { "INTEGER8",        1,  true,  false, false, false },            // 2
   { "INTEGER16",       2,  true,  false, false, false },            // 3
   { "INTEGER32",       4,  true,  false, false, false },            // 4
   { "UNSIGNED8",       1,  false, true,  false, false },            // 5
   { "UNSIGNED16",      2,  false, true,  false, false },            // 6
   { "UNSIGNED32",      4,  false, true,  false, false },            // 7
   { "REAL32",          4,  false, false, true,  false },            // 8
   { "VISIBLE_STRING",  0,  false, false, false, true  },            // 9
   { "OCTET_STRING",    0,  false, false, false, false },            // a
   { "UNICODE_STRING",  0,  false, false, false, true  },            // b
   { "TIME_OF_DAY",     6,  false, false, false, false },            // c
   { "TIME_DIFFERENCE", 6,  false, false, false, false },            // d
   { "reserved",        0,  false, false, false, false },            // e
   { "DOMAIN",          0,  false, false, false, false },            // f
   { "INTEGER24",       3,  true,  false, false, false },            //10
   { "REAL64",          8,  false, false, true,  false },            //11
   { "INTEGER40",       5,  true,  false, false, false },            //12
   { "INTEGER48",       6,  true,  false, false, false },            //13
   { "INTEGER56",       6,  true,  false, false, false },            //14
   { "INTEGER64",       7,  true,  false, false, false },            //15
   { "UNSIGNED24",      3,  false, true,  false, false },            //16
   { "reserved",        0,  false, false, false, false },            //17
   { "UNSIGNED40",      5,  false, true,  false, false },            //18
   { "UNSIGNED48",      6,  false, true,  false, false },            //19
   { "UNSIGNED56",      7,  false, true,  false, false },            //1A
   { "UNSIGNED64",      8,  false, true,  false, false },            //1B
   { "reserved",        0,  false, false, false, false },            //1C
   { "reserved",        0,  false, false, false, false },            //1D
   { "reserved",        0,  false, false, false, false },            //1E
   { "reserved",        0,             false, false, false, false }, //1F
   { "PDO_COMMUNICATION_PARAMETER", 0, false, false, false, false }, //20
   { "PDO_MAPPING",     0,             false, false, false, false }, //21
   { "SDO_PARAMETER",   0,             false, false, false, false }, //22
   { "IDENTITY",        0,             false, false, false, false }  //23
};

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   compare two object dictionary entries

   Compare two object dictionary entries. The index is the sorting criterion.

   \param[in]     orc_Object          other object

   \return
   true   -> index of true is <  index of orc_Object
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectData::operator <(const C_OscCanOpenObjectData & orc_Object) const
{
   bool q_IsLess = false;

   if (u16_Index != orc_Object.u16_Index)
   {
      q_IsLess = (u16_Index < orc_Object.u16_Index);
   }
   else
   {
      //mu8_NumSubs != 0xFF: main index -> first in sorting !
      if (u8_NumSubs != 0xFF)
      {
         q_IsLess = true;
      }
      else
      {
         //same index -> check sub-index !
         q_IsLess = (u8_SubIndex < orc_Object.u8_SubIndex);
      }
   }
   return q_IsLess;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscCanOpenObjectDictionary::C_OscCanOpenObjectDictionary() :
   mu32_OriginalFileHash(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: remember hash over (const) file content

   Calculate and remember hash value over file content of original EDS file.
   This will not change. So there's no need to recalculate at run-time.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenObjectDictionary::m_RememberFileHash()
{
   this->mu32_OriginalFileHash = 0xFFFFFFFFU;

   for (int32_t s32_Line = 0; s32_Line < this->c_TextFileContent.size(); s32_Line++)
   {
      stw::scl::C_SclChecksums::CalcCRC32(this->c_TextFileContent[s32_Line].toUtf8().constData(),
                                          this->c_TextFileContent[s32_Line].length(),
                                          this->mu32_OriginalFileHash);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load data from CANopen EDS file

   Strategy:
   * load all sections from file
   * go through all sections and extract all object and sub-object descriptions into "c_Objects"

   Load CANopen EDS file and store contents in c_Objects.
   Objects in c_Objects will be sorted by index and sub-index.

   \param[in]     orc_File        File path

   \return
   C_NO_ERR    file loaded, content stored in c_Objects
   C_RANGE     file does not exist
   C_CONFIG    could not parse file (is it a valid EDS file ?)
                use GetLastErrorText() to get details
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::LoadFromFile(const QString & orc_File)
{
   int32_t s32_Return = C_NO_ERR;

   mu32_OriginalFileHash = 0U;
   mc_LastError = "";
   c_OdObjects.clear();

   if (!(QFileInfo(orc_File).exists() && QFileInfo(orc_File).isFile()))
   {
      s32_Return = C_RANGE;
   }
   else
   {
      QSettings c_IniFile(orc_File, QSettings::IniFormat);

      //go through all sections and set up c_Objects
      QStringList c_Groups = c_IniFile.childGroups();

      for (int32_t s32_Section = 0U; s32_Section < c_Groups.size(); s32_Section++)
      {
         //We are only interested in the sections describing objects or objects with subobjects.
         //All other sections will be ignored.
         const QString c_SectionName = c_Groups[s32_Section];

         if (c_SectionName.length() == 4)
         {
            //Pattern: [<4 hex digits>sub<1 or 2 hex digits>, e.g. [12AB]
            try
            {
               const uint16_t u16_Index = static_cast<uint16_t>(("0x" + c_SectionName.mid(0, 4)).toInt(nullptr, 16));
               //create new map entry or use existing depending on sequence of sections in EDS file
               C_OscCanOpenObject & rc_Object = c_OdObjects[u16_Index];

               s32_Return = m_GetObjectDescription(u16_Index, 0U, false, c_SectionName, c_IniFile, rc_Object);
            }
            catch (...)
            {
               //Do not handle as an error. Malformed entry or valid section with size of 4.
            }
         }
         else if ((c_SectionName.length() > (4 + 3)) && (c_SectionName.contains("sub", Qt::CaseInsensitive) && c_SectionName.indexOf("sub", Qt::CaseInsensitive) == 4))
         {
            //Pattern: [<4 hex digits>sub<1 or 2 hex digits>, e.g. [12ABsubCD]
            try
            {
               const uint16_t u16_Index = static_cast<uint16_t>(("0x" + c_SectionName.mid(0, 4)).toInt(nullptr, 16));
               //1 or 2 characters:
               const uint8_t u8_SubIndex = static_cast<uint8_t>(("0x" + c_SectionName.mid(7, 2)).toInt(nullptr, 16));

               //create new map entry or use existing
               C_OscCanOpenObject & rc_Object = c_OdObjects[u16_Index];
               //sub-object really should be created here as it should not be present multiple times
               C_OscCanOpenObjectData & rc_SubObject = rc_Object.c_SubObjects[u8_SubIndex];

               s32_Return = m_GetObjectDescription(u16_Index, u8_SubIndex, true, c_SectionName, c_IniFile, rc_SubObject);
            }
            catch (...)
            {
               //Do not handle as an error. Most probably this is a malformed entry.
               //But there is a very low change this is a kind of valid section like e.g. "Foobsubs" which
               // does not contain an OD entry.
            }
         }
         else
         {
            //nothing that we are interested in for now ...
         }
         if (s32_Return != C_NO_ERR)
         {
            //error in EDS file; abort
            break;
         }
      }

      //check whether all referenced objects exists:
      s32_Return = m_CheckForExistingObjects("MandatoryObjects", c_IniFile);
      if (s32_Return == C_NO_ERR)
      {
         s32_Return = m_CheckForExistingObjects("OptionalObjects", c_IniFile);
      }

      if (s32_Return == C_NO_ERR)
      {
         s32_Return = m_CheckForExistingObjects("ManufacturerObjects", c_IniFile);
      }

      if (s32_Return != C_NO_ERR)
      {
         s32_Return = C_CONFIG;
      }

      if (s32_Return == C_NO_ERR)
      {
         QString c_InfoError;
         s32_Return = this->c_InfoBlock.LoadFromFile(c_IniFile, c_InfoError);
         if (s32_Return != C_NO_ERR)
         {
            this->mc_LastError = c_InfoError;
         }
      }

      if (s32_Return == C_NO_ERR)
      {
         //remember full original content of file
         QFile c_FileToRead(orc_File);
         if (c_FileToRead.open(QIODevice::ReadOnly | QIODevice::Text))
         {
            QTextStream c_Stream(&c_FileToRead);
            this->c_TextFileContent = c_Stream.readAll().split('\n');
            c_FileToRead.close();
         }
         this->m_RememberFileHash();
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check whether referenced objects exist in file.

   Loads section "orc_Blockname" and checks whether all referenced objects exist in "c_Objects"

   \param[in]         orc_Blockname               Name of EDS/DCF block to check
   \param[in,out]     orc_IniFile                 Loaded ini file instance

   \return
   C_NO_ERR    all referenced objects exist
   C_CONFIG    at least one referenced object does not exist
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::m_CheckForExistingObjects(const QString & orc_Blockname,
                                                                QSettings & orc_IniFile)
{
   int32_t s32_Return = C_NO_ERR;

   const QString c_Group = orc_Blockname + "/";
   const uint16_t u16_NumEntries = static_cast<uint16_t>(orc_IniFile.value(c_Group + "SupportedObjects", 0).toUInt());

   for (int32_t s32_Loop = 0; s32_Loop < u16_NumEntries; s32_Loop++)
   {
      const QString c_Directive = QString::number(s32_Loop + 1);
      const QString c_Index = orc_IniFile.value(c_Group + c_Directive, "").toString();
      if (c_Index == "")
      {
         mc_LastError = orc_Blockname + ": SupportedObjects inconsistent with object list !";
         s32_Return = C_CONFIG;
      }

      if (s32_Return == C_NO_ERR)
      {
         uint16_t u16_Index;
         //is the index numeric as expected?
         try
         {
            u16_Index = static_cast<uint16_t>(c_Index.toUInt());
         }
         catch (...)
         {
            mc_LastError = orc_Blockname + ": File contains non-numeric Index in SupportedObjects !";
            s32_Return = C_CONFIG;
         }

         if (s32_Return == C_NO_ERR)
         {
            //does the referenced object exist in the file?
            const std::map<uint16_t, C_OscCanOpenObject>::const_iterator c_Object = c_OdObjects.find(u16_Index);
            if (c_Object == c_OdObjects.end())
            {
               mc_LastError = orc_Blockname + ": References object 0x" +
                              C_OscUtils::h_IntToHex(u16_Index, 4) + "which is not described in the file.";
               s32_Return = C_CONFIG;
            }
         }
      }
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: get information from one EDS/DCF section

   \param[in]     ou16_Index         index of object
   \param[in]     ou8_SubIndex       sub-index of object
   \param[in]     oq_IsSubIndex      true: object is a subindex object; false: object is the main thing
   \param[in]     orc_SectionValues  EDS/DCF section contents (in the form of "parameter=value" strings)
   \param[out]    orc_Object         read information

   \return
   C_NO_ERR    data read and placed into orc_Object
   C_CONFIG    invalid data (see mc_LastError for details)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::m_GetObjectDescription(const uint16_t ou16_Index, const uint8_t ou8_SubIndex,
                                                             const bool oq_IsSubIndex, const QString & orc_SectionName,
                                                             QSettings & orc_IniFile, C_OscCanOpenObjectData & orc_Object)
{
   const uint32_t u32_NUM_STRINGS_TO_SEARCH = 10U;

   //Use table to speed up search.
   //upper case, so we only need to do this call on the side of the file content:
   static const QString ac_StringsToSearchFor[u32_NUM_STRINGS_TO_SEARCH] =
   {
      "PARAMETERNAME",
      "ACCESSTYPE",
      "SUBNUMBER",
      "DATATYPE",
      "DEFAULTVALUE",
      "LOWLIMIT",
      "HIGHLIMIT",
      "PDOMAPPING",
      "PARAMETERVALUE", //for DCF files
      "DENOTATION",     //for DCF files
   };
   bool aq_StringsAlreadyFound[u32_NUM_STRINGS_TO_SEARCH] =
   {
      false,
      false,
      false,
      false,
      false,
      false,
      false,
      false,
      false,
      false
   };

   int32_t s32_Return = C_NO_ERR;

   //preset values:
   orc_Object.u16_Index = ou16_Index;
   if (oq_IsSubIndex == true)
   {
      orc_Object.u8_NumSubs  = 0xFFU; //0xFF: is_a_sub
      orc_Object.u8_SubIndex = ou8_SubIndex;
   }
   else
   {
      orc_Object.u8_NumSubs = 0U; //spec: may be empty if no sub-index exists
      orc_Object.u8_SubIndex = 0U;
   }
   orc_Object.u8_DataType = C_OscCanOpenObjectData::hu8_DATA_TYPE_DOMAIN; //optional for "DOMAIN" objects
   orc_Object.q_IsMappableIntoPdo = false;

   const QString c_QsSection = orc_SectionName;
   orc_IniFile.beginGroup(c_QsSection);
   QStringList c_Keys = orc_IniFile.allKeys();

   for (int32_t s32_Key = 0; s32_Key < c_Keys.size(); s32_Key++)
   {
      const QString c_QsKey = c_Keys[s32_Key];
      const QString c_KeyUpperCase = c_QsKey.toUpper();

      for (uint32_t u32_StringIndex = 0U; u32_StringIndex < u32_NUM_STRINGS_TO_SEARCH; u32_StringIndex++)
      {
         if ((aq_StringsAlreadyFound[u32_StringIndex] == false) &&
             (c_KeyUpperCase == ac_StringsToSearchFor[u32_StringIndex]))
         {
            //got one !
            const QString rc_Value = orc_IniFile.value(c_QsKey, "").toString();
            aq_StringsAlreadyFound[u32_StringIndex] = true; //no need to string compare this one again

            switch (u32_StringIndex)
            {
            case 0:
               orc_Object.c_Name = rc_Value;
               break;
            case 1:
               orc_Object.c_Access = rc_Value.toUpper();
               break;
            case 2:
               try
               {
                  orc_Object.u8_NumSubs = static_cast<uint8_t>(rc_Value.toInt());
               }
               catch (...)
               {
                  mc_LastError = QString::asprintf("File contains non-numeric SubNumber for object %04X.%02X !",
                                              static_cast<uint32_t>(ou16_Index),
                                              static_cast<uint32_t>(ou8_SubIndex));
                  s32_Return = C_CONFIG;
               }
               break;
            case 3:
               try
               {
                  orc_Object.u8_DataType = static_cast<uint8_t>(rc_Value.toInt());
               }
               catch (...)
               {
                  mc_LastError = QString::asprintf("File contains non-numeric DataType for object %04X.%02X !",
                                              static_cast<uint32_t>(ou16_Index),
                                              static_cast<uint32_t>(ou8_SubIndex));
                  s32_Return = C_CONFIG;
               }
               break;
            case 4:
               orc_Object.c_DefaultValue = rc_Value;
               break;
            case 5:
               orc_Object.c_LowLimit = rc_Value;
               break;
            case 6:
               orc_Object.c_HighLimit = rc_Value;
               break;
            case 7:
               try
               {
                  const int32_t s32_Value = rc_Value.toInt();
                  if (s32_Value == 0)
                  {
                     orc_Object.q_IsMappableIntoPdo = false;
                  }
                  else if (s32_Value == 1)
                  {
                     orc_Object.q_IsMappableIntoPdo = true;
                  }
                  else
                  {
                     orc_Object.q_IsMappableIntoPdo = false;
                     mc_LastError = QString::asprintf(
                        "Invalid boolean value \"%s\" found in entry \"PDOMapping\" for object %04X.%02X !",
                        rc_Value.toUtf8().constData(),
                        static_cast<uint32_t>(ou16_Index),
                        static_cast<uint32_t>(ou8_SubIndex));
                     s32_Return = C_CONFIG;
                  }
               }
               catch (...)
               {
                  mc_LastError = QString::asprintf(
                     "Could not parse entry \"PDOMapping\" value \"%s\" for object %04X.%02X !",
                     rc_Value.toUtf8().constData(),
                     static_cast<uint32_t>(ou16_Index),
                     static_cast<uint32_t>(ou8_SubIndex));
                  s32_Return = C_CONFIG;
               }
               break;
            case 8:
               orc_Object.c_ParameterValue = rc_Value;
               break;
            case 9:
               orc_Object.c_Denotation = rc_Value;
               break;
            default:
               s32_Return = C_CONFIG;
               break;
            }
         }
      }
   }
   orc_IniFile.endGroup();
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is section read-only

   \param[in]   ou16_PdoIndex    PDO index (0 = first PDO)
   \param[in]   oq_MessageIsTx   Flag if message is TX (for CANopen manager usage this flag is inverse)
   \param[in]   ou8_OdSubIndex   Object dictionary sub index
   \param[out]  orq_IsRo         Is read-only

   \return
   STW error codes

   \retval   C_NO_ERR   Value found
   \retval   C_RANGE    Value missing
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::m_IsSectionRo(const uint16_t ou16_PdoIndex, const bool oq_MessageIsTx,
                                                    const uint8_t ou8_OdSubIndex, bool & orq_IsRo) const
{
   int32_t s32_Retval = C_NO_ERR;
   const uint16_t u16_ObjectIndex =
      C_OscCanOpenObjectDictionary::h_GetCanOpenObjectDictionaryIndexForPdo(ou16_PdoIndex, oq_MessageIsTx);
   const C_OscCanOpenObjectData * const pc_Object = this->GetCanOpenSubIndexObject(u16_ObjectIndex, ou8_OdSubIndex);

   if (pc_Object != NULL)
   {
      orq_IsRo = pc_Object->IsWriteable() == false;
   }
   else
   {
      s32_Retval = C_RANGE;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Does section exist

   \param[in]  ou16_PdoIndex     PDO index (0 = first PDO)
   \param[in]  oq_MessageIsTx    Flag if message is TX (for CANopen manager usage this flag is inverse)
   \param[in]  ou8_OdSubIndex    Object dictionary sub index

   \return
   Flags

   \retval   True    Section exists
   \retval   False   Section does not exist
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectDictionary::m_DoesSectionExist(const uint16_t ou16_PdoIndex, const bool oq_MessageIsTx,
                                                      const uint8_t ou8_OdSubIndex) const
{
   bool q_Retval;
   const uint16_t u16_ObjectIndex =
      C_OscCanOpenObjectDictionary::h_GetCanOpenObjectDictionaryIndexForPdo(ou16_PdoIndex, oq_MessageIsTx);
   const C_OscCanOpenObjectData * const pc_Object = this->GetCanOpenSubIndexObject(u16_ObjectIndex, ou8_OdSubIndex);

   if (pc_Object != NULL)
   {
      q_Retval = true;
   }
   else
   {
      q_Retval = false;
   }
   return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------

uint16_t C_OscCanOpenObjectData::GetSize(void) const
{
   return this->mu16_Size;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenObjectData::CalcHash(uint32_t & oru32_HashValue) const
{
   C_SclChecksums::CalcCRC32(this->c_Name.toUtf8().constData(), this->c_Name.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->u16_Index, sizeof(this->u16_Index), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->u8_SubIndex, sizeof(this->u8_SubIndex), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->u8_DataType, sizeof(this->u8_DataType), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_Access.toUtf8().constData(), this->c_Access.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->u8_NumSubs, sizeof(this->u8_NumSubs), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_DefaultValue.toUtf8().constData(), this->c_DefaultValue.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_ParameterValue.toUtf8().constData(), this->c_ParameterValue.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_LowLimit.toUtf8().constData(), this->c_LowLimit.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_HighLimit.toUtf8().constData(), this->c_HighLimit.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_Denotation.toUtf8().constData(), this->c_Denotation.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(&this->q_IsMappableIntoPdo, sizeof(this->q_IsMappableIntoPdo), oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------

void C_OscCanOpenObjectData::SetSize(const uint16_t ou16_Size)
{
   this->mu16_Size = ou16_Size;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get name and size of object

   \param[out]  opc_Text    type of object in text form
   \param[out]  opu8_Size   size of object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenObjectData::DataTypeToTextAndSize(QString * const opc_Text, uint8_t * const opu8_Size) const
{
   if ((u8_DataType < mu8_NUM_DATA_TYPES) && (u8_DataType > hu8_DATA_TYPE_INVALID))
   {
      if (opc_Text != NULL)
      {
         *opc_Text = mac_TextAndSizeTable[u8_DataType].c_Text;
      }
      if (opu8_Size != NULL)
      {
         *opu8_Size = mac_TextAndSizeTable[u8_DataType].u8_Size;
      }
   }
   else
   {
      if (opc_Text != NULL)
      {
         *opc_Text  = "?\?\?";
      }
      if (opu8_Size != NULL)
      {
         *opu8_Size = 0;
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscCanOpenObjectData::C_OscCanOpenObjectData() :
   mu16_Size(0),
   u16_Index(0),
   u8_SubIndex(0),
   u8_DataType(0),
   u8_NumSubs(0),
   q_IsMappableIntoPdo(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check whether object may be read from

   \return
   true:   object has access type RO, RW, RWW, RWR or CONST
   false:  object has none of those access types
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectData::IsReadable(void) const
{
   const QString c_Help = c_Access;

   return ((c_Help  == "RO") || (c_Help == "RW") || (c_Help == "RWW") || (c_Help == "RWR") || (c_Help == "CONST"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check whether object may be written to

   \return
   true:   object has access type RW, RWW, RWR or WO
   false:  object has none of those access types
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectData::IsWriteable(void) const
{
   const QString c_Help = c_Access;

   return ((c_Help == "RW") || (c_Help == "RWW") || (c_Help == "RWR") || (c_Help == "WO"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if object is mappable into a PDO

   \return
   Flags

   \retval   True    Object is mappable
   \retval   False   Object is not mappable
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectData::IsMappableIntoPdo() const
{
   return this->q_IsMappableIntoPdo;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if object is mappable into a TPDO

   \return
   Flags

   \retval   True    Object is mappable
   \retval   False   Object is not mappable
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectData::IsMappableIntoTxPdo() const
{
   const QString c_Help = c_Access.toUpper();

   return this->IsMappableIntoPdo() &&
          ((c_Help == "RO") || (c_Help == "RW") || (c_Help == "RWR") || (c_Help == "CONST"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if object is mappable into a RPDO

   \return
   Flags

   \retval   True    Object is mappable
   \retval   False   Object is not mappable
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectData::IsMappableIntoRxPdo() const
{
   const QString c_Help = c_Access.toUpper();

   return this->IsMappableIntoPdo() && ((c_Help == "WO") || (c_Help == "RW") || (c_Help == "RWW"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check whether object is of signed integer type

   \return
   true:   object is of signed integer type
   false:  it is not
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectData::IsIntegerDataType(void) const
{
   bool q_IsType = false;

   if ((u8_DataType < mu8_NUM_DATA_TYPES) && (u8_DataType != hu8_DATA_TYPE_INVALID))
   {
      q_IsType = mac_TextAndSizeTable[u8_DataType].q_IsInteger;
   }
   return q_IsType;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check whether object is of unsigned type

   \return
   true:   object is of unsigned type
   false:  it is not
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectData::IsUnsignedDataType(void) const
{
   bool q_IsType = false;

   if ((u8_DataType < mu8_NUM_DATA_TYPES) && (u8_DataType != hu8_DATA_TYPE_INVALID))
   {
      q_IsType = mac_TextAndSizeTable[u8_DataType].q_IsUnsigned;
   }
   return q_IsType;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check whether object is of REAL32 or REAL64 type

   \return
   true:   object is of REAL32 or REAL64 type
   false:  it is not
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectData::IsFloatDataType(void) const
{
   bool q_IsType = false;

   if ((u8_DataType < mu8_NUM_DATA_TYPES) && (u8_DataType != hu8_DATA_TYPE_INVALID))
   {
      q_IsType = mac_TextAndSizeTable[u8_DataType].q_IsFloat;
   }
   return q_IsType;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check whether object is of VISIBLE_STRING or UNICODE_STRING type

   \return
   true:   object is of VISIBLE_STRING or UNICODE_STRING type
   false:  it is not
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectData::IsStringDataType(void) const
{
   bool q_IsType = false;

   if ((u8_DataType < mu8_NUM_DATA_TYPES) && (u8_DataType != hu8_DATA_TYPE_INVALID))
   {
      q_IsType = mac_TextAndSizeTable[u8_DataType].q_IsString;
   }
   return q_IsType;
}

//----------------------------------------------------------------------------------------------------------------------

QString C_OscCanOpenObjectDictionary::GetLastErrorText(void) const
{
   return mc_LastError;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenObjectDictionary::CalcHash(uint32_t & oru32_HashValue) const
{
   C_SclChecksums::CalcCRC32(&this->mu32_OriginalFileHash, sizeof(this->mu32_OriginalFileHash), oru32_HashValue);

   this->c_InfoBlock.CalcHash(oru32_HashValue);

   for (std::map<uint16_t, C_OscCanOpenObject>::const_iterator c_Element = this->c_OdObjects.begin();
        c_Element != this->c_OdObjects.end();
        ++c_Element)
   {
      c_Element->second.CalcHash(oru32_HashValue);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is object heartbeat consumer supported

   \return
   0 : None supported
   Else : Number of supported heartbeat consumers
*/
//----------------------------------------------------------------------------------------------------------------------
uint8_t C_OscCanOpenObjectDictionary::GetNumHeartbeatConsumers() const
{
   uint8_t u8_Retval = 0;
   const C_OscCanOpenObjectData * const pc_Object = this->GetCanOpenObject(hu16_OD_INDEX_HEARTBEAT_CONSUMER);

   if (pc_Object != NULL)
   {
      u8_Retval = pc_Object->u8_NumSubs;
      if (u8_Retval > 0U)
      {
         //Don't count the first one (sub index counter)
         u8_Retval -= 1U;
      }
   }
   return u8_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is heartbeat consumer read-only

   \param[out]  orq_IsRo         Is read-only

   \return
   STW error codes

   \retval   C_NO_ERR   Value found
   \retval   C_RANGE    Value missing
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::IsHeartbeatConsumerRo(bool & orq_IsRo) const
{
   int32_t s32_Retval = C_NO_ERR;
   // Checking the first consumer for read only because this is the entry which will be used for the user specified
   // value when it is not read-only
   const C_OscCanOpenObjectData * const pc_Object =
      this->GetCanOpenSubIndexObject(hu16_OD_INDEX_HEARTBEAT_CONSUMER, 1U);

   if (pc_Object != NULL)
   {
      orq_IsRo = pc_Object->IsWriteable() == false;
   }
   else
   {
      s32_Retval = C_RANGE;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is heartbeat producer supported

   \return
   Flags

   \retval   True    Heartbeat producer object present
   \retval   False   Heartbeat producer object not present
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectDictionary::IsHeartbeatProducerSupported() const
{
   return CheckObjectPresentByIndex(hu16_OD_INDEX_HEARTBEAT_PRODUCER, 0);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is heartbeat producer read-only

   \param[out]  orq_IsRo         Is read-only

   \return
   STW error codes

   \retval   C_NO_ERR   Value found
   \retval   C_RANGE    Value missing
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::IsHeartbeatProducerRo(bool & orq_IsRo) const
{
   int32_t s32_Retval = C_NO_ERR;
   const C_OscCanOpenObjectData * const pc_Object = this->GetCanOpenObject(hu16_OD_INDEX_HEARTBEAT_PRODUCER);

   if (pc_Object != NULL)
   {
      orq_IsRo = pc_Object->IsWriteable() == false;
   }
   else
   {
      s32_Retval = C_RANGE;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Does device support EMCY

  \return
  Flags

   \retval   True    EMCY object present
   \retval   False   EMCY object not present
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectDictionary::IsEmcySupported() const
{
   return CheckObjectPresentByIndex(hu16_OD_INDEX_EMCY, 0);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get granularity

   \return
   Granularity
*/
//----------------------------------------------------------------------------------------------------------------------
uint8_t C_OscCanOpenObjectDictionary::GetGranularity() const
{
   return this->c_InfoBlock.GetGranularity();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get all available factory settings sub indices

   \return
   All available factory settings sub indices
*/
//----------------------------------------------------------------------------------------------------------------------
std::set<uint8_t> C_OscCanOpenObjectDictionary::GetAllAvailableFactorySettingsSubIndices() const
{
   const std::set<uint8_t> c_Retval = this->GetAllAvailableSubIndices(0x1011U);

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get all available sub indices

   \param[in]  ou16_OdIndex  Index

   \return
   All available sub indices
*/
//----------------------------------------------------------------------------------------------------------------------
std::set<uint8_t> C_OscCanOpenObjectDictionary::GetAllAvailableSubIndices(const uint16_t ou16_OdIndex) const
{
   std::set<uint8_t> c_Retval;

   const std::map<uint16_t, C_OscCanOpenObject>::const_iterator c_Object = c_OdObjects.find(ou16_OdIndex);
   if (c_Object != c_OdObjects.end())
   {
      for (std::map<uint8_t, C_OscCanOpenObjectData>::const_iterator c_Element = c_Object->second.c_SubObjects.begin();
           c_Element != c_Object->second.c_SubObjects.end();
           ++c_Element)
      {
         if (c_Element->second.u8_NumSubs == C_OscCanOpenObjectData::hu8_NUM_SUBS_WE_ARE_A_SUB)
         {
            c_Retval.insert(c_Element->second.u8_SubIndex);
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Does inhibit time section exist

   \param[in]  ou16_PdoIndex     PDO index (0 = first PDO)
   \param[in]  oq_MessageIsTx    Flag if message is TX (for CANopen manager usage this flag is inverse)

   \return
   Flags

   \retval   true    Section exists
   \retval   false   Section does not exist
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectDictionary::DoesInhibitTimeSectionExist(const uint16_t ou16_PdoIndex,
                                                               const bool oq_MessageIsTx) const
{
   return this->m_DoesSectionExist(ou16_PdoIndex, oq_MessageIsTx,
                                   C_OscCanOpenObjectDictionary::hu8_OD_SUB_INDEX_INHIBIT_TIME);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Does event timer section exist

   \param[in]  ou16_PdoIndex     PDO index (0 = first PDO)
   \param[in]  oq_MessageIsTx    Flag if message is TX (for CANopen manager usage this flag is inverse)

   \return
   Flags

   \retval   True    Section exists
   \retval   False   Section does not exist
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectDictionary::DoesEventTimerSectionExist(const uint16_t ou16_PdoIndex,
                                                              const bool oq_MessageIsTx) const
{
   return this->m_DoesSectionExist(ou16_PdoIndex, oq_MessageIsTx,
                                   C_OscCanOpenObjectDictionary::hu8_OD_SUB_INDEX_EVENT_TIMER);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is COB-ID read-only

   \param[in]   ou16_PdoIndex    PDO index (0 = first PDO)
   \param[in]   oq_MessageIsTx   Flag if message is TX (for CANopen manager usage this flag is inverse)
   \param[out]  orq_IsRo         Is read-only

   \return
   STW error codes

   \retval   C_NO_ERR   Value found
   \retval   C_RANGE    Value missing
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::IsCobIdRo(const uint16_t ou16_PdoIndex, const bool oq_MessageIsTx,
                                                bool & orq_IsRo) const
{
   const int32_t s32_Retval = this->m_IsSectionRo(ou16_PdoIndex, oq_MessageIsTx,
                                                  C_OscCanOpenObjectDictionary::hu8_OD_SUB_INDEX_COB_ID,
                                                  orq_IsRo);

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is inhibit time read-only

   \param[in]   ou16_PdoIndex    PDO index (0 = first PDO)
   \param[in]   oq_MessageIsTx   Flag if message is TX (for CANopen manager usage this flag is inverse)
   \param[out]  orq_IsRo         Is read-only

   \return
   STW error codes

   \retval   C_NO_ERR   Value found
   \retval   C_RANGE    Value missing
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::IsInhibitTimeRo(const uint16_t ou16_PdoIndex, const bool oq_MessageIsTx,
                                                      bool & orq_IsRo) const
{
   const int32_t s32_Retval = this->m_IsSectionRo(
      ou16_PdoIndex, oq_MessageIsTx,
      C_OscCanOpenObjectDictionary::hu8_OD_SUB_INDEX_INHIBIT_TIME,
      orq_IsRo);

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is event timer read-only

   \param[in]   ou16_PdoIndex    PDO index (0 = first PDO)
   \param[in]   oq_MessageIsTx   Flag if message is TX (for CANopen manager usage this flag is inverse)
   \param[out]  orq_IsRo         Is read-only

   \return
   STW error codes

   \retval   C_NO_ERR   Value found
   \retval   C_RANGE    Value missing
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::IsEventTimerRo(const uint16_t ou16_PdoIndex, const bool oq_MessageIsTx,
                                                     bool & orq_IsRo) const
{
   const int32_t s32_Retval = this->m_IsSectionRo(
      ou16_PdoIndex, oq_MessageIsTx,
      C_OscCanOpenObjectDictionary::hu8_OD_SUB_INDEX_EVENT_TIMER,
      orq_IsRo);

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is transmission type read-only

   \param[in]   ou16_PdoIndex    PDO index (0 = first PDO)
   \param[in]   oq_MessageIsTx   Flag if message is TX (for CANopen manager usage this flag is inverse)
   \param[out]  orq_IsRo         Is read-only

   \return
   STW error codes

   \retval   C_NO_ERR   Value found
   \retval   C_RANGE    Value missing
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::IsTransmissionTypeRo(const uint16_t ou16_PdoIndex, const bool oq_MessageIsTx,
                                                           bool & orq_IsRo) const
{
   const int32_t s32_Retval = this->m_IsSectionRo(ou16_PdoIndex, oq_MessageIsTx,
                                                  C_OscCanOpenObjectDictionary::hu8_OD_SUB_INDEX_TRANSMISSION_TYPE,
                                                  orq_IsRo);

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is sync start value read-only

   \param[in]   ou16_PdoIndex    PDO index (0 = first PDO)
   \param[in]   oq_MessageIsTx   Flag if message is TX (for CANopen manager usage this flag is inverse)
   \param[out]  orq_IsRo         Is read-only

   \return
   STW error codes

   \retval   C_NO_ERR   Value found
   \retval   C_RANGE    Value missing
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::IsSyncStartRo(const uint16_t ou16_PdoIndex, const bool oq_MessageIsTx,
                                                    bool & orq_IsRo) const
{
   const int32_t s32_Retval = this->m_IsSectionRo(ou16_PdoIndex, oq_MessageIsTx,
                                                  C_OscCanOpenObjectDictionary::hu8_OD_SUB_INDEX_SYNC_START_VALUE,
                                                  orq_IsRo);

   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is PDO mapping read-only

   \param[in]   ou16_PdoIndex    PDO index (0 = first PDO)
   \param[in]   oq_MessageIsTx   Flag if message is TX (for CANopen manager usage this flag is inverse)
   \param[out]  orq_IsRo         Is read-only

   \return
   STW error codes

   \retval   C_NO_ERR   Value found
   \retval   C_RANGE    Value missing
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanOpenObjectDictionary::IsPdoMappingRo(const uint16_t ou16_PdoIndex, const bool oq_MessageIsTx,
                                                     bool & orq_IsRo) const
{
   int32_t s32_Retval = C_NO_ERR;
   const uint16_t u16_ObjectIndex =
      C_OscCanOpenObjectDictionary::h_GetCanOpenObjectDictionaryIndexForPdo(ou16_PdoIndex, oq_MessageIsTx) +
      C_OscCanOpenObjectDictionary::hu16_OD_PDO_MAPPING_OFFSET;
   const C_OscCanOpenObjectData * const pc_Object = this->GetCanOpenSubIndexObject(u16_ObjectIndex, 0U);

   if (pc_Object != NULL)
   {
      orq_IsRo = pc_Object->IsWriteable() == false;
   }
   else
   {
      s32_Retval = C_RANGE;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get CANopen object

   Goes through the list of known object dictionary entries and returns a pointer to the object with the specified
    index.

   \param[in]  ou16_OdIndex   Object dictionary index

   \return
   NULL  CANopen object not found
   Else  Valid CANopen object
*/
//----------------------------------------------------------------------------------------------------------------------
const C_OscCanOpenObjectData * C_OscCanOpenObjectDictionary::GetCanOpenObject(const uint16_t ou16_OdIndex) const
{
   const C_OscCanOpenObjectData * pc_Retval = NULL;

   const std::map<uint16_t, C_OscCanOpenObject>::const_iterator c_Object = c_OdObjects.find(ou16_OdIndex);

   if (c_Object != c_OdObjects.end())
   {
      pc_Retval = &c_Object->second;
   }

   return pc_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get CANopen sub index object

   \param[in]  ou16_OdIndex      Object dictionary index
   \param[in]  ou8_OdSubIndex    Object dictionary sub index

   \return
   NULL  CANopen sub index object not found
   Else  Valid CANopen sub index object
*/
//----------------------------------------------------------------------------------------------------------------------
const C_OscCanOpenObjectData * C_OscCanOpenObjectDictionary::GetCanOpenSubIndexObject(const uint16_t ou16_OdIndex,
                                                                                      const uint8_t ou8_OdSubIndex)
const
{
   const C_OscCanOpenObjectData * pc_SubObject = NULL;

   const std::map<uint16_t, C_OscCanOpenObject>::const_iterator c_Object = this->c_OdObjects.find(ou16_OdIndex);

   if (c_Object != this->c_OdObjects.end())
   {
      const std::map<uint8_t, C_OscCanOpenObjectData>::const_iterator c_SubObject = c_Object->second.c_SubObjects.find(
         ou8_OdSubIndex);
      if (c_SubObject != c_Object->second.c_SubObjects.end())
      {
         pc_SubObject = &c_SubObject->second;
      }
   }
   return pc_SubObject;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check object present by index

   \param[in]  ou16_OdIndex      Object dictionary index
   \param[in]  ou8_OdSubIndex    Object dictionary sub index (0 = get main object)

   \return
   Flags

   \retval   True    Object present
   \retval   False   Object not present
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanOpenObjectDictionary::CheckObjectPresentByIndex(const uint16_t ou16_OdIndex,
                                                             const uint8_t ou8_OdSubIndex) const
{
   bool q_Retval = false;

   const std::map<uint16_t, C_OscCanOpenObject>::const_iterator c_Object = this->c_OdObjects.find(ou16_OdIndex);

   if (c_Object != this->c_OdObjects.end())
   {
      if (ou8_OdSubIndex == 0)
      {
         q_Retval = true;
      }
      else
      {
         const std::map<uint8_t,
                        C_OscCanOpenObjectData>::const_iterator c_SubObject = c_Object->second.c_SubObjects.find(
            ou8_OdSubIndex);
         if (c_SubObject != c_Object->second.c_SubObjects.end())
         {
            q_Retval = true;
         }
      }
   }

   return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get mappable objects

   Create a map with all objects that are mappable into a PDO.
   If an entry is a mappable a map entry "orc_SubIndices[index]" will be crated.
   Depending on the sub-index a value will be pushed to the map's vector:
   * if the mappable object is the object itself: 0
   * if the mappable object is a sub-object: sub-object index

   \param[in,out]  orc_SubIndices   Sub indices
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenObjectDictionary::GetMappableObjects(std::map<uint32_t, std::vector<uint32_t> > & orc_SubIndices)
const
{
   for (std::map<uint16_t, C_OscCanOpenObject>::const_iterator c_Element = this->c_OdObjects.begin();
        c_Element != this->c_OdObjects.end();
        ++c_Element)
   {
      if (c_Element->second.IsMappableIntoPdo() == true)
      {
         orc_SubIndices[c_Element->first].push_back(0U);
      }

      //check sub objects:
      for (std::map<uint8_t, C_OscCanOpenObjectData>::const_iterator c_SubElement =
              c_Element->second.c_SubObjects.begin();
           c_SubElement != c_Element->second.c_SubObjects.end();
           ++c_SubElement)
      {
         if (c_SubElement->second.IsMappableIntoPdo() == true)
         {
            orc_SubIndices[c_Element->first].push_back(c_SubElement->first);
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get CANopen object dictionary index for PDO

   \param[in]  ou16_PdoIndex     PDO index (0 = first PDO)
   \param[in]  oq_MessageIsTx    Flag if message is TX (for CANopen manager usage this flag is inverse)

   \return
   Object dictionary index
*/
//----------------------------------------------------------------------------------------------------------------------
uint16_t C_OscCanOpenObjectDictionary::h_GetCanOpenObjectDictionaryIndexForPdo(const uint16_t ou16_PdoIndex,
                                                                               const bool oq_MessageIsTx)
{
   uint16_t u16_Retval;

   if (oq_MessageIsTx)
   {
      u16_Retval = C_OscCanOpenObjectDictionary::hu16_OD_INDEX_FIRST_TX_PDO + ou16_PdoIndex;
   }
   else
   {
      u16_Retval = C_OscCanOpenObjectDictionary::hu16_OD_INDEX_FIRST_RX_PDO + ou16_PdoIndex;
   }
   return u16_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
