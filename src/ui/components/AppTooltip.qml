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
        }
        return true
    }

    function hideTooltip() {
        delayTimer.stop()
        root.open = false
        return true
    }

    Timer {
        id: delayTimer
        interval: root.delayDuration
        onTriggered: root.open = true
    }

    Rectangle {
        id: bubble
        visible: root.open
        color: root.theme.textPrimary
        radius: root.theme.radiusSmall
        implicitHeight: 30
        implicitWidth: Math.max(96, label.implicitWidth + 18)
        x: root.side === "left" ? -implicitWidth - 8 : root.side === "right" ? 8 : 0
        y: root.side === "bottom" ? 8 : root.side === "top" ? -implicitHeight - 8 : 0

        Text {
            id: label
            anchors.centerIn: parent
            text: root.text
            color: root.theme.bgColor
            font.pixelSize: 11
        }
    }

    Rectangle {
        visible: root.open
        width: 8
        height: 8
        radius: 2
        rotation: 45
        color: bubble.color
        x: root.side === "left" ? bubble.x + bubble.width - 2 : root.side === "right" ? 2 : bubble.width / 2 - 4
        y: root.side === "top" ? bubble.y + bubble.height - 2 : root.side === "bottom" ? 2 : bubble.height / 2 - 4
    }
}
