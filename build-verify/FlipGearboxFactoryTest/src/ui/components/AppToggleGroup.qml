pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property var model: []
    property string type: "single"
    property string orientation: "horizontal"
    property string variant: "default"
    property string size: "default"
    property int spacing: 0
    property bool disabled: false
    property var value: undefined
    property var values: []
    signal selectionChanged(var selection)
    signal itemTriggered(int index, var value, bool checked)

    readonly property bool multiple: type === "multiple"

    function itemCount() {
        if (Array.isArray(root.model))
            return root.model.length
        if (root.model && root.model.count !== undefined)
            return root.model.count
        return 0
    }

    function itemAt(index) {
        if (index < 0 || index >= root.itemCount())
            return null
        if (Array.isArray(root.model))
            return root.model[index]
        if (root.model && root.model.get)
            return root.model.get(index)
        return null
    }

    function itemTextAt(index) {
        const item = root.itemAt(index)
        if (typeof item === "string" || typeof item === "number")
            return String(item)
        return item && (item.text || item.label || item.title) || ""
    }

    function itemIconAt(index) {
        const item = root.itemAt(index)
        return item && (item.iconName || item.icon) || ""
    }

    function itemIconPositionAt(index) {
        const item = root.itemAt(index)
        return item && item.iconPosition || "start"
    }

    function itemValueAt(index) {
        const item = root.itemAt(index)
        if (item && typeof item === "object") {
            if (item.value !== undefined)
                return item.value
            if (item.id !== undefined)
                return item.id
        }
        return root.itemTextAt(index)
    }

    function itemVariantAt(index) {
        const item = root.itemAt(index)
        return item && item.variant || root.variant
    }

    function itemSizeAt(index) {
        const item = root.itemAt(index)
        return item && item.size || root.size
    }

    function itemEnabledAt(index) {
        const item = root.itemAt(index)
        if (root.disabled)
            return false
        if (!item || typeof item !== "object")
            return true
        if (item.disabled === true)
            return false
        if (item.enabled === false)
            return false
        return true
    }

    function containsValue(list, candidate) {
        if (!Array.isArray(list))
            return false
        return list.indexOf(candidate) >= 0
    }

    function normalizedValues(list) {
        const input = Array.isArray(list) ? list : []
        const uniqueValues = []

        for (let i = 0; i < input.length; i += 1) {
            if (uniqueValues.indexOf(input[i]) < 0)
                uniqueValues.push(input[i])
        }

        return uniqueValues
    }

    function itemCheckedAt(index) {
        const item = root.itemAt(index)
        const candidate = root.itemValueAt(index)

        if (root.multiple) {
            if (Array.isArray(root.values) && root.values.length > 0)
                return root.containsValue(root.values, candidate)
            return !!(item && typeof item === "object" && item.checked)
        }

        if (root.value !== undefined && root.value !== null && root.value !== "")
            return root.value === candidate

        return !!(item && typeof item === "object" && item.checked)
    }

    function setSingleValue(nextValue, emitSignal) {
        root.value = nextValue
        if (emitSignal)
            root.selectionChanged(root.value)
    }

    function setMultipleValues(nextValues, emitSignal) {
        root.values = root.normalizedValues(nextValues)
        if (emitSignal)
            root.selectionChanged(root.values.slice())
    }

    function toggleIndex(index, requestedChecked) {
        const candidate = root.itemValueAt(index)
        const nextChecked = requestedChecked === undefined ? !root.itemCheckedAt(index) : requestedChecked

        if (root.multiple) {
            const nextValues = root.normalizedValues(root.values).slice()
            const existingIndex = nextValues.indexOf(candidate)

            if (existingIndex >= 0)
                nextValues.splice(existingIndex, 1)
            else
                nextValues.push(candidate)

            root.setMultipleValues(nextValues, true)
        } else {
            root.setSingleValue(nextChecked ? candidate : undefined, true)
        }

        root.itemTriggered(index, candidate, nextChecked)
    }

    function selectValue(nextValue) {
        const count = root.itemCount()
        for (let index = 0; index < count; index += 1) {
            if (root.itemValueAt(index) === nextValue) {
                root.toggleIndex(index, true)
                return true
            }
        }
        return false
    }

    function toggleValue(nextValue) {
        const count = root.itemCount()
        for (let index = 0; index < count; index += 1) {
            if (root.itemValueAt(index) === nextValue) {
                root.toggleIndex(index)
                return true
            }
        }
        return false
    }

    implicitWidth: root.orientation === "vertical" ? verticalLayout.implicitWidth : horizontalLayout.implicitWidth
    implicitHeight: root.orientation === "vertical" ? verticalLayout.implicitHeight : horizontalLayout.implicitHeight

    Row {
        id: horizontalLayout
        visible: root.orientation !== "vertical"
        spacing: root.spacing

        Repeater {
            model: root.itemCount()

            delegate: AppToggle {
                required property int index

                theme: root.theme
                text: root.itemTextAt(index)
                iconName: root.itemIconAt(index)
                iconPosition: root.itemIconPositionAt(index)
                variant: root.itemVariantAt(index)
                size: root.itemSizeAt(index)
                checked: root.itemCheckedAt(index)
                autoToggle: false
                disabled: !root.itemEnabledAt(index)
                onChanged: function(nextChecked) {
                    root.toggleIndex(index, nextChecked)
                }
            }
        }
    }

    Column {
        id: verticalLayout
        visible: root.orientation === "vertical"
        spacing: root.spacing

        Repeater {
            model: root.itemCount()

            delegate: AppToggle {
                required property int index

                theme: root.theme
                text: root.itemTextAt(index)
                iconName: root.itemIconAt(index)
                iconPosition: root.itemIconPositionAt(index)
                variant: root.itemVariantAt(index)
                size: root.itemSizeAt(index)
                checked: root.itemCheckedAt(index)
                autoToggle: false
                disabled: !root.itemEnabledAt(index)
                onChanged: function(nextChecked) {
                    root.toggleIndex(index, nextChecked)
                }
            }
        }
    }
}
