pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property bool showCloseButton: true
    property string title: ""
    property string description: ""
    property int maxWidth: 448
    default property alias content: contentColumn.data
    signal closed

    function openDialog() {
        root.open = true
        overlayAnimation.start()
        contentAnimation.start()
        return true
    }

    function closeDialog() {
        closeOverlayAnimation.start()
        closeContentAnimation.start()
        return true
    }

    Rectangle {
        id: overlay
        objectName: root.objectName.length > 0 ? root.objectName + "Overlay" : ""
        anchors.fill: parent
        visible: root.open
        color: Qt.rgba(0, 0, 0, 0.15) // shadcn/ui 风格的更明显的遮罩
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
            onTapped: root.closeDialog()
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
        id: content
        objectName: root.objectName.length > 0 ? root.objectName + "Content" : ""
        visible: root.open
        z: 50
        width: Math.min(parent ? parent.width - 32 : root.maxWidth, root.maxWidth)
        implicitHeight: dialogColumn.implicitHeight + 32
        x: parent ? (parent.width - width) / 2 : 0
        y: parent ? (parent.height - height) / 2 : 0
        radius: root.theme.radiusLarge
        color: root.theme.bgPopover
        border.width: 1
        border.color: Qt.rgba(0, 0, 0, 0.05) // shadcn/ui 风格的更细微的边框
        opacity: 0
        scale: 0.95

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: Qt.rgba(0, 0, 0, 0.15) // shadcn/ui 风格的更明显的阴影
            shadowBlur: 0.6
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 8
        }

        ParallelAnimation {
            id: contentAnimation
            NumberAnimation {
                target: content
                property: "opacity"
                from: 0
                to: 1
                duration: 100
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: content
                property: "scale"
                from: 0.95
                to: 1.0
                duration: 100
                easing.type: Easing.OutCubic
            }
        }

        ParallelAnimation {
            id: closeContentAnimation
            NumberAnimation {
                target: content
                property: "opacity"
                from: 1
                to: 0
                duration: 100
                easing.type: Easing.InCubic
            }
            NumberAnimation {
                target: content
                property: "scale"
                from: 1.0
                to: 0.95
                duration: 100
                easing.type: Easing.InCubic
            }
        }

        Column {
            id: dialogColumn
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            Column {
                id: headerColumn
                width: parent.width
                spacing: 8
                visible: root.title.length > 0 || root.description.length > 0

                Text {
                    visible: root.title.length > 0
                    text: root.title
                    width: parent.width - (root.showCloseButton ? 40 : 0)
                    color: root.theme.textPrimary
                    font.pixelSize: 18
                    font.weight: Font.DemiBold
                    font.family: "Inter, system-ui, -apple-system, sans-serif"
                    wrapMode: Text.WordWrap
                }

                Text {
                    visible: root.description.length > 0
                    text: root.description
                    width: parent.width
                    color: root.theme.textSecondary
                    font.pixelSize: 14
                    font.family: "Inter, system-ui, -apple-system, sans-serif"
                    wrapMode: Text.WordWrap
                    lineHeight: 1.5
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
            anchors.topMargin: 8
            anchors.rightMargin: 8
            theme: root.theme
            iconName: "close"
            size: "icon-sm"
            variant: "ghost"
            text: ""
            onClicked: root.closeDialog()
        }
    }
}
