pragma ComponentBehavior: Bound

import QtQuick

FocusScope {
    id: root

    required property AppTheme theme
    property int length: 6
    property string value: ""
    property int activeIndex: 0
    signal valueChangedByUser(string value)

    function setValue(nextValue) {
        root.value = String(nextValue).slice(0, root.length)
        root.activeIndex = Math.min(root.value.length, root.length - 1)
        return true
    }

    function clear() {
        root.value = ""
        root.activeIndex = 0
        return true
    }

    function backspace() {
        if (root.value.length === 0)
            return false
        root.value = root.value.slice(0, root.value.length - 1)
        root.activeIndex = Math.max(0, root.value.length)
        root.valueChangedByUser(root.value)
        return true
    }

    function appendCharacter(character) {
        if (root.value.length >= root.length)
            return false
        root.value += character
        root.activeIndex = Math.min(root.value.length, root.length - 1)
        root.valueChangedByUser(root.value)
        return true
    }

    implicitWidth: slotRow.implicitWidth
    implicitHeight: slotRow.implicitHeight
    activeFocusOnTab: true

    Keys.onPressed: function(event) {
        if (event.text.length === 1 && /[0-9A-Za-z]/.test(event.text)) {
            root.appendCharacter(event.text)
            event.accepted = true
        } else if (event.key === Qt.Key_Backspace) {
            root.backspace()
            event.accepted = true
        }
    }

    Row {
        id: slotRow
        spacing: 4

        Repeater {
            model: root.length

            delegate: Rectangle {
                id: slotDelegate
                required property int index
                width: 34
                height: 38
                radius: root.theme.radiusSmall
                color: root.theme.cardColor
                border.width: 1
                border.color: root.activeIndex === index ? root.theme.accent : root.theme.stroke

                Text {
                    anchors.centerIn: parent
                    text: slotDelegate.index < root.value.length ? root.value.charAt(slotDelegate.index) : ""
                    color: root.theme.textPrimary
                    font.pixelSize: 14
                    font.bold: true
                }
            }
        }
    }
}
