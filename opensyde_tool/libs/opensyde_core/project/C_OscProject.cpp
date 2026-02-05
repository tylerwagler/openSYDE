//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle user project data

   This class handles all actions concerning the user project.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"
#include <QDateTime>
#include <QString>
#include <QRegularExpression>

#include "stwtypes.hpp"
#include "stwerrors.hpp"

#include "C_OscProject.hpp"
#include "C_SclChecksums.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

using namespace std;
using namespace stw::scl;
using namespace stw::errors;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor

   Initialize all elements with default values
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProject::C_OscProject(void) :
   c_CreationTime(QDateTime::currentDateTime()),
   c_ModificationTime(QDateTime::currentDateTime()),
   c_OpenSydeVersion("0.01r0"),
   c_Template("Empty project"),
   c_Version("0.01r0b0")
{
   stw::opensyde_core::C_OscUtils::h_GetSystemUserName(c_Author);
   c_Editor = c_Author;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscProject::~C_OscProject(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with initial [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscProject::CalcHash(uint32_t & oru32_HashValue) const
{
   // Using QString methods directly instead of toStdString()
   C_SclChecksums::CalcCRC32(this->c_Editor.toStdString().c_str(), this->c_Editor.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_OpenSydeVersion.toStdString().c_str(), this->c_OpenSydeVersion.length(), oru32_HashValue);
   // no need to check c_Author, c_CreationTime & c_ModificationTime
   C_SclChecksums::CalcCRC32(this->c_Template.toStdString().c_str(), this->c_Template.length(), oru32_HashValue);
   C_SclChecksums::CalcCRC32(this->c_Version.toStdString().c_str(), this->c_Version.length(), oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get modification date and time as formatted string

   Format: "DD.MM.JJJJ HH:MM"

   \param[in,out] orc_Time Time to output

   \return
   Formatted date
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscProject::h_GetTimeFormatted(const QDateTime & orc_Time)
{
   const QString c_StrTime = orc_Time.toString("dd.MM.yy yy HH:mm");
   return c_StrTime;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert string to time

   Expected format:
   - days months years hours minutes as numeric values
   - separated by "." " " or ":" (or any mixture of these)
   - example: "DD.MM.JJJJ HH:MM"

   \param[in] orc_Str String to be evaluated

   \return
   Time (current time of orc_Str is invalid)
*/
//----------------------------------------------------------------------------------------------------------------------
QDateTime C_OscProject::h_GetTimeOfString(const QString & orc_Str)
{
   QDateTime c_Retval;
   bool q_Err = true;

   // Now we work directly with QString since we're migrating to Qt native types
   QStringList c_Dyn = orc_Str.split(QRegularExpression("[. :]+"), Qt::SkipEmptyParts);
   if (c_Dyn.size() == 5)
   {
      try
      {
         const int s32_Day = c_Dyn[0].toInt();
         const int s32_Month = c_Dyn[1].toInt();
         const int s32_Year = c_Dyn[2].toInt();
         const int s32_Hour = c_Dyn[3].toInt();
         const int s32_Minute = c_Dyn[4].toInt();
         
         c_Retval = QDateTime(QDate(s32_Year, s32_Month, s32_Day), QTime(s32_Hour, s32_Minute));
         if (c_Retval.isValid())
         {
            q_Err = false;
         }
      }
      catch (...)
      {
         q_Err = true;
      }
   }
   if (q_Err == true)
   {
      c_Retval = QDateTime::currentDateTime();
   }
   return c_Retval;
}
