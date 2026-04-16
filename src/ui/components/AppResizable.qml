pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property string orientation: "horizontal"
    property real ratio: 50
    property real minRatio: 15
    property real maxRatio: 85
    default property alias firstContent: firstSlot.data
    property alias secondContent: secondSlot.data

    function setRatio(nextRatio) {
        root.ratio = Math.max(root.minRatio, Math.min(root.maxRatio, Number(nextRatio)))
        return true
    }

    function setRatioFromPosition(position) {
        if (root.orientation === "horizontal")
            return root.setRatio((position / Math.max(1, root.width)) * 100.0)
        return root.setRatio((position / Math.max(1, root.height)) * 100.0)
    }

    implicitWidth: 320
    implicitHeight: 180

    Rectangle {
        anchors.fill: parent
        radius: root.theme.radiusLarge
        color: root.theme.cardColor
        border.width: 1
        border.color: root.theme.dividerColor
    }

    Item {
        id: firstPanel
        x: 0
        y: 0
        width: root.orientation === "horizontal" ? Math.max(0, parent.width * (root.ratio / 100.0) - handle.width / 2) : parent.width
        height: root.orientation === "horizontal" ? parent.height : Math.max(0, parent.height * (root.ratio / 100.0) - handle.height / 2)

        Item {
            id: firstSlot
            anchors.fill: parent
            anchors.margins: 10
        }
    }

    Rectangle {
        id: handle
        x: root.orientation === "horizontal" ? parent.width * (root.ratio / 100.0) - width / 2 : 0
        y: root.orientation === "horizontal" ? 0 : parent.height * (root.ratio / 100.0) - height / 2
        width: root.orientation === "horizontal" ? 8 : parent.width
        height: root.orientation === "horizontal" ? parent.height : 8
        color: Qt.rgba(root.theme.stroke.r, root.theme.stroke.g, root.theme.stroke.b, 0.7)

        Rectangle {
            anchors.centerIn: parent
            width: root.orientation === "horizontal" ? 4 : 48
            height: root.orientation === "horizontal" ? 48 : 4
            radius: 999
            color: root.theme.textMuted
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: root.orientation === "horizontal" ? Qt.SplitHCursor : Qt.SplitVCursor
            onPressed: {
                if (root.orientation === "horizontal")
                    root.setRatioFromPosition(handle.x + mouse.x)
                else
                    root.setRatioFromPosition(handle.y + mouse.y)
            }
            onPositionChanged: {
                if (pressed) {
                    if (root.orientation === "horizontal")
                        root.setRatioFromPosition(handle.x + mouse.x)
                    else
                        root.setRatioFromPosition(handle.y + mouse.y)
                }
            }
        }
    }

    Item {
        id: secondPanel
        x: root.orientation === "horizontal" ? handle.x + handle.width : 0
        y: root.orientation === "horizontal" ? 0 : handle.y + handle.height
        width: root.orientation === "horizontal" ? parent.width - x : parent.width
        height: root.orientation === "horizontal" ? parent.height : parent.height - y

        Item {
            id: secondSlot
            anchors.fill: parent
            anchors.margins: 10
        }
    }
}
