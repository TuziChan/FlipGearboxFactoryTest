pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property var model: []
    property int currentIndex: -1
    property string separatorIconName: "chevron-right"
    property string separatorText: ""
    property bool useIconSeparator: separatorText.length === 0
    property int spacing: theme.spacingSmall
    signal crumbTriggered(int index, var value)

    Accessible.role: Accessible.PageTabList
    Accessible.name: "breadcrumb"

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
        return item && (item.text || item.label || item.title || item.name) || ""
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

    function itemCurrentAt(index) {
        const item = root.itemAt(index)
        if (item && typeof item === "object" && item.current !== undefined)
            return !!item.current
        if (root.currentIndex >= 0)
            return index === root.currentIndex
        return index === root.itemCount() - 1
    }

    function itemElidedAt(index) {
        const item = root.itemAt(index)
        return !!(item && typeof item === "object" && item.ellipsis)
    }

    implicitWidth: breadcrumbRow.implicitWidth
    implicitHeight: breadcrumbRow.implicitHeight

    Row {
        id: breadcrumbRow
        spacing: root.spacing

        Repeater {
            model: root.itemCount()

            delegate: Row {
                id: crumbRow
                required property int index
                spacing: root.spacing

                Item {
                    id: crumbBox
                    width: crumbContent.implicitWidth
                    height: Math.max(18, crumbContent.implicitHeight)

                    Row {
                        id: crumbContent
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 0

                        Text {
                            visible: !root.itemElidedAt(crumbRow.index)
                            text: root.itemTextAt(crumbRow.index)
                            color: root.itemCurrentAt(crumbRow.index)
                                   ? root.theme.textPrimary
                                     : crumbHover.hovered && crumbTap.enabled
                                       ? root.theme.textPrimary
                                     : root.theme.textSecondary
                            font.pixelSize: 14
                            font.bold: false

                            Behavior on color {
                                ColorAnimation { duration: 150 }
                            }
                        }

                        Text {
                            visible: root.itemElidedAt(crumbRow.index)
                            text: "..."
                            color: root.theme.textMuted
                            font.pixelSize: 14
                            font.bold: true
                        }
                    }
                }

                Item {
                    width: visible ? separatorContent.implicitWidth : 0
                    height: visible ? separatorContent.implicitHeight : 0
                    visible: crumbRow.index < root.itemCount() - 1

                    Row {
                        id: separatorContent
                        anchors.centerIn: parent
                        spacing: 0

                        AppIcon {
                            visible: root.useIconSeparator
                            name: root.separatorIconName
                            color: root.theme.textMuted
                            iconSize: 14
                        }

                        Text {
                            visible: !root.useIconSeparator
                            text: root.separatorText
                            color: root.theme.textMuted
                            font.pixelSize: 14
                        }
                    }
                }
            }
        }
    }
}
