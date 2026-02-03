# Plan: Replace QCustomPlot with Qt Native Charting

**Status**: Planning
**Date Created**: 2026-01-16
**Updated**: 2026-01-16
**Project**: openSYDE Codebase Simplification

---

## Executive Summary

This document outlines options for replacing the third-party QCustomPlot library with Qt-native charting solutions. This continues the codebase simplification effort by eliminating external dependencies.

**IMPORTANT UPDATE**: Qt Charts was deprecated in Qt 6.10. Qt Graphs is the official replacement, but it currently lacks widget-based C++ support (QML-only for 2D).

---

## Qt Charting Landscape (as of Qt 6.10)

| Module | Status | Widget Support | QML Support | Notes |
|--------|--------|----------------|-------------|-------|
| **Qt Charts** | ⚠️ Deprecated (Qt 6.10) | ✅ Yes | ✅ Yes | Still functional but no new development |
| **Qt Graphs** | ✅ Active | ❌ Not yet (2D) | ✅ Yes | Missing: widgets, legends, titles, log axes |
| **QCustomPlot** | Third-party | ✅ Yes | ❌ No | Current solution |

### Qt Graphs Limitations (Qt 6.10)
Per the [Qt Graphs Migration Guide](https://doc.qt.io/qt-6/qtgraphs-migration-guide-2d.html):
- **No widget-based implementations** for 2D charts
- Missing: Candlestick charts, box-and-whiskers, titles, legends, logarithmic axes
- Axes must be defined at GraphsView level (not per-series)
- No automatic axis range calculation

### Implications for openSYDE
The openSYDE application is **QWidget-based**, not QML-based. This means:
1. **Qt Graphs** cannot be used directly without significant architectural changes
2. **Qt Charts** (deprecated) is still the most viable Qt-native option for widgets
3. **QCustomPlot** remains a valid choice until Qt Graphs adds widget support

---

## Current State Analysis

### QCustomPlot Library Overview
- **Location**: `opensyde_tool/libs/qcustomplot/`
- **Files**:
  - `qcustomplot.h` (7,774 lines)
  - `libqcustomplot.a` (precompiled static library)
- **Version**: Unknown (header-only inspection needed)
- **License**: GPL with linking exception (compatible with commercial use)

### Files Using QCustomPlot

| File | Purpose | QCustomPlot Classes Used |
|------|---------|-------------------------|
| [C_SyvDaChaPlot.hpp](opensyde_tool/src/system_views/dashboards/tab_chart/C_SyvDaChaPlot.hpp) | QCustomPlot wrapper/extension | `QCustomPlot`, `QCPAbstractItem`, `QCPItemLine`, `QCPItemPosition`, `QCPItemTracer`, `QCPItemText`, `QCPPainter`, `QCPAxis` |
| [C_SyvDaChaPlot.cpp](opensyde_tool/src/system_views/dashboards/tab_chart/C_SyvDaChaPlot.cpp) | Implementation (787 lines) | Mouse/wheel event handling, cursor items, selection rect |
| [C_SyvDaChaPlotHandlerWidget.hpp](opensyde_tool/src/system_views/dashboards/tab_chart/C_SyvDaChaPlotHandlerWidget.hpp) | Chart widget handler | `QCPAxis`, `QCPItemTracer` |
| [C_SyvDaChaPlotHandlerWidget.cpp](opensyde_tool/src/system_views/dashboards/tab_chart/C_SyvDaChaPlotHandlerWidget.cpp) | Implementation (~2100 lines) | `QCPGraph`, `QCPAxis`, `QCPAxisRect`, `QCPRange`, `QCPScatterStyle`, `QCPItemTracer` |
| [C_NagMainWidget.cpp](opensyde_tool/src/navigable_gui/C_NagMainWidget.cpp) | Main widget | Minimal (likely include only) |
| [CMakeLists.txt](opensyde_tool/pjt/openSYDE/CMakeLists.txt) | Build configuration | Library linkage |

### QCustomPlot API Usage Summary

#### Core Classes Used
1. **QCustomPlot** - Main plot widget (subclassed as `C_SyvDaChaPlot`)
2. **QCPGraph** - Line graph data series
3. **QCPAxis** - X and Y axes (multiple Y axes supported)
4. **QCPAxisRect** - Axis rectangle container
5. **QCPRange** - Axis range management
6. **QCPItemLine** - Line items (cursors)
7. **QCPItemTracer** - Data point tracers
8. **QCPItemText** - Text labels
9. **QCPAbstractItem** - Base class for custom items
10. **QCPPainter** - Custom painting
11. **QCPScatterStyle** - Data point markers
12. **QCPLineEnding** - Line end caps

#### Key Features Used
- **Multiple Y axes** (left side, one per data series)
- **Real-time data streaming** (continuous data addition)
- **Interactive pan/zoom** (drag and wheel events)
- **Measurement cursors** (draggable vertical lines with labels)
- **Data point tracers** (following points on graph)
- **Selection rectangle** (rubberband zoom)
- **Custom items** (triangle markers, cursor tags)
- **Axis range management** (auto-scroll, fit to data)
- **Dark/light mode theming**
- **Sample point markers** (scatter style toggle)

---

## Qt Charts Overview

### Module Information
- **Qt Module**: Qt Charts (`Qt6::Charts`)
- **Since**: Qt 5.7 (significantly improved in Qt 6.x)
- **License**: GPL/LGPL (same as Qt)
- **Documentation**: https://doc.qt.io/qt-6/qtcharts-index.html

### Key Qt Charts Classes

| Qt Charts Class | QCustomPlot Equivalent | Purpose |
|----------------|------------------------|---------|
| `QChart` | `QCustomPlot` | Main chart container |
| `QChartView` | `QCustomPlot` (widget) | Widget to display chart |
| `QLineSeries` | `QCPGraph` | Line graph data |
| `QScatterSeries` | `QCPScatterStyle` | Scatter plot points |
| `QValueAxis` | `QCPAxis` | Numeric axis |
| `QAbstractAxis` | `QCPAxis` (base) | Axis base class |

### Feature Comparison

| Feature | QCustomPlot | Qt Charts | Notes |
|---------|-------------|-----------|-------|
| Multiple Y axes | ✅ Full support | ✅ Full support | Qt Charts supports attachAxis() |
| Real-time data | ✅ Efficient | ✅ Good (Qt 6) | Qt 6 significantly improved |
| Pan/Zoom | ✅ Built-in | ✅ Built-in | Use `setRubberBand()`, `setInteractive()` |
| Custom items | ✅ QCPAbstractItem | ⚠️ Limited | May need QGraphicsItem overlays |
| Performance | ✅ Optimized | ✅ Good (OpenGL) | Qt Charts can use OpenGL |
| Styling | ✅ Full control | ✅ Full control | Different API |
| Data point count | ✅ Millions | ⚠️ Thousands optimal | May need data decimation |

---

## Migration Strategy

### Recommended Approach: Phased Migration

Due to the complexity and deep integration of QCustomPlot features, a phased approach is recommended.

### Phase 1: Infrastructure and Basic Chart

**Goal**: Replace core QCustomPlot widget with QChartView

#### 1.1 Create New Chart Base Class
Create `C_SyvDaChaQtChart` as replacement for `C_SyvDaChaPlot`:

```cpp
// C_SyvDaChaQtChart.hpp
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class C_SyvDaChaQtChart : public QChartView
{
    Q_OBJECT

public:
    explicit C_SyvDaChaQtChart(QWidget* parent = nullptr);
    ~C_SyvDaChaQtChart() override;

    // Graph management
    QLineSeries* addGraph(QValueAxis* yAxis = nullptr);
    void removeGraph(int index);
    int graphCount() const;
    QLineSeries* graph(int index);

    // Axis management
    QValueAxis* xAxis() const;
    QValueAxis* addYAxis();

Q_SIGNALS:
    void SigCursorItemClicked(/*...*/);
    void SigCursorItemMovedOnHorizontalAxis(/*...*/);
    void SigCursorItemReleased(/*...*/);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QChart* m_chart;
    QValueAxis* m_xAxis;
    QList<QLineSeries*> m_series;
    QList<QValueAxis*> m_yAxes;
};
```

#### 1.2 API Mapping

| QCustomPlot Method | Qt Charts Equivalent |
|-------------------|---------------------|
| `addGraph()` | `chart()->addSeries(new QLineSeries())` |
| `graph(i)->setData(keys, values)` | `series->replace(points)` |
| `xAxis->setRange(min, max)` | `axis->setRange(min, max)` |
| `setInteraction(QCP::iRangeDrag)` | `setRubberBand(QChartView::NoRubberBand)` + custom drag |
| `setInteraction(QCP::iRangeZoom)` | `setRubberBand(QChartView::RectangleRubberBand)` |
| `axisRect()->addAxis()` | `chart()->addAxis(axis, Qt::AlignLeft)` |
| `replot()` | Automatic (or `update()`) |

#### 1.3 Files to Modify
- Create: `C_SyvDaChaQtChart.hpp`, `C_SyvDaChaQtChart.cpp`
- Modify: `CMakeLists.txt` (add Qt6::Charts dependency)

---

### Phase 2: Cursor and Measurement System

**Goal**: Implement custom cursor items using QGraphicsItem

#### 2.1 Cursor Item Implementation

Qt Charts uses QGraphicsView internally, so custom items can be added:

```cpp
// C_SyvDaChaCursorItem.hpp
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

class C_SyvDaChaCursorItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit C_SyvDaChaCursorItem(QChart* chart);

    void setPosition(qreal xValue);
    qreal position() const;
    void setColor(const QColor& color);

Q_SIGNALS:
    void positionChanged(qreal newPosition);
    void clicked();
    void released();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QRectF boundingRect() const override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QChart* m_chart;
    qreal m_xValue;
    QColor m_color;
    QGraphicsLineItem* m_line;
    QGraphicsTextItem* m_label;
};
```

#### 2.2 Tracer (Data Point Marker) Implementation

```cpp
// C_SyvDaChaTracerItem.hpp
class C_SyvDaChaTracerItem : public QGraphicsEllipseItem
{
public:
    explicit C_SyvDaChaTracerItem(QLineSeries* series, QChart* chart);

    void setGraphKey(qreal x);
    void updatePosition();
    void setStyle(TracerStyle style);

private:
    QLineSeries* m_series;
    QChart* m_chart;
    qreal m_graphKey;
};
```

#### 2.3 Files to Create/Modify
- Create: `C_SyvDaChaCursorItem.hpp`, `C_SyvDaChaCursorItem.cpp`
- Create: `C_SyvDaChaTracerItem.hpp`, `C_SyvDaChaTracerItem.cpp`
- Modify: `C_SyvDaChaQtChart` to manage cursors

---

### Phase 3: Handler Widget Migration

**Goal**: Update `C_SyvDaChaPlotHandlerWidget` to use new Qt Charts classes

#### 3.1 Member Variable Updates

| Current Member | New Member |
|---------------|------------|
| `QList<QCPAxis*> mc_DataElementConfigIndexToVerticalAxis` | `QList<QValueAxis*> mc_YAxes` |
| `QList<QCPItemTracer*> mc_ItemTracers` | `QList<C_SyvDaChaTracerItem*> mc_Tracers` |
| `C_SyvDaChaPlotCursorItem* mpc_FirstCursor` | `C_SyvDaChaCursorItem* mpc_FirstCursor` |
| `C_SyvDaChaPlot* mpc_Ui->pc_Plot` | `C_SyvDaChaQtChart* mpc_Ui->pc_Chart` |

#### 3.2 Key Method Updates

**AddGraphContent** (data streaming):
```cpp
// Before (QCustomPlot):
QCPGraph* pc_Graph = this->mpc_Ui->pc_Plot->graph(index);
pc_Graph->addData(timestamps, values);

// After (Qt Charts):
QLineSeries* series = this->mpc_Ui->pc_Chart->graph(index);
for (int i = 0; i < timestamps.size(); ++i) {
    series->append(timestamps[i], values[i]);
}
```

**SetScatterStyle** (sample points):
```cpp
// Before (QCustomPlot):
pc_Graph->setScatterStyle(QCPScatterStyle::ssDisc);

// After (Qt Charts):
// Use QScatterSeries overlay or series pointsVisible property
series->setPointsVisible(true);
```

**Range Management**:
```cpp
// Before (QCustomPlot):
this->mpc_Ui->pc_Plot->xAxis->setRange(0.0, 5000.0, Qt::AlignLeft);

// After (Qt Charts):
m_xAxis->setRange(0.0, 5000.0);
```

#### 3.3 Signal/Slot Updates

| QCustomPlot Signal | Qt Charts Equivalent |
|-------------------|---------------------|
| `QCPAxis::rangeChanged(QCPRange)` | `QValueAxis::rangeChanged(qreal min, qreal max)` |
| Custom cursor signals | Custom signals on `C_SyvDaChaCursorItem` |

---

### Phase 4: Performance Optimization

**Goal**: Ensure real-time performance matches or exceeds QCustomPlot

#### 4.1 OpenGL Acceleration
```cpp
// Enable hardware acceleration
m_chart = new QChart();
m_chart->setAnimationOptions(QChart::NoAnimation);

QChartView* view = new QChartView(m_chart);
view->setRenderHint(QPainter::Antialiasing, false);  // For max performance
// OR for better quality with GPU:
view->setRenderHint(QPainter::Antialiasing, true);
```

#### 4.2 Data Decimation for Large Datasets
```cpp
// Implement level-of-detail rendering
void C_SyvDaChaQtChart::updateVisibleData()
{
    // Only render visible data points
    // Decimate points when zoomed out
    qreal pixelsPerPoint = calculatePixelsPerPoint();
    if (pixelsPerPoint < 1.0) {
        // Apply decimation algorithm
        decimateSeries();
    }
}
```

#### 4.3 Batch Updates
```cpp
// Use replace() instead of multiple append() calls
QList<QPointF> newPoints;
for (int i = 0; i < count; ++i) {
    newPoints.append(QPointF(timestamps[i], values[i]));
}
series->replace(newPoints);  // Single update
```

---

### Phase 5: Cleanup and Testing

**Goal**: Remove QCustomPlot and verify all functionality

#### 5.1 Files to Delete
- `opensyde_tool/libs/qcustomplot/qcustomplot.h`
- `opensyde_tool/libs/qcustomplot/libqcustomplot.a`
- `opensyde_tool/src/system_views/dashboards/tab_chart/C_SyvDaChaPlot.hpp`
- `opensyde_tool/src/system_views/dashboards/tab_chart/C_SyvDaChaPlot.cpp`

#### 5.2 Files to Modify
- Remove `qcustomplot.h` includes from:
  - `C_SyvDaChaPlotHandlerWidget.hpp`
  - `C_NagMainWidget.cpp`
- Update `CMakeLists.txt`:
  - Remove QCustomPlot library linkage
  - Ensure Qt6::Charts is linked

#### 5.3 Testing Checklist

| Feature | Test Case |
|---------|-----------|
| Basic chart display | Add single data series, verify rendering |
| Multiple Y axes | Add 3+ data series with different scales |
| Real-time streaming | Stream 1000 points/sec for 5 minutes |
| Pan interaction | Drag chart left/right, up/down |
| Zoom interaction | Wheel zoom, rubberband zoom |
| Cursor placement | Add measurement cursor, drag to position |
| Cursor labels | Verify time values display correctly |
| Difference cursor | Add two cursors, verify delta calculation |
| Data tracers | Verify tracers follow data points |
| Dark/light mode | Toggle theme, verify all colors update |
| Sample points | Toggle scatter style visibility |
| Fit to data | Test "Fit X", "Fit Y", "Fit All" buttons |
| CSV export | Export data, verify format |
| Performance | Test with 100K+ data points |

---

## Risk Assessment

### High Risk Areas

1. **Custom Cursor Items**
   - QCustomPlot has built-in support for custom items
   - Qt Charts requires QGraphicsItem implementation
   - **Mitigation**: Design custom QGraphicsObject subclasses

2. **Real-time Performance**
   - QCustomPlot is highly optimized for streaming data
   - Qt Charts may be slower with very large datasets
   - **Mitigation**: Implement data decimation, use OpenGL

3. **Multiple Y Axes Complexity**
   - QCustomPlot: `axisRect()->addAxis()`
   - Qt Charts: `chart()->addAxis()` + `series->attachAxis()`
   - **Mitigation**: Create wrapper methods to manage axis-series relationships

### Medium Risk Areas

1. **API Differences**
   - Different naming conventions and patterns
   - **Mitigation**: Create adapter layer or thorough refactoring

2. **Coordinate System**
   - QCustomPlot: `coordToPixel()`, `pixelToCoord()`
   - Qt Charts: `mapToValue()`, `mapToPosition()`
   - **Mitigation**: Careful testing of cursor positioning

### Low Risk Areas

1. **Basic Charting**
   - Both libraries have similar core functionality
   - Well-documented Qt Charts API

2. **Styling**
   - Qt Charts has good theming support
   - Can achieve similar visual results

---

## Effort Estimation

| Phase | Complexity | Files Affected | Estimated Effort |
|-------|------------|----------------|------------------|
| Phase 1: Basic Chart | Medium | 4-6 | 2-3 days |
| Phase 2: Cursors/Tracers | High | 4-6 | 3-4 days |
| Phase 3: Handler Widget | High | 2-4 | 3-4 days |
| Phase 4: Performance | Medium | 2-3 | 1-2 days |
| Phase 5: Cleanup/Testing | Low | 5-8 | 2-3 days |
| **Total** | | | **~2-3 weeks** |

---

## Options Analysis

### Option A: Keep QCustomPlot (RECOMMENDED for now)
**Pros**:
- No migration effort
- Already working and tested
- Known performance characteristics
- Full widget support
- Rich feature set (cursors, multiple axes, real-time data)

**Cons**:
- External dependency
- Header-only library (7,774 lines) but minimal footprint

**Verdict**: ✅ Best current option given Qt Graphs limitations

### Option B: Migrate to Qt Charts (Deprecated)
**Pros**:
- Native Qt integration
- Widget-based (QChartView)
- Maintained (bug fixes) but deprecated

**Cons**:
- ⚠️ **Deprecated since Qt 6.10** - no new features
- Will eventually be removed
- Migration effort for uncertain future

**Verdict**: ❌ Not recommended - deprecated technology

### Option C: Migrate to Qt Graphs (Future)
**Pros**:
- Official Qt charting solution going forward
- Modern architecture (hardware-accelerated)
- Active development

**Cons**:
- ❌ **No widget support for 2D** (QML-only)
- Missing: legends, titles, log axes
- Would require QML integration or architecture change
- API still evolving

**Verdict**: ⏳ Wait for widget support (monitor Qt 6.11+)

### Option D: Embed QQuickWidget for Qt Graphs
**Pros**:
- Could use Qt Graphs today
- Future-proof

**Cons**:
- Significant complexity
- Performance overhead of QML in widgets
- Mixed rendering architectures
- Non-trivial integration

**Verdict**: ❌ Over-engineered for current needs

---

## Recommendation

**Option A: Keep QCustomPlot** (for now)

**Rationale**:
1. Qt Graphs lacks widget support for 2D charts (as of Qt 6.10)
2. Qt Charts is deprecated - migrating to deprecated tech is counterproductive
3. QCustomPlot works well and is actively maintained
4. The "external dependency" is minimal (single header + static lib)

**Action Items**:
1. **Monitor Qt Graphs roadmap** for widget support in Qt 6.11+
2. **Revisit this decision** when Qt Graphs adds:
   - Widget-based 2D chart support
   - Legends and titles
   - Feature parity with Qt Charts
3. **Keep QCustomPlot updated** to latest version

**Future Migration Path**:
When Qt Graphs gains widget support:
```
QCustomPlot → Qt Graphs (direct migration)
```
Skip Qt Charts entirely to avoid double migration.

---

## Success Criteria

Migration is complete when:
1. ✅ All chart functionality works identically
2. ✅ No QCustomPlot references remain in codebase
3. ✅ qcustomplot library files deleted
4. ✅ Real-time performance meets requirements
5. ✅ All test cases pass
6. ✅ Dark/light mode theming works correctly
7. ✅ Build succeeds without QCustomPlot

---

## Appendix: Qt Charts Quick Reference

### Adding Qt Charts to CMake
```cmake
find_package(Qt6 REQUIRED COMPONENTS Charts)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Charts)
```

### Basic Chart Setup
```cpp
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

// Create chart
QChart* chart = new QChart();
chart->setTitle("Data Chart");

// Create series
QLineSeries* series = new QLineSeries();
series->append(0, 0);
series->append(1, 1);
chart->addSeries(series);

// Create axes
QValueAxis* axisX = new QValueAxis();
axisX->setTitleText("Time [ms]");
axisX->setRange(0, 5000);
chart->addAxis(axisX, Qt::AlignBottom);
series->attachAxis(axisX);

QValueAxis* axisY = new QValueAxis();
axisY->setTitleText("Value");
chart->addAxis(axisY, Qt::AlignLeft);
series->attachAxis(axisY);

// Create view
QChartView* chartView = new QChartView(chart);
chartView->setRubberBand(QChartView::RectangleRubberBand);
```

### Real-time Data Update
```cpp
// Efficient batch update
QList<QPointF> points;
for (int i = 0; i < newDataCount; ++i) {
    points.append(QPointF(timestamps[i], values[i]));
}
series->append(points);

// Auto-scroll
if (autoScroll) {
    qreal maxX = series->at(series->count() - 1).x();
    qreal range = axisX->max() - axisX->min();
    axisX->setRange(maxX - range, maxX);
}
```

---

## References

- [Qt Charts Documentation](https://doc.qt.io/qt-6/qtcharts-index.html) - Deprecated since Qt 6.10
- [Qt Graphs Documentation](https://doc.qt.io/qt-6/qtgraphs-index.html) - Current solution
- [Qt Graphs 2D Migration Guide](https://doc.qt.io/qt-6/qtgraphs-migration-guide-2d.html) - Migration from Qt Charts
- [QCustomPlot](https://www.qcustomplot.com/) - Current third-party library

---

**Document Version**: 1.1
**Last Updated**: 2026-01-16
**Change Log**:
- v1.1: Updated with Qt Charts deprecation status; revised recommendation to keep QCustomPlot
- v1.0: Initial plan for Qt Charts migration
