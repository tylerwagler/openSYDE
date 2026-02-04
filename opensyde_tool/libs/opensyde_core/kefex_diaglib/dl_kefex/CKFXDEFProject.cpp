//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       KEFEX DEF Project handling

   Implements reading DEF-style KEFEX projects.

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers
#include <QFileInfo>

#include "DiagLib_config.hpp" //diaglib configuration

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "CKFXDEFProject.hpp"
#include <QSettings>
#include "C_SclString.hpp"

#include "DLLocalize.hpp"

//----------------------------------------------------------------------------------------------------------------------

using namespace stw::errors;
using namespace stw::diag_lib;
using namespace stw::scl;

using namespace stw::opensyde_core;

/* -- Defines ------------------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

//using unions is not nice but more portable than reinterpret_casting
union U_Union64
{
   int64_t s64_Value;
   float64_t f64_Value;
}; //lint !e1960  //cf. comment above

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
static const char_t macnc_RAM_FILE_EXTENSION[] = "ram";

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

C_KFXRAMNameIndex & C_KFXRAMNameIndex::operator =(const C_KFXRAMNameIndex & orc_Source)
{
   if (&orc_Source != this)
   {
      c_FileName  = orc_Source.c_FileName;
      u16_ListIndex = orc_Source.u16_ListIndex;
   }
   return (*this);
}

//----------------------------------------------------------------------------------------------------------------------

C_KFXRAMNameIndex::C_KFXRAMNameIndex()
{
   u16_ListIndex = 0;
}

//----------------------------------------------------------------------------------------------------------------------

C_KFXRAMNameIndex::C_KFXRAMNameIndex(const C_KFXRAMNameIndex & orc_Source)
{
   this->operator =(orc_Source);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief    Load data of one RAM variable list from corresponding .RAM file

   Open .RAM file and load variable descriptions for one list.

   Assumptions:
   - it has already been checked that the device name in the RAM file matches the project
   - file checksum has already been checked

   \param[in]     orc_FilePath       Full path to .ram-File
   \param[out]    orc_List           Variable List
   \param[out]    orc_ErrorText      additional text information on error

   \return
   C_NO_ERR           data loaded, results placed in oc_List   \n
   C_NOACT            no valid list name in file (is it a valid .ram file ?) \n
   C_CONFIG           invalid variable types in .ram file \n
   C_RANGE            invalid access right mode in .ram file \n
   C_RD_WR            file does not exist \n
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_KFXDEFProject::m_LoadRAMListFromFile(const C_SclString & orc_FilePath, C_KFXVariableListBase & orc_List,
                                               C_SclString & orc_ErrorText)
{
   uint16_t u16_NumVars;
   QSettings * pc_IniFile;
   C_KFXVariableBase * pt_Entry;
   QString c_Section;
   C_SclString c_Directive;
   C_SclString c_Temp;

   bool q_Ok;
   int32_t j;
   int32_t k;

   if ((QFileInfo(orc_FilePath.ToQString()).exists() && QFileInfo(orc_FilePath.ToQString()).isFile()) == false)
   {
      orc_ErrorText = orc_FilePath + ":\n" + stw::opensyde_core::C_OscUtils::h_LoadString(STR_DS_ERR_DOES_NOT_EXIST);
      return C_RD_WR;
   }

   //read out RAM file
   try
   {
      pc_IniFile = new QSettings(orc_FilePath.ToQString(), QSettings::IniFormat);
   }
   catch (...)
   {
      Q_ASSERT(false);
      return C_OVERFLOW;
   }

   orc_List.VariableList.resize(0);

   u16_NumVars                  = static_cast<uint16_t>(pc_IniFile->value("CONFIG/NUMOFVARS", 0).toUInt());
   orc_List.q_CheckSummed       = pc_IniFile->value("CONFIG/CHECKSUMMED", false).toBool();
   orc_List.u32_CheckSumAddress = static_cast<uint32_t>(pc_IniFile->value("CONFIG/CRCADDRESS", 1024).toUInt());
   orc_List.c_ListName          = pc_IniFile->value("CONFIG/LISTNAME", "").toString();
   orc_List.u8_ListType         = static_cast<uint8_t>(pc_IniFile->value("CONFIG/VARIABLETYPE", KFX_VARIABLE_TYPE_RAM).toUInt());
   if (orc_List.c_ListName == "")
   {
      orc_ErrorText = orc_FilePath + ":\n" + stw::opensyde_core::C_OscUtils::h_LoadString(STR_DS_ERR_NO_VALID_NAME_IN_LIST);
      delete pc_IniFile;
      return C_NOACT;
   }

   orc_List.SetNumDefaults(static_cast<uint16_t>(pc_IniFile->value("CONFIG/NUMDEFAULTS", 1).toUInt()));

   orc_List.VariableList.resize(u16_NumVars); //much faster than "++" in a for loop
   for (j = 0; j < u16_NumVars; j++)
   {
      pt_Entry = &orc_List.VariableList[j];
      c_Section = "VARIABLE" + QString::number(j + 1);

      pt_Entry->c_Name = pc_IniFile->value(c_Section + "/NAME", "").toString();
      c_Temp = C_SclString(pc_IniFile->value(c_Section + "/ADDRESS", "").toString().toStdString());
      pt_Entry->u32_Address = (c_Temp == "") ? 0UL : static_cast<uint32_t>(c_Temp.ToInt64());

      q_Ok = true;
      if (pc_IniFile->contains(c_Section + "/TYPE_INDEX") == false) //TYPE_INDEX exists !! -> type is contained as an index
      {
         q_Ok = false;
      }

      k = KFX_DATA_TYPE_NOVAR;
      if (q_Ok == true)
      {
         try
         {
            k = pc_IniFile->value(c_Section + "/TYPE_INDEX").toInt();
         }
         catch (...)
         {
            q_Ok = false;
         }
      }

      if (q_Ok == false)
      {
         q_Ok = true;
         //not numeric -> name of type (following very old conventions) !!! -> stay compatible to old files
         c_Temp = C_SclString(pc_IniFile->value(c_Section + "/TYPE", "").toString().toStdString()).UpperCase().Trim();
         if (c_Temp == "BYTE")
         {
            k = KFX_DATA_TYPE_UINT8;
         }
         else if (c_Temp == "CHAR")
         {
            k = KFX_DATA_TYPE_SINT8;
         }
         else if (c_Temp == "INT")
         {
            k = KFX_DATA_TYPE_SINT16;
         }
         else if (c_Temp == "WORD")
         {
            k = KFX_DATA_TYPE_UINT16;
         }
         else if (c_Temp == "LONG")
         {
            k = KFX_DATA_TYPE_SINT32;
         }
         else if (c_Temp == "DWORD")
         {
            k = KFX_DATA_TYPE_UINT32;
         }
         else if ((c_Temp == "STRING") || (c_Temp == "ARRAY"))
         {
            //"array" -> compatible to even older files
            k = KFX_DATA_TYPE_ASINT8;
         }
         else if (c_Temp == "AOBYTE")
         {
            k = KFX_DATA_TYPE_AUINT8;
         }
         else if (c_Temp == "FLOAT")
         {
            k = KFX_DATA_TYPE_FLOAT32;
         }
         else
         {
            q_Ok = false;
         }
      }
      if ((k >= KFX_NUM_DATA_TYPES) || (k == KFX_DATA_TYPE_NOVAR))
      {
         q_Ok = false;
      }
      if (q_Ok == false)
      {
         orc_ErrorText = orc_FilePath + ":\n" + c_Temp + " " + stw::opensyde_core::C_OscUtils::h_LoadString(STR_DS_INVALID_TYPE);
         delete pc_IniFile;
         return C_CONFIG;
      }
      pt_Entry->u8_Type = static_cast<uint8_t>(k);

      c_Temp = C_SclString(pc_IniFile->value(c_Section + "/SIZE", "").toString().toStdString());
      if ((c_Temp == "") || (c_Temp == "0"))
      {
         //possibly an old project file ...
         //if it's a numeric data type set size automatically ...
         switch (pt_Entry->u8_Type)
         {
         case KFX_DATA_TYPE_UINT8: //no break
         case KFX_DATA_TYPE_SINT8:
            pt_Entry->SetSize(1);
            break;
         case KFX_DATA_TYPE_SINT16: //no break
         case KFX_DATA_TYPE_UINT16: //no break
         case KFX_DATA_TYPE_CRC:
            pt_Entry->SetSize(2);
            break;
         case KFX_DATA_TYPE_SINT32: //no break
         case KFX_DATA_TYPE_UINT32:
            pt_Entry->SetSize(4);
            break;
         default:
            pt_Entry->SetSize(0);
            break;
         }
      }
      else
      {
         pt_Entry->SetSize(static_cast<uint32_t>(c_Temp.ToInt()));
      }
      c_Temp = C_SclString(pc_IniFile->value(c_Section + "/LOCATIONRAM", "").toString().toStdString()).UpperCase();
      if ((c_Temp == "1") || (c_Temp == "TRUE"))
      {
         pt_Entry->q_LocationRAM = true;
      }
      else
      {
         pt_Entry->q_LocationRAM = false;
      }

      m_VarStringsToMinMax(C_SclString(pc_IniFile->value(c_Section + "/MIN", "").toString().toStdString()),
                           C_SclString(pc_IniFile->value(c_Section + "/MAX", "").toString().toStdString()), *pt_Entry);

      for (k = 0; k < KFX_NUM_ACCESS_GROUPS; k++)
      {
         c_Directive = "ACCESS" + QString::number(k);
         c_Temp = C_SclString(pc_IniFile->value(c_Section + "/" + c_Directive.ToQString(), "").toString().toStdString()).UpperCase();
         if (c_Temp == "RO")
         {
            pt_Entry->au8_Access[k] = KFX_DATA_ACCESS_RO;
         }
         else if (c_Temp == "WO")
         {
            pt_Entry->au8_Access[k] = KFX_DATA_ACCESS_WO;
         }
         else if (c_Temp == "RW")
         {
            pt_Entry->au8_Access[k] = KFX_DATA_ACCESS_RW;
         }
         else if (c_Temp == "INV")
         {
            pt_Entry->au8_Access[k] = KFX_DATA_ACCESS_INVISIBLE;
         }
         else
         {
            orc_ErrorText = orc_FilePath + ":\n" + c_Temp + stw::opensyde_core::C_OscUtils::h_LoadString(STR_DS_INVALID_ACC_TYPE);
            delete pc_IniFile;
            return C_RANGE;
         }
      }
      pt_Entry->c_Unit       = C_SclString(pc_IniFile->value(c_Section + "/UNIT", "").toString().toStdString()).ToQString();
      pt_Entry->SetNumericValue(0LL);
      pt_Entry->SetNumDefaults(orc_List.GetNumDefaults());

      c_Temp = C_SclString(pc_IniFile->value(c_Section + "/SCALINGFACTOR", "").toString().toStdString());
      if (c_Temp == "")
      {
         c_Temp = KFX_FACTOR_VALUE_100_PERCENT;
      }
      try
      {
         pt_Entry->s32_ScalingFactor = c_Temp.ToInt();
      }
      catch (...)
      {
         pt_Entry->s32_ScalingFactor = KFX_FACTOR_VALUE_100_PERCENT;
      }
      c_Temp = C_SclString(pc_IniFile->value(c_Section + "/SCALINGDIGITS", "").toString().toStdString());
      if (c_Temp == "")
      {
         c_Temp = 0;
      }
      try
      {
         pt_Entry->u8_ScalingDigits = static_cast<uint8_t>(c_Temp.ToInt());
      }
      catch (...)
      {
         pt_Entry->u8_ScalingDigits = 0;
      }

      pt_Entry->e_DefTransmissionType = TransTypeStringToEnum(C_SclString(pc_IniFile->value(c_Section + "/TRANSTYPE", "").toString().toStdString()).UpperCase());
      try
      {
         pt_Entry->u16_DefInterval = static_cast<uint16_t>(pc_IniFile->value(c_Section + "/TRANSTIME", "").toInt());
      }
      catch (...)
      {
         pt_Entry->u16_DefInterval = 200;
      }
      try
      {
         pt_Entry->u32_DefLowerHysteresis = static_cast<uint32_t>(pc_IniFile->value(c_Section + "/LOWERHYST", "").toLongLong());
      }
      catch (...)
      {
         pt_Entry->u32_DefLowerHysteresis = 10;
      }
      try
      {
         pt_Entry->u32_DefUpperHysteresis = static_cast<uint32_t>(pc_IniFile->value(c_Section + "/UPPERHYST", "").toLongLong());
      }
      catch (...)
      {
         pt_Entry->u32_DefUpperHysteresis = 10;
      }

      try
      {
         pt_Entry->u8_Class = static_cast<uint8_t>(pc_IniFile->value(c_Section + "/VAR_CLASS", "").toInt());
      }
      catch (...)
      {
         pt_Entry->u8_Class = KFX_DATA_CLASS_SIGNAL;
      }
   }

   delete pc_IniFile;
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------

void C_KFXDEFProject::m_VarStringsToMinMax(const C_SclString & orc_Min, const C_SclString & orc_Max,
                                           C_KFXVariableBase & orc_Variable)
{
   C_SclString c_Min = orc_Min;
   C_SclString c_Max = orc_Max;

   QList<C_SclString> c_TokensMin;
   QList<C_SclString> c_TokensMax;
   uint32_t u32_Index;
   U_Union64 u_Val64;
   uint8_t u8_Value;

   if (c_Min == "")
   {
      c_Min = "0";
   }
   if (c_Max == "")
   {
      c_Max = "0";
   }
   else if (orc_Variable.IsArrayType() == true)
   {
      //preset with absolute min/max values:
      orc_Variable.SetMinMaxToMaximum();

      //compatibility for projects without min/max for arrays: use maximum range of variable type:
      if ((c_Min != "0") && (c_Max != "0"))
      {
         //parse and set
         c_Min.Tokenize(";", c_TokensMin);
         c_Max.Tokenize(";", c_TokensMax);

         for (u32_Index = 0U; u32_Index < orc_Variable.GetSize(); u32_Index++)
         {
            if (c_Min.Length() > u32_Index)
            {
               try
               {
                  u8_Value = static_cast<uint8_t>(c_TokensMin[u32_Index].ToInt());
                  orc_Variable.GetMinReference()[u32_Index] = u8_Value;
               }
               catch (...)
               {
                  //leave it as it is ...
               }
            }
            if (c_Max.Length() > u32_Index)
            {
               try
               {
                  u8_Value = static_cast<uint8_t>(c_TokensMax[u32_Index].ToInt());
                  orc_Variable.GetMaxReference()[u32_Index] = u8_Value;
               }
               catch (...)
               {
                  //leave it as it is ...
               }
            }
         }
      }
   }
   else
   {
      //integer or float type (no array)
      try
      {
         u_Val64.s64_Value = c_Min.ToInt64();
         //keep compatibility: float min/max values are saved in int64 representation
         if (orc_Variable.IsFloatType() == true)
         {
            orc_Variable.SetFloatMin(u_Val64.f64_Value);
         }
         else
         {
            orc_Variable.SetNumericMin(u_Val64.s64_Value);
         }

         u_Val64.s64_Value = c_Max.ToInt64();
         if (orc_Variable.IsFloatType() == true)
         {
            orc_Variable.SetFloatMax(u_Val64.f64_Value);
         }
         else
         {
            orc_Variable.SetNumericMax(u_Val64.s64_Value);
         }
      }
      catch (...)
      {
         orc_Variable.SetNumericMin(0LL);
         orc_Variable.SetNumericMax(0LL);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief    Convert transmission type string to enum value

   Convert transmission type textual description to corresponding enumeration.

   \param[in]     orc_Text           Transmission type as text

   \return
   Transmission type enum         if no valid transmission type is detected in oc_Text "eSRR" is returned
*/
//----------------------------------------------------------------------------------------------------------------------
E_TransmissionType C_KFXDEFProject::TransTypeStringToEnum(const C_SclString & orc_Text)
{
   E_TransmissionType e_Return;

   if (orc_Text == "TCRR")
   {
      e_Return = eTCRR;
   }
   else if (orc_Text == "ECRR")
   {
      e_Return = eECRR;
   }
   else if ((orc_Text == "TCRR(TS)") || (orc_Text == "TCRRTS"))
   {
      e_Return = eTCRRTS;
   }
   else
   {
      e_Return = eSRR; //default: SRR
   }

   return e_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief    Append data from .RAM files to variable table

   Clear VariableLists
   Search of all .ram files in oc_Directory belonging to the device oc_DeviceName.
   Read .RAM files and add the contents at the end of "VariableLists".

   \param[in]     orc_Directory    directory to seatch .ram files in
   \param[in]     orc_DeviceName   device name the .ram files must have.
   \param[in,out] orc_Lists        variable lists
   \param[out]    orc_ErrorText    additional text information on error
   \param[out]    orc_Warnings     warning texts (should be displayed by application even when returning C_NO_ERR)

   \return
   C_NO_ERR   -> lists appended           \n
   C_WARN     -> no lists found           \n
   C_CONFIG   -> list consistency error   \n
   C_RD_WR    -> error reading .ram file  \n
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_KFXDEFProject::LoadRAMFiles(const C_SclString & orc_Directory, const C_SclString & orc_DeviceName,
                                      C_KFXVariableLists & orc_Lists, C_SclString & orc_ErrorText,
                                      QList<C_SclString> & orc_Warnings)
{
   int32_t s32_Return;
   int32_t s32_Index;

   QList<C_KFXRAMNameIndex> c_Files;

   orc_Lists.resize(0);
   orc_Warnings.resize(0);

   //now search the work directory for all .RAM files which refer to this device !
   m_FindRelatedFiles(orc_Directory, orc_DeviceName, c_Files, orc_Warnings);
   if (c_Files.size() == 0)
   {
      orc_ErrorText = orc_Directory + ":\n" + stw::opensyde_core::C_OscUtils::h_LoadString(STR_DS_ERR_NO_RAM_LISTS_FOUND);
      return C_WARN;
   }

   //Sort by ListIndex
   s32_Return = m_SortRAMLists(c_Files, orc_ErrorText);
   if (s32_Return != C_NO_ERR)
   {
      return C_CONFIG;
   }

   orc_Lists.resize(c_Files.size());

   for (s32_Index = 0; s32_Index < c_Files.size(); s32_Index++)
   {
      //Read in the RAM-Files
      s32_Return = m_LoadRAMListFromFile(c_Files[s32_Index].c_FileName, orc_Lists[s32_Index], orc_ErrorText);
      if (s32_Return != C_NO_ERR)
      {
         return C_RD_WR;
      }
   }
   return C_NO_ERR;
}

//**************************************************************.FA*
//   .FUNCTION [ C_KFXDEFProject::m_SortRAMLists ]
//   .GROUP [ C_KFXDEFProject private functions ]
//--------------------------------------------------------------.F1-----------------------------------------------------
//   .DESCRIPTION
//   Sorts the given struct dependent of the "wListIndex".
//   - lowest listIndex at position 0 of the array
//   Also checks for consistency: list indexes must start with "0" and
//    there shall not be any gaps.
//----------------------------------------------------------------------------------------------------------------------
//   .PARAMETERS
//    Name              Comment
//    orc_Files         Reference to the list to change
//    orc_ErrorText     additional text information on error (out)
//----------------------------------------------------------------------------------------------------------------------
//   .RETURNVALUE
//   C_NO_ERR         sorted
//   C_CONFIG         consistency problem
//**************************************************************.FE*
int32_t C_KFXDEFProject::m_SortRAMLists(QList<C_KFXRAMNameIndex> & orc_Files, C_SclString & orc_ErrorText)
{
   int32_t s32_File;
   C_KFXRAMNameIndex c_Temp;

   Q_ASSERT(orc_Files.size() != 0); //assume: check for 0 was already made ...
   if (orc_Files.size() == 1)
   {
      //check for correct index if there's only one list ...
      if (orc_Files[0].u16_ListIndex != 0)
      {
         orc_ErrorText = orc_Files[0].c_FileName + ":\n" + stw::opensyde_core::C_OscUtils::h_LoadString(STR_DS_ERR_PROJECT_INVALID);
         return C_CONFIG;
      }
      return C_NO_ERR;
   }

   //Sort
   for (s32_File = 0; s32_File < (orc_Files.size() > 0 ? orc_Files.size() - 1 : 0); s32_File++)
   {
      if (orc_Files[s32_File + 1].u16_ListIndex < orc_Files[s32_File].u16_ListIndex)
      {
         c_Temp = orc_Files[s32_File];
         orc_Files[s32_File] = orc_Files[s32_File + 1];
         orc_Files[s32_File + 1] = c_Temp;
         //Search again
         s32_File = -1;
      }
   }

   //check consistency
   // list indices must start with 0, have no gaps and no listindex used twice
   for (s32_File = 0; s32_File < orc_Files.size(); s32_File++)
   {
      if (orc_Files[s32_File].u16_ListIndex < static_cast<uint16_t>(s32_File)) //two lists with the same index !
      {
         orc_ErrorText = orc_Files[s32_File].c_FileName + ":\n" + stw::opensyde_core::C_OscUtils::h_LoadString(STR_DS_ERR_PROJECT_INVALID) +
                         "\n Two lists have the same list index:\n" +
                         orc_Files[s32_File - 1].c_FileName +
                         " and\n" + orc_Files[s32_File].c_FileName;
         return C_CONFIG;
      }
      else if (orc_Files[s32_File].u16_ListIndex > static_cast<uint16_t>(s32_File)) //probably missing file ?
      {
         orc_ErrorText = orc_Files[s32_File].c_FileName + ":\n" +
                         stw::opensyde_core::C_OscUtils::h_LoadString(STR_DS_ERR_PROJECT_INVALID) +
                         "\nGap in list indexes (probably missing .ram file)";
         if (s32_File != 0)
         {
            orc_ErrorText += "\nbetween list files\n\"" + orc_Files[s32_File - 1].c_FileName + "\" and\n\"" +
                             orc_Files[s32_File].c_FileName + "\"";
         }
         else
         {
            orc_ErrorText += ".\nFirst list \n" + orc_Files[s32_File].c_FileName + " does not have index 0";
         }
         return C_CONFIG;
      }
      else
      {
         //nothing to do ...
      }
   }
   return C_NO_ERR;
}

//**************************************************************.FA*
//   .FUNCTION [ C_KFXDEFProject::FindRelatedFiles ]
//   .GROUP [ C_KFXDEFProject private functions ]
//--------------------------------------------------------------.F1-----------------------------------------------------
//   .DESCRIPTION
//    Get a list of all .ram files in the work directory which refer to the specified device
//----------------------------------------------------------------------------------------------------------------------
//   .PARAMETERS
//    Name             Comment
//   orc_DeviceName    Device name to check for
//----------------------------------------------------------------------------------------------------------------------
//   .RETURNVALUE
//**************************************************************.FE*
int32_t C_KFXDEFProject::m_FindRelatedFiles(const C_SclString & orc_Directory, const C_SclString & orc_DeviceName,
                                            QList<C_KFXRAMNameIndex> & orc_Files,
                                            QList<C_SclString> & orc_Warnings)
{
   C_SclString c_Device;
   C_SclString c_Dir;
   QSettings * pc_IniFile;
   
   orc_Files.resize(0);

   c_Dir = stw::opensyde_core::C_OscUtils::h_IncludeTrailingDelimiter(orc_Directory.ToQString());
   
   QDir c_QDir(QString(c_Dir.c_str()));
   QStringList c_Filter;
   c_Filter << "*." + QString(macnc_RAM_FILE_EXTENSION);
   
   QFileInfoList c_InfoList = c_QDir.entryInfoList(c_Filter, QDir::Files | QDir::NoDotAndDotDot);
   
   for (int s32_Index = 0; s32_Index < c_InfoList.count(); s32_Index++)
   {
      C_SclString c_FileName = c_InfoList.at(s32_Index).fileName().toStdString().c_str();

      pc_IniFile = new QSettings((c_Dir + c_FileName).ToQString(), QSettings::IniFormat);
      //first check device match
      c_Device = C_SclString(pc_IniFile->value("CONFIG/DEVICE", "").toString().toStdString());
      if (c_Device == orc_DeviceName)
      {
         //Collect the available RAM-Path,Name and listindexes
         orc_Files.resize(orc_Files.size() + 1);
         orc_Files[(orc_Files.size() > 0 ? orc_Files.size() - 1 : 0)].c_FileName  = (c_Dir + c_FileName);
         orc_Files[(orc_Files.size() > 0 ? orc_Files.size() - 1 : 0)].u16_ListIndex = static_cast<uint16_t>(pc_IniFile->value("CONFIG/LISTINDEX", 0xFFFFU).toUInt());
      }
      delete pc_IniFile;
   }
   return C_NO_ERR;
}

//**************************************************************.FA*
//   .FUNCTION [ C_KFXDEFProject::LoadComments ]
//   .GROUP [ C_KFXDEFProject public functions ]
//--------------------------------------------------------------.F1-----------------------------------------------------
//   .DESCRIPTION
//   Load comments from specified file.
//   Comments for variables that are not part of the project will be ignored.
//----------------------------------------------------------------------------------------------------------------------
//   .PARAMETERS
//    Bezeichner             Bedeutung
//    orc_FileName           Filename of .rec file
//----------------------------------------------------------------------------------------------------------------------
//   .RETURNVALUE
//   C_NO_ERR   -> no errors
//   C_NOACT    -> file does not exist
//   C_OVERFLOW -> file contains more than 5 languages
//   C_CONFIG   -> file does not refer to oc_DeviceName
//   C_RD_WR    -> invalid language name in .rec file
//**************************************************************.FE*
int32_t C_KFXDEFProject::LoadComments(const C_SclString & orc_FileName, const C_SclString & orc_DeviceName,
                                      C_KFXVariableLists & orc_VariableLists,
                                      C_SclString * const opc_CommentDescriptions)
{
   C_SclString c_Device;
   C_SclString c_Directive;
   QString c_Section;
   C_SclString c_Text;
   C_SclString c_ListName;
   C_SclString c_VariableName;
   QSettings * pc_IniFile;

   bool q_Found;
   int32_t i;
   uint32_t j;
   int32_t s32_List;
   int32_t k;
   int32_t l;
   uint16_t u16_NumLanguages;

   if ((QFileInfo(orc_FileName.ToQString()).exists() && QFileInfo(orc_FileName.ToQString()).isFile()) == false)
   {
      return C_NOACT;
   }

   //clear all existing comments
   for (s32_List = 0; s32_List < orc_VariableLists.size(); s32_List++)
   {
      for (k = 0; k < orc_VariableLists[s32_List].VariableList.size(); k++)
      {
         for (i = 0; i < KFX_DATA_MAX_NUM_LANGUAGES; i++)
         {
            orc_VariableLists[s32_List].VariableList[k].ac_Comments[i] = "";
         }
      }
   }

   for (i = 0; i < KFX_DATA_MAX_NUM_LANGUAGES; i++)
   {
      opc_CommentDescriptions[i] = "";
   }

   pc_IniFile = new QSettings(orc_FileName.ToQString(), QSettings::IniFormat);
   c_Device = C_SclString(pc_IniFile->value("CONFIG/DEVICE", "").toString().toStdString());
   if (c_Device != orc_DeviceName)
   {
      delete pc_IniFile;
      return C_CONFIG;
   }

   u16_NumLanguages = static_cast<uint16_t>(pc_IniFile->value("CONFIG/NUMOFLANG", 0).toUInt());
   if (u16_NumLanguages > KFX_DATA_MAX_NUM_LANGUAGES)
   {
      delete pc_IniFile;
      return C_OVERFLOW;
   }

   for (i = 0; i < u16_NumLanguages; i++)
   {
      c_Directive = "LANGNAME" + QString::number(i + 1);
      c_Section = pc_IniFile->value("CONFIG/" + c_Directive.ToQString(), "").toString();
      if ((c_Section == "") || (pc_IniFile->childGroups().contains(c_Section) == false && pc_IniFile->value(c_Section + "/DUMMY").isValid() == false)) // Hacky check for section existence
      {
         // QSettings doesn't support checking for empty groups easily without iterating.
         // But if we assume well formed ini...
         // Actually, let's just proceed. If section doesn't exist, we just won't find values.
         // But the logic below relies on c_Section being valid key.
         // Re-reading code: it reads LANGNAMEx to get the SECTION name for that language.
         // Then it reads parameters from that SECTION.
      }
      // Simplified:
      if (c_Section == "")
      {
          delete pc_IniFile;
          return C_RD_WR;
      }


      opc_CommentDescriptions[i] = c_Section;
      pc_IniFile->beginGroup(c_Section);
      QStringList c_Keys = pc_IniFile->childKeys();
      for (const QString & c_KeyQS : c_Keys)
      {
         c_Directive = c_KeyQS.toStdString();
         c_Text = C_SclString(pc_IniFile->value(c_KeyQS).toString().toStdString());
         // c_Text is the comment value

         c_ListName = c_Directive;
         c_ListName.Delete(c_ListName.Pos("."), c_ListName.Length());
         c_VariableName = c_Directive;
         c_VariableName.Delete(1, c_VariableName.Pos("."));

         //find the list + variable index
         q_Found = false;
         for (k = 0; k < orc_VariableLists.size(); k++)
         {
            if (orc_VariableLists[k].c_ListName.toUpper() == c_ListName.UpperCase().ToQString())
            {
               q_Found = true;
               break;
            }
         }
         if (q_Found == true)
         {
            q_Found = false;
            for (l = 0; l < orc_VariableLists[k].VariableList.size(); l++)
            {
               if (orc_VariableLists[k].VariableList[l].c_Name.toUpper() == c_VariableName.UpperCase().ToQString())
               {
                  q_Found = true;
                  break;
               }
            }
            if (q_Found == true)
            {
               orc_VariableLists[k].VariableList[l].ac_Comments[i] = c_Text.ToQString();
            }
         }
      }

      pc_IniFile->endGroup();
   }
   delete pc_IniFile;
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief    Load default set names from INI file

   Load names of default sets from opened INI file opc_IniFile.
   The names will be placed in orc_VariableLists.ac_DefaultNames.
   The default names are expected in the section "DEFAULT_SETS".

   \param[in]     opc_IniFile       opened INI file
   \param[out]    orc_VariableLists variable lists instance to place the names into
*/
//----------------------------------------------------------------------------------------------------------------------
void C_KFXDEFProject::LoadDefaultNames(QSettings * const opc_IniFile,
                                       C_KFXVariableLists & orc_VariableLists)
{
   int32_t i;

   orc_VariableLists.ac_DefaultNames.resize(static_cast<uint16_t>(opc_IniFile->value("DEFAULT_SETS/DEFAULT_NAMES", 0).toUInt()));
   for (i = 0; i < orc_VariableLists.ac_DefaultNames.size(); i++)
   {
      orc_VariableLists.ac_DefaultNames[i] =
         C_SclString(opc_IniFile->value("DEFAULT_SETS/NAMEDEFAULT" + QString::number(i),
                                 "DEFAULT_" + QString::number(i)).toString().toStdString()).ToQString();
   }
}

//----------------------------------------------------------------------------------------------------------------------
