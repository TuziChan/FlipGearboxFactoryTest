pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels

Column {
    id: root

    required property AppTheme theme
    property string label: ""
    property var model: []
    property int currentIndex: -1
    property string currentText: ""
    property string query: ""
    property int highlightedIndex: -1
    property string popupObjectName: ""
    signal activated(int index, string text)

    function openPopup() {
        innerModel.rows = root.normalizeRows()
        filteredModel.invalidate()
        root.highlightedIndex = filteredModel.count > 0 ? 0 : -1
        popup.open()
        listView.forceActiveFocus()
        return true
    }

    function closePopup() {
        popup.close()
        return true
    }

    function moveHighlight(delta) {
        if (filteredModel.count === 0)
            return false
        root.highlightedIndex = Math.max(0, Math.min(filteredModel.count - 1, root.highlightedIndex + delta))
        listView.positionViewAtIndex(root.highlightedIndex, ListView.Contain)
        return true
    }

    function confirmHighlighted() {
        if (root.highlightedIndex < 0 || root.highlightedIndex >= filteredModel.count)
            return false
        var item = filteredModel.get(root.highlightedIndex)
        root.currentIndex = item.sourceIndex
        root.currentText = item.display
        popup.close()
        root.activated(root.currentIndex, root.currentText)
        return true
    }

    function confirmIndex(proxyIndex) {
        var item = filteredModel.get(proxyIndex)
        root.currentIndex = item.sourceIndex
        root.currentText = item.display
        popup.close()
        root.activated(root.currentIndex, root.currentText)
        return true
    }

    function normalizeRows() {
        var rows = []
        for (var i = 0; i < root.model.length; ++i) {
            var item = root.model[i]
            var text = typeof item === "string" ? item : (item.text || item.label || item.title || "")
            rows.push({ display: text, sourceIndex: i })
        }
        return rows
    }

    spacing: 6

    AppLabel {
        visible: root.label.length > 0
        theme: root.theme
        muted: true
        text: root.label
    }

    Rectangle {
        implicitWidth: 200
        implicitHeight: 36
        radius: root.theme.radiusSmall
        color: Qt.tint(root.theme.cardColor, Qt.rgba(root.theme.stroke.r, root.theme.stroke.g, root.theme.stroke.b, 0.18))
        border.width: 1
        border.color: popup.visible ? root.theme.accent : root.theme.stroke

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            Text {
                Layout.fillWidth: true
                text: root.currentText.length > 0 ? root.currentText : "请选择..."
                color: root.currentText.length > 0 ? root.theme.textPrimary : root.theme.textMuted
                font.pixelSize: 14
                elide: Text.ElideRight
            }

            AppButton {
                theme: root.theme
                variant: "ghost"
                size: "icon-xs"
                iconName: popup.visible ? "chevron-up" : "chevron-down"
                text: ""
                onClicked: popup.visible ? root.closePopup() : root.openPopup()
            }
        }

        TapHandler {
            onTapped: popup.visible ? root.closePopup() : root.openPopup()
        }
    }

    Popup {
        id: popup
        objectName: root.popupObjectName
        y: 32
        width: 220
        padding: 4
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        background: Rectangle {
            radius: root.theme.radiusMedium
            color: root.theme.cardColor
            border.width: 1
            border.color: root.theme.dividerColor
        }

        contentItem: Column {
            spacing: 4

            Rectangle {
                height: 32
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
                        id: queryField
                        Layout.fillWidth: true
                        background: null
                        placeholderText: "搜索..."
                        color: root.theme.textPrimary
                        placeholderTextColor: root.theme.textMuted
                        onTextChanged: {
                            root.query = text
                            filteredModel.invalidate()
                            root.highlightedIndex = filteredModel.count > 0 ? 0 : -1
                        }
                    }
                }
            }

            ListView {
                id: listView
                width: popup.width - popup.padding * 2
                height: Math.min(contentHeight, 196)
                model: filteredModel
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
                        root.closePopup()
                        event.accepted = true
                    }
                }

                delegate: Rectangle {
                    id: itemDelegate
                    required property string display
                    required property int sourceIndex
                    required property int index
                    width: listView.width
                    height: 32
                    radius: root.theme.radiusSmall
                    color: root.highlightedIndex === index ? root.theme.accentWeak : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 8

                        Text {
                            Layout.fillWidth: true
                            text: itemDelegate.display
                            color: root.highlightedIndex === index ? root.theme.accentForeground : root.theme.textPrimary
                            font.pixelSize: 14
                            elide: Text.ElideRight
                        }

                        AppIcon {
                            visible: root.currentIndex === itemDelegate.sourceIndex
                            name: "check"
                            color: root.highlightedIndex === index ? root.theme.accentForeground : root.theme.accent
                            iconSize: 16
                        }
                    }

                    TapHandler {
                        onTapped: {
                            root.highlightedIndex = itemDelegate.index
                            root.confirmIndex(itemDelegate.index)
                        }
                    }

                    HoverHandler {
                        onHoveredChanged: if (hovered) root.highlightedIndex = itemDelegate.index
                    }
                }
            }
        }

        onOpened: queryField.forceActiveFocus()
    }

    TableModel {
        id: innerModel
        TableModelColumn { display: "display" }
        TableModelColumn { display: "sourceIndex" }
        rows: []
    }

    SortFilterProxyModel {
        id: filteredModel
        model: innerModel
        filters: [
            FunctionFilter {
                function filter(data): bool {
                    var q = root.query.trim()
                    if (q.length === 0)
                        return true
                    var display = data.display
                    return display !== undefined && display.indexOf(q) !== -1
                }
            }
        ]
    }
}
