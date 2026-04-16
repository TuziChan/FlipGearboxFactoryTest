import QtQuick

Item {
    id: root

    required property string name
    required property color color
    property int iconSize: 20

    implicitWidth: iconSize
    implicitHeight: iconSize

    NavIcon {
        anchors.centerIn: parent
        width: root.iconSize
        height: root.iconSize
        iconName: root.name
        iconColor: root.color
    }
}
