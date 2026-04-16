pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property bool showOutsideDays: true
    property string selectionMode: "single"
    property date visibleMonth: new Date(new Date().getFullYear(), new Date().getMonth(), 1)
    property date selectedDate: new Date(new Date().getFullYear(), new Date().getMonth(), new Date().getDate())
    property date rangeStart: new Date(0)
    property date rangeEnd: new Date(0)
    property int daySize: 32
    readonly property string monthLabel: Qt.formatDate(root.visibleMonth, "yyyy MMMM")
    readonly property var weekdayLabels: ["Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"]

    function isValidDate(dateValue) {
        return dateValue && dateValue.getFullYear && !isNaN(dateValue.getTime())
    }

    function isSameDate(first, second) {
        return root.isValidDate(first) && root.isValidDate(second)
               && first.getFullYear() === second.getFullYear()
               && first.getMonth() === second.getMonth()
               && first.getDate() === second.getDate()
    }

    function normalizeDate(dateValue) {
        return new Date(dateValue.getFullYear(), dateValue.getMonth(), dateValue.getDate())
    }

    function daysInMonth(year, month) {
        return new Date(year, month + 1, 0).getDate()
    }

    function dayEntries() {
        const year = root.visibleMonth.getFullYear()
        const month = root.visibleMonth.getMonth()
        const firstDay = new Date(year, month, 1)
        const offset = firstDay.getDay()
        const total = root.daysInMonth(year, month)
        const previousMonthDays = root.daysInMonth(year, month - 1)
        const entries = []

        for (let index = 0; index < 42; ++index) {
            let dayNumber
            let entryMonth = month
            let entryYear = year
            let outside = false

            if (index < offset) {
                dayNumber = previousMonthDays - offset + index + 1
                entryMonth = month - 1
                outside = true
            } else if (index >= offset + total) {
                dayNumber = index - offset - total + 1
                entryMonth = month + 1
                outside = true
            } else {
                dayNumber = index - offset + 1
            }

            const value = new Date(entryYear, entryMonth, dayNumber)
            entries.push({
                dateValue: value,
                year: value.getFullYear(),
                month: value.getMonth(),
                day: value.getDate(),
                outside: outside
            })
        }

        return entries
    }

    function isInRange(dateValue) {
        if (!root.isValidDate(root.rangeStart) || !root.isValidDate(root.rangeEnd))
            return false
        const start = root.normalizeDate(root.rangeStart).getTime()
        const end = root.normalizeDate(root.rangeEnd).getTime()
        const current = root.normalizeDate(dateValue).getTime()
        return current >= Math.min(start, end) && current <= Math.max(start, end)
    }

    function previousMonth() {
        root.visibleMonth = new Date(root.visibleMonth.getFullYear(), root.visibleMonth.getMonth() - 1, 1)
        return true
    }

    function nextMonth() {
        root.visibleMonth = new Date(root.visibleMonth.getFullYear(), root.visibleMonth.getMonth() + 1, 1)
        return true
    }

    function selectDate(dateValue) {
        if (!root.isValidDate(dateValue))
            return false

        const normalized = root.normalizeDate(dateValue)
        root.visibleMonth = new Date(normalized.getFullYear(), normalized.getMonth(), 1)

        if (root.selectionMode === "range") {
            if (!root.isValidDate(root.rangeStart) || (root.isValidDate(root.rangeStart) && root.isValidDate(root.rangeEnd))) {
                root.rangeStart = normalized
                root.rangeEnd = new Date(0)
            } else {
                if (normalized.getTime() < root.rangeStart.getTime()) {
                    root.rangeEnd = root.rangeStart
                    root.rangeStart = normalized
                } else {
                    root.rangeEnd = normalized
                }
            }
        } else {
            root.selectedDate = normalized
        }

        return true
    }

    implicitWidth: dayGrid.width + calendarColumn.anchors.margins * 2
    implicitHeight: calendarColumn.implicitHeight + 2

    Rectangle {
        anchors.fill: parent
        radius: root.theme.radiusLarge
        color: root.theme.cardColor
        border.width: 1
        border.color: root.theme.dividerColor
    }

    Column {
        id: calendarColumn
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            width: dayGrid.implicitWidth
            anchors.horizontalCenter: parent.horizontalCenter

            AppButton {
                theme: root.theme
                size: "icon"
                variant: "ghost"
                iconName: "chevron-left"
                text: ""
                onClicked: root.previousMonth()
            }

            Item { Layout.fillWidth: true }

            Text {
                text: root.monthLabel
                color: root.theme.textPrimary
                font.pixelSize: 12
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            AppButton {
                theme: root.theme
                size: "icon"
                variant: "ghost"
                iconName: "chevron-right"
                text: ""
                onClicked: root.nextMonth()
            }
        }

        Grid {
            id: dayGrid
            anchors.horizontalCenter: parent.horizontalCenter
            columns: 7
            rowSpacing: 4
            columnSpacing: 4
            width: (root.daySize * 7) + (columnSpacing * 6)

            Repeater {
                model: root.weekdayLabels

                delegate: Text {
                    required property var modelData
                    width: root.daySize
                    horizontalAlignment: Text.AlignHCenter
                    text: String(modelData)
                    color: root.theme.textMuted
                    font.pixelSize: 11
                }
            }

            Repeater {
                model: root.dayEntries()

                delegate: Rectangle {
                    id: dayDelegate
                    required property var modelData
                    readonly property bool isToday: root.isSameDate(new Date(), dayDelegate.modelData.dateValue)
                    readonly property bool isSelected: root.selectionMode === "single" && root.isSameDate(root.selectedDate, dayDelegate.modelData.dateValue)
                    readonly property bool isRangeStart: root.selectionMode === "range" && root.isSameDate(root.rangeStart, dayDelegate.modelData.dateValue)
                    readonly property bool isRangeEnd: root.selectionMode === "range" && root.isSameDate(root.rangeEnd, dayDelegate.modelData.dateValue)
                    readonly property bool isRangeMiddle: root.selectionMode === "range"
                                                         && root.isInRange(dayDelegate.modelData.dateValue)
                                                         && !isRangeStart && !isRangeEnd
                    readonly property bool filledState: isSelected || isRangeStart || isRangeEnd || isRangeMiddle
                    width: root.daySize
                    height: root.daySize
                    radius: root.theme.radiusSmall
                    color: isSelected || isRangeStart || isRangeEnd
                           ? root.theme.accent
                           : isRangeMiddle
                             ? root.theme.accentWeak
                             : "transparent"
                    border.width: isToday && !isSelected && !isRangeStart && !isRangeEnd ? 1 : 0
                    border.color: isToday ? Qt.rgba(root.theme.textMuted.r, root.theme.textMuted.g, root.theme.textMuted.b, 0.65) : "transparent"
                    opacity: dayDelegate.modelData.outside && !root.showOutsideDays ? 0 : 1

                    Text {
                        anchors.centerIn: parent
                        text: dayDelegate.modelData.day
                        color: !dayDelegate.filledState
                               ? (dayDelegate.modelData.outside ? Qt.rgba(root.theme.textMuted.r, root.theme.textMuted.g, root.theme.textMuted.b, 0.65) : root.theme.textPrimary)
                               : (dayDelegate.isRangeMiddle ? root.theme.textPrimary : root.theme.primaryForeground)
                        font.pixelSize: 11
                        font.bold: dayDelegate.isSelected || dayDelegate.isRangeStart || dayDelegate.isRangeEnd
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: !dayDelegate.modelData.outside || root.showOutsideDays
                        onClicked: root.selectDate(dayDelegate.modelData.dateValue)
                    }
                }
            }
        }
    }
}
