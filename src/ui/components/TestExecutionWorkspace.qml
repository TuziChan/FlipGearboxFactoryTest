pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

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
    required property var idleModel
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
    required property bool motorOnline
    required property bool torqueOnline
    required property bool encoderOnline
    required property bool brakeOnline
    required property string verdictState
    required property string verdictText
    required property var metricColorProvider
    required property var metricValueProvider
    required property var metricUnitProvider
    required property var onToggleChannel
    required property var onCopyReport
    required property var onResetRequested
    property var magnetMarkers: []
    property string anomalyMessage: ""
    property string anomalyType: ""
    property var physicsViolations: []
    property var physicsViolationStats: ({})

    function statusColor(active) {
        return active ? root.theme.okColor : root.theme.textMuted
    }

    Component.onCompleted: console.log("[StartupTrace] TestExecutionWorkspace completed")

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 16

        SectionCard {
            objectName: "phasePanel"
            Layout.preferredWidth: root.hostWidth < 1500 ? 244 : 268
            Layout.fillHeight: true
            theme: root.theme
            title: "测试流程"
            subtitle: root.running ? root.phaseTitle : "待机中"

            FlowList {
                Layout.fillWidth: true
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
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                ColumnLayout {
                    id: centerColumn
                    width: parent.width - 24
                    x: 12
                    y: 12
                    spacing: 16

                    SectionCard {
                        Layout.fillWidth: true
                        theme: root.theme
                        title: "测试概览"
                        subtitle: "核心实时指标与当前运行状态"

                        ColumnLayout {
                            width: parent.width
                            spacing: 16

                            GridLayout {
                                width: parent.width
                                columns: 4
                                columnSpacing: 12
                                rowSpacing: 12

                                Repeater {
                                    model: ["转速", "扭矩", "电机电流", "角度"]

                                    delegate: MetricCard {
                                        required property string modelData
                                        Layout.fillWidth: true
                                        compact: false
                                        theme: root.theme
                                        label: modelData
                                        value: root.metricValueProvider(modelData)
                                        unit: root.metricUnitProvider(modelData)
                                        subtext: modelData === "角度" ? root.phaseTitle : "核心指标"
                                        accentColor: root.metricColorProvider(modelData)
                                    }
                                }
                            }

                            GridLayout {
                                width: parent.width
                                columns: 5
                                columnSpacing: 10
                                rowSpacing: 10

                                Repeater {
                                    model: ["功率", "制动电流", "制动电压", "制动功率", "累计角度"]

                                    delegate: MetricCard {
                                        required property string modelData
                                        Layout.fillWidth: true
                                        compact: true
                                        theme: root.theme
                                        label: modelData
                                        value: root.metricValueProvider(modelData)
                                        unit: root.metricUnitProvider(modelData)
                                        subtext: modelData === "累计角度" ? "虚拟多圈" : "辅助参数"
                                        accentColor: root.metricColorProvider(modelData)
                                    }
                                }
                            }
                        }
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
                        magnetMarkers: root.magnetMarkers
                        anomalyMessage: root.anomalyMessage
                        anomalyType: root.anomalyType
                        onToggleChannel: root.onToggleChannel
                    }

                    SectionCard {
                        visible: root.currentPhaseIndex === 1
                                 || root.currentPhaseIndex === 2
                                 || root.currentPhaseIndex === 3
                                 || (root.idleModel.count > 0 && root.idleModel.get(0).result !== "待测")
                                 || (root.angleModel.count > 0 && root.angleModel.get(0).result !== "待测")
                                 || (root.loadModel.count > 0 && root.loadModel.get(0).result !== "待测")
                        Layout.fillWidth: true
                        theme: root.theme
                        title: "阶段明细"
                        subtitle: root.currentPhaseIndex === 1 ? "空载测试结果"
                                   : root.currentPhaseIndex === 2 ? "角度定位结果"
                                   : root.currentPhaseIndex === 3 ? "负载测试结果"
                                   : "测试结果汇总"

                        ColumnLayout {
                            width: parent.width
                            spacing: 12

                            DataTableCard {
                                visible: root.currentPhaseIndex === 1
                                         || (root.idleModel.count > 0 && root.idleModel.get(0).result !== "待测")
                                width: parent.width
                                theme: root.theme
                                headers: ["方向", "电流平均", "电流峰值", "转速平均", "转速峰值", "判定"]
                                rowModel: root.idleModel
                                columnKeys: ["direction", "currentAvg", "currentMax", "speedAvg", "speedMax", "result"]
                                resultKey: "result"
                                pendingText: "待测"
                            }

                            DataTableCard {
                                visible: root.currentPhaseIndex === 2
                                         || (root.angleModel.count > 0 && root.angleModel.get(0).result !== "待测")
                                width: parent.width
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
                                width: parent.width
                                theme: root.theme
                                headers: ["方向", "制动电流", "锁止扭矩", "下限", "判定"]
                                rowModel: root.loadModel
                                columnKeys: ["direction", "brakeCurrent", "torque", "limit", "result"]
                                resultKey: "result"
                                pendingText: "待测"
                            }
                        }
                    }
                }
            }

            PhysicsValidationPanel {
                Layout.fillWidth: true
                theme: root.theme
                running: root.running
                violations: root.physicsViolations
                violationStats: root.physicsViolationStats
                panelVisible: true
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
