import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property string side: "right"
    property bool showCloseButton: true
    property string title: ""
    property string description: ""
    default property alias content: contentColumn.data
    signal closed

    function openSheet() {
        root.open = true
        return true
    }

    function closeSheet() {
        root.open = false
        root.closed()
        return true
    }

    Rectangle {
        id: overlay
        objectName: root.objectName.length > 0 ? root.objectName + "Overlay" : ""
        anchors.fill: parent
        visible: root.open
        color: Qt.rgba(0, 0, 0, 0.5)
        z: 100

        MouseArea {
            anchors.fill: parent
            onClicked: root.closeSheet()
        }
    }

    Rectangle {
        id: panel
        objectName: root.objectName.length > 0 ? root.objectName + "Panel" : ""
        visible: root.open
        z: 101
        width: root.side === "left" || root.side === "right" ? Math.min(parent ? parent.width * 0.75 : 320, 360) : (parent ? parent.width : 420)
        height: root.side === "top" || root.side === "bottom" ? Math.min(parent ? parent.height * 0.45 : 240, 280) : (parent ? parent.height : 360)
        x: root.side === "right" ? (parent ? parent.width - width : 0) : 0
        y: root.side === "bottom" ? (parent ? parent.height - height : 0) : 0
        color: root.theme.bgColor
        border.width: 1
        border.color: root.theme.dividerColor

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Column {
                spacing: 4
                visible: root.title.length > 0 || root.description.length > 0

                Text {
                    visible: root.title.length > 0
                    text: root.title
                    color: root.theme.textPrimary
                    font.pixelSize: 16
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
                spacing: 10
            }
        }

        AppButton {
            visible: root.showCloseButton
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 12
            anchors.rightMargin: 12
            theme: root.theme
            iconName: "close"
            size: "icon"
            variant: "ghost"
            text: ""
            onClicked: root.closeSheet()
        }
    }
}
