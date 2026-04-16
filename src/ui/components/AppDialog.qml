import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property bool showCloseButton: true
    property string title: ""
    property string description: ""
    default property alias content: contentColumn.data
    signal closed

    function openDialog() {
        root.open = true
        return true
    }

    function closeDialog() {
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
            onClicked: root.closeDialog()
        }
    }

    Rectangle {
        id: content
        objectName: root.objectName.length > 0 ? root.objectName + "Content" : ""
        visible: root.open
        z: 101
        width: Math.min(parent ? parent.width - 32 : 520, 520)
        implicitHeight: dialogColumn.implicitHeight + 32
        x: parent ? (parent.width - width) / 2 : 0
        y: parent ? (parent.height - height) / 2 : 0
        radius: root.theme.radiusLarge
        color: root.theme.bgColor
        border.width: 1
        border.color: root.theme.dividerColor

        Column {
            id: dialogColumn
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
                    font.pixelSize: 18
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
            onClicked: root.closeDialog()
        }
    }
}
