pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property string orientation: "horizontal"

    color: theme.dividerColor
    implicitWidth: orientation === "vertical" ? 1 : 120
    implicitHeight: orientation === "vertical" ? 24 : 1
}
