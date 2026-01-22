//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle user project data

   \class       stw::opensyde_core::C_OscProject
   \brief       Handle user project data

   This class handles all actions concerning the user project.
   (See .cpp file for full description)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPROJECT_HPP
#define C_OSCPROJECT_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QDateTime>
#include "stwtypes.hpp"
#include <QString>
#include "C_OscXmlParser.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscProject
{
public:
   C_OscProject(void);
   virtual ~C_OscProject(void);

   void CalcHash(uint32_t & oru32_HashValue) const;

   static QString h_GetTimeFormatted(const QDateTime & orc_Time);
   static QDateTime h_GetTimeOfString(const QString & orc_Str);

   QString c_Author;             ///< Author (person who created this project)
   QString c_Editor;             ///< Editor (person who last modified this project)
   QDateTime c_CreationTime;                   ///< Time of creation
   QDateTime c_ModificationTime;               ///< Time of last known modification
   QString c_OpenSydeVersion;    ///< openSYDE version used to save this project
   QString c_Template;           ///< Template this project is based on
   QString c_Version;            ///< project version
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
