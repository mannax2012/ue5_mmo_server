// Auto-generated header for embedding index.html as a C++ string
#pragma once

const char* EMBEDDED_INDEX_HTML = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>MMO Server Dashboard</title>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.2/css/all.min.css">
  <style>
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: #23272b;
      color: #eaeaea;
      margin: 0;
      display: flex;
      min-height: 100vh;
    }
    .sidebar {
      width: 240px;
      background: #181a1b;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 2em 0 1em 0;
      box-shadow: 2px 0 8px #0006;
      z-index: 2;
    }
    .sidebar h1 {
      color: #6ec1e4;
      font-size: 1.5em;
      margin-bottom: 2em;
      letter-spacing: 1px;
    }
    .nav {
      width: 100%;
      display: flex;
      flex-direction: column;
      gap: 1em;
    }
    .nav button {
      background: none;
      border: none;
      color: #eaeaea;
      font-size: 1.1em;
      padding: 1em 2em;
      text-align: left;
      width: 100%;
      cursor: pointer;
      border-left: 4px solid transparent;
      transition: background 0.2s, border-color 0.2s;
      display: flex;
      align-items: center;
      gap: 1em;
    }
    .nav button.active, .nav button:hover {
      background: #23272b;
      border-left: 4px solid #6ec1e4;
      color: #6ec1e4;
    }
    .main-content {
      flex: 1;
      padding: 2em 3em;
      background: #23272b;
      min-width: 0;
      display: flex;
      flex-direction: column;
      gap: 2em;
    }
    .dashboard-grid {
      display: flex;
      flex-wrap: wrap;
      gap: 2em;
      margin-bottom: 2em;
    }
    .dashboard-card {
      background: #292d32;
      border-radius: 10px;
      box-shadow: 0 2px 8px #0008;
      padding: 1.5em 2em;
      min-width: 220px;
      flex: 1 1 220px;
      display: flex;
      flex-direction: column;
      align-items: flex-start;
      position: relative;
    }
    .dashboard-card h3 {
      margin: 0 0 0.5em 0;
      color: #6ec1e4;
      font-size: 1.2em;
      font-weight: 500;
      display: flex;
      align-items: center;
      gap: 0.5em;
    }
    .dashboard-card .value {
      font-size: 1.5em;
      color: #eaeaea;
      margin-bottom: 0.2em;
      font-weight: 600;
    }
    .dashboard-card .label {
      color: #aaa;
      font-size: 0.95em;
    }
    .section-panel {
      background: #292d32;
      border-radius: 10px;
      box-shadow: 0 2px 8px #0008;
      padding: 1.5em 2em;
      margin-bottom: 1em;
    }
    h2 {
      color: #6ec1e4;
      font-size: 1.3em;
      margin-top: 0;
      margin-bottom: 1em;
      font-weight: 500;
      letter-spacing: 0.5px;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 1em;
    }
    th, td {
      border: 1px solid #444;
      padding: 0.5em 1em;
      text-align: left;
    }
    th {
      background: #23272b;
      color: #6ec1e4;
    }
    td {
      background: #23272b;
    }
    button.refresh {
      background: #6ec1e4;
      color: #23272b;
      border: none;
      border-radius: 4px;
      padding: 0.4em 1em;
      font-size: 1em;
      cursor: pointer;
      margin-bottom: 1em;
      float: right;
    }
    button.refresh:hover {
      background: #4fa3c7;
    }
    pre {
      background: #181a1b;
      color: #eaeaea;
      padding: 1em;
      border-radius: 4px;
      overflow-x: auto;
    }
    .log-level-btn {
      background: #23272b;
      color: #eaeaea;
      border: 1px solid #444;
      border-radius: 4px;
      padding: 0.4em 0.9em;
      font-size: 1em;
      cursor: pointer;
      transition: background 0.2s, color 0.2s, border-color 0.2s;
    }
    .log-level-btn.selected {
      background: #6ec1e4;
      color: #23272b;
      border-color: #6ec1e4;
      font-weight: bold;
    }
    @media (max-width: 900px) {
      .main-content { padding: 1em; }
      .dashboard-grid { gap: 1em; }
      .dashboard-card { min-width: 140px; padding: 1em; }
      .section-panel { padding: 1em; }
    }
  </style>
</head>
<body>
  <div class="sidebar">
    <h1><i class="fa-solid fa-server"></i> MMO Server</h1>
    <nav class="nav">
      <button id="nav-dashboard" class="active" onclick="showSection('dashboard')"><i class="fa-solid fa-gauge-high"></i> Dashboard</button>
      <button id="nav-sessions" onclick="showSection('sessions')"><i class="fa-solid fa-users"></i> Sessions</button>
      <button id="nav-shards" onclick="showSection('shards')"><i class="fa-solid fa-layer-group"></i> Shards</button>
      <button id="nav-entities" onclick="showSection('entities')"><i class="fa-solid fa-cube"></i> Entities</button>
      <button id="nav-logs" onclick="showSection('logs')"><i class="fa-solid fa-scroll"></i> Logs</button>
    </nav>
  </div>
  <div class="main-content">
    <div id="section-dashboard" class="section-panel">
      <h2><i class="fa-solid fa-gauge-high"></i> Server Dashboard <button class="refresh" onclick="refreshDashboard()"><i class="fa-solid fa-rotate"></i> Refresh</button></h2>
      <div id="dashboard" class="dashboard-grid"></div>
    </div>
    <div id="section-sessions" class="section-panel" style="display:none;">
      <h2><i class="fa-solid fa-users"></i> Sessions <button class="refresh" onclick="refreshSessions()"><i class="fa-solid fa-rotate"></i> Refresh</button></h2>
      <div id="sessions"></div>
    </div>
    <div id="section-shards" class="section-panel" style="display:none;">
      <h2><i class="fa-solid fa-layer-group"></i> Shards <button class="refresh" onclick="refreshShards()"><i class="fa-solid fa-rotate"></i> Refresh</button></h2>
      <div id="shards"></div>
    </div>
    <div id="section-entities" class="section-panel" style="display:none;">
      <h2><i class="fa-solid fa-cube"></i> Entities <button class="refresh" onclick="refreshEntities()"><i class="fa-solid fa-rotate"></i> Refresh</button></h2>
      <div id="entities"></div>
    </div>
    <div id="section-logs" class="section-panel" style="display:none;">
      <h2><i class="fa-solid fa-scroll"></i> Logs <button class="refresh" onclick="refreshLogs()"><i class="fa-solid fa-rotate"></i> Refresh</button></h2>
      <div style="margin-bottom:1em;display:flex;gap:0.5em;align-items:center;">
        <input id="logFilterInput" type="text" placeholder="Filter logs..." style="flex:1;padding:0.4em 0.8em;border-radius:4px;border:1px solid #444;background:#181a1b;color:#eaeaea;" onkeydown="applyLogFilter();">
        <div id="logLevelButtons" style="display:flex;gap:0.3em;">
          <button type="button" class="log-level-btn" data-level="DEBUG" onclick="toggleLogLevel('DEBUG')">DEBUG</button>
          <button type="button" class="log-level-btn" data-level="INFO" onclick="toggleLogLevel('INFO')">INFO</button>
          <button type="button" class="log-level-btn" data-level="WARN" onclick="toggleLogLevel('WARN')">WARN</button>
          <button type="button" class="log-level-btn" data-level="ERROR" onclick="toggleLogLevel('ERROR')">ERROR</button>
        </div>
         <button onclick="clearLogFilter()" style="background:#444;color:#eaeaea;border:none;border-radius:4px;padding:0.4em 1em;font-size:1em;cursor:pointer;"><i class="fa-solid fa-xmark"></i></button>
      </div>
      <div id="logs"></div>
    </div>
  </div>
  <script>
function showSection(section) {
  const sections = ['dashboard','sessions','shards','entities','logs'];
  for (const s of sections) {
    const sec = document.getElementById('section-' + s);
    const nav = document.getElementById('nav-' + s);
    if (sec) sec.style.display = (s === section) ? '' : 'none';
    if (nav) nav.classList.toggle('active', s === section);
  }
  // Hide shards section if not game server
  if (window.serverType !== 'game') {
    const shardsPanel = document.getElementById('section-shards');
    const shardsNav = document.getElementById('nav-shards');
    if (shardsPanel) shardsPanel.style.display = 'none';
    if (shardsNav) shardsNav.style.display = 'none';
  }
}

async function refreshDashboard() {
  const [dashboardRes, sessionsRes, shardsRes, entitiesRes] = await Promise.all([
    fetch('/dashboard'),
    fetch('/sessions'),
    fetch('/shards'),
    fetch('/entities')
  ]);
  const d = await dashboardRes.json();
  window.serverType = d.serverType;
  const sessionsData = await sessionsRes.json();
  const shardsData = await shardsRes.json();
  const entitiesData = await entitiesRes.json();
  const numSessions = Array.isArray(sessionsData && sessionsData.sessions) ? sessionsData.sessions.length : 0;
  const numShards = Array.isArray(shardsData && shardsData.shards) ? shardsData.shards.length : 0;
  const numEntities = Array.isArray(entitiesData && entitiesData.entities) ? entitiesData.entities.length : 0;
  const icons = [
    'fa-server', 'fa-network-wired', 'fa-plug', 'fa-microchip', 'fa-clock', 'fa-microchip', 'fa-microchip', 'fa-memory', 'fa-memory', 'fa-users', 'fa-layer-group', 'fa-cube'
  ];
  const cards = [
    {label: 'Server Type', value: d.serverType},
    {label: 'IP Address', value: d.ip},
    {label: 'Port', value: d.port},
    {label: 'OS', value: d.os},
    {label: 'Uptime', value: formatUptime(d.uptime_sec)},
    {label: 'CPU Model', value: d.cpu_model || 'N/A'},
    {label: 'CPU Cores', value: d.cpu_cores || 'N/A'},
    {label: 'RAM Total', value: d.ram_total_kb ? (d.ram_total_kb/1024).toFixed(1)+' MB' : 'N/A'},
    {label: 'RAM Available', value: d.ram_available_kb ? (d.ram_available_kb/1024).toFixed(1)+' MB' : 'N/A'},
    {label: 'Sessions', value: numSessions},
    // Only show shards card for game server
    ...(d.serverType === 'game' ? [{label: 'Shards', value: numShards}] : []),
    {label: 'Entities', value: numEntities},
  ];
  document.getElementById('dashboard').innerHTML = cards.map(function(card, i) {
    return '<div class="dashboard-card">' +
      '<h3><i class="fa-solid ' + icons[i] + '"></i> ' + card.label + '</h3>' +
      '<div class="value">' + card.value + '</div>' +
    '</div>';
  }).join('');
}

function formatUptime(sec) {
  if (!sec) return 'N/A';
  const d = Math.floor(sec/86400), h = Math.floor((sec%86400)/3600), m = Math.floor((sec%3600)/60), s = sec%60;
  let out = '';
  if (d) out += d+'d ';
  if (h) out += h+'h ';
  if (m) out += m+'m ';
  out += s+'s';
  return out;
}

let logsRawData = [];
let logsColumns = ['timestamp','level','message'];

async function fetchAndTable(endpoint, containerId, columns, filterFn) {
  return new Promise(async (resolve) => {
    const res = await fetch(endpoint);
    const data = await res.json();
    let arr = [];
    if (data && typeof data === 'object') {
      const first = Object.values(data)[0];
      if (Array.isArray(first)) arr = first;
    }
    if (containerId === 'logs') {
      logsRawData = arr;
      logsColumns = columns;
    }
    if (typeof filterFn === 'function') {
      arr = arr.filter(filterFn);
    }
    let html = '';
    if (arr.length === 0) {
      html = '<em>No data</em>';
    } else {
      html = '<table><thead><tr>' + columns.map(c => `<th>${c}</th>`).join('') + '</tr></thead><tbody>';
      for (const row of arr) {
        html += '<tr>' + columns.map(c => `<td>${row[c] !== undefined ? row[c] : ''}</td>`).join('') + '</tr>';
      }
      html += '</tbody></table>';
    }
    const el = document.getElementById(containerId);
    if (el) el.innerHTML = html;
    resolve();
  });
}

function refreshSessions() {
  fetchAndTable('/sessions', 'sessions', ['endpoint','username','playerId','charId','connected','lastHeartbeat','playerName','sessionKey']);
}
function refreshShards() {
  fetchAndTable('/shards', 'shards', ['zoneId','entityCount','playerCount']);
}
function refreshEntities() {
  fetchAndTable('/entities', 'entities', ['id','type','zoneId','x','y','z']);
}
function refreshLogs() {
  fetchAndTable('/logs', 'logs', ['timestamp','level','message']).then(() => {
    applyLogFilter();
  });
}

let selectedLogLevels = [];
function toggleLogLevel(level) {
  const idx = selectedLogLevels.indexOf(level);
  if (idx === -1) {
    selectedLogLevels.push(level);
  } else {
    selectedLogLevels.splice(idx, 1);
  }
  updateLogLevelButtonUI();
  applyLogFilter();
}
function updateLogLevelButtonUI() {
  document.querySelectorAll('.log-level-btn').forEach(btn => {
    btn.classList.toggle('selected', selectedLogLevels.includes(btn.dataset.level));
  });
}
function applyLogFilter() {
  const filter = document.getElementById('logFilterInput').value.trim().toLowerCase();
  if (!logsRawData || !logsRawData.length) {
    renderLogsTable([]);
    return;
  }
  let filtered = logsRawData;
  if (filter) {
    filtered = filtered.filter(row =>
      logsColumns.some(col => (row[col] && String(row[col]).toLowerCase().includes(filter)))
    );
  }
  if (selectedLogLevels.length > 0) {
    filtered = filtered.filter(row => selectedLogLevels.includes(row.level));
  }
  renderLogsTable(filtered);
}
function clearLogFilter() {
  document.getElementById('logFilterInput').value = '';
  selectedLogLevels = [];
  updateLogLevelButtonUI();
  renderLogsTable(logsRawData);
}
function renderLogsTable(arr) {
  let html = '';
  if (!arr || arr.length === 0) {
    html = '<em>No data</em>';
  } else {
    html = '<table><thead><tr>' + logsColumns.map(c => `<th>${c}</th>`).join('') + '</tr></thead><tbody>';
    for (const row of arr) {
      html += '<tr>' + logsColumns.map(c => `<td>${row[c] !== undefined ? row[c] : ''}</td>`).join('') + '</tr>';
    }
    html += '</tbody></table>';
  }
  const el = document.getElementById('logs');
  if (el) el.innerHTML = html;
}

// Initial load after DOM is ready
window.onload = function() {
  refreshDashboard();
  refreshSessions();
  refreshShards();
  refreshEntities();
  refreshLogs();
  updateLogLevelButtonUI();
};
</script>
</body>
</html>

)HTML";
