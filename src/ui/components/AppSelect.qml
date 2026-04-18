pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Column {
    id: root

    required property AppTheme theme
    property string label: ""
    property var model: []
    property int currentIndex: 0
    property int highlightedIndex: currentIndex
    readonly property string currentText: itemTextAt(currentIndex)
    property var currentValue: undefined
    property string size: "default"
    property string popupObjectName: ""
    signal activated(int index)

    spacing: 6

    onCurrentIndexChanged: {
        const item = itemAt(currentIndex)
        if (item && typeof item === "object" && item.value !== undefined) {
            root.currentValue = item.value
        }
    }

    onCurrentValueChanged: {
        if (root.currentValue === undefined)
            return
        for (let i = 0; i < itemCount(); i++) {
            const item = itemAt(i)
            if (item && typeof item === "object" && item.value === root.currentValue) {
                if (root.currentIndex !== i) {
                    root.currentIndex = i
                }
                return
            }
        }
    }

    function isArrayLikeModel(model) {
        return model !== undefined
            && model !== null
            && typeof model !== "string"
            && model.length !== undefined
            && typeof model.length === "number"
    }

    function itemCount() {
        if (Array.isArray(root.model))
            return root.model.length
        if (isArrayLikeModel(root.model))
            return root.model.length
        if (root.model && root.model.count !== undefined)
            return root.model.count
        return 0
    }

    function itemAt(index) {
        if (Array.isArray(root.model))
            return root.model[index]
        if (isArrayLikeModel(root.model))
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
            return item.text || item.title || item.label || item.name || JSON.stringify(item)
        return String(item)
    }

    function openPopup() {
        if (!root.enabled)
            return false
        root.highlightedIndex = root.currentIndex
        popup.open()
        listView.forceActiveFocus()
        listView.positionViewAtIndex(root.highlightedIndex, ListView.Contain)
        return true
    }

    function closePopup() {
        popup.close()
        return true
    }

    function moveHighlight(delta) {
        if (!popup.visible || itemCount() === 0)
            return false
        const next = Math.max(0, Math.min(itemCount() - 1, root.highlightedIndex + delta))
        root.highlightedIndex = next
        listView.positionViewAtIndex(root.highlightedIndex, ListView.Contain)
        return true
    }

    function confirmHighlighted() {
        if (!popup.visible)
            return false
        return root.selectIndex(root.highlightedIndex)
    }

    function selectIndex(index) {
        if (index < 0 || index >= itemCount())
            return false
        root.currentIndex = index
        root.highlightedIndex = index
        popup.close()
        root.activated(index)
        return true
    }

    Text {
        visible: root.label.length > 0
        text: root.label
        color: root.theme.textSecondary
        font.pixelSize: 12
    }

    Rectangle {
        id: trigger
        width: root.width > 0 ? root.width : implicitWidth
        implicitWidth: 160
        implicitHeight: root.size === "sm" ? 24 : 28
        radius: root.theme.radiusSmall
        color: Qt.tint(root.theme.cardColor, Qt.rgba(root.theme.stroke.r, root.theme.stroke.g, root.theme.stroke.b, 0.18))
        border.width: 1
        border.color: root.activeFocus ? root.theme.accent : root.theme.stroke

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            Text {
                Layout.fillWidth: true
                text: root.currentText
                color: root.theme.textPrimary
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 12
                elide: Text.ElideRight
            }

            AppIcon {
                name: popup.visible ? "chevron-up" : "chevron-down"
                color: root.theme.textMuted
                iconSize: 12
            }
        }

        MouseArea {
            anchors.fill: parent
            enabled: root.enabled
            onClicked: root.openPopup()
        }

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Down || event.key === Qt.Key_Up) {
                root.openPopup()
                root.moveHighlight(event.key === Qt.Key_Down ? 1 : -1)
                event.accepted = true
            } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                root.openPopup()
                event.accepted = true
            }
        }
    }

    Popup {
        id: popup
        objectName: root.popupObjectName
        y: trigger.height + 4
        width: Math.max(trigger.width, 140)
        padding: 4
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        background: Rectangle {
            radius: root.theme.radiusMedium
            color: Qt.tint(root.theme.cardColor, Qt.rgba(root.theme.surface.r, root.theme.surface.g, root.theme.surface.b, 0.35))
            border.width: 1
            border.color: root.theme.dividerColor
        }

        contentItem: Column {
            spacing: 2

            Rectangle {
                visible: listView.contentY > 0
                width: parent.width
                height: 20
                radius: root.theme.radiusSmall
                color: "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "⌃"
                    color: root.theme.textMuted
                    font.pixelSize: 10
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: listView.contentY = Math.max(0, listView.contentY - 48)
                }
            }

            ListView {
                id: listView
                width: popup.width - popup.padding * 2
                height: Math.min(contentHeight, 196)
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
                        root.closePopup()
                        event.accepted = true
                    }
                }

                delegate: Rectangle {
                    id: itemDelegate
                    required property int index
                    readonly property bool selected: root.currentIndex === itemDelegate.index
                    readonly property bool highlighted: root.highlightedIndex === itemDelegate.index
                    width: listView.width
                    height: root.size === "sm" ? 24 : 28
                    radius: root.theme.radiusSmall
                    color: mouse.containsMouse || itemDelegate.highlighted ? root.theme.accentWeak : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        Text {
                            Layout.fillWidth: true
                            text: root.itemTextAt(itemDelegate.index)
                            color: mouse.containsMouse || itemDelegate.highlighted ? root.theme.accentForeground : root.theme.textPrimary
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }

                        AppIcon {
                            visible: itemDelegate.selected
                            name: "check"
                            color: mouse.containsMouse || itemDelegate.highlighted ? root.theme.accentForeground : root.theme.accent
                            iconSize: 12
                        }
                    }

                    MouseArea {
                        id: mouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: root.selectIndex(itemDelegate.index)
                    }
                }
            }

            Rectangle {
                visible: listView.contentHeight > listView.height && listView.contentY + listView.height < listView.contentHeight
                width: parent.width
                height: 20
                radius: root.theme.radiusSmall
                color: "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "⌄"
                    color: root.theme.textMuted
                    font.pixelSize: 10
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: listView.contentY = Math.min(listView.contentHeight - listView.height, listView.contentY + 48)
                }
            }
        }
    }
}
