pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property bool open: false
    property string title: "Command Palette"
    property string description: "Search for a command to run..."
    property var model: []
    property string query: ""
    property int highlightedIndex: -1
    readonly property int filteredCount: filteredItems.length
    property var filteredItems: []
    property string selectedText: ""
    signal triggered(int index, string text)

    function normalizeModel() {
        const values = []
        for (let i = 0; i < root.model.length; ++i) {
            const item = root.model[i]
            values.push(typeof item === "string" ? { index: i, text: item } : { index: i, text: item.text || item.label || item.title || "", shortcut: item.shortcut || "" })
        }
        return values
    }

    function refreshFilter() {
        const all = normalizeModel()
        const q = root.query.trim()
        root.filteredItems = q.length === 0 ? all : all.filter(item => item.text.indexOf(q) !== -1)
        root.highlightedIndex = root.filteredItems.length > 0 ? 0 : -1
    }

    function openCommand() {
        root.open = true
        dialog.openDialog()
        refreshFilter()
        return true
    }

    function closeCommand() {
        root.open = false
        dialog.closeDialog()
        return true
    }

    function setQuery(text) {
        root.query = text
        refreshFilter()
        return true
    }

    function moveHighlight(delta) {
        if (root.filteredItems.length === 0)
            return false
        root.highlightedIndex = Math.max(0, Math.min(root.filteredItems.length - 1, root.highlightedIndex + delta))
        listView.positionViewAtIndex(root.highlightedIndex, ListView.Contain)
        return true
    }

    function confirmHighlighted() {
        if (root.highlightedIndex < 0 || root.highlightedIndex >= root.filteredItems.length)
            return false
        const chosen = root.filteredItems[root.highlightedIndex]
        root.selectedText = chosen.text
        root.triggered(chosen.index, chosen.text)
        return closeCommand()
    }

    AppDialog {
        id: dialog
        anchors.fill: parent
        theme: root.theme
        objectName: root.objectName.length > 0 ? root.objectName + "Dialog" : ""
        title: root.title
        description: root.description
        showCloseButton: true

        Rectangle {
            height: 36
            radius: root.theme.radiusSmall
            color: Qt.tint(root.theme.cardColor, Qt.rgba(root.theme.stroke.r, root.theme.stroke.g, root.theme.stroke.b, 0.18))
            border.width: 1
            border.color: root.theme.stroke

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 8

                AppIcon {
                    name: "search"
                    color: root.theme.textMuted
                    iconSize: 12
                }

                TextField {
                    id: commandInput
                    Layout.fillWidth: true
                    background: null
                    text: root.query
                    placeholderText: "搜索命令..."
                    color: root.theme.textPrimary
                    placeholderTextColor: root.theme.textMuted
                    onTextChanged: root.setQuery(text)
                }
            }
        }

        ListView {
            id: listView
            width: parent.width
            height: Math.min(contentHeight, 260)
            model: root.filteredItems
            clip: true
            spacing: 2

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
                    root.closeCommand()
                    event.accepted = true
                }
            }

            delegate: Rectangle {
                required property var modelData
                required property int index
                width: listView.width
                height: 30
                radius: root.theme.radiusSmall
                color: root.highlightedIndex === index ? root.theme.accentWeak : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    Text {
                        Layout.fillWidth: true
                        text: modelData.text
                        color: root.highlightedIndex === index ? root.theme.accentForeground : root.theme.textPrimary
                        font.pixelSize: 12
                    }

                    Text {
                        visible: modelData.shortcut.length > 0
                        text: modelData.shortcut
                        color: root.highlightedIndex === index ? root.theme.accentForeground : root.theme.textMuted
                        font.pixelSize: 10
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: root.highlightedIndex = index
                    onClicked: {
                        root.highlightedIndex = index
                        root.confirmHighlighted()
                    }
                }
            }
        }

        onVisibleChanged: {
            if (visible)
                commandInput.forceActiveFocus()
        }
    }
}
