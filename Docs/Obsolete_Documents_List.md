# 废弃文档清单

**生成日期**: 2026-04-24  
**目的**: 识别临时性、过时或已完成使命的文档，建议归档或删除

---

## 一、临时性分析文档（建议归档）

### 类别 A: 头脑风暴产物

| 文档路径 | 大小 | 创建日期 | 内容摘要 | 状态 |
|---------|------|---------|---------|------|
| `Docs/Brainstorm_Architecture_Iteration_Plan.md` | 28K | ~2026-04-23 | 架构迭代计划头脑风暴 | 📦 归档 |
| `Docs/Brainstorm_Device_Function_Architecture_Assessment.md` | 17K | ~2026-04-23 | 设备功能架构评估头脑风暴 | 📦 归档 |

**废弃原因**:
- 这些是头脑风暴阶段的临时产物
- 最终决策已体现在正式架构文档中
- 保留价值：记录决策过程，但不应作为主要参考

**建议操作**:
- 移动到 `Docs/archive/brainstorming/`
- 添加归档说明：这些文档记录了早期架构探索过程，最终方案见 ARCHITECTURE.md

---

### 类别 B: 架构评估报告

| 文档路径 | 大小 | 创建日期 | 内容摘要 | 状态 |
|---------|------|---------|---------|------|
| `Docs/viewmodel-architecture-evaluation.md` | 23K | ~2026-04 | ViewModel 架构评估 | 📦 归档（合并后） |
| `Docs/viewmodel-signal-optimization-summary.md` | 6.1K | ~2026-04 | ViewModel 信号优化总结 | 📦 归档（合并后） |
| `Docs/DYN200_Integration_Architecture_Analysis.md` | 31K | ~2026-04-23 | DYN200 集成架构分析 | 📦 归档（合并后） |
| `Docs/DeviceImplementationReview.md` | 18K | ~2026-04-23 | 设备实现审查 | 📦 归档 |

**废弃原因**:
- 这些是特定时间点的架构评估快照
- 评估结论已应用到代码中
- 部分内容已合并到正式文档

**建议操作**:
- 合并有价值内容到正式文档后，移动到 `Docs/archive/evaluations/`
- 保留作为历史记录

---

### 类别 C: 补充计划和改进建议

| 文档路径 | 大小 | 创建日期 | 内容摘要 | 状态 |
|---------|------|---------|---------|------|
| `Docs/EncoderProactiveListener_Supplement_Plan.md` | 28K | ~2026-04-24 | 编码器主动监听器补充计划 | 📦 归档 |
| `Docs/error_handling_improvements.md` | 11K | ~2026-04-20 | 错误处理改进建议 | 📦 归档 |
| `Docs/infrastructure_improvements.md` | 5.2K | ~2026-04-20 | 基础设施改进建议 | 📦 归档（合并后） |
| `Docs/build-automation-best-practices.md` | 15K | ~2026-04-22 | 构建自动化最佳实践 | 📦 归档 |

**废弃原因**:
- 这些是特定改进任务的计划文档
- 改进已实施或决定不实施
- 建议已整合到正式文档

**建议操作**:
- 提取未实施的改进建议到 TODO 或 Issue
- 移动到 `Docs/archive/improvement-plans/`

---

### 类别 D: 研究报告

| 文档路径 | 大小 | 创建日期 | 内容摘要 | 状态 |
|---------|------|---------|---------|------|
| `Docs/testing-framework-research.md` | 8.5K | ~2026-04-22 | 测试框架研究 | 📦 归档 |
| `Docs/team-config-analysis.md` | 9.7K | ~2026-04 | 团队配置分析 | 📦 归档 |

**废弃原因**:
- 研究阶段的调研文档
- 研究结论已应用到项目中
- 不再需要作为日常参考

**建议操作**:
- 移动到 `Docs/archive/research/`
- 保留作为决策依据的历史记录

---

### 类别 E: 问题修复报告

| 文档路径 | 大小 | 创建日期 | 内容摘要 | 状态 |
|---------|------|---------|---------|------|
| `Docs/BoundaryProtectionReport.md` | 6.1K | ~2026-04-20 | 边界保护报告 | 📦 归档 |
| `Docs/Protocol_Layer_Thread_Safety_And_Error_Signal_Fix.md` | 7.6K | ~2026-04 | 协议层线程安全和错误信号修复 | 📦 归档 |
| `Docs/MockMotorDevice_MagnetDetection_Enhancement.md` | 7.8K | ~2026-04 | Mock 电机设备磁铁检测增强 | 📦 归档 |

**废弃原因**:
- 这些是特定问题的修复报告
- 问题已修复，代码已更新
- 修复细节已记录在 Git 提交历史中

**建议操作**:
- 移动到 `Docs/archive/fixes/`
- 如果是重要的架构决策，提取到 ADR（Architecture Decision Record）

---

## 二、根目录临时文档（建议移动）

### 类别 F: 交付和发布文档

| 文档路径 | 大小 | 创建日期 | 内容摘要 | 状态 |
|---------|------|---------|---------|------|
| `DELIVERY_PLAN.md` | 15K | ~2026-04-23 | 团队分工与分阶段交付计划 | 📦 移动到 Docs/project/ |
| `LAUNCH_READINESS_REPORT.md` | 14K | ~2026-04 | 发布准备报告 | 📦 移动到 Docs/project/ |
| `MERGE_STATUS_REPORT.md` | 1.3K | ~2026-04 | 合并状态报告 | 📦 归档 |

**废弃原因**:
- 这些是项目管理文档，不应放在根目录
- 部分文档是特定时间点的状态快照

**建议操作**:
- 移动到 `Docs/project/` 或 `Docs/archive/project/`
- 根目录只保留核心文档（README, BUILD, ARCHITECTURE 等）

---

### 类别 G: 实现验证报告

| 文档路径 | 大小 | 创建日期 | 内容摘要 | 状态 |
|---------|------|---------|---------|------|
| `MODBUS_0x04_0x05_IMPLEMENTATION_VERIFICATION.md` | 8.0K | ~2026-04 | Modbus 0x04/0x05 实现验证 | 📦 移动到 Docs/development/ |
| `ENCODER_PROACTIVE_INTEGRATION.md` | 2.9K | ~2026-04 | 编码器主动集成 | 📦 移动到 Docs/development/ |

**废弃原因**:
- 这些是开发阶段的验证文档
- 不应放在根目录

**建议操作**:
- 移动到 `Docs/development/` 或合并到相关的开发文档

---

### 类别 H: 执行计划

| 文档路径 | 大小 | 创建日期 | 内容摘要 | 状态 |
|---------|------|---------|---------|------|
| `EXECPLAN_DiagnosticsRedesign.md` | 32K | ~2026-04 | 诊断系统重设计执行计划 | 📦 移动到 Docs/project/ |

**废弃原因**:
- 这是特定功能的执行计划
- 不应放在根目录

**建议操作**:
- 移动到 `Docs/project/plans/` 或归档

---

## 三、废弃草稿（建议删除）

### 类别 I: HTML 原型草稿

| 文档路径 | 内容 | 状态 |
|---------|------|------|
| `Docs/drafts/apple-industrial-prototype/` | HTML/CSS/JS 原型草稿 | 🗑️ 删除或归档 |
| `Docs/drafts/apple-industrial-prototype/design-system.md` | shadcn-inspired 设计系统 | 🗑️ 删除或归档 |

**废弃原因**:
- 这是早期的 HTML 原型草稿
- 已被 QML 实现完全替代
- 设计系统已迁移到 QML 组件

**建议操作**:
- **选项 1（推荐）**: 完全删除（如果 Git 历史中有备份）
- **选项 2**: 移动到 `Docs/archive/prototypes/`，添加说明"此原型已废弃，请参考 QML 实现"

---

## 四、归档目录结构建议

```
Docs/archive/
├── brainstorming/              # 头脑风暴文档
│   ├── Brainstorm_Architecture_Iteration_Plan.md
│   └── Brainstorm_Device_Function_Architecture_Assessment.md
├── evaluations/                # 架构评估报告
│   ├── viewmodel-architecture-evaluation.md
│   ├── viewmodel-signal-optimization-summary.md
│   ├── DYN200_Integration_Architecture_Analysis.md
│   └── DeviceImplementationReview.md
├── improvement-plans/          # 改进计划
│   ├── EncoderProactiveListener_Supplement_Plan.md
│   ├── error_handling_improvements.md
│   ├── infrastructure_improvements.md
│   └── build-automation-best-practices.md
├── research/                   # 研究报告
│   ├── testing-framework-research.md
│   └── team-config-analysis.md
├── fixes/                      # 问题修复报告
│   ├── BoundaryProtectionReport.md
│   ├── Protocol_Layer_Thread_Safety_And_Error_Signal_Fix.md
│   └── MockMotorDevice_MagnetDetection_Enhancement.md
├── project/                    # 项目管理文档
│   ├── DELIVERY_PLAN.md
│   ├── LAUNCH_READINESS_REPORT.md
│   ├── MERGE_STATUS_REPORT.md
│   └── EXECPLAN_DiagnosticsRedesign.md
├── prototypes/                 # 废弃原型
│   └── apple-industrial-prototype/
└── pre-merge-backup/           # 合并前备份
    └── (待合并文档的备份)
```

---

## 五、归档操作清单

### Phase 1: 创建归档结构（预计 0.5h）

```bash
mkdir -p Docs/archive/{brainstorming,evaluations,improvement-plans,research,fixes,project,prototypes,pre-merge-backup}
```

### Phase 2: 移动文档（预计 1h）

| 优先级 | 操作 | 文档数量 | 预计时间 |
|--------|------|---------|---------|
| 🔴 P1 | 移动头脑风暴文档 | 2 | 5min |
| 🔴 P1 | 移动架构评估报告 | 4 | 10min |
| 🟡 P2 | 移动改进计划 | 4 | 10min |
| 🟡 P2 | 移动研究报告 | 2 | 5min |
| 🟡 P2 | 移动问题修复报告 | 3 | 10min |
| 🟡 P2 | 移动项目管理文档 | 4 | 10min |
| 🟢 P3 | 移动/删除废弃原型 | 1 | 10min |

### Phase 3: 添加归档说明（预计 0.5h）

在每个归档目录创建 `README.md`，说明：
- 归档日期
- 归档原因
- 如何查找最新文档

**示例**:
```markdown
# 归档说明

此目录包含已归档的头脑风暴文档。

**归档日期**: 2026-04-24  
**归档原因**: 这些文档是早期架构探索阶段的产物，最终决策已体现在正式架构文档中。

**查找最新文档**:
- 架构文档: [ARCHITECTURE.md](../../ARCHITECTURE.md)
- 设计决策: [architecture/ADR-*.md](../../architecture/)

这些文档保留作为历史记录，展示了项目架构的演进过程。
```

---

## 六、归档后文档统计

### 归档前

```
根目录: 14 个 .md 文件
Docs/: 40 个 .md 文件
总计: 54 个活跃文档
```

### 归档后（预期）

```
根目录: 3 个 .md 文件（README.md, BUILD.md, ARCHITECTURE.md）
Docs/: 20 个 .md 文件（核心文档 + 合并后文档）
Docs/archive/: 20 个 .md 文件（归档文档）
总计: 23 个活跃文档，20 个归档文档
```

**改善效果**:
- 活跃文档减少 57%（54 → 23）
- 根目录文档减少 79%（14 → 3）
- 文档结构更清晰，易于维护

---

## 七、归档风险评估

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| 归档后找不到历史文档 | 中 | 中 | 在归档目录添加详细的 README 和索引 |
| 误归档重要文档 | 低 | 高 | 归档前由团队 Leader 审查清单 |
| 文档引用失效 | 高 | 低 | 全局搜索更新所有引用 |
| 团队成员不知道归档位置 | 中 | 低 | 在主 README 中说明归档策略 |

---

## 八、归档后维护策略

### 8.1 归档文档的访问策略

- **归档文档不应被日常引用**
- 如果需要引用归档文档，说明该文档可能不应归档
- 归档文档主要用于：
  - 历史回顾
  - 决策追溯
  - 新成员了解项目演进

### 8.2 归档文档的更新策略

- **归档文档不应更新**
- 如果需要更新，应创建新的活跃文档
- 归档文档应保持原样，作为历史快照

### 8.3 定期清理策略

- **每季度审查归档文档**
- 超过 1 年且无访问记录的文档可考虑删除
- 删除前确保 Git 历史中有备份

---

## 九、归档操作脚本

### Bash 脚本示例

```bash
#!/bin/bash
# 文档归档脚本
# 使用方法: ./archive_docs.sh

set -e

BASE_DIR="E:/Work/FlipGearboxFactoryTest"
ARCHIVE_DIR="$BASE_DIR/Docs/archive"

echo "创建归档目录结构..."
mkdir -p "$ARCHIVE_DIR"/{brainstorming,evaluations,improvement-plans,research,fixes,project,prototypes,pre-merge-backup}

echo "归档头脑风暴文档..."
mv "$BASE_DIR/Docs/Brainstorm_Architecture_Iteration_Plan.md" "$ARCHIVE_DIR/brainstorming/"
mv "$BASE_DIR/Docs/Brainstorm_Device_Function_Architecture_Assessment.md" "$ARCHIVE_DIR/brainstorming/"

echo "归档架构评估报告..."
mv "$BASE_DIR/Docs/viewmodel-architecture-evaluation.md" "$ARCHIVE_DIR/evaluations/"
mv "$BASE_DIR/Docs/viewmodel-signal-optimization-summary.md" "$ARCHIVE_DIR/evaluations/"
mv "$BASE_DIR/Docs/DYN200_Integration_Architecture_Analysis.md" "$ARCHIVE_DIR/evaluations/"
mv "$BASE_DIR/Docs/DeviceImplementationReview.md" "$ARCHIVE_DIR/evaluations/"

echo "归档改进计划..."
mv "$BASE_DIR/Docs/EncoderProactiveListener_Supplement_Plan.md" "$ARCHIVE_DIR/improvement-plans/"
mv "$BASE_DIR/Docs/error_handling_improvements.md" "$ARCHIVE_DIR/improvement-plans/"
mv "$BASE_DIR/Docs/infrastructure_improvements.md" "$ARCHIVE_DIR/improvement-plans/"
mv "$BASE_DIR/Docs/build-automation-best-practices.md" "$ARCHIVE_DIR/improvement-plans/"

echo "归档研究报告..."
mv "$BASE_DIR/Docs/testing-framework-research.md" "$ARCHIVE_DIR/research/"
mv "$BASE_DIR/Docs/team-config-analysis.md" "$ARCHIVE_DIR/research/"

echo "归档问题修复报告..."
mv "$BASE_DIR/Docs/BoundaryProtectionReport.md" "$ARCHIVE_DIR/fixes/"
mv "$BASE_DIR/Docs/Protocol_Layer_Thread_Safety_And_Error_Signal_Fix.md" "$ARCHIVE_DIR/fixes/"
mv "$BASE_DIR/Docs/MockMotorDevice_MagnetDetection_Enhancement.md" "$ARCHIVE_DIR/fixes/"

echo "归档项目管理文档..."
mv "$BASE_DIR/DELIVERY_PLAN.md" "$ARCHIVE_DIR/project/"
mv "$BASE_DIR/LAUNCH_READINESS_REPORT.md" "$ARCHIVE_DIR/project/"
mv "$BASE_DIR/MERGE_STATUS_REPORT.md" "$ARCHIVE_DIR/project/"
mv "$BASE_DIR/EXECPLAN_DiagnosticsRedesign.md" "$ARCHIVE_DIR/project/"

echo "归档废弃原型..."
mv "$BASE_DIR/Docs/drafts/apple-industrial-prototype" "$ARCHIVE_DIR/prototypes/"

echo "归档完成！"
echo "归档文档位置: $ARCHIVE_DIR"
```

---

## 十、归档后的 README 更新

在主 README.md 中添加归档说明：

```markdown
## 文档

### 核心文档
- [构建指南](BUILD.md)
- [架构文档](ARCHITECTURE.md)
- [快速开始](GETTING_STARTED.md)

### 用户文档
- [用户手册](Docs/guides/USER_MANUAL.md)
- [部署指南](Docs/guides/DEPLOYMENT_GUIDE.md)
- [故障排查](Docs/guides/TROUBLESHOOTING.md)

### 开发文档
- [贡献指南](CONTRIBUTING.md)
- [测试指南](Docs/development/TESTING.md)
- [协议文档](Docs/development/PROTOCOL.md)

### 归档文档
历史文档和临时性分析报告已归档到 [Docs/archive/](Docs/archive/)。
这些文档保留作为历史记录，不应作为日常参考。
```

---

**报告生成时间**: 2026-04-24  
**预计归档完成时间**: 2026-04-26（2天）  
**归档文档总数**: 20 个  
**节省的维护负担**: 约 57%
