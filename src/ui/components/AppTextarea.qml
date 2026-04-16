import QtQuick
import QtQuick.Controls

Column {
    id: root

    required property AppTheme theme
    property string label: ""
    property string placeholderText: ""
    property alias text: field.text
    property bool readOnly: false
    property string tone: "default"
    spacing: 6

    AppLabel {
        visible: root.label.length > 0
        theme: root.theme
        muted: true
        text: root.label
    }

    TextArea {
        id: field
        readOnly: root.readOnly
        placeholderText: root.placeholderText
        implicitWidth: 220
        implicitHeight: 88
        wrapMode: TextArea.Wrap
        color: root.theme.textPrimary
        placeholderTextColor: root.theme.textMuted
        selectedTextColor: root.theme.primaryForeground
        selectionColor: root.theme.accent
        background: Rectangle {
            radius: root.theme.radiusSmall
            color: Qt.tint(root.theme.cardColor, Qt.rgba(root.theme.stroke.r, root.theme.stroke.g, root.theme.stroke.b, 0.18))
            border.width: 1
            border.color: root.tone === "danger" ? root.theme.danger : (field.activeFocus ? root.theme.accent : root.theme.stroke)
        }
    }
}
