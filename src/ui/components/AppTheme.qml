import QtQuick

QtObject {
    id: root

    property string themeName: "sky"
    property string styleName: "mira"
    property bool darkMode: false

    readonly property var availableThemes: [
        "neutral", "stone", "zinc", "mauve", "olive", "mist", "taupe",
        "amber", "blue", "cyan", "emerald", "fuchsia", "green", "indigo",
        "lime", "orange", "pink", "purple", "red", "rose", "sky", "teal",
        "violet", "yellow"
    ]
    readonly property var availableStyles: ["vega", "nova", "maia", "lyra", "mira", "luma"]

    function baseLight() {
        return {
            background: "#FFFFFF",
            foreground: "#18181B",
            card: "#FFFFFF",
            cardForeground: "#18181B",
            primary: "#27272A",
            primaryForeground: "#FAFAFA",
            secondary: "#F4F4F5",
            secondaryForeground: "#27272A",
            muted: "#F4F4F5",
            mutedForeground: "#71717A",
            accent: "#F4F4F5",
            accentForeground: "#27272A",
            border: "#E4E4E7",
            input: "#E4E4E7",
            sidebar: "#FAFAFA",
            sidebarAccent: "#F4F4F5",
            sidebarBorder: "#E4E4E7"
        }
    }

    function baseDark() {
        return {
            background: "#18181B",
            foreground: "#FAFAFA",
            card: "#27272A",
            cardForeground: "#FAFAFA",
            primary: "#E4E4E7",
            primaryForeground: "#18181B",
            secondary: "#3F3F46",
            secondaryForeground: "#FAFAFA",
            muted: "#3F3F46",
            mutedForeground: "#A1A1AA",
            accent: "#3F3F46",
            accentForeground: "#FAFAFA",
            border: "#3F3F46",
            input: "#3F3F46",
            sidebar: "#27272A",
            sidebarAccent: "#3F3F46",
            sidebarBorder: "#3F3F46"
        }
    }

    function lightOverrides() {
        return {
            neutral: { foreground: "#171717", primary: "#1F2937", primaryForeground: "#FAFAFA", secondary: "#F5F5F5", muted: "#F5F5F5", mutedForeground: "#737373", accent: "#F5F5F5", accentForeground: "#1F2937", border: "#E5E5E5", input: "#E5E5E5", sidebar: "#FAFAFA", sidebarAccent: "#F5F5F5", sidebarBorder: "#E5E5E5" },
            stone: { foreground: "#292524", primary: "#44403C", primaryForeground: "#FAFAF9", secondary: "#F5F5F4", muted: "#F5F5F4", mutedForeground: "#78716C", accent: "#F5F5F4", accentForeground: "#44403C", border: "#E7E5E4", input: "#E7E5E4", sidebar: "#FAFAF9", sidebarAccent: "#F5F5F4", sidebarBorder: "#E7E5E4" },
            zinc: {},
            mauve: { foreground: "#221A22", primary: "#50324D", primaryForeground: "#FAF7FB", secondary: "#F5F1F6", muted: "#F5F1F6", mutedForeground: "#7B6877", accent: "#F5F1F6", accentForeground: "#50324D", border: "#E6DFE7", input: "#E6DFE7", sidebarAccent: "#F5F1F6" },
            olive: { foreground: "#24281D", primary: "#475A2D", primaryForeground: "#FBFFF6", secondary: "#F3F6EB", muted: "#F3F6EB", mutedForeground: "#728064", accent: "#EEF4E0", accentForeground: "#475A2D", border: "#DDE5CB", input: "#DDE5CB", sidebarAccent: "#EEF4E0" },
            mist: { foreground: "#1F2937", primary: "#4A7DFF", primaryForeground: "#F8FBFF", secondary: "#EFF4FA", muted: "#EFF4FA", mutedForeground: "#6B7280", accent: "#EEF5FF", accentForeground: "#1E3A8A", border: "#DDE5F0", input: "#DDE5F0", sidebar: "#F7FAFD", sidebarAccent: "#EEF5FF", sidebarBorder: "#DDE5F0" },
            taupe: { foreground: "#2B2523", primary: "#544640", primaryForeground: "#FBF8F7", secondary: "#F5F1EF", muted: "#F5F1EF", mutedForeground: "#7D6F68", accent: "#F5F1EF", accentForeground: "#544640", border: "#E5DDD8", input: "#E5DDD8", sidebar: "#FBF8F7", sidebarAccent: "#F5F1EF", sidebarBorder: "#E5DDD8" },
            amber: { primary: "#B45309", primaryForeground: "#FFFBEB", accent: "#FFF5E1", accentForeground: "#9A4B07", sidebarAccent: "#FFF5E1" },
            blue: { primary: "#2563EB", primaryForeground: "#EFF6FF", accent: "#E8F0FF", accentForeground: "#1D4ED8", sidebarAccent: "#E8F0FF" },
            cyan: { primary: "#0EA5E9", primaryForeground: "#F0F9FF", accent: "#E6F7FD", accentForeground: "#0369A1", sidebarAccent: "#E6F7FD" },
            emerald: { primary: "#10B981", primaryForeground: "#ECFDF5", accent: "#E8FAF0", accentForeground: "#047857", sidebarAccent: "#E8FAF0" },
            fuchsia: { primary: "#C026D3", primaryForeground: "#FDF4FF", accent: "#FAE8FF", accentForeground: "#A21CAF", sidebarAccent: "#FAE8FF" },
            green: { primary: "#22C55E", primaryForeground: "#F0FDF4", accent: "#EAF9EF", accentForeground: "#15803D", sidebarAccent: "#EAF9EF" },
            indigo: { primary: "#4F46E5", primaryForeground: "#EEF2FF", accent: "#EAEFFF", accentForeground: "#3730A3", sidebarAccent: "#EAEFFF" },
            lime: { primary: "#84CC16", primaryForeground: "#365314", accent: "#F7FEE7", accentForeground: "#4D7C0F", sidebarAccent: "#F7FEE7" },
            orange: { primary: "#EA580C", primaryForeground: "#FFF7ED", accent: "#FFF1E7", accentForeground: "#C2410C", sidebarAccent: "#FFF1E7" },
            pink: { primary: "#EC4899", primaryForeground: "#FDF2F8", accent: "#FDECF5", accentForeground: "#BE185D", sidebarAccent: "#FDECF5" },
            purple: { primary: "#9333EA", primaryForeground: "#FAF5FF", accent: "#F4ECFF", accentForeground: "#7E22CE", sidebarAccent: "#F4ECFF" },
            red: { primary: "#DC2626", primaryForeground: "#FEF2F2", accent: "#FEECEC", accentForeground: "#B91C1C", sidebarAccent: "#FEECEC" },
            rose: { primary: "#E11D48", primaryForeground: "#FFF1F2", accent: "#FFECEF", accentForeground: "#BE123C", sidebarAccent: "#FFECEF" },
            sky: { foreground: "#162033", primary: "#0A84FF", primaryForeground: "#F5FAFF", secondary: "#EEF4FA", muted: "#EEF4FA", mutedForeground: "#6B7280", accent: "#EAF4FF", accentForeground: "#0A84FF", border: "#DCE3EC", input: "#DCE3EC", sidebar: "#F7F9FC", sidebarAccent: "#EAF4FF", sidebarBorder: "#DCE3EC" },
            teal: { primary: "#0F766E", primaryForeground: "#F0FDFA", accent: "#E6FFFA", accentForeground: "#115E59", sidebarAccent: "#E6FFFA" },
            violet: { primary: "#7C3AED", primaryForeground: "#F5F3FF", accent: "#F0EAFE", accentForeground: "#6D28D9", sidebarAccent: "#F0EAFE" },
            yellow: { primary: "#EAB308", primaryForeground: "#422006", accent: "#FEF9C3", accentForeground: "#854D0E", sidebarAccent: "#FEF9C3" }
        }
    }

    function darkOverrides() {
        return {
            neutral: { foreground: "#FAFAFA", card: "#262626", primary: "#E5E5E5", primaryForeground: "#171717", secondary: "#404040", muted: "#404040", mutedForeground: "#A3A3A3", accent: "#404040", accentForeground: "#FAFAFA", border: "#3F3F46", input: "#3F3F46", sidebar: "#262626", sidebarAccent: "#3A3A3A", sidebarBorder: "#3F3F46" },
            stone: { background: "#292524", foreground: "#FAFAF9", card: "#44403C", primary: "#E7E5E4", primaryForeground: "#292524", secondary: "#57534E", muted: "#57534E", mutedForeground: "#A8A29E", accent: "#57534E", accentForeground: "#FAFAF9", border: "#57534E", input: "#57534E", sidebar: "#44403C", sidebarAccent: "#57534E", sidebarBorder: "#57534E" },
            zinc: {},
            mauve: { background: "#211B22", foreground: "#FAF7FB", card: "#2C2430", primary: "#EBDCF2", primaryForeground: "#211B22", secondary: "#403546", muted: "#403546", mutedForeground: "#B8A8BF", accent: "#403546", accentForeground: "#FAF7FB", border: "#413548", input: "#413548", sidebar: "#2C2430", sidebarAccent: "#403546", sidebarBorder: "#413548" },
            olive: { background: "#1E2319", foreground: "#F8FBF4", card: "#2A3124", primary: "#DCE7C8", primaryForeground: "#1E2319", secondary: "#384132", muted: "#384132", mutedForeground: "#B6C0A9", accent: "#384132", accentForeground: "#F8FBF4", border: "#3E4837", input: "#3E4837", sidebar: "#2A3124", sidebarAccent: "#384132", sidebarBorder: "#3E4837" },
            mist: { background: "#161B24", foreground: "#F5F7FB", card: "#202734", primary: "#7BA2FF", primaryForeground: "#0F1622", secondary: "#2A3240", muted: "#2A3240", mutedForeground: "#A8B0BC", accent: "#2A3240", accentForeground: "#F5F7FB", border: "#313A49", input: "#313A49", sidebar: "#1E2633", sidebarAccent: "#2A3240", sidebarBorder: "#313A49" },
            taupe: { background: "#221E1D", foreground: "#FBF8F7", card: "#302927", primary: "#E6DBD4", primaryForeground: "#221E1D", secondary: "#3B3431", muted: "#3B3431", mutedForeground: "#B8AEA9", accent: "#3B3431", accentForeground: "#FBF8F7", border: "#433B38", input: "#433B38", sidebar: "#302927", sidebarAccent: "#3B3431", sidebarBorder: "#433B38" },
            amber: { primary: "#F59E0B", primaryForeground: "#422006", accent: "#3B2A10", accentForeground: "#FFE7A3" },
            blue: { primary: "#3B82F6", primaryForeground: "#EFF6FF", accent: "#1F2E4C", accentForeground: "#CFE0FF" },
            cyan: { primary: "#06B6D4", primaryForeground: "#E0F7FA", accent: "#17333A", accentForeground: "#B9F4FF" },
            emerald: { primary: "#10B981", primaryForeground: "#ECFDF5", accent: "#18372A", accentForeground: "#BFF7DE" },
            fuchsia: { primary: "#D946EF", primaryForeground: "#FDF4FF", accent: "#37203D", accentForeground: "#F6C5FF" },
            green: { primary: "#22C55E", primaryForeground: "#F0FDF4", accent: "#163022", accentForeground: "#BDF9D1" },
            indigo: { primary: "#6366F1", primaryForeground: "#EEF2FF", accent: "#232642", accentForeground: "#D7D8FF" },
            lime: { primary: "#A3E635", primaryForeground: "#2B3A11", accent: "#2E3319", accentForeground: "#E9FDB8" },
            orange: { primary: "#F97316", primaryForeground: "#FFF7ED", accent: "#3B2516", accentForeground: "#FFD2B0" },
            pink: { primary: "#F472B6", primaryForeground: "#FFF1F2", accent: "#3B1F2E", accentForeground: "#FFD1E7" },
            purple: { primary: "#A855F7", primaryForeground: "#FAF5FF", accent: "#302041", accentForeground: "#E7D1FF" },
            red: { primary: "#EF4444", primaryForeground: "#FEF2F2", accent: "#3A2020", accentForeground: "#FFC5C5" },
            rose: { primary: "#F43F5E", primaryForeground: "#FFF1F2", accent: "#3A1F28", accentForeground: "#FFCFD8" },
            sky: { background: "#151B25", foreground: "#F5F7FB", card: "#1D2430", primary: "#5AA9FF", primaryForeground: "#0E1722", secondary: "#283241", muted: "#283241", mutedForeground: "#A4AEBB", accent: "#283241", accentForeground: "#F5F7FB", border: "#334055", input: "#334055", sidebar: "#1D2430", sidebarAccent: "#283241", sidebarBorder: "#334055" },
            teal: { primary: "#14B8A6", primaryForeground: "#E6FFFA", accent: "#183533", accentForeground: "#BDFBF4" },
            violet: { primary: "#8B5CF6", primaryForeground: "#F5F3FF", accent: "#2C2140", accentForeground: "#E1D3FF" },
            yellow: { primary: "#FACC15", primaryForeground: "#422006", accent: "#3A3113", accentForeground: "#FFF2A6" }
        }
    }

    function palette() {
        const base = root.darkMode ? baseDark() : baseLight()
        const overrides = root.darkMode ? darkOverrides() : lightOverrides()
        const theme = overrides[root.themeName] || overrides.sky
        return Object.assign({}, base, theme)
    }

    function styleTokens() {
        const styles = {
            vega: { radiusSmall: 8, radiusMedium: 12, radiusLarge: 16, spacingTiny: 4, spacingSmall: 8, spacingMedium: 12, spacingLarge: 16, spacingXLarge: 20 },
            nova: { radiusSmall: 7, radiusMedium: 11, radiusLarge: 14, spacingTiny: 3, spacingSmall: 6, spacingMedium: 10, spacingLarge: 14, spacingXLarge: 18 },
            maia: { radiusSmall: 10, radiusMedium: 14, radiusLarge: 20, spacingTiny: 5, spacingSmall: 10, spacingMedium: 14, spacingLarge: 18, spacingXLarge: 24 },
            lyra: { radiusSmall: 4, radiusMedium: 8, radiusLarge: 12, spacingTiny: 2, spacingSmall: 6, spacingMedium: 10, spacingLarge: 12, spacingXLarge: 16 },
            mira: { radiusSmall: 6, radiusMedium: 10, radiusLarge: 14, spacingTiny: 2, spacingSmall: 6, spacingMedium: 10, spacingLarge: 14, spacingXLarge: 18 },
            luma: { radiusSmall: 12, radiusMedium: 16, radiusLarge: 22, spacingTiny: 4, spacingSmall: 8, spacingMedium: 12, spacingLarge: 18, spacingXLarge: 24 }
        }
        return styles[root.styleName] || styles.mira
    }

    readonly property var _palette: palette()
    readonly property var _style: styleTokens()

    readonly property color accent: _palette.primary
    readonly property color primaryForeground: _palette.primaryForeground
    readonly property color accentWeak: _palette.accent
    readonly property color accentLight: _palette.sidebarAccent
    readonly property color accentForeground: _palette.accentForeground || _palette.primaryForeground
    readonly property color accentHover: _palette.primary
    readonly property color bgColor: _palette.background
    readonly property color background: _palette.background
    readonly property color bgSecondary: _palette.secondary
    readonly property color cardColor: _palette.card
    readonly property color navColor: _palette.sidebar
    readonly property color panelColor: darkMode ? Qt.darker(_palette.sidebar, 1.08) : Qt.lighter(_palette.sidebar, 1.02)
    readonly property color sectionColor: _palette.secondary
    readonly property color borderColor: _palette.border
    readonly property color muted: _palette.muted
    readonly property color surface: _palette.secondary
    readonly property color stroke: _palette.input
    readonly property color dividerColor: _palette.border
    readonly property color textPrimary: _palette.foreground
    readonly property color textSecondary: _palette.mutedForeground
    readonly property color textMuted: darkMode ? Qt.lighter(_palette.mutedForeground, 1.05) : _palette.mutedForeground

    readonly property color bgPopover: _palette.card
    readonly property color bgMuted: _palette.muted
    readonly property color mutedColor: _palette.muted
    readonly property color ringColor: _palette.primary

    readonly property color secondary: _palette.secondary
    readonly property color secondaryForeground: _palette.secondaryForeground

    readonly property color ok: darkMode ? "#3CCF7A" : "#1F9A55"
    readonly property color okColor: ok
    readonly property color okWeak: darkMode ? "#1A2A21" : "#E6F7EE"
    readonly property color okBg: okWeak
    readonly property color okForeground: "#FFFFFF"
    readonly property color danger: darkMode ? "#FF6B6B" : "#D64545"
    readonly property color dangerWeak: darkMode ? "#321B1B" : "#FCEBEC"
    readonly property color ngColor: danger
    readonly property color ngBg: dangerWeak
    readonly property color dangerForeground: "#FFFFFF"
    readonly property color warn: darkMode ? "#F2B94B" : "#A16207"
    readonly property color warnWeak: darkMode ? "#342A18" : "#FFF5DB"
    readonly property color warnColor: warn
    readonly property color warnBg: warnWeak
    readonly property color warnForeground: "#FFFFFF"

    readonly property int radiusSmall: _style.radiusSmall
    readonly property int radiusMedium: _style.radiusMedium
    readonly property int radiusLarge: _style.radiusLarge
    readonly property int spacingTiny: _style.spacingTiny
    readonly property int spacingSmall: _style.spacingSmall
    readonly property int spacingMedium: _style.spacingMedium
    readonly property int spacingLarge: _style.spacingLarge
    readonly property int spacingXLarge: _style.spacingXLarge
}
