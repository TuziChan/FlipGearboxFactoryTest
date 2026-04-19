# I/O 诊断页重新设计

This ExecPlan is a living document. The sections Progress, Surprises & Discoveries, Decision Log, and Outcomes & Retrospective must be kept up to date as work proceeds.

This document follows the requirements in AGENTS.md located at F:\Work\FlipGearboxFactoryTest\AGENTS.md.


## Purpose / Big Picture

The I/O diagnostics page currently has usability problems: device status and controls are separated, manual controls are disorganized, the communication log wastes space when empty, and real-time telemetry values are buried in text summaries. After this redesign, users will be able to:

1. View all devices at a glance in a structured overview table
2. Navigate to dedicated tabs for each device to see large, readable telemetry values and device-specific controls
3. Access the communication log only when needed via a collapsible panel at the bottom
4. See structured numeric telemetry data (current, voltage, torque, speed, angle) in dedicated display cards instead of parsing text summaries

To verify success, start the application, navigate to the I/O diagnostics page, and observe: a tab bar with "设备总览", "AQMD 电机驱动器", "DYN200 扭矩传感器", "单圈绝对值编码器", and "制动电源" tabs. Click each tab to see device-specific telemetry cards and controls. The communication log should appear as a collapsible section at the bottom, collapsed by default.


## Progress

- [x] (2026-04-18 14:45) Read and understand current DiagnosticsViewModel implementation
- [x] (2026-04-18 14:45) Add structured telemetry properties to DiagnosticsViewModel.h (already completed)
- [x] (2026-04-18 14:45) Implement telemetry property updates in DiagnosticsViewModel.cpp (already completed)
- [x] (2026-04-18 14:45) Verify all required UI components exist and meet requirements
- [x] (2026-04-18 19:00) Create new tab-based layout in DiagnosticsPage.qml
- [x] (2026-04-18 19:00) Implement Tab 0: Device overview table
- [x] (2026-04-18 19:00) Implement Tab 1: AQMD motor driver detail page
- [x] (2026-04-18 19:00) Implement Tab 2: DYN200 torque sensor detail page
- [x] (2026-04-18 19:00) Implement Tab 3: Encoder detail page
- [x] (2026-04-18 19:00) Implement Tab 4: Brake power supply detail page
- [x] (2026-04-18 19:00) Move communication log to collapsible bottom panel
- [x] (2026-04-18 19:00) Build succeeds, application launches
- [ ] Test all tabs and verify telemetry display
- [ ] Test all control buttons in device tabs
- [ ] Verify communication log collapsible behavior

## Surprises & Discoveries

- Observation: ViewModel telemetry properties were already implemented in a previous session
  Evidence: DiagnosticsViewModel.h contains all 4 Q_PROPERTY declarations (motorTelemetry, torqueTelemetry, encoderTelemetry, brakeTelemetry) and DiagnosticsViewModel.cpp has complete update logic in all buildXxxStatus() methods

- Observation: All required UI components already exist with suitable interfaces
  Evidence: AppTabs supports showContent:false for manual content management, AppTable supports row variants (success/danger/warning) for color coding, AppCollapsible has open property and triggerContent slot for custom header actions

- Observation: Current page uses fixed 480px width for communication log, wasting space when empty
  Evidence: DiagnosticsPage.qml line 308: Layout.preferredWidth: 480

- Observation: MetricCard component is ideal for telemetry display cards
  Evidence: MetricCard.qml provides label/value/unit/subtext layout with accentColor, matching the planned telemetry card design perfectly

- Observation: AppCollapsible uses `default property alias content` pattern, so child items placed inside are automatically added to its contentColumn
  Evidence: AppCollapsible.qml line `default property alias content: contentColumn.data`

- Observation: AppCollapsible has `triggerContent` alias for adding custom items to the header row (used for the "清空" button)
  Evidence: AppCollapsible.qml `property alias triggerContent: triggerContentRow.data`

## Decision Log

- Decision: No UI component modifications needed
  Rationale: After reviewing AppTabs, AppTable, and AppCollapsible components, all required features are already available. AppTabs can manage content manually with showContent:false, AppTable supports row variants for color coding, and AppCollapsible provides the collapsible panel functionality we need.
  Date/Author: 2026-04-18 14:45 / AI Agent

- Decision: Skip ViewModel modification steps since already completed
  Rationale: Steps 1 and 2 of the plan (adding telemetry properties to ViewModel) were already completed in a previous session. All 4 telemetry properties are present and functional.
  Date/Author: 2026-04-18 14:45 / AI Agent

- Decision: Focus implementation on DiagnosticsPage.qml rewrite
  Rationale: The only remaining work is restructuring the QML page to use tabs and reorganize the layout. No C++ or component changes needed.
  Date/Author: 2026-04-18 14:45 / AI Agent

- Decision: Use MetricCard for telemetry value display instead of custom AppCard-based cards
  Rationale: MetricCard already provides exactly the label/value/unit/subtext layout with accentColor that the plan calls for. Using it avoids writing custom card templates and ensures visual consistency.
  Date/Author: 2026-04-18 19:00 / AI Agent

- Decision: Use StackLayout + Loader with sourceComponent for tab content instead of inline Loader+source
  Rationale: StackLayout provides clean currentIndex-based switching synchronized with AppTabs. Loader with sourceComponent keeps tab content as Component definitions in the same file, avoiding separate QML file creation while still supporting lazy loading.
  Date/Author: 2026-04-18 19:00 / AI Agent

## Outcomes & Retrospective

### What was done
- Completely rewrote DiagnosticsPage.qml from a two-column layout (device cards + manual controls on left, fixed-width communication log on right) to a tab-based layout
- Tab 0 "设备总览": AppTable showing all 4 devices with name, status, telemetry summary, last update, error count; rows color-coded green/red by online/offline status
- Tab 1 "AQMD 电机驱动器": Device info card + 2 MetricCards (电流, AI1 电平) + motor control buttons (正转/反转/停止 with confirmation dialog)
- Tab 2 "DYN200 扭矩传感器": Device info card + 3 MetricCards (扭矩, 转速, 功率) + read-only alert
- Tab 3 "单圈绝对值编码器": Device info card + 1 MetricCard (角度) + calibration button (置零)
- Tab 4 "制动电源": Device info card + 5 MetricCards (电流, 电压, 功率, 工作模式, 通道) + output enable/disable buttons + current/voltage parameter input fields
- Communication log moved to collapsible AppCollapsible panel at bottom, collapsed by default, with "清空" button in header
- Application builds and launches successfully

### Files modified
- `src/ui/pages/DiagnosticsPage.qml` — complete rewrite (from ~370 lines to ~560 lines)

### Remaining verification
- Manual testing of tab navigation and telemetry display with connected hardware
- Verification of control button functionality (motor forward/reverse/stop, brake output/current/voltage, encoder zero)
- Verification of communication log expand/collapse behavior


## Context and Orientation

The I/O diagnostics page is located at src/ui/pages/DiagnosticsPage.qml. It displays the status of four industrial devices:

1. AQMD 电机驱动器 (AQMD Motor Driver) - controls a motor with forward/reverse/stop commands
2. DYN200 扭矩传感器 (DYN200 Torque Sensor) - read-only sensor measuring torque, speed, and power
3. 单圈绝对值编码器 (Single-turn Absolute Encoder) - measures angle and has a zero-point calibration command
4. 制动电源 (Brake Power Supply) - programmable power supply with enable/disable, current, and voltage controls

The page is backed by DiagnosticsViewModel (located at src/viewmodels/DiagnosticsViewModel.h and .cpp), which communicates with hardware devices through a StationRuntime object.

The ViewModel currently exposes:

- deviceStatuses (QVariantList): array of 4 device status maps, each containing name, status (online/offline), lastUpdate, summary (a text string), and errorCount
- communicationLogs (QVariantList): array of communication log entries
- statusMessage (QString): current status text
- Control methods: setMotorForward(), setMotorReverse(), stopMotor(), setBrakeOutput(), setBrakeCurrent(), setBrakeVoltage(), setEncoderZero()

The current layout uses a two-column design: left column has device status cards and manual control card, right column has a fixed 480px communication log panel. Device status cards show only a text summary like "电流 1.23 A / AI1 高" without structured numeric displays.

The redesign will:
1. Add four new Q_PROPERTY members to DiagnosticsViewModel to expose structured telemetry data as QVariantMap objects
2. Rewrite DiagnosticsPage.qml to use AppTabs component for tab navigation
3. Create five tab pages: overview table + four device detail pages
4. Move communication log to a collapsible panel at the bottom using AppCollapsible component

Key QML components available in src/ui/components/:
- AppTabs: tab navigation component with horizontal tab bar
- AppTable: table component for structured data display
- AppCollapsible: collapsible panel with expand/collapse animation
- AppCard, AppButton, AppLabel, AppBadge, AppAlert: existing UI primitives


## Plan of Work

### Step 1: Extend DiagnosticsViewModel with structured telemetry properties

Open src/viewmodels/DiagnosticsViewModel.h. After the existing Q_PROPERTY declarations (around line 10-13), add four new properties:

    Q_PROPERTY(QVariantMap motorTelemetry READ motorTelemetry NOTIFY motorTelemetryChanged)
    Q_PROPERTY(QVariantMap torqueTelemetry READ torqueTelemetry NOTIFY torqueTelemetryChanged)
    Q_PROPERTY(QVariantMap encoderTelemetry READ encoderTelemetry NOTIFY encoderTelemetryChanged)
    Q_PROPERTY(QVariantMap brakeTelemetry READ brakeTelemetry NOTIFY brakeTelemetryChanged)

Add corresponding getter methods in the public section (around line 20):

    QVariantMap motorTelemetry() const { return m_motorTelemetry; }
    QVariantMap torqueTelemetry() const { return m_torqueTelemetry; }
    QVariantMap encoderTelemetry() const { return m_encoderTelemetry; }
    QVariantMap brakeTelemetry() const { return m_brakeTelemetry; }

Add signal declarations in the signals section (around line 35):

    void motorTelemetryChanged();
    void torqueTelemetryChanged();
    void encoderTelemetryChanged();
    void brakeTelemetryChanged();

Add member variables in the private section (around line 45):

    QVariantMap m_motorTelemetry;
    QVariantMap m_torqueTelemetry;
    QVariantMap m_encoderTelemetry;
    QVariantMap m_brakeTelemetry;

These properties will hold structured telemetry data. For example, motorTelemetry will contain keys like currentA (double), ai1Level (bool), and online (bool).

### Step 2: Update DiagnosticsViewModel.cpp to populate telemetry properties

Open src/viewmodels/DiagnosticsViewModel.cpp. Locate the buildMotorStatus() method (around line 140). After reading the device values and before the return statement, add code to update m_motorTelemetry:

    QVariantMap newMotorTelemetry{
        {"currentA", currentA},
        {"ai1Level", ai1Level},
        {"online", online}
    };
    if (m_motorTelemetry != newMotorTelemetry) {
        m_motorTelemetry = newMotorTelemetry;
        emit motorTelemetryChanged();
    }

Similarly, in buildTorqueStatus() (around line 160), add:

    QVariantMap newTorqueTelemetry{
        {"torqueNm", torqueNm},
        {"speedRpm", speedRpm},
        {"powerW", powerW},
        {"online", ok}
    };
    if (m_torqueTelemetry != newTorqueTelemetry) {
        m_torqueTelemetry = newTorqueTelemetry;
        emit torqueTelemetryChanged();
    }

In buildEncoderStatus() (around line 180), add:

    QVariantMap newEncoderTelemetry{
        {"angleDeg", angleDeg},
        {"online", ok}
    };
    if (m_encoderTelemetry != newEncoderTelemetry) {
        m_encoderTelemetry = newEncoderTelemetry;
        emit encoderTelemetryChanged();
    }

In buildBrakeStatus() (around line 200), add:

    QString modeStr = (mode == 1) ? "CV" : "CC";
    QVariantMap newBrakeTelemetry{
        {"currentA", currentA},
        {"voltageV", voltageV},
        {"powerW", powerW},
        {"mode", modeStr},
        {"channel", m_runtime->brakeChannel()},
        {"online", online}
    };
    if (m_brakeTelemetry != newBrakeTelemetry) {
        m_brakeTelemetry = newBrakeTelemetry;
        emit brakeTelemetryChanged();
    }

These updates ensure that whenever device status is refreshed, the structured telemetry properties are also updated and signals are emitted for QML bindings to react.

### Step 3: Rewrite DiagnosticsPage.qml with tab-based layout

Open src/ui/pages/DiagnosticsPage.qml. The current structure is:

    Rectangle (background)
      ColumnLayout
        AppCard (header with refresh controls)
        RowLayout
          ColumnLayout (left: device cards + manual controls)
          AppCard (right: communication log, fixed 480px)

Replace the entire content with a new structure:

    Rectangle (background)
      ColumnLayout
        AppCard (header with refresh controls) — keep unchanged
        AppTabs (main content area with 5 tabs)
        AppCollapsible (communication log at bottom)

The AppTabs component will have a model array with 5 items:
- Tab 0: "设备总览"
- Tab 1: "AQMD 电机驱动器"
- Tab 2: "DYN200 扭矩传感器"
- Tab 3: "单圈绝对值编码器"
- Tab 4: "制动电源"

Each tab will use a Loader to dynamically load its content based on currentIndex. This keeps the QML file organized and avoids creating all tab contents upfront.

### Step 4: Implement Tab 0 - Device Overview Table

Tab 0 will display an AppTable with columns:
- 设备名称 (Device Name)
- 状态 (Status) - with colored badge
- 关键遥测 (Key Telemetry) - brief summary
- 最后更新 (Last Update)
- 错误数 (Error Count)

The table data will be derived from viewModel.deviceStatuses. Each row will be color-coded: green tint for online, red tint for offline.

Below the table, display an AppAlert showing the current viewModel.statusMessage.

### Step 5: Implement Tab 1 - AQMD Motor Driver Detail Page

Tab 1 will have three sections in a ColumnLayout:

Section 1: Device Info Card
- Connection status badge (online/offline)
- Last update timestamp
- Error count (if > 0)

Section 2: Telemetry Cards Grid
Use a Grid or GridLayout with 2 columns to display:
- Card 1: "电流" (Current) - large font showing motorTelemetry.currentA with unit "A"
- Card 2: "AI1 电平" (AI1 Level) - showing "高" or "低" based on motorTelemetry.ai1Level

Each telemetry card will be an AppCard with:
- Small label at top (e.g., "电流")
- Large value in the center (e.g., "1.23")
- Unit label below (e.g., "A")

Section 3: Motor Control Buttons
Row of buttons:
- "正转" (Forward) - triggers confirmation dialog
- "反转" (Reverse) - triggers confirmation dialog
- "停止" (Stop) - calls viewModel.stopMotor() directly

Keep the existing confirmation dialog logic for forward/reverse commands.

### Step 6: Implement Tab 2 - DYN200 Torque Sensor Detail Page

Similar structure to Tab 1, but with three telemetry cards:
- Card 1: "扭矩" (Torque) - torqueTelemetry.torqueNm N·m
- Card 2: "转速" (Speed) - torqueTelemetry.speedRpm RPM
- Card 3: "功率" (Power) - torqueTelemetry.powerW W

No control section since this is a read-only sensor.

### Step 7: Implement Tab 3 - Encoder Detail Page

One telemetry card:
- "角度" (Angle) - encoderTelemetry.angleDeg °

Control section with one button:
- "置零" (Set Zero) - calls viewModel.setEncoderZero()

### Step 8: Implement Tab 4 - Brake Power Supply Detail Page

Four telemetry cards:
- "电流" (Current) - brakeTelemetry.currentA A
- "电压" (Voltage) - brakeTelemetry.voltageV V
- "功率" (Power) - brakeTelemetry.powerW W
- "工作模式" (Mode) - brakeTelemetry.mode (CC/CV)
- "通道" (Channel) - brakeTelemetry.channel

Control section with three groups:
1. Output enable/disable buttons
2. Current setting: input field + "设置" button
3. Voltage setting: input field + "设置电压" button

### Step 9: Move Communication Log to Collapsible Bottom Panel

After the AppTabs component, add an AppCollapsible component with:
- title: "通信日志"
- open: false (collapsed by default)
- Content: the existing ListView for communication logs
- Header action: "清空" button to call viewModel.clearLog()

The collapsible panel will be full-width and positioned at the bottom of the page. When collapsed, it shows only the header bar. When expanded, it reveals the log ListView with a reasonable height (e.g., 300px).


## Concrete Steps

All commands should be run from the working directory F:\Work\FlipGearboxFactoryTest.

### Build and run the application

After making code changes, rebuild and run:

    cmake --build build --config Debug
    .\build\Debug\FlipGearboxFactoryTest.exe

Expected output: Application window opens, navigate to "I/O 诊断" page from the sidebar.

### Verify ViewModel changes

After Step 1 and Step 2, rebuild the application. Open the diagnostics page and check the Qt console output for any QML binding errors. The new telemetry properties should be accessible from QML without errors.

### Verify tab navigation

After Step 3, the page should show a horizontal tab bar at the top with 5 tabs. Clicking each tab should switch the content area. Initially, tabs may be empty or show placeholder content.

### Verify device overview table

After Step 4, Tab 0 should display a table with 4 rows (one per device) and 5 columns. Online devices should have green-tinted rows, offline devices red-tinted.

### Verify device detail pages

After Steps 5-8, each device tab should show:
- Device info at top
- Large telemetry value cards in the middle
- Control buttons at bottom (where applicable)

Click the "正转" button in Tab 1 and verify the confirmation dialog appears. Click "停止" and verify the motor stop command is logged.

### Verify collapsible log

After Step 9, the communication log should appear as a collapsed bar at the bottom. Click the header to expand and see the log entries. Click "清空" to clear the log.


## Validation and Acceptance

Start the application and navigate to the I/O diagnostics page. Verify the following behaviors:

1. Tab Navigation: Five tabs are visible: "设备总览", "AQMD 电机驱动器", "DYN200 扭矩传感器", "单圈绝对值编码器", "制动电源". Clicking each tab switches the content area.

2. Device Overview Table: Tab 0 shows a table with 4 device rows. Each row displays device name, status badge, telemetry summary, last update time, and error count. Online devices have green indicators, offline devices have red indicators.

3. Motor Driver Tab: Tab 1 shows:
   - Device status card at top
   - Two telemetry cards: "电流" showing a numeric value with "A" unit, "AI1 电平" showing "高" or "低"
   - Three control buttons: "正转", "反转", "停止"
   - Clicking "正转" or "反转" opens a confirmation dialog
   - Clicking "停止" immediately sends the stop command

4. Torque Sensor Tab: Tab 2 shows:
   - Device status card
   - Three telemetry cards: "扭矩" (N·m), "转速" (RPM), "功率" (W)
   - No control buttons (read-only device)

5. Encoder Tab: Tab 3 shows:
   - Device status card
   - One telemetry card: "角度" (°)
   - One control button: "置零"

6. Brake Power Tab: Tab 4 shows:
   - Device status card
   - Five telemetry cards: "电流" (A), "电压" (V), "功率" (W), "工作模式" (CC/CV), "通道"
   - Control section with enable/disable buttons, current input + set button, voltage input + set button

7. Communication Log: At the bottom of the page, a collapsible panel with header "通信日志" is visible. Initially collapsed, showing only the header bar. Click the header to expand and see log entries. Click "清空" button in the header to clear the log. The log should update in real-time when commands are sent.

8. Auto-refresh: Enable auto-refresh in the header controls. Telemetry values in all tabs should update at the specified interval (default 2 seconds). The device overview table should also update.

9. Manual Refresh: Click "立即刷新" button in the header. All device statuses and telemetry values should update immediately.

Run the application for at least 2 minutes with auto-refresh enabled. Verify that telemetry values update smoothly without UI freezing or errors in the console.


## Idempotence and Recovery

All code changes are additive and safe to apply multiple times:

- Adding new Q_PROPERTY declarations to the header is idempotent
- Adding new member variables and methods is idempotent
- Rewriting DiagnosticsPage.qml completely replaces the old layout; no partial state to worry about

If the application fails to build after changes:
1. Check for syntax errors in the modified files
2. Ensure all opening braces have matching closing braces
3. Verify that all QVariantMap keys are quoted strings
4. Check that signal names match the property names (e.g., motorTelemetryChanged for motorTelemetry)

If the application builds but crashes on startup:
1. Check the Qt console output for QML errors
2. Verify that all required properties are initialized in the ViewModel constructor
3. Ensure that viewModel is not null before accessing its properties in QML

If telemetry values don't update:
1. Verify that the emit statements are present in the ViewModel methods
2. Check that the property bindings in QML use the correct property names
3. Enable auto-refresh and check if the refreshIncremental() method is being called

To roll back changes:
1. Use git diff to review changes
2. Use git checkout -- <file> to revert individual files
3. Rebuild and test after reverting


## Artifacts and Notes

### Expected ViewModel Header Structure

After Step 1, the header file should have this structure:

    class DiagnosticsViewModel : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantList deviceStatuses READ deviceStatuses NOTIFY deviceStatusesChanged)
        Q_PROPERTY(QVariantList communicationLogs READ communicationLogs NOTIFY communicationLogsChanged)
        Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
        Q_PROPERTY(bool runtimeInitialized READ runtimeInitialized NOTIFY deviceStatusesChanged)
        Q_PROPERTY(QVariantMap motorTelemetry READ motorTelemetry NOTIFY motorTelemetryChanged)
        Q_PROPERTY(QVariantMap torqueTelemetry READ torqueTelemetry NOTIFY torqueTelemetryChanged)
        Q_PROPERTY(QVariantMap encoderTelemetry READ encoderTelemetry NOTIFY encoderTelemetryChanged)
        Q_PROPERTY(QVariantMap brakeTelemetry READ brakeTelemetry NOTIFY brakeTelemetryChanged)

    public:
        // ... existing methods ...
        QVariantMap motorTelemetry() const { return m_motorTelemetry; }
        QVariantMap torqueTelemetry() const { return m_torqueTelemetry; }
        QVariantMap encoderTelemetry() const { return m_encoderTelemetry; }
        QVariantMap brakeTelemetry() const { return m_brakeTelemetry; }

    signals:
        void deviceStatusesChanged();
        void communicationLogsChanged();
        void statusMessageChanged();
        void motorTelemetryChanged();
        void torqueTelemetryChanged();
        void encoderTelemetryChanged();
        void brakeTelemetryChanged();

    private:
        // ... existing members ...
        QVariantMap m_motorTelemetry;
        QVariantMap m_torqueTelemetry;
        QVariantMap m_encoderTelemetry;
        QVariantMap m_brakeTelemetry;
    };

### Expected Telemetry Data Structure

Each telemetry QVariantMap will have the following keys:

motorTelemetry:
- currentA (double): motor current in amperes
- ai1Level (bool): AI1 digital input level (true = high, false = low)
- online (bool): device communication status

torqueTelemetry:
- torqueNm (double): torque in Newton-meters
- speedRpm (double): rotational speed in RPM
- powerW (double): mechanical power in watts
- online (bool): device communication status

encoderTelemetry:
- angleDeg (double): angle in degrees (0-360)
- online (bool): device communication status

brakeTelemetry:
- currentA (double): output current in amperes
- voltageV (double): output voltage in volts
- powerW (double): output power in watts
- mode (string): operating mode, either "CC" (constant current) or "CV" (constant voltage)
- channel (int): active channel number
- online (bool): device communication status

### QML Tab Structure Outline

The new DiagnosticsPage.qml will have this high-level structure:

    Item {
        id: root
        // ... properties ...

        Rectangle {
            anchors.fill: parent
            color: root.theme.bgColor

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16

                // Header card with refresh controls (unchanged)
                Components.AppCard { /* ... */ }

                // Main tabs area
                Components.AppTabs {
                    id: mainTabs
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    theme: root.theme
                    model: [
                        {text: "设备总览"},
                        {text: "AQMD 电机驱动器"},
                        {text: "DYN200 扭矩传感器"},
                        {text: "单圈绝对值编码器"},
                        {text: "制动电源"}
                    ]
                    showContent: false

                    // Content loader based on currentIndex
                    Loader {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        sourceComponent: {
                            switch (mainTabs.currentIndex) {
                                case 0: return overviewTab
                                case 1: return motorTab
                                case 2: return torqueTab
                                case 3: return encoderTab
                                case 4: return brakeTab
                                default: return null
                            }
                        }
                    }
                }

                // Communication log collapsible panel
                Components.AppCollapsible {
                    Layout.fillWidth: true
                    title: "通信日志"
                    open: false
                    theme: root.theme
                    // Log ListView content
                }
            }
        }

        // Tab content components
        Component { id: overviewTab; /* ... */ }
        Component { id: motorTab; /* ... */ }
        Component { id: torqueTab; /* ... */ }
        Component { id: encoderTab; /* ... */ }
        Component { id: brakeTab; /* ... */ }

        // Motor confirmation dialog (unchanged)
        Components.AppAlertDialog { /* ... */ }
    }

### Telemetry Card Component Pattern

Each telemetry card will follow this pattern:

    Components.AppCard {
        Layout.preferredWidth: 180
        Layout.preferredHeight: 120
        theme: root.theme

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 8

            Components.AppLabel {
                text: "电流"
                fontSize: 12
                color: root.theme.textSecondary
                theme: root.theme
                Layout.alignment: Qt.AlignHCenter
            }

            Components.AppLabel {
                text: root.viewModel && root.viewModel.motorTelemetry.online
                      ? root.viewModel.motorTelemetry.currentA.toFixed(2)
                      : "--"
                fontSize: 32
                fontWeight: 700
                theme: root.theme
                Layout.alignment: Qt.AlignHCenter
            }

            Components.AppLabel {
                text: "A"
                fontSize: 14
                color: root.theme.textSecondary
                theme: root.theme
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }

This pattern creates a visually prominent display of a single telemetry value with clear labeling and units.


## Interfaces and Dependencies

### ViewModel Interface

The DiagnosticsViewModel class (in namespace ViewModels) must expose these Q_PROPERTY members:

    Q_PROPERTY(QVariantMap motorTelemetry READ motorTelemetry NOTIFY motorTelemetryChanged)
    Q_PROPERTY(QVariantMap torqueTelemetry READ torqueTelemetry NOTIFY torqueTelemetryChanged)
    Q_PROPERTY(QVariantMap encoderTelemetry READ encoderTelemetry NOTIFY encoderTelemetryChanged)
    Q_PROPERTY(QVariantMap brakeTelemetry READ brakeTelemetry NOTIFY brakeTelemetryChanged)

Each property must have:
- A getter method returning QVariantMap
- A corresponding signal for property change notification
- A private member variable of type QVariantMap

The telemetry properties must be updated in the buildXxxStatus() methods before returning the status map. Use this pattern:

    QVariantMap newTelemetry{ /* keys and values */ };
    if (m_xxxTelemetry != newTelemetry) {
        m_xxxTelemetry = newTelemetry;
        emit xxxTelemetryChanged();
    }

This ensures that signals are only emitted when values actually change, avoiding unnecessary QML updates.

### QML Component Dependencies

The redesigned DiagnosticsPage.qml depends on these components from src/ui/components/:

- AppTabs: provides tab navigation UI
  - Required properties: theme, model (array of tab objects)
  - Key properties: currentIndex (int, current selected tab)
  - Must set showContent: false to manage content manually

- AppTable: displays device overview in tabular format
  - Required properties: theme, columns (array of column definitions), data (array of row objects)
  - Column definition: {key: string, label: string, width: number}

- AppCollapsible: provides expandable/collapsible panel for communication log
  - Required properties: theme, title (string)
  - Key properties: open (bool, controls expanded state)
  - Content goes in default property slot

- AppCard, AppButton, AppLabel, AppBadge, AppAlert, AppSeparator: existing UI primitives used throughout

All components require a theme property of type AppTheme.

### Data Flow

The data flow for telemetry updates is:

1. User triggers refresh (auto or manual) -> DiagnosticsViewModel::refresh() or refreshIncremental()
2. ViewModel calls updateDeviceStatus(index) -> calls buildXxxStatus()
3. buildXxxStatus() reads from hardware devices via StationRuntime
4. buildXxxStatus() updates both m_deviceStatuses[index] and m_xxxTelemetry member
5. ViewModel emits deviceStatusesChanged() and xxxTelemetryChanged() signals
6. QML bindings react to signals and update UI:
   - Device overview table reads from viewModel.deviceStatuses
   - Device detail tabs read from viewModel.xxxTelemetry
7. User sees updated values in the UI

Control flow for commands (e.g., motor forward):

1. User clicks button in device tab -> QML calls viewModel.setMotorForward()
2. ViewModel sends command to hardware via StationRuntime
3. ViewModel calls appendLog() to record the command
4. ViewModel calls refresh() to update device status
5. Telemetry values update and UI reflects new state

### Build Dependencies

The project uses CMake and Qt 6.11.0. After modifying C++ files, rebuild with:

    cmake --build build --config Debug

QML files are hot-reloadable in some cases, but for structural changes (like adding new components), a full application restart is required.

No external dependencies are added by this redesign. All components and classes already exist in the codebase.
