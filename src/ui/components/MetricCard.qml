import QtQuick

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
    border.color: root.theme.dividerColor
    border.width: 1
    implicitHeight: 104

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 6

        Text {
            text: root.label
            color: root.theme.textSecondary
            font.pixelSize: 11
            font.bold: true
        }

        Row {
            spacing: 5

            Text {
                text: root.value
                color: root.accentColor
                font.pixelSize: 28
                font.weight: Font.DemiBold
                font.family: "Consolas"
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: root.unit
                color: root.theme.textSecondary
                font.pixelSize: 13
            }
        }

        Text {
            text: root.subtext
            color: root.theme.textMuted
            font.pixelSize: 11
        }
    }
}
