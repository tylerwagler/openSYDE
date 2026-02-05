//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Thread for openSYDE communication driver (implementation)

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscLoggingHandler.hpp"
#include "C_SyvComDriverThread.hpp"

using namespace stw::opensyde_gui_logic;

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in]      opr_ThreadFunc   Function pointer to function which the
   thread shall call cyclic
   \param[in]      opv_Instance     Instance pointer to pass back when invoking
   read event callback
   \param[in,out]  opc_Parent       Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvComDriverThread::C_SyvComDriverThread(
    const PR_ThreadFunction opr_ThreadFunc, void *const opv_Instance,
    QObject *const opc_Parent)
    : QThread(opc_Parent), mpr_ThreadFunc(opr_ThreadFunc),
      mpv_FuncInstance(opv_Instance) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Thread function which is executed by the thread
 */
//----------------------------------------------------------------------------------------------------------------------
void C_SyvComDriverThread::run(void) {
  if ((this->mpr_ThreadFunc != NULL) && (this->mpv_FuncInstance != NULL)) {
    while (this->isInterruptionRequested() == false) {
      try {
        this->mpr_ThreadFunc(this->mpv_FuncInstance);
      } catch (const std::exception &orc_Exception) {
        // Log the exception and exit gracefully instead of crashing
        osc_write_log_error("Communication thread",
                            QString("Exception in thread: ") +
                                orc_Exception.what());
        break; // Exit thread loop on exception
      } catch (...) {
        // Catch any other exceptions
        osc_write_log_error("Communication thread",
                            "Unknown exception in thread");
        break; // Exit thread loop on exception
      }
    }
  }
}
