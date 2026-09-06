# QCustomPlot to Qt Graphs Migration Plan

**Target:** Replace QCustomPlot with Qt Graphs (Qt 6.11+)  
**Component:** Dashboard Chart Tab (`C_SyvDaChaPlotHandlerWidget`)  
**Estimated Effort:** 12-18 days (2.5-4 weeks)  
**Status:** Ready for implementation

---

## 1. Current State Analysis

### 1.1 Files Involved
| File | Lines | Purpose |
|------|-------|---------|
| `C_SyvDaChaPlot.hpp/cpp` | 785 | Custom QCustomPlot extension with cursor support |
| `C_SyvDaChaPlotHandlerWidget.hpp/cpp` | 3,434 | Main chart handler with graph management |
| `C_SyvDaChaPlotHandlerWidget.ui` | 1,119 | UI definition with embedded plot widget |
| `C_SyvDaChaDataSelectorWidget` | N/A | Signal selector sidebar (unaffected) |

### 1.2 QCustomPlot Features in Use

#### Core Classes
- `QCustomPlot` - Base plot widget
- `QCPGraph` - Line charts with timestamp data
- `QCPAxis` / `QCPAxisRect` - Multi-axis support (up to 47 Y-axes)
- `QCPItemLine` - Vertical measurement cursors
- `QCPItemTracer` - Square markers on graph points
- `QCPItemText` - Cursor position labels
- `QCPAbstractItem` - Custom triangle cursor markers
- `QCPPainter` - Custom drawing operations
- `QCPScatterStyle` - Data point markers (disc style)
- `QCP::iRangeDrag` / `QCP::iRangeZoom` - Interaction modes
- `QCP::srmZoom` / `QCP::srmNone` - Selection rect modes

#### Custom Extensions
- `C_SyvDaChaPlotTriangleItem` - Custom triangular cursor markers (left/right)
- `C_SyvDaChaPlotCursorTag` - Complex cursor with line + label + triangles
- `C_SyvDaChaPlotCursorItem` - Vertical measurement cursors (1-2 cursors)
- Custom mouse event handling (press/move/release/wheel/keyboard)

#### Key Features
1. **Real-time updates** - 10ms interval chart updates
2. **Multi-axis** - Each graph can have its own Y-axis
3. **Measurement cursors** - 1-2 vertical cursors with time difference calculation
4. **Zoom/pan** - Configurable zoom modes (XY, X, Y)
5. **Dark mode support** - Dynamic theme adaptation
6. **CSV export** - Data extraction functionality
7. **Dynamic signal management** - Add/remove signals at runtime
8. **47 different colors** - Predefined color scheme

---

## 2. Qt Graphs API Mapping

### 2.1 Class Equivalents

| QCustomPlot | Qt Graphs | Notes |
|-------------|-----------|-------|
| `QCustomPlot` | `QGraphsWidget` (Qt Quick) or custom `QWidget` with `QGraphsView` | May need QQuickWidget wrapper |
| `QCPGraph` | `QGraphsLineSeries` | Similar API for line charts |
| `QCPAxis` | `QGraphsAxis` | Axis configuration similar |
| `QCPAxisRect` | `QGraphsChart` | Container for charts |
| `QCPItemLine` | Custom `QWidget` overlay or QML Line | No direct equivalent |
| `QCPItemTracer` | Custom marker overlay | No direct equivalent |
| `QCPItemText` | `QLabel` overlay or QML Text | Overlay approach |
| `QCPAbstractItem` | Custom QPainter overlay | Need custom implementation |
| `QCPPainter` | `QPainter` | Standard Qt painter |
| `QCPScatterStyle` | `QGraphsScatterSeries` or custom | May need custom rendering |

### 2.2 API Differences

#### QCustomPlot (Current)
```cpp
QCPGraph *pc_Graph = mpc_Plot->addGraph(pc_KeyAxis, pc_ValueAxis);
pc_Graph->setData(pc_Keys, pc_Values, true);
pc_Graph->setPen(QPen(color));
pc_Graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));

QCPAxisRect *pc_AxisRect = mpc_Plot->axisRect();
pc_AxisRect->addAxis(QCPAxis::atLeft);
pc_AxisRect->setRangeDrag(Qt::Horizontal | Qt::Vertical);
pc_AxisRect->setRangeZoom(Qt::Horizontal | Qt::Vertical);
```

#### Qt Graphs (Target)
```cpp
QGraphsLineSeries *pc_Series = new QGraphsLineSeries();
pc_Series->append(x, y);  // or setData()
pc_Series->setColor(color);
pc_Series->setMarkerSize(2.0);

QGraphsChart *pc_Chart = new QGraphsChart();
pc_Chart->addSeries(pc_Series);
pc_Chart->addAxis(pc_Axis);
pc_Axis->setRange(min, max);
```

---

## 3. Migration Strategy

### Phase 1: Preparation (Days 1-2)

#### 3.1.1 Install Qt Graphs
```bash
sudo zypper install qt6-graphs-devel
```

#### 3.1.2 Update CMakeLists.txt
```cmake
find_package(Qt6 REQUIRED COMPONENTS Graphs)
target_link_libraries(openSYDE PRIVATE Qt6::Graphs)
```

#### 3.1.3 Create Migration Branch
```bash
git checkout -b feature/migrate-to-qt-graphs
```

#### 3.1.4 Study Qt Graphs Examples
- Review Qt Graphs examples in `/usr/lib64/qt6/examples/graphs/`
- Test basic line chart with Qt Graphs
- Verify multi-axis support

### Phase 2: Core Widget Migration (Days 3-7)

#### 3.2.1 Create New Plot Widget
**File:** `C_SyvDaChaGraphPlot.hpp/cpp` (new)

```cpp
// Replace C_SyvDaChaPlot (extends QCustomPlot)
class C_SyvDaChaGraphPlot : public QWidget
{
    Q_OBJECT
public:
    explicit C_SyvDaChaGraphPlot(QWidget *parent = nullptr);
    ~C_SyvDaChaGraphPlot() override;

    // Core functionality
    void addGraph(QGraphsAxis *keyAxis, QGraphsAxis *valueAxis);
    void setGraphData(int index, const QVector<double> &keys, 
                      const QVector<double> &values, bool asData);
    void clearGraphs();
    
    // Axis management
    QGraphsAxis *axisRect();  // Or create wrapper
    
    // Interaction
    void setRangeDrag(Qt::Orientations orientations);
    void setRangeZoom(Qt::Orientations orientations);
    
    // Styling
    void setDarkMode(bool dark);
    
Q_SIGNALS:
    void sigCursorItemClicked(C_SyvDaChaGraphCursorItem *item);
    void sigCursorItemMovedOnHorizontalAxis(C_SyvDaChaGraphCursorItem *item, 
                                            double posHorizontal);
    void sigCursorItemReleased(C_SyvDaChaGraphCursorItem *item);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphsChart *mpc_Chart;
    QList<QGraphsLineSeries *> mc_Graphs;
    QList<QGraphsAxis *> mc_VerticalAxes;
    // ... cursor items, etc.
};
```

#### 3.2.2 Implement Core Graph Functions
- Graph creation and management
- Data updates (real-time 10ms interval)
- Axis management (multi-axis support)
- Basic styling (colors, line width)

#### 3.2.3 Test Basic Functionality
- Create single graph with data
- Verify real-time updates
- Test zoom/pan interactions

### Phase 3: Cursor System Migration (Days 8-10)

#### 3.3.1 Design Cursor Overlay
**Approach:** Custom QWidget overlay for cursors

```cpp
class C_SyvDaChaGraphCursorItem : public QObject
{
    Q_OBJECT
public:
    C_SyvDaChaGraphCursorItem(QGraphsChart *chart, double initPos, 
                              const QString &labelText);
    ~C_SyvDaChaGraphCursorItem();
    
    void updatePosition(double posHorizontal);
    void updateColors(const QColor &labelColor, const QColor &lineColor);
    void setVisible(bool visible);
    
    // Access to line and label for hit testing
    QRectF lineBoundingRect() const;
    QRectF labelBoundingRect() const;
    
Q_SIGNALS:
    void sigPositionChanged(double pos);
    
private:
    // Custom drawing widget for the vertical line
    QWidget *mpc_LineWidget;
    // Label widget
    QLabel *mpc_Label;
    // Triangle markers
    QWidget *mpc_TriangleLeft;
    QWidget *mpc_TriangleRight;
    
    double md_Position;
    QString mc_LabelText;
};
```

#### 3.3.2 Implement Cursor Hit Testing
- Replace `layerableListAt()` with custom hit testing
- Detect clicks on cursor lines/labels
- Track cursor drag state

#### 3.3.3 Test Cursor Functionality
- Create 1-2 cursors
- Test drag movement
- Verify position updates

### Phase 4: Handler Widget Integration (Days 11-14)

#### 3.4.1 Update C_SyvDaChaPlotHandlerWidget
Replace all `C_SyvDaChaPlot` references with `C_SyvDaChaGraphPlot`

```cpp
// In C_SyvDaChaPlotHandlerWidget.hpp
// Change:
//   C_SyvDaChaPlot *pc_Plot;  // from ui
// To:
//   C_SyvDaChaGraphPlot *pc_Plot;

// Update type references:
// QCPGraph* -> QGraphsLineSeries*
// QCPAxis* -> QGraphsAxis*
// QCPItemTracer* -> Custom marker (or remove)
// QCPItemLine* -> C_SyvDaChaGraphCursorItem*
```

#### 3.4.2 Migrate Key Functions

**Graph Creation:**
```cpp
// OLD (QCustomPlot)
QCPGraph *pc_Graph = this->mpc_Ui->pc_Plot->addGraph(pc_KeyAxis, pc_ValueAxis);
pc_Graph->setPen(QPen(c_Color));
pc_Graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));

// NEW (Qt Graphs)
QGraphsLineSeries *pc_Series = new QGraphsLineSeries();
pc_Series->setColor(c_Color);
pc_Series->setMarkerSize(2.0);
pc_Series->setMarkerShape(QGraphsLineSeries::msCircle);
this->mpc_Ui->pc_Plot->addSeries(pc_Series, pc_KeyAxis, pc_ValueAxis);
```

**Data Update:**
```cpp
// OLD
pc_Graph->setData(pc_Keys, pc_Values, true);

// NEW
pc_Series->clear();
for (int i = 0; i < pc_Keys.size(); ++i) {
    pc_Series->append(pc_Keys[i], pc_Values[i]);
}
// Or use setData() if available in Qt Graphs
```

**Axis Management:**
```cpp
// OLD
QCPAxisRect *pc_AxisRect = this->mpc_Ui->pc_Plot->axisRect();
pc_VerticalAxis = pc_AxisRect->addAxis(QCPAxis::atLeft);

// NEW
QGraphsChart *pc_Chart = this->mpc_Ui->pc_Plot->chart();
pc_VerticalAxis = new QGraphsAxis();
pc_VerticalAxis->setOrientation(QGraphsAxis::Orientation::Vertical);
pc_Chart->addAxis(pc_VerticalAxis);
pc_Chart->setAxisForSeries(pc_Series, pc_VerticalAxis);
```

#### 3.4.3 Test Complete Handler
- Add/remove graphs dynamically
- Test multi-axis configuration
- Verify real-time updates

### Phase 5: Advanced Features (Days 15-17)

#### 3.5.1 Styling and Themes
- Implement dark mode support
- Migrate axis color adaptation
- Grid styling

#### 3.5.2 Zoom and Pan
- Implement zoom in/out functionality
- Configure zoom modes (XY, X, Y)
- Test drag mode vs zoom mode

#### 3.5.3 CSV Export
- Data extraction (should work independently of plotting library)
- File save dialog integration

#### 3.5.4 Measurement Cursors
- Complete cursor creation/deletion
- Time difference calculation
- Visibility management

### Phase 6: Testing and Validation (Days 18-20)

#### 3.6.1 Functional Testing
- Add/remove signals
- Real-time data updates
- Cursor drag functionality
- Zoom/pan operations
- Dark mode switching
- CSV export

#### 3.6.2 Performance Testing
- 10ms update interval stability
- Memory usage with 47 graphs
- CPU usage during real-time updates

#### 3.6.3 Edge Cases
- Empty graphs
- Very large datasets
- Rapid add/remove operations
- Extreme zoom levels

### Phase 7: Cleanup and Documentation (Days 21-22)

#### 3.7.1 Remove Old Code
- Delete `C_SyvDaChaPlot.hpp/cpp` (QCustomPlot wrapper)
- Remove QCustomPlot includes
- Clean up unused QCP types

#### 3.7.2 Update Documentation
- Update AGENTS.md if needed
- Document Qt Graphs usage patterns
- Add migration notes for future reference

#### 3.7.3 Final Commit
```bash
git add .
git commit -m "[DashboardChart] Migrate from QCustomPlot to Qt Graphs"
```

---

## 4. Risk Assessment

### High Risk Areas
1. **Multi-axis support** - Qt Graphs multi-axis API may differ significantly from QCustomPlot's flexible axis system
2. **Custom cursor items** - No direct Qt Graphs equivalent for `QCPAbstractItem` derivatives (triangles, custom tracers)

### Medium Risk Areas
1. **Qt Quick integration** - Qt Graphs is Qt Quick-based; embedding in QWidget hierarchy may require `QQuickWidget` wrapper

### Low Risk Areas
1. **Real-time performance** - Qt Graphs uses hardware acceleration (OpenGL/Vulkan/Metal), which should outperform QCustomPlot's software rendering. The 10ms update interval is more likely constrained by data processing than rendering.
2. **Styling/themes** - Qt Graphs has built-in theme support, though API differs
3. **Data handling** - `QVector<double>` data structures are compatible

### Mitigation Strategies
1. **Multi-axis:** Test multi-axis setup early in Phase 2; may need custom axis wrapper
2. **Cursors:** Use QWidget overlay approach (proven pattern for custom annotations)
3. **Qt Quick integration:** Use `QQuickWidget` for seamless QWidget embedding; test event forwarding

### Fallback Plan
If Qt Graphs migration proves too complex:
- Keep QCustomPlot (single dependency, well-maintained)
- Document why migration was abandoned
- Consider future Qt 6.12+ if Qt Graphs improves

---

## 5. Implementation Checklist

### Phase 1: Preparation
- [ ] Install qt6-graphs-devel package
- [ ] Update CMakeLists.txt with Qt6::Graphs dependency
- [ ] Create feature branch
- [ ] Study Qt Graphs examples

### Phase 2: Core Widget
- [ ] Create C_SyvDaChaGraphPlot widget
- [ ] Implement basic graph creation
- [ ] Implement data updates
- [ ] Implement axis management
- [ ] Test basic functionality

### Phase 3: Cursors
- [ ] Design cursor overlay system
- [ ] Implement C_SyvDaChaGraphCursorItem
- [ ] Implement hit testing
- [ ] Test cursor drag functionality

### Phase 4: Handler Integration
- [ ] Update C_SyvDaChaPlotHandlerWidget types
- [ ] Migrate graph management code
- [ ] Migrate axis management code
- [ ] Test complete workflow

### Phase 5: Advanced Features
- [ ] Implement styling/themes
- [ ] Implement zoom/pan
- [ ] Test CSV export
- [ ] Complete cursor system

### Phase 6: Testing
- [ ] Functional testing
- [ ] Performance profiling
- [ ] Edge case testing
- [ ] Fix identified issues

### Phase 7: Cleanup
- [ ] Remove QCustomPlot code
- [ ] Update documentation
- [ ] Final commit

---

## 6. Code Examples

### 6.1 Basic Graph Setup
```cpp
// Qt Graphs approach
QGraphsLineSeries *series = new QGraphsLineSeries();
series->setColor(Qt::blue);
series->setMarkerSize(2.0);

QGraphsAxis *xAxis = new QGraphsAxis();
xAxis->setTitle("Time");
xAxis->setRange(0, 100);

QGraphsAxis *yAxis = new QGraphsAxis();
yAxis->setTitle("Value");
yAxis->setRange(0, 10);

QGraphsChart *chart = new QGraphsChart();
chart->addSeries(series);
chart->addAxis(xAxis);
chart->addAxis(yAxis);
chart->attachSeries(series, xAxis, yAxis);
```

### 6.2 Real-time Data Update
```cpp
// Efficient data update for real-time
void updateGraphData(QGraphsLineSeries *series, 
                     const QVector<double> &keys,
                     const QVector<double> &values) {
    // Option 1: Clear and rebuild (simple but slower)
    series->clear();
    for (int i = 0; i < keys.size(); ++i) {
        series->append(keys[i], values[i]);
    }
    
    // Option 2: Replace data (if Qt Graphs supports it)
    // series->setData(keys, values);
}
```

### 6.3 Multi-axis Setup
```cpp
// Multiple Y-axes
QGraphsAxis *axis1 = new QGraphsAxis();
axis1->setRange(0, 100);
chart->addAxis(axis1);
chart->attachSeries(series1, xAxis, axis1);

QGraphsAxis *axis2 = new QGraphsAxis();
axis2->setRange(-10, 10);
chart->addAxis(axis2);
chart->attachSeries(series2, xAxis, axis2);
```

---

## 7. Next Steps

1. **Review this plan** - Confirm approach and effort estimates
2. **Install Qt Graphs** - `sudo zypper install qt6-graphs-devel`
3. **Create test project** - Verify basic Qt Graphs functionality
4. **Begin Phase 1** - Start with preparation tasks
5. **Weekly reviews** - Track progress and adjust as needed

---

**Document Created:** 2026-04-04  
**Last Updated:** 2026-04-04  
**Status:** Ready for implementation
