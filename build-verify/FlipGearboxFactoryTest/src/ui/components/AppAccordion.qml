pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property var model: []
    property string selectionMode: "single"
    property bool collapsible: true
    property bool fillWidth: true
    property bool showIndicators: true
    property int itemPadding: 10
    property int contentPadding: 10
    property var expandedValues: []
    signal itemToggled(int index, bool expanded, string value)

    function itemCount() {
        if (Array.isArray(root.model))
            return root.model.length
        if (root.model && typeof root.model.count === "number")
            return root.model.count
        return 0
    }

    function itemAt(index) {
        if (index < 0 || index >= root.itemCount())
            return null
        if (Array.isArray(root.model))
            return root.model[index]
        if (root.model && typeof root.model.get === "function")
            return root.model.get(index)
        return null
    }

    function itemValueAt(index) {
        const item = root.itemAt(index)
        if (item === null || item === undefined)
            return ""
        if (typeof item === "string")
            return index.toString()
        if (item.value !== undefined)
            return item.value.toString()
        if (item.id !== undefined)
            return item.id.toString()
        return index.toString()
    }

    function itemTitleAt(index) {
        const item = root.itemAt(index)
        if (item === null || item === undefined)
            return ""
        if (typeof item === "string")
            return item
        return item.title || item.label || item.text || ""
    }

    function itemDescriptionAt(index) {
        const item = root.itemAt(index)
        if (!item || typeof item === "string")
            return ""
        return item.description || item.subtitle || ""
    }

    function itemContentAt(index) {
        const item = root.itemAt(index)
        if (item === null || item === undefined || typeof item === "string")
            return ""
        return item.content || item.body || item.details || ""
    }

    function itemDisabledAt(index) {
        const item = root.itemAt(index)
        return !!(item && typeof item === "object" && item.disabled)
    }

    function isExpanded(indexOrValue) {
        const value = typeof indexOrValue === "number" ? root.itemValueAt(indexOrValue) : indexOrValue
        return root.expandedValues.indexOf(value) >= 0
    }

    function expandedIndex(position) {
        const normalizedPosition = Math.max(0, position)
        if (!Array.isArray(root.expandedValues) || normalizedPosition >= root.expandedValues.length)
            return -1

        const targetValue = root.expandedValues[normalizedPosition]
        for (let index = 0; index < root.itemCount(); ++index) {
            if (root.itemValueAt(index) === targetValue)
                return index
        }
        return -1
    }

    function setExpanded(indexOrValue, expanded) {
        const value = typeof indexOrValue === "number" ? root.itemValueAt(indexOrValue) : indexOrValue
        const current = Array.isArray(root.expandedValues) ? root.expandedValues.slice(0) : []
        const existingIndex = current.indexOf(value)

        if (root.selectionMode === "multiple") {
            if (expanded && existingIndex < 0)
                current.push(value)
            else if (!expanded && existingIndex >= 0)
                current.splice(existingIndex, 1)
            root.expandedValues = current
            return true
        }

        if (expanded) {
            root.expandedValues = [value]
            return true
        }

        if (!root.collapsible && existingIndex >= 0)
            return false

        root.expandedValues = []
        return true
    }

    function toggleIndex(index) {
        if (index < 0 || index >= root.itemCount() || root.itemDisabledAt(index))
            return false

        const nextExpanded = !root.isExpanded(index)
        const changed = root.setExpanded(index, nextExpanded)

        if (changed)
            root.itemToggled(index, root.isExpanded(index), root.itemValueAt(index))

        return changed
    }

    implicitWidth: container.implicitWidth
    implicitHeight: container.implicitHeight

    Rectangle {
        id: container
        anchors.fill: parent
        radius: root.theme.radiusMedium
        color: root.theme.cardColor
        border.width: 1
        border.color: root.theme.dividerColor
        implicitWidth: column.implicitWidth + 2
        implicitHeight: column.implicitHeight + 2

        Column {
            id: column
            anchors.fill: parent
            anchors.margins: 1
            spacing: 0

            Repeater {
                model: root.itemCount()

                delegate: Rectangle {
                    id: itemRoot
                    required property int index
                    readonly property bool expanded: root.isExpanded(itemRoot.index)
                    readonly property bool disabled: root.itemDisabledAt(itemRoot.index)
                    readonly property bool hovered: triggerArea.hovered

                    width: root.fillWidth && column.width > 0 ? column.width : implicitWidth
                    implicitWidth: 280
                    implicitHeight: triggerBackground.implicitHeight + contentViewport.height + divider.height
                    color: itemRoot.expanded ? Qt.rgba(root.theme.muted.r, root.theme.muted.g, root.theme.muted.b, 0.45) : root.theme.cardColor
                    opacity: itemRoot.disabled ? 0.6 : 1

                    Keys.onPressed: function(event) {
                        if (itemRoot.disabled)
                            return

                        if (event.key === Qt.Key_Space || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                            root.toggleIndex(itemRoot.index)
                            event.accepted = true
                        }
                    }

                    Column {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        spacing: 0

                        Rectangle {
                            id: triggerBackground
                            width: parent.width
                            implicitHeight: Math.max(40, headerRow.implicitHeight + root.itemPadding * 2)
                            color: "transparent"
                            radius: root.theme.radiusSmall

                            Rectangle {
                                id: focusRing
                                anchors.fill: parent
                                radius: parent.radius
                                color: "transparent"
                                border.width: itemRoot.activeFocus ? 3 : 0
                                border.color: Qt.rgba(root.theme.accent.r, root.theme.accent.g, root.theme.accent.b, 0.5)
                                visible: itemRoot.activeFocus
                            }

                            RowLayout {
                                id: headerRow
                                anchors.fill: parent
                                anchors.margins: root.itemPadding
                                spacing: 12

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Text {
                                        text: root.itemTitleAt(itemRoot.index)
                                        color: root.theme.textPrimary
                                        font.pixelSize: 12
                                        font.bold: true
                                        wrapMode: Text.WordWrap
                                        font.underline: itemRoot.hovered
                                    }

                                    Text {
                                        visible: root.itemDescriptionAt(itemRoot.index).length > 0
                                        text: root.itemDescriptionAt(itemRoot.index)
                                        color: root.theme.textSecondary
                                        font.pixelSize: 11
                                        wrapMode: Text.WordWrap
                                    }
                                }

                                Item {
                                    visible: root.showIndicators
                                    Layout.preferredWidth: 16
                                    Layout.preferredHeight: 16

                                    AppIcon {
                                        anchors.centerIn: parent
                                        name: "chevron-down"
                                        color: root.theme.textMuted
                                        iconSize: 14
                                        rotation: itemRoot.expanded ? 180 : 0

                                        Behavior on rotation {
                                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                                        }
                                    }
                                }
                            }

                            TapHandler {
                                id: triggerTap
                                enabled: !itemRoot.disabled
                                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onTapped: root.toggleIndex(itemRoot.index)
                            }

                            HoverHandler {
                                id: triggerArea
                                enabled: !itemRoot.disabled
                            }
                        }

                        Item {
                            id: contentViewport
                            width: parent.width
                            height: itemRoot.expanded ? contentFrame.implicitHeight : 0
                            clip: true
                            visible: height > 0 || contentFade.running
                            opacity: itemRoot.expanded ? 1 : 0

                            Behavior on height {
                                NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
                            }

                            Behavior on opacity {
                                NumberAnimation {
                                    id: contentFade
                                    duration: itemRoot.expanded ? 120 : 90
                                    easing.type: Easing.OutCubic
                                }
                            }

                            Rectangle {
                                id: contentFrame
                                width: parent.width
                                implicitHeight: contentText.implicitHeight + root.contentPadding * 2 + 4
                                color: "transparent"

                                Text {
                                    id: contentText
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.margins: root.contentPadding
                                    text: root.itemContentAt(itemRoot.index)
                                    visible: text.length > 0
                                    color: root.theme.textSecondary
                                    font.pixelSize: 11
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        Rectangle {
                            id: divider
                            visible: itemRoot.index < root.itemCount() - 1
                            width: parent.width
                            height: 1
                            color: root.theme.dividerColor
                        }
                    }
                }
            }
        }
    }
}
