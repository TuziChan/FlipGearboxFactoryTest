pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property alias content: rowLayout.data

    implicitWidth: parent.width
    implicitHeight: rowLayout.implicitHeight + 24

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        anchors.margins: 12
        anchors.topMargin: 0
        spacing: 8
    }
}
