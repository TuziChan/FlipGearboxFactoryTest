pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property var model: []
    property int currentIndex: -1
    property string value: ""
    property string orientation: "vertical"
    property bool disabled: false
    property string textRole: "text"
    property string valueRole: "value"
    property string enabledRole: "enabled"
    signal selectionChanged(int index, string value)

    property bool _syncing: false

    implicitWidth: contentLoader.implicitWidth
    implicitHeight: contentLoader.implicitHeight
    activeFocusOnTab: !root.disabled

    function itemCount() {
        if (Array.isArray(root.model))
            return root.model.length
        if (root.model && root.model.count !== undefined)
            return root.model.count
        return 0
    }

    function itemAt(index) {
        if (index < 0 || index >= itemCount())
            return undefined
        if (Array.isArray(root.model))
            return root.model[index]
        if (root.model && root.model.get)
            return root.model.get(index)
        return undefined
    }

    function itemTextAt(index) {
        const item = itemAt(index)
        if (item === undefined || item === null)
            return ""
        if (typeof item === "string")
            return item
        if (typeof item === "object")
            return item[root.textRole] || item.label || item.title || item.name || ""
        return String(item)
    }

    function itemValueAt(index) {
        const item = itemAt(index)
        if (item === undefined || item === null)
            return ""
        if (typeof item === "string")
            return item
        if (typeof item === "object") {
            const candidate = item[root.valueRole]
            if (candidate !== undefined && candidate !== null)
                return String(candidate)
            if (item.id !== undefined && item.id !== null)
                return String(item.id)
            if (item.key !== undefined && item.key !== null)
                return String(item.key)
        }
        return root.itemTextAt(index)
    }

    function itemEnabledAt(index) {
        const item = itemAt(index)
        if (item === undefined || item === null)
            return false
        if (typeof item === "string")
            return true
        if (typeof item === "object") {
            if (item[root.enabledRole] !== undefined)
                return !!item[root.enabledRole]
            if (item.disabled !== undefined)
                return !item.disabled
        }
        return true
    }

    function findIndexByValue(nextValue) {
        const normalized = nextValue === undefined || nextValue === null ? "" : String(nextValue)
        for (let index = 0; index < itemCount(); ++index) {
            if (root.itemValueAt(index) === normalized)
                return index
        }
        return -1
    }

    function syncSelection() {
        if (root._syncing)
            return

        root._syncing = true

        const count = root.itemCount()
        const indexFromValue = root.findIndexByValue(root.value)

        if (indexFromValue >= 0) {
            root.currentIndex = indexFromValue
            root.value = root.itemValueAt(indexFromValue)
        } else if (root.currentIndex >= 0 && root.currentIndex < count) {
            root.value = root.itemValueAt(root.currentIndex)
        } else {
            root.currentIndex = -1
            root.value = ""
        }

        root._syncing = false
    }

    function selectIndex(index) {
        if (root.disabled || index < 0 || index >= root.itemCount() || !root.itemEnabledAt(index))
            return false

        const nextValue = root.itemValueAt(index)
        if (root.currentIndex === index && root.value === nextValue)
            return true

        root._syncing = true
        root.currentIndex = index
        root.value = nextValue
        root._syncing = false
        root.selectionChanged(index, nextValue)
        return true
    }

    function selectValue(nextValue) {
        return root.selectIndex(root.findIndexByValue(nextValue))
    }

    function moveSelection(delta) {
        const count = root.itemCount()
        if (root.disabled || count === 0)
            return false

        let index = root.currentIndex
        if (index < 0)
            index = delta > 0 ? -1 : 0

        for (let step = 0; step < count; ++step) {
            index = (index + delta + count) % count
            if (root.itemEnabledAt(index))
                return root.selectIndex(index)
        }

        return false
    }

    onModelChanged: root.syncSelection()
    onValueChanged: if (!root._syncing) root.syncSelection()
    onCurrentIndexChanged: if (!root._syncing) root.syncSelection()
    Component.onCompleted: root.syncSelection()

    Keys.onPressed: function(event) {
        if (root.disabled)
            return

        if (root.orientation === "horizontal") {
            if (event.key === Qt.Key_Right || event.key === Qt.Key_Down) {
                root.moveSelection(1)
                event.accepted = true
            } else if (event.key === Qt.Key_Left || event.key === Qt.Key_Up) {
                root.moveSelection(-1)
                event.accepted = true
            }
        } else if (event.key === Qt.Key_Down || event.key === Qt.Key_Right) {
            root.moveSelection(1)
            event.accepted = true
        } else if (event.key === Qt.Key_Up || event.key === Qt.Key_Left) {
            root.moveSelection(-1)
            event.accepted = true
        }
    }

    Loader {
        id: contentLoader
        anchors.fill: parent
        sourceComponent: root.orientation === "horizontal" ? horizontalContent : verticalContent
    }

    Component {
        id: optionDelegate

        Item {
            id: optionRoot
            required property int index

            readonly property bool checkedOption: root.currentIndex === optionRoot.index
            readonly property bool enabledOption: !root.disabled && root.itemEnabledAt(optionRoot.index)

            implicitWidth: optionRow.implicitWidth
            implicitHeight: Math.max(16, optionRow.implicitHeight)

            Row {
                id: optionRow
                spacing: 8

                Rectangle {
                    width: 16
                    height: 16
                    radius: 8
                    color: optionRoot.checkedOption ? root.theme.accent : root.theme.cardColor
                    border.width: 1
                    border.color: root.activeFocus && optionRoot.checkedOption
                                  ? root.theme.accent
                                  : optionRoot.checkedOption
                                    ? root.theme.accent
                                    : root.theme.stroke
                    opacity: optionRoot.enabledOption ? 1 : 0.5

                    Rectangle {
                        anchors.centerIn: parent
                        width: 8
                        height: 8
                        radius: 4
                        visible: optionRoot.checkedOption
                        color: root.theme.primaryForeground
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.itemTextAt(optionRoot.index)
                    color: optionRoot.enabledOption ? root.theme.textPrimary : root.theme.textMuted
                    font.pixelSize: 12
                }
            }

            TapHandler {
                enabled: optionRoot.enabledOption
                onTapped: root.selectIndex(optionRoot.index)
            }
        }
    }

    Component {
        id: verticalContent

        Column {
            spacing: root.theme.spacingMedium

            Repeater {
                model: root.itemCount()
                delegate: optionDelegate
            }
        }
    }

    Component {
        id: horizontalContent

        Row {
            spacing: root.theme.spacingLarge

            Repeater {
                model: root.itemCount()
                delegate: optionDelegate
            }
        }
    }
}
