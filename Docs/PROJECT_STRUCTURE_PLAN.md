# FlipGearboxFactoryTest 项目目录结构规范方案

**文档版本**: v1.0  
**创建日期**: 2026-04-25  
**负责人**: 头脑风暴角色  
**状态**: 待评审

---

## 一、当前状况分析

### 1.1 根目录混乱现状

截至 2026-04-25，项目根目录存在 **32 个文件**，包括：

| 文件类型 | 数量 | 示例 |
|---------|------|------|
| Markdown 文档 | 9 | `README.md`, `QT6_COMPREHENSIVE_REPORT.md`, `EXECPLAN_DiagnosticsRedesign.md` |
| PowerShell 脚本 | 3 | `build-verify.ps1`, `run-dev.ps1`, `run-test.ps1` |
| 批处理脚本 | 3 | `build-verify.bat`, `run-dev.bat`, `run-test.bat` |
| 测试文件 | 6 | `test_*.cpp`, `*_Review.txt` |
| 构建配置 | 2 | `CMakeLists.txt`, `.gitignore` |
| 许可证 | 1 | `LICENSE` |
| QML 入口 | 1 | `Main.qml` |
| C++ 入口 | 1 | `main.cpp` |
| 其他 | 6 | `.clang-format`, `.clang-tidy`, 等 |

**核心问题**：
- ❌ 技术文档散落根目录，缺乏分类（9个MD文档）
- ❌ 脚本文件未按用途分类（构建/开发/测试混在一起）
- ❌ 测试产物和审查记录未隔离（污染版本控制）
- ❌ 入口文件（main.cpp/Main.qml）与源码目录分离
- ❌ 缺少临时文件隔离区（temp/）

### 1.2 Docs 目录文档冗余

`Docs/` 目录现有 **51 个 Markdown 文档**，存在以下问题：

| 问题类型 | 数量 | 示例 |
|---------|------|------|
| 重复的 Qt6 优化文档 | 5 | `QT6_OPTIMIZATION_SUMMARY.md`, `QT6_MODERNIZATION_PLAN.md` |
| 重复的模拟框架文档 | 5 | `SimulationTestFramework*.md`, `MockDeviceSimulator.md` |
| 临时分析报告 | 4 | `Documentation_System_Analysis_Report.md`, `Duplicate_Documents_List.md` |
| 已过时文档（已删除） | 3 | `infrastructure_improvements.md`, `MockDeviceImplementationSummary.md` |

**已完成清理**（P0 优先级）：
- ✅ 删除 `Docs/infrastructure_improvements.md`（过时的基础设施改进计划）
- ✅ 删除 `Docs/MockDeviceImplementationSummary.md`（已被 SIMULATION_GUIDE.md 替代）
- ✅ 删除 `Docs/MockDeviceSimulator.md`（已被 SIMULATION_GUIDE.md 替代）

---

## 二、目标目录结构设计

### 2.1 根目录精简原则

**目标**：根目录仅保留 **5 个核心文件**

```
FlipGearboxFactoryTest/
├── README.md                    # 项目概览（中文）
├── README.zh-CN.md              # 项目概览（英文）
├── LICENSE                      # 开源许可证
├── CMakeLists.txt               # CMake 构建配置
└── .gitignore                   # Git 忽略规则
```

### 2.2 完整目录结构

```
FlipGearboxFactoryTest/
├── src/                         # 源代码（已存在，保持不变）
│   ├── domain/                  # 领域层
│   ├── infrastructure/          # 基础设施层
│   ├── ui/                      # UI 层（QML 组件）
│   └── viewmodels/              # ViewModel 层
│
├── qml/                         # QML 入口文件（新建）
│   └── Main.qml                 # 应用主入口
│
├── app/                         # 应用入口（新建）
│   └── main.cpp                 # C++ 主函数
│
├── scripts/                     # 脚本工具（新建）
│   ├── build/                   # 构建脚本
│   │   ├── build-verify.ps1
│   │   └── build-verify.bat
│   ├── dev/                     # 开发脚本
│   │   ├── run-dev.ps1
│   │   └── run-dev.bat
│   └── test/                    # 测试脚本
│       ├── run-test.ps1
│       └── run-test.bat
│
├── temp/                        # 临时文件（新建，加入 .gitignore）
│   ├── test_*.cpp               # 临时测试文件
│   └── *_Review.txt             # 代码审查记录
│
├── Docs/                        # 技术文档（重组）
│   ├── user/                    # 用户文档（新建）
│   │   ├── USER_MANUAL.md
│   │   └── QUICK_START.md
│   ├── development/             # 开发文档（新建）
│   │   ├── ARCHITECTURE.md
│   │   ├── IMPLEMENTATION_PROGRESS.md
│   │   └── CODING_STANDARDS.md
│   ├── hardware/                # 硬件文档（新建）
│   │   ├── Device_Register_Reference_Guide.md
│   │   └── MODBUS_PROTOCOL.md
│   ├── reports/                 # 技术报告（新建）
│   │   ├── QT6_FEATURE_MAPPING.md
│   │   └── SIMULATION_GUIDE.md
│   └── archive/                 # 历史归档（已存在）
│       └── ...
│
├── tests/                       # 单元测试（已存在，保持不变）
└── build/                       # 构建输出（已存在，保持不变）
```

---

## 三、文件迁移清单

### 3.1 根目录文件迁移（P0 优先级）

| 源路径 | 目标路径 | 迁移原因 |
|--------|---------|---------|
| `main.cpp` | `app/main.cpp` | 应用入口应独立于源码目录 |
| `Main.qml` | `qml/Main.qml` | QML 入口应独立管理 |
| `build-verify.ps1` | `scripts/build/build-verify.ps1` | 按用途分类脚本 |
| `build-verify.bat` | `scripts/build/build-verify.bat` | 按用途分类脚本 |
| `run-dev.ps1` | `scripts/dev/run-dev.ps1` | 按用途分类脚本 |
| `run-dev.bat` | `scripts/dev/run-dev.bat` | 按用途分类脚本 |
| `run-test.ps1` | `scripts/test/run-test.ps1` | 按用途分类脚本 |
| `run-test.bat` | `scripts/test/run-test.bat` | 按用途分类脚本 |
| `test_*.cpp` | `temp/test_*.cpp` | 临时测试文件应隔离 |
| `*_Review.txt` | `temp/*_Review.txt` | 审查记录应隔离 |

### 3.2 根目录 MD 文档迁移（P1 优先级）

| 源路径 | 目标路径 | 分类依据 |
|--------|---------|---------|
| `QT6_COMPREHENSIVE_REPORT.md` | `Docs/reports/QT6_COMPREHENSIVE_REPORT.md` | 技术报告 |
| `QT6_MODERNIZATION_PLAN.md` | `Docs/reports/QT6_MODERNIZATION_PLAN.md` | 技术报告 |
| `QT6_OPTIMIZATION_SUMMARY.md` | `Docs/reports/QT6_OPTIMIZATION_SUMMARY.md` | 技术报告 |
| `QT6_UI_AUDIT_REPORT.md` | `Docs/reports/QT6_UI_AUDIT_REPORT.md` | 技术报告 |
| `QT6_UI_AUDIT_REPORT_FINAL.md` | `Docs/reports/QT6_UI_AUDIT_REPORT_FINAL.md` | 技术报告 |
| `EXECPLAN_DiagnosticsRedesign.md` | `Docs/development/EXECPLAN_DiagnosticsRedesign.md` | 开发计划 |
| `ARCHITECTURE.md` | `Docs/development/ARCHITECTURE.md` | 架构文档 |
| `IMPLEMENTATION_PROGRESS.md` | `Docs/development/IMPLEMENTATION_PROGRESS.md` | 进度追踪 |
| `SIMULATION_GUIDE.md` | `Docs/reports/SIMULATION_GUIDE.md` | 技术指南 |

### 3.3 Docs 目录内部重组（P1 优先级）

**需要合并的文档**：

| 合并目标 | 源文档 | 合并原因 |
|---------|--------|---------|
| `Docs/reports/SIMULATION_GUIDE.md` | `Docs/MOCK_MODE_GUIDE.md` | 模拟模式和仿真指南内容重叠 |
| `Docs/reports/QT6_FEATURE_MAPPING.md` | `Docs/archive/QT6_OPTIMIZATION_SUMMARY.md`<br>`Docs/archive/QT6_MODERNIZATION_PLAN.md` | Qt6 优化文档重复 |
| `Docs/hardware/Device_Register_Reference_Guide.md` | 各设备手册的寄存器章节 | 统一设备寄存器参考 |

**可删除的临时分析文档**（P2 优先级，完成整理后）：
- `Docs/Documentation_System_Analysis_Report.md`
- `Docs/Documentation_Coverage_Gap_Analysis.md`
- `Docs/Duplicate_Documents_List.md`
- `Docs/Obsolete_Documents_List.md`

---

## 四、目录命名和组织原则

### 4.1 命名规范

1. **英文命名优先**：目录名使用英文（除非业务特定术语）
2. **小写+下划线**：`test_artifacts`、`user_manual`（避免驼峰）
3. **功能分组**：按职能（user/dev/hardware）而非时间（2026-04-xx）
4. **临时隔离**：`temp/` 目录不纳入版本控制（添加到 .gitignore）
5. **归档明确**：`archive/` 目录仅保留历史参考，不参与日常开发

### 4.2 文档分类标准

| 分类 | 目录 | 内容类型 | 受众 |
|------|------|---------|------|
| 用户文档 | `Docs/user/` | 用户手册、快速入门、操作指南 | 终端用户、测试工程师 |
| 开发文档 | `Docs/development/` | 架构设计、编码规范、实施计划 | 开发团队 |
| 硬件文档 | `Docs/hardware/` | 设备手册、协议规范、寄存器参考 | 硬件工程师、协议开发 |
| 技术报告 | `Docs/reports/` | 技术调研、优化报告、测试指南 | 技术决策者 |
| 历史归档 | `Docs/archive/` | 过时文档、历史版本 | 仅供参考 |

---

## 五、实施步骤

### 阶段 1：根目录清理（P0 优先级）

**目标**：将根目录从 32 个文件精简到 5 个核心文件

**步骤**：
1. 创建新目录：
   ```bash
   mkdir -p app qml scripts/{build,dev,test} temp
   ```

2. 移动入口文件（使用 `git mv` 保留历史）：
   ```bash
   git mv main.cpp app/main.cpp
   git mv Main.qml qml/Main.qml
   ```

3. 移动脚本文件：
   ```bash
   git mv build-verify.ps1 scripts/build/
   git mv build-verify.bat scripts/build/
   git mv run-dev.ps1 scripts/dev/
   git mv run-dev.bat scripts/dev/
   git mv run-test.ps1 scripts/test/
   git mv run-test.bat scripts/test/
   ```

4. 移动临时文件：
   ```bash
   mv test_*.cpp temp/
   mv *_Review.txt temp/
   ```

5. 更新 `.gitignore`：
   ```gitignore
   # 临时文件目录
   /temp/
   
   # 测试产物
   *.exe
   test_*.cpp
   *_Review.txt
   
   # 构建输出
   /build/
   /out/
   
   # IDE 配置
   .vscode/
   .idea/
   *.user
   ```

6. 更新 `CMakeLists.txt`：
   - 修改 `add_executable` 路径：`main.cpp` → `app/main.cpp`
   - 修改 QML 资源路径：`Main.qml` → `qml/Main.qml`

**验证**：
- 运行 `cmake --build build` 确保构建成功
- 运行 `./build/FlipGearboxFactoryTest` 确保应用启动正常

**已完成项**：
- ✅ 删除 3 个过时文档（`infrastructure_improvements.md`, `MockDeviceImplementationSummary.md`, `MockDeviceSimulator.md`）

---

### 阶段 2：文档结构化（P1 优先级）

**目标**：重组 Docs 目录，建立清晰的文档分类体系

**步骤**：
1. 创建文档分类目录：
   ```bash
   mkdir -p Docs/{user,development,hardware,reports}
   ```

2. 移动根目录 MD 文档到 Docs：
   ```bash
   git mv QT6_*.md Docs/reports/
   git mv EXECPLAN_DiagnosticsRedesign.md Docs/development/
   git mv ARCHITECTURE.md Docs/development/
   git mv IMPLEMENTATION_PROGRESS.md Docs/development/
   git mv SIMULATION_GUIDE.md Docs/reports/
   ```

3. 重组 Docs 内部文档：
   - 将用户手册移动到 `Docs/user/`
   - 将设备手册移动到 `Docs/hardware/`
   - 将技术报告移动到 `Docs/reports/`

**验证**：
- 检查所有文档链接是否有效
- 更新 `README.md` 中的文档索引

---

### 阶段 3：文档合并（P2 优先级）

**目标**：消除重复文档，统一技术指南

**步骤**：
1. 合并模拟框架文档：
   - 将 `MOCK_MODE_GUIDE.md` 内容合并到 `SIMULATION_GUIDE.md`
   - 删除 `MOCK_MODE_GUIDE.md`

2. 合并 Qt6 优化文档：
   - 将 `QT6_OPTIMIZATION_SUMMARY.md` 和 `QT6_MODERNIZATION_PLAN.md` 合并到 `QT6_FEATURE_MAPPING.md`
   - 移动原文档到 `Docs/archive/`

3. 删除临时分析文档：
   - 删除 `Documentation_System_Analysis_Report.md`
   - 删除 `Documentation_Coverage_Gap_Analysis.md`
   - 删除 `Duplicate_Documents_List.md`
   - 删除 `Obsolete_Documents_List.md`

**验证**：
- 确保合并后的文档内容完整
- 更新文档索引和交叉引用

---

## 六、风险与注意事项

### 6.1 构建系统风险

⚠️ **CMakeLists.txt 需要同步更新**：
- `main.cpp` 移动后，需修改 `add_executable` 路径
- `Main.qml` 移动后，需修改 QML 资源路径
- 建议在修改后立即运行 `cmake --build build` 验证

### 6.2 QML 导入路径风险

⚠️ **QML 导入路径可能受影响**：
- 移动 `Main.qml` 后，检查 `import` 语句的相对路径
- 检查 `qrc` 资源文件中的路径引用
- 建议在修改后运行应用验证 UI 加载正常

### 6.3 脚本相对路径风险

⚠️ **脚本中的相对路径**：
- 移动 `.bat`/`.ps1` 后，检查脚本内的 `cd` 和文件引用路径
- 特别注意 `build-verify.ps1` 中的构建输出路径
- 建议在修改后运行脚本验证功能正常

### 6.4 Git 历史保留

⚠️ **使用 `git mv` 而非直接移动**：
- 使用 `git mv` 可以保留文件的 Git 历史记录
- 避免使用 `mv` 或文件管理器直接移动文件
- 移动后使用 `git log --follow <file>` 验证历史保留

### 6.5 团队协作风险

⚠️ **通知团队成员**：
- 在执行迁移前，通知所有团队成员
- 建议在独立分支上执行迁移，测试通过后再合并到主分支
- 更新团队文档和 Wiki，说明新的目录结构

---

## 七、预期成果

### 7.1 根目录精简

**当前**：32 个文件（9个MD、6个脚本、6个测试文件等）  
**目标**：5 个核心文件（README×2 + LICENSE + CMakeLists.txt + .gitignore）

**精简率**：84.4%

### 7.2 文档结构化

**当前**：51 个 MD 文档散落在 Docs 目录  
**目标**：按职能分类到 4 个子目录（user/development/hardware/reports）

**分类覆盖率**：100%

### 7.3 文档去重

**当前**：14 个重复或过时文档  
**目标**：合并到 3 个统一指南（SIMULATION_GUIDE、QT6_FEATURE_MAPPING、Device_Register_Reference_Guide）

**去重率**：78.6%

---

## 八、后续维护建议

### 8.1 目录结构维护

1. **新文件放置原则**：
   - 源代码 → `src/` 对应子目录
   - 脚本工具 → `scripts/` 对应子目录
   - 临时文件 → `temp/`（不提交到 Git）
   - 技术文档 → `Docs/` 对应分类

2. **定期清理**：
   - 每月检查 `temp/` 目录，删除过期文件
   - 每季度审查 `Docs/archive/`，删除无价值的历史文档

### 8.2 文档维护

1. **文档更新原则**：
   - 优先更新现有文档，避免创建新文档
   - 新增文档前，检查是否可以合并到现有文档
   - 过时文档移动到 `Docs/archive/`，不要直接删除

2. **文档索引维护**：
   - 在 `README.md` 中维护文档索引
   - 使用相对路径引用文档，避免绝对路径

---

## 九、审批与执行

### 9.1 审批流程

- [ ] 团队负责人审批
- [ ] 协议与设备开发审批（确认脚本路径变更）
- [ ] 领域引擎开发审批（确认构建配置变更）
- [ ] UI/QML 开发审批（确认 QML 路径变更）

### 9.2 执行计划

| 阶段 | 负责人 | 预计工时 | 开始日期 | 完成日期 |
|------|--------|---------|---------|---------|
| P0：根目录清理 | 基础设施/服务开发 | 2 小时 | TBD | TBD |
| P1：文档结构化 | 头脑风暴 + 代码审查 | 4 小时 | TBD | TBD |
| P2：文档合并 | 头脑风暴 | 3 小时 | TBD | TBD |

---

## 十、附录

### 10.1 参考标准

- Qt6 项目标准目录结构：https://doc.qt.io/qt-6/cmake-get-started.html
- CMake 最佳实践：https://cmake.org/cmake/help/latest/guide/tutorial/index.html
- Git 文件移动最佳实践：https://git-scm.com/docs/git-mv

### 10.2 相关文档

- `README.md` - 项目概览
- `ARCHITECTURE.md` - 架构设计
- `IMPLEMENTATION_PROGRESS.md` - 实施进度

---

**文档结束**
