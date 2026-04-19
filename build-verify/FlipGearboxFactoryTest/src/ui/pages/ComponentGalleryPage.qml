pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components" as Components

Item {
    id: root
    objectName: "componentGalleryPage"

    required property Components.AppTheme theme

    property string sampleDropdownSelectedText: sampleDropdownMenu.selectedText
    property bool sampleTooltipOpen: sampleTooltip.open
    property int sampleTabsCurrentIndex: sampleTabsDefault.currentIndex
    property string sampleTabsCurrentText: sampleTabsDefault.currentText
    property bool sampleSheetOpen: sampleSheet.open
    property bool samplePopoverOpen: samplePopover.open
    property string sampleComboboxCurrentText: sampleCombobox.currentText
    property int sampleComboboxFilteredCount: sampleCombobox.filteredCount
    property bool sampleCommandOpen: sampleCommand.open
    property int sampleCommandFilteredCount: sampleCommand.filteredCount
    property string sampleCommandSelectedText: sampleCommand.selectedText
    property bool sampleCheckboxChecked: sampleCheckbox.checked
    property string sampleRadioValue: sampleRadioGroup.value
    property bool sampleToggleChecked: sampleToggle.checked
    property string sampleToggleGroupValue: sampleToggleGroup.value === undefined || sampleToggleGroup.value === null
                                      ? ""
                                      : String(sampleToggleGroup.value)
    property bool sampleAlertDialogOpen: sampleAlertDialog.open
    property string sampleMenubarSelectedText: sampleMenubar.selectedText
    property string sampleContextMenuSelectedText: sampleContextMenu.selectedText
    property bool sampleCollapsibleOpen: sampleCollapsible.open
    property int sampleAccordionExpandedCount: sampleAccordion.expandedValues.length
    property string sampleAccordionCurrentText: sampleAccordion.expandedValues.length > 0
                                         ? sampleAccordion.itemContentAt(sampleAccordion.expandedIndex(0))
                                         : ""
    property string sampleButtonGroupLastValue: ""
    property string sampleBreadcrumbLastValue: ""
    property int sampleSliderValue: Math.round(sampleSlider.value)
    property int samplePaginationPage: samplePagination.currentPage
    property string sampleNativeSelectValue: sampleNativeSelect.currentValue
    property string sampleInputOtpValue: sampleInputOtp.value
    property string sampleNavigationMenuSelectedText: sampleNavigationMenu.selectedText
    property bool sampleHoverCardOpen: sampleHoverCard.open
    property bool sampleDrawerOpen: sampleDrawer.open
    property bool sampleScrollAtEnd: sampleScrollArea.atVerticalEnd
    property string sampleCalendarMonthLabel: sampleCalendar.monthLabel
    property int sampleCarouselIndex: sampleCarousel.currentIndex
    property int sampleResizableRatio: Math.round(sampleResizable.ratio)
    property var sampleRangeSliderValues: [15, 85]
    property string sampleRangeSliderValuesText: sampleRangeSliderValues.length >= 2
                                           ? String(Math.round(sampleRangeSliderValues[0])) + "-" + String(Math.round(sampleRangeSliderValues[1]))
                                           : ""
    property string sampleTableFooterText: "总计 " + sampleEnhancedTable.rows.length + " 条"

    function openSampleDialog() {
        return sampleDialog.openDialog()
    }

    function openSampleDropdownMenu() {
        return sampleDropdownMenu.openMenu()
    }

    function selectSampleDropdownIndex(index) {
        return sampleDropdownMenu.selectIndex(index)
    }

    function showSampleTooltip() {
        return sampleTooltip.showTooltip()
    }

    function hideSampleTooltip() {
        return sampleTooltip.hideTooltip()
    }

    function setSampleTabsIndex(index) {
        return sampleTabsDefault.setCurrentIndex(index)
    }

    function openSampleSheet() {
        return sampleSheet.openSheet()
    }

    function closeSampleSheet() {
        return sampleSheet.closeSheet()
    }

    function openSamplePopover() {
        return samplePopover.openPopover()
    }

    function closeSamplePopover() {
        return samplePopover.closePopover()
    }

    function openSampleCombobox() {
        return sampleCombobox.openPopup()
    }

    function setSampleComboboxQuery(query) {
        return sampleCombobox.setQuery(query)
    }

    function confirmSampleCombobox() {
        return sampleCombobox.confirmHighlighted()
    }

    function openSampleCommand() {
        return sampleCommand.openCommand()
    }

    function setSampleCommandQuery(query) {
        return sampleCommand.setQuery(query)
    }

    function confirmSampleCommand() {
        return sampleCommand.confirmHighlighted()
    }

    function toggleSampleCheckbox() {
        return sampleCheckbox.toggle()
    }

    function selectSampleRadioValue(value) {
        return sampleRadioGroup.selectValue(value)
    }

    function toggleSampleToggle() {
        sampleToggle.activate()
        return true
    }

    function selectSampleToggleGroupValue(value) {
        return sampleToggleGroup.selectValue(value)
    }

    function openSampleAlertDialog() {
        return sampleAlertDialog.openDialog()
    }

    function confirmSampleAlertDialog() {
        return sampleAlertDialog.acceptDialog()
    }

    function openSampleMenubar(index) {
        return sampleMenubar.openMenu(index)
    }

    function selectSampleMenubarItem(index) {
        return sampleMenubar.selectIndex(index)
    }

    function openSampleContextMenu() {
        return sampleContextMenu.openAt(24, 24)
    }

    function selectSampleContextMenuItem(index) {
        return sampleContextMenu.selectIndex(index)
    }

    function toggleSampleCollapsible() {
        return sampleCollapsible.toggle()
    }

    function toggleSampleAccordion(index) {
        return sampleAccordion.toggleIndex(index)
    }

    function setSampleSliderValue(value) {
        return sampleSlider.setValue(value)
    }

    function goToSamplePaginationPage(page) {
        return samplePagination.setPage(page)
    }

    function selectSampleNativeSelectIndex(index) {
        return sampleNativeSelect.selectIndex(index)
    }

    function setSampleInputOtpValue(value) {
        return sampleInputOtp.setValue(value)
    }

    function openSampleNavigationMenu(index) {
        return sampleNavigationMenu.openMenu(index)
    }

    function selectSampleNavigationMenuItem(index) {
        return sampleNavigationMenu.selectItem(index)
    }

    function showSampleHoverCard() {
        return sampleHoverCard.showCard()
    }

    function hideSampleHoverCard() {
        return sampleHoverCard.hideCard()
    }

    function openSampleDrawer() {
        return sampleDrawer.openDrawer()
    }

    function closeSampleDrawer() {
        return sampleDrawer.closeDrawer()
    }

    function scrollSampleScrollAreaToEnd() {
        return sampleScrollArea.scrollToEnd()
    }

    function goToNextSampleCalendarMonth() {
        return sampleCalendar.nextMonth()
    }

    function goToNextSampleCarousel() {
        return sampleCarousel.next()
    }

    function setSampleResizableRatio(value) {
        return sampleResizable.setRatio(value)
    }

    function setSampleRangeSliderValues(values) {
        const normalized = Array.isArray(values)
                           ? values.slice()
                           : (values !== undefined && values !== null && values.length !== undefined
                              ? [values[0], values[1]]
                              : [])
        sampleRangeSlider.setValues(normalized)
        sampleRangeSliderValues = normalized
        return true
    }

    Component.onCompleted: {
        sampleRangeSliderValues = sampleRangeSlider.values
    }

    Rectangle {
        anchors.fill: parent
        color: root.theme.bgColor
    }

    ScrollView {
        id: flickable
        anchors.fill: parent
        anchors.margins: 18
        contentWidth: availableWidth
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        Column {
            id: contentColumn
            width: flickable.availableWidth
            spacing: 24

            Components.SectionCard {
                objectName: "galleryIntroCard"
                width: parent.width
                theme: root.theme
                title: "组件库"
                subtitle: "独立承载 shadcn 风格组件示例、状态回显与交互冒烟，避免把样例逻辑继续塞进测试执行业务页。"

                RowLayout {
                    width: parent.width
                    spacing: 8

                    Components.AppBadge {
                        theme: root.theme
                        text: "shadcn/ui parity"
                        variant: "secondary"
                    }

                    Components.AppBadge {
                        theme: root.theme
                        text: root.theme.themeName + " / " + root.theme.styleName
                        variant: "outline"
                    }

                    Components.AppBadge {
                        theme: root.theme
                        text: root.theme.darkMode ? "Dark" : "Light"
                        variant: root.theme.darkMode ? "default" : "secondary"
                    }
                }

                Text {
                    width: parent.width
                    color: root.theme.textMuted
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    text: "当前页先覆盖 Dialog、DropdownMenu、Tooltip、Tabs、Sheet、Popover、Combobox、Command。后续新增组件时，优先继续放到这个 Gallery，而不是回流到业务页隐藏实例。"
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "选择类组件"
                subtitle: "下拉、组合框与命令选择都保留可编程接口，供 QML 冒烟测试直接调用。"

                RowLayout {
                    width: parent.width
                    spacing: 16

                    Column {
                        Layout.preferredWidth: Math.max(260, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Dropdown Menu"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Components.AppDropdownMenu {
                            id: sampleDropdownMenu
                            objectName: "sampleDropdownMenu"
                            theme: root.theme
                            triggerText: "打开操作菜单"
                            model: ["切换到标准视图", "切换到紧凑视图", "打开检查器"]
                        }

                        Text {
                            width: parent.width
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                            text: root.sampleDropdownSelectedText.length > 0
                                  ? "当前选择: " + root.sampleDropdownSelectedText
                                  : "尚未选择菜单项"
                        }
                    }

                    Column {
                        Layout.preferredWidth: Math.max(260, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Combobox"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Components.AppCombobox {
                            id: sampleCombobox
                            objectName: "sampleCombobox"
                            theme: root.theme
                            label: "测试步骤"
                            model: ["准备/找零", "空载正反转", "角度定位", "负载上升", "回零结束"]
                        }

                        Text {
                            width: parent.width
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                            text: root.sampleComboboxCurrentText.length > 0
                                  ? "当前步骤: " + root.sampleComboboxCurrentText + "，过滤结果 " + root.sampleComboboxFilteredCount + " 项"
                                  : "尚未选择步骤"
                        }
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "导航与提示"
                subtitle: "可见 Tabs 与显式显示/隐藏的 Tooltip 示例。"

                RowLayout {
                    width: parent.width
                    spacing: 16

                    Column {
                        Layout.preferredWidth: Math.max(260, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Tabs / Default"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Components.AppTabs {
                            id: sampleTabsDefault
                            objectName: "sampleTabs"
                            width: Math.min(parent.width, 320)
                            theme: root.theme
                            model: [
                                { text: "概述", content: "概览页展示总体信息与默认摘要。" },
                                { text: "分析", content: "分析页用于查看趋势、来源与诊断要点。" },
                                { text: "报告", content: "报告页聚焦导出、分享与归档入口。" },
                                { text: "背景设定", content: "管理您的账户偏好和选项。根据您的需求定制您的体验。" }
                            ]
                            variant: "default"
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: "当前 Tab 索引: " + root.sampleTabsCurrentIndex
                        }

                        Text {
                            width: parent.width
                            color: root.theme.textSecondary
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                            text: root.sampleTabsCurrentText
                        }
                    }

                    Column {
                        Layout.preferredWidth: Math.max(260, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Tabs / Line"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Components.AppTabs {
                            id: sampleTabsLine
                            objectName: "sampleTabsLine"
                            width: Math.min(parent.width, 320)
                            theme: root.theme
                            model: [
                                { text: "概览", content: "概览内容" },
                                { text: "明细", content: "明细内容" },
                                { text: "日志", content: "日志内容" }
                            ]
                            variant: "line"
                        }

                        Text {
                            width: parent.width
                            color: root.theme.textSecondary
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                            text: sampleTabsLine.currentText
                        }

                        Text {
                            text: "Tooltip"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Item {
                            width: parent.width
                            height: 72

                            Rectangle {
                                width: 136
                                height: 32
                                anchors.verticalCenter: parent.verticalCenter
                                radius: root.theme.radiusMedium
                                color: root.theme.surface
                                border.width: 1
                                border.color: root.theme.stroke

                                Text {
                                    anchors.centerIn: parent
                                    text: "Tooltip Anchor"
                                    color: root.theme.textPrimary
                                    font.pixelSize: 12
                                }
                            }

                            Components.AppTooltip {
                                id: sampleTooltip
                                objectName: "sampleTooltip"
                                x: 18
                                y: 36
                                theme: root.theme
                                text: "这是示例 Tooltip"
                            }
                        }

                        RowLayout {
                            spacing: 8

                            Components.AppButton {
                                theme: root.theme
                                text: "显示"
                                size: "sm"
                                onClicked: root.showSampleTooltip()
                            }

                            Components.AppButton {
                                theme: root.theme
                                text: "隐藏"
                                size: "sm"
                                variant: "outline"
                                onClicked: root.hideSampleTooltip()
                            }
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: root.sampleTooltipOpen ? "Tooltip 已显示" : "Tooltip 已隐藏"
                        }
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "选择与切换"
                subtitle: "checkbox、radio-group、toggle、toggle-group 的首批 QML 复刻样例。"

                RowLayout {
                    width: parent.width
                    spacing: 16

                    Column {
                        Layout.preferredWidth: Math.max(260, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Checkbox / Radio Group"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Column {
                            spacing: 8

                            Row {
                                spacing: 8

                                Components.AppCheckbox {
                                    id: sampleCheckbox
                                    objectName: "sampleCheckbox"
                                    theme: root.theme
                                }

                                Text {
                                    anchors.verticalCenter: sampleCheckbox.verticalCenter
                                    text: root.sampleCheckboxChecked ? "启用检查器" : "未启用检查器"
                                    color: root.theme.textPrimary
                                    font.pixelSize: 12
                                }
                            }

                            Row {
                                spacing: 8

                                Components.AppCheckbox {
                                    id: sampleCheckboxDisabled
                                    objectName: "sampleCheckboxDisabled"
                                    theme: root.theme
                                    checked: true
                                    disabled: true
                                }

                                Text {
                                    anchors.verticalCenter: sampleCheckboxDisabled.verticalCenter
                                    text: "禁用状态"
                                    color: root.theme.textMuted
                                    font.pixelSize: 12
                                }
                            }

                            Row {
                                spacing: 8

                                Components.AppCheckbox {
                                    id: sampleCheckboxInvalid
                                    objectName: "sampleCheckboxInvalid"
                                    theme: root.theme
                                    invalid: true
                                }

                                Text {
                                    anchors.verticalCenter: sampleCheckboxInvalid.verticalCenter
                                    text: "错误状态"
                                    color: root.theme.danger
                                    font.pixelSize: 12
                                }
                            }
                        }

                        Components.AppRadioGroup {
                            id: sampleRadioGroup
                            objectName: "sampleRadioGroup"
                            theme: root.theme
                            value: "live"
                            model: [
                                { text: "实时", value: "live" },
                                { text: "草稿", value: "draft" },
                                { text: "批量", value: "batch", disabled: true }
                            ]
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: "当前模式: " + root.sampleRadioValue
                        }
                    }

                    Column {
                        Layout.preferredWidth: Math.max(260, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Toggle / Toggle Group"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Components.AppToggle {
                            id: sampleToggle
                            objectName: "sampleToggle"
                            theme: root.theme
                            text: "启用列表检查"
                            variant: "outline"
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: root.sampleToggleChecked ? "Toggle 已开启" : "Toggle 已关闭"
                        }

                        Components.AppToggleGroup {
                            id: sampleToggleGroup
                            objectName: "sampleToggleGroup"
                            theme: root.theme
                            type: "single"
                            variant: "outline"
                            spacing: 0
                            value: "grid"
                            model: [
                                { text: "网格", value: "grid" },
                                { text: "列表", value: "list" },
                                { text: "堆叠", value: "stack" }
                            ]
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: "当前视图: " + root.sampleToggleGroupValue
                        }
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "菜单系统"
                subtitle: "menubar 与 context-menu 的首批 QML 复刻样例。"

                RowLayout {
                    width: parent.width
                    spacing: 16

                    Column {
                        Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Menubar"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Components.AppMenubar {
                            id: sampleMenubar
                            objectName: "sampleMenubar"
                            width: parent.width
                            theme: root.theme
                            menus: [
                                {
                                    text: "文件",
                                    items: [
                                        { text: "新建批次", shortcut: "Ctrl+N" },
                                        { text: "导出报告", shortcut: "Ctrl+E" },
                                        { type: "separator" },
                                        { text: "删除批次", variant: "destructive" }
                                    ]
                                },
                                {
                                    text: "视图",
                                    items: [
                                        { text: "标准视图", type: "radio", group: "layout", checked: true },
                                        { text: "紧凑视图", type: "radio", group: "layout" },
                                        { text: "显示检查器", type: "checkbox", checked: true }
                                    ]
                                }
                            ]
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: root.sampleMenubarSelectedText.length > 0
                                  ? "最近菜单动作: " + root.sampleMenubarSelectedText
                                  : "尚未触发菜单项"
                        }
                    }

                    Column {
                        Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Context Menu"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Rectangle {
                            width: parent.width
                            height: 84
                            radius: root.theme.radiusMedium
                            color: root.theme.surface
                            border.width: 1
                            border.color: root.theme.stroke

                            Text {
                                anchors.centerIn: parent
                                text: "右键菜单锚点"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                            }
                        }

                        Components.AppContextMenu {
                            id: sampleContextMenu
                            objectName: "sampleContextMenu"
                            theme: root.theme
                            model: [
                                { text: "刷新状态", shortcut: "F5" },
                                { text: "显示波形", type: "checkbox", checked: true },
                                { type: "separator" },
                                { text: "删除工单", variant: "destructive" }
                            ]
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: root.sampleContextMenuSelectedText.length > 0
                                  ? "最近右键动作: " + root.sampleContextMenuSelectedText
                                  : "尚未触发右键菜单"
                        }
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "展开容器"
                subtitle: "accordion 与 collapsible 的首批 QML 复刻样例。"

                RowLayout {
                    width: parent.width
                    spacing: 16

                    Column {
                        Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Accordion"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Components.AppAccordion {
                            id: sampleAccordion
                            objectName: "sampleAccordion"
                            width: parent.width
                            theme: root.theme
                            selectionMode: "multiple"
                            model: [
                                {
                                    value: "keyboard",
                                    title: "支持键盘操作吗？",
                                    description: "验证 Enter / Space 是否可切换展开",
                                    content: "支持。焦点落在触发器后，可使用 Enter 或 Space 切换当前项。"
                                },
                                {
                                    value: "premium",
                                    title: "高级功能说明",
                                    description: "此项已禁用",
                                    content: "升级到专业版以解锁此功能。",
                                    disabled: true
                                },
                                {
                                    value: "collapsible",
                                    title: "支持单项折叠吗？",
                                    description: "验证 single + collapsible 的组合行为",
                                    content: "支持。single 模式下始终保持最多一项展开，并允许再次点击全部收起。"
                                }
                            ]
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: "展开项数量: " + root.sampleAccordionExpandedCount + " (multiple 模式)"
                        }
                    }

                    Column {
                        Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Collapsible"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Components.AppCollapsible {
                            id: sampleCollapsible
                            objectName: "sampleCollapsible"
                            width: parent.width
                            theme: root.theme
                            title: "高级参数"
                            description: "展开后查看更细粒度的参数说明"

                            Text {
                                width: parent.width
                                text: "这里后续会放更完整的高级参数编辑面板。"
                                color: root.theme.textSecondary
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                            }
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: root.sampleCollapsibleOpen ? "Collapsible 已展开" : "Collapsible 已折叠"
                        }
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "基础组件"
                subtitle: "Card、Progress、Skeleton、Separator、Kbd、Label、Switch、Textarea、Select 等基础组件展示。"

                Column {
                    width: parent.width
                    spacing: 16

                    RowLayout {
                        width: parent.width
                        spacing: 16

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Card"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppCard {
                                id: sampleCard
                                objectName: "sampleCard"
                                width: parent.width
                                theme: root.theme

                                Components.AppCardHeader {
                                    Layout.fillWidth: true
                                    theme: root.theme

                                    Components.AppCardTitle {
                                        theme: root.theme
                                        text: "卡片标题"
                                    }

                                    Components.AppCardDescription {
                                        theme: root.theme
                                        text: "这是卡片的描述文本"
                                    }

                                    action: Components.AppCardAction {
                                        theme: root.theme

                                        Components.AppButton {
                                            theme: root.theme
                                            iconName: "more-vertical"
                                            size: "icon-sm"
                                            variant: "ghost"
                                        }
                                    }
                                }

                                Components.AppCardContent {
                                    Layout.fillWidth: true
                                    theme: root.theme

                                    Text {
                                        Layout.fillWidth: true
                                        text: "卡片内容区域，可以放置任意内容。"
                                        color: root.theme.textSecondary
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                    }
                                }

                                Components.AppCardFooter {
                                    Layout.fillWidth: true
                                    theme: root.theme

                                    Components.AppButton {
                                        theme: root.theme
                                        text: "确认"
                                        size: "sm"
                                    }

                                    Components.AppButton {
                                        theme: root.theme
                                        text: "取消"
                                        variant: "outline"
                                        size: "sm"
                                    }
                                }
                            }
                        }

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Progress & Skeleton"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppProgress {
                                id: sampleProgress
                                objectName: "sampleProgress"
                                width: parent.width
                                theme: root.theme
                                value: 65
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: "进度: 65%"
                            }

                            Components.AppSkeleton {
                                id: sampleSkeleton
                                objectName: "sampleSkeleton"
                                width: parent.width
                                height: 20
                                theme: root.theme
                            }

                            Components.AppSkeleton {
                                width: parent.width * 0.7
                                height: 20
                                theme: root.theme
                            }
                        }
                    }

                    Components.AppSeparator {
                        id: sampleSeparator
                        objectName: "sampleSeparator"
                        width: parent.width
                        theme: root.theme
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 16

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Kbd & Label"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            RowLayout {
                                spacing: 8

                                Components.AppLabel {
                                    id: sampleLabel
                                    objectName: "sampleLabel"
                                    theme: root.theme
                                    text: "快捷键："
                                }

                                Components.AppKbd {
                                    id: sampleKbd
                                    objectName: "sampleKbd"
                                    theme: root.theme
                                    text: "Ctrl"
                                }

                                Components.AppKbd {
                                    theme: root.theme
                                    text: "S"
                                }
                            }
                        }

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Switch"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppSwitch {
                                id: sampleSwitch
                                objectName: "sampleSwitch"
                                theme: root.theme
                                checked: true
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: sampleSwitch.checked ? "开关已开启" : "开关已关闭"
                            }
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 16

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Textarea"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppTextarea {
                                id: sampleTextarea
                                objectName: "sampleTextarea"
                                width: parent.width
                                theme: root.theme
                                placeholderText: "请输入多行文本..."
                            }
                        }

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Select"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppSelect {
                                id: sampleSelect
                                objectName: "sampleSelect"
                                width: parent.width
                                theme: root.theme
                                label: "选择选项"
                                model: ["选项 1", "选项 2", "选项 3"]
                            }
                        }
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "消息与确认"
                subtitle: "alert 与 alert-dialog 的首批 QML 复刻样例。"

                RowLayout {
                    width: parent.width
                    spacing: 16

                    Column {
                        Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Alert"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Components.AppAlert {
                            id: sampleAlert
                            objectName: "sampleAlert"
                            width: parent.width
                            theme: root.theme
                            variant: "default"
                            title: "新功能可用"
                            description: "我们添加了深色模式支持。您可以在账户设置中启用它。"

                            icon: [
                                Components.AppIcon {
                                    name: "info"
                                    color: root.theme.textPrimary
                                    iconSize: 16
                                }
                            ]
                        }

                        Components.AppAlert {
                            objectName: "sampleAlertDestructive"
                            width: parent.width
                            theme: root.theme
                            variant: "destructive"
                            title: "错误：连接失败"
                            description: "无法连接到设备。请检查网络连接并重试。"

                            icon: [
                                Components.AppIcon {
                                    name: "error"
                                    color: root.theme.danger
                                    iconSize: 16
                                }
                            ]
                        }
                    }

                    Components.AppButton {
                        theme: root.theme
                        text: "Show Dialog"
                        variant: "outline"
                        onClicked: root.openSampleAlertDialog()
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "身份与路径"
                subtitle: "avatar、breadcrumb、button-group、field、form 的首批 QML 复刻样例。"

                Column {
                    width: parent.width
                    spacing: 8

                    Components.AppAvatarGroup {
                        id: sampleAvatarGroup
                        objectName: "sampleAvatarGroup"
                        theme: root.theme

                        Components.AppAvatar {
                            id: sampleAvatar
                            objectName: "sampleAvatar"
                            theme: root.theme
                            fallbackText: "FG"
                            badgeVisible: true
                            badgeVariant: "success"
                        }

                        Components.AppAvatar {
                            objectName: "sampleAvatarSecondary"
                            theme: root.theme
                            size: "lg"
                            fallbackText: "QA"
                            badgeVisible: true
                            badgeText: "1"
                        }

                        Components.AppAvatarGroupCount {
                            theme: root.theme
                            text: "+2"
                        }
                    }

                    Components.AppBreadcrumb {
                        id: sampleBreadcrumb
                        objectName: "sampleBreadcrumb"
                        theme: root.theme
                        model: [
                            { text: "工位管理", value: "stations" },
                            { text: "测试执行", value: "execution" },
                            { text: "组件库", value: "gallery", current: true }
                        ]
                        onCrumbTriggered: function(index, value) {
                            root.sampleBreadcrumbLastValue = String(value)
                        }
                    }

                    Components.AppButtonGroup {
                        id: sampleButtonGroup
                        objectName: "sampleButtonGroup"
                        theme: root.theme
                        model: [
                            { text: "新建" },
                            { type: "separator" },
                            { text: "导出" },
                            { text: "删除", variant: "ghost" }
                        ]
                        onButtonClicked: function(index, value) {
                            root.sampleButtonGroupLastValue = String(value)
                        }
                    }

                    Components.AppForm {
                        id: sampleForm
                        objectName: "sampleForm"
                        width: parent.width
                        theme: root.theme
                        title: "参数表单"
                        description: "演示 field/form 的结构层，不接完整验证框架。"

                        Components.AppField {
                            id: sampleField
                            objectName: "sampleField"
                            width: parent.width
                            theme: root.theme
                            label: "批次编号"
                            description: "请输入当前测试批次编号"
                            errorText: "示例错误信息"
                            control: sampleFieldInput

                            Components.AppInput {
                                id: sampleFieldInput
                                width: 220
                                theme: root.theme
                                placeholderText: "FG-2026-0416"
                            }
                        }
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "数据与输入"
                subtitle: "table、slider、pagination、native-select、input-group、input-otp、scroll-area 的主线程批量复刻样例。"

                Column {
                    width: parent.width
                    spacing: 14

                    Components.AppTable {
                        id: sampleTable
                        objectName: "sampleTable"
                        width: parent.width
                        height: 180
                        theme: root.theme
                        headers: ["工位", "状态", "批次", "结果"]
                        columnKeys: ["station", "status", "batch", "result"]
                        rows: [
                            { station: "03", status: "运行中", batch: "FG-0416-01", result: "PASS" },
                            { station: "07", status: "待机", batch: "FG-0416-02", result: "WAIT" },
                            { station: "12", status: "告警", batch: "FG-0416-03", result: "NG" }
                        ]
                        caption: "通用表格将替代业务专用 DataTableCard 的底层展示责任。"
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 16

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Slider / Pagination"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppSlider {
                                id: sampleSlider
                                objectName: "sampleSlider"
                                width: parent.width
                                theme: root.theme
                                from: 0
                                to: 100
                                value: 25
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: "当前阈值: " + root.sampleSliderValue
                            }

                            Components.AppPagination {
                                id: samplePagination
                                objectName: "samplePagination"
                                theme: root.theme
                                totalPages: 8
                                currentPage: 2
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: "当前页: " + root.samplePaginationPage
                            }
                        }

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Native Select / Input OTP"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppNativeSelect {
                                id: sampleNativeSelect
                                objectName: "sampleNativeSelect"
                                theme: root.theme
                                label: "工位"
                                model: [
                                    { text: "工位 #03", value: "station-03" },
                                    { text: "工位 #07", value: "station-07" },
                                    { text: "工位 #12", value: "station-12" }
                                ]
                                currentIndex: 0
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: "当前工位值: " + root.sampleNativeSelectValue
                            }

                            Components.AppInputOtp {
                                id: sampleInputOtp
                                objectName: "sampleInputOtp"
                                theme: root.theme
                                length: 6
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: root.sampleInputOtpValue.length > 0 ? "OTP: " + root.sampleInputOtpValue : "尚未输入 OTP"
                            }
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 16

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Input Group"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppInputGroup {
                                id: sampleInputGroup
                                objectName: "sampleInputGroup"
                                width: parent.width
                                theme: root.theme
                                leadingText: "https://"
                                trailingText: ".local"

                                Components.AppInput {
                                    width: 180
                                    theme: root.theme
                                    placeholderText: "factory-station"
                                }
                            }
                        }

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Scroll Area"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppScrollArea {
                                id: sampleScrollArea
                                objectName: "sampleScrollArea"
                                width: parent.width
                                height: 140
                                theme: root.theme

                                Repeater {
                                    model: 10

                                    delegate: Text {
                                        required property int index
                                        width: sampleScrollArea.width - 28
                                        text: "滚动条目 #" + (index + 1)
                                        color: root.theme.textSecondary
                                        font.pixelSize: 12
                                    }
                                }
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: root.sampleScrollAtEnd ? "滚动已到底部" : "滚动未到底部"
                            }
                        }
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "复杂布局与比例"
                subtitle: "calendar、carousel、resizable、aspect-ratio，以及 range slider 与增强表格样例。"

                Column {
                    width: parent.width
                    spacing: 14

                    RowLayout {
                        width: parent.width
                        spacing: 16

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Calendar / Carousel"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppCalendar {
                                id: sampleCalendar
                                objectName: "sampleCalendar"
                                width: Math.min(parent.width, sampleCalendar.implicitWidth)
                                theme: root.theme
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: "当前月份: " + root.sampleCalendarMonthLabel
                            }

                            Components.AppCarousel {
                                id: sampleCarousel
                                objectName: "sampleCarousel"
                                width: Math.min(parent.width, 360)
                                theme: root.theme
                                model: [
                                    { title: "运行总览", description: "用于承载运行时总览卡片。" },
                                    { title: "告警列表", description: "用于承载当前告警摘要。" },
                                    { title: "报表入口", description: "用于承载报表与导出入口。" }
                                ]
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: "当前轮播索引: " + root.sampleCarouselIndex
                            }
                        }

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Aspect Ratio / Resizable"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppAspectRatio {
                                id: sampleAspectRatio
                                objectName: "sampleAspectRatio"
                                width: Math.min(parent.width, 320)
                                ratio: 16 / 9

                                Rectangle {
                                    anchors.fill: parent
                                    radius: root.theme.radiusMedium
                                    color: root.theme.surface
                                    border.width: 1
                                    border.color: root.theme.stroke

                                    Text {
                                        anchors.centerIn: parent
                                        text: "16:9"
                                        color: root.theme.textPrimary
                                        font.pixelSize: 12
                                        font.bold: true
                                    }
                                }
                            }

                            Components.AppResizable {
                                id: sampleResizable
                                objectName: "sampleResizable"
                                width: Math.min(parent.width, 360)
                                height: 120
                                theme: root.theme
                                ratio: 55

                                Rectangle {
                                    anchors.fill: parent
                                    radius: root.theme.radiusSmall
                                    color: root.theme.accentWeak

                                    Text {
                                        anchors.centerIn: parent
                                        text: "左侧面板"
                                        color: root.theme.textPrimary
                                        font.pixelSize: 12
                                    }
                                }

                                secondContent: [
                                    Rectangle {
                                        anchors.fill: parent
                                        radius: root.theme.radiusSmall
                                        color: root.theme.surface

                                        Text {
                                            anchors.centerIn: parent
                                            text: "右侧面板"
                                            color: root.theme.textSecondary
                                            font.pixelSize: 12
                                        }
                                    }
                                ]
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: "当前分割比例: " + root.sampleResizableRatio + "%"
                            }
                        }
                    }

                    RowLayout {
                        width: parent.width
                        spacing: 16

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Range Slider"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppSlider {
                                id: sampleRangeSlider
                                objectName: "sampleRangeSlider"
                                width: Math.min(parent.width, 320)
                                theme: root.theme
                                from: 0
                                to: 100
                                values: [15, 85]
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: "范围: " + root.sampleRangeSliderValuesText
                            }
                        }

                        Column {
                            Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                            spacing: 8

                            Text {
                                text: "Enhanced Table"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Components.AppTable {
                                id: sampleEnhancedTable
                                objectName: "sampleEnhancedTable"
                                width: parent.width
                                height: 170
                                theme: root.theme
                                headers: ["项目", "状态", "说明"]
                                columnKeys: ["name", "status", "desc"]
                                rows: [
                                    { name: "串口检查", status: "PASS", desc: "所有设备在线", variant: "success" },
                                    { name: "参数校验", status: "WARN", desc: "发现 1 处差异", variant: "warning" },
                                    { name: "权限检查", status: "PASS", desc: "当前用户可执行", variant: "selected" }
                                ]
                                footerRows: [
                                    { name: "总计 3 条", status: "", desc: "" }
                                ]
                                caption: "增强版表格已支持 footerRows 与行级 variant。"
                            }

                            Text {
                                color: root.theme.textMuted
                                font.pixelSize: 12
                                text: root.sampleTableFooterText
                            }
                        }
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "导航与延伸浮层"
                subtitle: "navigation-menu、hover-card、drawer 的主线程批量复刻样例。"

                RowLayout {
                    width: parent.width
                    spacing: 16

                    Column {
                        Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Navigation Menu"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Components.AppNavigationMenu {
                            id: sampleNavigationMenu
                            objectName: "sampleNavigationMenu"
                            width: Math.min(parent.width, 360)
                            theme: root.theme
                            model: [
                                {
                                    text: "布局",
                                    items: [
                                        { title: "标准布局", description: "适合大屏工位操作" },
                                        { title: "检查器布局", description: "适合调试与诊断使用" }
                                    ]
                                },
                                {
                                    text: "报告",
                                    items: [
                                        { title: "日报告", description: "按天汇总产线数据" },
                                        { title: "批次报告", description: "聚焦单批次追踪" }
                                    ]
                                }
                            ]
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: root.sampleNavigationMenuSelectedText.length > 0
                                  ? "最近导航项: " + root.sampleNavigationMenuSelectedText
                                  : "尚未选择导航项"
                        }
                    }

                    Column {
                        Layout.preferredWidth: Math.max(280, (parent.width - 16) / 2)
                        spacing: 8

                        Text {
                            text: "Hover Card / Drawer"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Rectangle {
                            width: Math.min(parent.width, 280)
                            height: 52
                            radius: root.theme.radiusMedium
                            color: root.theme.surface
                            border.width: 1
                            border.color: root.theme.stroke

                            Text {
                                anchors.centerIn: parent
                                text: "HoverCard Anchor"
                                color: root.theme.textPrimary
                                font.pixelSize: 12
                            }

                            HoverHandler {
                                onHoveredChanged: {
                                    if (hovered) root.showSampleHoverCard()
                                    else root.hideSampleHoverCard()
                                }
                            }

                            Components.AppHoverCard {
                                id: sampleHoverCard
                                objectName: "sampleHoverCard"
                                x: 0
                                y: 56
                                theme: root.theme
                                openDelay: 0
                                closeDelay: 0
                                title: "工位 #03"
                                description: "用于展示工位的扩展信息摘要。"

                                Text {
                                    width: parent.width
                                    text: "良率 98.4%，最近一次告警已恢复。"
                                    color: root.theme.textSecondary
                                    font.pixelSize: 12
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        Row {
                            spacing: 8

                            Components.AppButton {
                                theme: root.theme
                                text: "打开 Drawer"
                                size: "sm"
                                variant: "outline"
                                onClicked: root.openSampleDrawer()
                            }

                            Components.AppButton {
                                theme: root.theme
                                text: "关闭 Drawer"
                                size: "sm"
                                variant: "ghost"
                                onClicked: root.closeSampleDrawer()
                            }
                        }

                        Text {
                            color: root.theme.textMuted
                            font.pixelSize: 12
                            text: "HoverCard: " + (root.sampleHoverCardOpen ? "open" : "closed")
                                  + " | Drawer: " + (root.sampleDrawerOpen ? "open" : "closed")
                        }
                    }
                }
            }

            Components.SectionCard {
                width: parent.width
                theme: root.theme
                title: "弹出层"
                subtitle: "Dialog、Sheet、Popover、Command 都通过独立页统一承载，便于后续继续补齐 shadcn 交互矩阵。"

                Flow {
                    width: parent.width
                    spacing: 8

                    Components.AppButton {
                        theme: root.theme
                        text: "打开 Dialog"
                        onClicked: root.openSampleDialog()
                    }

                    Components.AppButton {
                        theme: root.theme
                        text: "打开 Sheet"
                        variant: "secondary"
                        onClicked: root.openSampleSheet()
                    }

                    Components.AppButton {
                        theme: root.theme
                        text: "打开 Popover"
                        variant: "outline"
                        onClicked: root.openSamplePopover()
                    }

                    Components.AppButton {
                        theme: root.theme
                        text: "打开 Command"
                        variant: "ghost"
                        onClicked: root.openSampleCommand()
                    }
                }

                Text {
                    width: parent.width
                    color: root.theme.textMuted
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    text: "Sheet: " + (root.sampleSheetOpen ? "open" : "closed")
                          + " | Popover: " + (root.samplePopoverOpen ? "open" : "closed")
                          + " | Command: " + (root.sampleCommandOpen ? "open" : "closed")
                          + (root.sampleCommandSelectedText.length > 0 ? " | 最近命令: " + root.sampleCommandSelectedText : "")
                }

                Item {
                    width: parent.width
                    height: 168

                    Rectangle {
                        width: 212
                        height: 44
                        radius: root.theme.radiusMedium
                        color: root.theme.surface
                        border.width: 1
                        border.color: root.theme.stroke

                        Text {
                            anchors.centerIn: parent
                            text: "Popover Anchor"
                            color: root.theme.textPrimary
                            font.pixelSize: 12
                        }
                    }

                    Components.AppPopover {
                        id: samplePopover
                        objectName: "samplePopover"
                        x: 0
                        y: 52
                        theme: root.theme
                        title: "示例 Popover"
                        description: "用于复刻 shadcn/ui 的 Popover 结构。"

                        Text {
                            width: parent.width
                            text: "这里后续会用于 Tooltip、Dropdown、Combobox 的局部能力复用。"
                            color: root.theme.textSecondary
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
    }

    Components.AppDialog {
        id: sampleDialog
        objectName: "sampleDialog"
        anchors.fill: parent
        theme: root.theme
        title: "示例对话框"
        description: "这是为 shadcn/ui 弹层体系复刻准备的基础 Dialog 样例。"
        showCloseButton: true

        Text {
            width: parent.width
            text: "后续 Sheet、Popover、DropdownMenu 会沿这条基础层继续扩展。"
            color: root.theme.textSecondary
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }

    Components.AppAlertDialog {
        id: sampleAlertDialog
        objectName: "sampleAlertDialog"
        anchors.fill: parent
        theme: root.theme
        title: "Are you absolutely sure?"
        description: "This action cannot be undone. This will permanently delete your account and remove your data from our servers."
        confirmText: "Continue"
        cancelText: "Cancel"
    }

    Components.AppDrawer {
        id: sampleDrawer
        objectName: "sampleDrawer"
        anchors.fill: parent
        theme: root.theme
        title: "批次详情"
        description: "这里后续会承载更完整的抽屉面板内容。"

        Text {
            width: parent.width
            text: "抽屉可用于承载不打断主页面的扩展详情。"
            color: root.theme.textSecondary
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }

    Components.AppSheet {
        id: sampleSheet
        objectName: "sampleSheet"
        anchors.fill: parent
        theme: root.theme
        title: "示例侧滑层"
        description: "用于复刻 shadcn/ui 的 Sheet 行为基线。"

        Text {
            width: parent.width
            text: "这里后续会承载更复杂的侧滑内容。"
            color: root.theme.textSecondary
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }

    Components.AppCommand {
        id: sampleCommand
        objectName: "sampleCommand"
        anchors.fill: parent
        theme: root.theme
        model: [
            { text: "切换到标准视图", shortcut: "Ctrl+1" },
            { text: "切换到紧凑视图", shortcut: "Ctrl+2" },
            { text: "打开检查器", shortcut: "Ctrl+3" }
        ]
    }

    Components.ThemeSwitcherPanel {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 16
        anchors.bottomMargin: 16
        z: 10
        theme: root.theme
        targetTheme: root.theme
    }
}


