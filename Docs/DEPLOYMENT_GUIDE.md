# 齿轮箱测试系统部署指南

## 版本信息
- 系统版本: 0.1
- 文档版本: 1.0
- 更新日期: 2026-04-23

---

## 1. 系统要求

### 1.1 硬件要求
- **处理器**: Intel Core i5 或更高
- **内存**: 8GB RAM 或更高
- **硬盘**: 至少10GB可用空间
- **串口**: 4个可用COM口（或USB转串口）
- **显示器**: 1920x1080 或更高分辨率

### 1.2 软件要求
- **操作系统**: Windows 10/11 (64位)
- **Qt运行库**: Qt 6.11.0 或更高
- **Visual C++运行库**: Microsoft Visual C++ 2015-2022 Redistributable

### 1.3 硬件设备
- AQMD电机驱动器
- DYN200扭矩传感器
- 单圈编码器
- 制动电源（支持Modbus RTU）

---

## 2. 硬件连接

### 2.1 连接拓扑图

```
[测试PC]
    |
    +-- COM3 --> [AQMD电机驱动器] --> [被测齿轮箱]
    |
    +-- COM4 --> [DYN200扭矩传感器] --> [被测齿轮箱]
    |
    +-- COM5 --> [单圈编码器] --> [被测齿轮箱]
    |
    +-- COM6 --> [制动电源] --> [制动器]
```

### 2.2 串口连接

#### AQMD电机驱动器 (COM3)
- **波特率**: 9600
- **数据位**: 8
- **校验位**: Even (偶校验)
- **停止位**: 1
- **Modbus地址**: 1

#### DYN200扭矩传感器 (COM4)
- **波特率**: 19200
- **数据位**: 8
- **校验位**: None (无校验)
- **停止位**: 2
- **Modbus地址**: 2

#### 单圈编码器 (COM5)
- **波特率**: 9600
- **数据位**: 8
- **校验位**: None (无校验)
- **停止位**: 1
- **Modbus地址**: 3

#### 制动电源 (COM6)
- **波特率**: 9600
- **数据位**: 8
- **校验位**: None (无校验)
- **停止位**: 1
- **Modbus地址**: 4

### 2.3 接线检查清单
- [ ] 所有设备供电正常
- [ ] 串口线连接牢固
- [ ] 串口号与配置文件一致
- [ ] 设备Modbus地址正确设置
- [ ] 接地线连接良好

---

## 3. 软件安装

### 3.1 安装Qt运行库

1. 下载Qt 6.11.0运行库：
   - 官方下载：https://www.qt.io/download
   - 或使用提供的安装包：`qt-6.11.0-runtime.exe`

2. 运行安装程序，选择以下组件：
   - Qt 6.11.0 MinGW 64-bit
   - Qt Quick Controls 2
   - Qt Serial Port

3. 安装路径建议：`F:\Qt\6.11.0\mingw_64`

### 3.2 安装Visual C++运行库

1. 下载并安装：
   - `vc_redist.x64.exe`
   - 下载地址：https://aka.ms/vs/17/release/vc_redist.x64.exe

2. 运行安装程序，按提示完成安装

### 3.3 部署应用程序

1. 创建安装目录：
   ```
   C:\Program Files\FlipGearboxFactoryTest\
   ```

2. 复制以下文件到安装目录：
   ```
   FlipGearboxFactoryTest\
   ├── appFlipGearboxFactoryTest.exe    # 主程序
   ├── Qt6Core.dll                       # Qt核心库
   ├── Qt6Gui.dll                        # Qt GUI库
   ├── Qt6Quick.dll                      # Qt Quick库
   ├── Qt6QuickControls2.dll             # Qt Quick Controls库
   ├── Qt6SerialPort.dll                 # Qt串口库
   ├── fonts\                            # 字体文件
   │   ├── HarmonyOS_Sans_SC_Regular.ttf
   │   ├── HarmonyOS_Sans_SC_Medium.ttf
   │   ├── HarmonyOS_Sans_SC_Bold.ttf
   │   └── HarmonyOS_Sans_SC_Light.ttf
   ├── config\                           # 配置文件
   │   ├── station.json
   │   └── recipes\
   │       └── GBX-42A.json
   ├── logs\                             # 日志目录（自动创建）
   └── reports\                          # 报告目录（自动创建）
   ```

3. 设置环境变量（可选）：
   ```
   PATH=%PATH%;F:\Qt\6.11.0\mingw_64\bin
   ```

---

## 4. 配置文件设置

### 4.1 站点配置 (config/station.json)

```json
{
  "stationId": "STATION-01",
  "stationName": "Gearbox Test Station 1",
  "brakeChannel": 1,
  "defaultRecipe": "GBX-42A",
  "aqmd": {
    "portName": "COM3",
    "slaveId": 1,
    "baudRate": 9600,
    "timeout": 500,
    "parity": "Even",
    "stopBits": 1,
    "pollIntervalUs": 5000,
    "enabled": true
  },
  "dyn200": {
    "portName": "COM4",
    "slaveId": 2,
    "baudRate": 19200,
    "timeout": 300,
    "parity": "None",
    "stopBits": 2,
    "pollIntervalUs": 5000,
    "enabled": true
  },
  "encoder": {
    "portName": "COM5",
    "slaveId": 3,
    "baudRate": 9600,
    "timeout": 200,
    "parity": "None",
    "stopBits": 1,
    "resolution": 4096,
    "pollIntervalUs": 5000,
    "enabled": true
  },
  "brake": {
    "portName": "COM6",
    "slaveId": 4,
    "baudRate": 9600,
    "timeout": 500,
    "parity": "None",
    "stopBits": 1,
    "channel": 1,
    "pollIntervalUs": 5000,
    "enabled": true
  }
}
```

### 4.2 配置参数说明

| 参数 | 说明 | 有效范围 |
|------|------|----------|
| portName | 串口名称 | COM1-COM256 |
| slaveId | Modbus从站地址 | 1-247 |
| baudRate | 波特率 | 9600/19200/38400/57600/115200 |
| timeout | 超时时间(ms) | 100-5000 |
| parity | 校验位 | None/Even/Odd |
| stopBits | 停止位 | 1/2 |
| pollIntervalUs | 轮询间隔(μs) | 1000-10000 |
| enabled | 是否启用 | true/false |

### 4.3 配方文件 (config/recipes/GBX-42A.json)

参考示例配方文件，根据实际产品规格调整参数。

---

## 5. 首次运行检查

### 5.1 模拟模式测试

1. 打开命令提示符
2. 进入安装目录
3. 运行模拟模式：
   ```
   appFlipGearboxFactoryTest.exe --mock
   ```
4. 检查界面是否正常显示
5. 执行一次模拟测试
6. 检查日志文件是否生成

### 5.2 硬件连接测试

1. 确认所有硬件已连接
2. 运行真实模式：
   ```
   appFlipGearboxFactoryTest.exe
   ```
3. 进入"设备诊断"页面
4. 逐个测试设备连接：
   - 点击"测试连接"按钮
   - 确认状态显示为绿色
   - 读取寄存器验证通信

### 5.3 完整测试流程

1. 选择测试配方
2. 输入测试序列号
3. 点击"开始测试"
4. 观察测试流程执行
5. 检查测试结果
6. 查看生成的报告文件

---

## 6. 故障排查

### 6.1 程序无法启动

**问题**: 双击程序无响应

**检查步骤**:
1. 确认Qt运行库已安装
2. 检查DLL文件是否完整
3. 查看日志文件：`logs/app_YYYY-MM-DD.log`
4. 使用依赖项检查工具：`dumpbin /dependents appFlipGearboxFactoryTest.exe`

**解决方法**:
- 重新安装Qt运行库
- 复制缺失的DLL文件
- 检查配置文件格式

### 6.2 设备连接失败

**问题**: 设备状态显示红色

**检查步骤**:
1. 使用设备管理器检查串口
2. 确认串口号与配置一致
3. 检查设备电源指示灯
4. 使用串口调试工具测试

**解决方法**:
- 修改配置文件中的串口号
- 检查串口线连接
- 重启设备
- 检查Modbus地址设置

### 6.3 配置文件错误

**问题**: 启动时提示配置加载失败

**检查步骤**:
1. 确认配置文件存在
2. 使用JSON验证工具检查格式
3. 查看日志中的详细错误信息

**解决方法**:
- 修正JSON格式错误
- 恢复默认配置文件
- 检查参数值范围

---

## 7. 性能优化

### 7.1 采集周期调整

默认采集周期为33ms，如需调整：

1. 修改`pollIntervalUs`参数（单位：微秒）
2. 建议范围：5000-50000 (5ms-50ms)
3. 过小的值可能导致CPU占用过高

### 7.2 缓冲区大小

默认遥测缓冲区大小为10000个样本：

- 33ms周期下可存储约5.5分钟数据
- 如需更长时间，需修改源码中的缓冲区大小

### 7.3 日志级别

生产环境建议设置日志级别为Warning：

```
QT_LOGGING_RULES=*.debug=false;*.info=false
```

---

## 8. 备份与恢复

### 8.1 备份内容

定期备份以下内容：
- 配置文件：`config/`
- 配方文件：`config/recipes/`
- 测试报告：`reports/`
- 日志文件：`logs/` (可选)

### 8.2 备份方法

**手动备份**:
```batch
xcopy /E /I /Y config backup\config
xcopy /E /I /Y reports backup\reports
```

**自动备份**:
使用Windows任务计划程序定期执行备份脚本

### 8.3 恢复方法

1. 停止应用程序
2. 恢复配置文件到`config/`目录
3. 恢复配方文件到`config/recipes/`目录
4. 重启应用程序
5. 验证配置是否正确

---

## 9. 安全注意事项

### 9.1 电气安全
- 确保设备接地良好
- 使用合格的电源线
- 定期检查线路绝缘

### 9.2 机械安全
- 测试时保持安全距离
- 设置急停按钮
- 定期检查机械部件

### 9.3 数据安全
- 定期备份重要数据
- 限制配置文件访问权限
- 使用防病毒软件

---

## 10. 维护计划

### 10.1 日常维护
- 检查设备连接状态
- 查看日志文件
- 清理临时文件

### 10.2 每周维护
- 备份配置文件
- 检查硬盘空间
- 清理过期日志

### 10.3 每月维护
- 完整系统备份
- 检查软件更新
- 校准传感器（如需）

### 10.4 每季度维护
- 硬件设备检查
- 性能测试
- 系统优化

---

## 11. 技术支持

### 11.1 联系方式
- 技术支持邮箱：support@example.com
- 技术支持电话：400-XXX-XXXX
- 在线文档：https://example.com/docs

### 11.2 远程支持
如需远程支持，请准备：
- 系统版本信息
- 错误日志文件
- 配置文件
- 问题描述和截图

---

## 附录A：部署检查清单

### 硬件检查
- [ ] 测试PC符合系统要求
- [ ] 所有设备已连接并供电
- [ ] 串口连接正确
- [ ] 接地线连接良好

### 软件检查
- [ ] Qt运行库已安装
- [ ] Visual C++运行库已安装
- [ ] 应用程序文件完整
- [ ] 配置文件已正确设置

### 功能检查
- [ ] 模拟模式运行正常
- [ ] 所有设备连接成功
- [ ] 完整测试流程通过
- [ ] 报告生成正常
- [ ] 日志记录正常

---

## 附录B：常用命令

### 查看串口列表
```batch
mode
```

### 测试串口通信
```batch
mode COM3: BAUD=9600 PARITY=E DATA=8 STOP=1
```

### 检查DLL依赖
```batch
dumpbin /dependents appFlipGearboxFactoryTest.exe
```

### 清理日志文件
```batch
forfiles /p logs /s /m *.log /d -30 /c "cmd /c del @path"
```

---

**文档结束**
