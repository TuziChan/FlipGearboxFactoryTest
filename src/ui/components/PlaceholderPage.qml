pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    required property string pageTitle
    required property string pageDescription
    required property string pageTag

    Rectangle {
        anchors.fill: parent
        color: root.theme.bgColor

        SectionCard {
            anchors.centerIn: parent
            width: Math.min(parent.width - 48, 760)
            height: 360
            theme: root.theme
            title: root.pageTitle
            subtitle: root.pageDescription

            AppBadge {
                theme: root.theme
                text: root.pageTag
                variant: "default"
            }

            Text {
                text: "当前已接入真实导航切换。后续只需要把该占位页替换成实际功能组件即可。"
                color: root.theme.textMuted
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width
            }
        }
    }
}
