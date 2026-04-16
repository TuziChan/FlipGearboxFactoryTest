import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property string title: ""
    property string description: ""
    property real panelWidth: 288
    default property alias content: contentColumn.data

    function openPopover() {
        root.open = true
        return true
    }

    function closePopover() {
        root.open = false
        return true
    }

    Rectangle {
        id: panel
        objectName: root.objectName.length > 0 ? root.objectName + "Panel" : ""
        visible: root.open
        z: 101
        width: root.panelWidth
        implicitHeight: column.implicitHeight + 24
        color: root.theme.cardColor
        border.width: 1
        border.color: root.theme.dividerColor
        radius: root.theme.radiusMedium

        Column {
            id: column
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            Column {
                spacing: 4
                visible: root.title.length > 0 || root.description.length > 0

                Text {
                    visible: root.title.length > 0
                    text: root.title
                    color: root.theme.textPrimary
                    font.pixelSize: 13
                    font.bold: true
                }

                Text {
                    visible: root.description.length > 0
                    text: root.description
                    color: root.theme.textSecondary
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }

            Column {
                id: contentColumn
                width: parent.width
                spacing: 8
            }
        }
    }
}
