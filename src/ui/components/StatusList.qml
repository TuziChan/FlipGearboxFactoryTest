import QtQuick
import QtQuick.Layouts

Column {
    id: root

    required property AppTheme theme
    required property var model
    spacing: 8

    Repeater {
        model: root.model

        delegate: RowLayout {
            required property var modelData
            width: root.width
            spacing: 8

            Rectangle {
                Layout.preferredWidth: 8
                Layout.preferredHeight: 8
                radius: 4
                color: modelData.online === false ? "#D9D9D9" : root.theme.ok
            }

            Text {
                Layout.fillWidth: true
                text: modelData.label || modelData.name || ""
                color: root.theme.textPrimary
                font.pixelSize: 11
            }

            Text {
                text: modelData.value || modelData.state || ""
                color: root.theme.textSecondary
                font.pixelSize: 11
            }
        }
    }
}
