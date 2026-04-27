pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    required property string label
    required property string value
    required property string unit
    required property string subtext
    required property color accentColor
    property bool compact: false

    radius: root.theme.radiusLarge
    color: root.theme.cardColor
    border.color: root.theme.borderColor
    border.width: 1
    clip: true

    implicitHeight: root.compact ? 60 : 108

    ColumnLayout {
        width: parent.width - (root.compact ? 28 : 40)
        x: root.compact ? 14 : 20
        y: root.compact ? 10 : 16
        spacing: root.compact ? 2 : 4

        Text {
            text: root.label
            color: root.theme.textSecondary
            font.pixelSize: root.compact ? 11 : 12
            font.weight: Font.Medium
            Layout.fillWidth: true
            elide: Text.ElideRight
        }

        RowLayout {
            spacing: 6
            Layout.fillWidth: true

            Text {
                text: root.value
                color: root.accentColor
                font.pixelSize: root.compact ? 20 : 28
                font.weight: Font.DemiBold
                font.family: "Consolas"
                Layout.fillWidth: true
                elide: Text.ElideRight
                maximumLineCount: 1
            }

            Text {
                text: root.unit
                color: root.theme.textSecondary
                font.pixelSize: root.compact ? 11 : 13
                visible: root.unit.length > 0
                Layout.alignment: Qt.AlignVCenter
            }
        }

        Text {
            text: root.subtext
            color: root.theme.textMuted
            font.pixelSize: root.compact ? 10 : 11
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }
}
