//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Specific tabbar implementation for drag functionality of C_SyvDaDashboardSelectorTabWidget

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QMouseEvent>
#include <QCursor>

#include "stwtypes.hpp"
#include "C_SyvDaDashboardSelectorTabBar.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in,out]  opc_Parent  Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvDaDashboardSelectorTabBar::C_SyvDaDashboardSelectorTabBar(QWidget * const opc_Parent) :
   QTabBar(opc_Parent),
   mq_MouseClicked(false),
   mc_LastMouseClickPosition(0, 0),
   mq_TearOff(false)
{
   this->setDrawBase(false);
   //This property holds when expanding is true QTabBar will expand the tabs to use the empty space.
   this->setExpanding(false);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvDaDashboardSelectorTabBar::~C_SyvDaDashboardSelectorTabBar()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Shows the screenshot as cursor

   \param[in] orc_Screenshot   Actual screenshot of the widget
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSelectorTabBar::SetScreenshot(const QPixmap & orc_Screenshot)
{
   this->setCursor(QCursor(orc_Screenshot, 0, 0));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten mouse press event slot

   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSelectorTabBar::mousePressEvent(QMouseEvent * const opc_Event)
{
   const int32_t s32_Index = this->tabAt(opc_Event->pos());

   QTabBar::mousePressEvent(opc_Event);

   this->mq_TearOff = false;

   if (((opc_Event->buttons() == static_cast<int32_t>(Qt::LeftButton)) &&
        (s32_Index >= 0)) && (this->isTabEnabled(s32_Index) == true))
   {
      this->mq_MouseClicked = true;
      this->mc_LastMouseClickPosition = opc_Event->pos();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten mouse release event slot

   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSelectorTabBar::mouseMoveEvent(QMouseEvent * const opc_Event)
{
   if (this->mq_MouseClicked == true)
   {
      const QPoint c_Diff = this->mc_LastMouseClickPosition - opc_Event->pos();

      // check the minimum drag distance
      if ((c_Diff.y() < -50) || (c_Diff.y() > 50))
      {
         if (this->mq_TearOff == false)
         {
            // we need a screenshot as cursor
            Q_EMIT this->SigNeedScreenshot(this->currentIndex());
            this->mq_TearOff = true;
         }
      }
      else
      {
         this->mq_TearOff = false;
      }
   }

   QTabBar::mouseMoveEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten mouse release event slot

   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSelectorTabBar::mouseReleaseEvent(QMouseEvent * const opc_Event)
{
   bool q_RequestSelectionChange = false;

   if (this->mq_MouseClicked == true)
   {
      if (this->mq_TearOff == true)
      {
         q_RequestSelectionChange = true;
         // tear off the tab to an own window
         Q_EMIT this->SigTabTearOff(this->currentIndex(), opc_Event->pos());
         this->mq_TearOff = false;
      }
      this->mq_MouseClicked = false;
   }

   this->setCursor(Qt::ArrowCursor);

   QTabBar::mouseReleaseEvent(opc_Event);
   if (q_RequestSelectionChange == true)
   {
      Q_EMIT this->SigReselect(this->currentIndex());
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten resize event slot

   Here: Q_EMIT this->signal

   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaDashboardSelectorTabBar::resizeEvent(QResizeEvent * const opc_Event)
{
   QTabBar::resizeEvent(opc_Event);
   Q_EMIT this->SigResizeHappened();
}
