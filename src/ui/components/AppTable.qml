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
    property var columnWidthHints: []

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

    function columnWidth(colIndex) {
        const totalCols = root.headers.length
        if (totalCols === 0)
            return 120

        const availableWidth = tableScroll.width
        const hint = root.columnWidthHints.length > colIndex
                     ? root.columnWidthHints[colIndex] : null

        if (hint) {
            if (hint.mode === "flex")
                return root._flexWidth
            if (typeof hint.minWidth === "number")
                return Math.max(hint.minWidth, hint.minWidth)
        }

        if (availableWidth <= 0)
            return 120

        var fixedTotal = 0
        var flexCount = 0
        for (var i = 0; i < totalCols; i++) {
            var h = root.columnWidthHints.length > i ? root.columnWidthHints[i] : null
            if (h && h.mode === "flex") {
                flexCount++
            } else if (h && typeof h.minWidth === "number") {
                fixedTotal += h.minWidth
            } else {
                fixedTotal += 120
            }
        }

        if (hint && hint.mode === "flex") {
            return flexCount > 0 ? Math.max(120, (availableWidth - fixedTotal) / flexCount) : 120
        }

        return Math.max(120, hint ? hint.minWidth : 120)
    }

    property real _flexWidth: {
        const totalCols = root.headers.length
        if (totalCols === 0) return 120
        const availableWidth = tableScroll.width
        if (availableWidth <= 0) return 120
        var fixedTotal = 0
        var flexCount = 0
        for (var i = 0; i < totalCols; i++) {
            var h = root.columnWidthHints.length > i ? root.columnWidthHints[i] : null
            if (h && h.mode === "flex") {
                flexCount++
            } else if (h && typeof h.minWidth === "number") {
                fixedTotal += h.minWidth
            } else {
                fixedTotal += 120
            }
        }
        return flexCount > 0 ? Math.max(120, (availableWidth - fixedTotal) / flexCount) : 120
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
                                required property int index
                                width: root.columnWidth(headerCell.index)
                                height: root.headerHeight
                                color: "transparent"

                                Text {
                                    id: headerLabel
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: parent.left
                                    anchors.leftMargin: 12
                                    anchors.right: parent.right
                                    anchors.rightMargin: 12
                                    text: String(headerCell.modelData)
                                    color: root.theme.textPrimary
                                    font.pixelSize: 11
                                    font.bold: true
                                    elide: Text.ElideRight
                                    clip: true
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
                            height: {
                                var hasLongText = false
                                for (var ci = 0; ci < root.headers.length; ci++) {
                                    if (root.cellText(rowDelegate.modelData, ci).length > 40) {
                                        hasLongText = true
                                        break
                                    }
                                }
                                return hasLongText ? root.rowHeight * 2 : root.rowHeight
                            }
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
                                        width: {
                                            var w = 120
                                            if (bodyCell.index < headerRow.children.length) {
                                                var hdr = headerRow.children[bodyCell.index]
                                                if (hdr) w = hdr.width
                                            }
                                            w
                                        }
                                        height: bodyCell.parent ? bodyCell.parent.height : root.rowHeight
                                        color: "transparent"

                                        Text {
                                            id: bodyCellText
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
                                            wrapMode: Text.NoWrap
                                            clip: true
                                        }

                                        MouseArea {
                                            id: bodyCellHover
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            acceptedButtons: Qt.NoButton
                                        }

                                        ToolTip.delay: 400
                                        ToolTip.visible: bodyCellHover.containsMouse
                                                         && bodyCellText.text.length > 0
                                                         && (bodyCellText.paintedWidth > bodyCellText.width
                                                             || bodyCellText.paintedHeight > bodyCellText.height)
                                        ToolTip.text: bodyCellText.text
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
                                    width: headerRow.children[footerCell.index]
                                           ? headerRow.children[footerCell.index].width
                                           : 120
                                    height: root.rowHeight
                                    color: "transparent"

                                    Text {
                                        id: footerCellText
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
                                        clip: true
                                    }

                                    MouseArea {
                                        id: footerCellHover
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        acceptedButtons: Qt.NoButton
                                    }

                                    ToolTip.delay: 400
                                    ToolTip.visible: footerCellHover.containsMouse
                                                     && footerCellText.text.length > 0
                                                     && (footerCellText.paintedWidth > footerCellText.width
                                                         || footerCellText.paintedHeight > footerCellText.height)
                                    ToolTip.text: footerCellText.text
                                }
                            }
                        }
                    }
                }
            }

            Text {
                visible: root.caption.length > 0
                width: parent.width - 24
                x: 12
                padding: 8
                text: root.caption
                color: root.theme.textMuted
                font.pixelSize: 11
                font.italic: true
            }
        }
    }
}
