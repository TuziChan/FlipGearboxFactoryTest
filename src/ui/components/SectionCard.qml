import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    property string title: ""
    property string subtitle: ""
    default property alias content: contentColumn.data

    radius: theme.radiusLarge
    color: theme.cardColor
    border.color: theme.dividerColor
    implicitHeight: sectionColumn.implicitHeight + 28

    Column {
        id: sectionColumn
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        Column {
            visible: root.title.length > 0 || root.subtitle.length > 0
            spacing: 2

            Text {
                visible: root.title.length > 0
                text: root.title
                color: root.theme.textPrimary
                font.pixelSize: 13
                font.bold: true
            }

            Text {
                visible: root.subtitle.length > 0
                text: root.subtitle
                color: root.theme.textMuted
                font.pixelSize: 11
            }
        }

        Column {
            id: contentColumn
            width: parent.width
            spacing: 8
        }
    }
}
