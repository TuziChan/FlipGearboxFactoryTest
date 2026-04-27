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

    implicitHeight: root.compact ? 70 : 108

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.compact ? 12 : 20
        spacing: root.compact ? 4 : 6

        Text {
            text: root.label
            color: root.theme.textSecondary
            font.pixelSize: root.compact ? 11 : 12
            font.weight: Font.Medium
            Layout.fillWidth: true
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
        }

        RowLayout {
            spacing: 4
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft

            Text {
                text: root.value
                color: root.accentColor
                font.pixelSize: root.compact ? 18 : 28
                font.weight: Font.DemiBold
                font.family: "Consolas"
                fontSizeMode: Text.HorizontalFit
                minimumPixelSize: root.compact ? 12 : 16
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
                Layout.fillWidth: true
            }

            Text {
                text: root.unit
                color: root.theme.textSecondary
                font.pixelSize: root.compact ? 10 : 13
                visible: root.unit.length > 0
                Layout.alignment: Qt.AlignBaseline
            }
        }

        Text {
            text: root.subtext
            color: root.theme.textMuted
            font.pixelSize: root.compact ? 9 : 11
            Layout.fillWidth: true
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
            visible: root.subtext.length > 0
        }
    }
}
