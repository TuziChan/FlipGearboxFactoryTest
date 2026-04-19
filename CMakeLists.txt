cmake_minimum_required(VERSION 3.16)

project(FlipGearboxFactoryTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(CTest)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 SerialPort Test)

qt_standard_project_setup(REQUIRES 6.10)

# Main application executable
qt_add_executable(appFlipGearboxFactoryTest
    main.cpp
    assets/app.rc
    
    # Domain Layer
    src/domain/TelemetrySnapshot.h
    src/domain/FailureReason.h
    src/domain/TestRecipe.h
    src/domain/TestResults.h
    src/domain/TestRunState.h
    src/domain/GearboxTestEngine.h
    src/domain/GearboxTestEngine.cpp
    
    # Infrastructure - Bus
    src/infrastructure/bus/ModbusFrame.h
    src/infrastructure/bus/ModbusFrame.cpp
    src/infrastructure/bus/IBusController.h
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
    src/infrastructure/devices/SingleTurnEncoderDevice.h
    src/infrastructure/devices/SingleTurnEncoderDevice.cpp
    src/infrastructure/devices/BrakePowerSupplyDevice.h
    src/infrastructure/devices/BrakePowerSupplyDevice.cpp
    
    # Infrastructure - Simulation
    src/infrastructure/simulation/SimulationContext.h
    src/infrastructure/simulation/SimulatedBusController.h
    src/infrastructure/simulation/SimulatedMotorDevice.h
    src/infrastructure/simulation/SimulatedMotorDevice.cpp
    src/infrastructure/simulation/SimulatedTorqueDevice.h
    src/infrastructure/simulation/SimulatedTorqueDevice.cpp
    src/infrastructure/simulation/SimulatedEncoderDevice.h
    src/infrastructure/simulation/SimulatedEncoderDevice.cpp
    src/infrastructure/simulation/SimulatedBrakeDevice.h
    src/infrastructure/simulation/SimulatedBrakeDevice.cpp
    
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
    src/infrastructure/config/DeviceConfigService.h
    src/infrastructure/config/DeviceConfigService.cpp
    src/infrastructure/config/StationRuntime.h
    src/infrastructure/config/StationRuntime.cpp
    src/infrastructure/config/StationRuntimeFactory.h
    src/infrastructure/config/StationRuntimeFactory.cpp
    
    # Infrastructure - Services
    src/infrastructure/services/HistoryService.h
    src/infrastructure/services/HistoryService.cpp
    src/infrastructure/services/RecipeService.h
    src/infrastructure/services/RecipeService.cpp
    
    # ViewModels
    src/viewmodels/TestExecutionViewModel.h
    src/viewmodels/TestExecutionViewModel.cpp
    src/viewmodels/DiagnosticsViewModel.h
    src/viewmodels/DiagnosticsViewModel.cpp
    src/viewmodels/HistoryViewModel.h
    src/viewmodels/HistoryViewModel.cpp
    src/viewmodels/RecipeViewModel.h
    src/viewmodels/RecipeViewModel.cpp
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
        src/ui/components/ChartPanel.qml
        src/ui/components/DataTableCard.qml
        src/ui/components/VerdictPanel.qml
        src/ui/components/PlaceholderPage.qml
        src/ui/components/TestExecutionWorkspace.qml
        
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
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::SerialPort
)

# ============================================================================
# Test Executables
# ============================================================================

# QML Smoke Tests
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

# Test Execution Verification
qt_add_executable(TestExecutionVerification
    tests/TestExecutionVerification.cpp
    tests/mocks/MockDevices.h
    src/domain/GearboxTestEngine.h
    src/domain/GearboxTestEngine.cpp
    src/domain/TelemetrySnapshot.h
    src/domain/TestRunState.h
    src/domain/TestRecipe.h
    src/domain/TestResults.h
    src/domain/FailureReason.h
    src/infrastructure/config/StationConfig.h
    src/infrastructure/config/ConfigLoader.h
    src/infrastructure/config/ConfigLoader.cpp
    src/infrastructure/config/RecipeConfig.h
    src/infrastructure/config/RecipeConfig.cpp
    src/infrastructure/bus/IBusController.h
    src/infrastructure/bus/ModbusFrame.h
    src/infrastructure/bus/ModbusFrame.cpp
    src/infrastructure/devices/IMotorDriveDevice.h
    src/infrastructure/devices/ITorqueSensorDevice.h
    src/infrastructure/devices/IEncoderDevice.h
    src/infrastructure/devices/IBrakePowerDevice.h
    src/infrastructure/devices/BrakePowerSupplyDevice.h
    src/infrastructure/devices/BrakePowerSupplyDevice.cpp
    src/infrastructure/acquisition/TelemetryBuffer.h
    src/infrastructure/acquisition/DevicePoller.h
    src/infrastructure/acquisition/AcquisitionScheduler.h
    src/infrastructure/acquisition/AcquisitionScheduler.cpp
)

set_target_properties(TestExecutionVerification PROPERTIES
    WIN32_EXECUTABLE FALSE
)

target_include_directories(TestExecutionVerification PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(TestExecutionVerification
    PRIVATE
        Qt6::Core
        Qt6::Test
)

# Protocol Layer Tests
qt_add_executable(ProtocolLayerTests
    tests/ProtocolLayerTests.cpp
    tests/mocks/MockDevices.h
    src/infrastructure/bus/ModbusFrame.h
    src/infrastructure/bus/ModbusFrame.cpp
    src/infrastructure/bus/IBusController.h
    src/infrastructure/devices/IMotorDriveDevice.h
    src/infrastructure/devices/ITorqueSensorDevice.h
    src/infrastructure/devices/IEncoderDevice.h
    src/infrastructure/devices/IBrakePowerDevice.h
)

set_target_properties(ProtocolLayerTests PROPERTIES
    WIN32_EXECUTABLE FALSE
)

target_include_directories(ProtocolLayerTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ProtocolLayerTests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

# Brake Power Constant Voltage Test
qt_add_executable(BrakePowerConstantVoltageTest
    tests/devices/BrakePowerConstantVoltageTest.cpp
    tests/mocks/MockDevices.h
    src/infrastructure/devices/BrakePowerSupplyDevice.h
    src/infrastructure/devices/BrakePowerSupplyDevice.cpp
    src/infrastructure/devices/IBrakePowerDevice.h
    src/infrastructure/devices/IMotorDriveDevice.h
    src/infrastructure/devices/ITorqueSensorDevice.h
    src/infrastructure/devices/IEncoderDevice.h
    src/infrastructure/bus/ModbusFrame.h
    src/infrastructure/bus/ModbusFrame.cpp
    src/infrastructure/bus/IBusController.h
    src/domain/TestRecipe.h
    src/infrastructure/config/RecipeConfig.h
    src/infrastructure/config/RecipeConfig.cpp
)

set_target_properties(BrakePowerConstantVoltageTest PROPERTIES
    WIN32_EXECUTABLE FALSE
)

target_include_directories(BrakePowerConstantVoltageTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(BrakePowerConstantVoltageTest
    PRIVATE
        Qt6::Core
        Qt6::Test
)

# History ViewModel Tests
qt_add_executable(HistoryViewModelTests
    tests/viewmodels/HistoryViewModelTests.cpp
    src/viewmodels/HistoryViewModel.h
    src/viewmodels/HistoryViewModel.cpp
    src/infrastructure/services/HistoryService.h
    src/infrastructure/services/HistoryService.cpp
)

set_target_properties(HistoryViewModelTests PROPERTIES
    WIN32_EXECUTABLE FALSE
)

target_include_directories(HistoryViewModelTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(HistoryViewModelTests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

# Recipe ViewModel Tests
qt_add_executable(RecipeViewModelTests
    tests/viewmodels/RecipeViewModelTests.cpp
    src/viewmodels/RecipeViewModel.h
    src/viewmodels/RecipeViewModel.cpp
    src/infrastructure/services/RecipeService.h
    src/infrastructure/services/RecipeService.cpp
    src/infrastructure/config/RecipeConfig.h
    src/infrastructure/config/RecipeConfig.cpp
)

set_target_properties(RecipeViewModelTests PROPERTIES
    WIN32_EXECUTABLE FALSE
)

target_include_directories(RecipeViewModelTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(RecipeViewModelTests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

# ============================================================================
# CTest Configuration
# ============================================================================

qt_add_executable(DomainEngineTests
    tests/DomainEngineTests.cpp
    tests/mocks/MockDevices.h
    src/domain/GearboxTestEngine.h
    src/domain/GearboxTestEngine.cpp
    src/domain/TelemetrySnapshot.h
    src/domain/TestRunState.h
    src/domain/TestRecipe.h
    src/domain/TestResults.h
    src/domain/FailureReason.h
    src/infrastructure/devices/IMotorDriveDevice.h
    src/infrastructure/devices/ITorqueSensorDevice.h
    src/infrastructure/devices/IEncoderDevice.h
    src/infrastructure/devices/IBrakePowerDevice.h
    src/infrastructure/acquisition/TelemetryBuffer.h
    src/infrastructure/acquisition/DevicePoller.h
    src/infrastructure/acquisition/AcquisitionScheduler.h
    src/infrastructure/acquisition/AcquisitionScheduler.cpp
)

set_target_properties(DomainEngineTests PROPERTIES
    WIN32_EXECUTABLE FALSE
)

target_include_directories(DomainEngineTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(DomainEngineTests
    PRIVATE
        Qt6::Core
        Qt6::Test
)

set(QT_TEST_PATH "F:/Qt/6.11.0/mingw_64/bin;F:/Qt/Tools/mingw1310_64/bin;$ENV{PATH}")

add_test(
    NAME TestExecutionVerification
    COMMAND
        ${CMAKE_COMMAND} -E env
        "QT_QPA_PLATFORM=offscreen"
        "PATH=${QT_TEST_PATH}"
        $<TARGET_FILE:TestExecutionVerification> -o - -txt
)

add_test(
    NAME ProtocolLayerTests
    COMMAND
        ${CMAKE_COMMAND} -E env
        "QT_QPA_PLATFORM=offscreen"
        "PATH=${QT_TEST_PATH}"
        $<TARGET_FILE:ProtocolLayerTests> -o - -txt
)

add_test(
    NAME BrakePowerConstantVoltageTest
    COMMAND
        ${CMAKE_COMMAND} -E env
        "QT_QPA_PLATFORM=offscreen"
        "PATH=${QT_TEST_PATH}"
        $<TARGET_FILE:BrakePowerConstantVoltageTest> -o - -txt
)

add_test(
    NAME QmlSmokeTests
    COMMAND
        ${CMAKE_COMMAND} -E env
        "QT_QPA_PLATFORM=offscreen"
        "QT_QUICK_CONTROLS_STYLE=Basic"
        "PATH=${QT_TEST_PATH}"
        $<TARGET_FILE:QmlSmokeTests> -o - -txt
)

add_test(
    NAME HistoryViewModelTests
    COMMAND
        ${CMAKE_COMMAND} -E env
        "QT_QPA_PLATFORM=offscreen"
        "PATH=${QT_TEST_PATH}"
        $<TARGET_FILE:HistoryViewModelTests> -o - -txt
)

add_test(
    NAME RecipeViewModelTests
    COMMAND
        ${CMAKE_COMMAND} -E env
        "QT_QPA_PLATFORM=offscreen"
        "PATH=${QT_TEST_PATH}"
        $<TARGET_FILE:RecipeViewModelTests> -o - -txt
)

add_test(
    NAME DomainEngineTests
    COMMAND
        ${CMAKE_COMMAND} -E env
        "QT_QPA_PLATFORM=offscreen"
        "PATH=${QT_TEST_PATH}"
        $<TARGET_FILE:DomainEngineTests> -o - -txt
)

# ============================================================================
# Installation
# ============================================================================

install(TARGETS appFlipGearboxFactoryTest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
