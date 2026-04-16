import QtQuick

Item {
    id: root

    required property string name
    required property color color
    property int iconSize: 20
    property bool useSvg: true  // Use SVG by default for better scaling

    implicitWidth: iconSize
    implicitHeight: iconSize

    // SVG-based rendering (preferred)
    Loader {
        anchors.centerIn: parent
        active: root.useSvg
        sourceComponent: Component {
            SvgIcon {
                width: root.iconSize
                height: root.iconSize
                name: root.name
                color: root.color
            }
        }
    }

    // Canvas-based rendering (fallback)
    Loader {
        anchors.centerIn: parent
        active: !root.useSvg
        sourceComponent: Component {
            NavIcon {
                width: root.iconSize
                height: root.iconSize
                iconName: root.name
                iconColor: root.color
            }
        }
    }
}
