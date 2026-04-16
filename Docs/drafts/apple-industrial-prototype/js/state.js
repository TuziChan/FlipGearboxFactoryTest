import { ANGLE_TARGETS, CHART_SERIES, DEVICES, STAGES } from "./data.js";

export function createInitialState() {
  return {
    activePage: "execution",
    sidebarExpanded: true,
    running: false,
    interrupted: false,
    stageIndex: -1,
    elapsedMs: 0,
    stageElapsedMs: 0,
    phaseLabel: "待机",
    verdict: "pending",
    alert: null,
    serialNumber: "GBX42A-20260415-001",
    model: "GBX-42A",
    backlash: "1.2",
    metrics: {
      speed: 0,
      torque: 0,
      angle: 0,
      power: 0,
      motorCurrent: 0,
      brakeCurrent: 0
    },
    summary: [
      { label: "当前阶段", value: "待机", result: "PENDING" },
      { label: "空载转速", value: "--", result: "WAIT" },
      { label: "锁止扭矩", value: "--", result: "WAIT" },
      { label: "总耗时", value: "00:00.0", result: "IDLE" }
    ],
    devices: DEVICES.map(item => ({ ...item })),
    stages: STAGES.map((stage, index) => ({
      ...stage,
      order: index + 1,
      state: "pending",
      elapsed: "00:00.0"
    })),
    angleRows: ANGLE_TARGETS.map(item => ({
      targetPoint: item.point,
      targetAngle: `${item.target.toFixed(1)}°`,
      currentAngle: "--",
      deviation: "--",
      tolerance: `±${item.tolerance.toFixed(1)}°`,
      result: "待测"
    })),
    loadRows: [
      { direction: "正转", brakeCurrent: "--", torque: "--", limit: "≥ 1.20 N·m", result: "待测" },
      { direction: "反转", brakeCurrent: "--", torque: "--", limit: "≥ 1.20 N·m", result: "待测" }
    ],
    chartSeries: CHART_SERIES.map(item => ({ ...item, values: [] }))
  };
}
