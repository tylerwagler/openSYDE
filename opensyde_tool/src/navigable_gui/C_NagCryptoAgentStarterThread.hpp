//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Simple thread to run crypt agent start in parallel
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_NAGCRYPTOAGENTSTARTERTHREAD_HPP
#define C_NAGCRYPTOAGENTSTARTERTHREAD_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QThread>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_NagCryptoAgentStarterThread :
   public QThread
{
public:
   C_NagCryptoAgentStarterThread();

protected:
   void run(void) override;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
