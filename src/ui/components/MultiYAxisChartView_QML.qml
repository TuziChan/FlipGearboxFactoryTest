// MultiYAxisChartView.qml - 使用 Qt Charts 的 QML API 实现多 Y 轴图表
pragma ComponentBehavior: Bound

import QtQuick
import QtCharts

Item {
    id: root

    // 属性接口（与 ChartPainter 兼容）
    property var speedData: []
    property var torqueData: []
    property var currentData: []
    property var angleData: []
    property bool speedChannelOn: true
    property bool torqueChannelOn: true
    property bool currentChannelOn: false
    property bool angleChannelOn: true

    // 缩放范围
    readonly property real speedScaleMax: 1600.0
    readonly property real torqueScaleMax: 2.0
    readonly property real currentScaleMax: 4.0
    readonly property real angleScaleMax: 180.0

    // 主图表视图
    ChartView {
        id: chartView
        anchors.fill: parent
        theme: ChartView.ChartThemeDark
        antialiasing: true
        backgroundColor: "#1E1E1E"
        plotAreaColor: "#2D2D2D"
        legend.visible: false
        
        // X 轴（时间轴，所有系列共用）
        ValueAxis {
            id: axisX
            min: 0
            max: Math.max(
                root.speedData.length,
                root.torqueData.length,
                root.currentData.length,
                root.angleData.length,
                100
            )
            labelFormat: "%.0f"
            titleText: "采样点"
            gridVisible: true
            labelsColor: "#FFFFFF"
            color: "#3E3E3E"
        }

        // Y 轴 - 转速（左侧）
        ValueAxis {
            id: axisYSpeed
            min: 0
            max: root.speedScaleMax
            labelFormat: "%.0f"
            titleText: "转速 (RPM)"
            gridVisible: true
            labelsColor: "#0078D4"
            color: "#0078D4"
        }

        // 转速系列
        LineSeries {
            id: speedSeries
            name: "转速"
            axisX: axisX
            axisY: axisYSpeed
            color: "#0078D4"
            width: 2
            visible: root.speedChannelOn
            
            Component.onCompleted: updateData()
            Connections {
                target: root
                function onSpeedDataChanged() { speedSeries.updateData() }
                function onSpeedChannelOnChanged() { speedSeries.visible = root.speedChannelOn }
            }
            
            function updateData() {
                speedSeries.clear()
                for (let i = 0; i < root.speedData.length; i++) {
                    speedSeries.append(i, root.speedData[i])
                }
            }
        }
    }

    // 扭矩图表（叠加，使用独立 Y 轴）
    ChartView {
        anchors.fill: parent
        theme: ChartView.ChartThemeDark
        antialiasing: true
        backgroundColor: "transparent"
        plotAreaColor: "transparent"
        legend.visible: false
        
        // 共享 X 轴范围
        ValueAxis {
            id: axisX2
            min: axisX.min
            max: axisX.max
            visible: false
        }
        
        // Y 轴 - 扭矩（右侧）
        ValueAxis {
            id: axisYTorque
            min: 0
            max: root.torqueScaleMax
            labelFormat: "%.2f"
            titleText: "扭矩 (N·m)"
            labelsColor: "#E74856"
            color: "#E74856"
        }
        
        LineSeries {
            id: torqueSeries
            name: "扭矩"
            axisX: axisX2
            axisY: axisYTorque
            color: "#E74856"
            width: 2
            visible: root.torqueChannelOn
            
            Component.onCompleted: updateData()
            Connections {
                target: root
                function onTorqueDataChanged() { torqueSeries.updateData() }
                function onTorqueChannelOnChanged() { torqueSeries.visible = root.torqueChannelOn }
            }
            
            function updateData() {
                torqueSeries.clear()
                for (let i = 0; i < root.torqueData.length; i++) {
                    torqueSeries.append(i, root.torqueData[i])
                }
            }
        }
    }

    // 电流图表（叠加）
    ChartView {
        anchors.fill: parent
        theme: ChartView.ChartThemeDark
        antialiasing: true
        backgroundColor: "transparent"
        plotAreaColor: "transparent"
        legend.visible: false
        
        ValueAxis {
            id: axisX3
            min: axisX.min
            max: axisX.max
            visible: false
        }
        
        ValueAxis {
            id: axisYCurrent
            min: 0
            max: root.currentScaleMax
            visible: false
        }
        
        LineSeries {
            id: currentSeries
            name: "电流"
            axisX: axisX3
            axisY: axisYCurrent
            color: "#FF8C00"
            width: 2
            visible: root.currentChannelOn
            
            Component.onCompleted: updateData()
            Connections {
                target: root
                function onCurrentDataChanged() { currentSeries.updateData() }
                function onCurrentChannelOnChanged() { currentSeries.visible = root.currentChannelOn }
            }
            
            function updateData() {
                currentSeries.clear()
                for (let i = 0; i < root.currentData.length; i++) {
                    currentSeries.append(i, root.currentData[i])
                }
            }
        }
    }

    // 角度图表（叠加）
    ChartView {
        anchors.fill: parent
        theme: ChartView.ChartThemeDark
        antialiasing: true
        backgroundColor: "transparent"
        plotAreaColor: "transparent"
        legend.visible: false
        
        ValueAxis {
            id: axisX4
            min: axisX.min
            max: axisX.max
            visible: false
        }
        
        ValueAxis {
            id: axisYAngle
            min: 0
            max: root.angleScaleMax
            visible: false
        }
        
        LineSeries {
            id: angleSeries
            name: "角度"
            axisX: axisX4
            axisY: axisYAngle
            color: "#16C60C"
            width: 2
            visible: root.angleChannelOn
            
            Component.onCompleted: updateData()
            Connections {
                target: root
                function onAngleDataChanged() { angleSeries.updateData() }
                function onAngleChannelOnChanged() { angleSeries.visible = root.angleChannelOn }
            }
            
            function updateData() {
                angleSeries.clear()
                for (let i = 0; i < root.angleData.length; i++) {
                    angleSeries.append(i, root.angleData[i])
                }
            }
        }
    }
}
