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
    implicitHeight: 88

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 2

        Text {
            text: root.label
            color: root.theme.textSecondary
            font.pixelSize: 10
            font.bold: true
        }

        Row {
            spacing: 5

            Text {
                text: root.value
                color: root.accentColor
                font.pixelSize: 24
                font.weight: Font.Light
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: root.unit
                color: root.theme.textSecondary
                font.pixelSize: 12
            }
        }

        Text {
            text: root.subtext
            color: root.theme.textMuted
            font.pixelSize: 10
        }
    }
}
