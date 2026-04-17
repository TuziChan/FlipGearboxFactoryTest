# 串口通信协议完善 Checklist — 覆盖全部设备手册协议

## ModbusFrame 0x10 功能码
- [ ] `buildWriteMultipleRegisters` 正确构建 0x10 请求帧（SlaveId + 0x10 + 起始地址 + 寄存器数量 + 字节数 + 数据 + CRC）
- [ ] `parseWriteMultipleRegistersResponse` 正确解析 0x10 响应帧（验证 CRC、功能码、地址和数量）
- [ ] 0x10 帧构建的单元测试通过（与已知帧数据对比）
- [ ] AQMD GPIO 配置可通过 0x10 批量写入触发 EEPROM 存储

## ModbusFrame 0x2B 功能码
- [ ] `buildReadDeviceIdentification` 正确构建 0x2B + MEI 0x0E 请求帧
- [ ] `parseReadDeviceIdentificationResponse` 正确解析设备信息对象列表（VendorName, ProductCode, MajorMinorRevision）
- [ ] 0x2B 帧构建和解析的单元测试通过

## ModbusFrame 异常响应解析
- [ ] `parseExceptionResponse` 正确识别异常帧（功能码 & 0x80）并提取异常码
- [ ] 所有标准 Modbus 异常码（0x01~0x0B）均有对应描述字符串
- [ ] AQMD 扩展异常码（0x40 禁止操作, 0xFF 未定义错误）有对应描述
- [ ] 所有现有 parse*Response 函数在收到异常帧时返回 false 且错误信息包含异常码描述
- [ ] 异常响应解析的单元测试通过（含标准码和 AQMD 扩展码）

## ModbusFrame CRC 字节序兼容
- [ ] 默认使用标准 Modbus RTU CRC 字节序（低字节在前）
- [ ] `setCrcByteOrder(true)` 切换为大端 CRC 后帧构建正确
- [ ] `verifyCRC` 在两种字节序模式下均能正确验证
- [ ] CRC 字节序兼容的单元测试通过

## ModbusRtuBusController 运行时重配置
- [ ] `reconfigure()` 在串口已打开时能更新波特率、校验位、停止位
- [ ] `reconfigure()` 在串口未打开时缓存参数供后续 `open()` 使用
- [ ] 重配置后 interFrameDelay 根据新波特率重新计算

## DYN200 HEX 6 字节主动上传协议（通信模式 0）
- [ ] `Dyn200ProactiveListener` 能正确解析 6 字节 HEX 帧（D1D2=扭矩, D3D4=转速, D5D6=CRC）
- [ ] 正扭矩值解析正确
- [ ] 负扭矩值解析正确（D3 最高位为 1 时扭矩取负）
- [ ] CRC 校验失败的帧被丢弃并发射 errorOccurred 信号
- [ ] 帧碎片拼接正确

## DYN200 HEX 8 字节主动上传协议（通信模式 3）
- [ ] `Dyn200ProactiveListener` 能正确解析 8 字节 HEX 帧（D1D2D3=扭矩, D4D5D6=转速, D7D8=CRC）
- [ ] 3 字节有符号扭矩值解析正确（D1 最高位为 1 时取负，最大 0x7FFFFF）
- [ ] 3 字节无符号转速值解析正确（最大 0xFFFFFF，支持超过 32767 的转速）
- [ ] CRC 校验失败的帧被丢弃
- [ ] 帧碎片拼接正确

## DYN200 ASCII 主动上传协议（通信模式 2）
- [ ] `Dyn200ProactiveListener` 能正确解析 ASCII 帧（`[+-]X.XXXX\r` 格式）
- [ ] 正扭矩和负扭矩均正确解析
- [ ] ASCII 模式下转速标记为不可用（speedValid = false）
- [ ] 以 `\r`(0x0D) 为帧分隔符
- [ ] 帧碎片拼接正确

## DYN200 四种通信模式切换
- [ ] `Dyn200TorqueSensorDevice::initialize()` 在 communicationMode=0 时使用现有 Modbus RTU 轮询
- [ ] `Dyn200TorqueSensorDevice::initialize()` 在 communicationMode=1 时写入寄存器 0x1CH=0 切换到 HEX 6 字节主动上传
- [ ] `Dyn200TorqueSensorDevice::initialize()` 在 communicationMode=2 时写入寄存器 0x1CH=3 切换到 HEX 8 字节主动上传
- [ ] `Dyn200TorqueSensorDevice::initialize()` 在 communicationMode=3 时写入寄存器 0x1CH=2 切换到 ASCII 主动上传
- [ ] 主动上传模式下 `readAll()` 从 ProactiveListener 缓存读取而非发送 Modbus 请求
- [ ] ASCII 模式下 `readSpeed()` 返回 false
- [ ] 代码中包含注释/日志提醒：从主动上传切回 Modbus RTU 需要手动通过传感器按键操作

## 编码器四种模式支持
- [ ] `SingleTurnEncoderDevice::initialize()` 在 communicationMode=0 时使用查询模式（现有行为）
- [ ] `SingleTurnEncoderDevice::initialize()` 在 communicationMode=2 时写入 0x0006=0x01（自动回传单圈值）和 0x0007=回传周期
- [ ] `SingleTurnEncoderDevice::initialize()` 在 communicationMode=3 时写入 0x0006=0x04（自动回传虚拟多圈值）和 0x0007=回传周期
- [ ] `SingleTurnEncoderDevice::initialize()` 在 communicationMode=4 时写入 0x0006=0x05（自动回传角速度值）和 0x0007=回传周期
- [ ] `setAutoReportMode()` 方法可运行时切换回传模式
- [ ] 查询模式（communicationMode=0）行为不受影响

## 编码器虚拟多圈值和角速度值读取
- [ ] `readVirtualMultiTurn()` 正确读取 2 个寄存器（0x0000~0x0001），组合为 uint32 并换算为角度
- [ ] `readAngularVelocity()` 正确读取 1 个寄存器（0x0003），有符号 int16 并换算为 RPM
- [ ] `IEncoderDevice` 接口新增虚函数声明
- [ ] 单元测试验证数据换算逻辑

## AQMD 设备识别码和批量写
- [ ] `AqmdMotorDriveDevice::readDeviceIdentification()` 通过 0x2B 正确读取设备信息
- [ ] `AqmdMotorDriveDevice::writeMultipleRegisters()` 通过 0x10 正确批量写入寄存器
- [ ] GPIO 批量写入可触发 EEPROM 存储

## DeviceConfig 扩展
- [ ] `DeviceConfig` 结构体包含 `communicationMode` 字段，默认值 0
- [ ] `DeviceConfigService` JSON 解析正确读取 `"communicationMode"` 键
- [ ] `StationRuntimeFactory` 将 communicationMode 传递给 Dyn200TorqueSensorDevice 和 SingleTurnEncoderDevice
- [ ] StationConfig 默认值与设备手册一致（AQMD: Even/1stop/9600, DYN200: None/2stop/19200, 编码器: None/1stop/9600, 数控电源: None/1stop/9600）

## 构建与测试
- [ ] 项目完整编译无错误无警告
- [ ] 所有新增单元测试通过
- [ ] 现有测试（QmlSmokeTests, TestExecutionVerification）不受影响

## 协议覆盖完整性
- [ ] DYN200: Modbus RTU (03H/05H/10H) ✅ 已有+新增
- [ ] DYN200: HEX 6 字节主动上传 🔴 → 实现
- [ ] DYN200: HEX 8 字节主动上传 🔴 → 实现
- [ ] DYN200: ASCII 主动上传 🔴 → 实现
- [ ] 编码器: 查询模式 (0x03/0x06) ✅ 已有
- [ ] 编码器: 自动回传单圈值 (0x01) 🔴 → 实现
- [ ] 编码器: 自动回传虚拟多圈值 (0x04) 🔴 → 实现
- [ ] 编码器: 自动回传角速度值 (0x05) 🔴 → 实现
- [ ] 编码器: 虚拟多圈值读取 (0x0000~0x0001) 🔴 → 实现
- [ ] 编码器: 角速度值读取 (0x0003) 🔴 → 实现
- [ ] AQMD: 0x03 读保持寄存器 ✅ 已有
- [ ] AQMD: 0x06 写单个寄存器 ✅ 已有
- [ ] AQMD: 0x10 写多个寄存器 🔴 → 实现
- [ ] AQMD: 0x2B 读设备识别码 🔴 → 实现
- [ ] 数控电源: 0x01/0x03/0x04/0x05/0x06 ✅ 全部已有
