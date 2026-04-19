pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    property real ratio: 16 / 9
    default property alias content: contentFrame.data
    property bool clipContent: true

    implicitWidth: 240
    implicitHeight: implicitWidth / Math.max(0.01, root.ratio)
    height: width / Math.max(0.01, root.ratio)
    clip: root.clipContent

    Item {
        id: contentFrame
        anchors.fill: parent
    }
}
