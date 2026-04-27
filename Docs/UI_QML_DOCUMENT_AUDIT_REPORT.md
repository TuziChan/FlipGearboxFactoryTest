# UI/QML 文档过时核查报告

**核查日期**: 2026-04-24  
**核查范围**: 项目根目录及 Docs/ 目录下 6 份 Qt6/QML 相关文档  
**代码对照**: src/ui/ 下全部 92 个 QML 文件

---

## 核查结论总览

| 序号 | 文档名 | 是否过时 | 建议处理方式 |
|------|--------|----------|--------------|
| 1 | QT6_COMPREHENSIVE_REPORT.md | **是，大部分过时** | 归档 |
| 2 | QT6_MODERNIZATION_PLAN.md | **是，部分过时** | 更新或归档 |
| 3 | QT6_OPTIMIZATION_SUMMARY.md | **是，部分过时** | 更新 |
| 4 | QT6_UI_AUDIT_REPORT.md | **是，大部分过时** | 归档 |
| 5 | QT6_UI_AUDIT_REPORT_FINAL.md | **是，部分过时** | 归档 |
| 6 | Docs/QT6.11_FEATURE_MAPPING.md | **是，部分过时** | 更新 |

---

## 详细核查结果

### 1. QT6_COMPREHENSIVE_REPORT.md（项目根目录，2026-04-19）

**状态**: 过时 ⛔

**过时原因**:
- **pragma ComponentBehavior: Bound 统计完全错误**：文档声称“40+ 个组件缺少此 pragma”，但经 grep 核查，src/ui/ 下全部 **92 个 QML 文件** 均已添加该 pragma（包括所有 App* 组件、页面及自定义组件）。此问题已在文档生成后的某次提交中被批量修复。
- **“已修复”列表包含未真正修复的文件**：
  - `FlowStepItem.qml`（第 26 行）仍在 `ColumnLayout` 内部使用 `anchors.fill: parent` + `anchors.leftMargin` 等混合格式。
  - `DataTableCard.qml`（第 33–34 行）仍在 `ColumnLayout` 内部使用 `anchors.fill: parent` + `anchors.margins: 6`。
- **“仍需修复”的 24 文件列表**虽然仍有不少文件存在 anchors 混用问题（组件目录下 grep 出约 100 处 `anchors.fill: parent`），但报告中的阶段估计（2–3 小时）和具体文件名列表已不能反映当前真实进度。

**建议处理方式**: 归档至 `Docs/drafts/` 或 `.history/`，后续若需追溯 Qt6 迁移历史可查阅。

---

### 2. QT6_MODERNIZATION_PLAN.md（项目根目录，2026-04-19）

**状态**: 部分过时 ⚠️

**过时原因**:
- 与上份报告同样错误地声称“40+ 文件缺少 pragma ComponentBehavior: Bound”→ 实际全部已有。
- **阶段 2 “已完成”文件不准确**：`FlowStepItem.qml`、`DataTableCard.qml` 的 Layout 混用问题并未真正解决（见上文）。
- **阶段 3 AppShell 改为 ApplicationWindow**：当前 `AppShell.qml` 根元素仍为 `Item`（第 7 行），该建议尚未实施；文档未说明这是“未来计划”还是“已完成”，容易误导。

**仍准确的内容**:
- Layout 系统分析（Column vs ColumnLayout 的对比说明）。
- Qt 6 导入语句风格说明。

**建议处理方式**: 更新。删除已过时的 pragma 统计，修正“已完成”文件列表，补充 AppShell 现代化进度说明。

---

### 3. QT6_OPTIMIZATION_SUMMARY.md（项目根目录，2026-04-19）

**状态**: 部分过时 ⚠️

**过时原因**:
- **声称已完成的优化与实际不符**：
  - “FlowStepItem.qml - Column 改为 ColumnLayout” → 确实改成了 `ColumnLayout`，但**同时保留了 `anchors.fill: parent`**（第 26 行），优化不彻底。
  - “DataTableCard.qml - Column 改为 ColumnLayout” → 同理，`anchors.fill: parent` + `anchors.margins: 6` 仍在 `ColumnLayout` 内部（第 33–34 行）。
- **FlowList.qml 使用 Column “合理”的说法不严谨**：`FlowList.qml` 第 13 行在 `Column` 根元素上写了 `Layout.fillWidth: true`。`Column` 作为定位器（Positioner）不支持 Layout 附加属性，该行无效；若 `FlowList` 被放入 `ColumnLayout` 等布局容器中，无法正确参与尺寸协商。
- **StatusList.qml 使用 Column “合理”**：基本合理，但其 `delegate` 是 `RowLayout`，若 `StatusList` 本身被放入 `Layout` 上下文，`Column` 根元素同样不支持 `Layout.fillWidth`。

**仍准确的内容**:
- `run.bat` 自动搜索 Qt 目录（C–F 盘）→ 已核实 `run.bat` 第 8–21 行确实实现了该功能。
- `TestExecutionWorkspace.qml` 的 `ScrollBar` 使用正确（第 110 行）。

**建议处理方式**: 更新。修正 FlowStepItem/DataTableCard 的优化状态为“部分完成，仍需移除 anchors 混用”；补充 FlowList 的 Layout.fillWidth 无效问题。

---

### 4. QT6_UI_AUDIT_REPORT.md（项目根目录，2026-04-19 17:29）

**状态**: 大部分过时 ⛔

**过时原因**:
- 这是一份**中间状态快照**，所列问题在后续提交中已被大量修复：
  - `FlowList.qml` 缺少 pragma → 已添加（第 1 行）。
  - `StatusList.qml` 缺少 pragma → 已添加（第 1 行）。
  - `ComponentGalleryPage.qml` Flickable 缺少 ScrollBar → 已改为 `ScrollView`（第 247 行）。
- 文档声称 `AppButton.qml` “在 Layout 中使用 anchors.fill” → 经核查，`AppButton` 根元素是 `Rectangle`，其内部子 `Rectangle`（hover 效果层）使用 `anchors.fill: parent` 是正常用法，不属于“Layout 中混用 anchors”问题。该描述属于误报。

**仍准确的内容**:
- `AlertStrip.qml`（第 20 行）确实在 `RowLayout` 内部使用 `anchors.fill: parent`。

**建议处理方式**: 归档。作为历史中间审计记录保留，但不应作为当前工作参考。

---

### 5. QT6_UI_AUDIT_REPORT_FINAL.md（项目根目录，2026-04-19 17:31）

**状态**: 部分过时 ⚠️

**过时原因**:
- 文档标题含 **“最终版”**，但实际并非最终状态，后续代码仍有变动：
  - “已完成”的 8 项中，第 5–8 项（`ComponentGalleryPage` 改 `ScrollView`、`FlowList`/`StatusList` 加 pragma、`run.bat`）确实已完成。
  - 但第 1–4 项中 `FlowStepItem`/`DataTableCard` 的 Layout 优化并不彻底（仍混用 anchors）。
- **“当前仍有问题”列表部分仍成立**：
  - `CommandBar.qml`（第 25 行）仍在 `RowLayout` 内部使用 `anchors.fill: parent` + `anchors.*Margin`，且内部仍使用 `Column`（第 37、59、79 行）而非 `ColumnLayout`。→ 描述准确。
  - `AppAccordion.qml`、`AppCalendar.qml`、`AppCollapsible.qml` 等是否仍有问题需进一步逐项核查，但基于 grep 结果，anchors 混用确实广泛存在。

**建议处理方式**: 归档。文档标注为“最终版”会误导团队成员认为审计已结束，实际上大量 Layout 问题仍待修复。

---

### 6. Docs/QT6.11_FEATURE_MAPPING.md（Docs/ 目录，2026-04-23）

**状态**: 部分过时 ⚠️

**过时原因**:
- **具体数据已不准确**：
  - 声称 `TestExecutionPage.qml` 为 **482 行** → 实际为 **533 行**（已增加 51 行，可能新增了功能或属性）。
  - 声称 UI 组件 **60+** → `src/ui/components/` 下实际有约 **73 个** QML 组件文件，加上页面共 92 个。
- **Qt 版本描述存在矛盾**：
  - 报告声称 Qt 版本 **6.11.0**，且 `run.bat` 中硬编码 `QT_VERSION=6.11.0`。
  - 但 `CMakeLists.txt`（第 15 行）写的是 `qt_standard_project_setup(REQUIRES 6.10)`。若构建系统要求 6.10，而脚本和报告统一口径 6.11.0，可能导致版本理解混乱。
- **已采用的 Qt6 特性描述基本准确**：
  - `pragma ComponentBehavior: Bound` ✅（全部已有）
  - `required property` ✅（广泛使用）
  - `qt_standard_project_setup` ✅（CMakeLists.txt 第 15 行）
  - Qt Test 框架“11 个测试套件”→ 经 grep 核查，项目中约 **10 个 QObject 测试类**，CMake 中约 **20 个 `add_test`**，数量级基本吻合。
- **推荐的 Qt 6.11 特性均未实施**：
  - QML State Machine（`TestExecutionPage.qml` 仍使用手动 `currentPhaseIndex` 字符串匹配，第 43–57 行）。
  - Property Binding 优化（仍有 `Connections` 块，第 106 行）。
  - Inline Components（未使用）。
  - `TableView` 替代 `ListModel`（未使用，仍为 `ListModel` + 手动 Repeater）。
  - `qmltc` 编译器（未启用）。
  - Value Types（未采用）。
  这些虽未实施，但文档定位为“推荐方案”，本身不算过时；不过文档声明“有效期：2026-04-23 至项目交付”，在项目临近交付时应评估是否仍具参考价值。

**建议处理方式**: 更新。修正 `TestExecutionPage.qml` 行数、组件数量、Qt 版本一致性说明；在推荐特性清单中标注当前实施状态（未开始/进行中/已完成）。

---

## 附：当前代码中仍存在的典型 Qt6 不规范问题（与文档无关，供参考）

经本次核查，src/ui/ 中仍存在以下与文档主题相关的实际代码问题：

1. **anchors 与 Layout 混用**（约 100 处）
   - `FlowStepItem.qml:26`：`ColumnLayout { anchors.fill: parent; anchors.leftMargin: 12; ... }`
   - `DataTableCard.qml:33-34`：`ColumnLayout { anchors.fill: parent; anchors.margins: 6; ... }`
   - `CommandBar.qml:25`：`RowLayout { anchors.fill: parent; anchors.leftMargin: 14; ... }`
   - `ChartPanel.qml:33`：`ColumnLayout { anchors.fill: parent; anchors.margins: 16; ... }`
   - `AlertStrip.qml:20`：`RowLayout { anchors.fill: parent; anchors.leftMargin: 18; ... }`
   - `TestExecutionWorkspace.qml:57`：`RowLayout { anchors.fill: parent; anchors.margins: 12; ... }`

2. **Column/Row 在 Layout 上下文中使用**
   - `CommandBar.qml:37,59,79`：在 `RowLayout` 内部使用 `Column` 而非 `ColumnLayout`。
   - `FlowList.qml:13`：`Column` 根元素上写 `Layout.fillWidth: true`（对 Positioner 无效）。

3. **AppShell.qml 仍为 Item 根元素**
   - `AppShell.qml:7`：`Item { ... }`，而非 `ApplicationWindow`。

---

## 建议的文档治理措施

1. **将 4 份 2026-04-19 的审计/报告类文档（QT6_COMPREHENSIVE_REPORT.md、QT6_UI_AUDIT_REPORT.md、QT6_UI_AUDIT_REPORT_FINAL.md、QT6_MODERNIZATION_PLAN.md）统一归档**，避免团队成员误将“历史审计”当作“当前状态”。
2. **更新 QT6_OPTIMIZATION_SUMMARY.md**：修正 FlowStepItem/DataTableCard 的优化状态，补充 FlowList Layout 无效属性问题。
3. **更新 Docs/QT6.11_FEATURE_MAPPING.md**：修正行数、组件数量、Qt 版本一致性；在每项推荐特性后追加 `[未实施]` 标签，避免重复评估。
4. **新建一份轻量级的 `UI_QML_CODING_STANDARDS.md`**：仅保留当前有效的编码规范（pragma、import 风格、Layout 规则），替代多份历史报告。
