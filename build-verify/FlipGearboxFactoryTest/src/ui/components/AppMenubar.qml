pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property var menus: []
    property int currentMenuIndex: -1
    property int highlightedIndex: -1
    property string selectedText: ""
    property real popupWidth: 208
    property real popupMaxHeight: 260

    signal itemTriggered(int menuIndex, int itemIndex, string text, var item)

    function menuCount() {
        return Array.isArray(root.menus) ? root.menus.length : 0
    }

    function menuAt(index) {
        return Array.isArray(root.menus) ? root.menus[index] : undefined
    }

    function menuTextAt(index) {
        const menu = menuAt(index)
        return menu && (menu.text || menu.label || menu.title) || ""
    }

    function menuItemsAt(index) {
        const menu = menuAt(index)
        return menu && Array.isArray(menu.items) ? menu.items : []
    }

    function itemCount() {
        return root.currentMenuIndex >= 0 ? menuItemsAt(root.currentMenuIndex).length : 0
    }

    function itemAt(index) {
        const items = menuItemsAt(root.currentMenuIndex)
        return index >= 0 && index < items.length ? items[index] : undefined
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

    function itemIconAt(index) {
        const item = itemAt(index)
        return item && item.icon ? item.icon : ""
    }

    function itemVariantAt(index) {
        const item = itemAt(index)
        return item && item.variant ? item.variant : "default"
    }

    function itemEnabled(index) {
        const item = itemAt(index)
        return !(item && item.disabled)
    }

    function itemSelectable(index) {
        const type = itemTypeAt(index)
        return itemEnabled(index) && (type === "item" || type === "checkbox" || type === "radio")
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

    function nextSelectableIndex(startIndex, delta) {
        if (itemCount() === 0)
            return -1

        let index = startIndex
        for (let step = 0; step < itemCount(); ++step) {
            index += delta
            if (index < 0)
                index = itemCount() - 1
            else if (index >= itemCount())
                index = 0

            if (itemSelectable(index))
                return index
        }

        return -1
    }

    function updateCheckbox(index) {
        if (root.currentMenuIndex < 0 || !Array.isArray(root.menus))
            return

        const nextMenus = root.menus.slice()
        const nextMenu = Object.assign({}, nextMenus[root.currentMenuIndex])
        const nextItems = menuItemsAt(root.currentMenuIndex).slice()
        nextItems[index] = Object.assign({}, nextItems[index], { checked: !nextItems[index].checked })
        nextMenu.items = nextItems
        nextMenus[root.currentMenuIndex] = nextMenu
        root.menus = nextMenus
    }

    function updateRadio(index) {
        if (root.currentMenuIndex < 0 || !Array.isArray(root.menus))
            return

        const currentItems = menuItemsAt(root.currentMenuIndex)
        const group = currentItems[index] && currentItems[index].group ? currentItems[index].group : ""
        const nextMenus = root.menus.slice()
        const nextMenu = Object.assign({}, nextMenus[root.currentMenuIndex])
        const nextItems = currentItems.slice()

        for (let i = 0; i < nextItems.length; ++i) {
            const candidate = nextItems[i]
            const type = candidate && candidate.type ? candidate.type : "item"
            const candidateGroup = candidate && candidate.group ? candidate.group : ""
            if (type === "radio" && candidateGroup === group)
                nextItems[i] = Object.assign({}, candidate, { checked: i === index })
        }

        nextMenu.items = nextItems
        nextMenus[root.currentMenuIndex] = nextMenu
        root.menus = nextMenus
    }

    function openMenu(index) {
        if (index < 0 || index >= menuCount())
            return false

        root.currentMenuIndex = index
        root.highlightedIndex = firstSelectableIndex()

        const triggerItem = triggerRepeater.itemAt(index)
        if (!triggerItem)
            return false

        const point = triggerItem.mapToItem(root, 0, triggerItem.height + root.theme.spacingTiny + 2)
        popup.x = point.x
        popup.y = point.y
        popup.open()
        listView.forceActiveFocus()
        if (root.highlightedIndex >= 0)
            listView.positionViewAtIndex(root.highlightedIndex, ListView.Contain)
        return true
    }

    function closeMenus() {
        root.currentMenuIndex = -1
        root.highlightedIndex = -1
        popup.close()
        return true
    }

    function moveHighlight(delta) {
        if (!popup.visible)
            return false

        if (root.highlightedIndex < 0)
            root.highlightedIndex = firstSelectableIndex()
        else
            root.highlightedIndex = nextSelectableIndex(root.highlightedIndex, delta)

        if (root.highlightedIndex >= 0)
            listView.positionViewAtIndex(root.highlightedIndex, ListView.Contain)
        return root.highlightedIndex >= 0
    }

    function moveMenu(delta) {
        if (menuCount() === 0)
            return false

        let nextMenu = root.currentMenuIndex
        if (nextMenu < 0)
            nextMenu = 0
        else {
            nextMenu += delta
            if (nextMenu < 0)
                nextMenu = menuCount() - 1
            else if (nextMenu >= menuCount())
                nextMenu = 0
        }

        return openMenu(nextMenu)
    }

    function selectIndex(index) {
        if (!itemSelectable(index))
            return false

        const item = itemAt(index)
        const type = itemTypeAt(index)
        if (type === "checkbox")
            updateCheckbox(index)
        else if (type === "radio")
            updateRadio(index)

        root.highlightedIndex = index
        root.selectedText = itemTextAt(index)
        root.itemTriggered(root.currentMenuIndex, index, root.selectedText, item)
        return closeMenus()
    }

    implicitWidth: rowLayout.implicitWidth + root.theme.spacingSmall * 2
    implicitHeight: 36

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Left) {
            root.moveMenu(-1)
            event.accepted = true
        } else if (event.key === Qt.Key_Right) {
            root.moveMenu(1)
            event.accepted = true
        } else if (event.key === Qt.Key_Down) {
            if (root.currentMenuIndex < 0)
                root.openMenu(0)
            else
                root.moveHighlight(1)
            event.accepted = true
        } else if (event.key === Qt.Key_Up) {
            if (root.currentMenuIndex >= 0)
                root.moveHighlight(-1)
            event.accepted = true
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
            if (root.currentMenuIndex < 0)
                root.openMenu(0)
            else
                root.selectIndex(root.highlightedIndex)
            event.accepted = true
        } else if (event.key === Qt.Key_Escape) {
            root.closeMenus()
            event.accepted = true
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: root.theme.radiusMedium
        color: root.theme.cardColor
        border.width: 1
        border.color: root.theme.borderColor
    }

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        anchors.margins: root.theme.spacingTiny
        spacing: root.theme.spacingTiny

        Repeater {
            id: triggerRepeater
            model: root.menuCount()

            delegate: Rectangle {
                id: triggerItem
                required property int index
                readonly property bool active: root.currentMenuIndex === triggerItem.index && popup.visible
                readonly property bool hovered: triggerHover.hovered
                readonly property color triggerTextColor: active || hovered ? root.theme.accentForeground : root.theme.textPrimary

                Layout.preferredWidth: triggerLabel.implicitWidth + root.theme.spacingLarge
                Layout.fillHeight: true
                radius: root.theme.radiusSmall
                color: active || hovered ? root.theme.accentWeak : "transparent"

                Text {
                    id: triggerLabel
                    anchors.centerIn: parent
                    text: root.menuTextAt(triggerItem.index)
                    color: triggerItem.triggerTextColor
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        }
    }

    Popup {
        id: popup
        width: root.popupWidth
        padding: root.theme.spacingTiny
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        modal: false
        onClosed: {
            root.currentMenuIndex = -1
            root.highlightedIndex = -1
        }

        background: Rectangle {
            radius: root.theme.radiusMedium
            color: root.theme.cardColor
            border.width: 1
            border.color: root.theme.borderColor
        }

        contentItem: ListView {
            id: listView
            width: popup.width - popup.padding * 2
            height: Math.min(contentHeight, root.popupMaxHeight)
            model: root.itemCount()
            clip: true
            spacing: 2
            boundsBehavior: Flickable.StopAtBounds
            focus: popup.visible

            Keys.onPressed: function(event) {
                if (event.key === Qt.Key_Down) {
                    root.moveHighlight(1)
                    event.accepted = true
                } else if (event.key === Qt.Key_Up) {
                    root.moveHighlight(-1)
                    event.accepted = true
                } else if (event.key === Qt.Key_Left) {
                    root.moveMenu(-1)
                    event.accepted = true
                } else if (event.key === Qt.Key_Right) {
                    root.moveMenu(1)
                    event.accepted = true
                } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                    root.selectIndex(root.highlightedIndex)
                    event.accepted = true
                } else if (event.key === Qt.Key_Escape) {
                    root.closeMenus()
                    event.accepted = true
                }
            }

            delegate: Item {
                id: itemDelegate
                required property int index

                readonly property string itemType: root.itemTypeAt(itemDelegate.index)
                readonly property bool selected: root.highlightedIndex === itemDelegate.index
                readonly property bool optionEnabled: root.itemEnabled(itemDelegate.index)
                readonly property bool destructive: root.itemVariantAt(itemDelegate.index) === "destructive"
                readonly property color selectedColor: destructive ? root.theme.dangerWeak : root.theme.accentWeak
                readonly property color activeForeground: destructive ? root.theme.danger : root.theme.accentForeground
                readonly property color idleForeground: destructive ? root.theme.danger : root.theme.textPrimary
                readonly property color leadingColor: selected ? activeForeground : (destructive ? root.theme.danger : root.theme.textMuted)

                width: listView.width
                height: itemType === "separator" ? 10 : itemType === "label" ? 24 : 28
                opacity: optionEnabled ? 1 : 0.5

                Rectangle {
                    visible: itemDelegate.itemType === "separator"
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    height: 1
                    color: root.theme.dividerColor
                }

                Text {
                    visible: itemDelegate.itemType === "label"
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.itemTextAt(itemDelegate.index)
                    color: root.theme.textMuted
                    font.pixelSize: 11
                    font.bold: true
                }

                Rectangle {
                    visible: itemDelegate.itemType !== "separator" && itemDelegate.itemType !== "label"
                    anchors.fill: parent
                    radius: root.theme.radiusSmall
                    color: itemDelegate.selected ? itemDelegate.selectedColor : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        Item {
                            Layout.preferredWidth: 14
                            Layout.preferredHeight: 14

                            AppIcon {
                                anchors.centerIn: parent
                                visible: root.itemIconAt(itemDelegate.index).length > 0
                                name: root.itemIconAt(itemDelegate.index)
                                color: itemDelegate.leadingColor
                                iconSize: 12
                            }

                            AppIcon {
                                anchors.centerIn: parent
                                visible: root.itemIconAt(itemDelegate.index).length === 0
                                         && itemDelegate.itemType === "checkbox"
                                         && root.itemChecked(itemDelegate.index)
                                name: "check"
                                color: itemDelegate.leadingColor
                                iconSize: 12
                            }

                            Rectangle {
                                anchors.centerIn: parent
                                visible: root.itemIconAt(itemDelegate.index).length === 0
                                         && itemDelegate.itemType === "radio"
                                width: 8
                                height: 8
                                radius: 4
                                color: root.itemChecked(itemDelegate.index)
                                       ? itemDelegate.leadingColor
                                       : Qt.rgba(root.theme.textMuted.r, root.theme.textMuted.g, root.theme.textMuted.b, 0.35)
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: root.itemTextAt(itemDelegate.index)
                            color: itemDelegate.selected ? itemDelegate.activeForeground : itemDelegate.idleForeground
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }

                        Text {
                            visible: root.itemShortcutAt(itemDelegate.index).length > 0
                            text: root.itemShortcutAt(itemDelegate.index)
                            color: itemDelegate.selected ? itemDelegate.activeForeground : root.theme.textMuted
                            font.pixelSize: 10
                        }
                    }

                    TapHandler {
                        enabled: itemDelegate.optionEnabled
                        onTapped: root.selectIndex(itemDelegate.index)
                    }

                    HoverHandler {
                        enabled: itemDelegate.optionEnabled
                        onHoveredChanged: if (hovered) root.highlightedIndex = itemDelegate.index
                    }
                }
            }
        }
    }
}
