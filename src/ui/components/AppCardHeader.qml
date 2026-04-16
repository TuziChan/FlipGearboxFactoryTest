pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property alias content: gridLayout.data

    implicitWidth: parent.width
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
