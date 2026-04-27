# Qt Charts 在 QML 中的正确使用方式

## 重要更正

Qt Charts 模块提供了两套 API：

1. **C++ Widget API** (QChart, QChartView, QGraphicsScene)
   - ❌ 不能在 QQuickPaintedItem 中使用
   - ✅ 只能在 QWidget 应用中使用

2. **QML API** (ChartView, LineSeries, ValueAxis)
   - ✅ 可以直接在 QML 中使用
   - ✅ 这是官方支持的方式

## 你的问题根源

你的 MultiYAxisChartView.cpp 试图在 QQuickPaintedItem 中使用 C++ Widget API：

`cpp
// ❌ 错误方式
class MultiYAxisChartView : public QQuickPaintedItem {
    QChart *m_chart;  // Widget API
    QValueAxis *axis; // Widget API
    // ...
}
`

这确实会崩溃，因为 Widget API 需要 QGraphicsScene 上下文。

## 正确方式：使用 Qt Charts 的 QML API

`qml
import QtCharts

ChartView {
    anchors.fill: parent
    theme: ChartView.ChartThemeDark
    antialiasing: true
    
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
        width: 2
    }
}
`

## 关键区别

| 方式 | 可行性 | 说明 |
|------|--------|------|
| QML 中直接用 ChartView | ✅ | 官方支持 |
| QQuickPaintedItem + QChart | ❌ | 架构冲突 |
| QWidget 中用 QChartView | ✅ | 官方支持 |

## 下一步

让我为你创建一个使用 Qt Charts QML API 的正确实现。
