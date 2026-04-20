pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
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

    radius: root.theme.radiusLarge
    color: root.theme.cardColor
    border.color: root.theme.dividerColor
    border.width: 1
    implicitHeight: 320

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

            Rectangle {
                visible: root.anomalyMessage !== ""
                radius: 4
                color: root.anomalyType === "timeout" ? "#FFF4E5" :
                       root.anomalyType === "disconnect" ? "#FFE5E5" : "#FFF0E5"
                border.color: root.anomalyType === "timeout" ? "#FF8C00" :
                              root.anomalyType === "disconnect" ? "#E74856" : "#FFB900"
                border.width: 1
                implicitHeight: 24
                implicitWidth: anomalyText.width + 20

                Text {
                    id: anomalyText
                    anchors.centerIn: parent
                    text: root.anomalyMessage
                    color: root.anomalyType === "timeout" ? "#FF8C00" :
                           root.anomalyType === "disconnect" ? "#E74856" : "#FFB900"
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

            ChartPainter {
                id: chartPainter
                anchors.fill: parent
                anchors.margins: 12

                speedData: root.speedData
                torqueData: root.torqueData
                currentData: root.currentData
                angleData: root.angleData
                speedChannelOn: root.speedChannelOn
                torqueChannelOn: root.torqueChannelOn
                currentChannelOn: root.currentChannelOn
                angleChannelOn: root.angleChannelOn
                backgroundColor: root.theme.bgSecondary
                gridColor: "#E8E8E8"
            }

            // Magnet detection markers overlay
            Repeater {
                model: root.magnetMarkers

                delegate: Rectangle {
                    required property var modelData
                    required property int index

                    visible: root.angleChannelOn && root.angleData.length > 0
                    x: chartPainter.x + 12 + (chartPainter.width - 24) * (index / Math.max(1, root.magnetMarkers.length - 1))
                    y: chartPainter.y + chartPainter.height - 30
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
                visible: !root.running && root.speedData.length === 0 && root.torqueData.length === 0
                         && root.currentData.length === 0 && root.angleData.length === 0
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
}
