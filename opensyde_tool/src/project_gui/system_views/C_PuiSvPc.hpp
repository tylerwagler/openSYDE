//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System view PC item data element (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_PUISVPC_HPP
#define C_PUISVPC_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include "C_PuiBsBox.hpp"
#include "C_PuiBsLineBase.hpp"
#include "C_OscCanAdapterConfig.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_PuiSvPc :
   public C_PuiBsBox
{
public:
   C_PuiSvPc(void);

   void CalcHash(uint32_t & oru32_HashValue) const override;

   const C_PuiBsLineBase & GetConnectionData(void) const;
   void SetConnectionData(const C_PuiBsLineBase & orc_Value);

   const stw::opensyde_core::C_OscCanAdapterConfig & GetAdapterConfig(void) const;
   void SetAdapterConfig(const stw::opensyde_core::C_OscCanAdapterConfig & orc_Config);

   //Specific setter
   void SetBox(const C_PuiBsBox & orc_Box);

private:
   C_PuiBsLineBase mc_ConnectionData;                          ///< Relevant connection UI data
   stw::opensyde_core::C_OscCanAdapterConfig mc_AdapterConfig; ///< CAN adapter (replaces legacy DLL fields)
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
