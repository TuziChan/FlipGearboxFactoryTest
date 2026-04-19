pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root

    required property AppTheme theme
    property var model: []
    property string orientation: "horizontal"
    property bool segmented: true
    property string variant: "outline"
    property string size: "default"
    property int spacing: segmented ? 0 : theme.spacingSmall
    signal buttonClicked(int index, var value)

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

    function itemTypeAt(index) {
        const item = root.itemAt(index)
        if (item && typeof item === "object" && item.type)
            return item.type
        return "button"
    }

    function itemTextAt(index) {
        const item = root.itemAt(index)
        if (typeof item === "string" || typeof item === "number")
            return String(item)
        return item && (item.text || item.label || item.title) || ""
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

    function itemIconAt(index) {
        const item = root.itemAt(index)
        return item && (item.iconName || item.icon) || ""
    }

    function itemIconPositionAt(index) {
        const item = root.itemAt(index)
        return item && item.iconPosition || "start"
    }

    function itemVariantAt(index) {
        const item = root.itemAt(index)
        return item && item.variant || root.variant
    }

    function itemEnabledAt(index) {
        const item = root.itemAt(index)
        if (!item || typeof item !== "object")
            return true
        if (item.enabled === false)
            return false
        if (item.disabled === true)
            return false
        return true
    }

    function itemPressedAt(index) {
        const item = root.itemAt(index)
        return !!(item && typeof item === "object" && item.pressed)
    }

    function isHorizontal() {
        return root.orientation !== "vertical"
    }

    function cellHeight() {
        return root.size === "sm" ? 24 : root.size === "lg" ? 32 : 28
    }

    function cellPadding() {
        return root.size === "sm" ? 8 : root.size === "lg" ? 14 : 12
    }

    function cellFontSize() {
        return root.size === "sm" ? 11 : 12
    }

    function iconSize() {
        return root.size === "lg" ? 16 : 14
    }

    function backgroundForVariant(variantName, pressed) {
        if (variantName === "default" || variantName === "primary")
            return root.theme.accent
        if (variantName === "secondary")
            return pressed ? root.theme.muted : root.theme.surface
        if (variantName === "ghost")
            return pressed ? root.theme.muted : "transparent"
        return root.theme.cardColor
    }

    function foregroundForVariant(variantName) {
        if (variantName === "default" || variantName === "primary")
            return root.theme.primaryForeground
        if (variantName === "secondary")
            return root.theme.textSecondary
        if (variantName === "ghost")
            return root.theme.textPrimary
        return root.theme.textPrimary
    }

    function separatorThickness() {
        return 1
    }

    implicitWidth: root.isHorizontal() ? rowLayout.implicitWidth : columnLayout.implicitWidth
    implicitHeight: root.isHorizontal() ? rowLayout.implicitHeight : columnLayout.implicitHeight
    radius: theme.radiusMedium
    color: segmented ? theme.cardColor : "transparent"
    border.width: segmented ? 1 : 0
    border.color: theme.stroke
    clip: segmented

    Row {
        id: rowLayout
        visible: root.isHorizontal()
        spacing: root.spacing

        Repeater {
            model: root.itemCount()

            delegate: Loader {
                id: rowLoader
                required property int index
                property int itemIndex: index
                onLoaded: {
                    const loadedItem = item
                    if (loadedItem)
                        loadedItem.itemIndex = itemIndex
                }
                sourceComponent: root.itemTypeAt(index) === "separator"
                                 ? separatorDelegate
                                 : root.itemTypeAt(index) === "text"
                                   ? textDelegate
                                   : buttonDelegate
            }
        }
    }

    Column {
        id: columnLayout
        visible: !root.isHorizontal()
        spacing: root.spacing

        Repeater {
            model: root.itemCount()

            delegate: Loader {
                id: columnLoader
                required property int index
                property int itemIndex: index
                onLoaded: {
                    const loadedItem = item
                    if (loadedItem)
                        loadedItem.itemIndex = itemIndex
                }
                sourceComponent: root.itemTypeAt(index) === "separator"
                                 ? separatorDelegate
                                 : root.itemTypeAt(index) === "text"
                                   ? textDelegate
                                   : buttonDelegate
            }
        }
    }

    Component {
        id: buttonDelegate

        Rectangle {
            id: buttonDelegateRoot
            property int itemIndex: -1

            implicitHeight: root.cellHeight()
            implicitWidth: buttonContent.implicitWidth + root.cellPadding() * 2
            color: root.backgroundForVariant(root.itemVariantAt(buttonDelegateRoot.itemIndex), buttonArea.pressed || root.itemPressedAt(buttonDelegateRoot.itemIndex))
            border.width: root.segmented ? 0 : 1
            border.color: root.theme.stroke
            radius: root.segmented ? 0 : root.theme.radiusMedium
            opacity: root.itemEnabledAt(buttonDelegateRoot.itemIndex) ? 1 : 0.6

            Row {
                id: buttonContent
                anchors.centerIn: parent
                spacing: 6

                AppIcon {
                    visible: root.itemIconAt(buttonDelegateRoot.itemIndex).length > 0 && root.itemIconPositionAt(buttonDelegateRoot.itemIndex) === "start"
                    name: root.itemIconAt(buttonDelegateRoot.itemIndex)
                    color: buttonLabel.color
                    iconSize: root.iconSize()
                }

                Text {
                    id: buttonLabel
                    text: root.itemTextAt(buttonDelegateRoot.itemIndex)
                    color: root.foregroundForVariant(root.itemVariantAt(buttonDelegateRoot.itemIndex))
                    font.pixelSize: root.cellFontSize()
                    font.bold: true
                }

                AppIcon {
                    visible: root.itemIconAt(buttonDelegateRoot.itemIndex).length > 0 && root.itemIconPositionAt(buttonDelegateRoot.itemIndex) === "end"
                    name: root.itemIconAt(buttonDelegateRoot.itemIndex)
                    color: buttonLabel.color
                    iconSize: root.iconSize()
                }
            }

            TapHandler {
                id: buttonTap
                enabled: root.itemEnabledAt(buttonDelegateRoot.itemIndex)
                cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                onTapped: root.buttonClicked(buttonDelegateRoot.itemIndex, root.itemValueAt(buttonDelegateRoot.itemIndex))
            }
        }
    }

    Component {
        id: textDelegate

        Rectangle {
            id: textDelegateRoot
            property int itemIndex: -1

            implicitHeight: root.cellHeight()
            implicitWidth: textLabel.implicitWidth + root.cellPadding() * 2
            color: root.theme.muted
            border.width: root.segmented ? 0 : 1
            border.color: root.theme.stroke
            radius: root.segmented ? 0 : root.theme.radiusMedium

            Text {
                id: textLabel
                anchors.centerIn: parent
                text: root.itemTextAt(textDelegateRoot.itemIndex)
                color: root.theme.textSecondary
                font.pixelSize: root.cellFontSize()
                font.bold: true
            }
        }
    }

    Component {
        id: separatorDelegate

        Rectangle {
            property int itemIndex: -1
            implicitWidth: root.isHorizontal() ? root.separatorThickness() : Math.max(24, root.implicitWidth)
            implicitHeight: root.isHorizontal() ? root.cellHeight() : root.separatorThickness()
            color: root.theme.stroke
        }
    }
}
