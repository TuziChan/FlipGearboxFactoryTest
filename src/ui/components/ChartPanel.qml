pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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

    radius: root.theme.radiusLarge
    color: root.theme.cardColor
    border.color: root.theme.dividerColor
    implicitHeight: 286

    function drawSeries(ctx, values, maxValue, color) {
        if (!values || values.length < 2)
            return

        ctx.beginPath()
        for (let i = 0; i < values.length; ++i) {
            const x = i / Math.max(1, values.length - 1) * chartCanvas.width
            const y = chartCanvas.height - 20 - (values[i] / maxValue) * (chartCanvas.height - 40)
            if (i === 0)
                ctx.moveTo(x, y)
            else
                ctx.lineTo(x, y)
        }
        ctx.lineWidth = 2
        ctx.strokeStyle = color
        ctx.stroke()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

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
                    implicitHeight: 24
                    implicitWidth: 68

                    Row {
                        anchors.centerIn: parent
                        spacing: 5

                        Rectangle {
                            width: 8
                            height: 8
                            radius: 4
                            color: modelData.color
                        }

                        Text {
                            text: modelData.label
                            color: modelData.enabled ? root.theme.accent : root.theme.textSecondary
                            font.pixelSize: 10
                            font.bold: modelData.enabled
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.onToggleChannel(modelData.key)
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: root.running ? "采样中" : "待机"
                color: root.running ? root.theme.accent : root.theme.textMuted
                font.pixelSize: 10
                font.bold: root.running
            }
        }

        Canvas {
            id: chartCanvas
            Layout.fillWidth: true
            Layout.fillHeight: true

            onPaint: {
                const ctx = getContext("2d")
                ctx.reset()
                ctx.fillStyle = "#FFFFFF"
                ctx.fillRect(0, 0, width, height)

                ctx.strokeStyle = "#E8E8E8"
                ctx.lineWidth = 1
                for (let i = 0; i < 5; ++i) {
                    const y = i / 4 * (height - 20)
                    ctx.beginPath()
                    ctx.moveTo(0, y)
                    ctx.lineTo(width, y)
                    ctx.stroke()
                }

                if (root.speedChannelOn)
                    root.drawSeries(ctx, root.speedData, 1600, "#0078D4")
                if (root.torqueChannelOn)
                    root.drawSeries(ctx, root.torqueData, 2.0, "#E74856")
                if (root.currentChannelOn)
                    root.drawSeries(ctx, root.currentData, 4.0, "#FF8C00")
                if (root.angleChannelOn)
                    root.drawSeries(ctx, root.angleData, 180.0, "#16C60C")
            }
        }
    }
}
