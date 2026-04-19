pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root

    required property AppTheme theme

    radius: root.theme.radiusSmall
    color: Qt.tint(root.theme.muted, Qt.rgba(1, 1, 1, root.theme.darkMode ? 0.06 : 0.4))

    SequentialAnimation on opacity {
        loops: Animation.Infinite
        running: true
        NumberAnimation { to: 0.55; duration: 700 }
        NumberAnimation { to: 1.0; duration: 700 }
    }
}
