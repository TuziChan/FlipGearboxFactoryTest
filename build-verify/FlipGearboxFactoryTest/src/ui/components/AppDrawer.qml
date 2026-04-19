pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects

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
        overlayAnimation.start()
        return true
    }

    function closeDrawer() {
        closeOverlayAnimation.start()
        return true
    }

    Rectangle {
        id: overlay
        objectName: root.objectName.length > 0 ? root.objectName + "Overlay" : ""
        anchors.fill: parent
        visible: root.open
        color: Qt.rgba(0, 0, 0, 0.1)
        z: 50
        opacity: 0

        layer.enabled: true
        layer.effect: MultiEffect {
            blurEnabled: true
            blur: 0.4
            blurMax: 32
            blurMultiplier: 1.0
        }

        TapHandler {
            onTapped: root.closeDrawer()
        }

        NumberAnimation {
            id: overlayAnimation
            target: overlay
            property: "opacity"
            from: 0
            to: 1
            duration: 100
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            id: closeOverlayAnimation
            target: overlay
            property: "opacity"
            from: 1
            to: 0
            duration: 100
            easing.type: Easing.InCubic
            onFinished: {
                root.open = false
                root.closed()
            }
        }
    }

    Rectangle {
        id: panel
        objectName: root.objectName.length > 0 ? root.objectName + "Panel" : ""
        visible: root.open || panelXAnimation.running || panelYAnimation.running
        z: 50
        width: {
            if (root.side === "left" || root.side === "right") {
                return Math.min(parent ? parent.width * 0.75 : 320, 448)
            }
            return parent ? parent.width : 420
        }
        height: {
            if (root.side === "top" || root.side === "bottom") {
                return Math.min(parent ? parent.height * 0.8 : 280, parent ? parent.height * 0.8 : 600)
            }
            return parent ? parent.height : 360
        }
        x: root.panelX()
        y: root.panelY()
        radius: {
            if (root.side === "bottom") return root.theme.radiusLarge
            if (root.side === "top") return root.theme.radiusLarge
            if (root.side === "left") return root.theme.radiusLarge
            if (root.side === "right") return root.theme.radiusLarge
            return 0
        }
        color: root.theme.bgPopover
        border.width: {
            if (root.side === "bottom") return 1
            if (root.side === "top") return 1
            if (root.side === "left") return 1
            if (root.side === "right") return 1
            return 0
        }
        border.color: Qt.rgba(0, 0, 0, 0.1)

        Behavior on x {
            NumberAnimation { id: panelXAnimation; duration: 200; easing.type: Easing.OutCubic }
        }

        Behavior on y {
            NumberAnimation { id: panelYAnimation; duration: 200; easing.type: Easing.OutCubic }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 0
            spacing: 0

            Rectangle {
                visible: root.side === "bottom"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 16
                width: 100
                height: 4
                radius: 999
                color: root.theme.textMuted
            }

            Column {
                id: headerColumn
                width: parent.width
                padding: 16
                spacing: 2
                visible: root.title.length > 0 || root.description.length > 0

                Text {
                    visible: root.title.length > 0
                    text: root.title
                    width: parent.width - 32
                    color: root.theme.textPrimary
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    horizontalAlignment: root.side === "bottom" || root.side === "top" ? Text.AlignHCenter : Text.AlignLeft
                }

                Text {
                    visible: root.description.length > 0
                    text: root.description
                    width: parent.width - 32
                    color: root.theme.textSecondary
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    horizontalAlignment: root.side === "bottom" || root.side === "top" ? Text.AlignHCenter : Text.AlignLeft
                }
            }

            Column {
                id: contentColumn
                width: parent.width
                padding: 16
                spacing: 16
            }
        }
    }
}
