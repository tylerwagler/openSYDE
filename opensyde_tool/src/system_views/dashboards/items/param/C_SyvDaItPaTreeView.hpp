//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Parameter tree view (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVDAITPATREEVIEW_HPP
#define C_SYVDAITPATREEVIEW_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QList>
#include <QStringList>
#include "C_PuiSvDbParam.hpp"
#include "C_SyvDaItPaTreeModel.hpp"
#include "C_SyvDaItPaTreeDelegate.hpp"
#include "C_OgeTreeViewToolTipBase.hpp"
#include "C_PuiSvDbDataElementHandler.hpp"
#include "C_SyvDaItTableHeaderView.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SyvDaItPaTreeView :
   public stw::opensyde_gui_elements::C_OgeTreeViewToolTipBase
{
   Q_OBJECT

public:
   C_SyvDaItPaTreeView(QWidget * const opc_Parent = NULL);
   ~C_SyvDaItPaTreeView(void) override;

   bool IsEmpty(void) const;
   void ClearEcuValues(void);
   void ReloadSetValues(void);
   bool CheckRange(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ListIds,
                   const QList<stw::opensyde_core::C_OscNodeDataPoolListId> & orc_ListIds2) const;
   bool CheckAllListsRead(void) const;
   void PrepareChangedValues(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ListIds) const;
   void RemoveValuesChangedFlag(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ListIds)
   const;
   void SetDark(const bool oq_Value);
   void SetEditMode(const bool oq_EditMode);
   void SetCursorHandlingActive(const bool oq_Active);
   void SetConnected(const bool oq_Connected);
   void SetLoadSaveActive(const bool oq_Active);
   void SetActionActive(const bool oq_Active);
   QString GetSelectedItemTypeTemplate(void) const;
   uint32_t GetSelectedItemCount(void) const;
   void Init(stw::opensyde_gui_logic::C_PuiSvDbDataElementHandler * const opc_DataWidget);
   void UpdateEcuValues(void);
   QList<int32_t> GetCurrentColumnWidths(void) const;
   QList<int32_t> GetCurrentColumnPositionIndices(void) const;
   void DeleteSpecified(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ListIds);
   void SetColumnPositionIndices(const QList<int32_t> & orc_NewColPositionIndices);
   void SetColumnWidth(const QList<int32_t> & orc_NewColWidths);
   bool CheckListsRead(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ListIds) const;
   QList<stw::opensyde_gui_logic::C_PuiSvDbExpandedTreeIndex> GetAllExpandedTreeItems(void) const;
   void SetAllExpandedTreeItems(const QList<stw::opensyde_gui_logic::C_PuiSvDbExpandedTreeIndex> & orc_Items);
   QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> GetAllListIds(void) const;
   QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> GetChangedListElementIds(void) const;
   QList<stw::opensyde_core::C_OscNodeDataPoolListId> GetInvalidListIds(void) const;
   void SetCrcStatus(const stw::opensyde_core::C_OscNodeDataPoolListId & orc_ListId, const bool oq_Status);
   void GetListSetValues(const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_ListId,
                         QList<stw::opensyde_core::C_OscNodeDataPoolContent> & orc_ListValues) const;
   void HideToolTip(void);

   //The signals keyword is necessary for Qt signal slot functionality
   //lint -save -e1736

Q_SIGNALS:
   //lint -restore
   void SigActionRead(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ListIds);
   void SigActionWrite(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ListIds);
   void SigActionLoad(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ElementIds,
                      const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                      const uint32_t ou32_ValidLayers);
   void SigActionSave(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ListIds,
                      const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                      const uint32_t ou32_ValidLayers);
   void SigActionRecord(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ListIds);
   void SigActionRemove(const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_ListIds);
   void SigInformUserFloatRangeCheck(
      const QList<stw::opensyde_core::C_OscNodeDataPoolListElementId> & orc_InvalidValueIds,
      const QStringList & orc_InvalidValues, const QStringList & orc_NewValues);

protected:
   void keyPressEvent(QKeyEvent * const opc_Event) override;
   bool event(QEvent * const opc_Event) override;
   void mouseReleaseEvent(QMouseEvent * const opc_Event) override;
   void mousePressEvent(QMouseEvent * const opc_Event) override;
   void mouseDoubleClickEvent(QMouseEvent * const opc_Event) override;

private:
   stw::opensyde_gui_logic::C_SyvDaItPaTreeDelegate mc_Delegate;
   stw::opensyde_gui_logic::C_SyvDaItPaTreeModel mc_Model;
   C_SyvDaItTableHeaderView * mpc_TableHeaderView;
   stw::opensyde_gui_logic::C_PuiSvDbDataElementHandler * mpc_DataWidget;
   bool mq_IgnoreChanges;

   void m_OnVerticalScrollBarChange(const int32_t os32_NewScrollBarValue);
   void m_HandleChange(void);
   bool m_ColumnsSortedAsExpected(const QList<int32_t> & orc_NewColPositionIndices) const;
   void m_HandleLinkClicked(const QModelIndex & orc_Index);
   void m_HandleActionRead(const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                           const uint32_t ou32_ValidLayers);
   void m_HandleActionWrite(const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                            const uint32_t ou32_ValidLayers);
   void m_HandleActionApply(const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                            const uint32_t ou32_ValidLayers);
   void m_HandleActionLoad(const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                           const uint32_t ou32_ValidLayers);
   void m_HandleActionSave(const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                           const uint32_t ou32_ValidLayers);
   void m_HandleActionRecord(const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                             const uint32_t ou32_ValidLayers);
   void m_HandleActionRemove(const stw::opensyde_core::C_OscNodeDataPoolListElementId & orc_Id,
                             const uint32_t ou32_ValidLayers);
   void m_ScrollBarRangeChangedVer(const int32_t os32_Min, const int32_t os32_Max) const;
   void m_ScrollBarRangeChangedHor(const int32_t os32_Min, const int32_t os32_Max) const;

   //Avoid call
   C_SyvDaItPaTreeView(const C_SyvDaItPaTreeView &);
   C_SyvDaItPaTreeView & operator =(const C_SyvDaItPaTreeView &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
