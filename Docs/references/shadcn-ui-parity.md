# shadcn-ui QML Parity Tracker

Source of truth:

- [shadcn-ui repo](/F:/Work/FlipGearboxFactoryTest/Docs/references/shadcn-ui)
- Source directory: `apps/v4/registry/new-york-v4/ui`

## Principles

- Components are ported against source behavior, not screenshots.
- Primitive and high-frequency components go first.
- QML ports keep shadcn-style concepts:
  - `variant`
  - `size`
  - `state`
  - theme/style driven appearance
- Components remain Qt-native. React/Radix implementation details are translated, not copied.

## Current Status

### In Progress / Partially Ported

- `button` -> `AppButton.qml`
- `badge` -> `AppBadge.qml`
- `alert` -> `AppAlert.qml`
- `alert-dialog` -> `AppAlertDialog.qml`
- `accordion` -> `AppAccordion.qml`
- `aspect-ratio` -> `AppAspectRatio.qml`
- `avatar` -> `AppAvatar.qml`
- `checkbox` -> `AppCheckbox.qml`
- `breadcrumb` -> `AppBreadcrumb.qml`
- `button-group` -> `AppButtonGroup.qml`
- `calendar` -> `AppCalendar.qml`
- `carousel` -> `AppCarousel.qml`
- `collapsible` -> `AppCollapsible.qml`
- `drawer` -> `AppDrawer.qml`
- `input` -> `AppInput.qml`
- `context-menu` -> `AppContextMenu.qml`
- `field` -> `AppField.qml`
- `form` -> `AppForm.qml`
- `hover-card` -> `AppHoverCard.qml`
- `menubar` -> `AppMenubar.qml`
- `input-group` -> `AppInputGroup.qml`
- `input-otp` -> `AppInputOtp.qml`
- `native-select` -> `AppNativeSelect.qml`
- `navigation-menu` -> `AppNavigationMenu.qml`
- `pagination` -> `AppPagination.qml`
- `radio-group` -> `AppRadioGroup.qml`
- `resizable` -> `AppResizable.qml`
- `scroll-area` -> `AppScrollArea.qml`
- `select` -> `AppSelect.qml`
- `dialog` -> `AppDialog.qml`
- `sheet` -> `AppSheet.qml`
- `slider` -> `AppSlider.qml`
- `popover` -> `AppPopover.qml`
- `separator` -> `AppSeparator.qml`
- `table` -> `AppTable.qml`
- `toggle` -> `AppToggle.qml`
- `toggle-group` -> `AppToggleGroup.qml`
- `card` -> `SectionCard.qml`
- `sidebar` -> `SidebarNav.qml`, `SidebarNavItem.qml`
- `business table` -> `DataTableCard.qml` (legacy business-specific wrapper over tabular content)

### Ported Utility/Theme Layer

- Theme registry support -> `AppTheme.qml`
- Runtime theme/style switcher -> `ThemeSwitcherPanel.qml`

### Next High-Priority Ports

- `dropdown-menu` depth pass
- `combobox` depth pass
- `command` depth pass
- `calendar` depth pass
- `carousel` depth pass
- `resizable` depth pass
- `table` depth pass
- `slider` depth pass
- `navigation-menu` depth pass
- `drawer` depth pass
- `hover-card` depth pass

## Registry Inventory

- accordion
- alert-dialog
- alert
- aspect-ratio
- avatar
- badge
- breadcrumb
- button-group
- button
- calendar
- card
- carousel
- chart
- checkbox
- collapsible
- combobox
- command
- context-menu
- dialog
- direction
- drawer
- dropdown-menu
- empty
- field
- form
- hover-card
- input-group
- input-otp
- input
- item
- kbd
- label
- menubar
- native-select
- navigation-menu
- pagination
- popover
- progress
- radio-group
- resizable
- scroll-area
- select
- separator
- sheet
- sidebar
- skeleton
- slider
- sonner
- spinner
- switch
- table
- tabs
- textarea
- toggle-group
- toggle
- tooltip
