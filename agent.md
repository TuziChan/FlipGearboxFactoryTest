# agent.md

## 目的
本文件约束本仓库内 AI Agent 的**构建、部署、运行、测试**行为，避免因错误的 Qt 运行时处理、错误的构建目录、错误的启动方式导致误判。

---

## 一、总原则

1. **优先复用现有构建目录**，不要随意新建新的 build 目录。
2. **优先使用已验证可用的 Qt 6.11 + llvm-mingw_64 环境**。
3. **Windows 下不要把“给 PATH 补 Qt bin”当成正式交付方案**；正式运行以 `cmake --install` 后的安装目录为准。
4. **运行应用优先使用 install 产物，不要默认直接运行 build 目录里的 exe。**
5. **默认使用 `--mock` 启动**，除非用户明确要求联机真实硬件。
6. **增量开发优先构建单目标**，不要默认全量构建所有 targets。

---

## 二、固定构建目录

日常开发默认使用：

- `build\\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug`

除非用户明确要求切换生成器、编译器或配置，否则不要创建新的构建目录。

---

## 三、CMake / Qt 工具规则

### 1. CMake
优先使用显式路径，避免系统 `PATH` 中没有 `cmake.exe`：

- `D:\\Qt\\Tools\\CMake_64\\bin\\cmake.exe`

### 2. Qt
当前项目以 Qt 6.11 / llvm-mingw_64 为准，已验证路径示例：

- `D:\\Qt\\6.11.0\\llvm-mingw_64`

### 3. 不要假设系统 PATH 已配置 Qt
除非只是临时排查问题，否则不要把“手工补 PATH”作为常规解决方案写入最终流程。

---

## 四、推荐构建流程

### 1. 仅在需要时重新 configure
当构建目录缺失、CMakeLists.txt 发生影响配置的变化、或缓存明显失效时，执行：

```powershell
D:\Qt\Tools\CMake_64\bin\cmake.exe -S . -B build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug
```

### 2. 日常增量构建
优先只构建主程序目标：

```powershell
D:\Qt\Tools\CMake_64\bin\cmake.exe --build build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug --target appFlipGearboxFactoryTest
```

如无必要，不要默认构建全部 targets。

---

## 五、Qt 运行时部署规则

本项目已切换到 **Qt 官方 CMake Deployment API**。

### 正式方式
使用安装步骤部署 Qt 运行时，而不是依赖 PATH：

```powershell
D:\Qt\Tools\CMake_64\bin\cmake.exe --install build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug --prefix E:\Work\FlipGearboxFactoryTest\build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug\install
```

### 强制要求
1. `--prefix` **必须使用绝对路径**。
2. 不要把相对 `--prefix` 当成默认方案。
3. 需要可直接运行的产物时，先 `build`，再 `install`。

---

## 六、运行规则

### 1. 默认运行 install 产物
优先运行：

```powershell
E:\Work\FlipGearboxFactoryTest\build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug\install\bin\appFlipGearboxFactoryTest.exe --mock
```

### 2. 不推荐默认运行 build 目录 exe
以下方式仅用于临时调试，不应作为默认结论：

```powershell
build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug\appFlipGearboxFactoryTest.exe
```

原因：它可能依赖当前 shell 的 Qt PATH 或临时运行时环境。

### 3. 默认参数
- 默认加：`--mock`
- 仅当用户明确要求真实硬件联调时，才不加 `--mock`

### 4. 图表后端
当前默认图表后端为 **Qt Graphs**。除非用户明确要求回退，否则不要主动改回 Qt Charts。

---

## 七、测试规则

### 1. 常规测试入口
优先使用：

```powershell
ctest --test-dir build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug --output-on-failure -C Debug
```

### 2. 快速测试
如只需快速验证，可使用 CMake preset：

```powershell
ctest --preset quick
```

### 3. UI / QML 测试
执行 UI/QML 测试时，应确保无头环境兼容：

- `QT_QPA_PLATFORM=offscreen`
- `QT_QUICK_CONTROLS_STYLE=Basic`

### 4. 不要因单个非关键长测失败就直接判定整个改动不可用
先区分：
- unit
- integration
- ui
- long-running stability

---

## 八、Agent 行为约束

1. 修改完与构建相关的文件后，至少执行一次：
   - `cmake configure`（如需要）
   - `cmake --build ... --target appFlipGearboxFactoryTest`
2. 如果结论涉及“程序可运行”，应优先基于 **install 目录中的 exe** 验证。
3. 如果结论涉及“Qt 运行时已齐全”，应优先基于 `cmake --install` 的结果，而不是基于 shell PATH。
4. 不要因为当前 shell 能跑就断言“部署已完成”。
5. 未经用户要求，不要清空整个 build 目录。
6. 未经用户要求，不要切换 Qt 版本、编译器或生成器。

---

## 九、推荐命令清单

### 增量构建主程序
```powershell
D:\Qt\Tools\CMake_64\bin\cmake.exe --build build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug --target appFlipGearboxFactoryTest
```

### 部署 Qt 运行时
```powershell
D:\Qt\Tools\CMake_64\bin\cmake.exe --install build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug --prefix E:\Work\FlipGearboxFactoryTest\build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug\install
```

### 启动 mock 模式
```powershell
E:\Work\FlipGearboxFactoryTest\build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug\install\bin\appFlipGearboxFactoryTest.exe --mock
```

### 运行全部测试
```powershell
ctest --test-dir build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug --output-on-failure -C Debug
```

---

## 十、禁止事项

1. 禁止把“修改系统 PATH”作为默认交付方案。
2. 禁止默认直接运行未部署运行时的 build 目录 exe，并据此下结论。
3. 禁止随意新建多个 build 目录造成状态分裂。
4. 禁止在未说明原因时全量 clean rebuild。
5. 禁止未经用户确认切回 Qt Charts。
