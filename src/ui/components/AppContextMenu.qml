pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property var model: []
    property int highlightedIndex: -1
    property string selectedText: ""
    property real popupWidth: 216
    property real popupMaxHeight: 280
    property bool closeOnTrigger: true

    signal triggered(int index, string text, var item)

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
        if (!Array.isArray(root.model))
            return

        const nextModel = root.model.slice()
        nextModel[index] = Object.assign({}, nextModel[index], { checked: !nextModel[index].checked })
        root.model = nextModel
    }

    function updateRadio(index) {
        if (!Array.isArray(root.model))
            return

        const group = root.model[index] && root.model[index].group ? root.model[index].group : ""
        const nextModel = root.model.slice()

        for (let i = 0; i < nextModel.length; ++i) {
            const candidate = nextModel[i]
            const type = candidate && candidate.type ? candidate.type : "item"
            const candidateGroup = candidate && candidate.group ? candidate.group : ""
            if (type === "radio" && candidateGroup === group)
                nextModel[i] = Object.assign({}, candidate, { checked: i === index })
        }

        root.model = nextModel
    }

    function openAt(x, y, modelOverride) {
        if (Array.isArray(modelOverride))
            root.model = modelOverride.slice()

        root.highlightedIndex = firstSelectableIndex()
        popup.x = x
        popup.y = y
        popup.open()
        listView.forceActiveFocus()
        if (root.highlightedIndex >= 0)
            listView.positionViewAtIndex(root.highlightedIndex, ListView.Contain)
        return true
    }

    function closeMenu() {
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
        root.triggered(index, root.selectedText, item)
        if (root.closeOnTrigger)
            root.closeMenu()
        return true
    }

    function openForMouse(mouse, modelOverride) {
        if (!mouse)
            return false
        return openAt(mouse.x, mouse.y, modelOverride)
    }

    implicitWidth: 1
    implicitHeight: 1

    Popup {
        id: popup
        width: root.popupWidth
        padding: root.theme.spacingTiny
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        modal: false
        onClosed: root.highlightedIndex = -1

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
                } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                    root.selectIndex(root.highlightedIndex)
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

                    MouseArea {
                        anchors.fill: parent
                        enabled: itemDelegate.optionEnabled
                        hoverEnabled: true
                        onEntered: root.highlightedIndex = itemDelegate.index
                        onClicked: root.selectIndex(itemDelegate.index)
                    }
                }
            }
        }
    }
}
