cmake_minimum_required(VERSION 3.16)

project(FlipGearboxFactoryTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(CTest)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Test)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(appFlipGearboxFactoryTest
    main.cpp
)

qt_add_qml_module(appFlipGearboxFactoryTest
    URI FlipGearboxFactoryTest
    QML_FILES
        Main.qml
        src/ui/components/AppTheme.qml
        src/ui/components/AppIcon.qml
        src/ui/components/AppButton.qml
        src/ui/components/AppInput.qml
        src/ui/components/AppSelect.qml
        src/ui/components/AppBadge.qml
        src/ui/components/AppDialog.qml
        src/ui/components/AppSheet.qml
        src/ui/components/AppPopover.qml
        src/ui/components/AppDropdownMenu.qml
        src/ui/components/AppTooltip.qml
        src/ui/components/AppTabs.qml
        src/ui/components/AppCombobox.qml
        src/ui/components/AppCommand.qml
        src/ui/components/AppAlert.qml
        src/ui/components/AppAlertDialog.qml
        src/ui/components/AppAccordion.qml
        src/ui/components/AppAspectRatio.qml
        src/ui/components/AppAvatar.qml
        src/ui/components/AppAvatarGroup.qml
        src/ui/components/AppAvatarGroupCount.qml
        src/ui/components/AppCheckbox.qml
        src/ui/components/AppCalendar.qml
        src/ui/components/AppCarousel.qml
        src/ui/components/AppBreadcrumb.qml
        src/ui/components/AppButtonGroup.qml
        src/ui/components/AppCollapsible.qml
        src/ui/components/AppContextMenu.qml
        src/ui/components/AppField.qml
        src/ui/components/AppForm.qml
        src/ui/components/AppLabel.qml
        src/ui/components/AppMenubar.qml
        src/ui/components/AppNativeSelect.qml
        src/ui/components/AppNavigationMenu.qml
        src/ui/components/AppHoverCard.qml
        src/ui/components/AppDrawer.qml
        src/ui/components/AppInputGroup.qml
        src/ui/components/AppInputOtp.qml
        src/ui/components/AppPagination.qml
        src/ui/components/AppScrollArea.qml
        src/ui/components/AppResizable.qml
        src/ui/components/AppSlider.qml
        src/ui/components/AppTable.qml
        src/ui/components/AppTextarea.qml
        src/ui/components/AppSwitch.qml
        src/ui/components/AppProgress.qml
        src/ui/components/AppRadioGroup.qml
        src/ui/components/AppSkeleton.qml
        src/ui/components/AppToggle.qml
        src/ui/components/AppToggleGroup.qml
        src/ui/components/AppKbd.qml
        src/ui/components/AppSeparator.qml
        src/ui/components/SectionCard.qml
        src/ui/components/TopToolbar.qml
        src/ui/components/SidebarNav.qml
        src/ui/components/SidebarNavItem.qml
        src/ui/components/FlowList.qml
        src/ui/components/FlowStepItem.qml
        src/ui/components/InspectorSection.qml
        src/ui/components/VerdictBadge.qml
        src/ui/components/StatusList.qml
        src/ui/components/AlertStrip.qml
        src/ui/components/BottomStatusBar.qml
        src/ui/components/ThemeSwitcherPanel.qml
        src/ui/components/SvgIcon.qml
        src/ui/components/CommandBar.qml
        src/ui/components/MetricCard.qml
        src/ui/components/ChartPanel.qml
        src/ui/components/DataTableCard.qml
        src/ui/components/VerdictPanel.qml
        src/ui/components/PlaceholderPage.qml
        src/ui/components/TestExecutionWorkspace.qml
        src/ui/pages/RecipePage.qml
        src/ui/pages/HistoryPage.qml
        src/ui/pages/DeviceConfigPage.qml
        src/ui/pages/DiagnosticsPage.qml
        src/ui/pages/AppShell.qml
        src/ui/pages/ComponentGalleryPage.qml
        src/ui/pages/TestExecutionPage.qml
)

set_target_properties(appFlipGearboxFactoryTest PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appFlipGearboxFactoryTest
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
)

qt_add_executable(QmlSmokeTests
    tests/ui/QmlSmokeTests.cpp
)

set_target_properties(QmlSmokeTests PROPERTIES
    WIN32_EXECUTABLE FALSE
)

target_link_libraries(QmlSmokeTests
    PRIVATE
        Qt6::Quick
        Qt6::Test
)

target_compile_definitions(QmlSmokeTests
    PRIVATE
        TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

set(QML_SMOKE_TEST_PATH "F:/Qt/6.11.0/mingw_64/bin;F:/Qt/Tools/mingw1310_64/bin;$ENV{PATH}")
add_test(
    NAME QmlSmokeTests
    COMMAND
        ${CMAKE_COMMAND} -E env
        "QT_QPA_PLATFORM=offscreen"
        "QT_QUICK_CONTROLS_STYLE=Basic"
        "PATH=${QML_SMOKE_TEST_PATH}"
        $<TARGET_FILE:QmlSmokeTests> -o - -txt
)

install(TARGETS appFlipGearboxFactoryTest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
