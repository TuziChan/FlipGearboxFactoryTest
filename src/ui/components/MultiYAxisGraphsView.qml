// MultiYAxisGraphsView.qml - 使用 Qt Graphs 实现多 Y 轴图表
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtGraphs

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
    GraphsView {
        id: graphsView
        anchors.fill: parent
        
        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Dark
            backgroundColor: "#1E1E1E"
            gridLineColor: "#3E3E3E"
            labelTextColor: "#FFFFFF"
            plotAreaBackgroundColor: "#2D2D2D"
        }

        // X 轴（时间轴，所有系列共用）
        axisX: ValueAxis {
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
            titleVisible: true
            gridVisible: true
            labelsVisible: true
        }

        // 主 Y 轴（转速）
        axisY: ValueAxis {
            id: axisYSpeed
            min: 0
            max: root.speedScaleMax
            labelFormat: "%.0f"
            titleText: "转速 (RPM)"
            titleVisible: true
            gridVisible: true
            labelsVisible: true
            lineVisible: true
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
            
            // 动态更新数据点
            Component.onCompleted: updateSpeedData()
            Connections {
                target: root
                function onSpeedDataChanged() { speedSeries.updateSpeedData() }
            }
            
            function updateSpeedData() {
                speedSeries.clear()
                for (let i = 0; i < root.speedData.length; i++) {
                    speedSeries.append(i, root.speedData[i])
                }
            }
        }
    }

    // 扭矩图表（叠加显示，使用独立 Y 轴）
    GraphsView {
        anchors.fill: parent
        theme: graphsView.theme
        backgroundColor: "transparent"
        
        axisX: ValueAxis {
            min: axisX.min
            max: axisX.max
            visible: false
        }
        
        axisY: ValueAxis {
            id: axisYTorque
            min: 0
            max: root.torqueScaleMax
            labelFormat: "%.2f"
            titleText: "扭矩 (N·m)"
            titleVisible: true
            labelsVisible: true
            alignment: Qt.AlignRight
            lineVisible: true
        }
        
        LineSeries {
            id: torqueSeries
            name: "扭矩"
            color: "#E74856"
            width: 2
            visible: root.torqueChannelOn
            
            Component.onCompleted: updateTorqueData()
            Connections {
                target: root
                function onTorqueDataChanged() { torqueSeries.updateTorqueData() }
            }
            
            function updateTorqueData() {
                torqueSeries.clear()
                for (let i = 0; i < root.torqueData.length; i++) {
                    torqueSeries.append(i, root.torqueData[i])
                }
            }
        }
    }

    // 电流图表
    GraphsView {
        anchors.fill: parent
        theme: graphsView.theme
        backgroundColor: "transparent"
        
        axisX: ValueAxis {
            min: axisX.min
            max: axisX.max
            visible: false
        }
        
        axisY: ValueAxis {
            id: axisYCurrent
            min: 0
            max: root.currentScaleMax
            visible: false
        }
        
        LineSeries {
            id: currentSeries
            name: "电流"
            color: "#FF8C00"
            width: 2
            visible: root.currentChannelOn
            
            Component.onCompleted: updateCurrentData()
            Connections {
                target: root
                function onCurrentDataChanged() { currentSeries.updateCurrentData() }
            }
            
            function updateCurrentData() {
                currentSeries.clear()
                for (let i = 0; i < root.currentData.length; i++) {
                    currentSeries.append(i, root.currentData[i])
                }
            }
        }
    }

    // 角度图表
    GraphsView {
        anchors.fill: parent
        theme: graphsView.theme
        backgroundColor: "transparent"
        
        axisX: ValueAxis {
            min: axisX.min
            max: axisX.max
            visible: false
        }
        
        axisY: ValueAxis {
            id: axisYAngle
            min: 0
            max: root.angleScaleMax
            visible: false
        }
        
        LineSeries {
            id: angleSeries
            name: "角度"
            color: "#16C60C"
            width: 2
            visible: root.angleChannelOn
            
            Component.onCompleted: updateAngleData()
            Connections {
                target: root
                function onAngleDataChanged() { angleSeries.updateAngleData() }
            }
            
            function updateAngleData() {
                angleSeries.clear()
                for (let i = 0; i < root.angleData.length; i++) {
                    angleSeries.append(i, root.angleData[i])
                }
            }
        }
    }

    // 图例（手动实现，因为多个 GraphsView 叠加）
    Row {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 8
        spacing: 12
        
        Repeater {
            model: [
                { name: "转速", color: "#0078D4", visible: root.speedChannelOn },
                { name: "扭矩", color: "#E74856", visible: root.torqueChannelOn },
                { name: "电流", color: "#FF8C00", visible: root.currentChannelOn },
                { name: "角度", color: "#16C60C", visible: root.angleChannelOn }
            ]
            
            delegate: Row {
                required property var modelData
                visible: modelData.visible
                spacing: 4
                
                Rectangle {
                    width: 12
                    height: 12
                    radius: 6
                    color: modelData.color
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Text {
                    text: modelData.name
                    color: "#FFFFFF"
                    font.pixelSize: 10
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
