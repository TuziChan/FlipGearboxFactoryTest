pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    property string title: ""
    property string subtitle: ""
    default property alias content: contentColumn.data

    radius: theme.radiusLarge
    color: theme.cardColor
    border.color: theme.dividerColor
    implicitHeight: sectionColumn.implicitHeight + 28

    ColumnLayout {
        id: sectionColumn
        width: parent.width - 28
        x: 14
        y: 14
        spacing: 12

        ColumnLayout {
            visible: root.title.length > 0 || root.subtitle.length > 0
            Layout.fillWidth: true
            spacing: 2

            Text {
                visible: root.title.length > 0
                text: root.title
                color: root.theme.textPrimary
                font.pixelSize: 13
                font.bold: true
            }

            Text {
                visible: root.subtitle.length > 0
                text: root.subtitle
                color: root.theme.textMuted
                font.pixelSize: 11
            }
        }

        ColumnLayout {
            id: contentColumn
            Layout.fillWidth: true
            spacing: 8
        }
    }
}
