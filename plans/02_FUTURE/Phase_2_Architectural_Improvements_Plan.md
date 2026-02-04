# Phase 2: Architectural Improvements + Qt Framework Leverage Implementation Plan

**Phase Duration**: 3-6 months
**Target Reduction**: 28,000-38,000 lines (enhanced with Qt-native patterns)
**Risk Level**: Medium
**Dependencies**: Phase 1 completion recommended (especially QList migration)
**Status**: Ready to start
**Qt-Native Priority**: ✅ **VERY HIGH** - Full Qt Framework integration

---

## Overview

Phase 2 introduces substantial architectural changes combined with deep Qt Framework integration. These changes require careful design, broader testing, and potentially affect user-facing components. The focus is on creating reusable, configurable components that replace combinatorial class explosions while leveraging Qt 6's powerful framework features.

**✨ Qt-Native Enhancements**:
- **Qt Model/View** for all table implementations
- **Qt Signals/Slots** for event communication (thread-safe)
- **QSS (Qt Style Sheets)** for declarative styling
- **Qt Property System** for dynamic configuration
- **QHash/QSet** migration for Qt-native associative containers
- **QPropertyAnimation** for visual effects
- **QStateMachine** for complex state management (if applicable)

---

## Task 5: Refactor GUI Elements System

### Current Status
- **Files to Consolidate**: 275 custom widget files (29,658 lines)
- **Target Reduction**: 10,000-15,000 lines (40-50% reduction)
- **Risk Level**: Medium (affects UI appearance and behavior)

### Problem Analysis

#### Widget Type Distribution
```
opensyde_tool/src/opensyde_gui_elements/
├── label/           70 types  (~7,000 lines)
├── push_button/     44 types  (~5,500 lines)
├── spin_box/        24 types  (~3,000 lines)
├── line_edit/       17 types  (~2,100 lines)
├── group_box/       17 types  (~1,900 lines)
├── combo_box/       14 types  (~1,700 lines)
├── widget/          23 types  (~2,800 lines)
├── check_box/       12 types  (~1,500 lines)
└── others/          54 types  (~5,158 lines)
```

#### Root Cause: Combinatorial Class Explosion
Most custom widgets combine these orthogonal concerns:
1. **Styling** (colors, fonts, borders) - Should be CSS
2. **Behavior** (tooltip, context menu, adaptiveness) - Should be mixins
3. **Validation** (input constraints) - Should be composition
4. **Context** (where used) - Shouldn't dictate separate class

**Example Problem**:
```cpp
// Current: Separate class for each combination
C_OgePubDialog          // Button styled for dialogs
C_OgePubDialogToolTip   // + tooltip support
C_OgePubCancel          // Cancel button styling
C_OgePubCancelToolTip   // + tooltip
C_OgePubToggle          // Toggle styling
C_OgePubToggleToolTip   // + tooltip
// ... 44 total button types
```

This creates **44 push button classes** when we need:
- **5-7 base types** (semantic purposes)
- **Dynamic styling** (via QSS)
- **Composable behaviors** (via mixins)

### Consolidation Strategy

#### Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│ Application Layer                                        │
│ Uses semantic widget types with dynamic configuration   │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│ Semantic Widget Layer (15-20 classes)                   │
│ C_OgePubPrimary, C_OgePubSecondary, C_OgePubDanger,    │
│ C_OgeLeStandard, C_OgeLeNumeric, etc.                  │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│ Behavior Mixin Layer (template classes)                 │
│ WithTooltip<T>, WithContextMenu<T>, WithValidation<T>  │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│ Base Widget Layer (Qt standard widgets)                 │
│ QPushButton, QLineEdit, QSpinBox, QComboBox, etc.      │
└──────────────────────────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│ QSS Styling Layer (CSS-based appearance)                │
│ Primary colors, fonts, borders, states (hover, pressed) │
└──────────────────────────────────────────────────────────┘
```

---

## Implementation: Push Buttons (44 types → 7 types)

### Step 5.1: Audit Current Button Usage

**Action**: Categorize all 44 button types by purpose
```bash
# Generate usage statistics
for file in opensyde_tool/src/opensyde_gui_elements/push_button/*.cpp; do
    classname=$(basename "$file" .cpp)
    count=$(grep -r "$classname" opensyde_tool/src --include="*.cpp" --include="*.hpp" | wc -l)
    echo "$classname: $count"
done > button_usage_stats.txt
```

**Deliverable**: Spreadsheet with:
- Button class name
- Usage count
- Primary purpose (dialog control, navigation, action, toggle, icon-only, etc.)
- Styling characteristics
- Behaviors (tooltip, menu, etc.)
- Migration target

### Step 5.2: Design Semantic Button Hierarchy

**Semantic Button Types** (based on purpose, not appearance):
```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Primary action button (e.g., OK, Save, Apply)

   Styled as primary action via QSS. Can be combined with behavior mixins.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePubPrimary : public QPushButton
{
   Q_OBJECT
public:
   explicit C_OgePubPrimary(QWidget * const opc_Parent = nullptr);
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Secondary action button (e.g., extra options, less important actions)
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePubSecondary : public QPushButton
{
   Q_OBJECT
public:
   explicit C_OgePubSecondary(QWidget * const opc_Parent = nullptr);
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Danger/destructive action button (e.g., Delete, Remove, Cancel)
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePubDanger : public QPushButton
{
   Q_OBJECT
public:
   explicit C_OgePubDanger(QWidget * const opc_Parent = nullptr);
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Toggle button (checkable button with on/off states)
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePubToggle : public QPushButton
{
   Q_OBJECT
public:
   explicit C_OgePubToggle(QWidget * const opc_Parent = nullptr);

   void SetChecked(const bool oq_Checked);
   bool IsChecked() const;
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Icon-only button (toolbar buttons, tool buttons)
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePubIconOnly : public QPushButton
{
   Q_OBJECT
public:
   explicit C_OgePubIconOnly(QWidget * const opc_Parent = nullptr);

   void SetIcon(const QIcon & orc_Icon);
   void SetIconSize(const QSize & orc_Size);
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Link-style button (looks like hyperlink)
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePubLink : public QPushButton
{
   Q_OBJECT
public:
   explicit C_OgePubLink(QWidget * const opc_Parent = nullptr);
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Flat/borderless button
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePubFlat : public QPushButton
{
   Q_OBJECT
public:
   explicit C_OgePubFlat(QWidget * const opc_Parent = nullptr);
};
```

### Step 5.3: Implement Behavior Mixins

**File**: `opensyde_tool/src/opensyde_gui_elements/C_OgeWidgetBehaviors.hpp`

```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget behavior mixins for composable functionality

   Template-based mixins that add behaviors to any widget type without requiring
   inheritance-based class combinatorics.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGEWIDGETBEHAVIORS_HPP
#define C_OGEWIDGETBEHAVIORS_HPP

#include <QWidget>
#include <QEvent>
#include <QMenu>
#include "C_OgeToolTipBase.hpp"
#include "C_OgeContextMenuBase.hpp"

namespace stw
{
namespace opensyde_gui_elements
{

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Tooltip behavior mixin

   Adds custom tooltip functionality to any widget.

   Usage: WithTooltip<QPushButton>, WithTooltip<C_OgePubPrimary>
*/
//----------------------------------------------------------------------------------------------------------------------
template<typename Base>
class WithTooltip : public Base, public C_OgeToolTipBase
{
public:
   using Base::Base;  // Inherit all constructors

   bool event(QEvent * const opc_Event) override
   {
      bool q_Return = Base::event(opc_Event);
      this->m_ToolTipEvent(opc_Event);
      return q_Return;
   }
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Context menu behavior mixin

   Adds custom context menu functionality to any widget.
*/
//----------------------------------------------------------------------------------------------------------------------
template<typename Base>
class WithContextMenu : public Base, public C_OgeContextMenuBase
{
public:
   using Base::Base;

   void SetContextMenuHandler(std::function<void(const QPoint&)> oprc_Handler)
   {
      mrc_MenuHandler = oprc_Handler;
   }

protected:
   void contextMenuEvent(QContextMenuEvent * const opc_Event) override
   {
      Base::contextMenuEvent(opc_Event);
      if (mrc_MenuHandler)
      {
         mrc_MenuHandler(opc_Event->pos());
      }
      this->m_SetupContextMenu();
   }

private:
   std::function<void(const QPoint&)> mrc_MenuHandler;
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Hover effect behavior mixin

   Adds visual feedback on mouse hover.
*/
//----------------------------------------------------------------------------------------------------------------------
template<typename Base>
class WithHoverEffect : public Base
{
public:
   using Base::Base;

   void SetHoverStyleSheet(const QString & orc_StyleSheet)
   {
      mc_HoverStyle = orc_StyleSheet;
   }

protected:
   void enterEvent(QEnterEvent * const opc_Event) override
   {
      Base::enterEvent(opc_Event);
      if (!mc_HoverStyle.isEmpty())
      {
         mc_OriginalStyle = this->styleSheet();
         this->setStyleSheet(mc_HoverStyle);
      }
      this->update();
   }

   void leaveEvent(QEvent * const opc_Event) override
   {
      Base::leaveEvent(opc_Event);
      if (!mc_HoverStyle.isEmpty())
      {
         this->setStyleSheet(mc_OriginalStyle);
      }
      this->update();
   }

private:
   QString mc_HoverStyle;
   QString mc_OriginalStyle;
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Double-click behavior mixin

   Emits signal on double-click.
*/
//----------------------------------------------------------------------------------------------------------------------
template<typename Base>
class WithDoubleClick : public Base
{
   Q_OBJECT

public:
   using Base::Base;

Q_SIGNALS:
   void SigDoubleClicked();

protected:
   void mouseDoubleClickEvent(QMouseEvent * const opc_Event) override
   {
      Base::mouseDoubleClickEvent(opc_Event);
      Q_EMIT SigDoubleClicked();
   }
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Adaptive sizing behavior mixin

   Auto-calculates optimal size based on content.
*/
//----------------------------------------------------------------------------------------------------------------------
template<typename Base>
class WithAdaptiveSize : public Base
{
public:
   using Base::Base;

   QSize sizeHint() const override
   {
      QSize c_Size = Base::sizeHint();
      // Customize based on widget type
      return c_Size;
   }

   QSize minimumSizeHint() const override
   {
      return sizeHint();
   }
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif // C_OGEWIDGETBEHAVIORS_HPP
```

### Step 5.4: Create Comprehensive QSS Styling

**File**: `opensyde_tool/src/opensyde_gui_elements/styling/buttons.qss`

```css
/* ===================================================================
   Push Button Styling
   =================================================================== */

/* Base button styles */
QPushButton {
    font-family: "Segoe UI";
    font-size: 10pt;
    padding: 6px 16px;
    border-radius: 4px;
    min-height: 24px;
}

/* ===================================================================
   Semantic Button Types
   =================================================================== */

/* Primary action buttons (OK, Save, Apply) */
C_OgePubPrimary {
    background-color: #0066CC;
    color: #FFFFFF;
    border: 1px solid #0052A3;
    font-weight: bold;
}

C_OgePubPrimary:hover {
    background-color: #0052A3;
    border: 1px solid #003D7A;
}

C_OgePubPrimary:pressed {
    background-color: #003D7A;
    border: 1px solid #002952;
}

C_OgePubPrimary:disabled {
    background-color: #CCCCCC;
    color: #808080;
    border: 1px solid #999999;
}

/* Secondary action buttons */
C_OgePubSecondary {
    background-color: #F0F0F0;
    color: #000000;
    border: 1px solid #AAAAAA;
}

C_OgePubSecondary:hover {
    background-color: #E0E0E0;
    border: 1px solid #888888;
}

C_OgePubSecondary:pressed {
    background-color: #D0D0D0;
    border: 1px solid #666666;
}

/* Danger/destructive action buttons (Delete, Remove, Cancel) */
C_OgePubDanger {
    background-color: #DC0000;
    color: #FFFFFF;
    border: 1px solid #A30000;
}

C_OgePubDanger:hover {
    background-color: #A30000;
    border: 1px solid #7A0000;
}

C_OgePubDanger:pressed {
    background-color: #7A0000;
    border: 1px solid #520000;
}

/* Toggle buttons */
C_OgePubToggle {
    background-color: #FFFFFF;
    color: #000000;
    border: 2px solid #AAAAAA;
}

C_OgePubToggle:checked {
    background-color: #0066CC;
    color: #FFFFFF;
    border: 2px solid #0052A3;
}

C_OgePubToggle:hover {
    border: 2px solid #0066CC;
}

/* Icon-only buttons */
C_OgePubIconOnly {
    background-color: transparent;
    border: none;
    padding: 4px;
}

C_OgePubIconOnly:hover {
    background-color: rgba(0, 0, 0, 0.05);
    border-radius: 4px;
}

C_OgePubIconOnly:pressed {
    background-color: rgba(0, 0, 0, 0.1);
}

/* Link-style buttons */
C_OgePubLink {
    background-color: transparent;
    color: #0066CC;
    border: none;
    text-decoration: underline;
    padding: 2px;
}

C_OgePubLink:hover {
    color: #0052A3;
}

C_OgePubLink:pressed {
    color: #003D7A;
}

/* Flat buttons */
C_OgePubFlat {
    background-color: transparent;
    border: 1px solid transparent;
}

C_OgePubFlat:hover {
    background-color: rgba(0, 0, 0, 0.05);
    border: 1px solid #AAAAAA;
}

/* ===================================================================
   Style Variants (via Qt properties)
   =================================================================== */

/* Size variants */
QPushButton[buttonSize="small"] {
    font-size: 8pt;
    padding: 4px 12px;
    min-height: 20px;
}

QPushButton[buttonSize="large"] {
    font-size: 12pt;
    padding: 8px 20px;
    min-height: 32px;
}

/* Shape variants */
QPushButton[buttonShape="round"] {
    border-radius: 16px;
}

QPushButton[buttonShape="square"] {
    border-radius: 0px;
}

/* Width variants */
QPushButton[buttonWidth="full"] {
    min-width: 100%;
}

QPushButton[buttonWidth="fixed"] {
    min-width: 120px;
    max-width: 120px;
}

/* Icon position */
QPushButton[iconPosition="left"] {
    text-align: left;
    padding-left: 32px;
}

QPushButton[iconPosition="right"] {
    text-align: right;
    padding-right: 32px;
}
```

### Step 5.5: Usage Examples

**Basic Usage**:
```cpp
// Simple primary button
C_OgePubPrimary * pc_ButtonOk = new C_OgePubPrimary(this);
pc_ButtonOk->setText("OK");

// Primary button with tooltip
using C_OgePubPrimaryToolTip = WithTooltip<C_OgePubPrimary>;
C_OgePubPrimaryToolTip * pc_Button = new C_OgePubPrimaryToolTip(this);
pc_Button->setText("Save");
pc_Button->SetToolTipInformation("Save", "Save changes to file");

// Danger button with confirmation menu
using C_OgePubDangerMenu = WithContextMenu<C_OgePubDanger>;
C_OgePubDangerMenu * pc_DeleteButton = new C_OgePubDangerMenu(this);
pc_DeleteButton->setText("Delete");
pc_DeleteButton->SetContextMenuHandler([](const QPoint & orc_Pos) {
   // Show confirmation before delete
});

// Multiple behaviors combined
using C_OgePubMultiBehavior = WithTooltip<WithDoubleClick<C_OgePubPrimary>>;
C_OgePubMultiBehavior * pc_ComplexButton = new C_OgePubMultiBehavior(this);
```

**Property-Based Styling**:
```cpp
// Create button with size variant
C_OgePubPrimary * pc_SmallButton = new C_OgePubPrimary(this);
pc_SmallButton->setProperty("buttonSize", "small");
pc_SmallButton->setText("Compact");

// Round-shaped button
C_OgePubSecondary * pc_RoundButton = new C_OgePubSecondary(this);
pc_RoundButton->setProperty("buttonShape", "round");
```

### Step 5.6: Migration Strategy

#### Phase 1: Create New Architecture (2 weeks)
1. Implement 7 semantic button classes
2. Implement behavior mixin templates
3. Create comprehensive QSS styling
4. Write unit tests
5. Create usage documentation

#### Phase 2: Map Old Classes to New (1 week)
**Create mapping table**: `button_migration_map.json`
```json
{
  "C_OgePubDialog": {
    "newClass": "C_OgePubPrimary",
    "properties": {}
  },
  "C_OgePubDialogToolTip": {
    "newClass": "WithTooltip<C_OgePubPrimary>",
    "properties": {}
  },
  "C_OgePubCancel": {
    "newClass": "C_OgePubDanger",
    "properties": {}
  },
  "C_OgePubToggle": {
    "newClass": "C_OgePubToggle",
    "properties": {}
  },
  "C_OgePubIconOnly": {
    "newClass": "C_OgePubIconOnly",
    "properties": {}
  },
  "C_OgePubSvgIcon16": {
    "newClass": "C_OgePubIconOnly",
    "properties": {
      "iconSize": "16x16"
    }
  }
}
```

#### Phase 3: Automated Migration (2-3 weeks)
**Write migration script**: `scripts/migrate_buttons.py`

Capabilities:
- Parse C++ source files
- Replace class instantiations
- Add behavior mixins where needed
- Set properties based on mapping
- Update includes
- Generate report of changes

#### Phase 4: Manual Migration (3-4 weeks)
For complex cases:
- Buttons with custom paintEvent
- Buttons with complex signal handling
- Buttons in .ui files (requires Qt Designer)

#### Phase 5: Testing & Validation (2-3 weeks)
- Visual regression testing
- Functional testing
- Performance testing
- User acceptance testing

#### Phase 6: Cleanup (1 week)
- Remove old button classes
- Update CMakeLists.txt
- Remove unused includes
- Update documentation

### Step 5.7: Testing Strategy

**Unit Tests**:
```cpp
// Test semantic button classes
TEST(C_OgePubPrimaryTest, DefaultStyling) {
   C_OgePubPrimary c_Button;
   EXPECT_FALSE(c_Button.text().isEmpty());
   EXPECT_TRUE(c_Button.isEnabled());
}

// Test behavior mixins
TEST(WithTooltipTest, TooltipDisplay) {
   using ButtonWithTooltip = WithTooltip<C_OgePubPrimary>;
   ButtonWithTooltip c_Button;
   c_Button.SetToolTipInformation("Title", "Description");
   // Verify tooltip appears on hover
}

// Test property-based styling
TEST(C_OgePubPrimaryTest, PropertyStyling) {
   C_OgePubPrimary c_Button;
   c_Button.setProperty("buttonSize", "small");
   // Verify size change
}
```

**Visual Regression Tests**:
- Screenshot all dialogs/windows before migration
- Screenshot after migration
- Automated pixel-by-pixel comparison
- Manual review of differences

**Integration Tests**:
- Test all major workflows
- Verify button clicks trigger correct actions
- Check keyboard navigation (Tab, Enter, Esc)
- Test tooltip display timing
- Verify context menus appear correctly

### Risk Mitigation

**Risk**: Visual appearance changes
- **Mitigation**: Pixel-perfect QSS recreation of original styles
- **Testing**: Visual regression testing with screenshots

**Risk**: Behavioral regressions
- **Mitigation**: Comprehensive unit tests for all behaviors
- **Testing**: Integration tests for all interactive features

**Risk**: Performance issues with mixins
- **Mitigation**: Benchmark before/after, optimize templates
- **Testing**: Performance tests for button creation and interaction

**Risk**: Breaking existing code
- **Mitigation**: Automated migration script with verification
- **Testing**: Compilation tests, functional tests

### Success Criteria
- [ ] Push button types reduced from 44 to 7 semantic types
- [ ] Behavior mixins implemented and tested
- [ ] QSS styling covers all variants
- [ ] All buttons migrated
- [ ] Visual appearance unchanged
- [ ] All functional behaviors work
- [ ] ~5,500 lines removed from push_button/

### Estimated Effort (Push Buttons Only)
- **Architecture Design**: 1 week
- **Implementation**: 2 weeks
- **Migration Scripting**: 1 week
- **Execution**: 3-4 weeks
- **Testing**: 2-3 weeks
- **Total**: 9-11 weeks

---

## Repeat for Other Widget Types

### Spin Boxes (24 types → 5 types)
**Semantic Types**:
- `C_OgeSbxStandard` - Standard integer spin box
- `C_OgeSbxDouble` - Double/float spin box
- `C_OgeSbxHex` - Hexadecimal value spin box
- `C_OgeSbxTime` - Time value spin box
- `C_OgeSbxAutoRange` - Auto-adjusting range spin box

**Estimated Reduction**: ~2,400 lines

### Line Edits (17 types → 4-5 types)
**Semantic Types**:
- `C_OgeLeStandard` - Standard text input
- `C_OgeLeNumeric` - Numeric input with validation
- `C_OgeLeFile` - File path input
- `C_OgeLePassword` - Password input
- `C_OgeLeSearch` - Search input with clear button

**Estimated Reduction**: ~1,700 lines

### Combo Boxes (14 types → 3-4 types)
**Semantic Types**:
- `C_OgeCbxStandard` - Standard dropdown
- `C_OgeCbxTable` - Dropdown for table cells
- `C_OgeCbxSearchable` - Searchable dropdown
- `C_OgeCbxMultiSelect` - Multi-selection dropdown

**Estimated Reduction**: ~1,400 lines

### Group Boxes (17 types → 3-4 types)
**Semantic Types**:
- `C_OgeGbxStandard` - Standard group box
- `C_OgeGbxCollapsible` - Collapsible group box
- `C_OgeGbxTransparent` - Borderless group box

**Estimated Reduction**: ~1,500 lines

### Check Boxes (12 types → 3 types)
**Semantic Types**:
- `C_OgeChxStandard` - Standard checkbox
- `C_OgeChxTriState` - Three-state checkbox
- `C_OgeChxToggleSwitch` - Toggle switch style

**Estimated Reduction**: ~1,200 lines

---

## Task 6: Consolidate CAN Monitor Protocols

### Current Status
- **Protocol Classes**: 9 protocol interpreters (11,084 lines)
- **Target Reduction**: 3,500-4,500 lines (30-40%)
- **Location**: `opensyde_tool/libs/opensyde_core/kefex_diaglib/cmonprotocol/`

### Problem Analysis

All protocol classes share similar structure:
```cpp
class C_CanMonProtocol[Name] : public C_CanMonProtocolBase {
   // Parse CAN message
   QString MessageToString(const T_STWCAN_Msg_RX & orc_Msg);

   // Repetitive helper functions
   QString mh_BytesToString(const uint8_t * const opu8_Data, const uint32_t ou32_Length);
   QString mh_BytesToDwordLowHigh(const uint8_t * const opu8_Data);
   QString mh_DecimalToHex(const uint32_t ou32_Value, const uint8_t ou8_Width);

   // Protocol-specific interpretation
   QString mh_InterpretServiceId(const uint8_t ou8_ServiceId);
   QString mh_InterpretData(const uint8_t * const opu8_Data);
};
```

### Consolidation Strategy

#### Step 6.1: Extract Common Utilities

**File**: `opensyde_tool/libs/opensyde_core/kefex_diaglib/cmonprotocol/C_CanMonProtocolUtil.hpp`

```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Common utilities for CAN protocol interpretation

   Shared helper functions for byte manipulation, formatting, and common
   protocol patterns used across all protocol interpreters.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CANMONPROTOCOLUTIL_HPP
#define C_CANMONPROTOCOLUTIL_HPP

#include <QString>
#include "stwtypes.hpp"

namespace stw
{
namespace opensyde_core
{

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   CAN protocol interpretation utilities
*/
//----------------------------------------------------------------------------------------------------------------------
class C_CanMonProtocolUtil
{
public:
   // Byte array conversions
   static QString h_BytesToString(const uint8_t * const opu8_Data, const uint32_t ou32_Length,
                                  const bool oq_Hex = true);
   static QString h_BytesToWordLowHigh(const uint8_t * const opu8_Data);
   static QString h_BytesToWordHighLow(const uint8_t * const opu8_Data);
   static QString h_BytesToDwordLowHigh(const uint8_t * const opu8_Data);
   static QString h_BytesToDwordHighLow(const uint8_t * const opu8_Data);
   static uint16_t h_BytesToWordLowHigh(const uint8_t ou8_Low, const uint8_t ou8_High);
   static uint32_t h_BytesToDwordLowHigh(const uint8_t ou8_LL, const uint8_t ou8_LH,
                                         const uint8_t ou8_HL, const uint8_t ou8_HH);

   // Formatting
   static QString h_DecimalToHex(const uint32_t ou32_Value, const uint8_t ou8_MinWidth = 2);
   static QString h_GetValueDecimalHex(const uint32_t ou32_Value);
   static QString h_GetByteAsString(const uint8_t ou8_Value, const bool oq_Hex = true);

   // CAN message parsing
   static QString h_GetCanIdAsString(const uint32_t ou32_CanId, const bool oq_Extended);
   static QString h_GetDlcAsString(const uint8_t ou8_Dlc);
   static QString h_GetDataAsString(const uint8_t * const opu8_Data, const uint8_t ou8_Dlc,
                                    const bool oq_Hex = true);

   // Bit manipulation
   static bool h_GetBit(const uint8_t ou8_Value, const uint8_t ou8_BitPosition);
   static uint8_t h_GetNibble(const uint8_t ou8_Value, const bool oq_HighNibble);

   // String formatting helpers
   static QString h_FormatServiceName(const QString & orc_Name, const uint8_t ou8_Id);
   static QString h_FormatKeyValue(const QString & orc_Key, const QString & orc_Value,
                                   const bool oq_Hex = false);
   static QString h_FormatParameterList(const QStringList & orc_Parameters);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_CANMONPROTOCOLUTIL_HPP
```

#### Step 6.2: Create Protocol Engine Architecture

**File**: `opensyde_tool/libs/opensyde_core/kefex_diaglib/cmonprotocol/C_CanMonProtocolEngine.hpp`

```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Generic protocol interpretation engine

   Template-based engine for CAN protocol interpretation. Reduces duplication
   by providing common infrastructure for message parsing.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CANMONPROTOCOLENGINE_HPP
#define C_CANMONPROTOCOLENGINE_HPP

#include <functional>
#include <map>
#include <QString>
#include "stwtypes.hpp"
#include "C_SclCanTypes.hpp"

namespace stw
{
namespace opensyde_core
{

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Protocol interpretation engine

   Provides framework for parsing CAN messages with protocol-specific handlers.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_CanMonProtocolEngine
{
public:
   // Message interpretation function type
   using MessageInterpreter = std::function<QString(const T_STWCAN_Msg_RX &)>;

   // Service/command interpretation function type
   using ServiceInterpreter = std::function<QString(const uint8_t, const uint8_t * const, const uint8_t)>;

   C_CanMonProtocolEngine();

   // Register message interpreters
   void RegisterCanIdRange(const uint32_t ou32_IdStart, const uint32_t ou32_IdEnd,
                           MessageInterpreter orc_Interpreter);
   void RegisterCanIdMask(const uint32_t ou32_IdPattern, const uint32_t ou32_IdMask,
                          MessageInterpreter orc_Interpreter);

   // Register service interpreters (for protocols with service IDs)
   void RegisterService(const uint8_t ou8_ServiceId, ServiceInterpreter orc_Interpreter);
   void SetDefaultServiceInterpreter(ServiceInterpreter orc_Interpreter);

   // Interpret message
   QString InterpretMessage(const T_STWCAN_Msg_RX & orc_Msg) const;

protected:
   struct C_IdRange
   {
      uint32_t u32_IdStart;
      uint32_t u32_IdEnd;
      MessageInterpreter c_Interpreter;
   };

   struct C_IdMask
   {
      uint32_t u32_Pattern;
      uint32_t u32_Mask;
      MessageInterpreter c_Interpreter;
   };

   std::vector<C_IdRange> mc_IdRanges;
   std::vector<C_IdMask> mc_IdMasks;
   std::map<uint8_t, ServiceInterpreter> mc_ServiceInterpreters;
   ServiceInterpreter mc_DefaultServiceInterpreter;
};

} // namespace opensyde_core
} // namespace stw

#endif // C_CANMONPROTOCOLENGINE_HPP
```

#### Step 6.3: Refactor Protocol Classes

**Before** - C_CanMonProtocolKefex.cpp (1,145 lines):
```cpp
QString C_CanMonProtocolKefex::m_MessageToString(const T_STWCAN_Msg_RX & orc_Msg) const
{
   QString c_Text;

   // Repetitive byte manipulation (appears in every protocol)
   const uint8_t u8_ServiceId = orc_Msg.au8_Data[0];
   const uint16_t u16_Index = mh_BytesToWordLowHigh(&orc_Msg.au8_Data[1]);
   const uint8_t u8_SubIndex = orc_Msg.au8_Data[3];

   // Service ID interpretation (protocol-specific)
   switch (u8_ServiceId)
   {
      case 0x01: c_Text = "Read Request"; break;
      case 0x02: c_Text = "Write Request"; break;
      // ... 50 more cases
   }

   // Format output (repetitive pattern)
   c_Text += " IDX:" + mh_DecimalToHex(u16_Index, 4);
   c_Text += " SIDX:" + mh_DecimalToHex(u8_SubIndex, 2);

   return c_Text;
}

// Repetitive helper functions (duplicated across 9 protocol classes)
QString C_CanMonProtocolKefex::mh_BytesToWordLowHigh(const uint8_t * const opu8_Data)
{
   const uint16_t u16_Value = static_cast<uint16_t>(opu8_Data[0]) |
                              (static_cast<uint16_t>(opu8_Data[1]) << 8U);
   return QString::number(u16_Value);
}

QString C_CanMonProtocolKefex::mh_DecimalToHex(const uint32_t ou32_Value, const uint8_t ou8_Width)
{
   return QString("0x%1").arg(ou32_Value, ou8_Width, 16, QChar('0')).toUpper();
}

// ... 30 more repetitive helpers
```

**After** - C_CanMonProtocolKefex.cpp (refactored, ~600 lines):
```cpp
C_CanMonProtocolKefex::C_CanMonProtocolKefex()
{
   // Register service interpreters in constructor
   mc_Engine.RegisterService(0x01, [this](uint8_t u8_Id, const uint8_t * const opu8_Data, uint8_t ou8_Dlc) {
      return this->mh_InterpretReadRequest(opu8_Data, ou8_Dlc);
   });

   mc_Engine.RegisterService(0x02, [this](uint8_t u8_Id, const uint8_t * const opu8_Data, uint8_t ou8_Dlc) {
      return this->mh_InterpretWriteRequest(opu8_Data, ou8_Dlc);
   });

   // ... register all services
}

QString C_CanMonProtocolKefex::m_MessageToString(const T_STWCAN_Msg_RX & orc_Msg) const
{
   // Use engine for interpretation
   return mc_Engine.InterpretMessage(orc_Msg);
}

QString C_CanMonProtocolKefex::mh_InterpretReadRequest(const uint8_t * const opu8_Data,
                                                        const uint8_t ou8_Dlc) const
{
   QString c_Text = "Read Request";

   if (ou8_Dlc >= 4)
   {
      // Use common utilities instead of local helpers
      const uint16_t u16_Index = C_CanMonProtocolUtil::h_BytesToWordLowHigh(opu8_Data[1], opu8_Data[2]);
      const uint8_t u8_SubIndex = opu8_Data[3];

      c_Text += " " + C_CanMonProtocolUtil::h_FormatKeyValue("IDX",
                 C_CanMonProtocolUtil::h_DecimalToHex(u16_Index, 4), true);
      c_Text += " " + C_CanMonProtocolUtil::h_FormatKeyValue("SIDX",
                 C_CanMonProtocolUtil::h_DecimalToHex(u8_SubIndex, 2), true);
   }

   return c_Text;
}

// No more repetitive helper functions - all use C_CanMonProtocolUtil
```

**Line Reduction**: 1,145 → ~600 lines (545 lines saved, 47% reduction)

#### Step 6.4: Create Protocol Configuration Tables

For protocols with many similar services, use data-driven configuration:

**File**: `C_CanMonProtocolOpenSydeConfig.cpp`
```cpp
// Define service table
struct C_ServiceDefinition
{
   uint8_t u8_ServiceId;
   const char_t * pcn_Name;
   bool q_HasSubFunction;
   uint8_t u8_MinDataLength;
};

static const C_ServiceDefinition hac_OPENSYDE_SERVICES[] =
{
   { 0x10, "Diagnostic Session Control", true, 2 },
   { 0x11, "ECU Reset", true, 2 },
   { 0x22, "Read Data By Identifier", false, 3 },
   { 0x23, "Read Memory By Address", false, 4 },
   { 0x2E, "Write Data By Identifier", false, 4 },
   // ... 50 more services
};

QString C_CanMonProtocolOpenSyde::mh_GetServiceName(const uint8_t ou8_ServiceId) const
{
   for (const auto & rc_Service : hac_OPENSYDE_SERVICES)
   {
      if (rc_Service.u8_ServiceId == ou8_ServiceId)
      {
         return rc_Service.pcn_Name;
      }
   }
   return C_CanMonProtocolUtil::h_DecimalToHex(ou8_ServiceId, 2);
}
```

### Step 6.5: Migration Plan

**Week 1-2**: Extract common utilities
- Implement C_CanMonProtocolUtil
- Write unit tests
- Benchmark performance

**Week 3-4**: Create protocol engine
- Implement C_CanMonProtocolEngine
- Test with simple protocol
- Performance validation

**Week 5-8**: Refactor protocols (priority order)
1. C_CanMonProtocolOpenSyde (2,140 lines) - most complex
2. C_CanMonProtocolShipIpIva (2,842 lines)
3. C_CanMonProtocolXfl (1,803 lines)
4. C_CanMonProtocolKefex (1,145 lines)
5. C_CanMonProtocolJ1939 (817 lines)
6. Remaining 4 protocols

**Week 9-10**: Testing
- Unit tests for each protocol
- Integration tests with CAN traces
- Regression tests with customer data

### Testing Strategy

**Unit Tests**:
```cpp
TEST(C_CanMonProtocolUtilTest, BytesToWordLowHigh) {
   const uint8_t au8_Data[2] = { 0x34, 0x12 };
   EXPECT_EQ(0x1234, C_CanMonProtocolUtil::h_BytesToWordLowHigh(au8_Data[0], au8_Data[1]));
}

TEST(C_CanMonProtocolKefexTest, ReadRequestParsing) {
   T_STWCAN_Msg_RX c_Msg;
   c_Msg.au8_Data[0] = 0x01;  // Read request service ID
   c_Msg.au8_Data[1] = 0x00;  // Index low
   c_Msg.au8_Data[2] = 0x10;  // Index high
   c_Msg.au8_Data[3] = 0x05;  // Subindex

   C_CanMonProtocolKefex c_Protocol;
   QString c_Result = c_Protocol.MessageToString(c_Msg);

   EXPECT_TRUE(c_Result.contains("Read Request"));
   EXPECT_TRUE(c_Result.contains("IDX:0x1000"));
   EXPECT_TRUE(c_Result.contains("SIDX:0x05"));
}
```

**Integration Tests**:
- Load real CAN trace files
- Parse all messages with each protocol
- Compare output to original implementation
- Verify identical interpretation

**Performance Tests**:
- Benchmark message parsing speed
- Should be equal or faster than original
- Target: <1ms per message for all protocols

### Success Criteria
- [ ] C_CanMonProtocolUtil utility class implemented
- [ ] All 9 protocols refactored
- [ ] Message interpretation identical to original
- [ ] All unit tests passing
- [ ] Integration tests with CAN traces passing
- [ ] 3,500-4,500 lines removed
- [ ] Performance maintained or improved

### Estimated Effort
- **Utilities & Engine**: 2-3 weeks
- **Protocol Refactoring**: 4-5 weeks
- **Testing**: 2-3 weeks
- **Total**: 8-11 weeks

---

## Task 7: Refactor Table Delegates

### Current Status
- **Delegate Classes**: 30+ custom delegates (~5,000 lines)
- **Target Reduction**: 2,500-3,500 lines (50-70%)
- **Pattern**: Most delegates differ only in editor type and validation

### Consolidation Strategy

#### Step 7.1: Design Configurable Delegate System

**File**: `opensyde_tool/src/opensyde_gui_elements/C_OgeDelegateConfigurable.hpp`

```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Configurable table delegate

   Generic delegate that can be configured per-column instead of requiring
   subclasses for each table.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgeDelegateConfigurable : public QStyledItemDelegate
{
   Q_OBJECT

public:
   enum E_EditorType
   {
      eNO_EDITOR,
      eLINE_EDIT,
      eSPIN_BOX,
      eDOUBLE_SPIN_BOX,
      eCOMBO_BOX,
      eCHECK_BOX,
      eCOLOR_PICKER,
      eFILE_SELECTOR,
      eCUSTOM
   };

   struct C_ColumnConfig
   {
      E_EditorType e_EditorType = eLINE_EDIT;

      // For numeric editors
      QVariant c_MinValue;
      QVariant c_MaxValue;
      QVariant c_StepSize;
      QString c_Suffix;
      QString c_Prefix;

      // For combo box
      QStringList c_ComboItems;

      // For validation
      QValidator * pc_Validator = nullptr;
      std::function<bool(const QVariant &)> c_CustomValidator;

      // For custom editors
      std::function<QWidget *(QWidget *)> c_CustomEditorFactory;

      // For painting
      std::function<void(QPainter *, const QStyleOptionViewItem &, const QModelIndex &)> c_CustomPainter;
      Qt::Alignment e_Alignment = Qt::AlignLeft | Qt::AlignVCenter;
      QColor c_TextColor;
      QColor c_BackgroundColor;
      QFont c_Font;
   };

   explicit C_OgeDelegateConfigurable(QObject * const opc_Parent = nullptr);

   void SetColumnConfig(const int32_t os32_Column, const C_ColumnConfig & orc_Config);
   void SetRowConfig(const int32_t os32_Row, const C_ColumnConfig & orc_Config);
   void SetCellConfig(const int32_t os32_Row, const int32_t os32_Column, const C_ColumnConfig & orc_Config);

   // QStyledItemDelegate interface
   QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                          const QModelIndex & index) const override;
   void setEditorData(QWidget * editor, const QModelIndex & index) const override;
   void setModelData(QWidget * editor, QAbstractItemModel * model,
                     const QModelIndex & index) const override;
   void paint(QPainter * painter, const QStyleOptionViewItem & option,
              const QModelIndex & index) const override;
   QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const override;

private:
   std::map<int32_t, C_ColumnConfig> mc_ColumnConfigs;
   std::map<int32_t, C_ColumnConfig> mc_RowConfigs;
   std::map<QPair<int32_t, int32_t>, C_ColumnConfig> mc_CellConfigs;

   const C_ColumnConfig * m_GetConfig(const QModelIndex & orc_Index) const;
   QWidget * m_CreateEditorByType(const C_ColumnConfig & orc_Config, QWidget * const opc_Parent) const;
};
```

#### Step 7.2: Usage Examples

**Before** - Dedicated delegate class:
```cpp
// C_SdNdeDpListTableDelegate.cpp - 180 lines
class C_SdNdeDpListTableDelegate : public QStyledItemDelegate
{
public:
   QWidget * createEditor(...) {
      // Complex logic for different columns
      if (column == 0) return new QLineEdit();
      if (column == 1) return new QSpinBox();  // Range 0-255
      if (column == 2) return new QComboBox();  // Type dropdown
      // ... etc
   }

   void paint(...) {
      // Custom painting for each column
   }
};

// In table setup
mc_Table->setItemDelegate(new C_SdNdeDpListTableDelegate(this));
```

**After** - Configured generic delegate:
```cpp
// No dedicated class needed - configure generic delegate
C_OgeDelegateConfigurable * pc_Delegate = new C_OgeDelegateConfigurable(this);

// Configure column 0: Text input
C_OgeDelegateConfigurable::C_ColumnConfig c_Col0;
c_Col0.e_EditorType = C_OgeDelegateConfigurable::eLINE_EDIT;
c_Col0.pc_Validator = new QRegExpValidator(QRegExp("[A-Za-z0-9_]+"), this);
pc_Delegate->SetColumnConfig(0, c_Col0);

// Configure column 1: Numeric input
C_OgeDelegateConfigurable::C_ColumnConfig c_Col1;
c_Col1.e_EditorType = C_OgeDelegateConfigurable::eSPIN_BOX;
c_Col1.c_MinValue = 0;
c_Col1.c_MaxValue = 255;
pc_Delegate->SetColumnConfig(1, c_Col1);

// Configure column 2: Dropdown
C_OgeDelegateConfigurable::C_ColumnConfig c_Col2;
c_Col2.e_EditorType = C_OgeDelegateConfigurable::eCOMBO_BOX;
c_Col2.c_ComboItems = {"uint8", "uint16", "uint32", "int8", "int16", "int32"};
pc_Delegate->SetColumnConfig(2, c_Col2);

mc_Table->setItemDelegate(pc_Delegate);
```

### Step 7.3: Migration Plan

**Phase 1**: Implement generic delegate (1-2 weeks)
**Phase 2**: Migrate simple tables (2-3 weeks)
**Phase 3**: Handle complex delegates (2-3 weeks)
**Phase 4**: Testing (1-2 weeks)

### Success Criteria
- [ ] Generic delegate supports all common editor types
- [ ] 25+ specialized delegates replaced
- [ ] Table behavior unchanged
- [ ] 2,500-3,500 lines removed

### Estimated Effort: 6-10 weeks

---

## Task 8: Simplify UI Files

### Current Status
- **UI Files**: 212 .ui files (79,043 lines XML)
- **Target Reduction**: 10,000-15,000 lines (15-20%)
- **Strategy**: Consolidate simple dialogs, create reusable templates

### Consolidation Strategy

#### Step 8.1: Audit UI Files
Categorize by complexity:
- **Simple** (<100 lines): Candidates for programmatic layouts
- **Template** (100-300 lines): Use reusable dialog templates
- **Complex** (>300 lines): Keep as .ui files

#### Step 8.2: Create Dialog Templates
```cpp
class C_OgeDialogStandard : public QDialog {
   void SetTitleWidget(QWidget * widget);
   void SetContentWidget(QWidget * widget);
   void SetButtonBox(QDialogButtonBox::StandardButtons buttons);
};
```

#### Step 8.3: Migrate Simple Dialogs
Convert to programmatic layouts

### Success Criteria
- [ ] 40-50 simple .ui files converted to code
- [ ] 5-7 reusable dialog templates created
- [ ] 10,000-15,000 lines removed from .ui files

### Estimated Effort: 8-12 weeks

---

---

## Task 9: ✨ NEW - Qt Associative Container Migration

### Current Status
- **Migration Target**: `std::map` and `std::unordered_map` to Qt containers
- **Estimated Occurrences**: 200-300 (needs verification)
- **Target Reduction**: 1,000-1,500 lines
- **Risk Level**: Low-Medium

### Qt Container Selection

| STL Container | Qt Replacement | When to Use | Performance |
|---------------|----------------|-------------|-------------|
| `std::map<QString, T>` | `QHash<QString, T>` | String keys (most cases) | O(1) vs O(log n) - **QHash faster** |
| `std::map<int, T>` | `QHash<int, T>` | Integer keys | O(1) vs O(log n) - **QHash faster** |
| `std::unordered_map<T1, T2>` | `QHash<T1, T2>` | Hash-based lookup | O(1) - Equivalent |
| `std::set<T>` | `QSet<T>` | Unique elements | O(1) contains vs O(log n) |

**Why QHash over QMap**:
- `QHash` is O(1) lookup (like `std::unordered_map`)
- `QMap` is O(log n) (like `std::map`)
- For string keys, `QHash` is almost always better
- Implicit sharing (copy-on-write) benefit

### Implementation Steps

#### Step 9.1: Audit Associative Container Usage

**Action**: Find all std::map and std::unordered_map occurrences
```bash
grep -r "std::map<QString" opensyde_tool/libs/opensyde_core --include="*.cpp" --include="*.hpp" | wc -l
grep -r "std::unordered_map" opensyde_tool/libs/opensyde_core --include="*.cpp" --include="*.hpp" | wc -l
grep -r "std::set<" opensyde_tool/libs/opensyde_core --include="*.cpp" --include="*.hpp" | wc -l
```

#### Step 9.2: Conversion Examples

**std::map → QHash**:
```cpp
// BEFORE (STL)
std::map<QString, C_OscNode> mc_Nodes;

auto it = mc_Nodes.find(key);
if (it != mc_Nodes.end()) {
   return it->second;
} else {
   return defaultNode;
}

// AFTER (Qt-native - cleaner)
QHash<QString, C_OscNode> mc_Nodes;

return mc_Nodes.value(key, defaultNode);  // One line!

// Or with exists check:
if (mc_Nodes.contains(key)) {
   return mc_Nodes[key];
}
```

**std::unordered_map → QHash**:
```cpp
// BEFORE
std::unordered_map<uint32_t, QString> mc_IdToName;
mc_IdToName.insert({id, name});

// AFTER (same performance, Qt-native)
QHash<uint32_t, QString> mc_IdToName;
mc_IdToName.insert(id, name);
```

**std::set → QSet**:
```cpp
// BEFORE
std::set<QString> mc_UniqueNames;
if (mc_UniqueNames.find(name) != mc_UniqueNames.end()) {
   // Already exists
}

// AFTER (cleaner)
QSet<QString> mc_UniqueNames;
if (mc_UniqueNames.contains(name)) {
   // Already exists
}
```

#### Step 9.3: Iteration Order Considerations

**⚠️ Important**: QHash iteration order is undefined (like std::unordered_map)

```cpp
// If you need ordered iteration, use QMap (not QHash)
QMap<QString, T> orderedMap;  // Keeps keys sorted

// For most cases, QHash is preferred (faster)
QHash<QString, T> unorderedMap;  // O(1) lookup
```

### Estimated Effort
- **Audit**: 2-3 hours
- **Migration**: 12-15 hours (200-300 occurrences)
- **Testing**: 3-5 hours
- **Total**: 17-23 hours (~3-4 weeks alongside other tasks)

### Success Criteria
- [ ] All `std::map<QString, T>` migrated to `QHash<QString, T>`
- [ ] All `std::unordered_map` migrated to `QHash`
- [ ] std::set migrated to `QSet` where applicable
- [ ] No performance regressions (QHash should be faster)
- [ ] All tests passing
- [ ] ~1,000-1,500 lines reduced through Qt idioms

---

## Phase 2 Summary (Qt-Native Enhanced)

### Total Impact

| Metric | Original Plan | Qt-Enhanced Plan | Improvement |
|--------|---------------|------------------|-------------|
| **Line Reduction** | 25,100-35,100 | **28,000-38,000** | **+3,000** |
| **Duration** | 3-6 months | **3-6 months** | Same |
| **Risk** | Medium | **Medium** | Same |
| **Qt Framework Usage** | Medium | **Very High** | ++ |

**Enhanced Impact Breakdown**:
- GUI elements: 10,000-15,000 lines (with Qt patterns)
- CAN protocols: 3,500-4,500 lines (with Qt Signals/Slots)
- Table delegates: 2,500-3,500 lines (with Qt Model/View)
- UI files: 10,000-15,000 lines
- **QHash migration: 1,000-1,500 lines (NEW)**

**Total**: 28,000-38,000 lines

### Completion Criteria (Updated)
1. ✅ GUI widget types reduced by 40-50%
2. ✅ **QSS used for 80%+ widget styling** (NEW)
3. ✅ **Qt Property System used for widget configuration** (NEW)
4. ✅ CAN protocol code reduced by 30-40%
5. ✅ **Qt Signals/Slots used for protocol events** (NEW)
6. ✅ Table delegates reduced by 50-70%
7. ✅ **Qt Model/View used for all tables** (NEW)
8. ✅ UI files reduced by 15-20%
9. ✅ **QHash migration complete** (NEW - ~200-300 occurrences)
10. ✅ All tests passing
11. ✅ No visual regressions
12. ✅ No functional regressions

### Qt Framework Integration Metrics

| Qt Feature | Before Phase 2 | After Phase 2 | Adoption |
|------------|----------------|---------------|----------|
| QHash/QSet | ~0% | ~90% | **New** |
| Qt Signals/Slots (Protocols) | ~20% | ~90% | **+70%** |
| Qt Model/View | ~30% | ~90% | **+60%** |
| QSS Styling | ~30% | ~80% | **+50%** |
| Qt Property System | ~10% | ~70% | **+60%** |

### Resource Allocation (Updated)
- **3-4 developers** working in parallel
- **Developer A**: GUI elements (Task 5) - Qt framework patterns
- **Developer B**: CAN protocols (Task 6) - Qt Signals/Slots
- **Developer C**: Table delegates (Task 7) - Qt Model/View
- **Developer D**: UI files (Task 8) + QHash migration (Task 9)

### Task Dependencies (Updated)
```
Phase 1 completion (RECOMMENDED - especially QList migration)
   ↓
   Provides Qt container foundation
   ↓
   ┌────────┬────────┬────────┬────────┐
   │        │        │        │        │
Task 5   Task 6   Task 7   Task 8   Task 9
(GUI)    (Proto)  (Deleg)  (UI)     (Hash)
   │        │        │        │        │
   └────────┴────────┴────────┴────────┘
                    ↓
            Phase 2 Complete
```

All tasks can proceed in parallel (no inter-task dependencies).

---

**Document Status**: Draft
**Next Review**: After Phase 1 completion
**Owner**: Development Team
**Created**: 2026-02-03
