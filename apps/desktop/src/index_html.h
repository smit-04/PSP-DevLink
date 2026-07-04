#ifndef PSPDL_INDEX_HTML_H
#define PSPDL_INDEX_HTML_H

const char* const INDEX_HTML_CONTENT = R"html(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PSP DevLink Companion Dashboard</title>
    <style>
        :root {
            --bg-color: #0b0c10;
            --card-bg: rgba(25, 27, 38, 0.6);
            --border-color: rgba(0, 242, 254, 0.15);
            --text-main: #f0f0f5;
            --text-muted: #8b9bb4;
            --primary: #00f2fe;
            --secondary: #4facfe;
            --success: #00e676;
            --danger: #ff1744;
            --warning: #ffea00;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
        }

        body {
            background-color: var(--bg-color);
            color: var(--text-main);
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            background-image: radial-gradient(circle at 10% 20%, rgba(0, 242, 254, 0.05) 0%, transparent 40%),
                              radial-gradient(circle at 90% 80%, rgba(79, 172, 254, 0.05) 0%, transparent 40%);
        }

        header {
            padding: 20px 40px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            border-bottom: 1px solid var(--border-color);
            background: rgba(11, 12, 16, 0.8);
            backdrop-filter: blur(10px);
            position: sticky;
            top: 0;
            z-index: 100;
        }

        .logo-section h1 {
            font-size: 1.5rem;
            font-weight: 700;
            letter-spacing: 1px;
            background: linear-gradient(135deg, var(--primary) 0%, var(--secondary) 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .logo-section p {
            font-size: 0.8rem;
            color: var(--text-muted);
            margin-top: 2px;
        }

        .status-badge {
            padding: 6px 16px;
            border-radius: 20px;
            font-size: 0.85rem;
            font-weight: 600;
            text-transform: uppercase;
            display: flex;
            align-items: center;
            gap: 8px;
            border: 1px solid rgba(255, 255, 255, 0.1);
        }

        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            display: inline-block;
        }

        .status-disconnected {
            background: rgba(255, 23, 68, 0.1);
            color: var(--danger);
            border-color: rgba(255, 23, 68, 0.3);
        }
        .status-disconnected .status-dot { background-color: var(--danger); box-shadow: 0 0 8px var(--danger); }

        .status-handshaking {
            background: rgba(255, 234, 0, 0.1);
            color: var(--warning);
            border-color: rgba(255, 234, 0, 0.3);
        }
        .status-handshaking .status-dot { background-color: var(--warning); box-shadow: 0 0 8px var(--warning); }

        .status-connected {
            background: rgba(0, 230, 118, 0.1);
            color: var(--success);
            border-color: rgba(0, 230, 118, 0.3);
        }
        .status-connected .status-dot { background-color: var(--success); box-shadow: 0 0 8px var(--success); }

        main {
            padding: 40px;
            max-width: 1200px;
            margin: 0 auto;
            width: 100%;
            display: grid;
            grid-template-columns: 1fr 1.2fr;
            gap: 30px;
            flex-grow: 1;
        }

        @media (max-width: 900px) {
            main {
                grid-template-columns: 1fr;
            }
        }

        .panel-column {
            display: flex;
            flex-direction: column;
            gap: 30px;
        }

        .card {
            background: var(--card-bg);
            border-radius: 16px;
            border: 1px solid var(--border-color);
            padding: 24px;
            backdrop-filter: blur(16px);
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.37);
        }

        .card-title {
            font-size: 1.1rem;
            font-weight: 600;
            margin-bottom: 20px;
            display: flex;
            align-items: center;
            gap: 10px;
            border-left: 3px solid var(--primary);
            padding-left: 10px;
        }

        .metric-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 16px;
        }

        .metric-label {
            color: var(--text-muted);
            font-size: 0.9rem;
        }

        .metric-value {
            font-weight: 700;
            font-size: 1.1rem;
        }

        .progress-container {
            width: 100%;
            height: 8px;
            background: rgba(255, 255, 255, 0.05);
            border-radius: 4px;
            margin-top: 6px;
            overflow: hidden;
            border: 1px solid rgba(255, 255, 255, 0.05);
        }

        .progress-bar {
            height: 100%;
            width: 0%;
            background: linear-gradient(90deg, var(--primary) 0%, var(--secondary) 100%);
            border-radius: 4px;
            transition: width 0.3s ease;
        }

        .config-item {
            margin-bottom: 20px;
        }

        .config-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 8px;
        }

        .slider-control {
            width: 100%;
            height: 6px;
            -webkit-appearance: none;
            background: rgba(255, 255, 255, 0.08);
            border-radius: 3px;
            outline: none;
        }

        .slider-control::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 18px;
            height: 18px;
            border-radius: 50%;
            background: var(--primary);
            cursor: pointer;
            box-shadow: 0 0 8px var(--primary);
            transition: transform 0.1s;
        }

        .slider-control::-webkit-slider-thumb:hover {
            transform: scale(1.2);
        }

        /* Toggle Switches */
        .switch {
            position: relative;
            display: inline-block;
            width: 44px;
            height: 24px;
        }

        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: rgba(255, 255, 255, 0.1);
            transition: .3s;
            border-radius: 24px;
            border: 1px solid rgba(255, 255, 255, 0.05);
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 16px;
            width: 16px;
            left: 3px;
            bottom: 3px;
            background-color: white;
            transition: .3s;
            border-radius: 50%;
        }

        input:checked + .slider {
            background-color: var(--success);
            box-shadow: 0 0 8px rgba(0, 230, 118, 0.4);
        }

        input:checked + .slider:before {
            transform: translateX(20px);
        }

        /* Buttons styling */
        .btn-group {
            display: flex;
            gap: 15px;
            margin-top: 10px;
        }

        .btn {
            flex: 1;
            padding: 12px 20px;
            border: none;
            border-radius: 8px;
            font-weight: 600;
            font-size: 0.9rem;
            cursor: pointer;
            transition: all 0.2s;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 8px;
        }

        .btn-exit {
            background: rgba(255, 23, 68, 0.15);
            color: #ff5252;
            border: 1px solid rgba(255, 23, 68, 0.4);
        }

        .btn-exit:hover:not(:disabled) {
            background: var(--danger);
            color: #fff;
            box-shadow: 0 0 16px var(--danger);
        }

        .btn-reboot {
            background: rgba(255, 234, 0, 0.1);
            color: var(--warning);
            border: 1px solid rgba(255, 234, 0, 0.4);
        }

        .btn-reboot:hover:not(:disabled) {
            background: var(--warning);
            color: #000;
            box-shadow: 0 0 16px var(--warning);
        }

        .btn:disabled {
            opacity: 0.3;
            cursor: not-allowed;
        }

        /* Log Panel */
        .log-container {
            width: 100%;
            height: 180px;
            background: rgba(0, 0, 0, 0.2);
            border-radius: 8px;
            padding: 12px;
            font-family: monospace;
            font-size: 0.85rem;
            overflow-y: auto;
            border: 1px solid rgba(255, 255, 255, 0.05);
            color: #a5b4fc;
        }

        .log-entry {
            margin-bottom: 6px;
            line-height: 1.4;
        }

        .log-entry.info { color: #93c5fd; }
        .log-entry.success { color: #86efac; }
        .log-entry.warn { color: #fde047; }
        .log-entry.error { color: #fca5a5; }

        footer {
            text-align: center;
            padding: 20px;
            color: var(--text-muted);
            font-size: 0.8rem;
            border-top: 1px solid var(--border-color);
            margin-top: auto;
        }
    </style>
</head>
<body>
    <header>
        <div class="logo-section">
            <h1>PSP DEVLINK</h1>
            <p>Companion Web Interface</p>
        </div>
        <div id="status-badge" class="status-badge status-disconnected">
            <span class="status-dot"></span>
            <span id="status-text">Disconnected</span>
        </div>
    </header>

    <main>
        <div class="panel-column">
            <!-- Telemetry Card -->
            <div class="card">
                <h2 class="card-title">Live System Telemetry</h2>
                
                <div class="config-item">
                    <div class="metric-row">
                        <span class="metric-label">CPU Usage</span>
                        <span class="metric-value" id="cpu-val">--%</span>
                    </div>
                    <div class="progress-container">
                        <div class="progress-bar" id="cpu-bar"></div>
                    </div>
                </div>

                <div class="config-item">
                    <div class="metric-row">
                        <span class="metric-label">RAM Usage</span>
                        <span class="metric-value" id="ram-val">--%</span>
                    </div>
                    <div class="progress-container">
                        <div class="progress-bar" id="ram-bar"></div>
                    </div>
                </div>

                <div class="metric-row">
                    <span class="metric-label">CPU Temperature</span>
                    <span class="metric-value" id="temp-val">-- C</span>
                </div>

                <div class="metric-row">
                    <span class="metric-label">RAM Total / Free</span>
                    <span class="metric-value" id="mem-val">-- / -- GB</span>
                </div>
            </div>

            <!-- Git Workspace Card -->
            <div class="card">
                <h2 class="card-title">Git Workspace Status</h2>
                <div class="metric-row">
                    <span class="metric-label">Branch</span>
                    <span class="metric-value" id="git-branch" style="color: var(--primary);">--</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Modified Files</span>
                    <span class="metric-value" id="git-modified">--</span>
                </div>
                <div class="metric-row">
                    <span class="metric-label">Untracked Files</span>
                    <span class="metric-value" id="git-untracked">--</span>
                </div>
            </div>
        </div>

        <div class="panel-column">
            <!-- Configurations Settings Card -->
            <div class="card">
                <h2 class="card-title">Dashboard Settings</h2>
                
                <!-- Telemetry Slider -->
                <div class="config-item">
                    <div class="config-header">
                        <span class="metric-label">Telemetry Update Interval</span>
                        <span class="metric-value" id="telemetry-label">1000 ms</span>
                    </div>
                    <input type="range" id="telemetry-slider" class="slider-control" min="200" max="5000" step="100" value="1000">
                </div>

                <!-- Git Slider -->
                <div class="config-item">
                    <div class="config-header">
                        <span class="metric-label">Git Sync Interval</span>
                        <span class="metric-value" id="git-label">3000 ms</span>
                    </div>
                    <input type="range" id="git-slider" class="slider-control" min="1000" max="20000" step="500" value="3000">
                </div>

                <!-- Toggles -->
                <div class="metric-row" style="margin-top: 24px;">
                    <span class="metric-label">Enable Git Sync Telemetry</span>
                    <label class="switch">
                        <input type="checkbox" id="git-toggle" checked>
                        <span class="slider"></span>
                    </label>
                </div>

                <div class="metric-row" style="margin-top: 14px;">
                    <span class="metric-label">Enable Desktop Notifications</span>
                    <label class="switch">
                        <input type="checkbox" id="notif-toggle" checked>
                        <span class="slider"></span>
                    </label>
                </div>
            </div>

            <!-- Remote Commands Card -->
            <div class="card">
                <h2 class="card-title">Remote Session Commands</h2>
                <p class="metric-label" style="margin-bottom: 16px;">Trigger instant control signals to exit the game or reset the PSP console.</p>
                <div class="btn-group">
                    <button class="btn btn-exit" id="btn-exit" disabled>Exit to XMB</button>
                    <button class="btn btn-reboot" id="btn-reboot" disabled>Reboot Console</button>
                </div>
            </div>

            <!-- Activity Logs -->
            <div class="card" style="flex-grow: 1;">
                <h2 class="card-title">Event & Connection Logs</h2>
                <div class="log-container" id="log-box">
                    <div class="log-entry info">Serving dashboard interface locally. Waiting for PSP connection...</div>
                </div>
            </div>
        </div>
    </main>

    <footer>
        PSP DevLink Project &copy; 2026. All rights reserved.
    </footer>

    <script>
        // DOM Elements
        const statusBadge = document.getElementById('status-badge');
        const statusText = document.getElementById('status-text');
        
        const cpuVal = document.getElementById('cpu-val');
        const cpuBar = document.getElementById('cpu-bar');
        const ramVal = document.getElementById('ram-val');
        const ramBar = document.getElementById('ram-bar');
        const tempVal = document.getElementById('temp-val');
        const memVal = document.getElementById('mem-val');
        
        const gitBranch = document.getElementById('git-branch');
        const gitModified = document.getElementById('git-modified');
        const gitUntracked = document.getElementById('git-untracked');
        
        const telemetrySlider = document.getElementById('telemetry-slider');
        const telemetryLabel = document.getElementById('telemetry-label');
        const gitSlider = document.getElementById('git-slider');
        const gitLabel = document.getElementById('git-label');
        
        const gitToggle = document.getElementById('git-toggle');
        const notifToggle = document.getElementById('notif-toggle');
        
        const btnExit = document.getElementById('btn-exit');
        const btnReboot = document.getElementById('btn-reboot');
        const logBox = document.getElementById('log-box');

        let connectionState = 'DISCONNECTED';
        let lastLogsCount = 0;

        function addLog(text, type = 'info') {
            const entry = document.createElement('div');
            entry.className = `log-entry ${type}`;
            entry.innerText = `[${new Date().toLocaleTimeString()}] ${text}`;
            logBox.appendChild(entry);
            logBox.scrollTop = logBox.scrollHeight;
        }

        // Poll API
        async function fetchStatus() {
            try {
                const res = await fetch('/api/status');
                if (!res.ok) return;
                const data = await res.json();
                
                // Update connection state
                if (data.connection_state !== connectionState) {
                    connectionState = data.connection_state;
                    statusBadge.className = `status-badge status-${connectionState.toLowerCase()}`;
                    statusText.innerText = connectionState;
                    addLog(`Connection state changed to ${connectionState}`, connectionState === 'CONNECTED' ? 'success' : 'warn');
                    
                    const isConnected = connectionState === 'CONNECTED';
                    btnExit.disabled = !isConnected;
                    btnReboot.disabled = !isConnected;
                }

                // Update Telemetry
                if (data.telemetry) {
                    cpuVal.innerText = `${data.telemetry.cpu_usage}%`;
                    cpuBar.style.width = `${data.telemetry.cpu_usage}%`;
                    ramVal.innerText = `${data.telemetry.ram_usage}%`;
                    ramBar.style.width = `${data.telemetry.ram_usage}%`;
                    tempVal.innerText = `${(data.telemetry.cpu_temp / 10).toFixed(1)} C`;
                    memVal.innerText = `${(data.telemetry.ram_free / (1024*1024*1024)).toFixed(2)} / ${(data.telemetry.ram_total / (1024*1024*1024)).toFixed(2)} GB`;
                } else {
                    cpuVal.innerText = ramVal.innerText = '--%';
                    cpuBar.style.width = ramBar.style.width = '0%';
                    tempVal.innerText = '-- C';
                    memVal.innerText = '-- / -- GB';
                }

                // Update Git
                if (data.git) {
                    gitBranch.innerText = data.git.branch;
                    gitModified.innerText = `${data.git.modified} files`;
                    gitModified.style.color = data.git.modified > 0 ? 'var(--danger)' : 'var(--success)';
                    gitUntracked.innerText = `${data.git.untracked} files`;
                    gitUntracked.style.color = data.git.untracked > 0 ? 'var(--warning)' : 'var(--success)';
                } else {
                    gitBranch.innerText = '--';
                    gitModified.innerText = gitUntracked.innerText = '--';
                    gitModified.style.color = gitUntracked.style.color = 'var(--text-muted)';
                }

                // Update config widgets if not actively being adjusted by user
                if (document.activeElement !== telemetrySlider) {
                    telemetrySlider.value = data.config.telemetry_interval;
                    telemetryLabel.innerText = `${data.config.telemetry_interval} ms`;
                }
                if (document.activeElement !== gitSlider) {
                    gitSlider.value = data.config.git_interval;
                    gitLabel.innerText = `${data.config.git_interval} ms`;
                }
                if (document.activeElement !== gitToggle) {
                    gitToggle.checked = data.config.enable_git;
                }
                if (document.activeElement !== notifToggle) {
                    notifToggle.checked = data.config.enable_notif;
                }

                // Update background log messages
                if (data.logs && data.logs.length > lastLogsCount) {
                    for (let i = lastLogsCount; i < data.logs.length; i++) {
                        addLog(data.logs[i].message, data.logs[i].type);
                    }
                    lastLogsCount = data.logs.length;
                }
            } catch (e) {
                console.error("Failed to fetch status:", e);
            }
        }

        // Config Changes submitter
        async function submitConfig() {
            const body = new URLSearchParams();
            body.append('telemetry_interval', telemetrySlider.value);
            body.append('git_interval', gitSlider.value);
            body.append('enable_git', gitToggle.checked ? '1' : '0');
            body.append('enable_notif', notifToggle.checked ? '1' : '0');

            try {
                await fetch('/api/config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: body.toString()
                });
            } catch (e) {
                addLog("Failed to update config settings", "error");
            }
        }

        // Button commands
        async function sendControl(action) {
            try {
                addLog(`Sending remote ${action} command...`, "info");
                const res = await fetch(`/api/control?action=${action}`, { method: 'POST' });
                if (res.ok) {
                    addLog(`Remote ${action} command sent successfully`, "success");
                } else {
                    addLog(`Failed to execute remote ${action} command`, "error");
                }
            } catch (e) {
                addLog(`Network error sending ${action} command`, "error");
            }
        }

        // Listeners
        telemetrySlider.addEventListener('input', () => {
            telemetryLabel.innerText = `${telemetrySlider.value} ms`;
        });
        telemetrySlider.addEventListener('change', submitConfig);
        
        gitSlider.addEventListener('input', () => {
            gitLabel.innerText = `${gitSlider.value} ms`;
        });
        gitSlider.addEventListener('change', submitConfig);

        gitToggle.addEventListener('change', submitConfig);
        notifToggle.addEventListener('change', submitConfig);

        btnExit.addEventListener('click', () => sendControl('exit'));
        btnReboot.addEventListener('click', () => sendControl('reboot'));

        // Start Loop
        setInterval(fetchStatus, 500);
        fetchStatus();
    </script>
</body>
</html>)html";

#endif // PSPDL_INDEX_HTML_H
