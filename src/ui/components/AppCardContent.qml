pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property alias content: container.data
    width: parent ? parent.width : implicitWidth
    height: implicitHeight
    implicitWidth: parent ? parent.width : container.childrenRect.width + 24
    implicitHeight: container.childrenRect.height + 24

    Item {
        id: container
        anchors.fill: parent
        anchors.margins: 12
        anchors.topMargin: 0
    }
}
