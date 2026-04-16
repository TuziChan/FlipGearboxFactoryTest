import QtQuick
import QtQuick.Layouts

SectionCard {
    id: root
    objectName: "themeSwitcherPanel"

    required property AppTheme targetTheme

    property bool collapsed: true

    function toggleCollapsed() {
        root.collapsed = !root.collapsed
        return true
    }

    function selectThemeName(name) {
        if (root.targetTheme.availableThemes.indexOf(name) === -1)
            return false
        root.targetTheme.themeName = name
        return true
    }

    function selectStyleName(name) {
        if (root.targetTheme.availableStyles.indexOf(name) === -1)
            return false
        root.targetTheme.styleName = name
        return true
    }

    function toggleDarkMode() {
        root.targetTheme.darkMode = !root.targetTheme.darkMode
        return true
    }

    width: collapsed ? 132 : 236
    title: collapsed ? "Theme" : "Theme Lab"
    subtitle: collapsed ? "" : "shadcn-inspired runtime switcher"

    AppButton {
        width: parent.width
        text: root.collapsed ? "展开主题" : "收起主题"
        variant: "secondary"
        block: true
        theme: root.theme
        onClicked: root.toggleCollapsed()
    }

    Column {
        visible: !root.collapsed
        width: parent.width
        spacing: 10

        AppSelect {
            id: themeSelect
            objectName: "themeSelectControl"
            width: parent.width
            theme: root.theme
            label: "Theme"
            model: root.targetTheme.availableThemes
            currentIndex: Math.max(0, root.targetTheme.availableThemes.indexOf(root.targetTheme.themeName))
            onCurrentIndexChanged: {
                const next = root.targetTheme.availableThemes[currentIndex]
                if (next)
                    root.selectThemeName(next)
            }
        }

        AppSelect {
            id: styleSelect
            objectName: "styleSelectControl"
            width: parent.width
            theme: root.theme
            label: "Style"
            model: root.targetTheme.availableStyles
            currentIndex: Math.max(0, root.targetTheme.availableStyles.indexOf(root.targetTheme.styleName))
            onCurrentIndexChanged: {
                const next = root.targetTheme.availableStyles[currentIndex]
                if (next)
                    root.selectStyleName(next)
            }
        }

        RowLayout {
            width: parent.width

            Text {
                text: "Dark Mode"
                color: root.theme.textSecondary
                font.pixelSize: 12
            }

            Item { Layout.fillWidth: true }

            AppButton {
                objectName: "darkModeToggle"
                text: root.targetTheme.darkMode ? "ON" : "OFF"
                variant: root.targetTheme.darkMode ? "primary" : "secondary"
                size: "sm"
                theme: root.theme
                onClicked: root.toggleDarkMode()
            }
        }

        AppBadge {
            theme: root.theme
            text: `${root.targetTheme.themeName} / ${root.targetTheme.styleName}`
            variant: "secondary"
        }
    }
}
