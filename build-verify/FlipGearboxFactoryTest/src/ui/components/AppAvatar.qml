pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    required property AppTheme theme
    property string imageSource: ""
    property string fallbackText: ""
    property string accessibleName: fallbackText
    property string size: "default"
    property bool imageEnabled: true
    property bool badgeVisible: false
    property string badgeText: ""
    property string badgeIconName: ""
    property string badgeVariant: "primary"
    property color badgeColor: "transparent"

    readonly property int avatarSize: size === "sm" ? 24 : size === "lg" ? 40 : 32
    readonly property int fallbackFontSize: size === "sm" ? 10 : 12
    readonly property int badgeSize: size === "sm" ? 8 : size === "lg" ? 12 : 10
    readonly property int badgeRingWidth: size === "sm" ? 1 : 2
    readonly property int badgeContentSize: size === "sm" ? 6 : 7
    readonly property color fallbackBackground: theme.muted
    readonly property color fallbackForeground: theme.textMuted
    readonly property color badgeBackground: badgeColor !== "transparent"
                                           ? badgeColor
                                           : badgeVariant === "danger"
                                             ? theme.danger
                                             : badgeVariant === "warning"
                                               ? theme.warn
                                               : badgeVariant === "success"
                                                 ? theme.ok
                                                 : theme.accent
    readonly property color badgeForeground: badgeVariant === "warning"
                                           ? theme.textPrimary
                                           : theme.primaryForeground
    readonly property bool hasImageSource: imageEnabled && imageSource.length > 0
    readonly property bool showImage: hasImageSource && avatarImage.status === Image.Ready
    readonly property bool showFallback: !showImage
    readonly property bool showBadgeText: badgeVisible && badgeText.length > 0
    readonly property bool showBadgeIcon: badgeVisible && badgeText.length === 0 && badgeIconName.length > 0

    implicitWidth: avatarSize
    implicitHeight: avatarSize

    Rectangle {
        id: avatarFrame
        anchors.fill: parent
        radius: width / 2
        color: root.fallbackBackground
        border.width: 1
        border.color: root.theme.stroke
        clip: true

        Image {
            id: avatarImage
            anchors.fill: parent
            source: root.imageSource
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            smooth: true
            cache: true
            visible: root.hasImageSource
        }

        Rectangle {
            anchors.fill: parent
            radius: width / 2
            color: "transparent"
            border.width: 1
            border.color: root.theme.borderColor
            opacity: 0.35
        }

        Text {
            anchors.centerIn: parent
            visible: root.showFallback
            text: root.fallbackText.length > 0 ? root.fallbackText : "?"
            color: root.fallbackForeground
            font.pixelSize: root.fallbackFontSize
            font.bold: true
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: root.badgeSize
        height: root.badgeSize
        radius: width / 2
        visible: root.badgeVisible
        color: root.badgeBackground
        border.width: root.badgeRingWidth
        border.color: root.theme.bgColor

        Text {
            anchors.centerIn: parent
            visible: root.showBadgeText
            text: root.badgeText
            color: root.badgeForeground
            font.pixelSize: root.badgeContentSize
            font.bold: true
        }

        AppIcon {
            anchors.centerIn: parent
            visible: root.showBadgeIcon
            name: root.badgeIconName
            color: root.badgeForeground
            iconSize: root.badgeContentSize
        }
    }
}
