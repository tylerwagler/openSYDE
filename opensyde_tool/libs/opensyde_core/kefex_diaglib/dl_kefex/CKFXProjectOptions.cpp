#include "precomp_headers.hpp" //pre-compiled headers

#include "DiagLib_config.hpp" //diaglib configuration

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "CKFXProjectOptions.hpp"


//--------------------------------------------------------------

using namespace stw::errors;
using namespace stw::scl;
using namespace stw::diag_lib;

//--------------------------------------------------------------

C_KFXProjectMetaInfo::C_KFXProjectMetaInfo(void)
{
}

//--------------------------------------------------------------

C_KFXProjectMetaInfo::~C_KFXProjectMetaInfo(void)
{
}

//--------------------------------------------------------------

C_KFXProjectMetaInfo::C_KFXProjectMetaInfo(const C_KFXProjectMetaInfo & orc_Source)
{
   (*this) = orc_Source;
}

//--------------------------------------------------------------

C_KFXProjectMetaInfo & C_KFXProjectMetaInfo::operator =(const C_KFXProjectMetaInfo & orc_Source)
{
   if (&orc_Source != this)
   {
      this->c_Text = orc_Source.c_Text;
   }
   return (*this);
}

//----------------------------------------------------------------------------------------------------------------------

void C_KFXProjectMetaInfo::LoadFromINI(QSettings & orc_IniFile, const C_SclString & orc_Section)
{
   C_SclString c_Temp;
   int32_t s32_Count = 0;
   QString c_Section = orc_Section.ToQString();

   c_Text.clear();
   c_Temp = C_SclString(orc_IniFile.value(c_Section + "/Line_" + QString::number(s32_Count),
                             "").toString().toStdString());

   while (c_Temp.Length() > 0)
   {
      c_Text.append(c_Temp.ToQString());
      s32_Count++;
      c_Temp = C_SclString(orc_IniFile.value(c_Section + "/Line_" + QString::number(s32_Count),
                                "").toString().toStdString());
   }
}

//--------------------------------------------------------------

bool C_KFXProjectMetaInfo::operator !=(C_KFXProjectMetaInfo & orc_Source)
{
   return (this->c_Text != orc_Source.c_Text);
}

//----------------------------------------------------------------------------------------------------------------------

C_KFXProjectOptions::C_KFXProjectOptions(void) :
   u16_MaxDesignatorLength(32U),
   u16_ProjectIndex(1U),
   u16_ListOffset(0U),
   u16_DataVersion(1U),
   c_DeviceName("Generic Node")
{
   ac_CommentDescriptions[0] = "";
   ac_CommentDescriptions[1] = "";
   ac_CommentDescriptions[2] = "";

   for (int32_t s32_Loop = 0; s32_Loop < KFX_NUM_ACCESS_GROUPS; s32_Loop++)
   {
      aq_UserMayWriteKDX[s32_Loop] = true;
   }
}

//----------------------------------------------------------------------------------------------------------------------

bool C_KFXProjectOptions::operator !=(C_KFXProjectOptions & orc_Source)
{
   //ac_CommentDescriptions are not handled by this class
   bool q_Result = false;
   if ((u16_MaxDesignatorLength != orc_Source.u16_MaxDesignatorLength) ||
       (u16_ProjectIndex != orc_Source.u16_ProjectIndex) ||
       (u16_ListOffset != orc_Source.u16_ListOffset) ||
       (u16_DataVersion != orc_Source.u16_DataVersion) ||
       (c_DeviceName != orc_Source.c_DeviceName) ||
       (c_MetaInfo != orc_Source.c_MetaInfo))
   {
      q_Result = true;
   }
   else
   {
      for (int32_t i = 0; i < KFX_NUM_ACCESS_GROUPS; i++)
      {
         if (aq_UserMayWriteKDX[i] != orc_Source.aq_UserMayWriteKDX[i])
         {
            q_Result = true;
            break;
         }
      }
   }
   return q_Result;
}

//--------------------------------------------------------------
//--------------------------------------------------------------

void C_KFXProjectOptions::LoadConfigFromIni(QSettings & orc_IniFile)
{
   uint8_t u8_Index;

   u16_MaxDesignatorLength  = static_cast<uint16_t>(orc_IniFile.value("EDIT/DESIGNATOR_MAX_LENGTH", 200U).toUInt());
   c_DeviceName             = stw::scl::C_SclString::FromQString(orc_IniFile.value("CONFIG/DEVICENAME", "").toString());
   u16_DataVersion          = static_cast<uint16_t>(orc_IniFile.value("CONFIG/RAMDATAVERSION", 0U).toUInt());
   u16_ListOffset           = static_cast<uint16_t>(orc_IniFile.value("CONFIG/LISTOFFSET", 0U).toUInt());
   u16_ProjectIndex         = static_cast<uint16_t>(orc_IniFile.value("CONFIG/PROJECTINDEX", 0U).toUInt());
   for (u8_Index = 0U; u8_Index < KFX_NUM_ACCESS_GROUPS; u8_Index++)
   {
      aq_UserMayWriteKDX[u8_Index] = orc_IniFile.value("CONFIG/KDX_WRITE_PERMITTED" + QString::number(u8_Index),
                                                       false).toBool();
   }
   c_MetaInfo.LoadFromINI(orc_IniFile, "META_DATA");
}

//--------------------------------------------------------------
