pragma ComponentBehavior: Bound

import QtQuick

// Icon component using SVG/Shape for optimal scaling and performance
Item {
    id: root

    required property string name
    required property color color
    property int iconSize: 20

    implicitWidth: iconSize
    implicitHeight: iconSize

    SvgIcon {
        anchors.centerIn: parent
        width: root.iconSize
        height: root.iconSize
        name: root.name
        color: root.color
    }
}
