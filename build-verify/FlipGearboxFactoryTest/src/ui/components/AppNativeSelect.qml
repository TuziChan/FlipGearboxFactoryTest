pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Column {
    id: root

    required property AppTheme theme
    property string label: ""
    property var model: []
    property alias currentIndex: combo.currentIndex
    readonly property string currentText: combo.displayText
    readonly property string currentValue: root.valueAt(combo.currentIndex)
    signal activated(int index, string value)

    function textAt(index) {
        const item = root.model[index]
        if (item === undefined || item === null)
            return ""
        if (typeof item === "string")
            return item
        return item.text || item.label || item.title || item.name || ""
    }

    function valueAt(index) {
        const item = root.model[index]
        if (item === undefined || item === null)
            return ""
        if (typeof item === "string")
            return item
        if (item.value !== undefined)
            return String(item.value)
        return root.textAt(index)
    }

    function selectIndex(index) {
        combo.currentIndex = index
        return true
    }

    spacing: 6

    AppLabel {
        visible: root.label.length > 0
        theme: root.theme
        muted: true
        text: root.label
    }

    ComboBox {
        id: combo
        implicitWidth: 220
        implicitHeight: 30
        model: root.model.length
        textRole: ""

        delegate: ItemDelegate {
            required property int index
            width: combo.width
            text: root.textAt(index)
        }

        contentItem: Text {
            leftPadding: 10
            rightPadding: 28
            text: combo.displayText
            color: root.theme.textPrimary
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 12
            elide: Text.ElideRight
        }

        background: Rectangle {
            radius: root.theme.radiusSmall
            color: root.theme.cardColor
            border.width: 1
            border.color: root.theme.stroke
        }

        indicator: AppIcon {
            x: combo.width - width - 10
            y: (combo.height - height) / 2
            name: "chevron-down"
            color: root.theme.textMuted
            iconSize: 12
        }

        popup: Popup {
            y: combo.height + 4
            width: combo.width
            padding: 4
            background: Rectangle {
                radius: root.theme.radiusMedium
                color: root.theme.cardColor
                border.width: 1
                border.color: root.theme.dividerColor
            }

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: combo.popup.visible ? combo.delegateModel : null
                currentIndex: combo.highlightedIndex
            }
        }

        onActivated: function(index) {
            root.activated(index, root.currentValue)
        }
    }
}
