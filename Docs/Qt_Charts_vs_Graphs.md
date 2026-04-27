# Qt Charts vs Qt Graphs 在 QML 中的使用指南

## 核心结论

**Qt Charts 模块无法在 QML/QQuickPaintedItem 中正常使用。**

## 技术原因

### Qt Charts (QtCharts)
- **体系**: Widget 体系
- **基础**: QGraphicsView/QGraphicsScene/QWidget
- **渲染**: Widget 渲染管线
- **适用**: QWidget 应用程序
- **QML 兼容性**: ❌ 不兼容

### Qt Graphs (QtGraphs) 
- **体系**: QML/Qt Quick 原生
- **基础**: Scene Graph + RHI (Rendering Hardware Interface)
- **渲染**: OpenGL/Vulkan/Metal/Direct3D
- **适用**: QML/Qt Quick 应用程序
- **QML 兼容性**: ✅ 完全兼容

## 崩溃原因详解

当在 QQuickPaintedItem 中使用 Qt Charts 时：

1. QChart::addAxis() 触发内部信号
2. 信号处理尝试访问 QGraphicsScene 上下文
3. QQuickPaintedItem 的 FBO 环境中没有 Widget 上下文
4. 空指针访问 → 段错误崩溃

即使延迟初始化、分阶段创建也无法解决，因为这是架构层面的不兼容。

## 解决方案

### 方案 1: Qt Graphs（推荐）✅

使用 Qt 6.x 专门为 QML 设计的图表模块。

#### 优点
- QML 原生支持
- 性能优秀（GPU 加速）
- API 现代化
- 支持多种图表类型

#### 缺点
- API 与 Qt Charts 不同，需要迁移
- Qt 6.2+ 才有（你用的是 6.11，完全支持）

### 方案 2: ChartPainter（当前方案）✅

使用纯 QPainter 在 QQuickPaintedItem 中绘制。

#### 优点
- 完全控制绘制逻辑
- 轻量级，无额外依赖
- 稳定可靠

#### 缺点
- 需要手动实现所有图表功能
- 性能不如 GPU 加速的 Qt Graphs

### 方案 3: QWidget 嵌入（不推荐）⚠️

使用 QQuickWidget 或 QWidget::createWindowContainer()。

#### 缺点
- 性能差
- 渲染问题多
- 架构混乱
- 不推荐用于生产环境

## 推荐实现：Qt Graphs

### 基本用法

\\\qml
import QtQuick
import QtGraphs

GraphsView {
    anchors.fill: parent
    
    // 主题配置
    theme: GraphsTheme {
        colorScheme: GraphsTheme.ColorScheme.Dark
        backgroundColor: "#1E1E1E"
        gridLineColor: "#3E3E3E"
    }
    
    // X 轴
    axisX: ValueAxis {
        min: 0
        max: 100
        labelFormat: "%.0f"
        titleText: "时间 (ms)"
    }
    
    // Y 轴
    axisY: ValueAxis {
        min: 0
        max: 1600
        labelFormat: "%.0f"
        titleText: "转速 (RPM)"
    }
    
    // 数据系列
    LineSeries {
        name: "转速"
        color: "#0078D4"
        width: 2
        
        // 动态数据绑定
        XYPoint { x: 0; y: speedData[0] || 0 }
        XYPoint { x: 1; y: speedData[1] || 0 }
        // ... 更多数据点
    }
}
\\\

### 多 Y 轴实现

Qt Graphs 支持多 Y 轴，但需要使用多个 GraphsView 叠加或使用自定义实现。

## 迁移建议

### 从 MultiYAxisChartView 迁移到 Qt Graphs

1. **移除 Qt Charts 依赖**
   - 删除 MultiYAxisChartView.h/cpp
   - 从 CMakeLists.txt 移除 Qt6::Charts

2. **创建 Qt Graphs 组件**
   - 使用纯 QML 实现
   - 利用 GraphsView + LineSeries

3. **数据绑定**
   - Qt Graphs 支持动态数据更新
   - 使用 QML 属性绑定即可

### 从 ChartPainter 迁移到 Qt Graphs

如果当前 ChartPainter 性能足够，可以保持不变。
如果需要更好的性能和交互，迁移到 Qt Graphs。

## 性能对比

| 方案 | 渲染方式 | 性能 | 功能完整度 |
|------|---------|------|-----------|
| Qt Graphs | GPU 加速 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| ChartPainter | CPU 绘制 | ⭐⭐⭐ | ⭐⭐⭐ |
| Qt Charts (Widget) | CPU 绘制 | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| Qt Charts (QML) | ❌ 崩溃 | ❌ | ❌ |

## 结论

**在 QML 应用中，必须使用 Qt Graphs，不能使用 Qt Charts。**

你的项目已经引入了 Qt Graphs 模块，建议：
1. 如果需要高性能和丰富功能 → 使用 Qt Graphs
2. 如果需求简单，当前方案够用 → 保持 ChartPainter
3. 绝对不要尝试在 QML 中使用 Qt Charts

## 参考资源

- Qt Graphs 官方文档: https://doc.qt.io/qt-6/qtgraphs-index.html
- Qt Graphs 示例: https://doc.qt.io/qt-6/qtgraphs-examples.html
