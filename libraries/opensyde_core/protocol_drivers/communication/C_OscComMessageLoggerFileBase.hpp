//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class with generic interface to handle and write a CAN log file (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCOMMESSAGELOGGERFILEBASE_HPP
#define C_OSCCOMMESSAGELOGGERFILEBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>
#include "stwtypes.hpp"
#include "C_OscComMessageLoggerData.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscComMessageLoggerFileBase
{
public:
   C_OscComMessageLoggerFileBase(const std::string & orc_FilePath,
                                 const std::string & orc_ProtocolName);
   virtual ~C_OscComMessageLoggerFileBase(void);

   virtual std::error_code OpenFile(void);
   virtual void AddMessageToFile(const C_OscComMessageLoggerData & orc_MessageData) = 0;

   void SetProtocolName(const std::string & orc_ProtocolName);

protected:
   std::string mc_FilePath;
   std::string mc_ProtocolName;

private:
   //Avoid call
   C_OscComMessageLoggerFileBase(const C_OscComMessageLoggerFileBase &);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
