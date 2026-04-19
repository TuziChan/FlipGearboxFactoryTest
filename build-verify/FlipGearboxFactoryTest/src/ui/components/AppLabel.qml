pragma ComponentBehavior: Bound

import QtQuick

Text {
    id: root

    required property AppTheme theme
    property bool muted: false
    property int fontSize: 12
    property int fontWeight: 400  // 400=normal, 500=medium, 600=semibold, 700=bold

    color: root.muted ? root.theme.textSecondary : root.theme.textPrimary
    font.pixelSize: root.fontSize
    font.weight: root.fontWeight
}
