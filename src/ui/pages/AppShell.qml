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

                    TestExecutionPage { theme: theme }
                    RecipePage { theme: theme }
                    HistoryPage { theme: theme }
                    DeviceConfigPage { theme: theme }
                    DiagnosticsPage { theme: theme }
                    ComponentGalleryPage { theme: theme }
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
                items: [
                    { name: "CAN 心跳" },
                    { name: "DYN200" },
                    { name: "AQMD" },
                    { name: "编码器" },
                    { name: "磁粉制动" }
                ]
                clockText: Qt.formatDateTime(new Date(), "yyyy-MM-dd hh:mm:ss")
            }
        }

        Components.ThemeSwitcherPanel {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 16
            anchors.bottomMargin: 46
            z: 10
            theme: theme
            targetTheme: theme
        }
    }
}
