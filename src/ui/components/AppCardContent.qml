pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property alias content: container.data

    implicitWidth: parent.width
    implicitHeight: container.childrenRect.height + 24

    Item {
        id: container
        anchors.fill: parent
        anchors.margins: 12
        anchors.topMargin: 0
    }
}
