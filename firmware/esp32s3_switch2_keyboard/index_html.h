static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0">
<title>Switch 2 Keyboard Bridge</title>
<style>
:root {
  color-scheme: dark;
  --bg: #0f172a;
  --panel: rgba(15, 23, 42, 0.86);
  --panel-strong: rgba(15, 23, 42, 0.96);
  --key: #1e293b;
  --key-border: #334155;
  --key-active: rgba(248,250,252,.66);
  --key-mapped: #2563eb;
  --text: #e2e8f0;
  --muted: #94a3b8;
  --danger: #ef4444;
  --warn: #f59e0b;
  --ok: #22c55e;
  --accent: #38bdf8;
}
* { box-sizing: border-box; }
html, body { margin: 0; min-height: 100%; font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; color: var(--text); background: radial-gradient(circle at top, #1e293b, var(--bg)); }
html.overlay, body.overlay { background: transparent !important; overflow: hidden; }
body.overlay { color: white; }
button, select, input, textarea { font: inherit; }
button { border: 0; border-radius: 10px; padding: 9px 12px; color: var(--text); background: #334155; cursor: pointer; }
button:hover { filter: brightness(1.12); }
button.primary { background: #2563eb; }
button.ghost { background: rgba(148, 163, 184, 0.14); }
button.danger { background: #7f1d1d; }
.app { min-height: 100vh; padding: 18px; display: grid; gap: 16px; grid-template-columns: minmax(720px, 1fr) 360px; }
.header { grid-column: 1 / -1; display: flex; align-items: center; justify-content: space-between; gap: 12px; padding: 14px 16px; border: 1px solid rgba(148,163,184,.22); border-radius: 18px; background: var(--panel); box-shadow: 0 12px 40px rgba(0,0,0,.25); }
.header h1 { font-size: 20px; margin: 0; letter-spacing: -0.02em; }
.header .sub { color: var(--muted); font-size: 13px; }
.pill-row { display: flex; flex-wrap: wrap; gap: 8px; justify-content: flex-end; }
.pill { border: 1px solid rgba(148,163,184,.22); border-radius: 999px; padding: 6px 10px; background: rgba(15,23,42,.6); color: var(--muted); font-size: 12px; }
.pill.ok { color: #86efac; border-color: rgba(34,197,94,.4); }
.pill.warn { color: #fcd34d; border-color: rgba(245,158,11,.45); }
.pill.bad { color: #fca5a5; border-color: rgba(239,68,68,.45); }
.keyboard-panel, .side-panel { border: 1px solid rgba(148,163,184,.22); border-radius: 20px; background: var(--panel); box-shadow: 0 12px 40px rgba(0,0,0,.24); }
.keyboard-panel { padding: 16px; overflow: auto; }
.keyboard { min-width: 900px; display: flex; flex-direction: column; gap: 8px; }
.key-row { display: flex; gap: 8px; }
.key { position: relative; min-width: 54px; height: 56px; padding: 7px 8px; border-radius: 12px; border: 1px solid var(--key-border); background: linear-gradient(180deg, #273449, var(--key)); color: var(--text); text-align: left; box-shadow: inset 0 -2px 0 rgba(0,0,0,.22); }
.key:hover { outline: 2px solid rgba(56,189,248,.35); }
.key.selected { outline: 2px solid var(--accent); }
.key.mapped { border-color: rgba(37,99,235,.75); box-shadow: inset 0 -2px 0 rgba(0,0,0,.22), 0 0 0 1px rgba(37,99,235,.3); }
.key.active { background: rgba(248,250,252,.66); border-color: rgba(255,255,255,.84); color: #0f172a; box-shadow: 0 0 0 1px rgba(255,255,255,.22), inset 0 -2px 0 rgba(0,0,0,.10); }
.key.toggle-on { box-shadow: inset 0 -2px 0 rgba(0,0,0,.22), 0 0 0 2px rgba(245,158,11,.7); }
.key .code { display: block; font-size: 12px; color: #cbd5e1; }
.key .map { position: absolute; left: 8px; right: 8px; bottom: 7px; color: #bfdbfe; font-size: 11px; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.key.active .map, .key.active .code { color: #0f172a; }
.w05 { flex: .55; min-width: 32px; opacity: .35; pointer-events: none; }
.w1 { flex: 1; } .w125 { flex: 1.25; } .w15 { flex: 1.5; } .w175 { flex: 1.75; } .w2 { flex: 2; } .w225 { flex: 2.25; } .w275 { flex: 2.75; } .w625 { flex: 6.25; }
.side-panel { padding: 16px; display: flex; flex-direction: column; gap: 14px; max-height: calc(100vh - 110px); overflow: auto; }
.section { padding: 12px; border: 1px solid rgba(148,163,184,.18); border-radius: 16px; background: rgba(15,23,42,.55); }
.section h2 { font-size: 14px; margin: 0 0 10px; color: #f8fafc; }
.field { display: grid; gap: 6px; margin: 10px 0; }
.field label { font-size: 12px; color: var(--muted); }
select, input[type="number"], textarea { width: 100%; border-radius: 10px; border: 1px solid rgba(148,163,184,.25); background: #020617; color: var(--text); padding: 9px 10px; }
textarea { min-height: 120px; resize: vertical; font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace; font-size: 12px; }
.button-row { display: flex; flex-wrap: wrap; gap: 8px; }
.status-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
.stat { padding: 8px; border-radius: 12px; background: rgba(30,41,59,.75); }
.stat .label { color: var(--muted); font-size: 11px; }
.stat .value { margin-top: 4px; font-size: 13px; }
.overlay-wrap { display: none; width: 100vw; height: 100vh; padding: 18px; box-sizing: border-box; align-items: flex-end; justify-content: center; background: transparent; pointer-events: none; }
body.overlay .app { display: none; }
body.overlay .overlay-wrap { display: flex; }
.overlay-card { width: min(96vw, 980px); border-radius: 18px; padding: 8px; background: transparent; border: 0; backdrop-filter: none; }
.overlay-keys { display: flex; justify-content: center; }
.overlay-pad { display: grid; grid-template-columns: repeat(24, 37px); grid-template-rows: repeat(2, 34px); gap: 5px; align-items: stretch; justify-items: stretch; }
.overlay-key { min-width: 0; min-height: 0; border-radius: 9px; padding: 4px 5px; border: 1px solid rgba(226,232,240,.38); background: rgba(15,23,42,.42); color: white; text-align: center; text-shadow: 0 1px 3px black, 0 0 6px black; box-shadow: 0 2px 10px rgba(0,0,0,.18); overflow: hidden; }
.overlay-key.empty { visibility: hidden; }
.overlay-key.active { background: rgba(248,250,252,.64); border-color: rgba(255,255,255,.82); color: #0f172a; text-shadow: none; box-shadow: 0 0 0 1px rgba(255,255,255,.24), 0 2px 10px rgba(0,0,0,.18); }
.overlay-key.active .m { color: #1e3a8a; }
.overlay-key .k { font-weight: 700; font-size: 11px; line-height: 1.05; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.overlay-key .m { margin-top: 1px; font-size: 9px; color: #bfdbfe; line-height: 1.05; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.overlay-key.anchor { background: rgba(15,23,42,.24); border-color: rgba(226,232,240,.20); opacity: .72; }
.overlay-status { display: flex; gap: 6px; justify-content: center; margin-bottom: 6px; color: white; font-size: 11px; }
.overlay-status span { padding: 3px 7px; border-radius: 999px; background: rgba(15,23,42,.30); border: 1px solid rgba(226,232,240,.18); text-shadow: 0 1px 3px black, 0 0 6px black; }
@media (max-width: 980px) { .overlay-pad { grid-template-columns: repeat(24, 32px); grid-template-rows: repeat(2, 31px); gap: 4px; } .overlay-key .k { font-size: 10px; } .overlay-key .m { font-size: 8px; } }
@media (max-width: 820px) { .overlay-card { transform: scale(.86); transform-origin: bottom center; } }
.hidden { display: none !important; }
@media (max-width: 1100px) { .app { grid-template-columns: 1fr; } .side-panel { max-height: none; } }
</style>
</head>
<body>
<div class="app">
  <header class="header">
    <div>
      <h1>Switch 2 Keyboard Bridge</h1>
      <div class="sub">Keyboard → WebSocket → ESP32-S3 → USB HID → Switch 2</div>
    </div>
    <div class="pill-row">
      <span id="wsPill" class="pill bad">WS closed</span>
      <span id="modePill" class="pill">L full · R full</span>
      <span id="rttPill" class="pill">RTT -- ms</span>
      <a class="pill" href="?overlay=1" target="_blank">Open OBS overlay</a>
    </div>
  </header>
  <main class="keyboard-panel"><div id="keyboard" class="keyboard"></div></main>
  <aside class="side-panel">
    <section class="section">
      <h2>Key mapping</h2>
      <div class="field"><label>Selected key</label><input id="selectedKey" readonly value="Click a key"></div>
      <div class="field"><label>Mapped Switch control</label><select id="controlSelect"></select></div>
      <div class="button-row"><button id="assignBtn" class="primary">Assign</button><button id="clearBtn" class="ghost">Clear</button><button id="resetBtn" class="danger">Reset defaults</button></div>
    </section>
    <section class="section">
      <h2>Stick sensitivity</h2>
      <div class="field"><label>Half stroke percent</label><input id="halfPercent" type="number" min="5" max="95" step="1"></div>
      <div class="field"><label>Left stick toggle key</label><select id="leftToggle"></select></div>
      <div class="field"><label>Right stick toggle key</label><select id="rightToggle"></select></div>
      <div class="button-row"><button id="toggleLeft" class="ghost">Toggle L</button><button id="toggleRight" class="ghost">Toggle R</button></div>
    </section>
    <section class="section">
      <h2>Import / export</h2>
      <textarea id="jsonBox" spellcheck="false"></textarea>
      <div class="button-row"><button id="exportBtn" class="ghost">Export</button><button id="importBtn" class="primary">Import</button></div>
    </section>
    <section class="section">
      <h2>Telemetry</h2>
      <div class="status-grid">
        <div class="stat"><div class="label">Socket</div><div id="socketStat" class="value">closed</div></div>
        <div class="stat"><div class="label">Reconnects</div><div id="reconnectStat" class="value">0</div></div>
        <div class="stat"><div class="label">Buffered</div><div id="bufferStat" class="value">0</div></div>
        <div class="stat"><div class="label">Last input</div><div id="lastInputStat" class="value">--</div></div>
      </div>
    </section>
  </aside>
</div>
<div class="overlay-wrap"><div class="overlay-card"><div id="overlayStatus" class="overlay-status"></div><div id="overlayKeys" class="overlay-keys"></div></div></div>
<script>
(function(){
'use strict';
const isOverlay = new URLSearchParams(location.search).has('overlay');
if (isOverlay) { document.documentElement.classList.add('overlay'); document.body.classList.add('overlay'); }
const PROTOCOL_VERSION = 1;
const SESSION_ID = 'ui-' + Math.random().toString(36).slice(2) + Date.now().toString(36);
const STORAGE_KEY = 'switch2KeyboardBridge.v1';
const SEND_REPEAT_MS = 20;
const MIN_PRESS_MS = 90;
const MAX_BUFFERED_AMOUNT = 4096;
const CENTER = 128;
const CONTROL_DEFS = [
  {id:'Y', label:'Y', kind:'button', code:0}, {id:'B', label:'B', kind:'button', code:1}, {id:'A', label:'A', kind:'button', code:2}, {id:'X', label:'X', kind:'button', code:3},
  {id:'L', label:'L', kind:'button', code:4}, {id:'R', label:'R', kind:'button', code:5}, {id:'ZL', label:'ZL', kind:'button', code:6}, {id:'ZR', label:'ZR', kind:'button', code:7},
  {id:'Minus', label:'-', kind:'button', code:8}, {id:'Plus', label:'+', kind:'button', code:9}, {id:'LSB', label:'LSB', kind:'button', code:10}, {id:'RSB', label:'RSB', kind:'button', code:11},
  {id:'Home', label:'Home', kind:'button', code:12}, {id:'Capture', label:'Capture', kind:'button', code:13},
  {id:'DPadUp', label:'D↑', kind:'dpad', code:16}, {id:'DPadDown', label:'D↓', kind:'dpad', code:17}, {id:'DPadLeft', label:'D←', kind:'dpad', code:18}, {id:'DPadRight', label:'D→', kind:'dpad', code:19},
  {id:'LeftStickUp', label:'L↑', kind:'axis', code:20, stick:'left', axis:'y', dir:-1}, {id:'LeftStickDown', label:'L↓', kind:'axis', code:21, stick:'left', axis:'y', dir:1},
  {id:'LeftStickLeft', label:'L←', kind:'axis', code:22, stick:'left', axis:'x', dir:-1}, {id:'LeftStickRight', label:'L→', kind:'axis', code:23, stick:'left', axis:'x', dir:1},
  {id:'RightStickUp', label:'R↑', kind:'axis', code:24, stick:'right', axis:'y', dir:-1}, {id:'RightStickDown', label:'R↓', kind:'axis', code:25, stick:'right', axis:'y', dir:1},
  {id:'RightStickLeft', label:'R←', kind:'axis', code:26, stick:'right', axis:'x', dir:-1}, {id:'RightStickRight', label:'R→', kind:'axis', code:27, stick:'right', axis:'x', dir:1}
];
const CONTROL_BY_ID = Object.fromEntries(CONTROL_DEFS.map(c => [c.id, c]));
const CONTROL_BY_CODE = Object.fromEntries(CONTROL_DEFS.map(c => [c.code, c]));
const KEY_ROWS = [
  [{c:'Escape',l:'Esc',w:'w1'}, {c:'Digit1',l:'1'}, {c:'Digit2',l:'2'}, {c:'Digit3',l:'3'}, {c:'Digit4',l:'4'}, {c:'Digit5',l:'5'}, {c:'Digit6',l:'6'}, {c:'Digit7',l:'7'}, {c:'Digit8',l:'8'}, {c:'Digit9',l:'9'}, {c:'Digit0',l:'0'}, {c:'Minus',l:'-'}, {c:'Equal',l:'='}, {c:'Backspace',l:'Backspace',w:'w2'}],
  [{c:'Tab',l:'Tab',w:'w15'}, {c:'KeyQ',l:'Q'}, {c:'KeyW',l:'W'}, {c:'KeyE',l:'E'}, {c:'KeyR',l:'R'}, {c:'KeyT',l:'T'}, {c:'KeyY',l:'Y'}, {c:'KeyU',l:'U'}, {c:'KeyI',l:'I'}, {c:'KeyO',l:'O'}, {c:'KeyP',l:'P'}, {c:'BracketLeft',l:'['}, {c:'BracketRight',l:']'}, {c:'Backslash',l:'\\',w:'w15'}],
  [{c:'CapsLock',l:'Caps',w:'w175'}, {c:'KeyA',l:'A'}, {c:'KeyS',l:'S'}, {c:'KeyD',l:'D'}, {c:'KeyF',l:'F'}, {c:'KeyG',l:'G'}, {c:'KeyH',l:'H'}, {c:'KeyJ',l:'J'}, {c:'KeyK',l:'K'}, {c:'KeyL',l:'L'}, {c:'Semicolon',l:';'}, {c:'Quote',l:"'"}, {c:'Enter',l:'Enter',w:'w225'}],
  [{c:'ShiftLeft',l:'Shift',w:'w225'}, {c:'KeyZ',l:'Z'}, {c:'KeyX',l:'X'}, {c:'KeyC',l:'C'}, {c:'KeyV',l:'V'}, {c:'KeyB',l:'B'}, {c:'KeyN',l:'N'}, {c:'KeyM',l:'M'}, {c:'Comma',l:','}, {c:'Period',l:'.'}, {c:'Slash',l:'/'}, {c:'ShiftRight',l:'Shift',w:'w275'}],
  [{c:'ControlLeft',l:'Ctrl',w:'w125'}, {c:'MetaLeft',l:'⌘',w:'w125'}, {c:'AltLeft',l:'Alt',w:'w125'}, {c:'Space',l:'Space',w:'w625'}, {c:'AltRight',l:'Alt',w:'w125'}, {c:'MetaRight',l:'⌘',w:'w125'}, {c:'ArrowLeft',l:'←'}, {c:'ArrowUp',l:'↑'}, {c:'ArrowDown',l:'↓'}, {c:'ArrowRight',l:'→'}]
];
const ALL_KEYS = KEY_ROWS.flat().map(k => k.c);
const DEFAULT_STATE = {
  keymap: { KeyW:'LeftStickUp', KeyS:'LeftStickDown', KeyA:'LeftStickLeft', KeyD:'LeftStickRight', ArrowUp:'RightStickUp', ArrowDown:'RightStickDown', ArrowLeft:'RightStickLeft', ArrowRight:'RightStickRight', KeyJ:'B', KeyK:'A', KeyU:'Y', KeyI:'X', KeyQ:'ZL', KeyE:'ZR', Enter:'Plus', Backspace:'Minus', Space:'B' },
  settings: { halfPercent: 50, leftToggleKey:'ControlLeft', rightToggleKey:'AltLeft' },
  modes: { left:'full', right:'full' },
  pressed: {}
};
let state = loadState();
let selectedCode = 'KeyW';
let websock = null;
let connected = false;
let reconnects = 0;
let reconnectTimer = null;
let lastRtt = null;
let lastInputAt = 0;
let liveSnapshotReceived = false;
const releaseTimers = new Map();
const repeatTimers = new Map();
const pressStartedAt = new Map();
function clone(x){ return JSON.parse(JSON.stringify(x)); }
function loadState(){ try { return normalizeState(JSON.parse(localStorage.getItem(STORAGE_KEY))); } catch(e) { return clone(DEFAULT_STATE); } }
function sanitizeKeymap(rawMap, fallback){ const out = {}; const source = rawMap && typeof rawMap === 'object' ? rawMap : fallback; Object.entries(source || {}).forEach(([code, controlId]) => { if (ALL_KEYS.includes(code) && CONTROL_BY_ID[controlId]) out[code] = controlId; }); return out; }
function sanitizeSettings(rawSettings, fallback){ const source = rawSettings && typeof rawSettings === 'object' ? rawSettings : {}; const base = Object.assign({}, fallback, source); const half = Math.max(5, Math.min(95, Number(base.halfPercent) || fallback.halfPercent)); return { halfPercent: half, leftToggleKey: ALL_KEYS.includes(base.leftToggleKey) ? base.leftToggleKey : fallback.leftToggleKey, rightToggleKey: ALL_KEYS.includes(base.rightToggleKey) ? base.rightToggleKey : fallback.rightToggleKey }; }
function sanitizeModes(rawModes, fallback){ const source = rawModes && typeof rawModes === 'object' ? rawModes : {}; const left = source.left === 'half' || source.left === 'full' ? source.left : fallback.left; const right = source.right === 'half' || source.right === 'full' ? source.right : fallback.right; return {left, right}; }
function sanitizePressed(rawPressed){ const out = {}; if (!rawPressed || typeof rawPressed !== 'object') return out; Object.entries(rawPressed).forEach(([id, value]) => { const code = id.startsWith('key:') ? id.slice(4) : ''; if (value === true && ALL_KEYS.includes(code)) out[id] = true; }); return out; }
function normalizeState(raw){ const next = clone(DEFAULT_STATE); if (!raw || typeof raw !== 'object') return next; if ('keymap' in raw) next.keymap = sanitizeKeymap(raw.keymap, {}); next.settings = sanitizeSettings(raw.settings, DEFAULT_STATE.settings); next.modes = sanitizeModes(raw.modes, DEFAULT_STATE.modes); next.pressed = sanitizePressed(raw.pressed); return next; }
function persist(){ if (!isOverlay) localStorage.setItem(STORAGE_KEY, JSON.stringify({keymap: state.keymap, settings: state.settings, modes: state.modes})); }
function controlForKey(code){ return CONTROL_BY_ID[state.keymap[code]] || null; }
function keyLabel(code){ const found = KEY_ROWS.flat().find(k => k.c === code); return found ? found.l : code.replace(/^Key|^Digit/, ''); }
function stickValue(control){ if (!control || control.kind !== 'axis') return undefined; const mode = state.modes[control.stick] || 'full'; const pct = Math.max(5, Math.min(95, Number(state.settings.halfPercent) || 50)); if (mode === 'full') return control.dir < 0 ? 0 : 255; const delta = Math.round(127 * pct / 100); return control.dir < 0 ? CENTER - delta : CENTER + delta; }
function snapshot(){ return {v:PROTOCOL_VERSION, type:'state_snapshot', owner:SESSION_ID, keymap:state.keymap, settings:state.settings, modes:state.modes, pressed:state.pressed, ts:Date.now()}; }
function statePatch(reason){ return {v:PROTOCOL_VERSION, type:'state_patch', owner:SESSION_ID, reason, keymap:state.keymap, settings:state.settings, modes:state.modes, pressed:state.pressed, ts:Date.now()}; }
function inputMsg(action, control){ const msg = {v:PROTOCOL_VERSION, type:'input', action, control:control.code, id:control.id, ts:performance.now()}; const value = stickValue(control); if (value !== undefined && action === 'down') msg.value = value; return msg; }
function wsSend(obj){ if (!connected || !websock || websock.readyState !== WebSocket.OPEN) return false; if (websock.bufferedAmount > MAX_BUFFERED_AMOUNT) { updateTelemetry(); return false; } websock.send(JSON.stringify(obj)); updateTelemetry(); return true; }
function publishSnapshot(reason){ persist(); wsSend(reason === 'snapshot' ? snapshot() : statePatch(reason || 'update')); renderAll(); }
function startRepeating(id, control){ clearTimeout(releaseTimers.get(id)); releaseTimers.delete(id); if (repeatTimers.has(id)) return; pressStartedAt.set(id, performance.now()); state.pressed[id] = true; lastInputAt = performance.now(); wsSend(inputMsg('down', control)); repeatTimers.set(id, setInterval(() => wsSend(inputMsg('down', control)), SEND_REPEAT_MS)); publishSnapshot('press'); }
function stopRepeating(id, control){ const started = pressStartedAt.get(id) || performance.now(); const delay = Math.max(0, MIN_PRESS_MS - (performance.now() - started)); clearInterval(repeatTimers.get(id)); repeatTimers.delete(id); pressStartedAt.delete(id); clearTimeout(releaseTimers.get(id)); releaseTimers.set(id, setTimeout(() => { wsSend(inputMsg('up', control)); setTimeout(() => wsSend(inputMsg('up', control)), SEND_REPEAT_MS); setTimeout(() => wsSend(inputMsg('up', control)), SEND_REPEAT_MS * 2); delete state.pressed[id]; releaseTimers.delete(id); lastInputAt = performance.now(); publishSnapshot('release'); }, delay)); }
function toggleStick(which){ state.modes[which] = state.modes[which] === 'half' ? 'full' : 'half'; publishSnapshot('stick-mode'); }
function renderKeyboard(){ const root = document.getElementById('keyboard'); if (!root) return; root.innerHTML = ''; for (const row of KEY_ROWS) { const rowEl = document.createElement('div'); rowEl.className = 'key-row'; for (const key of row) { const btn = document.createElement('button'); btn.type = 'button'; btn.className = 'key ' + (key.w || 'w1'); btn.dataset.code = key.c; const mapped = controlForKey(key.c); const active = !!state.pressed['key:' + key.c]; if (mapped) btn.classList.add('mapped'); if (active) btn.classList.add('active'); if (key.c === selectedCode) btn.classList.add('selected'); if ((key.c === state.settings.leftToggleKey && state.modes.left === 'half') || (key.c === state.settings.rightToggleKey && state.modes.right === 'half')) btn.classList.add('toggle-on'); btn.innerHTML = '<span class="code"></span><span class="map"></span>'; btn.querySelector('.code').textContent = key.l; btn.querySelector('.map').textContent = mapped ? mapped.label : ''; btn.addEventListener('click', () => { selectedCode = key.c; renderAll(); }); rowEl.appendChild(btn); } root.appendChild(rowEl); } }
function renderEditor(){ const selected = document.getElementById('selectedKey'); if (!selected) return; selected.value = selectedCode + ' (' + keyLabel(selectedCode) + ')'; const controlSelect = document.getElementById('controlSelect'); controlSelect.innerHTML = '<option value="">-- unmapped --</option>' + CONTROL_DEFS.map(c => '<option value="' + c.id + '">' + c.label + ' · ' + c.id + '</option>').join(''); controlSelect.value = state.keymap[selectedCode] || ''; const leftToggle = document.getElementById('leftToggle'); const rightToggle = document.getElementById('rightToggle'); const keyOptions = ALL_KEYS.map(c => '<option value="' + c + '">' + keyLabel(c) + ' · ' + c + '</option>').join(''); leftToggle.innerHTML = keyOptions; rightToggle.innerHTML = keyOptions; leftToggle.value = state.settings.leftToggleKey; rightToggle.value = state.settings.rightToggleKey; document.getElementById('halfPercent').value = state.settings.halfPercent; document.getElementById('jsonBox').value = JSON.stringify({keymap:state.keymap, settings:state.settings}, null, 2); }
function renderTelemetry(){ const wsClass = connected ? 'pill ok' : 'pill bad'; const wsText = connected ? 'WS open' : 'WS closed'; const wsPill = document.getElementById('wsPill'); if (wsPill) { wsPill.className = wsClass; wsPill.textContent = wsText; } const modeText = 'L ' + state.modes.left + ' · R ' + state.modes.right; const modePill = document.getElementById('modePill'); if (modePill) modePill.textContent = modeText; const rttPill = document.getElementById('rttPill'); if (rttPill) rttPill.textContent = 'RTT ' + (lastRtt == null ? '--' : Math.round(lastRtt)) + ' ms'; const socketStat = document.getElementById('socketStat'); if (socketStat) socketStat.textContent = websock ? ['CONNECTING','OPEN','CLOSING','CLOSED'][websock.readyState] : 'closed'; const reconnectStat = document.getElementById('reconnectStat'); if (reconnectStat) reconnectStat.textContent = reconnects; const bufferStat = document.getElementById('bufferStat'); if (bufferStat) bufferStat.textContent = websock ? websock.bufferedAmount : 0; const lastInput = document.getElementById('lastInputStat'); if (lastInput) lastInput.textContent = lastInputAt ? Math.round(performance.now() - lastInputAt) + ' ms ago' : '--'; }
const OVERLAY_LAYOUT = [
  {id:'LSB', c:1, r:1}, {id:'ZL', c:2, r:1}, {id:'L', c:3, r:1},
  {id:'LeftStickUp', c:5, r:1}, {id:'LeftStickLeft', c:4, r:2}, {id:'LeftStickDown', c:5, r:2}, {id:'LeftStickRight', c:6, r:2},
  {id:'DPadUp', c:8, r:1}, {id:'DPadLeft', c:7, r:2}, {id:'DPadDown', c:8, r:2}, {id:'DPadRight', c:9, r:2},
  {id:'Minus', c:11, r:1}, {id:'Capture', c:12, r:1}, {id:'Home', c:13, r:1}, {id:'Plus', c:14, r:1},
  {id:'X', c:16, r:1}, {id:'Y', c:17, r:1}, {id:'B', c:16, r:2}, {id:'A', c:17, r:2},
  {id:'RightStickUp', c:20, r:1}, {id:'RightStickLeft', c:19, r:2}, {id:'RightStickDown', c:20, r:2}, {id:'RightStickRight', c:21, r:2},
  {id:'RSB', c:22, r:1}, {id:'R', c:23, r:1}, {id:'ZR', c:24, r:1}
];
function overlayKeyCodesForControl(controlId){ const order = Object.fromEntries(ALL_KEYS.map((code, i) => [code, i])); return Object.keys(state.keymap).filter(code => ALL_KEYS.includes(code) && state.keymap[code] === controlId).sort((a,b) => order[a] - order[b]); }
function overlayKeyLabel(code){ const label = keyLabel(code); const short = {Backspace:'Bksp', ControlLeft:'Ctrl', ControlRight:'Ctrl', ShiftLeft:'Shift', ShiftRight:'Shift', ArrowLeft:'←', ArrowRight:'→', ArrowUp:'↑', ArrowDown:'↓', BracketLeft:'[', BracketRight:']', Backslash:'\\', Semicolon:';', Quote:"'", Slash:'/', Period:'.', Comma:',', Minus:'-', Equal:'='}; return short[code] || short[label] || label; }
function appendOverlayCell(parent, def){ const c = CONTROL_BY_ID[def.id]; const codes = overlayKeyCodesForControl(def.id); const item = document.createElement('div'); item.className = 'overlay-key' + (!codes.length ? ' empty' : '') + (codes.some(code => state.pressed['key:' + code]) ? ' active' : ''); item.style.gridColumn = String(def.c); item.style.gridRow = String(def.r); const k = document.createElement('div'); k.className = 'k'; k.textContent = codes.map(overlayKeyLabel).join('/'); const m = document.createElement('div'); m.className = 'm'; m.textContent = c ? c.label : def.id; item.append(k, m); parent.appendChild(item); }
function renderOverlay(){ const status = document.getElementById('overlayStatus'); const keys = document.getElementById('overlayKeys'); if (!status || !keys) return; const live = liveSnapshotReceived || !isOverlay; status.textContent = ''; [connected ? 'WS open' : 'WS closed', live ? 'live' : 'waiting live state', 'L ' + state.modes.left, 'R ' + state.modes.right, 'half ' + state.settings.halfPercent + '%'].forEach(text => { const span = document.createElement('span'); span.textContent = text; status.appendChild(span); }); keys.textContent = ''; const mappedCount = Object.keys(state.keymap).filter(code => ALL_KEYS.includes(code) && CONTROL_BY_ID[state.keymap[code]]).length; if (!mappedCount) { const item = document.createElement('div'); item.className = 'overlay-key'; const k = document.createElement('div'); k.className = 'k'; k.textContent = 'waiting'; const m = document.createElement('div'); m.className = 'm'; m.textContent = 'open control page'; item.append(k, m); keys.appendChild(item); return; }
  const pad = document.createElement('div'); pad.className = 'overlay-pad'; OVERLAY_LAYOUT.forEach(def => appendOverlayCell(pad, def)); keys.appendChild(pad); }
function renderAll(){ renderKeyboard(); renderEditor(); renderTelemetry(); renderOverlay(); }
function updateTelemetry(){ renderTelemetry(); renderOverlay(); }
function handleRemoteState(msg){ if (!msg || msg.v !== PROTOCOL_VERSION || (msg.type !== 'state_snapshot' && msg.type !== 'state_patch')) return; if (msg.owner === SESSION_ID) return; liveSnapshotReceived = true; if (!isOverlay) { renderOverlay(); return; } state = normalizeState(Object.assign({}, state, {keymap: msg.keymap || state.keymap, settings: msg.settings || state.settings, modes: msg.modes || state.modes, pressed: msg.pressed || state.pressed})); renderAll(); }
function connect(){ clearTimeout(reconnectTimer); websock = new WebSocket('ws://' + window.location.hostname + ':81/'); websock.onopen = () => { connected = true; updateTelemetry(); if (isOverlay) wsSend({v:PROTOCOL_VERSION, type:'state_request', ts:Date.now()}); else publishSnapshot('snapshot'); }; websock.onclose = () => { connected = false; reconnects++; updateTelemetry(); reconnectTimer = setTimeout(connect, 1000); }; websock.onerror = () => updateTelemetry(); websock.onmessage = evt => { try { const msg = JSON.parse(evt.data); if (msg.type === 'pong') { lastRtt = performance.now() - Number(msg.t || 0); updateTelemetry(); return; } handleRemoteState(msg); } catch(e) {} }; }
function bindEvents(){ document.addEventListener('keydown', event => { if (isOverlay) return; if (event.repeat) return; if (event.code === state.settings.leftToggleKey) { event.preventDefault(); toggleStick('left'); return; } if (event.code === state.settings.rightToggleKey) { event.preventDefault(); toggleStick('right'); return; } const control = controlForKey(event.code); if (!control) return; event.preventDefault(); startRepeating('key:' + event.code, control); }, false); document.addEventListener('keyup', event => { if (isOverlay) return; const control = controlForKey(event.code); if (!control) return; event.preventDefault(); stopRepeating('key:' + event.code, control); }, false);
  const assignBtn = document.getElementById('assignBtn'); if (assignBtn) assignBtn.onclick = () => { const v = document.getElementById('controlSelect').value; if (v) state.keymap[selectedCode] = v; publishSnapshot('keymap'); };
  const clearBtn = document.getElementById('clearBtn'); if (clearBtn) clearBtn.onclick = () => { delete state.keymap[selectedCode]; publishSnapshot('keymap'); };
  const resetBtn = document.getElementById('resetBtn'); if (resetBtn) resetBtn.onclick = () => { state = clone(DEFAULT_STATE); publishSnapshot('reset'); };
  const half = document.getElementById('halfPercent'); if (half) half.onchange = () => { state.settings.halfPercent = Math.max(5, Math.min(95, Number(half.value) || 50)); publishSnapshot('half'); };
  const left = document.getElementById('leftToggle'); if (left) left.onchange = () => { state.settings.leftToggleKey = left.value; publishSnapshot('toggle-key'); };
  const right = document.getElementById('rightToggle'); if (right) right.onchange = () => { state.settings.rightToggleKey = right.value; publishSnapshot('toggle-key'); };
  const tl = document.getElementById('toggleLeft'); if (tl) tl.onclick = () => toggleStick('left'); const tr = document.getElementById('toggleRight'); if (tr) tr.onclick = () => toggleStick('right');
  const exp = document.getElementById('exportBtn'); if (exp) exp.onclick = () => { document.getElementById('jsonBox').value = JSON.stringify({keymap:state.keymap, settings:state.settings}, null, 2); };
  const imp = document.getElementById('importBtn'); if (imp) imp.onclick = () => { try { const imported = JSON.parse(document.getElementById('jsonBox').value); state = normalizeState(Object.assign({}, state, imported)); publishSnapshot('import'); } catch(e) { alert('Invalid keymap JSON'); } };
}
setInterval(() => { if (connected) wsSend({v:PROTOCOL_VERSION, type:'ping', t:performance.now()}); updateTelemetry(); }, 1000);
bindEvents(); renderAll(); connect();
})();
</script>
</body>
</html>
)rawliteral";
