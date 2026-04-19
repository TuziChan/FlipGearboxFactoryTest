pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    required property string verdictState
    required property string verdictText
    required property string phaseTitle
    required property string totalTimeText
    required property real speedValue
    required property real torqueValue
    required property real ai1Value
    required property real ai2Value
    required property bool running
    required property var onCopyReport
    required property var onResetRequested

    objectName: "verdictPanel"

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        SectionCard {
            Layout.fillWidth: true
            theme: root.theme
            title: "状态总览"

            Rectangle {
                width: parent.width
                height: 96
                radius: root.theme.radiusLarge
                color: root.verdictState === "ok"
                       ? root.theme.okColor + "20"
                       : root.verdictState === "ng"
                         ? root.theme.ngColor + "20"
                         : root.theme.sectionColor
                border.color: root.verdictState === "ok"
                              ? root.theme.okColor
                              : root.verdictState === "ng"
                                ? root.theme.ngColor
                                : root.theme.dividerColor

                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 6

                    Text {
                        text: "整机结论"
                        color: root.theme.textSecondary
                        font.pixelSize: 11
                        font.bold: true
                    }

                    Text {
                        text: root.verdictText
                        color: root.verdictState === "ok"
                               ? root.theme.okColor
                               : root.verdictState === "ng"
                                 ? root.theme.ngColor
                                 : root.theme.textPrimary
                        font.pixelSize: 28
                        font.bold: true
                    }

                    Text {
                        text: root.running ? "测试进行中" : root.phaseTitle
                        color: root.theme.textSecondary
                        font.pixelSize: 11
                    }
                }
            }

            GridLayout {
                width: parent.width
                columns: 2
                columnSpacing: 12
                rowSpacing: 8

                Text {
                    text: root.phaseTitle
                    horizontalAlignment: Text.AlignRight
                    color: root.theme.textPrimary
                    font.pixelSize: 11
                    font.bold: true
                }

                Text {
                    text: "总耗时"
                    color: root.theme.textSecondary
                    font.pixelSize: 11
                }

                Text {
                    text: "当前阶段"
                    color: root.theme.textSecondary
                    font.pixelSize: 11
                }

                Text {
                    text: root.totalTimeText
                    horizontalAlignment: Text.AlignRight
                    color: root.theme.textPrimary
                    font.pixelSize: 13
                    font.bold: true
                    font.family: "Consolas"
                }
            }
        }

        SectionCard {
            Layout.fillWidth: true
            theme: root.theme
            title: "关键判据"

            Repeater {
                model: [
                    { label: "空载转速", value: root.speedValue.toFixed(0) + " RPM", result: root.speedValue > 0 ? (root.speedValue > 1000 ? "OK" : "监测中") : "--" },
                    { label: "锁止扭矩", value: root.torqueValue.toFixed(2) + " N·m", result: root.torqueValue >= 1.2 ? "OK" : "待判定" },
                    { label: "运行状态", value: root.running ? "RUN" : "IDLE", result: root.running ? "采样中" : "待机" },
                    { label: "磁铁感应", value: "AI1 " + (root.ai1Value > 0.5 ? "ON" : "--") + " / AI2 " + (root.ai2Value > 0.5 ? "ON" : "--"), result: root.ai1Value > 0.5 || root.ai2Value > 0.5 ? "触发" : "未触发" }
                ]

                delegate: RowLayout {
                    id: summaryRow
                    required property var modelData
                    width: parent.width
                    spacing: 8

                    Text {
                        Layout.fillWidth: true
                        text: summaryRow.modelData.label
                        color: root.theme.textSecondary
                        font.pixelSize: 11
                    }

                    Text {
                        text: summaryRow.modelData.value
                        color: root.theme.textPrimary
                        font.pixelSize: 11
                        font.bold: true
                    }

                    Text {
                        text: summaryRow.modelData.result
                        color: summaryRow.modelData.result === "OK"
                               ? root.theme.okColor
                               : summaryRow.modelData.result === "触发"
                                 ? root.theme.warnColor
                                 : root.theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                    }
                }
            }
        }

        InspectorSection {
            Layout.fillWidth: true
            theme: root.theme
            title: "报告操作"

            ColumnLayout {
                width: parent.width
                spacing: 8

                AppButton {
                    Layout.fillWidth: true
                    text: "复制报告"
                    variant: "secondary"
                    block: true
                    theme: root.theme
                    onClicked: root.onCopyReport()
                }

                AppButton {
                    Layout.fillWidth: true
                    text: "重置测试"
                    variant: "secondary"
                    block: true
                    theme: root.theme
                    onClicked: root.onResetRequested()
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
