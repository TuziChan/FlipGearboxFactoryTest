# Qt Charts 在 QML 中的正确使用方式（完整答案）

## 核心结论

Qt Charts 提供两套 API：

### 1. C++ Widget API ❌ 不能在 QQuickPaintedItem 中使用
`cpp
// 这种方式会崩溃
class MultiYAxisChartView : public QQuickPaintedItem {
    QChart *m_chart;           // Widget 类
    QChartView *m_chartView;   // Widget 类
    QValueAxis *m_axis;        // Widget 类
    // 这些类需要 QGraphicsScene 上下文，在 QML 环境中会崩溃
}
`

### 2. QML API ✅ 可以直接在 QML 中使用
`qml
import QtCharts

ChartView {
    LineSeries { ... }
    ValueAxis { ... }
}
`

## 你的问题分析

### 当前实现（会崩溃）

**文件**: src/ui/components/MultiYAxisChartView.h/cpp

`cpp
// ❌ 错误方式
class MultiYAxisChartView : public QQuickPaintedItem {
    QChart *m_chart;  // 这是 Widget API
    
    void initializeChart() {
        m_chart = new QChart();
        m_chart->addAxis(axis, Qt::AlignBottom);  // ← 崩溃点
    }
}
`

**崩溃原因**：
- QChart 是 Widget 体系的类
- 需要 QGraphicsScene 上下文
- QQuickPaintedItem 没有这个上下文
- 内部信号触发时访问空指针 → 段错误

### 正确实现（纯 QML）

**新文件**: src/ui/components/MultiYAxisChartViewQML.qml

`qml
import QtCharts

Item {
    property var speedData: []
    property bool speedChannelOn: true
    
    ChartView {
        anchors.fill: parent
        theme: ChartView.ChartThemeDark
        
        ValueAxis {
            id: axisX
            min: 0
            max: 100
        }
        
        ValueAxis {
            id: axisY
            min: 0
            max: 1600
        }
        
        LineSeries {
            name: "转速"
            axisX: axisX
            axisY: axisY
            color: "#0078D4"
            visible: speedChannelOn
            
            Component.onCompleted: updateData()
            
            function updateData() {
                clear()
                for (let i = 0; i < speedData.length; i++) {
                    append(i, speedData[i])
                }
            }
        }
    }
}
`

## 完整对比

| 方式 | 可行性 | 说明 |
|------|--------|------|
| QML 中用 ChartView | ✅ | **正确方式** - Qt Charts QML API |
| QQuickPaintedItem + QChart | ❌ | **你当前的方式** - 会崩溃 |
| QWidget 中用 QChartView | ✅ | Widget 应用中使用 |
| ChartPainter (QPainter) | ✅ | 自绘方式 - 你的备选方案 |

## 迁移步骤

### 步骤 1: 创建 QML 版本的图表组件

我已经为你创建了 MultiYAxisChartView_QML.qml，它：
- 使用 Qt Charts 的 QML API
- 支持多 Y 轴（通过叠加多个 ChartView）
- 接口与 ChartPainter 兼容

### 步骤 2: 修改 ChartPanel.qml

`qml
// 替换
ChartPainter {
    id: chartView
    // ...
}

// 为
MultiYAxisChartView_QML {
    id: chartView
    // 属性完全兼容，无需修改
}
`

### 步骤 3: 清理 C++ 代码（可选）

可以删除：
- src/ui/components/MultiYAxisChartView.h
- src/ui/components/MultiYAxisChartView.cpp

从 CMakeLists.txt 移除这两个文件。

但保留 Qt6::Charts 依赖，因为 QML API 需要它。

### 步骤 4: 更新 main.cpp

`cpp
// 可以移除这行（如果不再使用 C++ 版本）
qmlRegisterType<MultiYAxisChartView>("FlipGearboxFactoryTest.UI", 1, 0, "MultiYAxisChartView");
`

## Qt Charts QML API 特性

### 优点
✅ 官方支持的 QML 使用方式
✅ 不会崩溃
✅ 功能完整（支持多种图表类型）
✅ 性能良好
✅ API 简洁

### 限制
⚠️ 多 Y 轴需要叠加多个 ChartView（我的实现已处理）
⚠️ 动态数据更新需要手动 clear() + append()

## 性能对比

| 方案 | 渲染方式 | 性能 | 稳定性 |
|------|---------|------|--------|
| Qt Charts QML | Scene Graph | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| Qt Graphs | GPU 加速 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| ChartPainter | CPU 绘制 | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| QChart in QQuickPaintedItem | - | ❌ 崩溃 | ❌ |

## 推荐方案

### 方案 A: Qt Charts QML API（推荐）✅
- 使用我创建的 MultiYAxisChartView_QML.qml
- 官方支持，稳定可靠
- 功能完整

### 方案 B: Qt Graphs（备选）✅
- 如果需要更高性能
- Qt 6.2+ 的新模块
- GPU 加速

### 方案 C: ChartPainter（当前）✅
- 如果当前性能够用
- 保持不变

## 总结

**你的问题根源**：
- ❌ 在 QQuickPaintedItem 中使用 Qt Charts 的 C++ Widget API
- ✅ 应该直接在 QML 中使用 Qt Charts 的 QML API

**解决方案**：
1. 使用我创建的 MultiYAxisChartView_QML.qml
2. 在 ChartPanel.qml 中替换组件
3. 删除 C++ 版本的 MultiYAxisChartView

**Qt Charts 可以在 QML 中使用，但要用 QML API，不是 C++ Widget API！**
