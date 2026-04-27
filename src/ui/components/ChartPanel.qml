pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs
import FlipGearboxFactoryTest.UI 1.0

Rectangle {
    id: root

    required property AppTheme theme
    required property bool running
    required property var speedData
    required property var torqueData
    required property var currentData
    required property var angleData
    required property bool speedChannelOn
    required property bool torqueChannelOn
    required property bool currentChannelOn
    required property bool angleChannelOn
    required property var onToggleChannel
    property var magnetMarkers: [] // Array of {angle: number, detected: bool}
    property string anomalyMessage: ""
    property string anomalyType: "" // "timeout", "disconnect", "data_error"
    property string chartBackendMode: typeof chartBackend !== "undefined" ? String(chartBackend).toLowerCase() : "qtgraphs"
    readonly property bool useQtGraphs: chartBackendMode !== "painter"
    readonly property var activeChartItem: chartLoader.item
    readonly property var activeGraphsView: root.useQtGraphs && activeChartItem ? activeChartItem.graphsViewRef : null
    readonly property var activeChartPainter: !root.useQtGraphs && activeChartItem ? activeChartItem.chartPainterRef : null

    readonly property real speedScaleMax: 1600.0
    readonly property real torqueScaleMax: 2.0
    readonly property real currentScaleMax: 4.0
    readonly property real angleScaleMax: 180.0

    radius: root.theme.radiusLarge
    color: root.theme.cardColor
    border.color: root.theme.dividerColor
    border.width: 1
    implicitHeight: 320

    Component.onCompleted: console.log("[StartupTrace] ChartPanel completed backend=" + chartBackendMode)

    function normalizeValue(value, maxValue) {
        const numeric = Number(value)
        if (isNaN(numeric) || maxValue <= 0)
            return 0
        const normalized = numeric / maxValue * 100.0
        return Math.max(0, Math.min(100, normalized))
    }

    function visibleSampleCount() {
        return Math.max(
                    root.speedData ? root.speedData.length : 0,
                    root.torqueData ? root.torqueData.length : 0,
                    root.currentData ? root.currentData.length : 0,
                    root.angleData ? root.angleData.length : 0,
                    2)
    }

    function repopulateSeries(series, values, maxValue, enabled) {
        if (!series)
            return
        series.clear()
        if (!enabled || !values)
            return

        for (let i = 0; i < values.length; ++i)
            series.append(i, normalizeValue(values[i], maxValue))
    }

    function refreshChart() {
        if (!root.useQtGraphs)
            return

        const chartItem = root.activeChartItem
        const axisX = chartItem ? chartItem.xAxisRef : null
        const axisY = chartItem ? chartItem.yAxisRef : null
        if (!axisX || !axisY)
            return

        axisX.min = 0
        axisX.max = visibleSampleCount() - 1
        axisY.min = 0
        axisY.max = 100

        repopulateSeries(chartItem.speedSeriesRef, root.speedData, root.speedScaleMax, root.speedChannelOn)
        repopulateSeries(chartItem.torqueSeriesRef, root.torqueData, root.torqueScaleMax, root.torqueChannelOn)
        repopulateSeries(chartItem.currentSeriesRef, root.currentData, root.currentScaleMax, root.currentChannelOn)
        repopulateSeries(chartItem.angleSeriesRef, root.angleData, root.angleScaleMax, root.angleChannelOn)
    }

    function hasAnyData() {
        return (root.speedData && root.speedData.length > 0)
                || (root.torqueData && root.torqueData.length > 0)
                || (root.currentData && root.currentData.length > 0)
                || (root.angleData && root.angleData.length > 0)
    }

    onSpeedDataChanged: refreshChart()
    onTorqueDataChanged: refreshChart()
    onCurrentDataChanged: refreshChart()
    onAngleDataChanged: refreshChart()
    onSpeedChannelOnChanged: refreshChart()
    onTorqueChannelOnChanged: refreshChart()
    onCurrentChannelOnChanged: refreshChart()
    onAngleChannelOnChanged: refreshChart()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "实时波形"
                color: root.theme.textPrimary
                font.pixelSize: 14
                font.bold: true
            }

            Repeater {
                model: [
                    { key: "speed", label: "转速", color: "#0078D4", enabled: root.speedChannelOn },
                    { key: "torque", label: "扭矩", color: "#E74856", enabled: root.torqueChannelOn },
                    { key: "current", label: "电流", color: "#FF8C00", enabled: root.currentChannelOn },
                    { key: "angle", label: "角度", color: "#16C60C", enabled: root.angleChannelOn }
                ]

                delegate: Rectangle {
                    required property var modelData
                    radius: 11
                    color: modelData.enabled ? root.theme.accentLight : root.theme.sectionColor
                    border.color: modelData.enabled ? root.theme.accent : root.theme.dividerColor
                    border.width: 1
                    implicitHeight: 26
                    implicitWidth: 72

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 5

                        Rectangle {
                            Layout.alignment: Qt.AlignVCenter
                            width: 8
                            height: 8
                            radius: 4
                            color: modelData.color
                        }

                        Text {
                            Layout.alignment: Qt.AlignVCenter
                            text: modelData.label
                            color: modelData.enabled ? root.theme.accent : root.theme.textSecondary
                            font.pixelSize: 10
                            font.bold: modelData.enabled
                        }
                    }

                    TapHandler {
                        onTapped: root.onToggleChannel(modelData.key)
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: root.useQtGraphs ? "Qt Graphs" : "Painter"
                color: root.theme.textMuted
                font.pixelSize: 10
            }

            Rectangle {
                visible: root.anomalyMessage !== ""
                radius: 4
                color: root.anomalyType === "timeout" ? "#FFF4E5"
                       : root.anomalyType === "disconnect" ? "#FFE5E5" : "#FFF0E5"
                border.color: root.anomalyType === "timeout" ? "#FF8C00"
                              : root.anomalyType === "disconnect" ? "#E74856" : "#FFB900"
                border.width: 1
                implicitHeight: 24
                implicitWidth: anomalyText.width + 20

                Text {
                    id: anomalyText
                    anchors.centerIn: parent
                    text: root.anomalyMessage
                    color: root.anomalyType === "timeout" ? "#FF8C00"
                           : root.anomalyType === "disconnect" ? "#E74856" : "#FFB900"
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            Text {
                text: root.running ? "采样中" : "尚未开始"
                color: root.running ? root.theme.accent : root.theme.textMuted
                font.pixelSize: 11
                font.bold: root.running
            }
        }

        Rectangle {
            id: chartFrame
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: root.theme.radiusMedium
            color: root.theme.bgSecondary
            border.color: root.theme.dividerColor
            clip: true

            Loader {
                id: chartLoader
                anchors.fill: parent
                active: true
                sourceComponent: root.useQtGraphs ? graphsComponent : painterComponent
                onLoaded: {
                    console.log("[StartupTrace] ChartPanel loader ready backend=" + root.chartBackendMode)
                    root.refreshChart()
                }
            }

            Repeater {
                model: root.magnetMarkers

                delegate: Rectangle {
                    required property var modelData
                    required property int index

                    readonly property real denominator: Math.max(1, root.magnetMarkers.length - 1)
                    readonly property real plotX: root.useQtGraphs && root.activeGraphsView && root.activeGraphsView.plotArea.width > 0
                                                  ? root.activeGraphsView.plotArea.x
                                                  : root.activeChartPainter ? root.activeChartPainter.x : 0
                    readonly property real plotWidth: root.useQtGraphs && root.activeGraphsView && root.activeGraphsView.plotArea.width > 0
                                                      ? root.activeGraphsView.plotArea.width
                                                      : root.activeChartPainter ? root.activeChartPainter.width : 0
                    readonly property real plotY: root.useQtGraphs && root.activeGraphsView && root.activeGraphsView.plotArea.height > 0
                                                  ? root.activeGraphsView.plotArea.y
                                                  : root.activeChartPainter ? root.activeChartPainter.y : 0
                    readonly property real plotHeight: root.useQtGraphs && root.activeGraphsView && root.activeGraphsView.plotArea.height > 0
                                                       ? root.activeGraphsView.plotArea.height
                                                       : root.activeChartPainter ? root.activeChartPainter.height : 0

                    visible: root.angleChannelOn && root.angleData.length > 0 && plotWidth > 0
                    x: plotX + plotWidth * (index / denominator)
                    y: plotY + plotHeight - 26
                    width: 2
                    height: 20
                    color: modelData.detected ? "#16C60C" : "#FF8C00"

                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.top
                        anchors.bottomMargin: 2
                        width: markerText.width + 8
                        height: 16
                        radius: 3
                        color: modelData.detected ? "#16C60C" : "#FF8C00"

                        Text {
                            id: markerText
                            anchors.centerIn: parent
                            text: modelData.angle + "°"
                            color: "white"
                            font.pixelSize: 9
                            font.bold: true
                        }
                    }
                }
            }

            Column {
                anchors.centerIn: parent
                visible: !root.running && !root.hasAnyData()
                spacing: 6

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "尚未开始测试"
                    color: root.theme.textPrimary
                    font.pixelSize: 14
                    font.bold: true
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "点击“开始测试”后显示实时波形和采样曲线"
                    color: root.theme.textMuted
                    font.pixelSize: 11
                }
            }
        }
    }

    Component {
        id: graphsComponent

        GraphsView {
            id: graphsView
            property alias graphsViewRef: graphsView
            property alias xAxisRef: xAxis
            property alias yAxisRef: yAxis
            property alias speedSeriesRef: speedSeries
            property alias torqueSeriesRef: torqueSeries
            property alias currentSeriesRef: currentSeries
            property alias angleSeriesRef: angleSeries

            anchors.fill: parent
            anchors.margins: 8
            marginTop: 8
            marginBottom: 8
            marginLeft: 8
            marginRight: 8
            clipPlotArea: true
            axisXSmoothing: 1.0
            axisYSmoothing: 1.0
            gridSmoothing: 1.0
            shadowVisible: false
            theme: GraphsTheme {
                theme: GraphsTheme.Theme.UserDefined
                backgroundVisible: false
                plotAreaBackgroundVisible: true
                plotAreaBackgroundColor: root.theme.bgSecondary
                labelsVisible: false
                gridVisible: true
                grid.mainColor: "#E8E8E8"
                grid.subColor: "#F3F3F3"
                axisX.mainColor: root.theme.dividerColor
                axisX.subColor: root.theme.dividerColor
                axisY.mainColor: root.theme.dividerColor
                axisY.subColor: root.theme.dividerColor
                seriesColors: ["#0078D4", "#E74856", "#FF8C00", "#16C60C"]
                borderColors: ["#0078D4", "#E74856", "#FF8C00", "#16C60C"]
                labelTextColor: root.theme.textMuted
            }

            axisX: ValueAxis {
                id: xAxis
                min: 0
                max: 1
                tickInterval: 1
                subTickCount: 0
                labelDecimals: 0
            }

            axisY: ValueAxis {
                id: yAxis
                min: 0
                max: 100
                tickInterval: 25
                subTickCount: 0
                labelDecimals: 0
            }

            Component.onCompleted: {
                console.log("[StartupTrace] QtGraphs GraphsView completed")
                root.refreshChart()
            }

            LineSeries {
                id: speedSeries
                axisX: xAxis
                axisY: yAxis
                color: "#0078D4"
                width: 2
            }

            LineSeries {
                id: torqueSeries
                axisX: xAxis
                axisY: yAxis
                color: "#E74856"
                width: 2
            }

            LineSeries {
                id: currentSeries
                axisX: xAxis
                axisY: yAxis
                color: "#FF8C00"
                width: 2
            }

            LineSeries {
                id: angleSeries
                axisX: xAxis
                axisY: yAxis
                color: "#16C60C"
                width: 2
            }
        }
    }

    Component {
        id: painterComponent

        ChartPainter {
            id: chartPainter
            property alias chartPainterRef: chartPainter

            anchors.fill: parent
            anchors.margins: 8
            speedData: root.speedData || []
            torqueData: root.torqueData || []
            currentData: root.currentData || []
            angleData: root.angleData || []
            speedChannelOn: root.speedChannelOn
            torqueChannelOn: root.torqueChannelOn
            currentChannelOn: root.currentChannelOn
            angleChannelOn: root.angleChannelOn
            backgroundColor: root.theme.bgSecondary
            gridColor: root.theme.dividerColor

            Component.onCompleted: console.log("[StartupTrace] ChartPainter completed")
        }
    }
}
