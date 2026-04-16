pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: root

    required property AppTheme theme
    property bool showOutsideDays: true
    property string selectionMode: "single"
    property date visibleMonth: new Date(new Date().getFullYear(), new Date().getMonth(), 1)
    property date selectedDate: new Date(new Date().getFullYear(), new Date().getMonth(), new Date().getDate())
    property date rangeStart: new Date(0)
    property date rangeEnd: new Date(0)
    property var disabledDates: []
    property bool showWeekNumbers: false
    property string captionLayout: "label"
    property string buttonVariant: "ghost"
    property int cellSize: 32
    property int cellRadius: root.theme.radiusSmall
    readonly property string monthLabel: Qt.formatDate(root.visibleMonth, "MMMM yyyy")
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

    function isDateDisabled(dateValue) {
        for (let i = 0; i < root.disabledDates.length; i++) {
            if (root.isSameDate(root.disabledDates[i], dateValue))
                return true
        }
        return false
    }

    implicitWidth: dayGrid.width + calendarColumn.anchors.margins * 2
    implicitHeight: calendarColumn.implicitHeight + 2

    Column {
        id: calendarColumn
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        RowLayout {
            width: dayGrid.implicitWidth
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0

            AppButton {
                theme: root.theme
                size: "icon"
                variant: root.buttonVariant
                iconName: "chevron-left"
                text: ""
                Layout.preferredWidth: root.cellSize
                Layout.preferredHeight: root.cellSize
                onClicked: root.previousMonth()
            }

            Item { Layout.fillWidth: true }

            Text {
                text: root.monthLabel
                color: root.theme.textPrimary
                font.pixelSize: 14
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
            }

            Item { Layout.fillWidth: true }

            AppButton {
                theme: root.theme
                size: "icon"
                variant: root.buttonVariant
                iconName: "chevron-right"
                text: ""
                Layout.preferredWidth: root.cellSize
                Layout.preferredHeight: root.cellSize
                onClicked: root.nextMonth()
            }
        }

        Grid {
            id: dayGrid
            anchors.horizontalCenter: parent.horizontalCenter
            columns: 7
            rowSpacing: 8
            columnSpacing: 0
            width: root.cellSize * 7

            Repeater {
                model: root.weekdayLabels

                delegate: Text {
                    required property var modelData
                    width: root.cellSize
                    height: root.cellSize * 0.8
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: String(modelData)
                    color: root.theme.textMuted
                    font.pixelSize: 13
                    font.weight: Font.Normal
                }
            }

            Repeater {
                model: root.dayEntries()

                delegate: Item {
                    id: dayDelegate
                    required property var modelData
                    readonly property bool isToday: root.isSameDate(new Date(), dayDelegate.modelData.dateValue)
                    readonly property bool isSelected: root.selectionMode === "single" && root.isSameDate(root.selectedDate, dayDelegate.modelData.dateValue)
                    readonly property bool isRangeStart: root.selectionMode === "range" && root.isSameDate(root.rangeStart, dayDelegate.modelData.dateValue)
                    readonly property bool isRangeEnd: root.selectionMode === "range" && root.isSameDate(root.rangeEnd, dayDelegate.modelData.dateValue)
                    readonly property bool isRangeMiddle: root.selectionMode === "range"
                                                         && root.isInRange(dayDelegate.modelData.dateValue)
                                                         && !isRangeStart && !isRangeEnd
                    readonly property bool isDisabled: root.isDateDisabled(dayDelegate.modelData.dateValue)
                    width: root.cellSize
                    height: root.cellSize

                    Rectangle {
                        id: rangeBackground
                        anchors.fill: parent
                        visible: dayDelegate.isRangeMiddle || dayDelegate.isRangeStart || dayDelegate.isRangeEnd
                        color: root.theme.mutedColor
                        radius: 0
                    }

                    Rectangle {
                        id: dayButton
                        anchors.centerIn: parent
                        width: root.cellSize
                        height: root.cellSize
                        radius: root.cellRadius
                        color: {
                            if (dayDelegate.isDisabled)
                                return "transparent"
                            if (dayDelegate.isSelected || dayDelegate.isRangeStart || dayDelegate.isRangeEnd)
                                return root.theme.accent
                            if (dayDelegate.isToday)
                                return root.theme.mutedColor
                            if (dayButtonMouseArea.containsMouse)
                                return root.theme.accentWeak
                            return "transparent"
                        }
                        border.width: dayDelegate.isToday && !dayDelegate.isSelected && !dayDelegate.isRangeStart && !dayDelegate.isRangeEnd ? 1 : 0
                        border.color: root.theme.borderColor
                        opacity: {
                            if (dayDelegate.isDisabled)
                                return 0.5
                            if (dayDelegate.modelData.outside && !root.showOutsideDays)
                                return 0
                            return 1
                        }

                        Text {
                            anchors.centerIn: parent
                            text: dayDelegate.modelData.day
                            color: {
                                if (dayDelegate.isDisabled)
                                    return root.theme.textMuted
                                if (dayDelegate.isSelected || dayDelegate.isRangeStart || dayDelegate.isRangeEnd)
                                    return root.theme.primaryForeground
                                if (dayDelegate.modelData.outside)
                                    return root.theme.textMuted
                                return root.theme.textPrimary
                            }
                            font.pixelSize: 14
                            font.weight: Font.Normal
                        }

                        MouseArea {
                            id: dayButtonMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            enabled: !dayDelegate.isDisabled && (!dayDelegate.modelData.outside || root.showOutsideDays)
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: root.selectDate(dayDelegate.modelData.dateValue)
                        }
                    }
                }
            }
        }
    }
}
