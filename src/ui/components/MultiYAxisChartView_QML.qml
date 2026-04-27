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

    // 默认最小缩放范围（防止数据为0时坐标轴太小）
    readonly property real minSpeedScale: 100.0
    readonly property real minTorqueScale: 1.0
    readonly property real minCurrentScale: 1.0
    readonly property real minAngleScale: 10.0

    // 自适应 X 轴范围计算 (时间轴)
    function updateAxisXRange() {
        let minX = Infinity;
        let maxX = -Infinity;
        
        function checkArray(arr) {
            if (!arr || arr.length === 0) return;
            let first = arr[0];
            let last = arr[arr.length - 1];
            if (first && first.x !== undefined) {
                if (first.x < minX) minX = first.x;
            }
            if (last && last.x !== undefined) {
                if (last.x > maxX) maxX = last.x;
            }
        }
        
        checkArray(root.speedData);
        checkArray(root.torqueData);
        checkArray(root.currentData);
        checkArray(root.angleData);
        
        if (minX !== Infinity && maxX !== -Infinity) {
            if (minX === maxX) {
                axisX.min = new Date(minX - 1000);
                axisX.max = new Date(minX + 1000);
            } else {
                axisX.min = new Date(minX);
                axisX.max = new Date(maxX);
            }
        }
    }

    // 自适应 Y 坐标轴范围计算
    function updateAxisRange(axis, data, minScale) {
        if (!data || data.length === 0) return;
        let maxVal = -Infinity;
        let minVal = Infinity;
        for (let i = 0; i < data.length; ++i) {
            let val = data[i].y !== undefined ? data[i].y : data[i];
            if (val > maxVal) maxVal = val;
            if (val < minVal) minVal = val;
        }
        if (maxVal === -Infinity) return;
        
        let range = maxVal - Math.min(minVal, 0);
        if (range === 0) range = minScale;
        
        axis.max = Math.max(maxVal + range * 0.1, minScale);
        if (minVal < 0) {
            axis.min = minVal - range * 0.1;
        } else {
            axis.min = 0;
        }
    }

    // 主图表视图
    ChartView {
        id: chartView
        anchors.fill: parent
        theme: ChartView.ChartThemeLight
        antialiasing: true
        backgroundColor: "transparent"
        plotAreaColor: "transparent"
        legend.visible: false
        
        // X 轴（时间轴，所有系列共用）
        DateTimeAxis {
            id: axisX
            format: "mm:ss.zzz"
            titleText: "时间"
            titleFont.pixelSize: 12
            titleFont.bold: true
            labelsFont.pixelSize: 11
            gridVisible: true
            labelsColor: "#666666"
            color: "#E0E0E0"
            gridLineColor: "#E0E0E0"
        }

        // Y 轴 - 转速（左侧外）
        ValueAxis {
            id: axisYSpeed
            min: 0
            max: root.minSpeedScale
            labelFormat: "%.0f"
            titleText: "转速 (RPM)"
            titleFont.pixelSize: 12
            titleFont.bold: true
            labelsFont.pixelSize: 11
            gridVisible: true
            labelsColor: "#0078D4"
            color: "#0078D4"
            gridLineColor: "#E0E0E0"
            visible: root.speedChannelOn
        }

        // Y 轴 - 扭矩（右侧外）
        ValueAxis {
            id: axisYTorque
            min: 0
            max: root.minTorqueScale
            labelFormat: "%.2f"
            titleText: "扭矩 (N·m)"
            titleFont.pixelSize: 12
            titleFont.bold: true
            labelsFont.pixelSize: 11
            gridVisible: false // 隐藏网格线以避免与左侧网格线交错混乱
            labelsColor: "#E74856"
            color: "#E74856"
            visible: root.torqueChannelOn
        }

        // Y 轴 - 电流（左侧内）
        ValueAxis {
            id: axisYCurrent
            min: 0
            max: root.minCurrentScale
            labelFormat: "%.1f"
            titleText: "电流 (A)"
            titleFont.pixelSize: 12
            titleFont.bold: true
            labelsFont.pixelSize: 11
            gridVisible: false
            labelsColor: "#FF8C00"
            color: "#FF8C00"
            visible: root.currentChannelOn
        }

        // Y 轴 - 角度（右侧内）
        ValueAxis {
            id: axisYAngle
            min: 0
            max: root.minAngleScale
            labelFormat: "%.0f"
            titleText: "角度 (°)"
            titleFont.pixelSize: 12
            titleFont.bold: true
            labelsFont.pixelSize: 11
            gridVisible: false
            labelsColor: "#16C60C"
            color: "#16C60C"
            visible: root.angleChannelOn
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
                    let pt = root.speedData[i];
                    if (pt && pt.x !== undefined) {
                        speedSeries.append(pt.x, pt.y)
                    } else {
                        speedSeries.append(i, pt)
                    }
                }
                root.updateAxisRange(axisYSpeed, root.speedData, root.minSpeedScale)
                root.updateAxisXRange()
            }
        }

        // 扭矩系列
        LineSeries {
            id: torqueSeries
            name: "扭矩"
            axisX: axisX
            axisYRight: axisYTorque
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
                    let pt = root.torqueData[i];
                    if (pt && pt.x !== undefined) {
                        torqueSeries.append(pt.x, pt.y)
                    } else {
                        torqueSeries.append(i, pt)
                    }
                }
                root.updateAxisRange(axisYTorque, root.torqueData, root.minTorqueScale)
                root.updateAxisXRange()
            }
        }

        // 电流系列
        LineSeries {
            id: currentSeries
            name: "电流"
            axisX: axisX
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
                    let pt = root.currentData[i];
                    if (pt && pt.x !== undefined) {
                        currentSeries.append(pt.x, pt.y)
                    } else {
                        currentSeries.append(i, pt)
                    }
                }
                root.updateAxisRange(axisYCurrent, root.currentData, root.minCurrentScale)
                root.updateAxisXRange()
            }
        }

        // 角度系列
        LineSeries {
            id: angleSeries
            name: "角度"
            axisX: axisX
            axisYRight: axisYAngle
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
                    let pt = root.angleData[i];
                    if (pt && pt.x !== undefined) {
                        angleSeries.append(pt.x, pt.y)
                    } else {
                        angleSeries.append(i, pt)
                    }
                }
                root.updateAxisRange(axisYAngle, root.angleData, root.minAngleScale)
                root.updateAxisXRange()
            }
        }
    }
}
