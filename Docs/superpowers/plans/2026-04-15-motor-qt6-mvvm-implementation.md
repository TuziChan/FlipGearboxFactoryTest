# Qt6 Motor MVVM Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a Qt 6.11.0 desktop application that faithfully recreates [motor.html](/F:/Work/产品出厂程序qt版/Docs/motor.html) as a responsive `QML + Qt Quick` interface, runs the test flow in simulation mode, and can switch to real multi-bus RS485/Modbus RTU devices without changing the UI layer.

**Architecture:** The application is a layered MVVM system. QML renders the UI, C++ ViewModels expose bindable state and commands, a domain-layer `TestSequenceEngine` owns the test flow and verdict logic, and an infrastructure layer handles JSON configuration, multi-bus Modbus RTU communication, device adapters, and simulated devices. The same ViewModels and flow engine are used in both simulation and real-device modes.

**Tech Stack:** Qt 6.11.0 from `F:\Qt`, Qt Quick/QML, Qt Charts, Qt SerialPort, Qt Test, CMake 4.0.3, MinGW 13.1.0 from `F:\Qt\Tools\mingw1310_64`, C++20, JSON config files.

---

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds.

No `PLANS.md` file is checked into this repository as of 2026-04-15. Maintain this document according to the ExecPlan rules from `AGENTS.md` in the repository root context.

## Purpose / Big Picture

After this work is complete, an operator can start a native Windows Qt application instead of opening a static HTML mockup. In simulation mode, the operator can run through the full gearbox test flow and observe the same high-level behaviors visible in `motor.html`: stage progression, live metrics, waveform updates, phase-specific detail tables, verdict changes, and report generation. In real mode, the same UI controls actual devices distributed across multiple RS485 buses, surfaces device-online status, and distinguishes communication failures from product failures.

The result matters because the HTML file already proves the intended operator experience, but it cannot become a factory tool until the UI is connected to a real execution model. This plan turns the mockup into a maintainable application with test coverage, reproducible build steps, and a clear extension path for the remaining pages.

## Progress

- [x] (2026-04-15 14:00 +08:00) Wrote and approved the design specification at [2026-04-15-motor-qt6-mvvm-design.md](/F:/Work/产品出厂程序qt版/docs/superpowers/specs/2026-04-15-motor-qt6-mvvm-design.md).
- [x] (2026-04-15 14:00 +08:00) Confirmed local toolchain availability: `F:\Qt\6.11.0\mingw_64`, `F:\Qt\Tools\mingw1310_64`, `C:\Program Files\CMake\bin\cmake.exe`, `Qt6Charts`, and `Qt6SerialPort`.
- [x] (2026-04-15 14:00 +08:00) Created this implementation plan.
- [x] (2026-04-15 15:12 +08:00) Expanded the existing Hello World Qt skeleton into a testable app target with `Qt Test`, a `QmlSmokeTests` executable, and a reproducible `build\mingw-debug` configure/build flow.
- [x] (2026-04-15 15:12 +08:00) Wrote the first failing application smoke test that asserted the native shell title and presence of the command bar, phase panel, metrics area, and verdict panel.
- [x] (2026-04-15 15:12 +08:00) Implemented the minimal application bootstrap to make the smoke test pass by replacing `Hello World` with a structured `motor.html` replica shell.
- [x] (2026-04-16 00:10 +08:00) Extracted a dedicated [AppShell.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/AppShell.qml) shell, moved component samples into [ComponentGalleryPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/ComponentGalleryPage.qml), rewired [Main.qml](/F:/Work/FlipGearboxFactoryTest/Main.qml), and updated [QmlSmokeTests.cpp](/F:/Work/FlipGearboxFactoryTest/tests/ui/QmlSmokeTests.cpp) so component examples are exercised on a separate page instead of living under the business page.
- [x] (2026-04-16 00:10 +08:00) Stabilized UI smoke execution by switching the test command to explicit `cmake -E env`, forcing `QT_QUICK_CONTROLS_STYLE=Basic`, fixing the `AppCommand` import gap, and cleaning the most obvious QML runtime warnings (`modelData`, empty-model guards, flow ordering). `QmlSmokeTests` now reports 17 passing checks.
- [x] (2026-04-16 00:14 +08:00) Ran `qmllint` on [Main.qml](/F:/Work/FlipGearboxFactoryTest/Main.qml), [AppShell.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/AppShell.qml), [TestExecutionPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/TestExecutionPage.qml), and [ComponentGalleryPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/ComponentGalleryPage.qml); cleared the remaining unqualified-access and layout warnings in those files.
- [x] (2026-04-16 00:18 +08:00) Fixed the “组件库页空白” regression by giving [SectionCard.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/SectionCard.qml) a content-derived `implicitHeight`, adding a smoke assertion that the gallery intro card has positive height, and verifying the gallery page is no longer structurally empty.
- [x] (2026-04-16 00:33 +08:00) Used parallel worker agents for the first source-based component batch, then integrated and reviewed the returned files locally: [AppCheckbox.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppCheckbox.qml), [AppRadioGroup.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppRadioGroup.qml), [AppToggle.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppToggle.qml), and [AppToggleGroup.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppToggleGroup.qml).
- [x] (2026-04-16 00:33 +08:00) Extended [ComponentGalleryPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/ComponentGalleryPage.qml) and [QmlSmokeTests.cpp](/F:/Work/FlipGearboxFactoryTest/tests/ui/QmlSmokeTests.cpp) with checkbox/radio-group/toggle/toggle-group examples plus gallery helper methods. Fresh verification now reports `QmlSmokeTests` 19 passing checks and `qmllint` clean on the new component files.
- [x] (2026-04-16 09:51 +08:00) Completed the second main-thread integration wave for parallel worker output: [AppAlert.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppAlert.qml), [AppAlertDialog.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppAlertDialog.qml), [AppMenubar.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppMenubar.qml), [AppContextMenu.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppContextMenu.qml), [AppAccordion.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppAccordion.qml), [AppCollapsible.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppCollapsible.qml), [AppField.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppField.qml), [AppForm.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppForm.qml), [AppAvatar.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppAvatar.qml), [AppBreadcrumb.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppBreadcrumb.qml), and [AppButtonGroup.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppButtonGroup.qml).
- [x] (2026-04-16 09:51 +08:00) Expanded [ComponentGalleryPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/ComponentGalleryPage.qml), [CMakeLists.txt](/F:/Work/FlipGearboxFactoryTest/CMakeLists.txt), and [QmlSmokeTests.cpp](/F:/Work/FlipGearboxFactoryTest/tests/ui/QmlSmokeTests.cpp) for the new menu, alert, accordion/collapsible, field/form, and identity-path samples. Fresh verification now reports `QmlSmokeTests` 23 passing checks and `qmllint` clean on the newly integrated component wave.
- [x] (2026-04-16 10:19 +08:00) Completed the next main-thread component batch without subagents: [AppTable.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppTable.qml), [AppSlider.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppSlider.qml), [AppScrollArea.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppScrollArea.qml), [AppPagination.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppPagination.qml), [AppNativeSelect.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppNativeSelect.qml), [AppInputGroup.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppInputGroup.qml), [AppInputOtp.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppInputOtp.qml), [AppNavigationMenu.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppNavigationMenu.qml), [AppHoverCard.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppHoverCard.qml), and [AppDrawer.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppDrawer.qml).
- [x] (2026-04-16 10:19 +08:00) Extended [ComponentGalleryPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/ComponentGalleryPage.qml) and [QmlSmokeTests.cpp](/F:/Work/FlipGearboxFactoryTest/tests/ui/QmlSmokeTests.cpp) with table/slider/pagination/native-select/input-group/input-otp/navigation-menu/hover-card/drawer samples and helper methods. Fresh verification now reports `QmlSmokeTests` 27 passing checks and `qmllint` clean on this batch.
- [x] (2026-04-16 10:19 +08:00) Completed the requested high-complexity visual batch in the main thread: [AppCalendar.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppCalendar.qml), [AppCarousel.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppCarousel.qml), [AppResizable.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppResizable.qml), [AppAspectRatio.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppAspectRatio.qml), plus deeper revisions to [AppSlider.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppSlider.qml) for multi-thumb/range behavior and [AppTable.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppTable.qml) for footer rows and row variants.
- [x] (2026-04-16 10:19 +08:00) Extended [ComponentGalleryPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/ComponentGalleryPage.qml), [CMakeLists.txt](/F:/Work/FlipGearboxFactoryTest/CMakeLists.txt), and [QmlSmokeTests.cpp](/F:/Work/FlipGearboxFactoryTest/tests/ui/QmlSmokeTests.cpp) with calendar/carousel/resizable/aspect-ratio/range-slider/enhanced-table samples and helper methods. Fresh verification now reports `QmlSmokeTests` 27 passing checks and `qmllint` clean on the entire new batch.
- [x] (2026-04-16 10:50 +08:00) Completed the follow-up depth pass requested for the same family by stabilizing the new calendar/carousel/resizable/aspect-ratio batch and finishing the gallery/test wiring. Fresh verification now reports `QmlSmokeTests` 29 passing checks, with the new `operatesCalendarCarouselAndResizable()` and `operatesAspectRatioRangeSliderAndEnhancedTable()` cases both green.
- [ ] Commit the scaffold milestone.
- [ ] Write failing domain tests for stage transitions, elapsed time tracking, and verdict classification.
- [ ] Implement the domain model, `TestSequenceEngine`, limits profile, and report builder.
- [ ] Run domain tests until all pass and commit the domain milestone.
- [ ] Write failing tests for simulated devices and `StationRuntimeFactory` in simulation mode.
- [ ] Implement simulated motor drive, torque sensor, encoder, power supply, and runtime factory wiring.
- [ ] Run simulation and domain tests together and commit the simulation milestone.
- [ ] Write failing ViewModel tests for command validation, step state projection, metrics formatting, chart channel toggles, and verdict/report projection.
- [ ] Implement `MainViewModel`, `TestExecutionViewModel`, and all child ViewModels.
- [ ] Run ViewModel tests and commit the ViewModel milestone.
- [ ] Write failing UI smoke tests that load the page and assert the presence of key sections.
- [ ] Implement the QML shell, responsive layout system, shared style constants, and placeholder secondary pages (completed: shared shell layout, theme object, and reusable components for title bar, nav rail, command bar, step panel, metric card, chart panel, shared data table, verdict panel, info bar, and status bar; remaining: secondary placeholder pages and stricter lint cleanup).
- [ ] Implement the test-execution components and live chart binding (completed: timer-driven simulated metrics, tables, verdict state, and channel toggles; remaining: move simulation logic out of the page layer into ViewModels/runtime objects).
- [ ] Run `qmllint`, UI smoke tests, and manual resolution checks; commit the UI milestone.
- [ ] Write failing infrastructure tests for Modbus CRC, frame encoding, bus scheduling, timeout classification, and adapter register mapping.
- [ ] Implement `IBusController`, `ModbusRtuBusController`, `BusManager`, and the four device adapters.
- [ ] Run infrastructure tests and commit the communication milestone.
- [ ] Write failing config-loader tests for `config/app.json` and `config/stations.json`.
- [ ] Implement JSON config parsing, real/simulation mode selection, and station assembly.
- [ ] Run full automated tests, execute the app in simulation mode, and commit the configuration milestone.
- [ ] Verify real-mode startup against attached hardware if hardware is available; otherwise document the exact expected operator procedure and residual risk.
- [ ] Run the full acceptance checklist, update this plan’s outcomes, and make the final implementation commit.

## Surprises & Discoveries

- Observation: This repository is not documentation-only. It already had a generated Qt Quick Hello World skeleton (`CMakeLists.txt`, `main.cpp`, `Main.qml`) that could be upgraded instead of replaced from scratch.
  Evidence: The initial working tree contained those three files before implementation work began, and the first smoke test failed because the window title was still `Hello World`.

- Observation: `ctest` on Windows did not inherit the Qt runtime directories automatically, which made the first smoke-test executions fail before any assertions ran.
  Evidence: `QmlSmokeTests` initially exited with Windows code `0xc0000135` until `ENVIRONMENT_MODIFICATION` prepended the Qt runtime folders to `PATH`.

- Observation: Qt 6.11.0's QML tooling is strict about `required property` misuse and delegate scope leakage, especially after breaking the page into reusable components.
  Evidence: `qmllint` and the QML cache compiler rejected `required property alias ...` declarations in `CommandBar.qml`, and still report unqualified-access warnings that now need explicit cleanup.

- Observation: Moving the sample components into a new `StackLayout` slot was not enough to satisfy the “separate page” requirement because the shell itself still lived inside `TestExecutionPage.qml`, making the sample subtree a descendant of the business page.
  Evidence: The new smoke assertion still found `sampleDialog` under `testExecutionPage` until [AppShell.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/AppShell.qml) became the outer shell and [TestExecutionPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/TestExecutionPage.qml) was reduced to business-only content.

- Observation: After extracting `AppShell.qml`, unqualified assignments such as `theme: theme` in [TestExecutionPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/TestExecutionPage.qml) became self-bindings instead of references to the shared theme object.
  Evidence: Runtime warnings like `Binding loop detected for property "theme"` and cascaded `Cannot read property 'textPrimary' of null` disappeared once those bindings were rewritten to `theme: root.theme`.

- Observation: [SectionCard.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/SectionCard.qml) worked in placeholder pages only because those call sites assigned an explicit `height`. The new component gallery relied on implicit sizing, so every card collapsed to zero height until the component itself defined `implicitHeight`.
  Evidence: The gallery cards in [ComponentGalleryPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/ComponentGalleryPage.qml) only set `width`, and the blank-page report disappeared after `SectionCard.qml` started deriving `implicitHeight` from its internal column content.

- Observation: QtTest function targeting was briefly misleading during the new gallery test rollout. The new test functions were compiled into `QmlSmokeTests.exe`, but the first direct invocations still returned a `Function not found` message before succeeding on immediate rerun.
  Evidence: `QmlSmokeTests.exe -functions` listed `togglesCheckboxAndRadioGroupSelection` and `togglesToggleAndToggleGroupSelection`, and the real red phase only appeared on rerun when the gallery helper methods were still missing.

- Observation: Upstream subagent transport could fail with `502 Bad Gateway` while still leaving usable progress behind. One `avatar / breadcrumb / button-group` worker appeared to error in notifications, but closing the thread later surfaced a completed result and created the expected component files.
  Evidence: The failed `019d93d2-a040-76a0-9961-31168fae5626` thread later returned a completed status during shutdown, and [AppAvatar.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppAvatar.qml), [AppBreadcrumb.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppBreadcrumb.qml), and [AppButtonGroup.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppButtonGroup.qml) were present locally.

- Observation: `Loader`-based delegate composition in [AppButtonGroup.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/components/AppButtonGroup.qml) made `index` propagation easy to get wrong. The first integration pass compiled and passed tests, but emitted repeated runtime warnings because the loaded delegates were not receiving a stable item index.
  Evidence: `QmlSmokeTests.exe` initially reported repeated `Required property index was not initialized` and then `setProperty is not a function` warnings until the delegate index handoff was rewritten.

- Observation: Newly added QtTest cases could still show an initial `Function not found` response even after the executable was rebuilt, but the same binary immediately listed the functions via `-functions` and the second run hit the expected behavioral failures.
  Evidence: The third-wave tests for table/slider/input/navigation initially produced the same transient false red seen earlier, then reran into real component/helper failures once `QmlSmokeTests.exe -functions` confirmed they were compiled in.

- Observation: For the requested “complete in main thread” wave, the actual blocking work shifted from component creation to gallery/test integration correctness. The new source files compiled quickly, but the first real failures came from missing sample objects, helper methods, and one malformed `Q_ARG` call in `QmlSmokeTests.cpp`.
  Evidence: The red phase for `operatesTableSliderAndPagination`, `operatesNativeSelectAndInputOtp`, `operatesNavigationMenuHoverCardAndDrawer`, and `scrollsScrollAreaAndLoadsInputGroup` initially failed on missing gallery objects/methods, and the first build failed on `Q_ARG(QVariant, QVariantList{...})`.

- Observation: The deepest red point in the follow-up batch was not component creation but data marshaling from `QVariantList` into the range-slider helper path. The first pass created the range slider UI, but the gallery-facing summary string stayed empty until the helper normalized the incoming variant list explicitly.
  Evidence: `operatesAspectRatioRangeSliderAndEnhancedTable()` kept failing with `sampleRangeSliderValuesText == ""` until `setSampleRangeSliderValues()` in [ComponentGalleryPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/ComponentGalleryPage.qml) stopped relying on implicit array conversion.

- Observation: The repository currently contains only documentation assets under `Docs`; there is no existing Qt or CMake project to extend.
  Evidence: `Get-ChildItem -Force F:\Work\产品出厂程序qt版` returned only the `Docs` directory before planning work began.

- Observation: The requested Qt installation already includes the required modules for this plan.
  Evidence: `F:\Qt\6.11.0\mingw_64\lib\cmake` contains `Qt6Quick`, `Qt6Charts`, `Qt6SerialPort`, and `Qt6Test`.

- Observation: The local Qt installation includes both `mingw_64` and `msvc2022_64`, but only the MinGW toolchain is fully self-contained inside `F:\Qt`.
  Evidence: `F:\Qt\Tools\mingw1310_64\bin` contains `gcc.exe`, `g++.exe`, and `mingw32-make.exe`, while `cmake.exe` is available separately at `C:\Program Files\CMake\bin\cmake.exe`.

- Observation: All documented target devices support `RS485`, and the three clearest first-party integration paths use `Modbus RTU`.
  Evidence: [AQMD3610NS-A2.md](/F:/Work/产品出厂程序qt版/Docs/AQMD3610NS-A2.md), [单圈编码器.md](/F:/Work/产品出厂程序qt版/Docs/单圈编码器.md), and [双通道数控电源用户手册.md](/F:/Work/产品出厂程序qt版/Docs/双通道数控电源用户手册.md) all document Modbus RTU explicitly; the DYN-200 manual documents both Modbus RTU and active upload.

## Decision Log

- Decision: Use the `mingw_64` Qt 6.11.0 kit for the first implementation pass.
  Rationale: The MinGW kit is fully available under `F:\Qt`, avoids requiring a pre-configured Visual Studio developer shell, and keeps the build instructions reproducible on this machine.
  Date/Author: 2026-04-15 / Codex

- Decision: Use `Qt Charts` for the first realtime waveform implementation rather than writing a custom QML canvas renderer.
  Rationale: `Qt6Charts` is already installed, supports line-series updates, and reduces custom drawing code while the higher-risk device integration pieces are still being built.
  Date/Author: 2026-04-15 / Codex

- Decision: Treat DYN-200 as a Modbus RTU device in the first release even though it also supports active upload.
  Rationale: A single protocol path across all devices reduces scheduling complexity, simplifies tests, and is sufficient to prove the multi-bus runtime model.
  Date/Author: 2026-04-15 / Codex

- Decision: Keep the first release focused on the “测试执行” page and ship the remaining navigation targets as skeleton pages only.
  Rationale: This matches the approved scope and keeps the work outcome-focused: a working native replacement for `motor.html`, not a diluted half-built suite of unrelated features.
  Date/Author: 2026-04-15 / Codex

- Decision: Do not implement the `motor.html` replica as one giant QML file; instead, extract reusable shell and card components under `src/ui/components`, leaving `TestExecutionPage.qml` as a layout/composition layer.
  Rationale: The user explicitly requested shared, reusable components. This also aligns with the approved MVVM direction by preventing the page file from becoming the de facto design system and making later ViewModel wiring less risky.
  Date/Author: 2026-04-15 / Codex

- Decision: Split the application shell into [AppShell.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/AppShell.qml) and keep [TestExecutionPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/TestExecutionPage.qml) focused on the actual test-execution page content.
  Rationale: The user objected to putting component examples “inside the test page”. A real shell/page split makes `ComponentGalleryPage.qml` a sibling page instead of a hidden subtree under the business screen, which also clarifies navigation ownership in tests.
  Date/Author: 2026-04-16 / Codex

- Decision: Force the Qt Quick Controls `Basic` style for the smoke-test environment and the main application entry point.
  Rationale: The shadcn-style QML controls customize `TextField` and related controls. Qt's native Windows style warns against that customization, while `Basic` keeps the control visuals deterministic and compatible with the custom design system.
  Date/Author: 2026-04-16 / Codex

- Decision: Use parallel worker agents only for isolated component-file batches, and keep all integration, gallery wiring, and verification in the main thread.
  Rationale: The user explicitly requested subagents for full component ports but also required the main thread to check correctness and run tests. Restricting workers to disjoint write sets avoided merge conflicts while preserving main-thread review authority.
  Date/Author: 2026-04-16 / Codex

- Decision: Scale the second parallel wave to five `gpt-5.4` workers at once rather than continuing with two-worker batches.
  Rationale: The user explicitly asked for more aggressive multi-agent throughput. Grouping the next missing component families into five disjoint write sets increased component coverage per cycle while keeping integration centralized in the main thread.
  Date/Author: 2026-04-16 / Codex

- Decision: Execute the third wave entirely in the main thread instead of spawning more workers.
  Rationale: The user explicitly redirected to “主线程一次性完成这几个组件”. That batch touched ten related gallery/test integration points, so keeping implementation local reduced coordination overhead and let the red-green cycle stay tight.
  Date/Author: 2026-04-16 / Codex

- Decision: Keep the fourth wave in the main thread as well, and deepen existing `AppSlider` and `AppTable` in the same pass as the new components rather than splitting “new files” and “depth pass” into separate iterations.
  Rationale: The user explicitly requested one combined pass covering `calendar`, `carousel`, `resizable`, `aspect-ratio`, multi-thumb/range slider behavior, and richer table capabilities. Implementing them together avoided another gallery/test rewrite cycle.
  Date/Author: 2026-04-16 / Codex

## Outcomes & Retrospective

Milestone 1 is still functionally in place and now covers a substantially broader reusable component surface. The repository builds in `build\mingw-debug`, `QmlSmokeTests` passes with 29 assertions, and the native application launches with a structured `motor.html` replica shell instead of the generated Hello World window. The shell and page split remains explicit: [AppShell.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/AppShell.qml) owns navigation and theming, [TestExecutionPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/TestExecutionPage.qml) owns the business screen, and [ComponentGalleryPage.qml](/F:/Work/FlipGearboxFactoryTest/src/ui/pages/ComponentGalleryPage.qml) now demonstrates dialogs, menus, accordion/collapsible containers, form primitives, identity/path primitives, tabular/data-input primitives, extended overlay/navigation primitives, and the newly added calendar/carousel/resizable/aspect-ratio/range-layout group.

The main remaining gaps are still architectural, not visual. `TestExecutionPage.qml` continues to own timer-driven simulation state directly, which remains acceptable only as a temporary UI proof. On the component-library side, several families are still missing or only partially ported, but the reusable base is now materially wider and the gallery can exercise more categories without touching business pages. The next meaningful step remains moving phase progression, metrics, verdict logic, and report generation behind the planned ViewModel/domain/runtime layers while continuing to fill the remaining component inventory.

## Context and Orientation

The repository currently contains documentation only. The key existing files are:

- [motor.html](/F:/Work/产品出厂程序qt版/Docs/motor.html): the operator-facing HTML prototype that defines the target UI and simulated flow behavior.
- [AQMD3610NS-A2.md](/F:/Work/产品出厂程序qt版/Docs/AQMD3610NS-A2.md): the motor drive manual.
- [DYN-200使用手册印刷V3.7版本中性(12)(3)_2025-08-02-13_23_22.md](/F:/Work/产品出厂程序qt版/Docs/DYN-200使用手册印刷V3.7版本中性(12)(3)_2025-08-02-13_23_22.md): the torque sensor manual.
- [单圈编码器.md](/F:/Work/产品出厂程序qt版/Docs/单圈编码器.md): the encoder manual.
- [双通道数控电源用户手册.md](/F:/Work/产品出厂程序qt版/Docs/双通道数控电源用户手册.md): the dual-channel power supply manual.

Several terms of art appear throughout this plan:

- A “ViewModel” is a C++ `QObject` that exposes bindable properties and invokable commands to QML. In this repository, ViewModels live under `src/viewmodels` and must not know about raw QML items.
- A “bus controller” is the object that owns one physical serial line such as `COM3` and serializes Modbus requests for devices on that line. In this repository, bus controllers live under `src/infrastructure/bus`.
- A “device adapter” is a small class that translates business operations such as “start forward” or “read torque” into device-specific Modbus register reads and writes. In this repository, adapters live under `src/infrastructure/devices`.
- A “station runtime” is the assembled set of device interfaces required to test one gearbox station. It hides which serial line each device is on.
- “Simulation mode” means the application uses fake devices to produce deterministic telemetry. “Real mode” means the application uses actual serial communication.

The project will be created from scratch. That makes this plan prescriptive: every path below is a path that must exist by the time the milestone is complete.

## File Map

The project should be created with the following responsibility boundaries:

- Create [CMakeLists.txt](/F:/Work/产品出厂程序qt版/CMakeLists.txt) as the single top-level build entry point. It must define the app target, test targets, runtime output directory, and Qt modules.
- Create [src/app/main.cpp](/F:/Work/产品出厂程序qt版/src/app/main.cpp) to initialize `QGuiApplication`, parse mode and configuration arguments, construct the bootstrapper, and load QML.
- Create [src/app/ApplicationBootstrapper.h](/F:/Work/产品出厂程序qt版/src/app/ApplicationBootstrapper.h) and [src/app/ApplicationBootstrapper.cpp](/F:/Work/产品出厂程序qt版/src/app/ApplicationBootstrapper.cpp) to wire configuration, runtimes, and ViewModels together.
- Create [src/domain/TestPhase.h](/F:/Work/产品出厂程序qt版/src/domain/TestPhase.h), [src/domain/TestResult.h](/F:/Work/产品出厂程序qt版/src/domain/TestResult.h), [src/domain/LimitsProfile.h](/F:/Work/产品出厂程序qt版/src/domain/LimitsProfile.h), [src/domain/StationConfig.h](/F:/Work/产品出厂程序qt版/src/domain/StationConfig.h), [src/domain/TestSequenceEngine.h](/F:/Work/产品出厂程序qt版/src/domain/TestSequenceEngine.h), [src/domain/TestSequenceEngine.cpp](/F:/Work/产品出厂程序qt版/src/domain/TestSequenceEngine.cpp), [src/domain/ReportBuilder.h](/F:/Work/产品出厂程序qt版/src/domain/ReportBuilder.h), and [src/domain/ReportBuilder.cpp](/F:/Work/产品出厂程序qt版/src/domain/ReportBuilder.cpp) for business logic only.
- Create [src/infrastructure/bus/IBusController.h](/F:/Work/产品出厂程序qt版/src/infrastructure/bus/IBusController.h), [src/infrastructure/bus/ModbusRtuBusController.h](/F:/Work/产品出厂程序qt版/src/infrastructure/bus/ModbusRtuBusController.h), [src/infrastructure/bus/ModbusRtuBusController.cpp](/F:/Work/产品出厂程序qt版/src/infrastructure/bus/ModbusRtuBusController.cpp), [src/infrastructure/bus/BusManager.h](/F:/Work/产品出厂程序qt版/src/infrastructure/bus/BusManager.h), and [src/infrastructure/bus/BusManager.cpp](/F:/Work/产品出厂程序qt版/src/infrastructure/bus/BusManager.cpp) for serial and Modbus scheduling.
- Create [src/infrastructure/devices/IMotorDriveDevice.h](/F:/Work/产品出厂程序qt版/src/infrastructure/devices/IMotorDriveDevice.h), [src/infrastructure/devices/ITorqueSensorDevice.h](/F:/Work/产品出厂程序qt版/src/infrastructure/devices/ITorqueSensorDevice.h), [src/infrastructure/devices/IEncoderDevice.h](/F:/Work/产品出厂程序qt版/src/infrastructure/devices/IEncoderDevice.h), [src/infrastructure/devices/IPowerSupplyDevice.h](/F:/Work/产品出厂程序qt版/src/infrastructure/devices/IPowerSupplyDevice.h), and one adapter pair per physical device type.
- Create [src/infrastructure/simulation/SimulatedMotorDriveDevice.h](/F:/Work/产品出厂程序qt版/src/infrastructure/simulation/SimulatedMotorDriveDevice.h), [src/infrastructure/simulation/SimulatedMotorDriveDevice.cpp](/F:/Work/产品出厂程序qt版/src/infrastructure/simulation/SimulatedMotorDriveDevice.cpp), [src/infrastructure/simulation/SimulatedTorqueSensorDevice.h](/F:/Work/产品出厂程序qt版/src/infrastructure/simulation/SimulatedTorqueSensorDevice.h), [src/infrastructure/simulation/SimulatedTorqueSensorDevice.cpp](/F:/Work/产品出厂程序qt版/src/infrastructure/simulation/SimulatedTorqueSensorDevice.cpp), [src/infrastructure/simulation/SimulatedEncoderDevice.h](/F:/Work/产品出厂程序qt版/src/infrastructure/simulation/SimulatedEncoderDevice.h), [src/infrastructure/simulation/SimulatedEncoderDevice.cpp](/F:/Work/产品出厂程序qt版/src/infrastructure/simulation/SimulatedEncoderDevice.cpp), [src/infrastructure/simulation/SimulatedPowerSupplyDevice.h](/F:/Work/产品出厂程序qt版/src/infrastructure/simulation/SimulatedPowerSupplyDevice.h), and [src/infrastructure/simulation/SimulatedPowerSupplyDevice.cpp](/F:/Work/产品出厂程序qt版/src/infrastructure/simulation/SimulatedPowerSupplyDevice.cpp) for deterministic fake devices.
- Create [src/infrastructure/config/ConfigLoader.h](/F:/Work/产品出厂程序qt版/src/infrastructure/config/ConfigLoader.h), [src/infrastructure/config/ConfigLoader.cpp](/F:/Work/产品出厂程序qt版/src/infrastructure/config/ConfigLoader.cpp), [src/infrastructure/config/StationRuntimeFactory.h](/F:/Work/产品出厂程序qt版/src/infrastructure/config/StationRuntimeFactory.h), and [src/infrastructure/config/StationRuntimeFactory.cpp](/F:/Work/产品出厂程序qt版/src/infrastructure/config/StationRuntimeFactory.cpp) to parse JSON and construct station runtimes.
- Create [src/viewmodels/MainViewModel.h](/F:/Work/产品出厂程序qt版/src/viewmodels/MainViewModel.h), [src/viewmodels/MainViewModel.cpp](/F:/Work/产品出厂程序qt版/src/viewmodels/MainViewModel.cpp), [src/viewmodels/TestExecutionViewModel.h](/F:/Work/产品出厂程序qt版/src/viewmodels/TestExecutionViewModel.h), [src/viewmodels/TestExecutionViewModel.cpp](/F:/Work/产品出厂程序qt版/src/viewmodels/TestExecutionViewModel.cpp), and the child ViewModels listed in the design spec.
- Create [src/ui/pages/TestExecutionPage.qml](/F:/Work/产品出厂程序qt版/src/ui/pages/TestExecutionPage.qml) and the QML components under [src/ui/components](/F:/Work/产品出厂程序qt版/src/ui/components).
- Create [config/app.json](/F:/Work/产品出厂程序qt版/config/app.json) and [config/stations.json](/F:/Work/产品出厂程序qt版/config/stations.json) with simulation-mode defaults that allow the app to boot without hardware.
- Create [tests/domain/TestSequenceEngineTests.cpp](/F:/Work/产品出厂程序qt版/tests/domain/TestSequenceEngineTests.cpp), [tests/domain/ReportBuilderTests.cpp](/F:/Work/产品出厂程序qt版/tests/domain/ReportBuilderTests.cpp), [tests/infrastructure/ModbusFrameTests.cpp](/F:/Work/产品出厂程序qt版/tests/infrastructure/ModbusFrameTests.cpp), [tests/infrastructure/BusManagerTests.cpp](/F:/Work/产品出厂程序qt版/tests/infrastructure/BusManagerTests.cpp), [tests/infrastructure/ConfigLoaderTests.cpp](/F:/Work/产品出厂程序qt版/tests/infrastructure/ConfigLoaderTests.cpp), [tests/simulation/StationRuntimeFactoryTests.cpp](/F:/Work/产品出厂程序qt版/tests/simulation/StationRuntimeFactoryTests.cpp), [tests/viewmodels/TestExecutionViewModelTests.cpp](/F:/Work/产品出厂程序qt版/tests/viewmodels/TestExecutionViewModelTests.cpp), and [tests/ui/QmlSmokeTests.cpp](/F:/Work/产品出厂程序qt版/tests/ui/QmlSmokeTests.cpp).

## Milestones

### Milestone 1: Buildable Qt skeleton with a passing smoke test

At the end of this milestone, the repository has a valid Qt 6.11.0 CMake project, the executable starts, and at least one automated smoke test proves that QML loading is wired correctly. This milestone exists to eliminate environment risk early. Run the build and the smoke test after every change in this milestone. Accept the milestone only when a clean checkout can be configured and built with the commands in `Concrete Steps`, and when `ctest` reports the smoke test passing.

### Milestone 2: Domain flow and report logic proven by tests

At the end of this milestone, the approved test flow exists in code as a state machine with deterministic transitions, timing, verdict rules, and report generation. This is the first milestone where `@test-driven-development` must be applied rigorously: write a failing domain test for each behavior, watch it fail, implement only enough code to pass, and rerun the domain suite. Accept the milestone when the domain tests pass without requiring any UI or serial hardware.

### Milestone 3: Simulation mode runtime

At the end of this milestone, the application can assemble a simulated station runtime that feeds the flow engine and ViewModels with realistic values. This proves the end-to-end shape of the architecture before serial communication is introduced. Accept the milestone when the runtime factory can create a simulation station from `config/app.json`, when simulation tests pass, and when the domain engine can complete a full run using only simulated devices.

### Milestone 4: MVVM layer and responsive UI

At the end of this milestone, the native Qt application visually replaces `motor.html` for the main page. The UI must be responsive at the four approved target resolutions, and all operator-facing controls on the test-execution page must be bound to ViewModel state. Accept the milestone when `qmllint` passes, the UI smoke test loads `TestExecutionPage.qml`, the app can be run in simulation mode, and manual checks at `1366x768`, `1600x900`, `1920x1080`, and `2560x1440` confirm there is no critical clipping.

### Milestone 5: Real-device infrastructure

At the end of this milestone, the system can talk Modbus RTU over multiple serial buses, expose device status to the UI, and map device-specific registers into capability interfaces. This milestone is complete when CRC, frame packing, timeout classification, and adapter mapping all have automated coverage and the runtime factory can create real-mode objects from `stations.json`.

### Milestone 6: Final wiring, acceptance, and deployment readiness

At the end of this milestone, the application boots in either simulation or real mode from configuration, produces a report, distinguishes communication failures from product verdict failures, and has a documented path for packaging with `windeployqt`. Accept the milestone only after the complete automated suite passes and the operator acceptance checklist has been executed.

## Plan of Work

Start by creating the build skeleton with test support. In [CMakeLists.txt](/F:/Work/产品出厂程序qt版/CMakeLists.txt), set `CMAKE_CXX_STANDARD 20`, call `qt_standard_project_setup()`, configure a single app target named `gearbox-factory-test`, configure a runtime output directory of `build/<config>/bin`, and register one test executable per logical suite rather than one giant test binary. Link the app to `Qt6::Quick`, `Qt6::Charts`, and `Qt6::SerialPort`. Link the tests to `Qt6::Test`, and where QML loading is involved also link `Qt6::Quick`. Add a `qt_add_qml_module()` block for the application UI so QML imports are resolved through the build system instead of ad-hoc runtime paths.

Before writing any production code, create [tests/ui/QmlSmokeTests.cpp](/F:/Work/产品出厂程序qt版/tests/ui/QmlSmokeTests.cpp) with a failing test that expects the root page to load without errors. The minimal first test should construct `QQmlApplicationEngine`, load the app’s main QML entry point, and assert `!engine.rootObjects().isEmpty()`. Run only that test with `ctest -R QmlSmokeTests --output-on-failure` and confirm it fails because the QML module or app shell is missing. Then create the smallest amount of bootstrapping code to make it pass: `main.cpp`, a minimal bootstrapper, a root QML file, and the QML module registration.

Once the app starts, move into the domain layer. In [tests/domain/TestSequenceEngineTests.cpp](/F:/Work/产品出厂程序qt版/tests/domain/TestSequenceEngineTests.cpp), add one failing test per fundamental behavior: initial phase list is present, starting the test moves the first phase to running, phase completion advances to the next phase, elapsed time is recorded, and verdict classification separates communication failure from product NG. Use small deterministic fake telemetry values rather than mocks. After watching each test fail, implement `TestPhase`, `TestResult`, `LimitsProfile`, and `TestSequenceEngine` with only enough logic to satisfy the tests. Then add [tests/domain/ReportBuilderTests.cpp](/F:/Work/产品出厂程序qt版/tests/domain/ReportBuilderTests.cpp) to prove that a finished result creates a plain-text report containing model, SN, timestamp, idle results, angle results, load results, and verdict.

With the flow engine in place, build the simulation runtime. Write [tests/simulation/StationRuntimeFactoryTests.cpp](/F:/Work/产品出厂程序qt版/tests/simulation/StationRuntimeFactoryTests.cpp) first so that the code must prove a simulated station can be created from config and exposes all required device capabilities. Then implement the simulated device classes under `src/infrastructure/simulation`. Each simulation device should be deterministic enough for tests but lively enough to drive the UI. The simulated motor drive should accept forward, reverse, stop, and brake commands; the simulated torque sensor and encoder should produce values consistent with the current phase; and the simulated power supply should store setpoints and expose readback values. Implement `StationRuntimeFactory` so that simulation mode creates these fake devices while real mode remains unimplemented until Milestone 5.

After the simulation runtime works, create the ViewModels. Write [tests/viewmodels/TestExecutionViewModelTests.cpp](/F:/Work/产品出厂程序qt版/tests/viewmodels/TestExecutionViewModelTests.cpp) first. The first failing tests should prove that invalid SN blocks `startTest()`, that valid input locks the form, that stage states project into a list model, that metrics are formatted with units, that toggling chart channels updates the exposed state, and that a finished run produces a copyable report string. Only after these tests fail should `MainViewModel`, `TestExecutionViewModel`, `CommandBarViewModel`, `StepListViewModel`, `MetricsViewModel`, `ChartViewModel`, `JudgmentViewModel`, and `DeviceStatusViewModel` be implemented.

With the ViewModels stable, implement the QML UI. Create [src/ui/pages/TestExecutionPage.qml](/F:/Work/产品出厂程序qt版/src/ui/pages/TestExecutionPage.qml) with a `ColumnLayout` root containing the title bar, content body, info bar, and status bar. Split child sections into the component files already listed in the file map. Store shared sizes, colors, spacing, and breakpoint decisions in a small QML style singleton such as [src/ui/components/AppTheme.qml](/F:/Work/产品出厂程序qt版/src/ui/components/AppTheme.qml) if the file is helpful; keep it focused on theme constants rather than behavior. Recreate the approved three-column structure, then bind every operator-visible field to its ViewModel counterpart. Use `QtCharts` for the waveform panel. Run `qmllint` after each component batch rather than waiting until the end.

Once the UI runs in simulation mode, add the real communication layer. Begin with [tests/infrastructure/ModbusFrameTests.cpp](/F:/Work/产品出厂程序qt版/tests/infrastructure/ModbusFrameTests.cpp). The first failing tests should prove CRC16 generation matches known values from the manuals, request frames are encoded correctly, exception responses are decoded, and timeout conditions become typed communication failures. Then implement `IBusController`, `ModbusRtuBusController`, and `BusManager`. Create one adapter class per physical device family and write adapter tests that prove the correct register addresses and scaling are used. Use the manuals in `Docs` as the single source of truth. Do not embed magic numbers in ViewModels or domain code.

Finally, complete configuration and final app wiring. Write [tests/infrastructure/ConfigLoaderTests.cpp](/F:/Work/产品出厂程序qt版/tests/infrastructure/ConfigLoaderTests.cpp) before implementing `ConfigLoader`. The test should prove that `config/app.json` defaults to simulation mode and that `config/stations.json` can map devices across three buses. Then complete `ApplicationBootstrapper` so command-line overrides and config defaults select the correct mode. Add the final acceptance commands and packaging steps, then run the full suite and document the results back into `Progress` and `Outcomes & Retrospective`.

## Concrete Steps

All commands below are run from `F:\Work\产品出厂程序qt版` in PowerShell.

Before the first configure step in a new shell, prepend the Qt and MinGW bins to `PATH`:

    $env:PATH = 'F:\Qt\6.11.0\mingw_64\bin;F:\Qt\Tools\mingw1310_64\bin;' + $env:PATH

Configure the project after [CMakeLists.txt](/F:/Work/产品出厂程序qt版/CMakeLists.txt) exists:

    cmake -S . -B build\mingw-debug -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=F:/Qt/6.11.0/mingw_64

Expected result:

    -- Configuring done
    -- Generating done
    -- Build files have been written to: F:/Work/产品出厂程序qt版/build/mingw-debug

Build after each TDD cycle:

    cmake --build build\mingw-debug --parallel

Run all tests after each milestone:

    ctest --test-dir build\mingw-debug --output-on-failure

Run an individual suite during red/green work:

    ctest --test-dir build\mingw-debug -R QmlSmokeTests --output-on-failure
    ctest --test-dir build\mingw-debug -R TestSequenceEngineTests --output-on-failure
    ctest --test-dir build\mingw-debug -R TestExecutionViewModelTests --output-on-failure
    ctest --test-dir build\mingw-debug -R ModbusFrameTests --output-on-failure

Lint QML after the UI milestone starts:

    F:\Qt\6.11.0\mingw_64\bin\qmllint.exe src\ui\pages\TestExecutionPage.qml

Run the application in simulation mode once the app target is wired:

    .\build\mingw-debug\bin\gearbox-factory-test.exe --mode simulation

Expected operator-visible result:

    The window opens on the “测试执行” page, shows “工位 #03” by default, and allows the full phase flow to run without attached hardware.

Run the application in real mode once `stations.json` is valid and the hardware is attached:

    .\build\mingw-debug\bin\gearbox-factory-test.exe --mode real --station station-03

Expected operator-visible result:

    The window opens with real bus/device status. If a configured device is missing, the UI reports a communication failure rather than silently staying idle.

Package the app after acceptance using `windeployqt`:

    F:\Qt\6.11.0\mingw_64\bin\windeployqt.exe .\build\mingw-debug\bin\gearbox-factory-test.exe

Commit after each milestone:

    git add .
    git commit -m "feat: scaffold qt gearbox test app"

    git add .
    git commit -m "feat: add test sequence domain and simulation runtime"

    git add .
    git commit -m "feat: implement qml mvvm test execution page"

    git add .
    git commit -m "feat: add modbus multi-bus device integration"

## Validation and Acceptance

Validation is complete only when all of the following are true.

Automated validation:

- `ctest --test-dir build\mingw-debug --output-on-failure` passes with no failing test suites.
- `QmlSmokeTests` proves the root QML page loads.
- `TestSequenceEngineTests` proves stage transitions, timing, and verdict classification.
- `ReportBuilderTests` prove report formatting.
- `StationRuntimeFactoryTests` prove simulation runtime assembly.
- `TestExecutionViewModelTests` prove command validation, list projection, metrics formatting, chart toggles, and report projection.
- `ModbusFrameTests`, `BusManagerTests`, and adapter tests prove serial protocol correctness and communication failure classification.
- `ConfigLoaderTests` prove JSON parsing and multi-bus station mapping.

Manual simulation acceptance:

1. Start the app with `--mode simulation`.
2. Confirm the screen layout matches the approved structure from [motor.html](/F:/Work/产品出厂程序qt版/Docs/motor.html): title bar, command bar, left step list, center metrics/chart, right verdict panel, bottom status bar.
3. Enter a valid SN and start the test.
4. Verify the phase list advances through all five phases.
5. Verify the center metrics and waveform update while the test is running.
6. Verify the angle table appears during the angle phase and the load table appears during the load phase.
7. Verify the verdict box changes from pending to `OK` or `NG`.
8. Verify “复制报告” returns a report string containing model, SN, verdict, idle results, angle results, and load results.
9. Verify “重置测试” returns the UI to the pending state.

Manual real-mode acceptance, if hardware is available:

1. Configure `config/stations.json` with the actual COM ports and device addresses.
2. Start the app with `--mode real --station station-03`.
3. Confirm each configured bus reports online status or a clear failure.
4. Start the test and verify actual telemetry changes when the physical devices move.
5. Disconnect one configured device or use a deliberately incorrect station address.
6. Verify the UI shows a communication failure and does not mislabel the product as an `NG` part.

Responsive UI acceptance:

- Run or resize the app at `1366x768`, `1600x900`, `1920x1080`, and `2560x1440`.
- At each size, confirm the start/stop controls remain visible, the verdict panel remains readable, and no core control becomes unreachable.
- If a low-height layout requires scrolling, it must occur inside the left or right panel, not by clipping the entire application window.

## Idempotence and Recovery

This plan is intentionally additive. Re-running the configure, build, test, and lint commands is safe. If a milestone fails partway through, do not delete unrelated files; instead, revert only the last local changes in the files touched by that milestone after understanding them. Keep `config/app.json` defaulted to simulation mode so the application can always start without hardware. If real-mode validation is blocked because hardware is unavailable, complete the simulation acceptance, leave the real-mode verification item unchecked in `Progress`, and document the exact missing hardware dependency in `Outcomes & Retrospective`.

When editing config files during real-mode testing, preserve a working simulation default. That means the committed `config/app.json` should keep `"mode": "simulation"` unless the project owner explicitly wants real mode as the default. Local overrides for specific hardware runs should be documented but not silently committed as the new default.

## Artifacts and Notes

The first domain tests should use concise, deterministic expectations. For example, [tests/domain/TestSequenceEngineTests.cpp](/F:/Work/产品出厂程序qt版/tests/domain/TestSequenceEngineTests.cpp) should contain a test equivalent to:

    void TestSequenceEngineTests::startsWithPrepPhaseRunning()
    {
        TestSequenceEngine engine(defaultLimits());
        const auto result = engine.start(validInput());
        QCOMPARE(result.currentPhaseId(), QStringLiteral("prep"));
        QCOMPARE(result.phaseStatus("prep"), PhaseStatus::Running);
    }

The first ViewModel tests should include a validation case equivalent to:

    void TestExecutionViewModelTests::emptySnBlocksStart()
    {
        TestExecutionViewModel vm(...);
        vm.commandBar()->setSerialNumber(QString());
        QVERIFY(!vm.canStart());
        QCOMPARE(vm.commandBar()->validationMessage(), QStringLiteral("SN 不能为空"));
    }

The first Modbus test should assert a known CRC from the manuals, for example:

    void ModbusFrameTests::encoderReadRequestMatchesManual()
    {
        const QByteArray frame = ModbusFrame::readHoldingRegisters(0x01, 0x0000, 0x0001);
        QCOMPARE(frame.toHex(' ').toUpper(), QByteArray("01 03 00 00 00 01 84 0A"));
    }

Keep these snippets small. The production implementation should remain in the source tree, not in this plan.

## Interfaces and Dependencies

The following types or their exact-responsibility equivalents must exist by the end of implementation.

In [src/domain/TestSequenceEngine.h](/F:/Work/产品出厂程序qt版/src/domain/TestSequenceEngine.h), define a stateful flow engine with an interface equivalent to:

    class TestSequenceEngine {
    public:
        explicit TestSequenceEngine(LimitsProfile limits);
        TestRunSnapshot start(const TestInput &input);
        TestRunSnapshot tick(const TelemetrySnapshot &telemetry, std::chrono::milliseconds dt);
        TestRunSnapshot emergencyStop();
        TestRunSnapshot reset();
        const TestRunSnapshot &snapshot() const;
    };

In [src/infrastructure/config/StationRuntimeFactory.h](/F:/Work/产品出厂程序qt版/src/infrastructure/config/StationRuntimeFactory.h), define a runtime factory equivalent to:

    class StationRuntimeFactory {
    public:
        StationRuntimeFactory(ConfigLoader &loader);
        std::unique_ptr<StationRuntime> create(const AppConfig &appConfig, const QString &stationId);
    };

In [src/viewmodels/TestExecutionViewModel.h](/F:/Work/产品出厂程序qt版/src/viewmodels/TestExecutionViewModel.h), define a ViewModel equivalent to:

    class TestExecutionViewModel : public QObject {
        Q_OBJECT
        Q_PROPERTY(CommandBarViewModel* commandBar READ commandBar CONSTANT)
        Q_PROPERTY(StepListViewModel* stepList READ stepList CONSTANT)
        Q_PROPERTY(MetricsViewModel* metrics READ metrics CONSTANT)
        Q_PROPERTY(ChartViewModel* chart READ chart CONSTANT)
        Q_PROPERTY(JudgmentViewModel* judgment READ judgment CONSTANT)
        Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    public:
        Q_INVOKABLE void startTest();
        Q_INVOKABLE void emergencyStop();
        Q_INVOKABLE void resetTest();
        Q_INVOKABLE QString buildReport() const;
    };

In [src/infrastructure/bus/IBusController.h](/F:/Work/产品出厂程序qt版/src/infrastructure/bus/IBusController.h), define an abstract bus controller equivalent to:

    class IBusController {
    public:
        virtual ~IBusController() = default;
        virtual QString busId() const = 0;
        virtual bool isOnline() const = 0;
        virtual ModbusReply transact(const ModbusRequest &request) = 0;
    };

Required Qt modules:

- `Qt6::Quick`
- `Qt6::Charts`
- `Qt6::SerialPort`
- `Qt6::Test`

Required local toolchain paths:

- `F:\Qt\6.11.0\mingw_64`
- `F:\Qt\Tools\mingw1310_64`
- `C:\Program Files\CMake\bin\cmake.exe`

During implementation, apply `@test-driven-development` before every production change and `@verification-before-completion` before claiming the feature is finished.

## Change Note

2026-04-15: Initial implementation plan created to convert the approved design specification into a concrete ExecPlan with exact file targets, TDD-first milestones, reproducible Qt 6.11.0 MinGW build commands, and acceptance criteria for simulation mode, real mode, and multiple screen resolutions.

2026-04-15: Updated after the first implementation pass to reflect the upgraded Qt skeleton, passing `QmlSmokeTests`, Windows test-runtime discovery fix, and the decision to extract shared QML components instead of building the entire `motor.html` replica inside one page file.

2026-04-16: Updated after extracting `AppShell.qml`, moving reusable component examples into `ComponentGalleryPage.qml`, rewriting smoke tests to target the gallery page, forcing the `Basic` Quick Controls style, and documenting the remaining MVVM/runtime migration gap.

2026-04-16: Updated after the second parallel component wave landed, expanding the gallery and smoke suite to 23 passing checks, and recording the multi-agent integration process plus the `Loader` delegate indexing discovery in `AppButtonGroup.qml`.

2026-04-16: Updated after the third main-thread component wave landed, expanding the gallery and smoke suite to 27 passing checks with table/input/navigation/drawer-related primitives.

2026-04-16: Updated after the fourth main-thread batch landed, adding calendar/carousel/resizable/aspect-ratio and deepening slider/table while keeping the gallery and smoke suite green.

2026-04-16: Updated after the follow-up verification/fix pass raised the smoke suite to 29 passing checks and stabilized the range-slider/gallery wiring.
