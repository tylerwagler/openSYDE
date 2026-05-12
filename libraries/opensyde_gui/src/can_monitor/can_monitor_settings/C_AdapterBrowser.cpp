//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       CAN adapter picker (implementation)

   Backend dropdown (one entry per libcan backend compiled in) drives an adapter dropdown
   populated from can::ICanBackend::enumerateAdapters(); a bitrate dropdown carries common
   classic-CAN rates. Selection changes emit SigConfigChanged with the openSYDE-side
   C_OscCanAdapterConfig so consumers (CAN Monitor and SYDE Flash settings widgets) can
   save without polling.

   Visually matches the other settings subsections (Database / Receive Filter / Logging): a
   C_CamOgeWiSettingSubSection header with title + icon + expand chevron, sitting on the same
   dark background as its siblings, and a content area below that hides/shows in response to
   the header's expand toggle. Adapter info is rendered as a flat details panel — no white
   inner card that would break the visual rhythm.

   Adapted from Elytron Defense's Qt_Template/AdapterBrowser. License-compatible (both GPL-3).

   \copyright   Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "C_AdapterBrowser.hpp"
#include "C_CamOgeCbxDark.hpp"
#include "C_CamOgeWiSettingSubSection.hpp"
#include "C_GtGetText.hpp"
#include "C_OgeLabGenericNoPaddingNoMargins.hpp"
#include "C_OgePubToolTipBase.hpp"
#include "C_OgeWiUtil.hpp"
#include "C_UsHandler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_elements;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */
namespace
{
QString h_Sanitize(const std::string & orc_Value)
{
   return orc_Value.empty() ? QStringLiteral("—") : QString::fromStdString(orc_Value);
}

QString h_FormatAdapter(const ::can::AdapterInfo & orc_Info)
{
   QString c_Html = QStringLiteral("<table cellpadding='4' style='color:white;'>");
   const auto c_Row = [&](const char * const opcn_Key, const QString & orc_Value) {
      c_Html += QStringLiteral("<tr><td><b>%1</b></td><td><tt>%2</tt></td></tr>")
                .arg(QString::fromUtf8(opcn_Key), orc_Value.toHtmlEscaped());
   };
   c_Row("backend",          QString::fromStdString(::can::backendKindToString(orc_Info.backend)));
   c_Row("channel_id",       h_Sanitize(orc_Info.channel_id));
   c_Row("device_name",      h_Sanitize(orc_Info.device_name));
   c_Row("serial_number",    h_Sanitize(orc_Info.serial_number));
   c_Row("firmware_version", h_Sanitize(orc_Info.firmware_version));
   c_Row("driver_version",   h_Sanitize(orc_Info.driver_version));
   c_Row("part_number",      h_Sanitize(orc_Info.hardware_part_number));
   c_Row("channel_index",    QString::number(orc_Info.channel_index));
   for (const auto & rc_Extra : orc_Info.extra)
   {
      c_Row(("extra." + rc_Extra.first).c_str(), QString::fromStdString(rc_Extra.second));
   }
   c_Html += QStringLiteral("</table>");
   return c_Html;
}
} // anonymous

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
C_AdapterBrowser::C_AdapterBrowser(QWidget * const opc_Parent) :
   C_OgeWiOnlyBackground(opc_Parent),
   mpc_Header(NULL),
   mpc_Content(NULL),
   mpc_BackendCombo(NULL),
   mpc_AdapterCombo(NULL),
   mpc_BitrateCombo(NULL),
   mpc_RefreshBtn(NULL),
   mpc_DetailsFrame(NULL),
   mpc_Details(NULL)
{
   this->SetBackgroundColor(5);
   this->m_BuildUi();
   this->m_RefreshAdapters();
}

//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::m_BuildUi(void)
{
   QVBoxLayout * const pc_Outer = new QVBoxLayout(this);
   pc_Outer->setContentsMargins(0, 0, 0, 0);
   pc_Outer->setSpacing(0);

   mpc_Header = new C_CamOgeWiSettingSubSection(this);
   mpc_Header->SetTitle(C_GtGetText::h_GetText("PC CAN Interface Configuration"));
   mpc_Header->SetIcon("://images/IconConfig.svg");
   mpc_Header->SetToggle(false);
   pc_Outer->addWidget(mpc_Header);

   mpc_Content = new QWidget(this);
   QVBoxLayout * const pc_ContentLayout = new QVBoxLayout(mpc_Content);
   pc_ContentLayout->setContentsMargins(10, 6, 10, 10);
   pc_ContentLayout->setSpacing(6);

   mpc_BackendCombo = new C_CamOgeCbxDark(mpc_Content);
   for (auto e_Kind : ::can::ICanBackend::availableBackends())
   {
      mpc_BackendCombo->addItem(QString::fromStdString(::can::backendKindToString(e_Kind)),
                                static_cast<int>(e_Kind));
   }
   if (mpc_BackendCombo->count() == 0)
   {
      mpc_BackendCombo->addItem(tr("(no backends compiled in)"), -1);
      mpc_BackendCombo->setEnabled(false);
   }

   mpc_AdapterCombo = new C_CamOgeCbxDark(mpc_Content);

   mpc_BitrateCombo = new C_CamOgeCbxDark(mpc_Content);
   const struct {const char * pcn_Label; uint32_t u32_Bps;} c_Bitrates[] = {
      {"1 Mbps",   1000000U}, {"800 kbps", 800000U},  {"500 kbps", 500000U},
      {"250 kbps", 250000U},  {"125 kbps", 125000U},  {"100 kbps", 100000U},
      {"50 kbps",  50000U},   {"20 kbps",  20000U},   {"10 kbps",  10000U}
   };
   for (const auto & rc_B : c_Bitrates)
   {
      mpc_BitrateCombo->addItem(QString::fromUtf8(rc_B.pcn_Label), rc_B.u32_Bps);
   }
   mpc_BitrateCombo->setCurrentIndex(2); // 500 kbps default

   mpc_RefreshBtn = new C_OgePubToolTipBase(mpc_Content);
   mpc_RefreshBtn->setText(tr("Refresh"));
   mpc_RefreshBtn->setMinimumHeight(28);
   mpc_RefreshBtn->setMinimumWidth(80);

   const auto c_MakeLabel = [this](const char * const opcn_Text) -> C_OgeLabGenericNoPaddingNoMargins * {
      C_OgeLabGenericNoPaddingNoMargins * const pc_Lab = new C_OgeLabGenericNoPaddingNoMargins(this->mpc_Content);
      pc_Lab->setText(C_GtGetText::h_GetText(opcn_Text));
      pc_Lab->SetForegroundColor(0);
      pc_Lab->SetFontPixel(13);
      return pc_Lab;
   };

   QFormLayout * const pc_Form = new QFormLayout;
   pc_Form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
   pc_Form->setHorizontalSpacing(8);
   pc_Form->setVerticalSpacing(6);
   pc_Form->addRow(c_MakeLabel("Backend"), mpc_BackendCombo);
   pc_Form->addRow(c_MakeLabel("Adapter"), mpc_AdapterCombo);
   pc_Form->addRow(c_MakeLabel("Bitrate"), mpc_BitrateCombo);

   QHBoxLayout * const pc_Row = new QHBoxLayout;
   pc_Row->addLayout(pc_Form, 1);
   pc_Row->addWidget(mpc_RefreshBtn, 0, Qt::AlignTop);
   pc_ContentLayout->addLayout(pc_Row);

   mpc_DetailsFrame = new QFrame(mpc_Content);
   mpc_DetailsFrame->setFrameShape(QFrame::NoFrame);
   QVBoxLayout * const pc_DetailsLayout = new QVBoxLayout(mpc_DetailsFrame);
   pc_DetailsLayout->setContentsMargins(0, 6, 0, 0);
   pc_DetailsLayout->setSpacing(4);

   C_OgeLabGenericNoPaddingNoMargins * const pc_DetailsTitle =
      new C_OgeLabGenericNoPaddingNoMargins(mpc_DetailsFrame);
   pc_DetailsTitle->setText(C_GtGetText::h_GetText("Adapter info"));
   pc_DetailsTitle->SetForegroundColor(0);
   pc_DetailsTitle->SetFontPixel(13, true);
   pc_DetailsLayout->addWidget(pc_DetailsTitle);

   mpc_Details = new QLabel(mpc_DetailsFrame);
   mpc_Details->setTextFormat(Qt::RichText);
   mpc_Details->setTextInteractionFlags(Qt::TextSelectableByMouse);
   mpc_Details->setWordWrap(true);
   mpc_Details->setAlignment(Qt::AlignLeft | Qt::AlignTop);
   mpc_Details->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
   mpc_Details->setStyleSheet(QStringLiteral(
                                 "QLabel { background-color: rgba(0, 0, 0, 0.35); "
                                 "color: white; border: 1px solid rgba(255, 255, 255, 0.08); "
                                 "border-radius: 4px; padding: 6px; }"));
   pc_DetailsLayout->addWidget(mpc_Details);
   pc_ContentLayout->addWidget(mpc_DetailsFrame);

   pc_Outer->addWidget(mpc_Content);

   connect(mpc_Header, &C_CamOgeWiSettingSubSection::SigExpandSection,
           this, &C_AdapterBrowser::m_OnExpand);
   connect(mpc_Header, &C_CamOgeWiSettingSubSection::SigHide,
           this, &C_AdapterBrowser::SigHide);
   connect(mpc_BackendCombo, static_cast<void(C_CamOgeCbxDark::*)(int)>(&C_CamOgeCbxDark::currentIndexChanged),
           this, &C_AdapterBrowser::m_RefreshAdapters);
   connect(mpc_RefreshBtn, &QPushButton::clicked,
           this, &C_AdapterBrowser::m_RefreshAdapters);
   connect(mpc_AdapterCombo, static_cast<void(C_CamOgeCbxDark::*)(int)>(&C_CamOgeCbxDark::currentIndexChanged),
           this, &C_AdapterBrowser::m_DisplaySelected);
   connect(mpc_AdapterCombo, static_cast<void(C_CamOgeCbxDark::*)(int)>(&C_CamOgeCbxDark::currentIndexChanged),
           this, &C_AdapterBrowser::m_EmitChanged);
   connect(mpc_BitrateCombo, static_cast<void(C_CamOgeCbxDark::*)(int)>(&C_CamOgeCbxDark::currentIndexChanged),
           this, &C_AdapterBrowser::m_EmitChanged);
}

//----------------------------------------------------------------------------------------------------------------------
C_OscCanAdapterConfig C_AdapterBrowser::GetAdapterConfig(void) const
{
   C_OscCanAdapterConfig c_Config = C_OscCanAdapterConfig::h_GetPlatformDefault();
   const int32_t s32_KindInt = mpc_BackendCombo->currentData().toInt();
   if (s32_KindInt >= 0)
   {
      c_Config.e_BackendKind = static_cast<::can::BackendKind>(s32_KindInt);
   }
   const int32_t s32_AdapterIdx = mpc_AdapterCombo->currentIndex();
   if ((s32_AdapterIdx >= 0) && (s32_AdapterIdx < static_cast<int32_t>(mc_CurrentAdapters.size())))
   {
      c_Config.c_ChannelId = mc_CurrentAdapters[s32_AdapterIdx].channel_id;
   }
   c_Config.u32_BitrateBps = mpc_BitrateCombo->currentData().toUInt();
   return c_Config;
}

//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::SetAdapterConfig(const C_OscCanAdapterConfig & orc_Config)
{
   const int32_t s32_BackendIdx = mpc_BackendCombo->findData(static_cast<int>(orc_Config.e_BackendKind));
   if (s32_BackendIdx >= 0)
   {
      mpc_BackendCombo->setCurrentIndex(s32_BackendIdx);
   }
   for (int32_t s32_Idx = 0; s32_Idx < static_cast<int32_t>(mc_CurrentAdapters.size()); ++s32_Idx)
   {
      if (mc_CurrentAdapters[s32_Idx].channel_id == orc_Config.c_ChannelId)
      {
         mpc_AdapterCombo->setCurrentIndex(s32_Idx);
         break;
      }
   }
   const int32_t s32_BitrateIdx = mpc_BitrateCombo->findData(orc_Config.u32_BitrateBps);
   if (s32_BitrateIdx >= 0)
   {
      mpc_BitrateCombo->setCurrentIndex(s32_BitrateIdx);
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::m_RefreshAdapters(void)
{
   mpc_AdapterCombo->blockSignals(true);
   mpc_AdapterCombo->clear();
   mc_CurrentAdapters.clear();

   const int32_t s32_KindInt = mpc_BackendCombo->currentData().toInt();
   if (s32_KindInt < 0)
   {
      mpc_Details->setText(tr("<i style='color:white;'>No backends compiled in.</i>"));
      mpc_AdapterCombo->blockSignals(false);
      return;
   }

   const ::can::BackendKind e_Kind = static_cast<::can::BackendKind>(s32_KindInt);
   const std::unique_ptr< ::can::ICanBackend> c_Backend = ::can::ICanBackend::create(e_Kind);
   if (c_Backend == NULL)
   {
      mpc_Details->setText(tr("<i style='color:white;'>Backend factory returned nullptr — check build configuration.</i>"));
      mpc_AdapterCombo->blockSignals(false);
      return;
   }

   mc_CurrentAdapters = c_Backend->enumerateAdapters();
   if (mc_CurrentAdapters.empty() == true)
   {
      mpc_AdapterCombo->addItem(tr("(no adapters found)"));
      mpc_AdapterCombo->setEnabled(false);
      mpc_Details->setText(tr("<i style='color:white;'>No adapters detected for this backend.</i>"));
   }
   else
   {
      mpc_AdapterCombo->setEnabled(true);
      for (const auto & rc_Adapter : mc_CurrentAdapters)
      {
         QString c_Label = QString::fromStdString(rc_Adapter.channel_id);
         if ((rc_Adapter.device_name.empty() == false) && (rc_Adapter.device_name != rc_Adapter.channel_id))
         {
            c_Label += QStringLiteral(" — ") + QString::fromStdString(rc_Adapter.device_name);
         }
         mpc_AdapterCombo->addItem(c_Label);
      }
   }
   mpc_AdapterCombo->blockSignals(false);
   this->m_DisplaySelected();
   this->m_EmitChanged();
}

//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::m_DisplaySelected(void)
{
   const int32_t s32_Idx = mpc_AdapterCombo->currentIndex();
   if ((s32_Idx >= 0) && (s32_Idx < static_cast<int32_t>(mc_CurrentAdapters.size())))
   {
      mpc_Details->setText(h_FormatAdapter(mc_CurrentAdapters[s32_Idx]));
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::m_EmitChanged(void)
{
   Q_EMIT (this->SigConfigChanged(this->GetAdapterConfig()));
}

//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::m_OnExpand(const bool oq_Expand)
{
   mpc_Content->setVisible(oq_Expand);
   C_UsHandler::h_GetInstance()->SetWiDllConfigExpanded(oq_Expand);
}

//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::LoadUserSettings(void) const
{
   const bool q_Expanded = C_UsHandler::h_GetInstance()->GetWiDllConfigExpanded();
   mpc_Header->SetExpanded(q_Expanded);
   mpc_Content->setVisible(q_Expanded);
}

//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::PrepareForExpanded(const bool oq_Expand) const
{
   if (oq_Expand == false)
   {
      mpc_Content->setVisible(true);
   }
   else
   {
      mpc_Content->setVisible(C_UsHandler::h_GetInstance()->GetWiDllConfigExpanded());
   }
   mpc_Header->ShowExpandButton(oq_Expand);
}

//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::OnCommunicationStarted(const bool oq_Online) const
{
   mpc_BackendCombo->setDisabled(oq_Online);
   mpc_AdapterCombo->setDisabled(oq_Online);
   mpc_BitrateCombo->setDisabled(oq_Online);
   mpc_RefreshBtn->setDisabled(oq_Online);
}
