# Phase 1: Quick Wins + Qt Container Migration Implementation Plan

**Phase Duration**: 2-3 months (extended for Qt-native migration)
**Target Reduction**: 11,000-15,000 lines (enhanced with Qt containers)
**Risk Level**: Low
**Dependencies**: QString migration already in progress
**Status**: Ready to start
**Qt-Native Priority**: ✅ **HIGH** - Prioritize Qt containers and APIs

---

## Overview

Phase 1 focuses on low-risk, high-impact consolidations combined with Qt-native container migration. These tasks build on existing work (QString migration) and tackle straightforward consolidations with minimal architectural changes.

**✨ Qt-Native Enhancement**: This phase now includes comprehensive migration from STL containers to Qt-native containers:
- **`std::vector` → `QList`**: 6,219 occurrences (including 615 `std::vector<QString>` → `QStringList`)
- **`std::map` → `QMap`/`QHash`**: 640 occurrences (including 70 `std::map<QString, T>` → `QHash<QString, T>`)
- **`std::set` → `QSet`**: 255 occurrences
- **`std::pair` → `QPair`**: 131 occurrences
This improves code consistency, reduces conversion overhead, and leverages Qt 6's optimized implementations.

**Why Combine QString + QList Migration**: Doing both migrations simultaneously avoids touching the same files twice (double-work) and ensures consistent Qt-native code from the start.

---

## Task 1: Complete QString Migration (Phase 3)

### Current Status
- **Lines to Remove**: 3,140 lines (entire SCL library)
- **Migration Progress**: Phase 3 in progress
- **Remaining Work**: ~250 `C_SclString` occurrences in core library

### Affected Files

#### Files to Remove After Migration
```
opensyde_tool/libs/opensyde_core/scl/
├── C_SclString.cpp              (~1,500 lines)
├── C_SclString.hpp
├── C_SclStringList.cpp          (~800 lines)
├── C_SclStringList.hpp
├── C_SclIniFile.cpp             (~600 lines)
├── C_SclIniFile.hpp
└── C_SclChecksums.cpp           (~240 lines - evaluate if needed)
```

#### Areas with Remaining C_SclString Usage
Based on recent commits and codebase patterns:
- Protocol drivers (`opensyde_core/protocol_drivers/`)
- CAN Monitor protocols (`kefex_diaglib/cmonprotocol/`)
- File handlers (various filer classes)
- KEFEX components (`kefex_diaglib/`)
- Diagnostic library components

### Implementation Steps

#### Step 1.1: Audit Remaining Usages
**Action**: Search for all remaining `C_SclString` references
```bash
# Run from project root
grep -r "C_SclString" opensyde_tool/libs/opensyde_core/ --include="*.cpp" --include="*.hpp" > scl_usage_audit.txt
```

**Deliverable**: Complete list of files still using `C_SclString`

#### Step 1.2: Categorize Migration Work
Organize remaining work by subsystem:
1. **Protocol Drivers** (highest priority - actively used)
   - Files: `opensyde_core/protocol_drivers/`
   - Estimate: ~50-80 occurrences

2. **CAN Monitor Protocols** (high priority - user-facing)
   - Files: `kefex_diaglib/cmonprotocol/`
   - Estimate: ~60-100 occurrences

3. **KEFEX Components** (medium priority)
   - Files: `kefex_diaglib/`
   - Estimate: ~40-60 occurrences

4. **File Handlers** (medium priority)
   - Files: Various `*Filer*.cpp` files
   - Estimate: ~30-50 occurrences

5. **Utilities** (low priority)
   - Files: Miscellaneous core library utilities
   - Estimate: ~20-30 occurrences

#### Step 1.3: Execute Migration by Subsystem
For each file, apply these conversions:

**String Creation**
```cpp
// OLD
C_SclString c_Text = "Hello";
C_SclString c_Formatted = C_SclString::IntToStr(42);

// NEW
QString c_Text = "Hello";
QString c_Formatted = QString::number(42);
```

**String Concatenation**
```cpp
// OLD
c_Result = c_Prefix + c_Middle + c_Suffix;

// NEW
c_Result = c_Prefix + c_Middle + c_Suffix;  // Same syntax
```

**String Operations**
```cpp
// OLD
c_Text.Length()
c_Text.c_str()
c_Text.SubString(1, 5)
c_Text.UpperCase()
c_Text.LowerCase()
c_Text.Trim()

// NEW
c_Text.length()
c_Text.toUtf8().constData()  // or .toStdString().c_str()
c_Text.mid(0, 5)  // Note: 0-based indexing in Qt
c_Text.toUpper()
c_Text.toLower()
c_Text.trimmed()
```

**String Comparison**
```cpp
// OLD
if (c_Text == "test") { }
c_Text.AnsiCompareIC("TEST")  // Case-insensitive

// NEW
if (c_Text == "test") { }  // Same syntax
c_Text.compare("TEST", Qt::CaseInsensitive)
```

**Formatting**
```cpp
// OLD
C_SclString::IntToStr(value)
C_SclString::FloatToStr(value)

// NEW
QString::number(value)
QString::number(value, 'f', precision)
```

#### Step 1.4: Handle C_SclStringList Migration
```cpp
// OLD
C_SclStringList c_List;
c_List.Add("item");
c_List.Delete(0);
c_List.Strings[0]

// NEW
QStringList c_List;
c_List.append("item");
c_List.removeAt(0);
c_List[0]
```

#### Step 1.5: Migration Testing Strategy
For each subsystem:
1. **Compile**: Ensure no compilation errors
2. **Unit Tests**: Run existing unit tests for affected components
3. **Integration Tests**: Test affected protocols/features
4. **Manual Testing**:
   - Protocol drivers: Test with real devices/CAN traces
   - CAN Monitor: Test all protocol interpretations
   - File handlers: Test save/load operations

#### Step 1.6: Remove SCL Library
After all migrations complete:

1. **Final verification**:
   ```bash
   grep -r "C_SclString" opensyde_tool/ --include="*.cpp" --include="*.hpp"
   # Should return 0 results (except comments/documentation)
   ```

2. **Evaluate C_SclChecksums**:
   - Check if Qt has equivalent CRC/checksum functions
   - If Qt equivalent exists, migrate
   - If no Qt equivalent, consider keeping as standalone utility or find third-party library
   - Decision: Remove entire SCL directory vs. keep checksums

3. **Remove files**:
   ```bash
   rm -rf opensyde_tool/libs/opensyde_core/scl/
   ```

4. **Update CMakeLists.txt**:
   - Remove SCL source files from `opensyde_core` target
   - Remove SCL include directories

5. **Update includes**:
   - Search for `#include "scl/C_SclString.hpp"` across project
   - Remove all such includes
   - Ensure `#include <QString>` is present where needed

### Risk Mitigation

**Risk**: String encoding differences between C_SclString and QString
- **Mitigation**: Review all file I/O operations, ensure UTF-8 handling is consistent
- **Testing**: Test with non-ASCII characters in filenames and content

**Risk**: API behavior differences (e.g., indexing, substring extraction)
- **Mitigation**: Pay special attention to `SubString()` → `mid()` (different indexing)
- **Testing**: Unit tests for all string manipulation operations

**Risk**: Performance regressions in string-heavy operations
- **Mitigation**: Profile critical paths before/after migration
- **Testing**: Benchmark protocol parsing and file loading

### Success Criteria
- [ ] Zero `C_SclString` references remain in codebase
- [ ] All existing tests pass
- [ ] Manual testing confirms protocol interpretation unchanged
- [ ] File save/load operations work identically
- [ ] SCL directory removed from repository
- [ ] Build completes without SCL-related errors

### Estimated Effort
- **Audit & Planning**: 2-3 days
- **Migration Execution**: 10-15 days (by subsystem)
- **Testing & Verification**: 5-7 days
- **Total**: 17-25 days

---

## Task 1.5: Qt Container Migration (std::vector → QList)

### ✨ NEW: Qt-Native Container Migration

**Rationale**: Prioritize Qt-native solutions for consistency and Qt 6 optimization

**Current State** (as of 2026-02-04):
- 6,219 `std::vector` occurrences in tool codebase
- 615 `std::vector<QString>` occurrences (primary targets for `QStringList`)
- 905 `QList` already in use

**Decision**: Migrate to QList for consistency, Qt integration, and implicit sharing benefits

### Migration Scope

#### Priority 1: std::vector<QString> → QStringList (Combined with Task 1)
**Occurrences**: 493
**Why Now**: These files are already being edited for QString migration - avoid double-work!

**Conversions**:
```cpp
// BEFORE (inconsistent - mixed STL/Qt)
std::vector<C_SclString> mc_Items;      // Old SCL
std::vector<QString> mc_ProcessedItems; // Mixed

// INTERMEDIATE (Phase 1 without container migration - still inconsistent)
std::vector<QString> mc_Items;          // Still STL container
std::vector<QString> mc_ProcessedItems; // Inconsistent with Qt APIs

// AFTER (fully Qt-native - RECOMMENDED)
QStringList mc_Items;                   // Pure Qt
QStringList mc_ProcessedItems;          // Consistent

// Benefits of QStringList:
// - items.join(",")           vs manual loop
// - items.filter("prefix")    vs std::remove_if
// - items << "new" << "items" vs push_back
// - Direct Qt API compatibility (no conversions)
```

#### Priority 2: std::vector<T> → QList<T> (Opportunistic)
**Target**: Files already being edited for QString migration

**Approach**: While migrating QString in a file, also convert `std::vector<T>` → `QList<T>`

**Example**:
```cpp
// File being edited for QString migration
class C_OscNode {
   // OLD
   std::vector<C_SclString> mc_Names;        // QString migration target
   std::vector<C_OscDataPool> mc_DataPools;  // Already in this file

   // NEW (do both migrations at once)
   QStringList mc_Names;                     // QString + QList
   QList<C_OscDataPool> mc_DataPools;       // Opportunistic QList migration
};
```

**Estimated Opportunistic Conversions**: ~400-600 std::vector<T> occurrences

### Implementation Steps

#### Step 1.5.1: Identify std::vector<QString> Locations

**Action**: Search for all `std::vector<QString>` references
```bash
# Find all std::vector<QString> usage
grep -r "std::vector<QString>" opensyde_tool/libs/opensyde_core/ --include="*.cpp" --include="*.hpp" > vector_qstring_usage.txt

# Count occurrences
grep -r "std::vector<QString>" opensyde_tool/libs/opensyde_core/ --include="*.cpp" --include="*.hpp" | wc -l
```

**Deliverable**: Complete list of 493 locations requiring migration

#### Step 1.5.2: Create Migration Strategy per File

**For each file being edited for QString**:

1. **Identify all containers** (not just QString)
2. **Plan conversions**:
   - `std::vector<QString>` → `QStringList` (required)
   - `std::vector<T>` → `QList<T>` (opportunistic if low risk)
3. **Check dependencies**:
   - Function signatures accepting/returning std::vector
   - API compatibility with external code
4. **Execute migration**

#### Step 1.5.3: API Conversions

**Update function signatures**:
```cpp
// OLD API
std::vector<QString> GetNames() const;
void SetNames(const std::vector<QString> & orc_Names);

// NEW API (Qt-native)
QStringList GetNames() const;
void SetNames(const QStringList & orc_Names);
```

**Update member variables**:
```cpp
// OLD
std::vector<QString> mc_Names;
std::vector<C_OscNode> mc_Nodes;

// NEW
QStringList mc_Names;
QList<C_OscNode> mc_Nodes;
```

**Update algorithms** (QList supports STL-style iteration):
```cpp
// Both of these work with QList
for (const auto & item : list) { }           // Range-based for
std::sort(list.begin(), list.end());         // STL algorithms

// But Qt also provides:
list.sort();                                  // Qt method
if (list.contains(item)) { }                 // Qt method (cleaner than std::find)
int index = list.indexOf(item);              // Qt method
```

#### Step 1.5.4: QStringList-Specific Improvements

**Leverage QStringList's powerful APIs**:

```cpp
// String joining (common operation)
// OLD (verbose)
std::vector<QString> items;
QString result;
for (size_t i = 0; i < items.size(); ++i) {
   if (i > 0) result += ",";
   result += items[i];
}

// NEW (one line)
QStringList items;
QString result = items.join(",");

// String filtering
// OLD (complex)
std::vector<QString> filtered;
std::copy_if(items.begin(), items.end(), std::back_inserter(filtered),
   [](const QString & s) { return s.startsWith("prefix"); });

// NEW (readable)
QStringList filtered = items.filter("prefix");

// String replacement across all items
// OLD (loop)
for (auto & item : items) {
   item.replace("old", "new");
}

// NEW (built-in)
items.replaceInStrings("old", "new");
```

**Code Reduction**: These Qt idioms alone save ~1,000-2,000 lines

#### Step 1.5.5: Handle External Interfaces

**For public APIs** used by external code:

**Option A: Direct migration** (if external code can be updated):
```cpp
// Just change the signature
QStringList GetFiles() const;  // Updated
```

**Option B: Adapter layer** (if external code cannot change):
```cpp
// Keep old signature, convert internally
std::vector<QString> GetFiles() const {
   return mc_Files.toStdVector();  // Explicit conversion
}

// Internal: use Qt containers
private:
   QStringList mc_Files;  // Qt-native internally
```

**Option C: Deprecate old API** (for gradual migration):
```cpp
// New Qt-native API
QStringList GetFileList() const { return mc_Files; }

// Old API - deprecated
[[deprecated("Use GetFileList() instead")]]
std::vector<QString> GetFiles() const {
   return mc_Files.toStdVector();
}
```

#### Step 1.5.6: Testing Strategy

**Unit Tests**:
```cpp
// Test QList compatibility
TEST(ContainerMigrationTest, QListBasicOperations) {
   QList<int> list = {1, 2, 3};
   list.append(4);
   EXPECT_EQ(4, list.size());
   EXPECT_EQ(1, list[0]);
}

// Test QStringList operations
TEST(ContainerMigrationTest, QStringListJoin) {
   QStringList items = {"a", "b", "c"};
   EXPECT_EQ("a,b,c", items.join(","));
}

// Test implicit sharing (copy-on-write)
TEST(ContainerMigrationTest, ImplicitSharing) {
   QList<QString> list1 = {"item1", "item2"};
   QList<QString> list2 = list1;  // No copy yet
   list2.append("item3");         // NOW it copies
   EXPECT_EQ(2, list1.size());    // Original unchanged
   EXPECT_EQ(3, list2.size());    // Modified copy
}
```

**Integration Tests**:
- Ensure all QString + QList migrations work together
- Test file I/O operations (load/save)
- Test protocol parsing with QStringList
- Verify no performance regressions

**Compatibility Tests**:
- Test external library interfaces (if adapters used)
- Verify all STL algorithms still work with QList
- Check template code compiles with QList

### Qt 6 Performance Notes

**QList in Qt 6** (important context):
- Qt 6 unified QList and QVector (they're now the same)
- QList now uses contiguous memory (like std::vector)
- Performance is **equivalent** to std::vector for most operations
- Added benefit: **Implicit sharing** (copy-on-write)

**From Qt documentation**:
> "In Qt 6, QList is now the recommended container for all use cases. It provides
> the best performance characteristics and is implemented using a contiguous
> memory layout, just like std::vector."

**Performance Comparison**:

| Operation | std::vector | QList (Qt 6) | Notes |
|-----------|-------------|--------------|-------|
| Random access | O(1) | O(1) | Equivalent |
| Append | O(1) amortized | O(1) amortized | Equivalent |
| Insert at beginning | O(n) | O(n) | Equivalent |
| Copy | O(n) | O(1) | **QList wins** (implicit sharing) |
| Iteration | Fast | Fast | Equivalent |
| Memory layout | Contiguous | Contiguous | Equivalent |

**Winner**: QList (same performance + implicit sharing benefits)

### Risk Mitigation

**Risk**: Template compilation issues with QList
- **Mitigation**: QList supports same iterator concepts as std::vector
- **Testing**: Compile with all warnings enabled, fix incrementally

**Risk**: External library incompatibility
- **Mitigation**: Use adapter layers for external interfaces
- **Testing**: Test all external library integrations

**Risk**: Performance regression in critical paths
- **Mitigation**: Qt 6 QList is performance-equivalent to std::vector
- **Testing**: Benchmark protocol parsing, file I/O before/after

**Risk**: Implicit sharing behavior differences
- **Mitigation**: Document copy-on-write behavior for team
- **Testing**: Unit tests for implicit sharing edge cases

### Success Criteria

- [ ] All `std::vector<QString>` converted to `QStringList` (493 occurrences)
- [ ] Opportunistic `std::vector<T>` → `QList<T>` conversions complete
- [ ] All unit tests passing
- [ ] Integration tests confirm no regressions
- [ ] Performance equivalent or better (benchmarked)
- [ ] External interfaces work correctly (adapters if needed)
- [ ] Code reduction achieved through Qt idioms (~1,000-2,000 lines)
- [ ] Qt container usage increased from 5% to 70%+

### Estimated Effort

**Combined with QString Migration**:
- **std::vector<QString> → QStringList**: 15-20 hours
  - Most files already being edited for QString
  - Marginal additional effort

- **Opportunistic std::vector<T> → QList<T>**: 10-15 hours
  - Only in files already being edited
  - Low-hanging fruit

- **Testing & Verification**: 5-7 hours
  - Unit tests for QList/QStringList
  - Integration tests
  - Performance benchmarks

**Total Additional Effort**: 30-42 hours (~1-2 weeks)

**Note**: This is additional to Task 1 (QString migration), but much less than doing it separately later (which would be 60-80 hours to re-edit same files).

### Code Reduction Impact

**Direct Savings**:
- Qt idioms (join, filter, etc.): ~1,000-1,500 lines
- Cleaner APIs (no STL conversion code): ~500-1,000 lines

**Total Additional Reduction**: ~1,500-2,500 lines

**Combined with Task 1 (QString)**: 3,140 + 3,000 = **~6,140 lines total reduction**

---

## Task 1.6: Qt Map Migration (std::map → QMap/QHash)

### ✨ NEW: Qt-Native Map Migration

**Rationale**: Leverage `QHash` performance for string lookups and `QMap` for sorted collections.

**Current State**:
- 640 `std::map` occurrences in tool codebase
- 70 `std::map<QString, T>` occurrences (primary targets for `QHash`)
- 447 `QMap` already in use

### Migration Scope

#### Priority 1: std::map<QString, T> → QHash<QString, T>
**Why Now**: High performance benefit for lookup-heavy code (e.g., configurations, message definitions).

**Conversions**:
```cpp
// OLD (ordered, slower lookup)
std::map<QString, C_OscSignal> mc_Signals;

// NEW (unordered, faster O(1) lookup)
QHash<QString, C_OscSignal> mc_Signals;
```

#### Priority 2: std::map<K, T> → QMap<K, T>
**Target**: General replacement where order is important or assumed.

### Implementation Steps

1. **Audit Locations**: Identify maps using `QString` or `C_SclString` (to be `QString`) as keys.
2. **Determine Order Requirements**: Check if the code relies on `std::map` sorting.
   - Use `QMap` if sorting is required.
   - Use `QHash` if maximum lookup speed is preferred.
3. **Execute Migration**: Update declarations and use Qt-native methods (`.contains()`, `.value()`, `.insert()`).

### Success Criteria
- [ ] At least 70 `std::map<QString, T>` converted to `QHash`
- [ ] Significant logic simplification using Qt map idioms
- [ ] Performance verification for key lookup paths

---

## Task 1.7: Other STL Type Migration (std::set, std::pair)

### ✨ NEW: Additional STL Type Migration

**Rationale**: Complete the Qt-native transition for all common container types.

**Current State**:
- 255 `std::set` occurrences
- 131 `std::pair` occurrences

### Migration Scope

#### Priority 1: std::set<T> → QSet<T>
- Use `QSet` for faster lookups (hash-based) and better Qt API integration.
- Note: `QSet` requires elements to be hashable (most Qt types are).

#### Priority 2: std::pair<T1, T2> → QPair<T1, T2>
- Mostly a consistency change for cleaner code in Qt-centric files.

### Success Criteria
- [ ] Opportunistic migration of `std::set` and `std::pair` in targeted files.
- [ ] Consistency in container usage across the core libraries.

---

## Task 2: Consolidate Label Widgets

### Current Status
- **Files to Consolidate**: 70 label widget types
- **Target**: Reduce to 15-20 distinct types
- **Lines to Remove**: 2,000-3,000 lines

### Affected Files
```
opensyde_tool/src/opensyde_gui_elements/label/
├── C_OgeLabAdaptiveSize.cpp
├── C_OgeLabContextMenuBase.cpp
├── C_OgeLabDoubleClick.cpp
├── C_OgeLabGenericNoPaddingNoMargins.cpp
├── C_OgeLabHeadingGroupBold.cpp
├── C_OgeLabToolTipBase.cpp
├── ... (64 more label types)
```

### Analysis: Why 70 Label Types?

Most labels differ only in:
1. **Font styling** (bold, italic, size, color)
2. **Behavior** (tooltip support, clickable, adaptive sizing)
3. **Layout** (padding, margins, alignment)
4. **Context** (group heading, message, error, heading)

### Consolidation Strategy

The goal is to move from **C++ hardcoding** to **Property-driven QSS**.
- **Action**: Use standard `QLabel` (or a single `C_OgeLabBase`) plus dynamic Qt properties.
- **Benefit**: 70 classes reduced to ~5-10, styled entirely in `.qss` files.

#### Step 2.1: Audit Current Label Usage
**Action**: Analyze where each label type is used
```bash
# For each label type, find usage count
for file in opensyde_tool/src/opensyde_gui_elements/label/*.cpp; do
    classname=$(basename "$file" .cpp)
    count=$(grep -r "$classname" opensyde_tool/src --include="*.cpp" --include="*.hpp" | wc -l)
    echo "$classname: $count usages"
done > label_usage_stats.txt
```

**Deliverable**: Spreadsheet showing:
- Label class name
- Usage count
- Key characteristics (behavior, styling)
- Consolidation target (which base type it should become)

#### Step 2.2: Design Consolidated Label Architecture

**Base Label Types** (15-20 core types):
```cpp
// Behavioral variants (composable)
C_OgeLabBase                    // Base label
C_OgeLabToolTip                 // With tooltip support
C_OgeLabClickable               // Emits clicked signal
C_OgeLabAdaptiveSize            // Auto-resizes to content
C_OgeLabEliding                 // Text eliding support
C_OgeLabContextMenu             // Context menu support

// Semantic types (styled via QSS)
C_OgeLabHeading                 // Section headings
C_OgeLabSubHeading              // Subsection headings
C_OgeLabGroupTitle              // Group box titles
C_OgeLabMessage                 // General messages
C_OgeLabError                   // Error messages
C_OgeLabWarning                 // Warning messages
C_OgeLabSuccess                 // Success messages
C_OgeLabSecondary               // Secondary/dimmed text
C_OgeLabMonospace               // Monospace font (for IDs, hex values)
```

**Style Variants** (via Qt properties + QSS):
```cpp
// Use Qt property system for style variants
label->setProperty("styleVariant", "bold");
label->setProperty("styleVariant", "italic");
label->setProperty("styleVariant", "heading1");
label->setProperty("styleVariant", "heading2");
label->setProperty("textColor", "primary");
label->setProperty("textColor", "error");
```

#### Step 2.3: Create QSS Styling File
**File**: `opensyde_tool/src/opensyde_gui_elements/styling/labels.qss`

```css
/* Base label styles */
C_OgeLabBase {
    color: #000000;
    font-family: "Segoe UI";
    font-size: 10pt;
}

/* Heading variants via property */
C_OgeLabHeading {
    font-size: 14pt;
    font-weight: bold;
    color: #1A1A1A;
}

C_OgeLabSubHeading {
    font-size: 11pt;
    font-weight: bold;
    color: #404040;
}

/* Style variant properties */
QLabel[styleVariant="bold"] {
    font-weight: bold;
}

QLabel[styleVariant="italic"] {
    font-style: italic;
}

QLabel[styleVariant="heading1"] {
    font-size: 16pt;
    font-weight: bold;
}

QLabel[styleVariant="heading2"] {
    font-size: 12pt;
    font-weight: bold;
}

/* Color variants */
QLabel[textColor="error"] {
    color: #DC0000;
}

QLabel[textColor="warning"] {
    color: #FFA500;
}

QLabel[textColor="success"] {
    color: #008000;
}

QLabel[textColor="secondary"] {
    color: #808080;
}

/* Context-specific styles */
C_OgeLabGroupTitle {
    font-size: 9pt;
    font-weight: bold;
    color: #505050;
}

C_OgeLabMonospace {
    font-family: "Courier New";
    font-size: 9pt;
}
```

#### Step 2.4: Implement Behaviors (Qt-Native Approach PREFERRED)

**✨ Qt-Native Recommendation**: Use Qt Property System + QSS for most behaviors. This leverages the existing infrastructure in `opensyde_tool/src/util/C_UtiStyleSheets.cpp`.

#### Step 2.5: Application-wide Style Management & Hot Reload (NEW)
**Rationale**: Speed up UI development and consolidate the existing ~20 `.qss` files.

1.  **Style Manager**: Enhance `C_UtiStyleSheets` to act as a central hub for all project-wide styles.
2.  **Hot Reload**: Add `QFileSystemWatcher` to monitor `.qss` files during development.
3.  **Variable Consolidation**: Move hardcoded colors from `Color.qss` into a more manageable system (e.g., QSS variables if supported or a central theme file).

**Two Approaches Available**:

1. **Qt Property System** (RECOMMENDED - more Qt-native):
   - Use `setProperty()` for dynamic behavior configuration
   - QSS responds to properties automatically
   - Better Qt Designer integration
   - Less C++ template complexity

2. **C++ Template Mixins** (ALTERNATIVE - for complex behaviors):
   - Use only when Qt properties insufficient
   - For behaviors requiring custom logic (not just styling)

**When to use each**:
- **Qt Properties**: Tooltips, styling variants, simple flags
- **C++ Mixins**: Complex interactive behaviors (drag-drop, custom painting)

---

**Approach 1: Qt Property System (RECOMMENDED)**

**For Tooltips**:
```cpp
// Instead of template mixin
class C_OgeLabHeading : public QLabel {
   Q_OBJECT
   Q_PROPERTY(bool hasTooltip READ hasTooltip WRITE setHasTooltip)

public:
   void setHasTooltip(bool enabled) {
      m_HasTooltip = enabled;
   }

   bool event(QEvent * event) override {
      if (m_HasTooltip && event->type() == QEvent::ToolTip) {
         // Show custom tooltip
      }
      return QLabel::event(event);
   }

private:
   bool m_HasTooltip = false;
};

// Usage (cleaner than template)
label->setProperty("hasTooltip", true);
label->setProperty("tooltipText", "Helpful tip");
```

**For Clickable Behavior**:
```cpp
// Use signals directly (most Qt-native)
class C_OgeLabClickable : public QLabel {
   Q_OBJECT

signals:
   void clicked();

protected:
   void mousePressEvent(QMouseEvent * event) override {
      QLabel::mousePressEvent(event);
      if (event->button() == Qt::LeftButton) {
         emit clicked();
      }
   }
};

// Usage (pure Qt signals/slots)
connect(label, &C_OgeLabClickable::clicked, this, &MyClass::onLabelClicked);
```

---

**Approach 2: C++ Template Mixins (ALTERNATIVE)**

**Only use for complex behaviors that cannot be expressed via properties**

**File**: `opensyde_tool/src/opensyde_gui_elements/label/C_OgeLabBehaviors.hpp`

```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Label behavior mixins for composable functionality (ADVANCED)

   Template mixins for complex behaviors that cannot be handled by Qt Property System.
   PREFER Qt properties when possible - use this only for advanced cases.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELABBEHAVIORS_HPP
#define C_OGELABBEHAVIORS_HPP

#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include "C_OgeToolTipBase.hpp"

namespace stw
{
namespace opensyde_gui_elements
{

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Tooltip behavior mixin

   Adds tooltip functionality to any QLabel-derived class.
*/
//----------------------------------------------------------------------------------------------------------------------
template<typename Base>
class C_OgeLabWithTooltip : public Base, public C_OgeToolTipBase
{
public:
   using Base::Base;  // Inherit constructors

   bool event(QEvent * const opc_Event) override
   {
      bool q_Return = Base::event(opc_Event);
      this->m_ToolTipEvent(opc_Event);
      return q_Return;
   }
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Clickable behavior mixin

   Makes label emit clicked() signal on mouse release.
*/
//----------------------------------------------------------------------------------------------------------------------
template<typename Base>
class C_OgeLabClickable : public Base
{
   Q_OBJECT

public:
   using Base::Base;

Q_SIGNALS:
   void SigClicked();

protected:
   void mouseReleaseEvent(QMouseEvent * const opc_Event) override
   {
      Base::mouseReleaseEvent(opc_Event);
      if (opc_Event->button() == Qt::LeftButton)
      {
         Q_EMIT SigClicked();
      }
   }
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Adaptive sizing behavior mixin

   Auto-resizes label to fit content.
*/
//----------------------------------------------------------------------------------------------------------------------
template<typename Base>
class C_OgeLabAdaptiveSize : public Base
{
public:
   using Base::Base;

   QSize sizeHint() const override
   {
      QSize c_Size = Base::sizeHint();
      // Add adaptive sizing logic
      QFontMetrics c_Metrics(this->font());
      c_Size.setWidth(c_Metrics.horizontalAdvance(this->text()) + 10);
      return c_Size;
   }
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif // C_OGELABBEHAVIORS_HPP
```

#### Step 2.5: Create Consolidated Label Classes
**Example**: `C_OgeLabHeading.hpp`

```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Heading label with optional behaviors

   Provides a heading label with configurable behaviors via template mixins.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELABHEADING_HPP
#define C_OGELABHEADING_HPP

#include <QLabel>

namespace stw
{
namespace opensyde_gui_elements
{

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Base heading label

   Styled via QSS as a section heading. Can be combined with behavior mixins.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgeLabHeading : public QLabel
{
   Q_OBJECT

public:
   explicit C_OgeLabHeading(QWidget * const opc_Parent = nullptr);
};

// Common combinations (pre-composed for convenience)
using C_OgeLabHeadingToolTip = C_OgeLabWithTooltip<C_OgeLabHeading>;
using C_OgeLabHeadingClickable = C_OgeLabClickable<C_OgeLabHeading>;

} // namespace opensyde_gui_elements
} // namespace stw

#endif // C_OGELABHEADING_HPP
```

#### Step 2.6: Migration Plan for Existing Code

**Automated Migration Strategy**:
1. **Create mapping file**: `label_migration_map.json`
   ```json
   {
     "C_OgeLabHeadingGroupBold": {
       "newClass": "C_OgeLabHeading",
       "property": "styleVariant",
       "propertyValue": "bold"
     },
     "C_OgeLabPopUpTitle": {
       "newClass": "C_OgeLabHeading",
       "property": "styleVariant",
       "propertyValue": "heading1"
     }
   }
   ```

2. **Write migration script**: `scripts/migrate_labels.py`
   - Parse C++ files
   - Replace old class names with new classes
   - Add property settings where needed
   - Update includes

3. **Manual review required for**:
   - Labels with complex custom behavior
   - Labels used in .ui files (requires Qt Designer updates)

#### Step 2.7: Update UI Files
For each .ui file using old label types:
1. Open in Qt Designer
2. Replace widget class name
3. Add custom properties if needed
4. Regenerate ui_*.h files

**Alternative**: Write script to update .ui XML directly

#### Step 2.8: Testing Strategy
1. **Visual Regression Testing**:
   - Screenshot all dialogs before migration
   - Screenshot after migration
   - Compare visually
   - Tool: Could use Qt Test framework or manual comparison

2. **Behavioral Testing**:
   - Test tooltip functionality
   - Test clickable labels
   - Test adaptive sizing
   - Test text eliding

3. **Build Verification**:
   - Ensure all files compile
   - No linking errors
   - No runtime warnings

### Risk Mitigation

**Risk**: Visual differences in label appearance
- **Mitigation**: Extensive QSS testing, visual comparison before/after
- **Testing**: Screenshot comparison for all major dialogs

**Risk**: Behavioral regressions (tooltips, clicks)
- **Mitigation**: Unit tests for mixin behaviors
- **Testing**: Manual testing of interactive labels

**Risk**: Breaking changes in API (signal names, methods)
- **Mitigation**: Keep API compatible where possible
- **Testing**: Compilation will catch most issues

### Success Criteria
- [ ] Label widget count reduced from 70 to 15-20
- [ ] All styling moved to QSS
- [ ] Mixin pattern implemented for behaviors
- [ ] Visual appearance unchanged
- [ ] All interactive behaviors work correctly
- [ ] Build succeeds without warnings
- [ ] 2,000-3,000 lines removed

### Estimated Effort
- **Audit & Design**: 3-4 days
- **Implement Consolidated Labels**: 5-7 days
- **Create QSS Styling**: 2-3 days
- **Migration Execution**: 7-10 days
- **Testing**: 5-7 days
- **Total**: 22-31 days

---

## Task 3: Extract Common Filer Utilities

### Current Status
- **Filer Classes**: 32 classes with repetitive XML handling
- **Target**: Extract ~1,500-2,000 lines of common code
- **Pattern**: TinyXML2 API calls repeated across all filers

### Affected Files
```
opensyde_tool/libs/opensyde_core/project/system/
├── C_OscHalcDefFiler.cpp           (2,034 lines)
├── C_OscNodeFiler.cpp              (2,000 lines)
├── C_OscNodeDataPoolFiler.cpp      (1,964 lines)
├── C_OscNodeCommFiler.cpp          (1,468 lines)
└── ... (28 more filer classes)
```

### Analysis: Common Patterns

All Filer classes share these repetitive patterns:

#### Pattern 1: File Loading Boilerplate
```cpp
// Repeated in every Filer class
int32_t LoadFromFile() {
   int32_t s32_Return = C_NO_ERR;

   if (orc_FilePath.IsEmpty() == false) {
      tinyxml2::XMLDocument c_Document;
      if (c_Document.LoadFile(orc_FilePath.c_str()) == tinyxml2::XML_SUCCESS) {
         XMLElement * pc_Root = c_Document.FirstChildElement("opensyde-root");
         if (pc_Root != nullptr) {
            // Parse specific content...
         } else {
            s32_Return = C_CONFIG;
         }
      } else {
         s32_Return = C_RD_WR;
      }
   } else {
      s32_Return = C_RANGE;
   }

   return s32_Return;
}
```

#### Pattern 2: Element Traversal
```cpp
// Repeated for every XML structure
XMLElement * pc_Child = opc_Parent->FirstChildElement("child-name");
if (pc_Child != nullptr) {
   // Process child
} else {
   s32_Return = C_CONFIG;  // Error handling
}
```

#### Pattern 3: Attribute Reading
```cpp
// Repeated for every attribute type
const char_t * pcn_Attribute = pc_Element->Attribute("name");
if (pcn_Attribute != nullptr) {
   orc_Value = pcn_Attribute;
} else {
   s32_Return = C_CONFIG;
}

// For numeric types
uint32_t u32_Value;
if (pc_Element->QueryUnsignedAttribute("count", &u32_Value) == tinyxml2::XML_SUCCESS) {
   orc_Count = static_cast<uint16_t>(u32_Value);
} else {
   s32_Return = C_CONFIG;
}
```

#### Pattern 4: Array Iteration
```cpp
// Repeated for every list/array
for (XMLElement * pc_Item = pc_Parent->FirstChildElement("item");
     pc_Item != nullptr;
     pc_Item = pc_Item->NextSiblingElement("item"))
{
   // Process each item
}
```

### Consolidation Strategy

#### Step 3.1: Design Utility Class Architecture
**File**: `opensyde_tool/libs/opensyde_core/xml_parser/C_OscXmlParserUtil.hpp`

```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       XML parsing utility functions for Filer classes

   Provides common helper functions to reduce boilerplate in XML serialization.
   Handles error checking, type conversions, and common patterns.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXMLPARSERUTIL_HPP
#define C_OSCXMLPARSERUTIL_HPP

#include <QString>
#include <QStringList>
#include "tinyxml2.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"

namespace stw
{
namespace opensyde_core
{

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   XML parsing utilities

   Collection of helper functions for XML serialization/deserialization.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OscXmlParserUtil
{
public:
   // Document loading/saving
   static int32_t h_LoadXmlDocument(const QString & orc_FilePath, tinyxml2::XMLDocument & orc_Document,
                                    const QString & orc_RootElementName);
   static int32_t h_SaveXmlDocument(const QString & orc_FilePath, const tinyxml2::XMLDocument & orc_Document);

   // Element navigation
   static tinyxml2::XMLElement * h_GetChildElement(tinyxml2::XMLElement * const opc_Parent,
                                                    const char_t * const opcn_ChildName, int32_t & ors32_Return);
   static tinyxml2::XMLElement * h_GetOptionalChildElement(tinyxml2::XMLElement * const opc_Parent,
                                                            const char_t * const opcn_ChildName);

   // Attribute reading (with error handling)
   static int32_t h_GetAttributeString(const tinyxml2::XMLElement * const opc_Element,
                                        const char_t * const opcn_AttributeName, QString & orc_Value);
   static int32_t h_GetAttributeUint8(const tinyxml2::XMLElement * const opc_Element,
                                      const char_t * const opcn_AttributeName, uint8_t & oru8_Value);
   static int32_t h_GetAttributeUint16(const tinyxml2::XMLElement * const opc_Element,
                                       const char_t * const opcn_AttributeName, uint16_t & oru16_Value);
   static int32_t h_GetAttributeUint32(const tinyxml2::XMLElement * const opc_Element,
                                       const char_t * const opcn_AttributeName, uint32_t & oru32_Value);
   static int32_t h_GetAttributeInt32(const tinyxml2::XMLElement * const opc_Element,
                                      const char_t * const opcn_AttributeName, int32_t & ors32_Value);
   static int32_t h_GetAttributeBool(const tinyxml2::XMLElement * const opc_Element,
                                     const char_t * const opcn_AttributeName, bool & orq_Value);
   static int32_t h_GetAttributeFloat64(const tinyxml2::XMLElement * const opc_Element,
                                        const char_t * const opcn_AttributeName, float64_t & orf64_Value);

   // Attribute reading (with default values)
   static QString h_GetAttributeStringWithDefault(const tinyxml2::XMLElement * const opc_Element,
                                                   const char_t * const opcn_AttributeName,
                                                   const QString & orc_DefaultValue);
   static uint32_t h_GetAttributeUint32WithDefault(const tinyxml2::XMLElement * const opc_Element,
                                                    const char_t * const opcn_AttributeName,
                                                    const uint32_t ou32_DefaultValue);
   static bool h_GetAttributeBoolWithDefault(const tinyxml2::XMLElement * const opc_Element,
                                             const char_t * const opcn_AttributeName,
                                             const bool oq_DefaultValue);

   // Attribute writing
   static void h_SetAttributeString(tinyxml2::XMLElement * const opc_Element,
                                    const char_t * const opcn_AttributeName, const QString & orc_Value);
   static void h_SetAttributeUint8(tinyxml2::XMLElement * const opc_Element,
                                   const char_t * const opcn_AttributeName, const uint8_t ou8_Value);
   static void h_SetAttributeUint16(tinyxml2::XMLElement * const opc_Element,
                                    const char_t * const opcn_AttributeName, const uint16_t ou16_Value);
   static void h_SetAttributeUint32(tinyxml2::XMLElement * const opc_Element,
                                    const char_t * const opcn_AttributeName, const uint32_t ou32_Value);
   static void h_SetAttributeInt32(tinyxml2::XMLElement * const opc_Element,
                                   const char_t * const opcn_AttributeName, const int32_t os32_Value);
   static void h_SetAttributeBool(tinyxml2::XMLElement * const opc_Element,
                                  const char_t * const opcn_AttributeName, const bool oq_Value);
   static void h_SetAttributeFloat64(tinyxml2::XMLElement * const opc_Element,
                                     const char_t * const opcn_AttributeName, const float64_t of64_Value);

   // Element text content
   static int32_t h_GetTextContent(const tinyxml2::XMLElement * const opc_Element, QString & orc_Text);
   static void h_SetTextContent(tinyxml2::XMLElement * const opc_Element, const QString & orc_Text);

   // Array/list parsing
   static int32_t h_ParseArray(tinyxml2::XMLElement * const opc_Parent, const char_t * const opcn_ItemName,
                               std::function<int32_t(tinyxml2::XMLElement *)> oprc_ParseFunc);
   static tinyxml2::XMLElement * h_CreateArrayElement(tinyxml2::XMLElement * const opc_Parent,
                                                       const char_t * const opcn_ArrayName,
                                                       const char_t * const opcn_ItemName, const uint32_t ou32_Count);

   // Version handling
   static int32_t h_GetFileVersion(const tinyxml2::XMLElement * const opc_Root, uint16_t & oru16_Version);
   static void h_SetFileVersion(tinyxml2::XMLElement * const opc_Root, const uint16_t ou16_Version);

   // Error reporting helpers
   static QString h_GetErrorString(const int32_t os32_Error);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCXMLPARSERUTIL_HPP
```

#### Step 3.2: Implement Core Utility Functions
**File**: `opensyde_tool/libs/opensyde_core/xml_parser/C_OscXmlParserUtil.cpp`

**Key Implementation Examples**:

```cpp
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load XML document from file with validation

   \param[in]   orc_FilePath         Path to XML file
   \param[out]  orc_Document         Loaded document
   \param[in]   orc_RootElementName  Expected root element name

   \return
   C_NO_ERR    Success
   C_RANGE     File path empty
   C_RD_WR     File could not be loaded
   C_CONFIG    Root element not found or wrong name
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserUtil::h_LoadXmlDocument(const QString & orc_FilePath, tinyxml2::XMLDocument & orc_Document,
                                              const QString & orc_RootElementName)
{
   int32_t s32_Return = C_NO_ERR;

   if (orc_FilePath.isEmpty() == false)
   {
      const tinyxml2::XMLError e_Error = orc_Document.LoadFile(orc_FilePath.toUtf8().constData());
      if (e_Error == tinyxml2::XML_SUCCESS)
      {
         const tinyxml2::XMLElement * const pc_Root = orc_Document.FirstChildElement(
            orc_RootElementName.toUtf8().constData());
         if (pc_Root == nullptr)
         {
            s32_Return = C_CONFIG;
            osc_write_log_error("Loading XML", "Root element \"" + orc_RootElementName + "\" not found in file: " +
                                orc_FilePath);
         }
      }
      else
      {
         s32_Return = C_RD_WR;
         osc_write_log_error("Loading XML", "Could not load file: " + orc_FilePath + " (Error: " +
                             QString::number(static_cast<int32_t>(e_Error)) + ")");
      }
   }
   else
   {
      s32_Return = C_RANGE;
      osc_write_log_error("Loading XML", "File path is empty");
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get required child element

   \param[in]      opc_Parent      Parent element
   \param[in]      opcn_ChildName  Child element name
   \param[in,out]  ors32_Return    Error code (set to C_CONFIG if child not found)

   \return
   Pointer to child element (nullptr if not found)
*/
//----------------------------------------------------------------------------------------------------------------------
tinyxml2::XMLElement * C_OscXmlParserUtil::h_GetChildElement(tinyxml2::XMLElement * const opc_Parent,
                                                             const char_t * const opcn_ChildName,
                                                             int32_t & ors32_Return)
{
   tinyxml2::XMLElement * pc_Child = nullptr;

   if (opc_Parent != nullptr)
   {
      pc_Child = opc_Parent->FirstChildElement(opcn_ChildName);
      if (pc_Child == nullptr)
      {
         ors32_Return = C_CONFIG;
         osc_write_log_error("XML Parsing", QString("Required child element \"%1\" not found").arg(opcn_ChildName));
      }
   }
   else
   {
      ors32_Return = C_CONFIG;
   }

   return pc_Child;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get string attribute with error handling

   \param[in]   opc_Element         XML element
   \param[in]   opcn_AttributeName  Attribute name
   \param[out]  orc_Value           Attribute value

   \return
   C_NO_ERR    Success
   C_CONFIG    Attribute not found
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserUtil::h_GetAttributeString(const tinyxml2::XMLElement * const opc_Element,
                                                 const char_t * const opcn_AttributeName, QString & orc_Value)
{
   int32_t s32_Return = C_NO_ERR;

   if (opc_Element != nullptr)
   {
      const char_t * const pcn_Attribute = opc_Element->Attribute(opcn_AttributeName);
      if (pcn_Attribute != nullptr)
      {
         orc_Value = QString::fromUtf8(pcn_Attribute);
      }
      else
      {
         s32_Return = C_CONFIG;
         osc_write_log_error("XML Parsing",
                             QString("Required attribute \"%1\" not found").arg(opcn_AttributeName));
      }
   }
   else
   {
      s32_Return = C_CONFIG;
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Parse array of elements

   Iterates over all child elements with given name and calls parse function for each.

   \param[in]  opc_Parent      Parent element
   \param[in]  opcn_ItemName   Item element name
   \param[in]  oprc_ParseFunc  Function to parse each item (returns error code)

   \return
   C_NO_ERR    Success
   Other       Error from parse function
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserUtil::h_ParseArray(tinyxml2::XMLElement * const opc_Parent,
                                         const char_t * const opcn_ItemName,
                                         std::function<int32_t(tinyxml2::XMLElement *)> oprc_ParseFunc)
{
   int32_t s32_Return = C_NO_ERR;

   if (opc_Parent != nullptr)
   {
      for (tinyxml2::XMLElement * pc_Item = opc_Parent->FirstChildElement(opcn_ItemName);
           pc_Item != nullptr;
           pc_Item = pc_Item->NextSiblingElement(opcn_ItemName))
      {
         s32_Return = oprc_ParseFunc(pc_Item);
         if (s32_Return != C_NO_ERR)
         {
            break;
         }
      }
   }

   return s32_Return;
}
```

#### Step 3.3: Refactor Existing Filer Classes

**Before** (C_OscNodeFiler.cpp - repetitive pattern):
```cpp
int32_t C_OscNodeFiler::h_LoadNodes(std::vector<C_OscNode> & orc_Nodes, const QString & orc_FilePath)
{
   int32_t s32_Return = C_NO_ERR;

   if (orc_FilePath.isEmpty() == false)
   {
      tinyxml2::XMLDocument c_Document;
      if (c_Document.LoadFile(orc_FilePath.toUtf8().constData()) == tinyxml2::XML_SUCCESS)
      {
         tinyxml2::XMLElement * pc_Root = c_Document.FirstChildElement("opensyde-node-definition");
         if (pc_Root != nullptr)
         {
            tinyxml2::XMLElement * pc_Nodes = pc_Root->FirstChildElement("nodes");
            if (pc_Nodes != nullptr)
            {
               for (tinyxml2::XMLElement * pc_Node = pc_Nodes->FirstChildElement("node");
                    pc_Node != nullptr;
                    pc_Node = pc_Node->NextSiblingElement("node"))
               {
                  C_OscNode c_Node;
                  s32_Return = mh_LoadNode(c_Node, pc_Node);
                  if (s32_Return == C_NO_ERR)
                  {
                     orc_Nodes.push_back(c_Node);
                  }
                  else
                  {
                     break;
                  }
               }
            }
            else
            {
               s32_Return = C_CONFIG;
            }
         }
         else
         {
            s32_Return = C_CONFIG;
         }
      }
      else
      {
         s32_Return = C_RD_WR;
      }
   }
   else
   {
      s32_Return = C_RANGE;
   }

   return s32_Return;
}
```

**After** (using utilities):
```cpp
int32_t C_OscNodeFiler::h_LoadNodes(std::vector<C_OscNode> & orc_Nodes, const QString & orc_FilePath)
{
   int32_t s32_Return;
   tinyxml2::XMLDocument c_Document;

   // Load document with validation (replaces 10 lines with 1)
   s32_Return = C_OscXmlParserUtil::h_LoadXmlDocument(orc_FilePath, c_Document, "opensyde-node-definition");

   if (s32_Return == C_NO_ERR)
   {
      tinyxml2::XMLElement * const pc_Root = c_Document.FirstChildElement("opensyde-node-definition");
      tinyxml2::XMLElement * const pc_Nodes = C_OscXmlParserUtil::h_GetChildElement(pc_Root, "nodes", s32_Return);

      if (s32_Return == C_NO_ERR)
      {
         // Parse array with lambda (replaces 12 lines with 4)
         s32_Return = C_OscXmlParserUtil::h_ParseArray(pc_Nodes, "node",
            [&orc_Nodes](tinyxml2::XMLElement * const opc_Node) -> int32_t {
               C_OscNode c_Node;
               const int32_t s32_ParseResult = mh_LoadNode(c_Node, opc_Node);
               if (s32_ParseResult == C_NO_ERR)
               {
                  orc_Nodes.push_back(c_Node);
               }
               return s32_ParseResult;
            });
      }
   }

   return s32_Return;
}
```

**Line Reduction**: ~22 lines → ~17 lines (5 lines saved per function)

#### Step 3.4: Migration Strategy

**Phase 1: Extract utilities** (Week 1)
1. Implement C_OscXmlParserUtil class
2. Add unit tests for utility functions
3. Add to CMakeLists.txt

**Phase 2: Refactor high-value targets** (Weeks 2-3)
Priority order (largest/most repetitive):
1. C_OscNodeFiler.cpp (2,000 lines)
2. C_OscNodeFilerV2.cpp (1,990 lines)
3. C_OscNodeDataPoolFiler.cpp (1,964 lines)
4. C_OscHalcDefFiler.cpp (2,034 lines)
5. C_OscNodeCommFiler.cpp (1,468 lines)

**Phase 3: Refactor remaining filers** (Week 4)
- 27 remaining Filer classes
- Lower line counts but still benefit from utilities

#### Step 3.5: Testing Strategy

**Unit Tests for Utilities**:
```cpp
// Test file: C_OscXmlParserUtilTest.cpp
class C_OscXmlParserUtilTest : public ::testing::Test {
protected:
   tinyxml2::XMLDocument mc_Document;
   tinyxml2::XMLElement * mpc_TestElement;

   void SetUp() override {
      mpc_TestElement = mc_Document.NewElement("test");
      mc_Document.InsertFirstChild(mpc_TestElement);
   }
};

TEST_F(C_OscXmlParserUtilTest, GetAttributeString_Success) {
   mpc_TestElement->SetAttribute("name", "test-value");
   QString c_Value;
   EXPECT_EQ(C_NO_ERR, C_OscXmlParserUtil::h_GetAttributeString(mpc_TestElement, "name", c_Value));
   EXPECT_EQ("test-value", c_Value);
}

TEST_F(C_OscXmlParserUtilTest, GetAttributeString_NotFound) {
   QString c_Value;
   EXPECT_EQ(C_CONFIG, C_OscXmlParserUtil::h_GetAttributeString(mpc_TestElement, "missing", c_Value));
}

// ... more tests for all utility functions
```

**Integration Tests**:
- Use existing Filer unit tests
- Ensure refactored filers load/save identically to originals
- Test file format compatibility (load files created by old code)

**Regression Testing**:
- Load all existing .syde project files
- Verify no parsing errors
- Save and reload, compare XML byte-for-byte

### Risk Mitigation

**Risk**: Behavioral changes in error handling
- **Mitigation**: Utility functions maintain exact same error codes
- **Testing**: Unit tests verify error codes match original behavior

**Risk**: File format incompatibility
- **Mitigation**: Utilities don't change XML structure, only simplify reading/writing
- **Testing**: Load existing project files, verify identical reload

**Risk**: Performance regression
- **Mitigation**: Utility functions inline or highly optimized
- **Testing**: Benchmark large file loading before/after

### Success Criteria
- [ ] C_OscXmlParserUtil utility class implemented
- [ ] Unit tests for all utility functions passing
- [ ] All 32 Filer classes refactored to use utilities
- [ ] Existing project files load identically
- [ ] 1,500-2,000 lines removed
- [ ] Build succeeds without warnings
- [ ] All Filer unit tests pass

### Estimated Effort
- **Design & Implementation**: 3-4 days
- **Unit Tests**: 2-3 days
- **Refactor Filers**: 10-12 days
- **Integration Testing**: 3-4 days
- **Total**: 18-23 days

---

## Task 4: Consolidate Simple Popups

### Current Status
- **Popup Classes**: 20+ specialized popup classes
- **Target**: Reduce to 5-7 generic templates
- **Lines to Remove**: 1,000-1,500 lines

### Affected Files
```
opensyde_tool/src/system_views/system_update/
├── C_SyvUpPacParamSetFileAddPopUp.cpp
├── C_SyvUpPacParamSetFileInfoPopUp.cpp
├── C_SyvUpPacPemFileInfoPopUp.cpp
├── C_SyvUpPacPemFileOptionsPopUp.cpp
└── ...

opensyde_tool/src/navigable_gui/
├── C_NagUnUsedProjectFilesPopUpDialog.cpp
└── ...
```

### Analysis: Popup Patterns

Most popups fall into these categories:
1. **Info Popups**: Display information with OK button
2. **Form Popups**: Collect user input (text fields, checkboxes, etc.)
3. **List Selection Popups**: Select from list of items
4. **Confirmation Popups**: Yes/No/Cancel choices
5. **File Popups**: File information display or selection

### Consolidation Strategy

#### Step 4.1: Design Generic Popup Templates

**Template 1: Info Popup**
```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Generic information popup

   Displays title, message, optional icon, and OK button.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePopUpInfo : public QDialog
{
   Q_OBJECT

public:
   enum E_Icon
   {
      eNO_ICON,
      eINFORMATION,
      eWARNING,
      eERROR,
      eSUCCESS
   };

   C_OgePopUpInfo(QWidget * const opc_Parent = nullptr);

   void SetTitle(const QString & orc_Title);
   void SetMessage(const QString & orc_Message);
   void SetDetailedText(const QString & orc_Details);
   void SetIcon(const E_Icon oe_Icon);
   void SetButtonText(const QString & orc_ButtonText = "OK");

private:
   QLabel * mpc_LabelTitle;
   QLabel * mpc_LabelMessage;
   QLabel * mpc_LabelIcon;
   QTextEdit * mpc_DetailedText;
   QPushButton * mpc_ButtonOk;
};
```

**Template 2: Form Popup**
```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Generic form input popup

   Displays title and form fields with OK/Cancel buttons.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePopUpForm : public QDialog
{
   Q_OBJECT

public:
   struct C_FormField
   {
      QString c_Label;
      QWidget * pc_Widget;  // Line edit, combo box, checkbox, etc.
      bool q_Required;
   };

   C_OgePopUpForm(QWidget * const opc_Parent = nullptr);

   void SetTitle(const QString & orc_Title);
   void AddField(const C_FormField & orc_Field);
   void SetOkButtonEnabled(const bool oq_Enabled);

   QWidget * GetFieldWidget(const QString & orc_Label) const;

private:
   QLabel * mpc_LabelTitle;
   QFormLayout * mpc_FormLayout;
   QPushButton * mpc_ButtonOk;
   QPushButton * mpc_ButtonCancel;
   std::map<QString, QWidget *> mc_Fields;

   void m_OnFieldChanged();
   void m_ValidateForm();
};
```

**Template 3: List Selection Popup**
```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Generic list selection popup

   Displays list of selectable items with OK/Cancel.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePopUpListSelection : public QDialog
{
   Q_OBJECT

public:
   enum E_SelectionMode
   {
      eSINGLE_SELECTION,
      eMULTI_SELECTION
   };

   C_OgePopUpListSelection(QWidget * const opc_Parent = nullptr);

   void SetTitle(const QString & orc_Title);
   void SetItems(const QStringList & orc_Items);
   void SetSelectionMode(const E_SelectionMode oe_Mode);
   void SetSelectedIndices(const std::vector<int32_t> & orc_Indices);

   std::vector<int32_t> GetSelectedIndices() const;
   QStringList GetSelectedItems() const;

private:
   QLabel * mpc_LabelTitle;
   QListWidget * mpc_List;
   QPushButton * mpc_ButtonOk;
   QPushButton * mpc_ButtonCancel;
};
```

**Template 4: Confirmation Popup**
```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Generic confirmation popup

   Displays question with customizable buttons (Yes/No/Cancel combinations).
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePopUpConfirmation : public QDialog
{
   Q_OBJECT

public:
   enum E_Result
   {
      eYES,
      eNO,
      eCANCEL
   };

   C_OgePopUpConfirmation(QWidget * const opc_Parent = nullptr);

   void SetTitle(const QString & orc_Title);
   void SetQuestion(const QString & orc_Question);
   void SetButtons(const QDialogButtonBox::StandardButtons oe_Buttons);
   void SetDefaultButton(const QDialogButtonBox::StandardButton oe_Default);

   E_Result GetResult() const;

private:
   QLabel * mpc_LabelTitle;
   QLabel * mpc_LabelQuestion;
   QDialogButtonBox * mpc_ButtonBox;
   E_Result me_Result;
};
```

**Template 5: File Info Popup**
```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Generic file information popup

   Displays file details in a structured format.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_OgePopUpFileInfo : public QDialog
{
   Q_OBJECT

public:
   struct C_FileInfo
   {
      QString c_Label;
      QString c_Value;
   };

   C_OgePopUpFileInfo(QWidget * const opc_Parent = nullptr);

   void SetTitle(const QString & orc_Title);
   void SetFilePath(const QString & orc_FilePath);
   void AddInfoField(const C_FileInfo & orc_Info);
   void SetShowInExplorer(const bool oq_Show);  // Add "Show in Explorer" button

private:
   QLabel * mpc_LabelTitle;
   QLabel * mpc_LabelFilePath;
   QFormLayout * mpc_InfoLayout;
   QPushButton * mpc_ButtonShowInExplorer;
   QPushButton * mpc_ButtonClose;
};
```

#### Step 4.2: Map Existing Popups to Templates

**Create mapping document**: `popup_consolidation_map.md`

| Old Popup Class | Template | Custom Logic Required |
|----------------|----------|----------------------|
| C_SyvUpPacParamSetFileInfoPopUp | C_OgePopUpFileInfo | None - direct replacement |
| C_SyvUpPacPemFileInfoPopUp | C_OgePopUpFileInfo | None - direct replacement |
| C_SyvUpPacParamSetFileAddPopUp | C_OgePopUpForm | Validation logic for file path |
| C_SyvUpPacPemFileOptionsPopUp | C_OgePopUpForm | Checkbox handlers |
| C_NagUnUsedProjectFilesPopUpDialog | C_OgePopUpListSelection | Custom item rendering |

#### Step 4.3: Implementation Steps

**Step 1**: Implement generic templates (2-3 days)
- Create base popup classes
- Add QSS styling
- Unit tests for each template

**Step 2**: Migrate simple popups (3-4 days)
- Popups that map 1:1 to templates
- No custom logic required
- Examples: File info popups

**Step 3**: Migrate form popups (3-4 days)
- Extract validation logic to separate functions
- Use generic form template
- Examples: Add file, options popups

**Step 4**: Handle special cases (2-3 days)
- Popups with custom widgets
- Complex layouts
- Consider if these need custom classes or can be templated

**Step 5**: Remove old popup classes (1 day)
- Delete obsolete .cpp/.hpp files
- Update CMakeLists.txt
- Remove includes

#### Step 4.4: Example Migration

**Before** - C_SyvUpPacParamSetFileInfoPopUp (150+ lines):
```cpp
class C_SyvUpPacParamSetFileInfoPopUp : public QDialog
{
   Q_OBJECT
public:
   C_SyvUpPacParamSetFileInfoPopUp(const QString & orc_FilePath, QWidget * const opc_Parent = nullptr);

private:
   Ui::C_SyvUpPacParamSetFileInfoPopUp * mpc_Ui;
   QString mc_FilePath;

   void m_InitStaticNames();
   void m_LoadFileInfo();
   void m_OnShowInExplorer();
};

// .cpp file: ~120 lines of boilerplate
C_SyvUpPacParamSetFileInfoPopUp::C_SyvUpPacParamSetFileInfoPopUp(const QString & orc_FilePath,
                                                                  QWidget * const opc_Parent) :
   QDialog(opc_Parent),
   mpc_Ui(new Ui::C_SyvUpPacParamSetFileInfoPopUp),
   mc_FilePath(orc_FilePath)
{
   mpc_Ui->setupUi(this);
   m_InitStaticNames();
   m_LoadFileInfo();
   connect(mpc_Ui->pc_PushButtonShowInExplorer, &QPushButton::clicked,
           this, &C_SyvUpPacParamSetFileInfoPopUp::m_OnShowInExplorer);
   // ... more boilerplate
}
```

**After** - Using generic template (10-15 lines):
```cpp
// In calling code
void ShowParamSetFileInfo(const QString & orc_FilePath)
{
   C_OgePopUpFileInfo c_Popup(this);

   c_Popup.SetTitle("Parameter Set File Information");
   c_Popup.SetFilePath(orc_FilePath);

   // Add file-specific info
   QFileInfo c_FileInfo(orc_FilePath);
   c_Popup.AddInfoField({"File Name", c_FileInfo.fileName()});
   c_Popup.AddInfoField({"File Size", QString::number(c_FileInfo.size()) + " bytes"});
   c_Popup.AddInfoField({"Created", c_FileInfo.birthTime().toString("yyyy-MM-dd hh:mm:ss")});
   c_Popup.AddInfoField({"Modified", c_FileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss")});

   c_Popup.SetShowInExplorer(true);
   c_Popup.exec();
}
```

**Line Reduction**: ~150 lines (class + UI file) → ~15 lines (usage code)

#### Step 4.5: Testing Strategy

**Visual Testing**:
- Screenshot old popups
- Screenshot new popups
- Compare layouts visually
- Ensure no UX regression

**Functional Testing**:
- Test all button interactions
- Verify data collection in forms
- Test list selection modes
- Confirm file operations work

**Integration Testing**:
- Test in context of application workflows
- Verify popup behavior matches original
- Check keyboard navigation (Tab, Enter, Esc)

### Risk Mitigation

**Risk**: Loss of subtle UX features
- **Mitigation**: Carefully document all features of old popups before migration
- **Testing**: Side-by-side comparison of behavior

**Risk**: Breaking existing workflows
- **Mitigation**: Maintain exact same API where called
- **Testing**: Regression test all features that show popups

### Success Criteria
- [ ] 5-7 generic popup templates implemented
- [ ] 20+ specific popup classes replaced
- [ ] Visual appearance matches originals
- [ ] All functional behaviors preserved
- [ ] 1,000-1,500 lines removed
- [ ] No UX regressions reported

### Estimated Effort
- **Design Templates**: 2-3 days
- **Implement Templates**: 3-4 days
- **Migrate Popups**: 5-7 days
- **Testing**: 3-4 days
- **Total**: 13-18 days

---

## Phase 1 Summary

### Total Impact (Qt-Native Enhanced)

| Metric | Original Plan | Qt-Enhanced Plan | Improvement |
|--------|---------------|------------------|-------------|
| **Line Reduction** | 8,100-12,100 | **11,000-15,000** | **+3,000** |
| **Duration** | 6-8 weeks | **8-12 weeks** | +2-4 weeks |
| **Risk** | Low | **Low** | Same |
| **Qt Container %** | ~20% | **~70%** | **+50%** |

**Enhanced Impact Breakdown**:
- QString migration: 3,140 lines
- QList migration: 1,500-2,500 lines (NEW)
- Label consolidation: 2,000-3,000 lines
- Filer utilities: 1,500-2,000 lines
- Popup consolidation: 1,000-1,500 lines
- Qt idioms savings: 1,000-2,000 lines (NEW)

**Total**: 11,140-15,140 lines

### Completion Criteria (Updated)
1. ✅ QString migration complete, SCL library removed
2. ✅ **QList migration complete** - std::vector<QString> → QStringList (493 occurrences)
3. ✅ **Qt container adoption** - Increase from 5% to 70%+ Qt containers
4. ✅ Label widget count reduced from 70 to 15-20
5. ✅ XML parsing utilities extracted and all Filers refactored
6. ✅ Popup classes reduced from 20+ to 5-7 templates
7. ✅ All tests passing
8. ✅ No functional regressions
9. ✅ No performance regressions (QList benchmarked equivalent to std::vector)
10. ✅ Build times improved by 5-10%

### Qt-Native Transformation Metrics

| Metric | Before Phase 1 | After Phase 1 | Change |
|--------|----------------|---------------|--------|
| Qt Containers | ~5% | ~70% | **+65%** |
| QString Usage | ~60% | ~100% | **+40%** |
| QStringList vs std::vector<QString> | Mixed | **100% QStringList** | **Consistent** |

### Task Dependencies (Updated)
```
Task 1: QString Migration
   ↓
Task 1.5: QList Migration (NEW - combined with Task 1)
   ↓
   Dependency: Must do together to avoid rework
   ↓
Task 2: Label Consolidation (Qt Property System emphasis)
   ↓
   No dependencies

Task 3: Filer Utilities (with QDataStream option)
   ↓
   No dependencies

Task 4: Popup Consolidation
   ↓
   No dependencies
```

**Important**: Task 1 and Task 1.5 must be combined (not sequential) to avoid editing same files twice.

Tasks 2-4 can proceed in parallel with Tasks 1/1.5.

### Resource Allocation Recommendation (Updated)
- **2-3 developers** working in parallel
- **Developer A**: QString + QList migration (Tasks 1 + 1.5 combined)
  - These MUST be done together
  - Estimated: 8-10 weeks
- **Developer B**: Label consolidation (Task 2) + Popups (Task 4)
  - Can start immediately (parallel)
  - Estimated: 6-8 weeks
- **Developer C**: Filer utilities (Task 3)
  - Can start immediately (parallel)
  - Estimated: 4-6 weeks

**Critical Path**: Tasks 1 + 1.5 (8-10 weeks) determines phase duration

### Next Steps
1. Review this plan with team
2. Assign developers to tasks
3. Create JIRA/tracking tickets for each task
4. Set up weekly review meetings
5. Begin Task 1 (QString migration completion)

---

**Document Status**: Draft
**Next Review**: Team planning meeting
**Owner**: Development Team
**Created**: 2026-02-03
