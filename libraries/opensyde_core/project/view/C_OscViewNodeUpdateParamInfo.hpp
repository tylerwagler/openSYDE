//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update information for parameter sets (header)

   See cpp file for detailed description

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCVIEWNODEUPDATEPARAMINFO_HPP
#define C_OSCVIEWNODEUPDATEPARAMINFO_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscViewNodeUpdateParamInfo
{
public:
   C_OscViewNodeUpdateParamInfo(void);

   void CalcHash(uint32_t & oru32_HashValue) const;

   //Set
   void SetContent(const std::string & orc_FilePath, const uint32_t ou32_LastKnownCrc);

   //Get
   const std::string & GetPath(void) const;
   uint32_t GetLastKnownCrc(void) const;

private:
   std::string mc_FilePath;
   uint32_t mu32_LastKnownCrc;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
