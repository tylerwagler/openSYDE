//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Storage container for all CAN message types UI part (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_PUISDNODECANMESSAGECONTAINER_HPP
#define C_PUISDNODECANMESSAGECONTAINER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QList>
#include "C_PuiSdNodeCanMessage.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_PuiSdNodeCanMessageContainer
{
public:
   C_PuiSdNodeCanMessageContainer(void);

   void CalcHash(uint32_t & oru32_HashValue) const;
   const QList<C_PuiSdNodeCanMessage> & GetMessagesConst(const bool & orq_IsTx) const;
   QList<C_PuiSdNodeCanMessage> & GetMessages(const bool & orq_IsTx);

   QList<C_PuiSdNodeCanMessage> c_TxMessages;
   QList<C_PuiSdNodeCanMessage> c_RxMessages;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
