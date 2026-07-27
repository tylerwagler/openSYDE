//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for toolbox of system definition

   Offers devices in different lists (C_SdTopologyListWidget) as icons.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"
#include <QBoxLayout>

#include "constants.hpp"
#include "C_SdTopologyToolbox.hpp"
#include "ui_C_SdTopologyToolbox.h"
#include "C_SebToolboxUtil.hpp"
#include "C_OscSystemDefinition.hpp"

#include "stwtypes.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdTopologyToolbox::C_SdTopologyToolbox(QWidget * const opc_Parent) :
   QWidget(opc_Parent),
   mpc_Ui(new Ui::C_SdTopologyToolbox),
   mpc_List(NULL)
{
   this->mpc_Ui->setupUi(this);
   this->mpc_Ui->pc_ScrollAreaSearch->setVisible(false);
   this->mpc_Ui->pc_BlankWidgetSearch->setVisible(false);
   this->mpc_Ui->pc_BlankWidgetSearch->SetBackgroundColor(12);
   this->mpc_Ui->pc_BlankTopWidgetSearch->setVisible(false);
   this->mpc_Ui->pc_BlankTopWidgetSearch->SetBackgroundColor(12);
   this->mpc_Ui->pc_LabelSearchNotFound->setVisible(false);
   this->mpc_Ui->pc_LabelSearchNotFound->setText("No results found");
   this->mpc_Ui->pc_LabelSearchNotFound->setStyleSheet("background-color: rgb(250, 250, 250);");

   this->mpc_Ui->pc_ScrollAreaNodesWidget->SetBackgroundColor(-1);
   this->mpc_Ui->pc_ScrollAreaBusesWidget->SetBackgroundColor(-1);
   this->mpc_Ui->pc_ScrollAreaDrawingWidget->SetBackgroundColor(-1);
   this->mpc_Ui->pc_ScrollAreaSearchWidget->SetBackgroundColor(-1);
   this->mpc_Ui->pc_ScrollAreaSearch->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

   this->InitStaticNames();
   this->m_FillToolboxStatic();
   this->m_FillToolboxDynamic();

   // set the 'nodes' tab initially
   this->mpc_Ui->pc_TabWidget->setCurrentIndex(0);

   // The search list shall use the whole free area of the widget
   this->mpc_Ui->pc_ListWidgetSearch->SetMaximumHeightAdaption(false);

   this->mpc_Ui->pc_ListWidgetDrawing->setDropIndicatorShown(false);
   this->mpc_Ui->pc_ListWidgetSearch->setDropIndicatorShown(false);

   this->mpc_Ui->pc_VerticalLayout1->setDirection(QBoxLayout::TopToBottom);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
//lint -e{1540}  no memory leak because of the parent all elements and the Qt memory management

C_SdTopologyToolbox::~C_SdTopologyToolbox()
{
   delete mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot for a search entry change

   If the search text is empty, the search was finished.
   If the search text is not empty, the search result has to be updated.

   \param[in] orc_Text   Updated search text
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdTopologyToolbox::SearchChanged(const QString & orc_Text)
{
   this->mpc_Ui->pc_ListWidgetSearch->clear();
   QList<QListWidgetItem *> c_ListItems;
   QList<QListWidgetItem *>::iterator c_ItItem;
   bool q_NoItemsFound = true;

   if (orc_Text == "")
   {
      // no active search
      this->mpc_Ui->pc_BlankWidgetSearch->setVisible(false);
      this->mpc_Ui->pc_BlankTopWidgetSearch->setVisible(false);
      this->mpc_Ui->pc_LabelSearchNotFound->setVisible(false);
      this->mpc_Ui->pc_ScrollAreaSearch->setVisible(false);
      this->mpc_Ui->pc_TabWidget->setVisible(true);
      this->mpc_Ui->pc_ListWidgetSearch->SetMinimumSize(true);
   }
   else
   {
      this->mpc_Ui->pc_TabWidget->setVisible(false);
      this->mpc_Ui->pc_BlankWidgetSearch->setVisible(true);
      this->mpc_Ui->pc_BlankTopWidgetSearch->setVisible(true);
      this->mpc_Ui->pc_ScrollAreaSearch->setVisible(true);

      if (this->mpc_Ui->pc_ListWidgetSearch->GetMinimumSize() == true)
      {
         this->mpc_Ui->pc_ListWidgetSearch->SetMinimumSize(false);
         this->mpc_Ui->pc_ListWidgetSearch->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      }
   }

   for (int32_t s32_ItListWidget = 0; s32_ItListWidget < mc_ListWidgets.size(); ++s32_ItListWidget)
   {
      c_ListItems = mc_ListWidgets[s32_ItListWidget]->findItems(orc_Text, Qt::MatchContains);
      for (c_ItItem = c_ListItems.begin(); c_ItItem != c_ListItems.end(); ++c_ItItem)
      {
         this->mpc_Ui->pc_ListWidgetSearch->addItem((*c_ItItem)->clone());
         q_NoItemsFound = false;
      }
   }

   c_ListItems = this->mpc_Ui->pc_ListWidgetDrawing->findItems(orc_Text, Qt::MatchContains);
   for (c_ItItem = c_ListItems.begin(); c_ItItem != c_ListItems.end(); ++c_ItItem)
   {
      this->mpc_Ui->pc_ListWidgetSearch->addItem((*c_ItItem)->clone());
      q_NoItemsFound = false;
   }

   this->mpc_Ui->pc_LabelSearchNotFound->setVisible(q_NoItemsFound);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdTopologyToolbox::InitStaticNames(void) const
{
   this->mpc_Ui->pc_TabWidget->tabBar()->setTabText(0, "Nodes");
   this->mpc_Ui->pc_TabWidget->tabBar()->setTabText(1, "Buses");
   this->mpc_Ui->pc_TabWidget->tabBar()->setTabText(2, "Meta");
   this->mpc_Ui->pc_LabelFreeElements->setText("Meta Information Elements");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten enter event slot

   To get the correct cursor back from the resizing implementation of
   C_OgeWiHover.

   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdTopologyToolbox::enterEvent(QEnterEvent * const opc_Event)
{
   Q_UNUSED(opc_Event)

   this->setCursor(Qt::ArrowCursor);
   this->parentWidget()->setMouseTracking(false);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten leave event slot

   To get the correct cursor back from the resizing implementation of
   C_OgeWiHover.

   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdTopologyToolbox::leaveEvent(QEvent * const opc_Event)
{
   Q_UNUSED(opc_Event)

   this->parentWidget()->setMouseTracking(true);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Fill toolbox with dynamic content

   Iterates the device groups produced by the filesystem scan; each group becomes a
   collapsible section in the toolbox. Group names follow the folder hierarchy of the
   scan roots ("/"-separated); the root-level group has an empty name.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdTopologyToolbox::m_FillToolboxDynamic(void)
{
   const std::vector<C_OscDeviceGroup> c_DeviceGroups = C_OscSystemDefinition::hc_Devices.GetDeviceGroups();
   for (uint32_t u32_ItDeviceGroup = 0U; u32_ItDeviceGroup < c_DeviceGroups.size(); ++u32_ItDeviceGroup)
   {
      const std::vector<C_OscDeviceDefinition> & rc_Devices = c_DeviceGroups[u32_ItDeviceGroup].GetDevices();

      this->mpc_List = C_SebToolboxUtil::h_AddNewList(
         c_DeviceGroups[u32_ItDeviceGroup].GetGroupName().c_str(),
         this->mpc_Ui->pc_VerticalLayout1, this->mc_ListWidgets, this);

      if (this->mpc_List != NULL)
      {
         for (uint32_t u32_ItDevice = 0U; u32_ItDevice < rc_Devices.size(); ++u32_ItDevice)
         {
            if (rc_Devices[u32_ItDevice].c_ManufacturerDisplayValue != "Sensor-Technik Wiedemann GmbH")
            {
               this->mc_Icon.addPixmap(
                  static_cast<QPixmap>(rc_Devices[u32_ItDevice].c_ToolboxIcon.c_str()), QIcon::Normal);
               this->mc_Icon.addPixmap(
                  static_cast<QPixmap>(rc_Devices[u32_ItDevice].c_ToolboxIcon.c_str()), QIcon::Selected);
            }
            else
            {
               this->mc_Icon.addPixmap(static_cast<QPixmap>("://images/system_definition/PreviewNode.svg"),
                                       QIcon::Normal);
               this->mc_Icon.addPixmap(static_cast<QPixmap>("://images/system_definition/PreviewNode.svg"),
                                       QIcon::Selected);
            }
            this->m_FillToolboxWithDynamicNodes(rc_Devices[u32_ItDevice]);
         }
      }
   }

   // Add final spacer
   C_SebToolboxUtil::h_AddFinalSpacer(this->mpc_Ui->pc_VerticalLayout1, this->mpc_List);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Fill toolbox with static content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdTopologyToolbox::m_FillToolboxStatic(void)
{
   QListWidget * pc_List;

   // fill draw lists with items
   pc_List = C_SebToolboxUtil::h_AddNewList("Bus Types", this->mpc_Ui->pc_VerticalLayout1_3, this->mc_ListWidgets,
                                            this);

   C_SebToolboxUtil::h_AddElementToList(pc_List, "CAN Bus", "://images/system_definition/IconBus.svg", "",
                                        "CAN Bus");
   C_SebToolboxUtil::h_AddElementToList(pc_List, "Ethernet Bus", "://images/system_definition/IconBus.svg", "",
                                        "Ethernet Bus");
   C_SebToolboxUtil::h_AddFinalSpacer(this->mpc_Ui->pc_VerticalLayout1_3, pc_List);

   C_SebToolboxUtil::h_InitFreeElements(this->mpc_Ui->pc_ListWidgetDrawing);

   // configure Headings
   this->mpc_Ui->pc_LabelFreeElements->setMaximumHeight(C_SebToolboxUtil::hs32_LABEL_SIZE);
   this->mpc_Ui->pc_LabelFreeElements->setMinimumHeight(C_SebToolboxUtil::hs32_LABEL_SIZE);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add a device to toolbox
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdTopologyToolbox::m_FillToolboxWithDynamicNodes(const C_OscDeviceDefinition & orc_Device)
{
   const QString c_DeviceName = orc_Device.c_DeviceName.c_str();
   // Tooltip
   const QString c_DeviceDescription = static_cast<QString>(orc_Device.c_DeviceDescription.c_str());

   if (this->mpc_List != NULL)
   {
      QListWidgetItem * pc_Item;
      this->mpc_List->addItem(orc_Device.GetDisplayName().c_str());
      pc_Item = this->mpc_List->item(this->mpc_List->count() - 1);
      pc_Item->setData(ms32_USER_ROLE_ADDITIONAL_INFORMATION, c_DeviceName);
      // Toolbox icon
      pc_Item->setIcon(this->mc_Icon);
      // Tooltip
      pc_Item->setData(ms32_USER_ROLE_TOOL_TIP_HEADING, orc_Device.GetDisplayName().c_str());
      pc_Item->setData(ms32_USER_ROLE_TOOL_TIP_CONTENT, c_DeviceDescription);
   }
}
