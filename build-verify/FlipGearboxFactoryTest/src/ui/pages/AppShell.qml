pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import "../components" as Components

Item {
    id: root
    objectName: "appShell"

    property int activeNavIndex: 0
    readonly property int componentGalleryNavIndex: 5
    property string infoText: ""
    property string infoType: ""
    property string currentPageTitle: activeNavIndex >= 0 && activeNavIndex < navModel.count
                                      ? navModel.get(activeNavIndex).title
                                      : ""

    readonly property var devStatuses: {
        if (typeof diagnosticsViewModel !== 'undefined' && diagnosticsViewModel && diagnosticsViewModel.deviceStatuses.length > 0) {
            return diagnosticsViewModel.deviceStatuses
        }
        return [
            { name: "AQMD 电机驱动器", status: "offline" },
            { name: "DYN200 扭矩传感器", status: "offline" },
            { name: "单圈绝对值编码器", status: "offline" },
            { name: "制动电源", status: "offline" }
        ]
    }
    readonly property bool allDevicesOnline: {
        if (devStatuses.length === 0) return false
        for (var i = 0; i < devStatuses.length; i++) {
            if (devStatuses[i].status !== "online") return false
        }
        return true
    }
    readonly property string connText: devStatuses.length > 0
                                       ? (allDevicesOnline ? "所有设备在线" : "设备离线")
                                       : ""

    function selectNav(index) {
        if (index < 0 || index >= navModel.count)
            return false
        root.activeNavIndex = index
        if (index === 0) {
            root.infoText = ""
            root.infoType = ""
        } else {
            root.infoText = "已切换到 " + navModel.get(index).title
            root.infoType = "success"
        }
        return true
    }

    function toggleNavRail() {
        sidebarNav.expanded = !sidebarNav.expanded
        return true
    }

    function openComponentGallery() {
        return selectNav(componentGalleryNavIndex)
    }

    Components.AppTheme {
        id: theme
        objectName: "appTheme"
        themeName: "sky"
        styleName: "mira"
        darkMode: false
    }

    ListModel {
        id: navModel
        ListElement { iconName: "test"; title: "测试执行" }
        ListElement { iconName: "recipe"; title: "配方管理" }
        ListElement { iconName: "history"; title: "历史记录" }
        ListElement { iconName: "device"; title: "设备配置" }
        ListElement { iconName: "diagnostics"; title: "I/O 诊断" }
        ListElement { iconName: "library"; title: "组件库" }
    }

    Rectangle {
        anchors.fill: parent
        color: theme.bgColor

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Components.TopToolbar {
                Layout.fillWidth: true
                theme: theme
                pageTitle: root.currentPageTitle
                connectionText: root.connText
                isConnected: root.allDevicesOnline
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                Components.SidebarNav {
                    id: sidebarNav
                    Layout.fillHeight: true
                    theme: theme
                    model: navModel
                    activeIndex: root.activeNavIndex
                    onItemSelected: function(index) { root.selectNav(index) }
                }

                StackLayout {
                    objectName: "contentStack"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: root.activeNavIndex

                    Loader { active: StackLayout.isCurrentItem; sourceComponent: TestExecutionPage { theme: theme } }
                    Loader { active: StackLayout.isCurrentItem; sourceComponent: RecipePage { theme: theme } }
                    Loader { active: StackLayout.isCurrentItem; sourceComponent: HistoryPage { theme: theme } }
                    Loader { active: StackLayout.isCurrentItem; sourceComponent: DeviceConfigPage { theme: theme } }
                    Loader { active: StackLayout.isCurrentItem; sourceComponent: DiagnosticsPage { theme: theme } }
                    Loader { active: StackLayout.isCurrentItem; sourceComponent: ComponentGalleryPage { theme: theme } }
                }
            }

            Components.AlertStrip {
                Layout.fillWidth: true
                theme: theme
                variant: root.infoType === "error" ? "danger" : "warning"
                title: root.infoType === "success" ? "OK" : root.infoType === "error" ? "NG" : "提示"
                text: root.activeNavIndex === 0 ? "" : root.infoText
            }

            Components.BottomStatusBar {
                Layout.fillWidth: true
                theme: theme
                items: {
                    var result = [];
                    for (var i = 0; i < root.devStatuses.length; i++) {
                        var d = root.devStatuses[i];
                        result.push({ name: d.name, online: d.status === "online" });
                    }
                    return result;
                }
                clockText: Qt.formatDateTime(new Date(), "yyyy-MM-dd hh:mm:ss")
            }
        }

        // ThemeSwitcherPanel is now only shown in the ComponentGalleryPage
    }
}
