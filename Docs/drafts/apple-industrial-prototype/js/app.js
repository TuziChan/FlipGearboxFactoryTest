import { NAV_ITEMS, PLACEHOLDERS, STAGES } from "./data.js";
import { createInitialState } from "./state.js";

const state = createInitialState();
let timer = null;

const $ = id => document.getElementById(id);
const refs = {
  sidebar: $("sidebar"),
  sidebarToggle: $("sidebarToggle"),
  navList: $("navList"),
  pageTitle: $("pageTitle"),
  pageSubtitle: $("pageSubtitle"),
  toolbarControls: $("toolbarControls"),
  modelSelect: $("modelSelect"),
  serialInput: $("serialInput"),
  backlashInput: $("backlashInput"),
  startButton: $("startButton"),
  stopButton: $("stopButton"),
  executionPage: $("executionPage"),
  placeholderPage: $("placeholderPage"),
  placeholderTag: $("placeholderTag"),
  placeholderTitle: $("placeholderTitle"),
  placeholderDescription: $("placeholderDescription"),
  flowStatusText: $("flowStatusText"),
  flowList: $("flowList"),
  elapsedTotal: $("elapsedTotal"),
  heroMetrics: $("heroMetrics"),
  detailMetrics: $("detailMetrics"),
  detailTitle: $("detailTitle"),
  detailMeta: $("detailMeta"),
  detailContent: $("detailContent"),
  chartToggles: $("chartToggles"),
  chartCanvas: $("chartCanvas"),
  chartEmpty: $("chartEmpty"),
  verdictBadge: $("verdictBadge"),
  verdictStage: $("verdictStage"),
  verdictElapsed: $("verdictElapsed"),
  summaryList: $("summaryList"),
  deviceList: $("deviceList"),
  copyButton: $("copyButton"),
  resetButton: $("resetButton"),
  alertBar: $("alertBar"),
  heartbeatList: $("heartbeatList"),
  clockText: $("clockText")
};

function formatTime(ms) {
  const total = ms / 1000;
  const mins = Math.floor(total / 60);
  const secs = total - mins * 60;
  return `${String(mins).padStart(2, "0")}:${secs.toFixed(1).padStart(4, "0")}`;
}

function tableHtml(headers, rows) {
  return `<table class="detail-table"><thead><tr>${headers.map(label => `<th>${label}</th>`).join("")}</tr></thead><tbody>${
    rows.map(cells => `<tr>${cells.map(cell => {
      const cls = cell === "OK" ? "result-ok" : cell === "NG" ? "result-ng" : "";
      return `<td class="${cls}">${cell}</td>`;
    }).join("")}</tr>`).join("")
  }</tbody></table>`;
}

function metricCard(label, value, unit, note, cls = "") {
  return `<article class="card metric-card"><div class="metric-label">${label}</div><div class="metric-value ${cls}"><strong>${value}</strong><span>${unit}</span></div><div class="metric-note">${note}</div></article>`;
}

function updateStageStates() {
  state.stages.forEach((stage, index) => {
    if (index < state.stageIndex) stage.state = "done";
    else if (index === state.stageIndex) {
      stage.state = state.running ? "current" : state.interrupted ? "interrupted" : "done";
      stage.elapsed = formatTime(state.stageElapsedMs);
    } else stage.state = "pending";
  });
}

function pushSeries(id, value) {
  const series = state.chartSeries.find(item => item.id === id);
  if (!series) return;
  series.values.push(value);
  if (series.values.length > 140) series.values.shift();
}

function updateAngleRows() {
  const targets = [3.0, 49.0, 3.0, 113.5, 0.0];
  state.angleRows = state.angleRows.map((row, index) => {
    const actual = targets[index] + Math.sin(state.elapsedMs / 900 + index) * 1.2;
    const deviation = actual - targets[index];
    return { ...row, currentAngle: `${actual.toFixed(1)}°`, deviation: `${deviation.toFixed(1)}°`, result: Math.abs(deviation) <= 3 ? "OK" : "NG" };
  });
}

function updateLoadRows() {
  const f = 1.36 + Math.sin(state.elapsedMs / 850) * 0.15;
  const r = 1.33 + Math.cos(state.elapsedMs / 920) * 0.12;
  state.loadRows = [
    { direction: "正转", brakeCurrent: `${(0.82 + Math.sin(state.elapsedMs / 1300) * 0.05).toFixed(2)} A`, torque: `${f.toFixed(2)} N·m`, limit: "≥ 1.20 N·m", result: f >= 1.2 ? "OK" : "NG" },
    { direction: "反转", brakeCurrent: `${(0.85 + Math.cos(state.elapsedMs / 1400) * 0.04).toFixed(2)} A`, torque: `${r.toFixed(2)} N·m`, limit: "≥ 1.20 N·m", result: r >= 1.2 ? "OK" : "NG" }
  ];
}

function simulateMetrics() {
  const t = state.elapsedMs / 1000;
  if (state.stageIndex === 0) {
    Object.assign(state.metrics, { speed: 180 + Math.sin(t * 1.2) * 18, torque: 0.12 + Math.cos(t * 0.8) * 0.03, angle: Math.max(0, 36 - t * 8), power: 32, motorCurrent: 0.48, brakeCurrent: 0 });
  } else if (state.stageIndex === 1) {
    Object.assign(state.metrics, { speed: 1220 + Math.sin(t * 1.7) * 45, torque: 0.34 + Math.cos(t) * 0.05, angle: (state.metrics.angle + 2.8) % 360, power: 118, motorCurrent: 2.82, brakeCurrent: 0 });
  } else if (state.stageIndex === 2) {
    Object.assign(state.metrics, { speed: 340 + Math.sin(t * 2.0) * 44, torque: 0.42 + Math.cos(t * 0.9) * 0.06, angle: 44 + Math.sin(t * 0.7) * 18, power: 74, motorCurrent: 1.92, brakeCurrent: 0 });
    updateAngleRows();
  } else if (state.stageIndex === 3) {
    Object.assign(state.metrics, { speed: 760 + Math.sin(t * 1.5) * 64, torque: 1.42 + Math.cos(t * 0.8) * 0.12, angle: 42 + Math.sin(t * 0.4) * 8, power: 130, motorCurrent: 3.04, brakeCurrent: 0.85 });
    updateLoadRows();
  } else if (state.stageIndex === 4) {
    Object.assign(state.metrics, { speed: Math.max(0, 180 - state.stageElapsedMs / 32), torque: Math.max(0, 0.18 - state.stageElapsedMs / 25000), angle: Math.max(0, 12 - state.stageElapsedMs / 700), power: 26, motorCurrent: 0.66, brakeCurrent: 0 });
  }
  pushSeries("speed", state.metrics.speed);
  pushSeries("torque", state.metrics.torque * 500);
  pushSeries("current", state.metrics.motorCurrent * 100);
}

function updateSummary() {
  state.summary = [
    { label: "当前阶段", value: state.phaseLabel || "待机", result: state.running ? "RUN" : state.verdict.toUpperCase() },
    { label: "空载转速", value: `${Math.round(state.metrics.speed)} RPM`, result: state.metrics.speed > 1000 ? "OK" : "WAIT" },
    { label: "锁止扭矩", value: `${state.metrics.torque.toFixed(2)} N·m`, result: state.metrics.torque > 1.2 ? "OK" : "WAIT" },
    { label: "总耗时", value: formatTime(state.elapsedMs), result: state.running ? "RUN" : state.interrupted ? "STOP" : "DONE" }
  ];
}

function renderNav() {
  refs.navList.innerHTML = NAV_ITEMS.map(item => `<button class="nav-item${item.id === state.activePage ? " is-active" : ""}" type="button" data-page="${item.id}"><svg><use href="#${item.icon}"></use></svg><span>${item.title}</span></button>`).join("");
  refs.sidebar.style.width = state.sidebarExpanded ? "184px" : "76px";
  refs.sidebar.querySelectorAll(".nav-item span, .sidebar__toggle span, .brand-copy").forEach(node => {
    node.style.display = state.sidebarExpanded ? "" : "none";
  });
}

function renderHeader() {
  refs.pageTitle.textContent = state.activePage === "execution" ? "测试执行" : PLACEHOLDERS[state.activePage].title;
  refs.pageSubtitle.textContent = state.activePage === "execution" ? "工业测试工具草稿 · Apple restraint" : "导航草稿页";
  refs.toolbarControls.style.visibility = state.activePage === "execution" ? "visible" : "hidden";
  refs.startButton.disabled = state.running || state.activePage !== "execution";
  refs.stopButton.disabled = !state.running || state.activePage !== "execution";
}

function renderFlow() {
  refs.flowStatusText.textContent = state.running ? state.phaseLabel : state.interrupted ? "已中断" : "待机";
  refs.flowList.innerHTML = state.stages.map(stage => {
    const cls = stage.state === "current" ? " is-current" : stage.state === "done" ? " is-done" : "";
    return `<article class="flow-item${cls}"><div class="flow-item__row"><div class="flow-item__dot">${stage.state === "done" ? "✓" : stage.order}</div><div class="flow-item__title">${stage.name}</div><div class="flow-item__time">${stage.elapsed}</div></div><div class="flow-item__detail">${stage.detail}</div></article>`;
  }).join("");
  refs.elapsedTotal.textContent = formatTime(state.elapsedMs);
}

function renderMetrics() {
  refs.heroMetrics.innerHTML = [
    metricCard("转速", Math.round(state.metrics.speed), "RPM", "当前主指标"),
    metricCard("扭矩", state.metrics.torque.toFixed(2), "N·m", "当前主指标", state.metrics.torque > 1.2 ? "metric-value--ok" : ""),
    metricCard("角度", state.metrics.angle.toFixed(1), "°", state.phaseLabel || "等待开始", "metric-value--accent")
  ].join("");
  refs.detailMetrics.innerHTML = [
    metricCard("功率", state.metrics.power.toFixed(0), "W", "设备负载估算"),
    metricCard("电机电流", state.metrics.motorCurrent.toFixed(2), "A", "运行电流"),
    metricCard("制动电流", state.metrics.brakeCurrent.toFixed(2), "A", "负载阶段可见", state.metrics.brakeCurrent > 0 ? "metric-value--warn" : "")
  ].join("");
}

function renderDetail() {
  refs.detailMeta.textContent = state.phaseLabel || "准备/找零";
  if (!state.running && state.stageIndex === -1) {
    refs.detailTitle.textContent = "阶段明细";
    refs.detailContent.innerHTML = `<div class="detail-placeholder">开始测试后，界面只展示当前阶段需要的明细，不再长期保留空白表格。</div>`;
  } else if (state.stageIndex === 2) {
    refs.detailTitle.textContent = "角度定位明细";
    refs.detailContent.innerHTML = tableHtml(["目标位", "目标角度", "当前角度", "偏差", "公差", "判定"], state.angleRows.map(row => [row.targetPoint, row.targetAngle, row.currentAngle, row.deviation, row.tolerance, row.result]));
  } else if (state.stageIndex === 3) {
    refs.detailTitle.textContent = "负载阶段明细";
    refs.detailContent.innerHTML = tableHtml(["方向", "制动电流", "锁止扭矩", "下限", "判定"], state.loadRows.map(row => [row.direction, row.brakeCurrent, row.torque, row.limit, row.result]));
  } else {
    refs.detailTitle.textContent = "当前阶段说明";
    refs.detailContent.innerHTML = `<div class="detail-placeholder"><div><strong>${state.phaseLabel}</strong><br>当前阶段以关键指标和波形为主，表格明细将在角度定位和负载上升阶段自动出现。</div></div>`;
  }
}

function renderChart() {
  refs.chartToggles.innerHTML = state.chartSeries.map(series => `<button class="chart-chip${series.enabled ? " is-active" : ""}" type="button" data-series="${series.id}"><i style="background:${series.color}"></i><span>${series.label}</span></button>`).join("");
  const canvas = refs.chartCanvas;
  const rect = canvas.parentElement.getBoundingClientRect();
  canvas.width = rect.width;
  canvas.height = rect.height;
  const ctx = canvas.getContext("2d");
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  if (!state.running && state.chartSeries.every(item => item.values.length === 0)) {
    refs.chartEmpty.hidden = false;
    return;
  }
  refs.chartEmpty.hidden = true;
  ctx.strokeStyle = "#E8EBF0";
  ctx.lineWidth = 1;
  for (let i = 1; i < 5; i += 1) {
    const y = (canvas.height / 5) * i;
    ctx.beginPath();
    ctx.moveTo(0, y);
    ctx.lineTo(canvas.width, y);
    ctx.stroke();
  }
  state.chartSeries.filter(item => item.enabled).forEach(series => {
    if (series.values.length < 2) return;
    const max = Math.max(...series.values, 1);
    ctx.beginPath();
    ctx.strokeStyle = series.color;
    ctx.lineWidth = 2;
    series.values.forEach((value, index) => {
      const x = index / Math.max(1, series.values.length - 1) * canvas.width;
      const y = canvas.height - value / max * (canvas.height - 24) - 12;
      if (index === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
    });
    ctx.stroke();
  });
}

function renderInspector() {
  refs.verdictBadge.className = `verdict-badge verdict-badge--${state.verdict}`;
  refs.verdictBadge.textContent = state.verdict === "pending" ? "待测试" : state.verdict === "ok" ? "OK" : "中断";
  refs.verdictStage.textContent = state.phaseLabel || "等待开始";
  refs.verdictElapsed.textContent = formatTime(state.elapsedMs);
  refs.summaryList.innerHTML = state.summary.map(item => `<div class="summary-row"><span>${item.label}</span><strong>${item.value}</strong></div>`).join("");
  refs.deviceList.innerHTML = state.devices.map(item => `<div class="device-row"><span class="device-row__label"><i></i>${item.name}</span><strong>${item.state}</strong></div>`).join("");
}

function renderAlert() {
  if (!state.alert) {
    refs.alertBar.hidden = true;
    refs.alertBar.textContent = "";
  } else {
    refs.alertBar.hidden = false;
    refs.alertBar.innerHTML = `<strong>${state.alert.kind === "danger" ? "NG" : "提示"}</strong><span>${state.alert.text}</span>`;
  }
}

function renderHeartbeat() {
  refs.heartbeatList.innerHTML = state.devices.map(item => `<span class="heartbeat-item"><i></i>${item.name}</span>`).join("");
  refs.clockText.textContent = new Date().toLocaleString("zh-CN");
}

function renderPages() {
  const execution = state.activePage === "execution";
  refs.executionPage.hidden = !execution;
  refs.placeholderPage.hidden = execution;
  if (!execution) {
    const placeholder = PLACEHOLDERS[state.activePage];
    refs.placeholderTag.textContent = placeholder.tag;
    refs.placeholderTitle.textContent = placeholder.title;
    refs.placeholderDescription.textContent = placeholder.description;
  }
}

function render() {
  renderNav();
  renderHeader();
  renderPages();
  renderFlow();
  renderMetrics();
  renderDetail();
  renderChart();
  renderInspector();
  renderAlert();
  renderHeartbeat();
}

function finishTest() {
  state.running = false;
  window.clearInterval(timer);
  timer = null;
  if (!state.interrupted) state.verdict = "ok";
  updateStageStates();
  render();
}

function advanceStage() {
  if (state.stageIndex < STAGES.length - 1) {
    state.stageIndex += 1;
    state.stageElapsedMs = 0;
    state.phaseLabel = STAGES[state.stageIndex].name;
    updateStageStates();
  } else {
    finishTest();
  }
}

function tick() {
  if (!state.running) return;
  state.elapsedMs += 180;
  state.stageElapsedMs += 180;
  simulateMetrics();
  updateSummary();
  if (state.stageElapsedMs >= STAGES[state.stageIndex].durationMs) advanceStage();
  render();
}

function startTest() {
  if (refs.serialInput.value.trim().length < 8) {
    state.alert = { kind: "danger", text: "SN 长度至少 8 位，才能开始测试。" };
    renderAlert();
    refs.serialInput.focus();
    return;
  }
  Object.assign(state, createInitialState(), {
    activePage: state.activePage,
    sidebarExpanded: state.sidebarExpanded,
    running: true,
    stageIndex: 0,
    phaseLabel: STAGES[0].name,
    model: refs.modelSelect.value,
    serialNumber: refs.serialInput.value.trim(),
    backlash: refs.backlashInput.value
  });
  updateStageStates();
  updateSummary();
  render();
  timer = window.setInterval(tick, 180);
}

function stopTest() {
  if (!state.running) return;
  state.running = false;
  state.interrupted = true;
  state.verdict = "interrupted";
  state.alert = { kind: "danger", text: "急停触发，测试已中断。请复核工位状态后再重置。" };
  window.clearInterval(timer);
  timer = null;
  updateStageStates();
  render();
}

function resetTest() {
  const page = state.activePage;
  const sidebar = state.sidebarExpanded;
  Object.assign(state, createInitialState(), { activePage: page, sidebarExpanded: sidebar });
  render();
}

refs.sidebarToggle.addEventListener("click", () => {
  state.sidebarExpanded = !state.sidebarExpanded;
  renderNav();
});

refs.navList.addEventListener("click", event => {
  const button = event.target.closest(".nav-item");
  if (!button) return;
  state.activePage = button.dataset.page;
  render();
});

refs.chartToggles.addEventListener("click", event => {
  const chip = event.target.closest(".chart-chip");
  if (!chip) return;
  const series = state.chartSeries.find(item => item.id === chip.dataset.series);
  if (!series) return;
  series.enabled = !series.enabled;
  renderChart();
});

refs.startButton.addEventListener("click", startTest);
refs.stopButton.addEventListener("click", stopTest);
refs.resetButton.addEventListener("click", resetTest);
refs.copyButton.addEventListener("click", async () => {
  const report = ["齿轮箱测试草稿报告", `型号: ${refs.modelSelect.value}`, `SN: ${refs.serialInput.value}`, `阶段: ${state.phaseLabel}`, `结论: ${refs.verdictBadge.textContent}`].join("\n");
  try { await navigator.clipboard.writeText(report); state.alert = { kind: "success", text: "报告内容已复制到剪贴板。" }; }
  catch { state.alert = { kind: "success", text: "浏览器未授权剪贴板，草稿中仅模拟复制成功提示。" }; }
  renderAlert();
});

render();
