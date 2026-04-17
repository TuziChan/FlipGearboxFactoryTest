pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property alias content: gridLayout.data
    width: parent ? parent.width : implicitWidth
    height: implicitHeight
    implicitWidth: parent ? parent.width : gridLayout.implicitWidth + 24
    implicitHeight: gridLayout.implicitHeight + 24

    GridLayout {
        id: gridLayout
        anchors.fill: parent
        anchors.margins: 12
        columns: 2
        rowSpacing: 4
        columnSpacing: 8
    }
}
