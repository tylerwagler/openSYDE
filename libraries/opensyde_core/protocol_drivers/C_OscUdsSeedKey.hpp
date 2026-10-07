//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Seed-to-key calculation for UDS SecurityAccess (0x27)

   The algorithm behind SecurityAccess is the server vendor's secret; there is no standard one. The generic UDS
   driver takes it through this interface so a device can bring its own without the driver knowing it.
   C_OscUdsSeedKeyConstant is the one built in: it answers every seed with the same key, which is what a server
   with no real security (and the openSYDE flashloader in non-secure mode) accepts.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCUDSSEEDKEY_HPP
#define C_OSCUDSSEEDKEY_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <system_error>
#include <vector>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///interface: turn a SecurityAccess seed into the key the server expects
class C_OscUdsSeedKey
{
public:
   virtual ~C_OscUdsSeedKey(void);

   //----------------------------------------------------------------------------------------------------------------
   /*! \brief   Calculate the key for a seed

      \param[in]   ou8_Level   security level the seed was requested for (the odd sub-function, e.g. 0x01)
      \param[in]   orc_Seed    seed as the server sent it
      \param[out]  orc_Key     key to send back

      \return
      Errc::success   key calculated
      anything else   no key for this level or seed; the driver reports the SecurityAccess as failed
   */
   //----------------------------------------------------------------------------------------------------------------
   [[nodiscard]] virtual std::error_code CalculateKey(const uint8_t ou8_Level, const std::vector<uint8_t> & orc_Seed,
                                                      std::vector<uint8_t> & orc_Key) const = 0;
};

///the same key for every seed
class C_OscUdsSeedKeyConstant :
   public C_OscUdsSeedKey
{
public:
   explicit C_OscUdsSeedKeyConstant(const std::vector<uint8_t> & orc_Key = {0x00U, 0x00U, 0x00U, 0x17U});

   [[nodiscard]] std::error_code CalculateKey(const uint8_t ou8_Level, const std::vector<uint8_t> & orc_Seed,
                                              std::vector<uint8_t> & orc_Key) const override;

private:
   std::vector<uint8_t> mc_Key;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
