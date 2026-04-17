import QtQuick
import QtQuick.Layouts

FocusScope {
    id: root

    required property AppTheme theme
    property bool open: false
    property bool closeOnEscape: true
    property bool closeOnOverlayPress: false
    property string variant: "default"
    property string size: "default"
    property string title: ""
    property string description: ""
    property string confirmText: "Delete"
    property string cancelText: "Cancel"
    property bool confirmEnabled: true
    property bool cancelEnabled: true
    property string confirmButtonVariant: root.variant === "destructive" || root.variant === "danger" ? "destructive" : "default"
    property string cancelButtonVariant: "outline"
    property real maximumWidth: root.size === "sm" ? 320 : 512
    default property alias content: bodyColumn.data
    property alias media: mediaSlot.data
    signal confirmed
    signal cancelled
    signal closed

    function openDialog() {
        root.open = true
        dialogPanel.forceActiveFocus()
        return true
    }

    function closeDialog() {
        if (!root.open) {
            return false
        }

        root.open = false
        root.closed()
        return true
    }

    function acceptDialog() {
        if (!root.confirmEnabled) {
            return false
        }

        root.confirmed()
        return root.closeDialog()
    }

    function rejectDialog() {
        if (!root.cancelEnabled) {
            return false
        }

        root.cancelled()
        return root.closeDialog()
    }

    Keys.onEscapePressed: function(event) {
        if (root.open && root.closeOnEscape) {
            root.rejectDialog()
            event.accepted = true
        }
    }

    Rectangle {
        id: overlay
        objectName: root.objectName.length > 0 ? root.objectName + "Overlay" : ""
        anchors.fill: parent
        visible: root.open
        color: Qt.rgba(0, 0, 0, root.theme.darkMode ? 0.72 : 0.55)
        opacity: root.open ? 1 : 0
        z: 200

        Behavior on opacity {
            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
        }

        MouseArea {
            anchors.fill: parent
            enabled: root.closeOnOverlayPress
            onClicked: root.rejectDialog()
        }
    }

    Rectangle {
        id: dialogPanel
        objectName: root.objectName.length > 0 ? root.objectName + "Content" : ""
        visible: root.open
        z: 201
        width: Math.min(root.maximumWidth, parent ? parent.width - root.theme.spacingXLarge * 2 : root.maximumWidth)
        implicitHeight: panelLayout.implicitHeight + root.theme.spacingXLarge * 2
        x: parent ? (parent.width - width) / 2 : 0
        y: parent ? (parent.height - height) / 2 : 0
        radius: root.theme.radiusLarge
        color: root.theme.cardColor
        border.width: 1
        border.color: root.variant === "destructive" || root.variant === "danger"
                      ? Qt.rgba(root.theme.danger.r, root.theme.danger.g, root.theme.danger.b, 0.28)
                      : root.theme.dividerColor
        focus: visible
        opacity: root.open ? 1 : 0
        scale: root.open ? 1 : 0.95

        Behavior on opacity {
            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
        }

        Behavior on scale {
            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
        }

        Keys.onReturnPressed: function(event) {
            if (root.open && root.confirmEnabled) {
                root.acceptDialog()
                event.accepted = true
            }
        }

        Keys.onEnterPressed: function(event) {
            if (root.open && root.confirmEnabled) {
                root.acceptDialog()
                event.accepted = true
            }
        }

        ColumnLayout {
            id: panelLayout
            anchors.fill: parent
            anchors.margins: root.theme.spacingXLarge
            spacing: root.theme.spacingLarge

            RowLayout {
                id: headerLayout
                Layout.fillWidth: true
                spacing: root.theme.spacingMedium
                visible: mediaSlot.children.length > 0 || root.title.length > 0 || root.description.length > 0

                Item {
                    id: mediaSlot
                    objectName: root.objectName.length > 0 ? root.objectName + "Media" : ""
                    Layout.alignment: Qt.AlignTop
                    implicitWidth: children.length > 0 ? 64 : 0
                    implicitHeight: children.length > 0 ? 64 : 0
                    visible: children.length > 0

                    Rectangle {
                        anchors.fill: parent
                        radius: root.theme.radiusMedium
                        color: root.theme.muted
                        visible: parent.children.length > 0
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: root.theme.spacingTiny

                    Text {
                        objectName: root.objectName.length > 0 ? root.objectName + "Title" : ""
                        visible: root.title.length > 0
                        text: root.title
                        color: root.theme.textPrimary
                        font.pixelSize: 18
                        font.weight: Font.DemiBold
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Text {
                        objectName: root.objectName.length > 0 ? root.objectName + "Description" : ""
                        visible: root.description.length > 0
                        text: root.description
                        color: root.theme.textMuted
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            ColumnLayout {
                id: bodyColumn
                Layout.fillWidth: true
                spacing: root.theme.spacingMedium
            }

            RowLayout {
                objectName: root.objectName.length > 0 ? root.objectName + "Footer" : ""
                Layout.fillWidth: true
                Layout.topMargin: root.theme.spacingTiny
                spacing: root.theme.spacingSmall

                Item {
                    Layout.fillWidth: true
                }

                AppButton {
                    objectName: root.objectName.length > 0 ? root.objectName + "Cancel" : ""
                    theme: root.theme
                    text: root.cancelText
                    variant: root.cancelButtonVariant
                    disabled: !root.cancelEnabled
                    onClicked: root.rejectDialog()
                }

                AppButton {
                    objectName: root.objectName.length > 0 ? root.objectName + "Confirm" : ""
                    theme: root.theme
                    text: root.confirmText
                    variant: root.confirmButtonVariant
                    disabled: !root.confirmEnabled
                    onClicked: root.acceptDialog()
                }
            }
        }
    }
}
