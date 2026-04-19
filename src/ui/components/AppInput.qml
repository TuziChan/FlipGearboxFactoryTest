pragma ComponentBehavior: Bound

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
        font.weight: Font.Medium
        elide: Text.ElideRight
        width: parent.width

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
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
            font.pixelSize: root.size === "sm" ? 12 : 14

            // shadcn/ui 风格的输入体验
            selectByMouse: true
            cursorVisible: true

            // 焦点状态动画
            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
            background: Rectangle {
                id: inputBackground
                radius: root.theme.radiusSmall
                color: root.enabled ? Qt.tint(root.theme.cardColor, Qt.rgba(root.theme.stroke.r, root.theme.stroke.g, root.theme.stroke.b, 0.15)) : root.theme.surface
                border.width: field.activeFocus ? 2 : 1
                border.color: root.tone === "danger" ? root.theme.danger : (field.activeFocus ? root.theme.accent : root.theme.stroke)

                // shadcn/ui 风格的 hover 效果
                Rectangle {
                    anchors.fill: parent
                    radius: parent.radius
                    color: Qt.rgba(0, 0, 0, 0.03)
                    visible: inputHover.hovered && !field.activeFocus && root.enabled
                }

                Behavior on color {
                    ColorAnimation { duration: 150; easing.type: Easing.InOutQuad }
                }

                Behavior on border.width {
                    NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
                }

                Behavior on border.color {
                    ColorAnimation { duration: 150; easing.type: Easing.InOutQuad }
                }
            }

            HoverHandler {
                id: inputHover
                enabled: root.enabled && !root.readOnly
            }
        }

        Text {
            visible: root.suffixText.length > 0
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            text: root.suffixText
            color: root.theme.textSecondary
            font.pixelSize: root.size === "sm" ? 11 : 12
            font.family: field.font.family

            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
        }
    }
}
