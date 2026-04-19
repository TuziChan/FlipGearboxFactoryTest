pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    required property var items
    property string clockText: ""

    color: "#FFFFFF"
    border.color: theme.dividerColor
    implicitHeight: 34

    RowLayout {
        width: parent.width - 36
        height: parent.height
        x: 18
        spacing: 16

        Repeater {
            model: root.items

            delegate: RowLayout {
                required property var modelData
                spacing: 6

                Rectangle {
                    Layout.preferredWidth: 10
                    Layout.preferredHeight: 10
                    Layout.alignment: Qt.AlignVCenter
                    radius: 5
                    color: {
                        if (modelData.online === true) return root.theme.ok
                        if (modelData.online === false) return root.theme.ngColor
                        if (modelData.status === "online") return root.theme.ok
                        return root.theme.ngColor
                    }
                }

                Text {
                    text: modelData.name || modelData
                    color: root.theme.textSecondary
                    font.pixelSize: 12
                }
            }
        }

        Item { Layout.fillWidth: true }

        Text {
            text: root.clockText
            color: root.theme.textMuted
            font.pixelSize: 12
        }
    }
}
