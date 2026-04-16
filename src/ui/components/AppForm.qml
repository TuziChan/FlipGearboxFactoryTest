pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property string title: ""
    property string description: ""
    property bool disabled: false
    property bool readOnly: false
    property bool dense: false
    property real maximumWidth: -1
    property int fieldSpacing: root.dense ? root.theme.spacingMedium : root.theme.spacingLarge
    property int sectionSpacing: root.dense ? root.theme.spacingLarge : root.theme.spacingXLarge
    default property alias content: contentColumn.data
    property alias headerContent: headerSlot.data
    property alias footerContent: footerSlot.data
    property alias actions: actionRow.data

    readonly property bool hasHeader: root.title.length > 0 || root.description.length > 0 || headerSlot.children.length > 0
    readonly property bool hasFooter: footerSlot.children.length > 0 || actionRow.children.length > 0

    implicitWidth: root.maximumWidth > 0 ? Math.min(root.maximumWidth, frame.implicitWidth) : frame.implicitWidth
    implicitHeight: frame.implicitHeight
    opacity: root.disabled ? 0.72 : 1
    enabled: !root.disabled

    ColumnLayout {
        id: frame
        anchors.fill: parent
        spacing: root.sectionSpacing

        ColumnLayout {
            id: headerColumn
            Layout.fillWidth: true
            spacing: root.theme.spacingTiny
            visible: root.hasHeader

            Text {
                visible: root.title.length > 0
                text: root.title
                color: root.theme.textPrimary
                font.pixelSize: 14
                font.bold: true
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Text {
                visible: root.description.length > 0
                text: root.description
                color: root.theme.textSecondary
                font.pixelSize: 11
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Item {
                id: headerSlot
                Layout.fillWidth: true
                implicitWidth: childrenRect.width
                implicitHeight: childrenRect.height
                visible: children.length > 0
            }
        }

        ColumnLayout {
            id: contentColumn
            Layout.fillWidth: true
            spacing: root.fieldSpacing
        }

        ColumnLayout {
            id: footerColumn
            Layout.fillWidth: true
            spacing: root.theme.spacingMedium
            visible: root.hasFooter

            Item {
                id: footerSlot
                Layout.fillWidth: true
                implicitWidth: childrenRect.width
                implicitHeight: childrenRect.height
                visible: children.length > 0
            }

            RowLayout {
                id: actionRow
                Layout.fillWidth: true
                spacing: root.theme.spacingSmall
                visible: children.length > 0
            }
        }
    }
}
