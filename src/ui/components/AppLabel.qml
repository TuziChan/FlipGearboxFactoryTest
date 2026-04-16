import QtQuick

Text {
    id: root

    required property AppTheme theme
    property bool muted: false

    color: root.muted ? root.theme.textSecondary : root.theme.textPrimary
    font.pixelSize: 12
    font.bold: true
}
