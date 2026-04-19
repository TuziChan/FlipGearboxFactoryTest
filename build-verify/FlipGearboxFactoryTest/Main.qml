pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import "src/ui/pages" as Pages

ApplicationWindow {
    width: 1440
    height: 900
    minimumWidth: 1280
    minimumHeight: 760
    visible: true
    color: "#F3F3F3"
    title: qsTr("齿轮箱产线测试系统 v1.0")

    Pages.AppShell {
        anchors.fill: parent
    }
}
