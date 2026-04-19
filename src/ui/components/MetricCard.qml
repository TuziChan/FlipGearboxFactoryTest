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

    radius: root.theme.radiusLarge
    color: root.theme.cardColor
    border.color: root.theme.borderColor
    border.width: 1
    clip: true

    implicitHeight: 108

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 16
        anchors.bottomMargin: 16
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 4

        Text {
            text: root.label
            color: root.theme.textSecondary
            font.pixelSize: 12
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
                font.pixelSize: 28
                font.weight: Font.DemiBold
                font.family: "Consolas"
                Layout.fillWidth: true
                elide: Text.ElideRight
                maximumLineCount: 1
            }

            Text {
                text: root.unit
                color: root.theme.textSecondary
                font.pixelSize: 13
                visible: root.unit.length > 0
                Layout.alignment: Qt.AlignVCenter
            }
        }

        Text {
            text: root.subtext
            color: root.theme.textMuted
            font.pixelSize: 11
            Layout.fillWidth: true
            elide: Text.ElideRight
        }
    }
}
