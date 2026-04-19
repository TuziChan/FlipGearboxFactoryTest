pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property string text: ""
    property string side: "top"
    property int delayDuration: 0

    function showTooltip() {
        if (root.delayDuration > 0) {
            delayTimer.restart()
        } else {
            root.open = true
            showAnimation.start()
        }
        return true
    }

    function hideTooltip() {
        delayTimer.stop()
        hideAnimation.start()
        return true
    }

    Timer {
        id: delayTimer
        interval: root.delayDuration
        onTriggered: {
            root.open = true
            showAnimation.start()
        }
    }

    Rectangle {
        id: bubble
        visible: root.open
        color: root.theme.textPrimary
        radius: root.theme.radiusMedium
        implicitHeight: 30
        implicitWidth: Math.max(96, label.implicitWidth + 24)
        x: root.side === "left" ? -implicitWidth - 8 : root.side === "right" ? 8 : 0
        y: root.side === "bottom" ? 8 : root.side === "top" ? -implicitHeight - 8 : 0
        z: 50
        opacity: 0
        scale: 0.95

        ParallelAnimation {
            id: showAnimation
            NumberAnimation {
                target: bubble
                property: "opacity"
                from: 0
                to: 1
                duration: 100
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: bubble
                property: "scale"
                from: 0.95
                to: 1.0
                duration: 100
                easing.type: Easing.OutCubic
            }
        }

        ParallelAnimation {
            id: hideAnimation
            NumberAnimation {
                target: bubble
                property: "opacity"
                from: 1
                to: 0
                duration: 100
                easing.type: Easing.InCubic
            }
            NumberAnimation {
                target: bubble
                property: "scale"
                from: 1.0
                to: 0.95
                duration: 100
                easing.type: Easing.InCubic
            }
            onFinished: root.open = false
        }

        Text {
            id: label
            anchors.centerIn: parent
            text: root.text
            color: root.theme.bgColor
            font.pixelSize: 12
        }
    }

    Rectangle {
        visible: root.open
        width: 10
        height: 10
        radius: 2
        rotation: 45
        color: bubble.color
        x: root.side === "left" ? bubble.x + bubble.width - 3 : root.side === "right" ? 3 : bubble.width / 2 - 5
        y: root.side === "top" ? bubble.y + bubble.height - 3 : root.side === "bottom" ? 3 : bubble.height / 2 - 5
        z: 49
        opacity: bubble.opacity
    }
}
