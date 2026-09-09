//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System view node user settings (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_USSYSTEMVIEWNODE_HPP
#define C_USSYSTEMVIEWNODE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QVector>
#include <QMap>
#include <cstdint>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_UsSystemViewNode
{
public:
   C_UsSystemViewNode(void);

   const QVector<bool> & GetSectionsExpanded(void) const;
   void SetSectionsExpanded(const QVector<bool> & orc_SectionsExpanded);

   const QMap<uint32_t, double> & GetUpdateDataRateHistory() const;
   void AddUpdateDataRate(const uint32_t ou32_Checksum, const double of64_Value);

private:
   QVector<bool> mc_SectionsExpanded;
   QMap<uint32_t, double > mc_UpdateDataRateHistory;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
