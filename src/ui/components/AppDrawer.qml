pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property string side: "bottom"
    property string title: ""
    property string description: ""
    default property alias content: contentColumn.data
    signal closed

    function panelX() {
        if (root.side === "right")
            return root.open ? (parent ? parent.width - panel.width : 0) : (parent ? parent.width : 0)
        if (root.side === "left")
            return root.open ? 0 : -panel.width
        return 0
    }

    function panelY() {
        if (root.side === "bottom")
            return root.open ? (parent ? parent.height - panel.height : 0) : (parent ? parent.height : 0)
        if (root.side === "top")
            return root.open ? 0 : -panel.height
        return 0
    }

    function openDrawer() {
        root.open = true
        return true
    }

    function closeDrawer() {
        root.open = false
        root.closed()
        return true
    }

    Rectangle {
        id: overlay
        objectName: root.objectName.length > 0 ? root.objectName + "Overlay" : ""
        anchors.fill: parent
        visible: root.open
        color: Qt.rgba(0, 0, 0, 0.45)
        z: 100

        MouseArea {
            anchors.fill: parent
            onClicked: root.closeDrawer()
        }
    }

    Rectangle {
        id: panel
        objectName: root.objectName.length > 0 ? root.objectName + "Panel" : ""
        visible: root.open || panelXAnimation.running || panelYAnimation.running
        z: 101
        width: root.side === "left" || root.side === "right" ? Math.min(parent ? parent.width * 0.75 : 320, 360) : (parent ? parent.width : 420)
        height: root.side === "top" || root.side === "bottom" ? Math.min(parent ? parent.height * 0.55 : 280, 320) : (parent ? parent.height : 360)
        x: root.panelX()
        y: root.panelY()
        radius: root.side === "bottom" || root.side === "top" ? root.theme.radiusLarge : 0
        color: root.theme.cardColor
        border.width: 1
        border.color: root.theme.dividerColor

        Behavior on x {
            NumberAnimation { id: panelXAnimation; duration: 180; easing.type: Easing.OutCubic }
        }

        Behavior on y {
            NumberAnimation { id: panelYAnimation; duration: 180; easing.type: Easing.OutCubic }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 10

            Rectangle {
                visible: root.side === "bottom"
                anchors.horizontalCenter: parent.horizontalCenter
                width: 72
                height: 4
                radius: 999
                color: root.theme.textMuted
            }

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

            Column {
                id: contentColumn
                width: parent.width
                spacing: 10
            }
        }
    }
}
