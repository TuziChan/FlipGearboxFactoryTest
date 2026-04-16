export const NAV_ITEMS = [
  { id: "execution", title: "测试执行", icon: "icon-grid" },
  { id: "recipe", title: "配方管理", icon: "icon-list" },
  { id: "history", title: "历史记录", icon: "icon-trend" },
  { id: "device", title: "设备配置", icon: "icon-doc" },
  { id: "diagnostics", title: "I/O 诊断", icon: "icon-gauge" }
];

export const PLACEHOLDERS = {
  recipe: {
    tag: "RECIPE",
    title: "配方管理",
    description: "配方管理页在这个草稿里只保留轻量占位。正式版应承载型号参数模板、阈值版本和测试策略。"
  },
  history: {
    tag: "HISTORY",
    title: "历史记录",
    description: "历史记录页应聚焦检索、筛选、回放和追溯，而不是复用测试执行页的三栏结构。"
  },
  device: {
    tag: "DEVICE",
    title: "设备配置",
    description: "设备配置页应集中处理串口映射、总线地址和模式设置，不与执行页抢主视觉。"
  },
  diagnostics: {
    tag: "DIAGNOSTICS",
    title: "I/O 诊断",
    description: "I/O 诊断页应强调设备联机和单步操作反馈，用工具型布局而不是大面积指标卡。"
  }
};

export const STAGES = [
  { id: "prep", name: "准备/找零", detail: "编码器归零并建立补偿", durationMs: 4200 },
  { id: "idle", name: "空载正反转", detail: "采集正反转电流与转速", durationMs: 9200 },
  { id: "angle", name: "角度定位", detail: "执行五个目标位定位判定", durationMs: 9800 },
  { id: "load", name: "负载上升", detail: "锁止并采集制动扭矩", durationMs: 7600 },
  { id: "home", name: "回零结束", detail: "回到零位并汇总结果", durationMs: 3600 }
];

export const ANGLE_TARGETS = [
  { point: "①", target: 3.0, tolerance: 3.0 },
  { point: "②", target: 49.0, tolerance: 3.0 },
  { point: "①", target: 3.0, tolerance: 3.0 },
  { point: "③", target: 113.5, tolerance: 3.0 },
  { point: "零", target: 0.0, tolerance: 3.0 }
];

export const DEVICES = [
  { name: "CAN 心跳", state: "在线" },
  { name: "DYN200", state: "在线" },
  { name: "AQMD", state: "在线" },
  { name: "编码器", state: "在线" },
  { name: "磁粉制动", state: "在线" }
];

export const CHART_SERIES = [
  { id: "speed", label: "转速", color: "#0A84FF", enabled: true },
  { id: "torque", label: "扭矩", color: "#28A164", enabled: true },
  { id: "current", label: "电流", color: "#D18B1F", enabled: false }
];
