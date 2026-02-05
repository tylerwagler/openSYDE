#ifndef CKFXCOMMCONFIGURATIONHPP
#define CKFXCOMMCONFIGURATIONHPP
//---------------------------------------------------------------------------

#include "stwtypes.hpp"
#include <QList>
#include <QSettings>
#include <QString>
#include "DiagLib_config.hpp"

//---------------------------------------------------------------------------

namespace stw
{
namespace diag_lib
{
//---------------------------------------------------------------------------

///holds one communication protocol parameter
class C_KFXCommParameter
{
public:
   QString c_Name;         ///< name of parameter
   int64_t s64_MinValue;                 ///< valid minimum value
   int64_t s64_MaxValue;                 ///< valid maximum value
   int64_t s64_DefaultValue;             ///< default value
   QString c_DisplayName;  ///< text to display in configuration dialog
   QString c_Comment;      ///< can for example be used for "hint"
   QString c_INIDirective; ///< directive for storing parameter value in an INI file

   C_KFXCommParameter(void);
};

//---------------------------------------------------------------------------

///engine for handling communication protocol specific parameters
class C_KFXCommConfigurationBase
{
public:
   C_KFXCommConfigurationBase(void);
   C_KFXCommConfigurationBase(const C_KFXCommConfigurationBase & orc_Source);
   C_KFXCommConfigurationBase(const QList<C_KFXCommParameter> & orc_Parameters);
   virtual ~C_KFXCommConfigurationBase(void);

   C_KFXCommConfigurationBase & operator =(const C_KFXCommConfigurationBase & orc_Source);

   QList<C_KFXCommParameter> mc_Parameters;
   QList<int64_t> mc_Values;
   QString mc_ProtocolName;

   void SetParameterList(const QList<C_KFXCommParameter> & orc_Parameters);
   int32_t GetParameterValue(const QString & orc_ParameterName, int64_t & ors64_Value) const;
   int32_t SetParameterValue(const QString & orc_ParameterName, const int64_t os64_Value);
   void SetAllDefaults(void);

   int32_t LoadConfigFromINI(QSettings & orc_File, const QString & orc_Section);
   int32_t SaveConfigToINI(QSettings & orc_File, const QString & orc_Section) const;
};

//---------------------------------------------------------------------------
///higher-level class (more comfortable functions for KEFEX protocol)
class C_KFXCommConfiguration :
   public C_KFXCommConfigurationBase
{
private:
public:
   C_KFXCommConfiguration & operator =(const C_KFXCommConfiguration & orc_Source);

   //getters and setters:
   uint8_t  GetBaseID(void) const;
   uint8_t  GetClientAddress(void) const;
   uint8_t  GetServerAddress(void) const;
   uint16_t GetTimeout(void) const;
   uint8_t  GetSTMin(void) const;
   uint8_t  GetBSMax(void) const;

   void SetBaseID       (const uint8_t ou8_Value);
   void SetClientAddress(const uint8_t ou8_Value);
   void SetServerAddress(const uint8_t ou8_Value);
   void SetTimeout      (const uint16_t ou16_Value);
   void SetSTMin        (const uint8_t ou8_Value);
   void SetBSMax        (const uint8_t ou8_Value);
};

//---------------------------------------------------------------------------

//stay compatible for "old" VisLib applications:
typedef C_KFXCommConfiguration t_KFXProtocolInfo;

//---------------------------------------------------------------------------
}
}
//---------------------------------------------------------------------------

#endif
