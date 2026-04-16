pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    required property real hostWidth
    required property int currentPhaseIndex
    required property string phaseTitle
    required property string totalTimeText
    required property var stepModel
    required property var angleModel
    required property var loadModel
    required property var chartSpeed
    required property var chartTorque
    required property var chartCurrent
    required property var chartAngle
    required property bool running
    required property bool speedChannelOn
    required property bool torqueChannelOn
    required property bool currentChannelOn
    required property bool angleChannelOn
    required property real speedValue
    required property real torqueValue
    required property real ai1Value
    required property real ai2Value
    required property string verdictState
    required property string verdictText
    required property var metricColorProvider
    required property var metricValueProvider
    required property var metricUnitProvider
    required property var onToggleChannel
    required property var onCopyReport
    required property var onResetRequested

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SectionCard {
            objectName: "phasePanel"
            Layout.preferredWidth: root.hostWidth < 1500 ? 214 : 236
            Layout.fillHeight: true
            theme: root.theme
            title: "测试流程"
            subtitle: root.running ? root.phaseTitle : "待机"

            FlowList {
                width: parent.width
                theme: root.theme
                model: root.stepModel
            }

            AppSeparator {
                width: parent.width
                theme: root.theme
            }

            RowLayout {
                width: parent.width

                Text {
                    text: "总耗时"
                    color: root.theme.textSecondary
                    font.pixelSize: 11
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: root.totalTimeText
                    color: root.theme.textPrimary
                    font.pixelSize: 14
                    font.bold: true
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: root.theme.bgColor

            Flickable {
                anchors.fill: parent
                contentWidth: width
                contentHeight: centerColumn.height + 20
                clip: true

                ColumnLayout {
                    id: centerColumn
                    width: parent.width - 24
                    x: 12
                    y: 12
                    spacing: 12

                    GridLayout {
                        id: metricsGrid
                        objectName: "metricsGrid"
                        Layout.fillWidth: true
                        columns: root.hostWidth < 1550 ? 2 : 3
                        columnSpacing: 10
                        rowSpacing: 10

                        Repeater {
                            model: ["转速", "扭矩", "功率", "电机电流", "制动电流", "角度"]

                            delegate: MetricCard {
                                required property string modelData

                                Layout.fillWidth: true
                                theme: root.theme
                                label: modelData
                                value: root.metricValueProvider(modelData)
                                unit: root.metricUnitProvider(modelData)
                                subtext: modelData === "角度" ? root.phaseTitle : "采样周期 100 ms"
                                accentColor: root.metricColorProvider(modelData)
                            }
                        }
                    }

                    DataTableCard {
                        visible: root.currentPhaseIndex === 2
                                 || (root.angleModel.count > 0 && root.angleModel.get(0).result !== "待测")
                        Layout.fillWidth: true
                        theme: root.theme
                        headers: ["目标位", "目标角度", "当前角度", "偏差", "公差", "判定"]
                        rowModel: root.angleModel
                        columnKeys: ["target", "targetAngle", "currentAngle", "deviation", "tolerance", "result"]
                        resultKey: "result"
                        pendingText: "待测"
                    }

                    DataTableCard {
                        visible: root.currentPhaseIndex === 3
                                 || (root.loadModel.count > 0 && root.loadModel.get(0).result !== "待测")
                        Layout.fillWidth: true
                        theme: root.theme
                        headers: ["方向", "制动电流", "锁止扭矩", "下限", "判定"]
                        rowModel: root.loadModel
                        columnKeys: ["direction", "brakeCurrent", "torque", "limit", "result"]
                        resultKey: "result"
                        pendingText: "待测"
                    }

                    ChartPanel {
                        Layout.fillWidth: true
                        theme: root.theme
                        running: root.running
                        speedData: root.chartSpeed
                        torqueData: root.chartTorque
                        currentData: root.chartCurrent
                        angleData: root.chartAngle
                        speedChannelOn: root.speedChannelOn
                        torqueChannelOn: root.torqueChannelOn
                        currentChannelOn: root.currentChannelOn
                        angleChannelOn: root.angleChannelOn
                        onToggleChannel: root.onToggleChannel
                    }
                }
            }
        }

        VerdictPanel {
            Layout.preferredWidth: root.hostWidth < 1500 ? 248 : 272
            Layout.fillHeight: true
            theme: root.theme
            verdictState: root.verdictState
            verdictText: root.verdictText
            phaseTitle: root.phaseTitle
            totalTimeText: root.totalTimeText
            speedValue: root.speedValue
            torqueValue: root.torqueValue
            ai1Value: root.ai1Value
            ai2Value: root.ai2Value
            running: root.running
            onCopyReport: root.onCopyReport
            onResetRequested: root.onResetRequested
        }
    }
}
