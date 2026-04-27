cmake_minimum_required(VERSION 3.16)

project(FlipGearboxFactoryTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(CTest)
enable_testing()

# CTest Configuration
set(CTEST_PARALLEL_LEVEL 8 CACHE STRING "Number of parallel test jobs")
set(DART_TESTING_TIMEOUT 300 CACHE STRING "Test timeout in seconds")
set(CTEST_OUTPUT_ON_FAILURE ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Charts SerialPort Test Qml Widgets)

qt_standard_project_setup(REQUIRES 6.10)

get_filename_component(QT6_INSTALL_PREFIX "${Qt6_DIR}/../../.." ABSOLUTE)
set(QT6_RUNTIME_QML_DIR "${QT6_INSTALL_PREFIX}/qml")
set(QT6_RUNTIME_PLUGINS_DIR "${QT6_INSTALL_PREFIX}/plugins")

# ============================================================================
# Layer 1: Infrastructure Library (Base Layer - No Dependencies)
# ============================================================================

qt_add_library(FlipGearboxInfra STATIC
    # Infrastructure - Bus
    src/infrastructure/bus/ModbusFrame.h
    src/infrastructure/bus/ModbusFrame.cpp
    src/infrastructure/bus/IBusController.h
    src/infrastructure/bus/IBusController.cpp
    src/infrastructure/bus/ModbusRtuBusController.h
    src/infrastructure/bus/ModbusRtuBusController.cpp

    # Infrastructure - Devices (Interfaces)
    src/infrastructure/devices/IMotorDriveDevice.h
    src/infrastructure/devices/ITorqueSensorDevice.h
    src/infrastructure/devices/IEncoderDevice.h
    src/infrastructure/devices/IBrakePowerDevice.h

    # Infrastructure - Devices (Real Hardware)
    src/infrastructure/devices/AqmdMotorDriveDevice.h
    src/infrastructure/devices/AqmdMotorDriveDevice.cpp
    src/infrastructure/devices/Dyn200TorqueSensorDevice.h
    src/infrastructure/devices/Dyn200ProactiveListener.h
    src/infrastructure/devices/Dyn200ProactiveListener.cpp
    src/infrastructure/devices/Dyn200TorqueSensorDevice.cpp
    src/infrastructure/devices/EncoderProactiveListener.h
    src/infrastructure/devices/EncoderProactiveListener.cpp
    src/infrastructure/devices/SingleTurnEncoderDevice.h
    src/infrastructure/devices/SingleTurnEncoderDevice.cpp
    src/infrastructure/devices/BrakePowerSupplyDevice.h
    src/infrastructure/devices/BrakePowerSupplyDevice.cpp

    # Infrastructure - Testing
    src/infrastructure/testing/TestOrchestrator.h
    src/infrastructure/testing/TestOrchestrator.cpp
    src/infrastructure/testing/TeamExecutionMonitor.h
    src/infrastructure/testing/TeamExecutionMonitor.cpp
    src/infrastructure/testing/BuildPipelineRunner.h
    src/infrastructure/testing/BuildPipelineRunner.cpp

    # Infrastructure - Acquisition
    src/infrastructure/acquisition/TelemetryBuffer.h
    src/infrastructure/acquisition/DevicePoller.h
    src/infrastructure/acquisition/AcquisitionScheduler.h
    src/infrastructure/acquisition/AcquisitionScheduler.cpp

    # Infrastructure - Config
    src/infrastructure/config/AppConfig.h
    src/infrastructure/config/StationConfig.h
    src/infrastructure/config/RecipeConfig.h
    src/infrastructure/config/RecipeConfig.cpp
    src/infrastructure/config/ConfigLoader.h
    src/infrastructure/config/ConfigLoader.cpp
    src/infrastructure/config/StationConfigValidator.h
    src/infrastructure/config/StationConfigValidator.cpp
    src/infrastructure/config/DeviceConfigService.h
    src/infrastructure/config/DeviceConfigService.cpp
    src/infrastructure/config/StationRuntime.h
    src/infrastructure/config/StationRuntime.cpp
    src/infrastructure/config/StationRuntimeFactory.h
    src/infrastructure/config/StationRuntimeFactory.cpp
    src/infrastructure/config/RuntimeManager.h
    src/infrastructure/config/RuntimeManager.cpp

    # Infrastructure - Logging
    src/infrastructure/logging/LogManager.h
    src/infrastructure/logging/LogManager.cpp

    # Infrastructure - Monitoring
    src/infrastructure/monitoring/PerformanceMonitor.h
    src/infrastructure/monitoring/PerformanceMonitor.cpp

    # Infrastructure - Validation
    src/infrastructure/validation/PhysicsValidator.h
    src/infrastructure/validation/PhysicsValidator.cpp
    src/infrastructure/validation/PhysicsViolationLogger.h
    src/infrastructure/validation/PhysicsViolationLogger.cpp

    # Infrastructure - Reporting
    src/infrastructure/reporting/ReportGenerator.h
    src/infrastructure/reporting/ReportGenerator.cpp
    src/infrastructure/reporting/JsonReportWriter.h
    src/infrastructure/reporting/JsonReportWriter.cpp
    src/infrastructure/reporting/TestReportGenerator.h
    src/infrastructure/reporting/TestReportGenerator.cpp

    # Infrastructure - Services
    src/infrastructure/services/HistoryService.h
    src/infrastructure/services/HistoryService.cpp
    src/infrastructure/services/RecipeService.h
    src/infrastructure/services/RecipeService.cpp
)

target_include_directories(FlipGearboxInfra
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FlipGearboxInfra
    PUBLIC
        Qt6::Core
        Qt6::SerialPort
)

# ============================================================================
# Layer 2: Domain Library (Depends on Infrastructure)
# ============================================================================

qt_add_library(FlipGearboxDomain STATIC
    # Domain Layer
    src/domain/TelemetrySnapshot.h
    src/domain/FailureReason.h
    src/domain/TestRecipe.h
    src/domain/TestResults.h
    src/domain/TestRunState.h
    src/domain/RecipeValidator.h
    src/domain/RecipeValidator.cpp
    src/domain/GearboxTestEngine.h
    src/domain/GearboxTestEngine.cpp
)

target_include_directories(FlipGearboxDomain
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FlipGearboxDomain
    PUBLIC
        FlipGearboxInfra
        Qt6::Core
)

# ============================================================================
# Layer 3: UI Library (Depends on Domain)
# ============================================================================

qt_add_library(FlipGearboxUI STATIC
    # TeamOps - AI Team Monitoring
    src/teamops/TeamRoleStatus.h
    src/teamops/TeamRoleStatus.cpp
    src/teamops/ITeamDataProvider.h
    src/teamops/ITeamDataProvider.cpp

    src/teamops/FileTeamDataProvider.h
    src/teamops/FileTeamDataProvider.cpp
    src/teamops/TeamMonitorService.h
    src/teamops/TeamMonitorService.cpp
    src/teamops/TeamOpsViewModel.h
    src/teamops/TeamOpsViewModel.cpp

    # ViewModels
    src/viewmodels/TestExecutionViewModel.h
    src/viewmodels/TestExecutionViewModel.cpp
    src/viewmodels/DiagnosticsViewModel.h
    src/viewmodels/DiagnosticsViewModel.cpp
    src/viewmodels/HistoryViewModel.h
    src/viewmodels/HistoryViewModel.cpp
    src/viewmodels/RecipeViewModel.h
    src/viewmodels/RecipeViewModel.cpp

    # UI Components (C++)
    # Chart components moved to QML
)

target_include_directories(FlipGearboxUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FlipGearboxUI
    PUBLIC
        FlipGearboxDomain
        Qt6::Core
        Qt6::Quick
        Qt6::Charts
)

# ============================================================================
# Legacy Compatibility: FlipGearboxTestLib (Transitional)
# ============================================================================

qt_add_library(FlipGearboxTestLib STATIC
    tests/framework/AutoTestFramework.cpp
    tests/framework/AutoTestFramework.h

)

target_link_libraries(FlipGearboxTestLib
    PUBLIC
        FlipGearboxUI
        Qt6::Test
)

# ============================================================================
# Main Application Executable
# ============================================================================

qt_add_executable(appFlipGearboxFactoryTest
    main.cpp
)

# QML module for the application
qt_add_qml_module(appFlipGearboxFactoryTest
    URI FlipGearboxFactoryTest
    QML_FILES
        Main.qml
        
        # UI Components - Base
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
        
        # UI Components - Card
        src/ui/components/AppCard.qml
        src/ui/components/AppCardHeader.qml
        src/ui/components/AppCardTitle.qml
        src/ui/components/AppCardDescription.qml
        src/ui/components/AppCardAction.qml
        src/ui/components/AppCardContent.qml
        src/ui/components/AppCardFooter.qml
        
        # UI Components - Advanced
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
        
        # UI Components - Domain Specific
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
        src/ui/components/MultiYAxisChartView_QML.qml
        src/ui/components/ChartPanel.qml
        src/ui/components/DataTableCard.qml
        src/ui/components/VerdictPanel.qml
        src/ui/components/PlaceholderPage.qml

        src/ui/components/TestExecutionWorkspace.qml
        src/ui/components/PhysicsValidationPanel.qml
        
        # UI Pages
        src/ui/pages/AppShell.qml
        src/ui/pages/TestExecutionPage.qml
        src/ui/pages/RecipePage.qml
        src/ui/pages/HistoryPage.qml
        src/ui/pages/DeviceConfigPage.qml
        src/ui/pages/DiagnosticsPage.qml
        src/ui/pages/ComponentGalleryPage.qml
        
        # UI Pages - Recipe Forms
        src/ui/pages/recipe/RecipeBasicInfoForm.qml
        src/ui/pages/recipe/RecipeHomingForm.qml
        src/ui/pages/recipe/RecipeIdleForm.qml
        src/ui/pages/recipe/RecipeAngleForm.qml
        src/ui/pages/recipe/RecipeLoadForm.qml
        src/ui/pages/recipe/RecipeImpactForm.qml
)

# Copy fonts to build directory
set(FONT_FILES
    ${CMAKE_SOURCE_DIR}/assets/fonts/HarmonyOS_Sans_SC_Regular.ttf
    ${CMAKE_SOURCE_DIR}/assets/fonts/HarmonyOS_Sans_SC_Medium.ttf
    ${CMAKE_SOURCE_DIR}/assets/fonts/HarmonyOS_Sans_SC_Bold.ttf
    ${CMAKE_SOURCE_DIR}/assets/fonts/HarmonyOS_Sans_SC_Light.ttf
)

foreach(FONT_FILE ${FONT_FILES})
    get_filename_component(FONT_NAME ${FONT_FILE} NAME)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/fonts/${FONT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${FONT_FILE}
                ${CMAKE_BINARY_DIR}/fonts/${FONT_NAME}
        DEPENDS ${FONT_FILE}
        COMMENT "Copying font ${FONT_NAME}"
    )
    list(APPEND FONT_OUTPUTS ${CMAKE_BINARY_DIR}/fonts/${FONT_NAME})
endforeach()

add_custom_target(copy_fonts ALL DEPENDS ${FONT_OUTPUTS})
add_dependencies(appFlipGearboxFactoryTest copy_fonts)

# Application properties
set_target_properties(appFlipGearboxFactoryTest PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Link libraries for main application
target_link_libraries(appFlipGearboxFactoryTest
    PRIVATE
        FlipGearboxTestLib
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Charts
)

if(WIN32)
    target_link_libraries(appFlipGearboxFactoryTest PRIVATE dbghelp)
endif()

# ============================================================================
# Individual Test Executables
# ============================================================================

# Unified Test Runner (standalone executable with main())
qt_add_executable(UnifiedTestRunner tests/framework/UnifiedTestRunner.cpp)
set_target_properties(UnifiedTestRunner PROPERTIES WIN32_EXECUTABLE FALSE)
target_include_directories(UnifiedTestRunner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(UnifiedTestRunner PRIVATE FlipGearboxUI Qt6::Core)

# Encoder Stability Test (manual test tool)
qt_add_executable(EncoderStabilityTest test_encoder_stability.cpp)
set_target_properties(EncoderStabilityTest PROPERTIES WIN32_EXECUTABLE FALSE)
target_include_directories(EncoderStabilityTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(EncoderStabilityTest PRIVATE FlipGearboxUI Qt6::Core Qt6::SerialPort)

# Helper macro to create test executables
macro(add_gearbox_test TEST_NAME TEST_SOURCE)
    qt_add_executable(${TEST_NAME} ${TEST_SOURCE})
    set_target_properties(${TEST_NAME} PROPERTIES WIN32_EXECUTABLE FALSE)
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(${TEST_NAME} PRIVATE TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(${TEST_NAME} PRIVATE FlipGearboxTestLib Qt6::Test)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES ENVIRONMENT
        "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${QT6_RUNTIME_PLUGINS_DIR};QML2_IMPORT_PATH=${QT6_RUNTIME_QML_DIR};QML_IMPORT_PATH=${QT6_RUNTIME_QML_DIR};QT_QUICK_CONTROLS_STYLE=Basic")
endmacro()

# Protocol Layer Tests
add_gearbox_test(ModbusCrcTests tests/protocol/ModbusCrcTests.cpp)
set_tests_properties(ModbusCrcTests PROPERTIES LABELS "protocol;unit")

# Device Layer Tests
add_gearbox_test(BoundaryProtectionTests tests/BoundaryProtectionTests.cpp)
set_tests_properties(BoundaryProtectionTests PROPERTIES LABELS "device;integration")

# Domain Engine Tests

# ViewModel Tests
add_gearbox_test(HistoryViewModelTests tests/viewmodels/HistoryViewModelTests.cpp)
set_tests_properties(HistoryViewModelTests PROPERTIES LABELS "viewmodel;integration")

add_gearbox_test(RecipeViewModelTests tests/viewmodels/RecipeViewModelTests.cpp)
set_tests_properties(RecipeViewModelTests PROPERTIES LABELS "viewmodel;integration")

# UI Tests
add_gearbox_test(QmlSmokeTests tests/ui/QmlSmokeTests.cpp)
set_tests_properties(QmlSmokeTests PROPERTIES LABELS "ui;integration")

# Stability Tests

# Framework Tests
add_gearbox_test(AutoTestFrameworkTests tests/framework/AutoTestFrameworkTests.cpp)
set_tests_properties(AutoTestFrameworkTests PROPERTIES LABELS "infrastructure;unit")

add_gearbox_test(PerformanceMonitorTests tests/infrastructure/PerformanceMonitorTests.cpp)
set_tests_properties(PerformanceMonitorTests PROPERTIES LABELS "infrastructure;unit")

add_gearbox_test(TestReportGeneratorTests tests/infrastructure/TestReportGeneratorTests.cpp)
set_tests_properties(TestReportGeneratorTests PROPERTIES LABELS "infrastructure;unit")

# ============================================================================
# Installation
# ============================================================================

install(TARGETS appFlipGearboxFactoryTest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install runtime resources (config, data, fonts, QML module) alongside the binary
# so that relative paths like ../config/station.json resolve correctly
# from install/bin/ to install/config/
install(DIRECTORY config/ DESTINATION config)
install(DIRECTORY data/ DESTINATION data)
install(DIRECTORY ${CMAKE_BINARY_DIR}/fonts/ DESTINATION fonts)
install(DIRECTORY ${CMAKE_BINARY_DIR}/FlipGearboxFactoryTest/ DESTINATION qml/FlipGearboxFactoryTest)

if(WIN32)
    qt_generate_deploy_qml_app_script(
        TARGET appFlipGearboxFactoryTest
        OUTPUT_SCRIPT appFlipGearboxFactoryTest_deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${appFlipGearboxFactoryTest_deploy_script})
endif()
