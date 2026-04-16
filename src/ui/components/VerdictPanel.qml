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
        spacing: 12

        SectionCard {
            Layout.fillWidth: true
            theme: root.theme
            title: "整机结论"

            VerdictBadge {
                theme: root.theme
                state: root.verdictState
                text: root.verdictText
            }

            RowLayout {
                width: parent.width

                Text {
                    text: root.phaseTitle
                    color: root.theme.textSecondary
                    font.pixelSize: 12
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: root.totalTimeText
                    color: root.theme.textPrimary
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        }

        InspectorSection {
            Layout.fillWidth: true
            theme: root.theme
            title: "实时判据"

            Repeater {
                model: [
                    { label: "当前阶段", value: root.phaseTitle, result: root.running ? "RUN" : root.verdictText },
                    { label: "空载转速", value: root.speedValue.toFixed(0) + " RPM", result: root.speedValue > 1000 ? "OK" : "WAIT" },
                    { label: "锁止扭矩", value: root.torqueValue.toFixed(2) + " N·m", result: root.torqueValue > 1.2 ? "OK" : "WAIT" },
                    { label: "总耗时", value: root.totalTimeText, result: root.running ? "RUN" : "DONE" }
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
                        font.pixelSize: 12
                    }

                    Text {
                        text: summaryRow.modelData.value
                        color: root.theme.textPrimary
                        font.pixelSize: 11
                        font.bold: true
                    }
                }
            }
        }

        InspectorSection {
            Layout.fillWidth: true
            theme: root.theme
            title: "磁铁感应"

            StatusList {
                width: parent.width
                theme: root.theme
                model: [
                    { label: "AI1", value: root.ai1Value > 0.5 ? "ON" : "--", online: root.ai1Value > 0.5 },
                    { label: "AI2", value: root.ai2Value > 0.5 ? "ON" : "--", online: root.ai2Value > 0.5 }
                ]
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
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
}
