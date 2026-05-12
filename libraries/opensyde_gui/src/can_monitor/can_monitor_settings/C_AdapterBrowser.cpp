//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       CAN adapter picker (implementation)

   Backend dropdown (one entry per libcan backend compiled in) drives an adapter dropdown
   populated from can::ICanBackend::enumerateAdapters(); a bitrate dropdown carries common
   classic-CAN rates. Selection changes emit SigConfigChanged with the openSYDE-side
   C_OscCanAdapterConfig so consumers (CAN Monitor and SYDE Flash settings widgets) can
   save without polling.

   The bottom panel renders the live AdapterInfo for the highlighted adapter — serial, firmware,
   driver version, and any backend-specific extras. Refresh button re-runs enumerateAdapters().

   Adapted from Elytron Defense's Qt_Template/AdapterBrowser. License-compatible (both GPL-3).

   \copyright   Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextDocument>
#include <QVBoxLayout>

#include "C_AdapterBrowser.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui;

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
   QString c_Html = QStringLiteral("<table cellpadding='4'>");
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
   QWidget(opc_Parent),
   mpc_BackendCombo(new QComboBox(this)),
   mpc_AdapterCombo(new QComboBox(this)),
   mpc_BitrateCombo(new QComboBox(this)),
   mpc_RefreshBtn(new QPushButton(tr("Refresh"), this)),
   mpc_Details(new QTextBrowser(this))
{
   const std::vector< ::can::BackendKind> c_Kinds = ::can::ICanBackend::availableBackends();
   for (size_t s_Idx = 0U; s_Idx < c_Kinds.size(); ++s_Idx)
   {
      mpc_BackendCombo->addItem(QString::fromStdString(::can::backendKindToString(c_Kinds[s_Idx])),
                                static_cast<int>(c_Kinds[s_Idx]));
   }
   if (mpc_BackendCombo->count() == 0)
   {
      mpc_BackendCombo->addItem(tr("(no backends compiled in)"), -1);
      mpc_BackendCombo->setEnabled(false);
   }

   // Common classic-CAN bitrates. CAN-FD data-phase rates will land alongside when openSYDE
   // grows FD support; for now arbitration-only.
   const struct {const char * pcn_Label; uint32_t u32_Bps;} c_Bitrates[] = {
      {"1 Mbps",   1000000U}, {"800 kbps", 800000U},  {"500 kbps", 500000U},
      {"250 kbps", 250000U},  {"125 kbps", 125000U},  {"100 kbps", 100000U},
      {"50 kbps",  50000U},   {"20 kbps",  20000U},   {"10 kbps",  10000U}
   };
   for (const auto & rc_B : c_Bitrates)
   {
      mpc_BitrateCombo->addItem(QString::fromUtf8(rc_B.pcn_Label), rc_B.u32_Bps);
   }
   mpc_BitrateCombo->setCurrentIndex(2); // 500 kbps default — matches platform default

   mpc_Details->setOpenExternalLinks(false);

   QFormLayout * const pc_Form = new QFormLayout;
   pc_Form->addRow(tr("Backend:"), mpc_BackendCombo);
   pc_Form->addRow(tr("Adapter:"), mpc_AdapterCombo);
   pc_Form->addRow(tr("Bitrate:"), mpc_BitrateCombo);

   QHBoxLayout * const pc_Row = new QHBoxLayout;
   pc_Row->addLayout(pc_Form);
   pc_Row->addStretch();
   pc_Row->addWidget(mpc_RefreshBtn);

   QGroupBox * const pc_DetailsBox = new QGroupBox(tr("Adapter info"), this);
   QVBoxLayout * const pc_DetailsLayout = new QVBoxLayout(pc_DetailsBox);
   pc_DetailsLayout->addWidget(mpc_Details);

   QVBoxLayout * const pc_MainLayout = new QVBoxLayout(this);
   pc_MainLayout->addLayout(pc_Row);
   pc_MainLayout->addWidget(pc_DetailsBox, 1);

   connect(mpc_BackendCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this, &C_AdapterBrowser::m_RefreshAdapters);
   connect(mpc_RefreshBtn, &QPushButton::clicked,
           this, &C_AdapterBrowser::m_RefreshAdapters);
   connect(mpc_AdapterCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this, &C_AdapterBrowser::m_DisplaySelected);
   connect(mpc_AdapterCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this, &C_AdapterBrowser::m_EmitChanged);
   connect(mpc_BitrateCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this, &C_AdapterBrowser::m_EmitChanged);

   this->m_RefreshAdapters();
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
   // m_RefreshAdapters was triggered by the backend change above; pick the right adapter entry.
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
      mpc_Details->setHtml(tr("<i>No backends compiled in.</i>"));
      mpc_AdapterCombo->blockSignals(false);
      return;
   }

   const ::can::BackendKind e_Kind = static_cast<::can::BackendKind>(s32_KindInt);
   const std::unique_ptr< ::can::ICanBackend> c_Backend = ::can::ICanBackend::create(e_Kind);
   if (c_Backend == NULL)
   {
      mpc_Details->setHtml(tr("<i>Backend factory returned nullptr — check build configuration.</i>"));
      mpc_AdapterCombo->blockSignals(false);
      return;
   }

   mc_CurrentAdapters = c_Backend->enumerateAdapters();
   if (mc_CurrentAdapters.empty() == true)
   {
      mpc_AdapterCombo->addItem(tr("(no adapters found)"));
      mpc_AdapterCombo->setEnabled(false);
      mpc_Details->setHtml(tr("<i>No adapters detected for this backend.</i>"));
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
      mpc_Details->setHtml(h_FormatAdapter(mc_CurrentAdapters[s32_Idx]));
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::m_EmitChanged(void)
{
   Q_EMIT (this->SigConfigChanged(this->GetAdapterConfig()));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Compatibility shim: legacy widget hook for loading persisted state.

   The new browser loads its initial state via SetAdapterConfig() from the parent. This entry point
   stays so existing parents (C_CamMosWidget / C_FlaSetWidget) that call LoadUserSettings() on the
   embedded widget keep compiling. No-op here.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::LoadUserSettings(void) const
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Compatibility shim: legacy widget reacted to settings-panel expand/collapse.

   The new browser is always-visible; there's nothing to toggle. No-op.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::PrepareForExpanded(const bool oq_Expand) const
{
   (void)oq_Expand;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Compatibility shim: disable the picker while a CAN session is active.

   Greys out the combos and the refresh button so the user can't change selection mid-session.
   The legacy widget disabled radio buttons + line edit; here we disable the equivalent controls.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_AdapterBrowser::OnCommunicationStarted(const bool oq_Online) const
{
   mpc_BackendCombo->setDisabled(oq_Online);
   mpc_AdapterCombo->setDisabled(oq_Online);
   mpc_BitrateCombo->setDisabled(oq_Online);
   mpc_RefreshBtn->setDisabled(oq_Online);
}
