pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {
    id: root

    required property AppTheme theme
    property var headers: []
    property var rows: []
    property var footerRows: []
    property var columnKeys: []
    property string caption: ""
    property bool striped: false
    property int rowHeight: 32
    property int headerHeight: 32
    property string rowVariantKey: "variant"

    function cellText(row, columnIndex) {
        if (row === undefined || row === null)
            return ""

        if (Array.isArray(root.columnKeys) && columnIndex < root.columnKeys.length) {
            const key = root.columnKeys[columnIndex]
            if (typeof row === "object" && row[key] !== undefined && row[key] !== null)
                return String(row[key])
        }

        if (Array.isArray(row) && columnIndex < row.length)
            return String(row[columnIndex])

        return ""
    }

    function rowVariant(row) {
        if (row && typeof row === "object" && row[root.rowVariantKey] !== undefined)
            return String(row[root.rowVariantKey])
        return ""
    }

    function rowFillColor(row, index) {
        const variant = root.rowVariant(row)
        if (variant === "selected")
            return root.theme.accentWeak
        if (variant === "danger")
            return root.theme.dangerWeak
        if (variant === "success")
            return root.theme.okWeak
        if (variant === "warning")
            return root.theme.warnWeak
        if (root.striped && (index % 2 === 1))
            return Qt.rgba(root.theme.surface.r, root.theme.surface.g, root.theme.surface.b, 0.6)
        return "transparent"
    }

    function rowTextColor(row) {
        const variant = root.rowVariant(row)
        if (variant === "danger")
            return root.theme.danger
        if (variant === "success")
            return root.theme.ok
        if (variant === "warning")
            return root.theme.warn
        return root.theme.textSecondary
    }

    implicitWidth: Math.max(320, tableColumn.implicitWidth)
    implicitHeight: tableFrame.implicitHeight

    Rectangle {
        id: tableFrame
        anchors.fill: parent
        radius: root.theme.radiusLarge
        color: root.theme.cardColor
        border.width: 1
        border.color: root.theme.dividerColor
        implicitHeight: tableColumn.implicitHeight + 2

        Column {
            id: tableColumn
            anchors.fill: parent
            anchors.margins: 1
            spacing: 0

            Flickable {
                id: tableScroll
                width: parent.width
                height: headerRect.height + bodyColumn.height
                contentWidth: Math.max(width, headerRow.implicitWidth)
                contentHeight: headerRect.height + bodyColumn.height
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                ScrollBar.horizontal: ScrollBar {
                    policy: tableScroll.contentWidth > tableScroll.width ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
                }

                Rectangle {
                    id: headerRect
                    width: tableScroll.contentWidth
                    height: root.headerHeight
                    color: root.theme.surface

                    Row {
                        id: headerRow
                        height: parent.height
                        spacing: 0

                        Repeater {
                            model: root.headers

                            delegate: Rectangle {
                                id: headerCell
                                required property var modelData
                                width: Math.max(120, headerLabel.implicitWidth + 24)
                                height: root.headerHeight
                                color: "transparent"

                                Text {
                                    id: headerLabel
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left
                                    anchors.leftMargin: 12
                                    text: String(headerCell.modelData)
                                    color: root.theme.textPrimary
                                    font.pixelSize: 11
                                    font.bold: true
                                }
                            }
                        }
                    }
                }

                Column {
                    id: bodyColumn
                    y: headerRect.height
                    width: tableScroll.contentWidth
                    spacing: 0

                    Repeater {
                        model: root.rows

                        delegate: Rectangle {
                            id: rowDelegate
                            required property int index
                            required property var modelData
                            width: bodyColumn.width
                            height: root.rowHeight
                            color: root.rowFillColor(rowDelegate.modelData, rowDelegate.index)
                            border.width: 0

                            Row {
                                anchors.fill: parent
                                spacing: 0

                                Repeater {
                                    model: root.headers.length

                                    delegate: Rectangle {
                                        id: bodyCell
                                        required property int index
                                        width: headerRow.children[index] ? headerRow.children[index].width : 120
                                        height: root.rowHeight
                                        color: "transparent"

                                        Text {
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.left: parent.left
                                            anchors.leftMargin: 12
                                            anchors.right: parent.right
                                            anchors.rightMargin: 12
                                            text: root.cellText(rowDelegate.modelData, bodyCell.index)
                                            color: root.rowTextColor(rowDelegate.modelData)
                                            font.pixelSize: 11
                                            font.bold: root.rowVariant(rowDelegate.modelData) === "selected"
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                height: 1
                                color: root.theme.dividerColor
                                opacity: rowDelegate.index < root.rows.length - 1 ? 1 : 0
                            }
                        }
                    }
                }
            }

            Column {
                visible: root.footerRows.length > 0
                width: parent.width
                spacing: 0

                Rectangle {
                    width: parent.width
                    height: 1
                    color: root.theme.dividerColor
                }

                Repeater {
                    model: root.footerRows

                    delegate: Rectangle {
                        id: footerRowDelegate
                        required property int index
                        required property var modelData
                        width: parent.width
                        height: root.rowHeight
                        color: Qt.rgba(root.theme.muted.r, root.theme.muted.g, root.theme.muted.b, 0.5)

                        Row {
                            anchors.fill: parent
                            spacing: 0

                            Repeater {
                                model: root.headers.length

                                delegate: Rectangle {
                                    id: footerCell
                                    required property int index
                                    width: headerRow.children[index] ? headerRow.children[index].width : 120
                                    height: root.rowHeight
                                    color: "transparent"

                                    Text {
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.left: parent.left
                                        anchors.leftMargin: 12
                                        anchors.right: parent.right
                                        anchors.rightMargin: 12
                                        text: root.cellText(footerRowDelegate.modelData, footerCell.index)
                                        color: root.theme.textPrimary
                                        font.pixelSize: 11
                                        font.bold: true
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Text {
                visible: root.caption.length > 0
                width: parent.width
                leftPadding: 12
                rightPadding: 12
                topPadding: 10
                bottomPadding: 10
                text: root.caption
                color: root.theme.textMuted
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
        }
    }
}
