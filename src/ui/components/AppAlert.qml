pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    required property AppTheme theme
    property string variant: "default"
    property string title: ""
    property string description: ""
    property real maximumWidth: -1
    default property alias content: contentColumn.data
    property alias icon: iconSlot.data
    property alias actions: actionColumn.data

    readonly property bool destructiveVariant: root.variant === "destructive" || root.variant === "danger"
    readonly property bool warningVariant: root.variant === "warning" || root.variant === "warn"
    readonly property bool successVariant: root.variant === "success" || root.variant === "ok"
    readonly property color accentColor: root.destructiveVariant ? root.theme.danger : root.theme.textPrimary
    readonly property color fillColor: root.theme.cardColor
    readonly property color outlineColor: root.destructiveVariant
                                         ? Qt.rgba(root.theme.danger.r, root.theme.danger.g, root.theme.danger.b, 0.28)
                                         : root.theme.dividerColor

    Accessible.role: Accessible.AlertMessage
    Accessible.name: root.title
    Accessible.description: root.description

    radius: root.theme.radiusLarge
    color: root.fillColor
    border.width: 1
    border.color: root.outlineColor
    implicitWidth: root.maximumWidth > 0
                   ? Math.min(root.maximumWidth, alertLayout.implicitWidth + paddingFrame.anchors.margins * 2)
                   : alertLayout.implicitWidth + paddingFrame.anchors.margins * 2
    implicitHeight: alertLayout.implicitHeight + paddingFrame.anchors.margins * 2

    Item {
        id: paddingFrame
        anchors.fill: parent
        anchors.margins: root.theme.spacingLarge

        RowLayout {
            id: alertLayout
            anchors.fill: parent
            spacing: root.theme.spacingMedium

            ColumnLayout {
                id: iconColumn
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: visible ? implicitWidth : 0
                Layout.topMargin: 2
                spacing: 0
                visible: iconSlot.children.length > 0

                Item {
                    id: iconSlot
                    objectName: root.objectName.length > 0 ? root.objectName + "Icon" : ""
                    implicitWidth: 20
                    implicitHeight: 20
                }
            }

            ColumnLayout {
                id: textColumn
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: root.theme.spacingTiny

                Text {
                    objectName: root.objectName.length > 0 ? root.objectName + "Title" : ""
                    visible: root.title.length > 0
                    text: root.title
                    color: root.accentColor
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                    maximumLineCount: 1
                    wrapMode: Text.NoWrap
                    Layout.fillWidth: true
                }

                Text {
                    objectName: root.objectName.length > 0 ? root.objectName + "Description" : ""
                    visible: root.description.length > 0
                    text: root.description
                    color: root.destructiveVariant ? Qt.rgba(root.theme.danger.r, root.theme.danger.g, root.theme.danger.b, 0.9) : root.theme.textSecondary
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                ColumnLayout {
                    id: contentColumn
                    Layout.fillWidth: true
                    spacing: root.theme.spacingSmall
                }
            }

            ColumnLayout {
                id: actionColumn
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: visible ? implicitWidth : 0
                spacing: root.theme.spacingSmall
                visible: actionColumn.children.length > 0
            }
        }
    }
}
