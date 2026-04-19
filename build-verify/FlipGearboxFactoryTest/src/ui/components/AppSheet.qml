pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

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
        overlayAnimation.start()
        panelAnimation.start()
        return true
    }

    function closeSheet() {
        closeOverlayAnimation.start()
        closePanelAnimation.start()
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
            onTapped: root.closeSheet()
        }

        NumberAnimation {
            id: overlayAnimation
            target: overlay
            property: "opacity"
            from: 0
            to: 1
            duration: 150
            easing.type: Easing.OutCubic
        }

        NumberAnimation {
            id: closeOverlayAnimation
            target: overlay
            property: "opacity"
            from: 1
            to: 0
            duration: 150
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
        visible: root.open || panelAnimation.running || closePanelAnimation.running
        z: 50
        width: root.side === "left" || root.side === "right" ? Math.min(parent ? parent.width * 0.75 : 320, 448) : (parent ? parent.width : 420)
        height: root.side === "top" || root.side === "bottom" ? Math.min(parent ? parent.height * 0.45 : 240, 280) : (parent ? parent.height : 360)
        x: {
            if (root.side === "right") return parent ? parent.width - width : 0
            if (root.side === "left") return 0
            return 0
        }
        y: {
            if (root.side === "bottom") return parent ? parent.height - height : 0
            if (root.side === "top") return 0
            return 0
        }
        color: root.theme.bgPopover
        border.width: {
            if (root.side === "right") return 1
            if (root.side === "left") return 1
            if (root.side === "top") return 1
            if (root.side === "bottom") return 1
            return 0
        }
        border.color: Qt.rgba(0, 0, 0, 0.1)
        opacity: 0

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: Qt.rgba(0, 0, 0, 0.15)
            shadowBlur: 0.8
            shadowHorizontalOffset: root.side === "right" ? -4 : root.side === "left" ? 4 : 0
            shadowVerticalOffset: root.side === "bottom" ? -4 : root.side === "top" ? 4 : 0
        }

        ParallelAnimation {
            id: panelAnimation
            NumberAnimation {
                target: panel
                property: "opacity"
                from: 0
                to: 1
                duration: 200
                easing.type: Easing.OutCubic
            }
        }

        ParallelAnimation {
            id: closePanelAnimation
            NumberAnimation {
                target: panel
                property: "opacity"
                from: 1
                to: 0
                duration: 200
                easing.type: Easing.InCubic
            }
        }

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            Column {
                width: parent.width
                spacing: 2
                visible: root.title.length > 0 || root.description.length > 0

                Text {
                    visible: root.title.length > 0
                    text: root.title
                    width: parent.width - (root.showCloseButton ? 40 : 0)
                    color: root.theme.textPrimary
                    font.pixelSize: 16
                    font.weight: Font.Medium
                    wrapMode: Text.WordWrap
                }

                Text {
                    visible: root.description.length > 0
                    text: root.description
                    width: parent.width
                    color: root.theme.textSecondary
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    lineHeight: 1.4
                }
            }

            Column {
                id: contentColumn
                width: parent.width
                spacing: 16
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
            size: "icon-sm"
            variant: "ghost"
            text: ""
            onClicked: root.closeSheet()
        }
    }
}
