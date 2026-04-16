pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property string triggerText: ""
    property var model: []
    property int highlightedIndex: -1
    property string selectedText: ""
    signal triggered(int index, string text)

    function itemCount() {
        return Array.isArray(root.model) ? root.model.length : 0
    }

    function itemAt(index) {
        return Array.isArray(root.model) ? root.model[index] : undefined
    }

    function itemTypeAt(index) {
        const item = itemAt(index)
        return item && item.type ? item.type : "item"
    }

    function itemTextAt(index) {
        const item = itemAt(index)
        if (typeof item === "string")
            return item
        return item && (item.text || item.label || item.title) || ""
    }

    function itemShortcutAt(index) {
        const item = itemAt(index)
        return item && item.shortcut ? item.shortcut : ""
    }

    function itemSelectable(index) {
        const type = itemTypeAt(index)
        return type === "item" || type === "checkbox" || type === "radio"
    }

    function itemChecked(index) {
        const item = itemAt(index)
        return !!(item && item.checked)
    }

    function firstSelectableIndex() {
        for (let i = 0; i < itemCount(); ++i) {
            if (itemSelectable(i))
                return i
        }
        return -1
    }

    function openMenu() {
        root.open = true
        popup.open()
        root.highlightedIndex = root.highlightedIndex >= 0 ? root.highlightedIndex : firstSelectableIndex()
        listView.forceActiveFocus()
        if (root.highlightedIndex >= 0)
            listView.positionViewAtIndex(root.highlightedIndex, ListView.Contain)
        return true
    }

    function closeMenu() {
        root.open = false
        popup.close()
        return true
    }

    function moveHighlight(delta) {
        if (!root.open || itemCount() === 0)
            return false

        let next = root.highlightedIndex
        if (next < 0)
            next = firstSelectableIndex()

        do {
            next += delta
            if (next < 0 || next >= itemCount())
                return false
        } while (!itemSelectable(next))

        root.highlightedIndex = next
        listView.positionViewAtIndex(next, ListView.Contain)
        return true
    }

    function updateCheckbox(index) {
        if (!Array.isArray(root.model))
            return
        root.model[index].checked = !root.model[index].checked
        root.model = root.model.slice()
    }

    function updateRadio(index) {
        if (!Array.isArray(root.model))
            return
        const group = root.model[index].group || ""
        for (let i = 0; i < root.model.length; ++i) {
            if ((root.model[i].type || "item") === "radio" && (root.model[i].group || "") === group)
                root.model[i].checked = i === index
        }
        root.model = root.model.slice()
    }

    function selectIndex(index) {
        if (index < 0 || index >= itemCount() || !itemSelectable(index))
            return false
        const type = itemTypeAt(index)
        if (type === "checkbox")
            updateCheckbox(index)
        if (type === "radio")
            updateRadio(index)
        root.selectedText = itemTextAt(index)
        root.highlightedIndex = index
        root.triggered(index, root.selectedText)
        return closeMenu()
    }

    function confirmHighlighted() {
        if (root.highlightedIndex < 0)
            return false
        return selectIndex(root.highlightedIndex)
    }

    implicitWidth: 120
    implicitHeight: 28

    AppButton {
        anchors.fill: parent
        theme: root.theme
        variant: "outline"
        text: root.triggerText
        iconName: "chevron-down"
        iconPosition: "end"
        onClicked: root.open ? root.closeMenu() : root.openMenu()
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Down) {
            root.openMenu()
            root.moveHighlight(1)
            event.accepted = true
        } else if (event.key === Qt.Key_Up) {
            root.openMenu()
            root.moveHighlight(-1)
            event.accepted = true
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
            root.open ? root.confirmHighlighted() : root.openMenu()
            event.accepted = true
        } else if (event.key === Qt.Key_Escape) {
            root.closeMenu()
            event.accepted = true
        }
    }

    Popup {
        id: popup
        y: parent ? parent.height + 4 : 32
        width: 196
        padding: 4
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        onClosed: root.open = false

        enter: Transition {
            ParallelAnimation {
                NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 100; easing.type: Easing.OutCubic }
                NumberAnimation { property: "scale"; from: 0.95; to: 1.0; duration: 100; easing.type: Easing.OutCubic }
            }
        }

        exit: Transition {
            ParallelAnimation {
                NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 100; easing.type: Easing.InCubic }
                NumberAnimation { property: "scale"; from: 1.0; to: 0.95; duration: 100; easing.type: Easing.InCubic }
            }
        }

        background: Rectangle {
            radius: root.theme.radiusMedium
            color: root.theme.bgPopover
            border.width: 1
            border.color: Qt.rgba(0, 0, 0, 0.1)

            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: Qt.rgba(0, 0, 0, 0.1)
                shadowBlur: 0.5
                shadowHorizontalOffset: 0
                shadowVerticalOffset: 2
            }
        }

        contentItem: Column {
            spacing: 2

            ListView {
                id: listView
                width: popup.width - popup.padding * 2
                height: Math.min(contentHeight, 220)
                model: root.itemCount()
                clip: true
                spacing: 2
                focus: popup.visible

                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_Down) {
                        root.moveHighlight(1)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Up) {
                        root.moveHighlight(-1)
                        event.accepted = true
                    } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        root.confirmHighlighted()
                        event.accepted = true
                    } else if (event.key === Qt.Key_Escape) {
                        root.closeMenu()
                        event.accepted = true
                    }
                }

                delegate: Item {
                    id: itemDelegate
                    required property int index
                    readonly property string itemType: root.itemTypeAt(itemDelegate.index)
                    readonly property bool selected: root.highlightedIndex === itemDelegate.index
                    width: listView.width
                    height: itemType === "separator" ? 10 : itemType === "label" ? 24 : 28

                    Rectangle {
                        visible: itemType === "separator"
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        height: 1
                        color: root.theme.dividerColor
                    }

                    Text {
                        visible: itemType === "label"
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                        text: root.itemTextAt(itemDelegate.index)
                        color: root.theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                    }

                    Rectangle {
                        visible: itemType !== "separator" && itemType !== "label"
                        anchors.fill: parent
                        radius: root.theme.radiusSmall
                        color: mouse.containsMouse || itemDelegate.selected ? root.theme.accentWeak : "transparent"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 8

                            AppIcon {
                                visible: itemType === "checkbox" && root.itemChecked(itemDelegate.index)
                                name: "check"
                                color: mouse.containsMouse || itemDelegate.selected ? root.theme.accentForeground : root.theme.accent
                                iconSize: 12
                            }

                            Rectangle {
                                visible: itemType === "radio"
                                width: 8
                                height: 8
                                radius: 4
                                color: root.itemChecked(itemDelegate.index)
                                       ? (mouse.containsMouse || itemDelegate.selected ? root.theme.accentForeground : root.theme.accent)
                                       : Qt.rgba(root.theme.textMuted.r, root.theme.textMuted.g, root.theme.textMuted.b, 0.35)
                            }

                            Text {
                                Layout.fillWidth: true
                                text: root.itemTextAt(itemDelegate.index)
                                color: mouse.containsMouse || itemDelegate.selected ? root.theme.accentForeground : root.theme.textPrimary
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }

                            Text {
                                visible: root.itemShortcutAt(itemDelegate.index).length > 0
                                text: root.itemShortcutAt(itemDelegate.index)
                                color: mouse.containsMouse || itemDelegate.selected ? root.theme.accentForeground : root.theme.textMuted
                                font.pixelSize: 10
                            }
                        }

                        MouseArea {
                            id: mouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: root.highlightedIndex = itemDelegate.index
                            onClicked: root.selectIndex(itemDelegate.index)
                        }
                    }
                }
            }
        }
    }
}
