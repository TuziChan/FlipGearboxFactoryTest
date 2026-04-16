pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property string label: ""
    property string description: ""
    property string errorText: ""
    property string orientation: "vertical"
    property Item control: null
    property bool invalid: root.errorText.length > 0
    property bool dense: false
    property real labelWidth: 140
    default property alias content: fieldSlot.data
    property alias labelContent: labelSlot.data
    property alias descriptionContent: descriptionSlot.data
    property alias errorContent: errorSlot.data

    readonly property bool horizontal: root.orientation === "horizontal"
    readonly property bool disabledState: !root.enabled || root._readBoolean(root.control, "disabled", false) || !root._readBoolean(root.control, "enabled", true)
    readonly property bool hasDescription: root.description.length > 0 || descriptionSlot.children.length > 0
    readonly property bool hasError: root.invalid || errorSlot.children.length > 0
    readonly property color labelColor: root.hasError ? root.theme.danger : root.theme.textPrimary
    readonly property color descriptionColor: root.theme.textSecondary

    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    function _hasCallable(item, name) {
        return !!item && typeof item[name] === "function"
    }

    function _hasProperty(item, name) {
        return !!item && item[name] !== undefined
    }

    function _readBoolean(item, name, fallbackValue) {
        if (!_hasProperty(item, name))
            return fallbackValue
        return !!item[name]
    }

    function focusControl() {
        if (!root.control || root.disabledState)
            return

        if (root._hasCallable(root.control, "forceActiveFocus")) {
            root.control.forceActiveFocus()
            return
        }

        if (root._hasProperty(root.control, "focus"))
            root.control.focus = true
    }

    GridLayout {
        id: layout
        anchors.fill: parent
        columns: root.horizontal ? 2 : 1
        columnSpacing: root.theme.spacingLarge
        rowSpacing: root.dense ? root.theme.spacingTiny : root.theme.spacingSmall

        ColumnLayout {
            id: labelColumn
            Layout.fillWidth: !root.horizontal
            Layout.preferredWidth: root.horizontal ? root.labelWidth : -1
            Layout.alignment: Qt.AlignTop
            spacing: root.theme.spacingTiny
            visible: labelRow.visible || (root.horizontal && descriptionText.visible)

            RowLayout {
                id: labelRow
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall
                visible: root.label.length > 0 || labelSlot.children.length > 0

                AppLabel {
                    id: labelText
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillWidth: true
                    theme: root.theme
                    text: root.label
                    color: root.labelColor
                    muted: false
                    visible: root.label.length > 0
                    opacity: root.disabledState ? 0.6 : 1
                }

                Item {
                    id: labelSlot
                    Layout.alignment: Qt.AlignVCenter
                    Layout.fillWidth: !labelText.visible
                    implicitWidth: childrenRect.width
                    implicitHeight: childrenRect.height
                    visible: children.length > 0
                }

                TapHandler {
                    enabled: root.control !== null
                    onTapped: root.focusControl()
                }
            }

            Text {
                id: descriptionText
                Layout.fillWidth: true
                visible: root.horizontal && root.description.length > 0
                text: root.description
                color: root.descriptionColor
                opacity: root.disabledState ? 0.8 : 1
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
        }

        ColumnLayout {
            id: bodyColumn
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: root.theme.spacingTiny

            Item {
                id: fieldSlot
                Layout.fillWidth: true
                implicitWidth: childrenRect.width
                implicitHeight: childrenRect.height
            }

            Text {
                visible: !root.horizontal && root.description.length > 0
                text: root.description
                color: root.descriptionColor
                opacity: root.disabledState ? 0.8 : 1
                font.pixelSize: 11
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Item {
                id: descriptionSlot
                Layout.fillWidth: true
                implicitWidth: childrenRect.width
                implicitHeight: childrenRect.height
                visible: children.length > 0
            }

            Text {
                visible: root.invalid && root.errorText.length > 0
                text: root.errorText
                color: root.theme.danger
                font.pixelSize: 11
                font.bold: true
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Item {
                id: errorSlot
                Layout.fillWidth: true
                implicitWidth: childrenRect.width
                implicitHeight: childrenRect.height
                visible: children.length > 0
            }
        }
    }
}
