# Tasks

- [ ] Task 1: ModbusFrame 补全 0x10 Write Multiple Registers 功能码
  - [ ] SubTask 1.1: 在 ModbusFrame.h 中声明 `buildWriteMultipleRegisters(slaveId, startAddress, const QVector<uint16_t>& values)` 和 `parseWriteMultipleRegistersResponse(response, expectedAddress, expectedCount)`
  - [ ] SubTask 1.2: 在 ModbusFrame.cpp 中实现 buildWriteMultipleRegisters：帧格式为 SlaveId + 0x10 + 起始地址(2B) + 寄存器数量(2B) + 字节总数(1B) + 数据(NB) + CRC(2B)
  - [ ] SubTask 1.3: 在 ModbusFrame.cpp 中实现 parseWriteMultipleRegistersResponse：验证 CRC、功能码 0x10、起始地址和寄存器数量匹配
  - [ ] SubTask 1.4: 编写单元测试验证 0x10 帧构建和解析的正确性（使用已知帧数据）

- [ ] Task 2: ModbusFrame 补全 0x2B Read Device Identification 功能码
  - [ ] SubTask 2.1: 在 ModbusFrame.h FunctionCode 枚举中新增 `ReadDeviceIdentification = 0x2B`；声明 `buildReadDeviceIdentification(slaveId)` 和 `parseReadDeviceIdentificationResponse(response, outVendor, outProduct, outVersion)`
  - [ ] SubTask 2.2: 在 ModbusFrame.cpp 中实现 buildReadDeviceIdentification：MEI 类型 0x0E，ReadDevID=0x01（基本设备信息）
  - [ ] SubTask 2.3: 在 ModbusFrame.cpp 中实现 parseReadDeviceIdentificationResponse：解析 MEI 响应中的对象列表（VendorName, ProductCode, MajorMinorRevision）
  - [ ] SubTask 2.4: 编写单元测试验证 0x2B 帧构建和解析

- [ ] Task 3: ModbusFrame 异常响应解析
  - [ ] SubTask 3.1: 在 ModbusFrame.h 中声明 `parseExceptionResponse(response)` 返回 QPair<uint8_t, QString>；新增静态方法 `exceptionCodeToString(uint8_t code)` 返回标准 Modbus 异常码描述（含 AQMD 扩展异常码 0x40/0xFF）
  - [ ] SubTask 3.2: 在 ModbusFrame.cpp 中实现异常码解析（功能码最高位为 1 + 1 字节异常码），覆盖标准异常码 0x01~0x0B 和 AQMD 扩展码
  - [ ] SubTask 3.3: 修改所有现有 parse*Response 函数，在检测到异常帧（功能码 & 0x80）时调用 parseExceptionResponse 提取异常信息，设置明确错误消息
  - [ ] SubTask 3.4: 编写单元测试覆盖各异常码场景（含 AQMD 扩展码）

- [ ] Task 4: ModbusFrame CRC 字节序兼容
  - [ ] SubTask 4.1: 在 ModbusFrame 中新增静态成员 `s_crcBigEndian`（默认 false）和对应的 `setCrcByteOrder(bool bigEndian)` / `isCrcBigEndian()` 方法
  - [ ] SubTask 4.2: 修改所有 build* 函数，根据 `s_crcBigEndian` 决定 CRC 字节追加顺序
  - [ ] SubTask 4.3: 修改 `verifyCRC` 函数，根据当前 CRC 模式验证
  - [ ] SubTask 4.4: 编写单元测试验证大端和小端 CRC 模式下的帧构建和解析

- [ ] Task 5: ModbusRtuBusController 运行时重配置
  - [ ] SubTask 5.1: 在 ModbusRtuBusController.h 中声明 `reconfigure(int baudRate, const QString& parity, int stopBits)` 方法
  - [ ] SubTask 5.2: 在 ModbusRtuBusController.cpp 中实现 reconfigure：如果串口已打开则直接设置新参数；未打开则缓存参数供 open() 使用；更新 interFrameDelay
  - [ ] SubTask 5.3: 编写单元测试验证重配置逻辑（使用 Mock 串口）

- [ ] Task 6: Dyn200ProactiveListener 实现（统一支持 HEX 6 字节 / HEX 8 字节 / ASCII 三种主动上传协议）
  - [ ] SubTask 6.1: 创建 Dyn200ProactiveListener.h，继承 QObject，声明枚举 `ProtocolMode { Hex6Byte = 0, Hex8Byte = 3, Ascii = 2 }`；持有 QSerialPort 指针、协议模式、内部缓冲区；声明 `dataReceived(double torqueNm, double speedRpm, bool speedValid)` 信号和 `errorOccurred(QString)` 信号
  - [ ] SubTask 6.2: 创建 Dyn200ProactiveListener.cpp，实现 `start(QSerialPort*)`/`stop()` 方法
  - [ ] SubTask 6.3: 实现 `onReadyRead()` 私有槽：
    - HEX 6 字节模式：缓冲区累计到 6 字节后解析（D1D2=扭矩 uint16, D3D4=转速 int16, D3 最高位=扭矩正负, D5D6=CRC16）
    - HEX 8 字节模式：缓冲区累计到 8 字节后解析（D1D2D3=扭矩 3 字节有符号, D4D5D6=转速 3 字节无符号, D7D8=CRC16）
    - ASCII 模式：以 `\r`(0x0D) 为分隔符，解析 `[+-]X.XXXX` 格式扭矩字符串
  - [ ] SubTask 6.4: 实现数据缩放：扭矩 ×0.01 N·m（HEX 模式），ASCII 模式直接使用解析后的浮点值；转速直接 RPM
  - [ ] SubTask 6.5: 实现 CRC16 校验（HEX 模式）和帧碎片拼接（所有模式）
  - [ ] SubTask 6.6: 编写单元测试：使用 QByteArray 模拟串口数据验证三种协议的帧解析（含正常帧、负扭矩、CRC 错误、碎片拼接、高转速值）

- [ ] Task 7: Dyn200TorqueSensorDevice 四种通信模式切换支持
  - [ ] SubTask 7.1: 在 Dyn200TorqueSensorDevice.h 中新增 `m_proactiveListener` 成员、`m_communicationMode` 字段（0=Modbus RTU, 1=HEX6, 2=HEX8, 3=ASCII）；构造函数增加 communicationMode 参数
  - [ ] SubTask 7.2: 修改 `initialize()`：communicationMode=0 保持现有行为；communicationMode=1/2/3 时通过 Modbus RTU 写入寄存器 0x1CH=0/3/2 切换到对应主动上传模式，然后创建并启动 Dyn200ProactiveListener
  - [ ] SubTask 7.3: 修改 `readTorque()`/`readSpeed()`/`readAll()`：当处于主动上传模式时，从 ProactiveListener 的最新缓存值读取；ASCII 模式下转速返回 false
  - [ ] SubTask 7.4: 在代码注释和日志中明确警告：从主动上传切回 Modbus RTU 需要手动按键恢复出厂设置
  - [ ] SubTask 7.5: 编写单元测试验证四种模式的切换逻辑

- [ ] Task 8: SingleTurnEncoderDevice 四种模式 + 虚拟多圈/角速度读取
  - [ ] SubTask 8.1: 在 IEncoderDevice.h 中新增虚函数 `readVirtualMultiTurn(double& totalAngleDeg)` 和 `readAngularVelocity(double& velocityRpm)`
  - [ ] SubTask 8.2: 在 SingleTurnEncoderDevice.h 中新增 `m_communicationMode`（0=查询, 2=自动单圈, 3=自动多圈, 4=自动角速度）和 `m_autoReportIntervalMs` 字段
  - [ ] SubTask 8.3: 实现 `readVirtualMultiTurn()`：读取 2 个寄存器（0x0000~0x0001），组合为 uint32，换算 `angle = value * 360.0 / resolution`
  - [ ] SubTask 8.4: 实现 `readAngularVelocity()`：读取 1 个寄存器（0x0003），有符号 int16，换算 `rpm = value / resolution / (sampleTimeMs / 60000.0)`
  - [ ] SubTask 8.5: 修改 `initialize()`：根据 communicationMode 写入 0x0006 = 0x00/0x01/0x04/0x05 和 0x0007 = 回传周期
  - [ ] SubTask 8.6: 新增 `setAutoReportMode(uint16_t mode, int intervalMs)` 公共方法用于运行时切换
  - [ ] SubTask 8.7: 编写单元测试验证配置写入和数据换算逻辑

- [ ] Task 9: AqmdMotorDriveDevice 新增 0x2B 和 0x10 支持
  - [ ] SubTask 9.1: 在 AqmdMotorDriveDevice.h 中声明 `readDeviceIdentification(QString& vendor, QString& product, QString& version)` 和 `writeMultipleRegisters(uint16_t address, const QVector<uint16_t>& values)`
  - [ ] SubTask 9.2: 实现 `readDeviceIdentification()`：通过 ModbusFrame 0x2B 构建请求，发送并解析响应
  - [ ] SubTask 9.3: 实现 `writeMultipleRegisters()`：通过 ModbusFrame 0x10 构建请求，发送并解析响应；用于 GPIO 批量配置和 EEPROM 存储
  - [ ] SubTask 9.4: 编写单元测试验证 0x2B 和 0x10 调用逻辑

- [ ] Task 10: DeviceConfig 和 StationConfig 扩展
  - [ ] SubTask 10.1: 在 DeviceConfig 中新增 `int communicationMode = 0` 字段
  - [ ] SubTask 10.2: 修改 DeviceConfigService.cpp 的 JSON 解析逻辑，读取 `"communicationMode"` 键
  - [ ] SubTask 10.3: 修改 StationRuntimeFactory.cpp，将 communicationMode 传递给 Dyn200TorqueSensorDevice 和 SingleTurnEncoderDevice 构造函数
  - [ ] SubTask 10.4: 确认 StationConfig 默认值与设备手册一致（AQMD: Even/1stop/9600, DYN200: None/2stop/19200, 编码器: None/1stop/9600, 数控电源: None/1stop/9600）

# Task Dependencies
- Task 6 依赖 Task 3（异常响应解析，用于模式切换时的错误诊断）
- Task 7 依赖 Task 6（Dyn200ProactiveListener 实现）
- Task 8 依赖 Task 1（需要 0x06 写寄存器配置编码器模式）
- Task 9 依赖 Task 1 和 Task 2（需要 0x10 和 0x2B 帧构建）
- Task 10 依赖 Task 7、Task 8 和 Task 9（需要设备接口扩展完成后再更新工厂）
- Task 1, Task 2, Task 3, Task 4, Task 5 可并行执行
