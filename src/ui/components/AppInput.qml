import QtQuick
import QtQuick.Controls

Column {
    id: root

    required property AppTheme theme
    property string label: ""
    property string placeholderText: ""
    property string suffixText: ""
    property alias text: field.text
    property bool readOnly: false
    property string tone: "default"
    property string size: "default"
    spacing: 6

    Text {
        visible: root.label.length > 0
        text: root.label
        color: root.theme.textSecondary
        font.pixelSize: 12
    }

    Item {
        width: root.width > 0 ? root.width : field.implicitWidth
        height: field.implicitHeight

        TextField {
            id: field
            anchors.fill: parent
            enabled: root.enabled
            readOnly: root.readOnly
            placeholderText: root.placeholderText
            implicitWidth: 180
            implicitHeight: root.size === "sm" ? 24 : 28
            color: root.theme.textPrimary
            placeholderTextColor: root.theme.textMuted
            selectedTextColor: root.theme.primaryForeground
            selectionColor: root.theme.accent
            rightPadding: root.suffixText.length > 0 ? 24 : leftPadding
            background: Rectangle {
                radius: root.theme.radiusSmall
                color: root.enabled ? Qt.tint(root.theme.cardColor, Qt.rgba(root.theme.stroke.r, root.theme.stroke.g, root.theme.stroke.b, 0.18)) : root.theme.surface
                border.width: 1
                border.color: root.tone === "danger" ? root.theme.danger : (field.activeFocus ? root.theme.accent : root.theme.stroke)
            }
        }

        Text {
            visible: root.suffixText.length > 0
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            text: root.suffixText
            color: root.theme.textSecondary
            font.pixelSize: 12
        }
    }
}
