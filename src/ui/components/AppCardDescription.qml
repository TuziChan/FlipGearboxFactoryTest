pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Text {
    id: root

    required property AppTheme theme
    property string description: ""

    text: root.description
    color: root.theme.textMuted
    font.pixelSize: 14
    wrapMode: Text.WordWrap
    Layout.fillWidth: true
    Layout.columnSpan: parent && parent.columns > 1 ? 2 : 1
}
