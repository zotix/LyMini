#pragma once
#include <Arduino.h>

const char PAGE_INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<meta name="apple-mobile-web-app-title" content="LyMini">
<meta name="mobile-web-app-capable" content="yes">
<meta name="theme-color" content="#0b0f17">
<link rel="manifest" href="/manifest.webmanifest">
<link rel="apple-touch-icon" href="/icon-192.png">
<link rel="icon" type="image/svg+xml" href="/icon.svg">
<title>LyMini Arcade & Smart TV</title>
<style>
  :root {
    --bg: #0b0f17;
    --card: #151c28;
    --border: #2b3648;
    --text: #e2e8f0;
    --muted: #94a3b8;
    --accent: #38bdf8;
    --green: #10b981;
    --red: #ef4444;
    --purple: #a855f7;
    --gold: #facc15;
  }
  * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
  body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: max(10px, env(safe-area-inset-top)) 10px max(18px, env(safe-area-inset-bottom)) 10px; }
  .container { max-width: 720px; margin: 0 auto; }
  h1 { font-size: 18px; margin: 0 0 4px 0; color: #fff; }
  h2 { font-size: 15px; margin: 0 0 10px 0; color: var(--accent); }
  .topbar { display: flex; flex-wrap: wrap; gap: 8px; align-items: center; justify-content: space-between; margin-bottom: 10px; padding: 10px 12px; background: var(--card); border: 1px solid var(--border); border-radius: 10px; }
  .badge { display: inline-block; padding: 3px 8px; border-radius: 12px; font-size: 11px; font-weight: 600; background: #1e293b; color: var(--accent); }
  .badge.online { background: rgba(16, 185, 129, 0.2); color: #34d399; border: 1px solid #10b981; }
  .badge.offline { background: rgba(239, 68, 68, 0.2); color: #f87171; border: 1px solid #ef4444; }
  .lang-btn { background: #1e293b; border: 1px solid var(--accent); color: #fff; padding: 5px 10px; border-radius: 8px; font-size: 12px; font-weight: 700; cursor: pointer; width: auto; margin: 0; }
  .tabs { display: flex; gap: 4px; overflow-x: auto; margin-bottom: 12px; padding-bottom: 4px; }
  .tab-btn { flex: 1; white-space: nowrap; background: #111827; color: var(--muted); border: 1px solid var(--border); padding: 9px 10px; border-radius: 8px; font-size: 12px; font-weight: 700; cursor: pointer; margin: 0; }
  .tab-btn.active { background: var(--accent); color: #090d14; border-color: var(--accent); }
  .tab-pane { display: none; }
  .tab-pane.active { display: block; }
  .card { background: var(--card); border: 1px solid var(--border); border-radius: 10px; padding: 14px; margin-bottom: 14px; }
  label { display: block; margin-top: 8px; font-weight: 600; font-size: 13px; color: var(--muted); }
  input[type=text], input[type=password], input[type=number], select {
    width: 100%; background: #090d14; border: 1px solid var(--border); color: #fff; padding: 9px 10px; border-radius: 6px; margin-top: 4px; font-size: 14px;
  }
  input[type=range] { width: 100%; accent-color: var(--accent); height: 24px; cursor: pointer; }
  button {
    background: var(--accent); color: #090d14; border: none; padding: 10px 14px; border-radius: 6px;
    font-weight: 700; cursor: pointer; margin-top: 8px; width: 100%; font-size: 14px;
  }
  button.green { background: var(--green); color: #fff; }
  button.purple { background: var(--purple); color: #fff; }
  button.red { background: var(--red); color: #fff; }
  button.gold { background: var(--gold); color: #090d14; }
  button.sm { width: auto; padding: 5px 9px; margin: 0; font-size: 11px; }
  .row { display: flex; gap: 8px; align-items: center; margin-top: 8px; }
  .row > * { flex: 1; }
  .cropper-wrap { display: flex; flex-direction: column; align-items: center; background: #080b11; border: 1px dashed var(--border); border-radius: 8px; padding: 12px; margin-top: 10px; position: relative; }
  canvas.preview-cv { border: 2px solid var(--accent); border-radius: 6px; width: 256px; height: 256px; image-rendering: pixelated; background: #000; touch-action: none; cursor: grab; }
  .chk-grid { display: grid; grid-template-columns: 1fr; gap: 6px; margin-top: 8px; }
  .chk-item { display: flex; align-items: center; justify-content: space-between; gap: 8px; background: #0b0f17; padding: 8px 10px; border-radius: 6px; border: 1px solid var(--border); font-size: 13px; color: #fff; }
  .chk-left { display: flex; align-items: center; gap: 8px; flex: 1; }
  .chk-left input { width: 18px; height: 18px; margin: 0; }
  .media-list { margin-top: 10px; display: flex; flex-direction: column; gap: 6px; }
  .media-row { display: flex; align-items: center; justify-content: space-between; gap: 8px; background: #0b0f17; padding: 8px 10px; border-radius: 6px; border: 1px solid var(--border); font-size: 13px; }
  .media-name { flex: 1; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; color: #fff; }
  .wifi-item { display: flex; justify-content: space-between; align-items: center; gap: 6px; background: #0b0f17; padding: 7px 10px; border-radius: 6px; margin-top: 6px; border: 1px solid var(--border); font-size: 13px; }
  .hint { font-size: 12px; color: var(--muted); margin: 4px 0 0 0; }
  #srcVideo { position: fixed; left: -9999px; top: 0; width: 64px; height: 64px; opacity: 0.01; pointer-events: none; z-index: -1; }
  .modal-overlay {
    display: none; position: fixed; inset: 0; background: rgba(7, 10, 16, 0.85);
    backdrop-filter: blur(4px); z-index: 9999; align-items: center; justify-content: center; padding: 20px;
  }
  .modal-overlay.show { display: flex; }
  .modal-card {
    background: var(--card); border: 2px solid var(--accent); border-radius: 14px; padding: 22px 20px;
    width: 100%; max-width: 340px; text-align: center; box-shadow: 0 12px 32px rgba(0,0,0,0.6);
  }
  .spinner {
    width: 42px; height: 42px; border: 4px solid #1e293b; border-top-color: var(--accent);
    border-radius: 50%; animation: spin 0.8s linear infinite; margin: 0 auto 12px auto;
  }
  @keyframes spin { to { transform: rotate(360deg); } }
  .prog-bg { width: 100%; height: 12px; background: #090d14; border-radius: 8px; overflow: hidden; border: 1px solid var(--border); margin-top: 12px; }
  .prog-fill { height: 100%; width: 0%; background: linear-gradient(90deg, var(--accent), var(--green)); transition: width 0.15s ease; }
  .toast {
    position: fixed; left: 50%; bottom: calc(82px + env(safe-area-inset-bottom)); transform: translate(-50%, 16px);
    background: #0f172a; color: #fff; border: 1px solid var(--accent); border-radius: 999px;
    padding: 8px 16px; font-size: 13px; font-weight: 700; box-shadow: 0 8px 24px rgba(0,0,0,0.55);
    opacity: 0; pointer-events: none; transition: opacity 0.2s, transform 0.2s; z-index: 950; white-space: nowrap;
  }
  .toast.show { opacity: 1; transform: translate(-50%, 0); }
  @media (max-width: 640px) {
    body { padding-bottom: calc(82px + env(safe-area-inset-bottom)); }
    .topbar { padding: 10px; gap: 6px; }
    .topbar h1 { font-size: 16px; }
    .tabs {
      position: fixed; left: 0; right: 0; bottom: 0; z-index: 900; margin: 0;
      padding: 6px 6px max(8px, env(safe-area-inset-bottom)) 6px;
      background: rgba(11, 15, 23, 0.96); backdrop-filter: blur(10px);
      border-top: 1px solid var(--border); gap: 4px; overflow: hidden;
    }
    .tab-btn {
      min-height: 48px; padding: 6px 4px; font-size: 11px; line-height: 1.25;
      display: flex; flex-direction: column; align-items: center; justify-content: center;
      white-space: normal; text-align: center; border-radius: 10px;
    }
    input[type=text], input[type=password], input[type=number], select { font-size: 16px; min-height: 44px; }
    button { min-height: 44px; }
    button.sm { min-height: 34px; padding: 6px 10px; }
    input[type=range] { height: 34px; }
    canvas.preview-cv { width: 100%; max-width: 256px; height: auto; aspect-ratio: 1 / 1; }
    .toast { bottom: calc(74px + env(safe-area-inset-bottom)); }
  }
</style>
</head>
<body>
<!-- Processing & Upload Modal Popup -->
<div id="busyModal" class="modal-overlay">
  <div class="modal-card">
    <div class="spinner" id="busySpinner"></div>
    <div id="busyTitle" style="font-size:16px; font-weight:700; color:#fff; margin-bottom:6px;">⏳ กำลังดำเนินการ...</div>
    <div id="busyStep" class="hint" style="color:var(--accent); font-size:13px; font-weight:600;">กรุณารอสักครู่ ห้ามปิดหน้านี้</div>
    <div class="prog-bg"><div id="busyBar" class="prog-fill"></div></div>
    <div id="busyPct" style="font-size:13px; font-weight:700; color:var(--gold); margin-top:8px;">0%</div>
  </div>
</div>
<div id="autoSaveToast" class="toast">✅ บันทึกอัตโนมัติแล้ว</div>
<div class="container">
  <div class="topbar">
    <div>
      <h1>🕹️ LyMini <span id="topFwVer" class="badge">v0.3.2</span> <span id="connBadge" class="badge online">🟢 Online</span></h1>
      <div class="hint" id="statusText">IP: <b>192.168.4.1</b> | <b>http://lymini.local</b></div>
    </div>
    <div style="display:flex; gap:6px; align-items:center;">
      <div id="battBadge" class="badge">🔋 --%</div>
      <button type="button" id="btnLang" class="lang-btn">🇹🇭 TH / 🇬🇧 EN</button>
    </div>
  </div>

  <!-- Navigation Tabs -->
  <div class="tabs">
    <button type="button" class="tab-btn active" onclick="switchTab(0)" data-i18n="tab_studio">1. ✂️ สตูดิโอ</button>
    <button type="button" class="tab-btn" onclick="switchTab(1)" data-i18n="tab_idle">2. 🎲 พักหน้าจอ & คลัง</button>
    <button type="button" class="tab-btn" onclick="switchTab(2)" data-i18n="tab_display">3. 🔆 ความสว่าง & จอ</button>
    <button type="button" class="tab-btn" onclick="switchTab(3)" data-i18n="tab_wifi">4. 📶 Wi-Fi & อากาศ</button>
    <button type="button" class="tab-btn" onclick="switchTab(4)" data-i18n="tab_about">5. 🚀 OTA & เกี่ยวกับ</button>
  </div>

  <video id="srcVideo" playsinline webkit-playsinline muted loop preload="auto"></video>

  <!-- TAB 0: 128x128 Media Studio -->
  <div class="tab-pane active" id="tab0">
    <div class="card">
      <h2 data-i18n="st_title">✂️ 128×128 Media Studio (iOS / Android / PC + Live View)</h2>
      <p class="hint" data-i18n="st_desc">รองรับรูปภาพ (.JPG, .PNG, .WEBP, .HEIC) และวิดีโอจาก iPhone/มือถือ (.MOV, .MP4) ลากนิ้วเพื่อเลื่อนกรอบ หรือถ่าง 2 นิ้วเพื่อซูม</p>
      <input type="file" id="mediaInput" accept="image/*,video/*,.mov,.mp4,.m4v,.heic" style="margin-top:8px; width:100%;" />

      <div class="cropper-wrap">
        <canvas id="cropCanvas" class="preview-cv" width="128" height="128"></canvas>
        <div class="hint" data-i18n="st_pan_hint">💡 ลากบนกรอบภาพเพื่อจัดตำแหน่ง (Pan) / ถ่าง 2 นิ้วเพื่อซูม</div>
        <div class="row" style="width:100%;">
          <button type="button" id="btnZoomIn">Zoom +</button>
          <button type="button" id="btnZoomOut">Zoom -</button>
          <button type="button" id="btnRotate" data-i18n="st_rot">หมุน 90°</button>
          <button type="button" id="btnResetCrop" data-i18n="st_reset">รีเซ็ต</button>
        </div>
        <div class="chk-item" style="width:100%; margin-top:8px;">
          <div class="chk-left">
            <input type="checkbox" id="chkAutoLive" checked />
            <span data-i18n="st_autolive">👁️ Live View บนจอ LyMini อัตโนมัติ ขณะลาก/ปรับแต่ง</span>
          </div>
        </div>
        <div class="row" style="width:100%; margin-top:8px; gap:8px;">
          <label class="chk-item" style="flex:1; margin:0; padding:6px 8px; font-size:12px;">
            <input type="checkbox" id="chkSharpen" checked />
            <span data-i18n="st_sharpen">✨ เร่งความคมชัดภาพ</span>
          </label>
          <label class="chk-item" style="flex:1; margin:0; padding:6px 8px; font-size:12px;">
            <input type="checkbox" id="chkVibrant" checked />
            <span data-i18n="st_vibrant">🎨 สีสดใสพิเศษ (LCD)</span>
          </label>
        </div>
        <div class="chk-item" style="width:100%; margin-top:8px;">
          <div class="chk-left">
            <span style="font-size:12px; font-weight:600; color:var(--text-muted);" data-i18n="st_quality_lbl">คุณภาพวิดีโอ:</span>
          </div>
          <select id="selVidQuality" style="padding:4px 8px; background:var(--surface); color:#fff; border:1px solid var(--border); border-radius:6px; font-size:12px;">
            <option value="0" selected data-i18n="st_q_hd">🌟 HD 16-bit (65,536 สี คมชัดสูงสุด)</option>
            <option value="1" data-i18n="st_q_compact">⚡ Compact 8-bit (ประหยัดพื้นที่)</option>
          </select>
        </div>
        <div class="row" style="width:100%;">
          <button type="button" class="purple" id="btnLiveNow" data-i18n="st_btn_live">👁️ ส่งขึ้นจอทันที</button>
          <button type="button" class="green" id="btnUploadMedia" data-i18n="st_btn_save">🚀 บันทึกลงเครื่อง & เล่นทันที</button>
        </div>
        <div id="uploadMsg" class="hint" style="margin-top:8px; color:var(--gold); font-weight:600;"></div>
      </div>
    </div>
  </div>

  <!-- TAB 1: Idle Mode & Media Gallery -->
  <div class="tab-pane" id="tab1">
    <div class="card">
      <h2 data-i18n="id_title">🎲 ตั้งค่าโหมดพักหน้าจอ (Smart Idle & Live Preview)</h2>
      <p class="hint" data-i18n="id_smart_hint">✨ กดปุ่ม 👁️ ท้ายรายการใดก็ได้ เพื่อดูตัวอย่างหน้านั้นบนจอจริงของ LyMini ทันที!</p>
      <label data-i18n="id_select">เลือกหน้าจอที่จะแสดงเมื่อพักหน้าจอ (กด 👁️ เพื่อดูบนจอจริง):</label>
      <div class="chk-grid">
        <div class="chk-item">
          <label class="chk-left"><input type="checkbox" id="chkClock" checked /> <span data-i18n="id_clk_dig">🕒 นาฬิกาดิจิทัล (Digital Clock)</span></label>
          <button type="button" class="sm purple" onclick="previewIdle(0)">👁️ ดูบนจอ</button>
        </div>
        <div class="chk-item">
          <label class="chk-left"><input type="checkbox" id="chkAnalog" checked /> <span data-i18n="id_clk_ana">🕰️ นาฬิกาเข็ม (Analog Clock)</span></label>
          <button type="button" class="sm purple" onclick="previewIdle(4)">👁️ ดูบนจอ</button>
        </div>
        <div class="chk-item">
          <label class="chk-left"><input type="checkbox" id="chkFlip" checked /> <span data-i18n="id_clk_flp">🔢 นาฬิกาพลิก (Flip Clock)</span></label>
          <button type="button" class="sm purple" onclick="previewIdle(5)">👁️ ดูบนจอ</button>
        </div>
        <div class="chk-item">
          <label class="chk-left"><input type="checkbox" id="chkWeather" checked /> <span data-i18n="id_weather">🌡️ สภาพอากาศล้วน (Weather)</span></label>
          <button type="button" class="sm purple" onclick="previewIdle(1)">👁️ ดูบนจอ</button>
        </div>
        <div class="chk-item">
          <label class="chk-left"><input type="checkbox" id="chkClkWeather" checked /> <span data-i18n="id_clk_wt">🕒+🌡️ นาฬิกา + สภาพอากาศ (Clock + Weather)</span></label>
          <button type="button" class="sm purple" onclick="previewIdle(7)">👁️ ดูบนจอ</button>
        </div>
        <div class="chk-item">
          <label class="chk-left"><input type="checkbox" id="chkPhoto" checked /> <span data-i18n="id_photo">🖼️ กรอบรูปภาพ (Photos)</span></label>
          <button type="button" class="sm purple" onclick="previewIdle(2)">👁️ ดูบนจอ</button>
        </div>
        <div class="chk-item">
          <label class="chk-left"><input type="checkbox" id="chkVideo" checked /> <span data-i18n="id_video">🎬 วิดีโอแอนิเมชัน (Videos)</span></label>
          <button type="button" class="sm purple" onclick="previewIdle(3)">👁️ ดูบนจอ</button>
        </div>
        <!-- Mandatory PromptPay QR option: ALWAYS checked & disabled so it cannot be unchecked -->
        <div class="chk-item" style="border-color:var(--gold);">
          <label class="chk-left">
            <input type="checkbox" id="chkAboutQr" checked disabled />
            <span style="color:var(--gold);" data-i18n="id_qr">☕ Buy Me a Coffee (PromptPay QR — เปิดแสดงเสมอ)</span>
          </label>
          <button type="button" class="sm gold" onclick="previewIdle(6)">👁️ ดูบนจอ</button>
        </div>
      </div>

      <div class="chk-item" style="margin-top:8px;">
        <label class="chk-left">
          <input type="checkbox" id="chkRandom" checked />
          <span data-i18n="id_random">🔀 Random Mode (สุ่มลำดับหน้าจอ และสุ่มรูปภาพ/วิดีโอ)</span>
        </label>
      </div>
      <div class="chk-item" style="margin-top:6px;">
        <label class="chk-left">
          <input type="checkbox" id="chkEcoVideo" />
          <span data-i18n="id_ecovid">🎬 เล่นวิดีโอเมื่ออยู่ใน Eco Mode (ปิด = ประหยัดแบตมากขึ้น)</span>
        </label>
      </div>

      <div class="row">
        <div>
          <label data-i18n="id_timeout">เข้า Idle (40MHz / แสง 50%) เมื่อไม่กดปุ่ม (วินาที):</label>
          <input type="number" id="inpIdleTime" min="5" max="600" value="30" />
        </div>
        <div>
          <label data-i18n="eco_timeout">เข้า Eco (20MHz / แสง 25%) เมื่อไม่กดปุ่ม (วินาที):</label>
          <input type="number" id="inpEcoTime" min="30" max="3600" value="300" />
        </div>
      </div>
      <div class="row" style="margin-top:10px;">
        <div>
          <label data-i18n="id_cycle">สลับหน้าทุกๆ (วินาที):</label>
          <input type="number" id="inpIdleCycle" min="5" max="120" value="12" />
        </div>
        <div>
          <label data-i18n="sleep_timeout">เข้า Sleep Mode ปิดจอ/พักเครื่อง (นาที, 0=ปิด):</label>
          <input type="number" id="inpSleepTime" min="0" max="720" step="any" value="45" />
        </div>
      </div>
      <p class="hint" data-i18n="auto_save_hint">⚡ บันทึกการตั้งค่าอัตโนมัติทันทีเมื่อมีการเปลี่ยนแปลง</p>
    </div>

    <div class="card">
      <div style="display:flex; justify-content:space-between; align-items:center;">
        <h2 style="margin:0;" data-i18n="gal_title">📂 คลังภาพและวิดีโอในบอร์ด (LittleFS)</h2>
        <span id="fsInfo" class="badge">-- KB</span>
      </div>
      <p class="hint" data-i18n="gal_hint">ติ๊กถูกเพื่อเปิดแสดงในรอบ Idle หรือกด 👁️ เพื่อยิงขึ้นจอ LyMini ทันที</p>
      <div id="mediaList" class="media-list">Loading...</div>
    </div>
  </div>

  <!-- TAB 2: Dedicated Auto-Update Max Brightness + Display Rotation + Night Schedule -->
  <div class="tab-pane" id="tab2">
    <!-- Dedicated Max Brightness + Eco Power-Saving Brightness Auto-Update Card -->
    <div class="card" style="border-color:var(--accent);">
      <div style="display:flex; justify-content:space-between; align-items:center;">
        <h2 style="margin:0;" data-i18n="br_title">🔆 ความสว่างสูงสุด (Max Brightness — Auto Update)</h2>
        <span id="briValueBadge" class="badge" style="font-size:14px;">75%</span>
      </div>
      <p class="hint" data-i18n="br_desc">เลื่อนแถบเพื่อปรับความสว่างหน้าจอแบบเรียลไทม์ทันที (และเป็นค่าเพดานความสว่างสูงสุดที่หน้าจอจะไม่สว่างเกินค่านี้)</p>
      <input type="range" id="rngBright" min="5" max="100" step="5" value="75" style="margin-top:10px;" />
      <div class="row">
        <button type="button" class="sm" onclick="setQuickBri(25)">25%</button>
        <button type="button" class="sm" onclick="setQuickBri(50)">50%</button>
        <button type="button" class="sm" onclick="setQuickBri(75)">75%</button>
        <button type="button" class="sm" onclick="setQuickBri(100)">100%</button>
      </div>

      <hr style="border:none; border-top:1px solid var(--border); margin:14px 0;">

      <div style="display:flex; justify-content:space-between; align-items:center;">
        <h2 style="margin:0; color:var(--accent);" data-i18n="idb_title">🌙 ความสว่างโหมดพักหน้าจอ (Idle Brightness)</h2>
        <span id="idleBriValueBadge" class="badge" style="font-size:14px;">45%</span>
      </div>
      <p class="hint" data-i18n="idb_desc">ความสว่างเมื่อเข้าโหมด Idle TV (>= 30 วินาที) ปรับทีละ 5%</p>
      <input type="range" id="rngIdleBright" min="5" max="100" step="5" value="45" style="margin-top:10px;" />
      <div class="row">
        <button type="button" class="sm" onclick="setQuickIdleBri(25)">25%</button>
        <button type="button" class="sm" onclick="setQuickIdleBri(45)">45%</button>
        <button type="button" class="sm" onclick="setQuickIdleBri(60)">60%</button>
        <button type="button" class="sm" onclick="setQuickIdleBri(80)">80%</button>
      </div>

      <hr style="border:none; border-top:1px solid var(--border); margin:14px 0;">

      <div style="display:flex; justify-content:space-between; align-items:center;">
        <h2 style="margin:0; color:var(--green);" data-i18n="ec_title">🔋 ความสว่างช่วงประหยัดพลังงาน / พักหน้าจอ (Eco Brightness)</h2>
        <span id="ecoBriValueBadge" class="badge" style="font-size:14px; color:var(--green);">15%</span>
      </div>
      <p class="hint" data-i18n="ec_desc">เลื่อนแถบเพื่อตั้งค่าความสว่างเมื่ออยู่ในโหมดประหยัดพลังงาน Eco (>= 5 นาที) แบบเรียลไทม์ (ไม่เกินค่า Max Brightness)</p>
      <input type="range" id="rngEcoBright" min="5" max="100" step="5" value="15" style="margin-top:10px; accent-color:var(--green);" />
      <div class="row">
        <button type="button" class="sm green" onclick="setQuickEcoBri(5)">5%</button>
        <button type="button" class="sm green" onclick="setQuickEcoBri(15)">15%</button>
        <button type="button" class="sm green" onclick="setQuickEcoBri(25)">25%</button>
        <button type="button" class="sm green" onclick="setQuickEcoBri(35)">35%</button>
      </div>
    </div>

    <div class="card">
      <h2 data-i18n="dp_title">🖥️ การหมุนหน้าจอ ปุ่มควบคุม และลำดับสี</h2>
      <div class="row">
        <div>
          <label data-i18n="dp_rot">การหมุนหน้าจอ (Screen Rotation):</label>
          <select id="selRotation" onchange="saveAllSettings(true)">
            <option value="0">0° (Portrait)</option>
            <option value="1">90° (Landscape Flip)</option>
            <option value="2">180° (Inverted Portrait)</option>
            <option value="3" selected>270° (Landscape - ค่าเริ่มต้น)</option>
          </select>
        </div>
        <div>
          <label data-i18n="dp_color">ลำดับสีจอ (Color Order):</label>
          <select id="selColorBgr" onchange="saveAllSettings(true)">
            <option value="1" selected>BGR (มาตรฐาน ST7735)</option>
            <option value="0">RGB (ทางเลือก)</option>
          </select>
        </div>
      </div>

      <div class="chk-item" style="margin-top:10px;">
        <label class="chk-left">
          <input type="checkbox" id="chkBtnRot" checked onchange="saveAllSettings(true)" />
          <span data-i18n="dp_btnrot">🔄 หมุนปุ่มซ้าย/ขวาตามการหมุนหน้าจออัตโนมัติ (Rotate Buttons with Screen)</span>
        </label>
      </div>

      <hr style="border:none; border-top:1px solid var(--border); margin:14px 0;">

      <h2 data-i18n="nt_title">🌙 ตั้งเวลาปิดไฟหน้าจอกลางคืน (Night Mode Screen-Off)</h2>
      <p class="hint" data-i18n="nt_desc">เมื่ออยู่ในโหมดพักหน้าจอ (Idle) และถึงช่วงเวลาที่กำหนด หน้าจอจะปิดไฟ Backlight (0%) อัตโนมัติ และตื่นเมื่อกดปุ่ม</p>
      <div class="chk-item" style="margin-top:8px;">
        <label class="chk-left">
          <input type="checkbox" id="chkNightEn" />
          <span data-i18n="nt_enable">🌙 เปิดใช้งานตั้งเวลาปิดหน้าจอกลางคืน</span>
        </label>
      </div>
      <div class="row">
        <div>
          <label data-i18n="nt_start">เวลาเริ่มปิดจอ (ชั่วโมง 0-23):</label>
          <input type="number" id="inpNightStart" min="0" max="23" value="23" />
        </div>
        <div>
          <label data-i18n="nt_end">เวลาเปิดจอตอนเช้า (ชั่วโมง 0-23):</label>
          <input type="number" id="inpNightEnd" min="0" max="23" value="6" />
        </div>
      </div>

      <p class="hint" data-i18n="auto_save_hint">⚡ บันทึกการตั้งค่าอัตโนมัติทันทีเมื่อมีการเปลี่ยนแปลง</p>
    </div>
  </div>

  <!-- TAB 3: Wi-Fi & Multi-Location Weather -->
  <div class="tab-pane" id="tab3">
    <div class="card">
      <h2 data-i18n="wf_title">📶 ตั้งค่าและจำค่า Wi-Fi (บันทึกถาวรสูงสุด 3 เครือข่าย)</h2>
      <div id="savedWifiList"></div>
      <button type="button" id="btnScanWifi" style="background:#334155; color:#fff;" data-i18n="wf_scan">🔍 สแกนหา Wi-Fi รอบข้าง</button>
      <div id="scanResults"></div>
      <label data-i18n="wf_ssid">ชื่อ Wi-Fi (SSID):</label>
      <input type="text" id="inpSsid" placeholder="SSID" />
      <label data-i18n="wf_pass">รหัสผ่าน Wi-Fi (Password):</label>
      <input type="password" id="inpPass" placeholder="Password" />
      <button type="button" id="btnSaveWifi" data-i18n="wf_save">💾 บันทึก Wi-Fi และเชื่อมต่อ</button>
    </div>

    <div class="card">
      <h2 data-i18n="wt_title">🌡️ สถานที่สภาพอากาศ (เลือกได้หลายเมือง สูงสุด 5 แห่ง)</h2>
      <p class="hint" data-i18n="wt_desc">บันทึกได้หลายสถานที่ ระบบจะสลับแสดงเมืองในหน้า Weather และหน้า Clock+Weather (กด 👁️ เพื่อดูเมืองนั้นบนจอทันที)</p>
      <div id="cityListBox"></div>

      <label data-i18n="wt_city">เพิ่มเมืองใหม่ (ภาษาอังกฤษ เช่น Bangkok, Chiang Mai, Phuket, Tokyo):</label>
      <div class="row">
        <input type="text" id="inpNewCity" placeholder="e.g. Chiang Mai" style="margin-top:0;" />
        <button type="button" class="green" id="btnAddCity" style="margin-top:0; width:auto; white-space:nowrap;" data-i18n="wt_add">➕ เพิ่มเมือง</button>
      </div>
      <button type="button" class="purple" id="btnRefreshWeather" data-i18n="wt_refresh">🔄 อัปเดตสภาพอากาศทุกเมืองเดี๋ยวนี้</button>
    </div>
  </div>

  <!-- TAB 4: About Me & Buy Me a Coffee (PromptPay QR) -->
  <div class="tab-pane" id="tab4">
    <div class="card" style="text-align:center;">
      <h2 data-i18n="ab_title">☕ เกี่ยวกับ LyMini & เลี้ยงกาแฟผู้พัฒนา (PromptPay QR)</h2>
      <p class="hint" id="abDescText" data-i18n="ab_desc">LyMini ESP32-C3 1.44" Retro Arcade & Smart Idle TV v0.3.2 by Zotix</p>

      <div style="margin: 14px auto; display:inline-block; padding:12px; background:#fff; border-radius:12px;">
        <canvas id="qrCanvas" width="112" height="112" style="width:224px; height:224px; image-rendering:pixelated; display:block;"></canvas>
      </div>
      <div style="font-weight:700; color:var(--gold); margin-bottom:10px;" data-i18n="ab_scan">📱 สแกน PromptPay QR เพื่อสนับสนุนค่ากาแฟผู้พัฒนา (Zotix)</div>

      <div class="row">
        <button type="button" class="gold" id="btnShowQrEsp" data-i18n="ab_show_esp">👁️ แสดง QR Code บนจอ LyMini ทันที</button>
      </div>
    </div>

    <div class="card" style="border-color:var(--accent);">
      <div style="display:flex; justify-content:space-between; align-items:center;">
        <h2 style="margin:0;">🚀 อัปเดตเฟิร์มแวร์ (OTA Update)</h2>
        <span id="curFwVer" class="badge">v0.3.2</span>
      </div>
      <p class="hint">อัปเดตเกมใหม่หรือระบบผ่าน Wi-Fi โดยไม่ลบข้อมูลรูป/วิดีโอใน LittleFS และไม่ลบ High Score (หรือกดปุ่ม BOOT บนบอร์ดค้าง 10 วินาที)</p>

      <div style="margin-top:10px; padding:10px; background:rgba(0,0,0,0.2); border-radius:8px;">
        <label style="margin-top:0;">🌐 Check &amp; Update Online (GitHub: zotix/LyMini)</label>
        <input type="text" id="inpOtaUrl" placeholder="URL .bin (เว้นว่างเพื่อดึงจาก GitHub Release ล่าสุด)" />
        <div class="row" style="margin-top:8px;">
          <button type="button" id="btnCheckOta">🔍 Check Online</button>
          <button type="button" class="green" id="btnRunOnlineOta">⬇️ Update Online</button>
        </div>
        <div id="otaOnlineMsg" class="hint" style="margin-top:6px;"></div>
      </div>

      <hr style="border:none; border-top:1px solid var(--border); margin:14px 0;">

      <div>
        <label style="margin-top:0;">📦 Manual Firmware Upload (.bin)</label>
        <input type="file" id="otaFileInput" accept=".bin" />
        <div class="row" style="margin-top:8px;">
          <button type="button" class="purple" id="btnUploadOta">🚀 Upload &amp; Flash .bin</button>
        </div>
        <div id="otaUploadMsg" class="hint" style="margin-top:6px;"></div>
      </div>
    </div>
  </div>
</div>

<script>
  let curLang = localStorage.getItem('lymini_lang') || 'th';
  const i18n = {
    th: {
      tab_studio: '1. ✂️ สตูดิโอ',
      tab_idle: '2. 🎲 พักหน้าจอ & คลัง',
      tab_display: '3. 🔆 ความสว่าง & จอ',
      tab_wifi: '4. 📶 Wi-Fi & อากาศ',
      tab_about: '5. 🚀 OTA & เกี่ยวกับ',
      st_title: '✂️ 128×128 Media Studio (iOS / Android / PC + Live View)',
      st_desc: 'รองรับรูปภาพ (.JPG, .PNG, .WEBP, .HEIC) และวิดีโอจาก iPhone/มือถือ (.MOV, .MP4) ลากนิ้วเพื่อเลื่อนกรอบ หรือถ่าง 2 นิ้วเพื่อซูม',
      st_pan_hint: '💡 ลากบนกรอบภาพเพื่อจัดตำแหน่ง (Pan) / ถ่าง 2 นิ้วเพื่อซูม',
      st_rot: 'หมุน 90°',
      st_reset: 'รีเซ็ต',
      st_autolive: '👁️ Live View บนจอ LyMini อัตโนมัติ ขณะลาก/ปรับแต่ง',
      st_sharpen: '✨ เร่งความคมชัดภาพ (Edge Sharpening)',
      st_vibrant: '🎨 สีสดใสพิเศษ (Vibrant LCD Boost)',
      st_quality_lbl: 'คุณภาพวิดีโอ:',
      st_q_hd: '🌟 HD 16-bit (65,536 สี - คมชัดสูงสุด)',
      st_q_compact: '⚡ Compact 8-bit (ประหยัดพื้นที่)',
      st_btn_live: '👁️ ส่งขึ้นจอทันที',
      st_btn_save: '🚀 บันทึกลงเครื่อง & เล่นทันที',
      id_title: '🎲 ตั้งค่าโหมดพักหน้าจอ (Smart Idle & Live Preview)',
      id_smart_hint: '✨ กดปุ่ม 👁️ ท้ายรายการใดก็ได้ เพื่อดูตัวอย่างหน้านั้นบนจอจริงของ LyMini ทันที!',
      id_select: 'เลือกหน้าจอที่จะแสดงเมื่อพักหน้าจอ (กด 👁️ เพื่อดูบนจอจริง):',
      id_clk_dig: '🕒 นาฬิกาดิจิทัล (Digital Clock)',
      id_clk_ana: '🕰️ นาฬิกาเข็ม (Analog Clock)',
      id_clk_flp: '🔢 นาฬิกาพลิก (Flip Clock)',
      id_weather: '🌡️ สภาพอากาศล้วน (Weather)',
      id_clk_wt: '🕒+🌡️ นาฬิกา + สภาพอากาศ (Clock + Weather)',
      id_photo: '🖼️ กรอบรูปภาพ (Photos)',
      id_video: '🎬 วิดีโอแอนิเมชัน (Videos)',
      id_qr: '☕ Buy Me a Coffee (PromptPay QR — เปิดแสดงเสมอ)',
      id_lock_qr: '🔒 ล็อกหน้าจอ Idle ให้แสดงเฉพาะ PromptPay QR Code ตลอดเวลา',
      id_random: '🔀 Random Mode (สุ่มลำดับหน้าจอ และสุ่มรูปภาพ/วิดีโอ)',
      id_ecovid: '🎬 เล่นวิดีโอเมื่ออยู่ใน Eco Mode (ปิด = ประหยัดแบตมากขึ้น)',
      id_timeout: 'เข้าสู่ Idle Display (40MHz / แสง Idle) เมื่อไม่กดปุ่ม (วินาที):',
      eco_timeout: 'เข้าสู่ Eco Display (20MHz / แสง Eco) เมื่อไม่กดปุ่ม (วินาที):',
      sleep_timeout: 'เข้าสู่ Sleep Mode ปิดจอ/พักเครื่อง (นาที, 0=ปิด):',
      id_cycle: 'สลับหน้าทุกๆ (วินาที):',
      gal_title: '📂 คลังภาพและวิดีโอในบอร์ด (LittleFS)',
      gal_hint: 'ติ๊กถูกเพื่อเปิดแสดงในรอบ Idle หรือกด 👁️ เพื่อยิงขึ้นจอ LyMini ทันที',
      br_title: '🔆 ความสว่างสูงสุด (Max Brightness — Auto Update)',
      br_desc: 'เลื่อนแถบเพื่อปรับความสว่างหน้าจอแบบเรียลไทม์ทันที (และเป็นเพดานความสว่างสูงสุดที่หน้าจอจะไม่สว่างเกินค่านี้)',
      ec_title: '🔋 ความสว่างช่วงประหยัดพลังงาน / พักหน้าจอ (Eco Brightness)',
      ec_desc: 'เลื่อนแถบเพื่อตั้งค่าความสว่างเมื่ออยู่ในโหมดพักหน้าจอ (Idle) แบบเรียลไทม์ (ไม่เกินค่า Max Brightness)',
      dp_title: '🖥️ การหมุนหน้าจอ ปุ่มควบคุม และลำดับสี',
      dp_rot: 'การหมุนหน้าจอ (Screen Rotation):',
      dp_color: 'ลำดับสีจอ (Color Order):',
      dp_btnrot: '🔄 หมุนปุ่มซ้าย/ขวาตามการหมุนหน้าจออัตโนมัติ (Rotate Buttons with Screen)',
      nt_title: '🌙 ตั้งเวลาปิดไฟหน้าจอกลางคืน (Night Mode Screen-Off)',
      nt_desc: 'เมื่ออยู่ในโหมดพักหน้าจอ (Idle) และถึงช่วงเวลาที่กำหนด หน้าจอจะปิดไฟ Backlight (0%) อัตโนมัติ และตื่นเมื่อกดปุ่ม',
      nt_enable: '🌙 เปิดใช้งานตั้งเวลาปิดหน้าจอกลางคืน',
      nt_start: 'เวลาเริ่มปิดจอ (ชั่วโมง 0-23):',
      nt_end: 'เวลาเปิดจอตอนเช้า (ชั่วโมง 0-23):',
      wf_title: '📶 ตั้งค่าและจำค่า Wi-Fi (บันทึกถาวรสูงสุด 3 เครือข่าย)',
      wf_scan: '🔍 สแกนหา Wi-Fi รอบข้าง',
      wf_ssid: 'ชื่อ Wi-Fi (SSID):',
      wf_pass: 'รหัสผ่าน Wi-Fi (Password):',
      wf_save: '💾 บันทึก Wi-Fi และเชื่อมต่อ',
      wt_title: '🌡️ สถานที่สภาพอากาศ (เลือกได้หลายเมือง สูงสุด 5 แห่ง)',
      wt_desc: 'บันทึกได้หลายสถานที่ ระบบจะสลับแสดงเมืองในหน้า Weather และหน้า Clock+Weather (กด 👁️ เพื่อดูเมืองนั้นบนจอทันที)',
      wt_city: 'เพิ่มเมืองใหม่ (ภาษาอังกฤษ เช่น Bangkok, Chiang Mai, Phuket, Tokyo):',
      wt_add: '➕ เพิ่มเมือง',
      wt_refresh: '🔄 อัปเดตสภาพอากาศทุกเมืองเดี๋ยวนี้',
      ab_title: '☕ เกี่ยวกับ LyMini & เลี้ยงกาแฟผู้พัฒนา (PromptPay QR)',
      ab_desc: 'LyMini ESP32-C3 1.44" Retro Arcade & Smart Idle TV v0.3.2 by Zotix',
      ab_scan: '📱 สแกน PromptPay QR เพื่อสนับสนุนค่ากาแฟผู้พัฒนา (Zotix)',
      ab_show_esp: '👁️ แสดง QR Code บนจอ LyMini ทันที',
      auto_save_hint: '⚡ บันทึกการตั้งค่าอัตโนมัติทันทีเมื่อมีการเปลี่ยนแปลง',
      btn_save: '💾 บันทึกการตั้งค่า'
    },
    en: {
      tab_studio: '1. ✂️ Studio',
      tab_idle: '2. 🎲 Idle & Gallery',
      tab_display: '3. 🔆 Brightness & Display',
      tab_wifi: '4. 📶 Wi-Fi & Weather',
      tab_about: '5. 🚀 OTA & About',
      st_title: '✂️ 128×128 Media Studio (iOS / Android / PC + Live View)',
      st_desc: 'Supports photos (.JPG, .PNG, .WEBP, .HEIC) and mobile videos (.MOV, .MP4). Drag to pan or pinch to zoom.',
      st_pan_hint: '💡 Drag on canvas to pan / Pinch with 2 fingers to zoom',
      st_rot: 'Rotate 90°',
      st_reset: 'Reset',
      st_autolive: '👁️ Auto Live View on LyMini while editing',
      st_sharpen: '✨ Edge Sharpening',
      st_vibrant: '🎨 Vibrant LCD Color Boost',
      st_quality_lbl: 'Video Quality:',
      st_q_hd: '🌟 HD 16-bit (65,536 colors - Crisp)',
      st_q_compact: '⚡ Compact 8-bit (Space Saver)',
      st_btn_live: '👁️ Push to Screen Now',
      st_btn_save: '🚀 Save to Flash & Play',
      id_title: '🎲 Smart Idle Display & Live Preview',
      id_smart_hint: '✨ Click 👁️ next to any mode to preview it immediately on the physical LyMini screen!',
      id_select: 'Select screens to include in Idle rotation:',
      id_clk_dig: '🕒 Digital Clock',
      id_clk_ana: '🕰️ Analog Clock',
      id_clk_flp: '🔢 Flip Clock',
      id_weather: '🌡️ Weather Only',
      id_clk_wt: '🕒+🌡️ Clock + Weather Combo',
      id_photo: '🖼️ Photo Frame',
      id_video: '🎬 Video / TV',
      id_qr: '☕ Buy Me a Coffee (PromptPay QR — Always Active)',
      id_lock_qr: '🔒 Lock Idle Screen on PromptPay QR Code only',
      id_random: '🔀 Random Mode (Shuffle screens, photos & videos)',
      id_ecovid: '🎬 Play Video in Eco Mode (Off = better battery life)',
      id_timeout: 'Enter Idle Display (40MHz / Idle Bri) after inactivity (sec):',
      eco_timeout: 'Enter Eco Display (20MHz / Eco Bri) after inactivity (sec):',
      sleep_timeout: 'Enter Sleep Mode Screen-Off (minutes, 0=off):',
      id_cycle: 'Cycle screen every (sec):',
      gal_title: '📂 On-Board Media Gallery (LittleFS)',
      gal_hint: 'Check to enable in Idle rotation, or click 👁️ to display immediately on LyMini',
      br_title: '🔆 Max Brightness Ceiling (Auto Update)',
      br_desc: 'Slide to adjust screen brightness in real-time (sets the maximum brightness limit the screen will never exceed).',
      ec_title: '🔋 Eco / Idle Power-Saving Brightness (Auto Update)',
      ec_desc: 'Slide to adjust screen brightness used during Idle power-saving mode (never exceeds Max Brightness).',
      dp_title: '🖥️ Screen Rotation, Buttons & Color',
      dp_rot: 'Screen Rotation:',
      dp_color: 'Display Color Order:',
      dp_btnrot: '🔄 Rotate Left/Right Buttons with Screen Orientation',
      nt_title: '🌙 Night Mode Screen-Off Schedule',
      nt_desc: 'Automatically turns off LCD backlight (0%) during scheduled night hours while in Idle mode.',
      nt_enable: '🌙 Enable Night Mode Screen-Off Schedule',
      nt_start: 'Turn Off Hour (0-23):',
      nt_end: 'Wake Up Hour (0-23):',
      wf_title: '📶 Persistent Multi-WiFi Setup (Up to 3 Networks)',
      wf_scan: '🔍 Scan Nearby Wi-Fi Networks',
      wf_ssid: 'Wi-Fi SSID:',
      wf_pass: 'Wi-Fi Password:',
      wf_save: '💾 Save Wi-Fi & Connect',
      wt_title: '🌡️ Multi-Location Weather (Up to 5 Cities)',
      wt_desc: 'Save multiple cities to cycle in Weather and Clock+Weather modes (click 👁️ to preview on screen).',
      wt_city: 'Add New City (e.g. Bangkok, Chiang Mai, Phuket, Tokyo):',
      wt_add: '➕ Add City',
      wt_refresh: '🔄 Refresh All Weather Now',
      ab_title: '☕ About LyMini & Buy Me a Coffee (PromptPay QR)',
      ab_desc: 'LyMini ESP32-C3 1.44" Retro Arcade & Smart Idle TV v0.3.2 by Zotix',
      ab_scan: '📱 Scan PromptPay QR Code to Support the Developer (Zotix)',
      ab_show_esp: '👁️ Show QR Code on LyMini Now',
      auto_save_hint: '⚡ Auto-saved immediately whenever you change any setting',
      btn_save: '💾 Save Settings'
    }
  };

  function applyLang() {
    document.querySelectorAll('[data-i18n]').forEach(el => {
      const k = el.getAttribute('data-i18n');
      if (i18n[curLang][k]) el.innerText = i18n[curLang][k];
    });
    document.getElementById('btnLang').innerText = (curLang === 'th') ? '🇹🇭 ไทย | Switch EN' : '🇬🇧 EN | สลับภาษาไทย';
  }

  document.getElementById('btnLang').onclick = () => {
    curLang = (curLang === 'th') ? 'en' : 'th';
    localStorage.setItem('lymini_lang', curLang);
    applyLang();
  };

  function stopAndResetStudio() {
    if (animReq) {
      cancelAnimationFrame(animReq);
      animReq = null;
    }
    try {
      srcVideo.pause();
      srcVideo.removeAttribute('src');
      srcVideo.load();
    } catch (e) {}
    const hadActiveMedia = (mediaType !== 'none');
    mediaType = 'none';
    currentImg = null;
    if (mediaInput) mediaInput.value = '';
    zoom = 1.0; rotation = 0; panX = 0; panY = 0;
    liveQueued = false;
    drawPlaceholder();
    if (uploadMsg) uploadMsg.innerText = '';
    if (hadActiveMedia) {
      fetch('/api/stop_live', { method: 'POST' }).catch(() => {});
    }
  }

  window.switchTab = (idx) => {
    if (idx !== 0) {
      stopAndResetStudio();
    }
    document.querySelectorAll('.tab-btn').forEach((b, i) => b.classList.toggle('active', i === idx));
    document.querySelectorAll('.tab-pane').forEach((p, i) => p.classList.toggle('active', i === idx));
    if (idx === 4) loadQrCanvas();
  };

  // --- Real-Time Connection Heartbeat Check (Every 4s) ---
  async function checkConnectionHeartbeat() {
    if (isUploadingMedia) return;
    const badge = document.getElementById('connBadge');
    try {
      const ctrl = new AbortController();
      const tid = setTimeout(() => ctrl.abort(), 2500);
      const r = await fetch('/api/ping', { signal: ctrl.signal });
      clearTimeout(tid);
      if (r.ok) {
        const d = await r.json();
        badge.className = 'badge online';
        badge.innerText = (curLang === 'th') ? '🟢 เชื่อมต่ออยู่' : '🟢 Online';
        document.getElementById('battBadge').innerText = (d.usb ? '⚡ USB ' : '🔋 ') + d.batt + '%';
        return;
      }
    } catch (e) {}
    badge.className = 'badge offline';
    badge.innerText = (curLang === 'th') ? '🔴 ขาดการเชื่อมต่อ' : '🔴 Offline';
  }
  setInterval(checkConnectionHeartbeat, 4000);

  // --- Instant Auto-Update Max Brightness Slider ---
  const rngBright = document.getElementById('rngBright');
  const briBadge = document.getElementById('briValueBadge');
  let briBusy = false, briQueuedVal = null;

  function sendBrightnessAuto(val, saveToNvs) {
    briBadge.innerText = val + '%';
    if (briBusy && !saveToNvs) { briQueuedVal = val; return; }
    briBusy = true;
    fetch(`/api/brightness?val=${val}&save=${saveToNvs ? 1 : 0}`, { method: 'POST' }).finally(() => {
      briBusy = false;
      if (briQueuedVal !== null) {
        const nextV = briQueuedVal;
        briQueuedVal = null;
        sendBrightnessAuto(nextV, false);
      }
    });
  }

  rngBright.addEventListener('input', (e) => {
    sendBrightnessAuto(e.target.value, false);
  });
  rngBright.addEventListener('change', (e) => {
    sendBrightnessAuto(e.target.value, true);
  });

  window.setQuickBri = (val) => {
    rngBright.value = val;
    sendBrightnessAuto(val, true);
  };

  // --- Instant Auto-Update Eco / Idle Power-Saving Brightness Slider ---
  const rngEcoBright = document.getElementById('rngEcoBright');
  const ecoBriBadge = document.getElementById('ecoBriValueBadge');
  let ecoBriBusy = false, ecoBriQueuedVal = null;

  function sendEcoBrightnessAuto(val, saveToNvs) {
    ecoBriBadge.innerText = val + '%';
    if (ecoBriBusy && !saveToNvs) { ecoBriQueuedVal = val; return; }
    ecoBriBusy = true;
    fetch(`/api/eco_brightness?val=${val}&save=${saveToNvs ? 1 : 0}`, { method: 'POST' }).finally(() => {
      ecoBriBusy = false;
      if (ecoBriQueuedVal !== null) {
        const nextV = ecoBriQueuedVal;
        ecoBriQueuedVal = null;
        sendEcoBrightnessAuto(nextV, false);
      }
    });
  }

  rngEcoBright.addEventListener('input', (e) => {
    sendEcoBrightnessAuto(e.target.value, false);
  });
  rngEcoBright.addEventListener('change', (e) => {
    sendEcoBrightnessAuto(e.target.value, true);
  });

  window.setQuickEcoBri = (val) => {
    rngEcoBright.value = val;
    sendEcoBrightnessAuto(val, true);
  };

  // --- Instant Auto-Update Idle Brightness Slider ---
  const rngIdleBright = document.getElementById('rngIdleBright');
  const idleBriBadge = document.getElementById('idleBriValueBadge');
  let idleBriBusy = false, idleBriQueuedVal = null;

  function sendIdleBrightnessAuto(val, saveToNvs) {
    idleBriBadge.innerText = val + '%';
    if (idleBriBusy && !saveToNvs) { idleBriQueuedVal = val; return; }
    idleBriBusy = true;
    fetch(`/api/idle_brightness?val=${val}&save=${saveToNvs ? 1 : 0}`, { method: 'POST' }).finally(() => {
      idleBriBusy = false;
      if (idleBriQueuedVal !== null) {
        const nextV = idleBriQueuedVal;
        idleBriQueuedVal = null;
        sendIdleBrightnessAuto(nextV, false);
      }
    });
  }

  rngIdleBright.addEventListener('input', (e) => {
    sendIdleBrightnessAuto(e.target.value, false);
  });
  rngIdleBright.addEventListener('change', (e) => {
    sendIdleBrightnessAuto(e.target.value, true);
  });

  window.setQuickIdleBri = (val) => {
    rngIdleBright.value = val;
    sendIdleBrightnessAuto(val, true);
  };

  // --- Preview Any Idle Mode Immediately on ESP32 ---
  window.previewIdle = async (scrIdx, cityIdx = -1) => {
    document.getElementById('chkNightEn').checked = false;
    let url = `/api/preview_idle?scr=${scrIdx}`;
    if (cityIdx >= 0) url += `&cityIdx=${cityIdx}`;
    await fetch(url, { method: 'POST' });
  };

  // --- Studio Canvas & Video Upload Fix ---
  const canvas = document.getElementById('cropCanvas');
  const ctx = canvas.getContext('2d');
  const mediaInput = document.getElementById('mediaInput');
  const srcVideo = document.getElementById('srcVideo');
  const uploadMsg = document.getElementById('uploadMsg');

  let mediaType = 'none';
  let currentImg = null;
  let zoom = 1.0, rotation = 0, panX = 0, panY = 0;
  let isDragging = false, startX = 0, startY = 0, lastPinchDist = 0;
  let liveBusy = false, liveQueued = false, isUploadingMedia = false;
  let animReq = null;
  let lastVideoLiveMs = 0;

  function drawPlaceholder() {
    ctx.fillStyle = '#090d16';
    ctx.fillRect(0, 0, 128, 128);
    ctx.strokeStyle = '#38bdf8';
    ctx.strokeRect(2, 2, 124, 124);
    ctx.fillStyle = '#38bdf8';
    ctx.font = 'bold 12px sans-serif';
    ctx.textAlign = 'center';
    ctx.fillText('128 × 128 PREVIEW', 64, 58);
    ctx.fillStyle = '#94a3b8';
    ctx.font = '10px sans-serif';
    ctx.fillText('Select Photo or Video', 64, 76);
  }
  drawPlaceholder();

  function getSteppedCanvas(source) {
    let curW = source.naturalWidth || source.videoWidth || source.width;
    let curH = source.naturalHeight || source.videoHeight || source.height;
    if (!curW || !curH || (curW <= 512 && curH <= 512)) return source;

    let srcCanvas = document.createElement('canvas');
    srcCanvas.width = curW;
    srcCanvas.height = curH;
    let sctx = srcCanvas.getContext('2d');
    sctx.drawImage(source, 0, 0);

    while (curW > 512 || curH > 512) {
      const nextW = Math.max(32, Math.round(curW * 0.5));
      const nextH = Math.max(32, Math.round(curH * 0.5));
      const stepCanvas = document.createElement('canvas');
      stepCanvas.width = nextW;
      stepCanvas.height = nextH;
      const stepCtx = stepCanvas.getContext('2d');
      stepCtx.imageSmoothingEnabled = true;
      stepCtx.imageSmoothingQuality = 'high';
      stepCtx.drawImage(srcCanvas, 0, 0, nextW, nextH);
      srcCanvas = stepCanvas;
      curW = nextW;
      curH = nextH;
    }
    return srcCanvas;
  }

  function renderSourceToCanvas(source, sw, sh) {
    if (!sw || !sh) return;
    const vibrant = document.getElementById('chkVibrant') ? document.getElementById('chkVibrant').checked : true;
    ctx.imageSmoothingEnabled = true;
    ctx.imageSmoothingQuality = 'high';
    ctx.filter = vibrant ? 'contrast(1.10) saturate(1.20)' : 'none';
    ctx.fillStyle = '#000';
    ctx.fillRect(0, 0, 128, 128);
    ctx.save();
    ctx.translate(64 + panX, 64 + panY);
    ctx.rotate(rotation * Math.PI / 180);
    ctx.scale(zoom, zoom);
    const aspect = sw / sh;
    let dw = 128, dh = 128;
    if (aspect > 1) { dh = 128; dw = 128 * aspect; }
    else { dw = 128; dh = 128 / aspect; }
    ctx.drawImage(source, -dw / 2, -dh / 2, dw, dh);
    ctx.restore();
    ctx.filter = 'none';
  }

  function renderCanvas() {
    if (mediaType === 'image' && currentImg) {
      renderSourceToCanvas(currentImg, currentImg.width, currentImg.height);
      if (document.getElementById('chkAutoLive').checked && !isUploadingMedia) triggerLiveFrame();
    } else if (mediaType === 'video') {
      const vw = srcVideo.videoWidth || 128;
      const vh = srcVideo.videoHeight || 128;
      renderSourceToCanvas(srcVideo, vw, vh);
      if (document.getElementById('chkAutoLive').checked && !isUploadingMedia) triggerLiveFrame();
    }
  }

  function videoLoop() {
    if (mediaType === 'video' && !srcVideo.paused && !srcVideo.ended && !isUploadingMedia) {
      renderSourceToCanvas(srcVideo, srcVideo.videoWidth || 128, srcVideo.videoHeight || 128);
      const now = Date.now();
      if (document.getElementById('chkAutoLive').checked && (now - lastVideoLiveMs >= 160)) {
        lastVideoLiveMs = now;
        triggerLiveFrame();
      }
      animReq = requestAnimationFrame(videoLoop);
    }
  }

  mediaInput.addEventListener('change', (e) => {
    const file = e.target.files[0];
    if (!file) return;
    if (animReq) cancelAnimationFrame(animReq);
    srcVideo.pause();
    zoom = 1.0; rotation = 0; panX = 0; panY = 0;

    const isVid = file.type.startsWith('video/') || /\.(mov|mp4|webm|m4v)$/i.test(file.name);
    if (isVid) {
      mediaType = 'video';
      uploadMsg.innerText = (curLang === 'th') ? '⏳ กำลังโหลดวิดีโอ...' : '⏳ Decoding video...';
      const url = URL.createObjectURL(file);
      srcVideo.src = url;
      srcVideo.muted = true;
      srcVideo.playsInline = true;
      srcVideo.load();

      const startVideoPreview = () => {
        srcVideo.play().then(() => {
          uploadMsg.innerText = (curLang === 'th') ? '🎬 วิดีโอกำลังเล่น! ปรับมุมกล้องแล้วกดบันทึกได้เลย' : '🎬 Video playing! Adjust crop & click Save.';
          videoLoop();
        }).catch(() => {
          srcVideo.currentTime = 0.1;
          setTimeout(renderCanvas, 150);
        });
      };

      srcVideo.onloadeddata = startVideoPreview;
    } else {
      mediaType = 'image';
      uploadMsg.innerText = (curLang === 'th') ? '⏳ กำลังโหลดรูปภาพ...' : '⏳ Loading image...';
      const url = URL.createObjectURL(file);
      const img = new Image();
      img.onload = () => {
        currentImg = getSteppedCanvas(img);
        renderCanvas();
        uploadMsg.innerText = (curLang === 'th') ? '🖼️ โหลดรูปสำเร็จ! ลากจัดตำแหน่งได้เลย' : '🖼️ Image ready!';
      };
      img.src = url;
    }
  });

  // Mouse & Touch Gestures
  canvas.addEventListener('mousedown', (e) => { isDragging = true; startX = e.clientX - panX * 2; startY = e.clientY - panY * 2; });
  window.addEventListener('mousemove', (e) => {
    if (!isDragging) return;
    panX = (e.clientX - startX) * 0.5; panY = (e.clientY - startY) * 0.5;
    renderCanvas();
  });
  window.addEventListener('mouseup', () => { isDragging = false; });
  canvas.addEventListener('wheel', (e) => {
    e.preventDefault();
    if (e.deltaY < 0) zoom *= 1.1;
    else zoom /= 1.1;
    zoom = Math.max(0.3, Math.min(5.0, zoom));
    renderCanvas();
  }, { passive: false });

  canvas.addEventListener('touchstart', (e) => {
    e.preventDefault();
    if (e.touches.length === 1) {
      isDragging = true;
      startX = e.touches[0].clientX - panX * 2;
      startY = e.touches[0].clientY - panY * 2;
    } else if (e.touches.length === 2) {
      isDragging = false;
      const dx = e.touches[0].clientX - e.touches[1].clientX;
      const dy = e.touches[0].clientY - e.touches[1].clientY;
      lastPinchDist = Math.hypot(dx, dy);
    }
  }, { passive: false });

  canvas.addEventListener('touchmove', (e) => {
    e.preventDefault();
    if (e.touches.length === 1 && isDragging) {
      panX = (e.touches[0].clientX - startX) * 0.5;
      panY = (e.touches[0].clientY - startY) * 0.5;
      renderCanvas();
    } else if (e.touches.length === 2 && lastPinchDist > 0) {
      const dx = e.touches[0].clientX - e.touches[1].clientX;
      const dy = e.touches[0].clientY - e.touches[1].clientY;
      const dist = Math.hypot(dx, dy);
      zoom *= (dist / lastPinchDist);
      if (zoom < 0.3) zoom = 0.3;
      if (zoom > 5.0) zoom = 5.0;
      lastPinchDist = dist;
      renderCanvas();
    }
  }, { passive: false });

  canvas.addEventListener('touchend', () => { isDragging = false; lastPinchDist = 0; });

  document.getElementById('btnZoomIn').onclick = () => { zoom *= 1.15; renderCanvas(); };
  document.getElementById('btnZoomOut').onclick = () => { zoom /= 1.15; renderCanvas(); };
  document.getElementById('btnRotate').onclick = () => { rotation = (rotation + 90) % 360; renderCanvas(); };
  document.getElementById('btnResetCrop').onclick = () => { zoom = 1.0; rotation = 0; panX = 0; panY = 0; renderCanvas(); };
  const chkSharpenEl = document.getElementById('chkSharpen');
  const chkVibrantEl = document.getElementById('chkVibrant');
  if (chkSharpenEl) chkSharpenEl.onchange = renderCanvas;
  if (chkVibrantEl) chkVibrantEl.onchange = renderCanvas;

  function applyEdgeSharpen(d) {
    const copy = new Uint8ClampedArray(d);
    const amount = 0.45;
    for (let y = 1; y < 127; ++y) {
      for (let x = 1; x < 127; ++x) {
        const idx = (y * 128 + x) * 4;
        for (let c = 0; c < 3; ++c) {
          const val = copy[idx + c];
          const blur = (copy[((y - 1) * 128 + x) * 4 + c] +
                        copy[((y + 1) * 128 + x) * 4 + c] +
                        copy[(y * 128 + (x - 1)) * 4 + c] +
                        copy[(y * 128 + (x + 1)) * 4 + c]) * 0.25;
          const res = val + amount * (val - blur);
          d[idx + c] = Math.min(255, Math.max(0, Math.round(res)));
        }
      }
    }
  }

  function canvasToRgb332() {
    const imgData = ctx.getImageData(0, 0, 128, 128);
    const d = imgData.data;
    const rBuf = new Float32Array(128 * 128);
    const gBuf = new Float32Array(128 * 128);
    const bBuf = new Float32Array(128 * 128);
    for (let i = 0, j = 0; i < d.length; i += 4, ++j) {
      rBuf[j] = d[i]; gBuf[j] = d[i + 1]; bBuf[j] = d[i + 2];
    }
    const out = new Uint8Array(128 * 128);
    let outIdx = 0;
    for (let y = 0; y < 128; ++y) {
      for (let x = 0; x < 128; ++x) {
        const p = y * 128 + x;
        const curR = Math.min(255, Math.max(0, rBuf[p]));
        const curG = Math.min(255, Math.max(0, gBuf[p]));
        const curB = Math.min(255, Math.max(0, bBuf[p]));

        const r3 = Math.min(7, Math.max(0, Math.round(curR * 7 / 255)));
        const g3 = Math.min(7, Math.max(0, Math.round(curG * 7 / 255)));
        const b2 = Math.min(3, Math.max(0, Math.round(curB * 3 / 255)));

        const qR = (r3 * 255 + 3) / 7;
        const qG = (g3 * 255 + 3) / 7;
        const qB = (b2 * 255 + 1) / 3;

        const errR = curR - qR;
        const errG = curG - qG;
        const errB = curB - qB;

        if (x + 1 < 128) {
          const n1 = p + 1;
          rBuf[n1] += errR * 0.4375; gBuf[n1] += errG * 0.4375; bBuf[n1] += errB * 0.4375;
        }
        if (y + 1 < 128) {
          if (x > 0) {
            const n2 = p + 128 - 1;
            rBuf[n2] += errR * 0.1875; gBuf[n2] += errG * 0.1875; bBuf[n2] += errB * 0.1875;
          }
          const n3 = p + 128;
          rBuf[n3] += errR * 0.3125; gBuf[n3] += errG * 0.3125; bBuf[n3] += errB * 0.3125;
          if (x + 1 < 128) {
            const n4 = p + 128 + 1;
            rBuf[n4] += errR * 0.0625; gBuf[n4] += errG * 0.0625; bBuf[n4] += errB * 0.0625;
          }
        }
        out[outIdx++] = (r3 << 5) | (g3 << 2) | b2;
      }
    }
    return out;
  }

  function canvasToRgb565() {
    const imgData = ctx.getImageData(0, 0, 128, 128);
    const d = imgData.data;
    const sharpen = document.getElementById('chkSharpen') ? document.getElementById('chkSharpen').checked : true;
    if (sharpen) applyEdgeSharpen(d);

    const rBuf = new Float32Array(128 * 128);
    const gBuf = new Float32Array(128 * 128);
    const bBuf = new Float32Array(128 * 128);
    for (let i = 0, j = 0; i < d.length; i += 4, ++j) {
      rBuf[j] = d[i]; gBuf[j] = d[i + 1]; bBuf[j] = d[i + 2];
    }

    const out = new Uint8Array(128 * 128 * 2);
    let outIdx = 0;
    for (let y = 0; y < 128; ++y) {
      for (let x = 0; x < 128; ++x) {
        const p = y * 128 + x;
        const curR = Math.min(255, Math.max(0, rBuf[p]));
        const curG = Math.min(255, Math.max(0, gBuf[p]));
        const curB = Math.min(255, Math.max(0, bBuf[p]));

        const r5 = Math.min(31, Math.max(0, Math.round(curR * 31 / 255)));
        const g6 = Math.min(63, Math.max(0, Math.round(curG * 63 / 255)));
        const b5 = Math.min(31, Math.max(0, Math.round(curB * 31 / 255)));

        const qR = (r5 * 255 + 15) / 31;
        const qG = (g6 * 255 + 31) / 63;
        const qB = (b5 * 255 + 15) / 31;

        const errR = curR - qR;
        const errG = curG - qG;
        const errB = curB - qB;

        if (x + 1 < 128) {
          const n1 = p + 1;
          rBuf[n1] += errR * 0.4375; gBuf[n1] += errG * 0.4375; bBuf[n1] += errB * 0.4375;
        }
        if (y + 1 < 128) {
          if (x > 0) {
            const n2 = p + 128 - 1;
            rBuf[n2] += errR * 0.1875; gBuf[n2] += errG * 0.1875; bBuf[n2] += errB * 0.1875;
          }
          const n3 = p + 128;
          rBuf[n3] += errR * 0.3125; gBuf[n3] += errG * 0.3125; bBuf[n3] += errB * 0.3125;
          if (x + 1 < 128) {
            const n4 = p + 128 + 1;
            rBuf[n4] += errR * 0.0625; gBuf[n4] += errG * 0.0625; bBuf[n4] += errB * 0.0625;
          }
        }

        const rgb565 = (r5 << 11) | (g6 << 5) | b5;
        out[outIdx++] = (rgb565 >> 8) & 0xFF;
        out[outIdx++] = rgb565 & 0xFF;
      }
    }
    return out;
  }

  function triggerLiveFrame() {
    if (isUploadingMedia) return;
    document.getElementById('chkNightEn').checked = false;
    if (liveBusy) { liveQueued = true; return; }
    liveBusy = true;
    const buf = canvasToRgb332();
    const fd = new FormData();
    fd.append('frame', new Blob([buf], { type: 'application/octet-stream' }), 'live.bin');
    fetch('/api/liveframe', { method: 'POST', body: fd }).finally(() => {
      liveBusy = false;
      if (liveQueued && !isUploadingMedia) { liveQueued = false; triggerLiveFrame(); }
    });
  }

  document.getElementById('btnLiveNow').onclick = () => {
    triggerLiveFrame();
    uploadMsg.innerText = '✅ Sent to LyMini screen!';
  };

  const busyModal = document.getElementById('busyModal');
  const busyTitle = document.getElementById('busyTitle');
  const busyStep  = document.getElementById('busyStep');
  const busyBar   = document.getElementById('busyBar');
  const busyPct   = document.getElementById('busyPct');

  function showBusyModal(title, step, pct) {
    busyModal.classList.add('show');
    busyTitle.innerText = title;
    busyStep.innerText  = step;
    busyBar.style.width = pct + '%';
    busyPct.innerText   = Math.round(pct) + '%';
  }

  function hideBusyModal() {
    busyModal.classList.remove('show');
  }

  document.getElementById('btnUploadMedia').onclick = async () => {
    if (mediaType === 'none') {
      uploadMsg.innerText = (curLang === 'th') ? '⚠️ กรุณาเลือกรูปหรือวิดีโอก่อนครับ' : '⚠️ Please select a photo or video first';
      return;
    }

    isUploadingMedia = true;
    liveQueued = false;
    if (animReq) cancelAnimationFrame(animReq);

    showBusyModal(
      (curLang === 'th') ? '⏳ กำลังดำเนินการบันทึก...' : '⏳ Processing & Saving...',
      (curLang === 'th') ? 'กำลังเตรียมการเชื่อมต่อกับบอร์ด...' : 'Preparing upload...',
      5
    );

    // Wait for any in-flight liveframe request to finish first so ESP32 WebServer is free
    let waitTicks = 0;
    while (liveBusy && waitTicks < 30) {
      await new Promise(r => setTimeout(r, 40));
      waitTicks++;
    }

    try {
      if (mediaType === 'image') {
        showBusyModal(
          (curLang === 'th') ? '🖼️ กำลังบันทึกรูปภาพ...' : '🖼️ Saving Photo...',
          (curLang === 'th') ? 'กำลังส่งข้อมูลภาพ 128×128 ลงบอร์ด...' : 'Uploading 128×128 RGB565...',
          45
        );
        const buf = canvasToRgb565();
        const fname = 'img_' + (Date.now() % 100000) + '.rgb';
        const fd = new FormData();
        fd.append('file', new Blob([buf], { type: 'application/octet-stream' }), fname);
        const res = await fetch('/api/upload?folder=photos', { method: 'POST', body: fd });
        if (res.ok) {
          showBusyModal(
            (curLang === 'th') ? '✅ บันทึกรูปภาพสำเร็จ!' : '✅ Photo Saved!',
            (curLang === 'th') ? 'กำลังแสดงผลบนหน้าจอ LyMini' : 'Displayed on LyMini screen!',
            100
          );
          uploadMsg.innerText = (curLang === 'th') ? '✅ บันทึกรูปภาพและแสดงบนหน้าจอเรียบร้อย!' : '✅ Uploaded & displayed on LyMini!';
          await new Promise(r => setTimeout(r, 700));
          loadStatusAndMedia();
        } else {
          throw new Error('Upload failed');
        }
      } else if (mediaType === 'video') {
        const qualityEl = document.getElementById('selVidQuality');
        const vidFormat = qualityEl ? parseInt(qualityEl.value, 10) : 0; // 0 = HD 16-bit RGB565 (default), 1 = Compact 8-bit RGB332
        const frames = (vidFormat === 0) ? 10 : 12;
        const fps = 9;
        const fname = 'vid_' + (Date.now() % 100000) + '.mvid';

        showBusyModal(
          (curLang === 'th') ? '🎬 กำลังประมวลผลวิดีโอ HD (16-bit)...' : '🎬 Processing HD Video (16-bit)...',
          (curLang === 'th') ? 'กำลังเตรียมพื้นที่จัดเก็บในบอร์ด...' : 'Initializing video file on board...',
          8
        );

        const initRes = await fetch(`/api/upload_init?name=${encodeURIComponent(fname)}&frames=${frames}&fps=${fps}&format=${vidFormat}`, { method: 'POST' });
        if (!initRes.ok) throw new Error('Init failed');

        // Ensure video is playing so frames advance naturally on iOS/Android/PC
        await srcVideo.play().catch(() => {});

        const rawDur = (srcVideo.duration && isFinite(srcVideo.duration) && srcVideo.duration > 0.2) ? srcVideo.duration : 2.0;
        const dur = Math.min(Math.max(rawDur, 0.8), 2.8);
        const stepMs = Math.max(95, Math.floor((dur * 1000) / frames));

        for (let f = 0; f < frames; f++) {
          const pct = 10 + Math.round(((f + 1) / frames) * 82);
          showBusyModal(
            (curLang === 'th') ? '🎬 กำลังประมวลผลและบันทึกวิดีโอ HD...' : '🎬 Saving HD Video to LyMini...',
            (curLang === 'th')
              ? `กำลังสกัดเฟรมที่ ${f + 1} / ${frames} (${vidFormat === 0 ? '16-bit RGB565 คมชัดสูง' : '8-bit RGB332'})...`
              : `Capturing frame ${f + 1} / ${frames} (${vidFormat === 0 ? '16-bit HD' : '8-bit'})...`,
            pct
          );

          if (srcVideo.paused) {
            try { srcVideo.currentTime = Math.min((f * dur) / frames + 0.02, rawDur - 0.01); } catch (e) {}
          }
          await new Promise(r => setTimeout(r, stepMs));

          renderSourceToCanvas(srcVideo, srcVideo.videoWidth || 128, srcVideo.videoHeight || 128);
          const fbuf = (vidFormat === 0) ? canvasToRgb565() : canvasToRgb332();
          const fd = new FormData();
          fd.append('frame', new Blob([fbuf], { type: 'application/octet-stream' }), `f${f}.bin`);
          const fRes = await fetch(`/api/upload_frame?idx=${f}`, { method: 'POST', body: fd });
          if (!fRes.ok) throw new Error('Frame upload failed');
        }

        showBusyModal(
          (curLang === 'th') ? '🚀 กำลังเปิดเล่นวิดีโอบนหน้าจอ...' : '🚀 Finalizing Video...',
          (curLang === 'th') ? 'กำลังบันทึกไฟล์และแสดงผลบน LyMini...' : 'Flushing to flash & starting playback...',
          96
        );

        await fetch('/api/upload_finish', { method: 'POST' });

        showBusyModal(
          (curLang === 'th') ? '✅ บันทึกวิดีโอสำเร็จ!' : '✅ Video Saved!',
          (curLang === 'th') ? 'กำลังเล่นวิดีโอระดับ HD 16-bit บนจอ LyMini' : 'Playing HD 16-bit video on LyMini!',
          100
        );

        uploadMsg.innerText = (curLang === 'th')
          ? '✅ บันทึกวิดีโอสำเร็จ! กำลังเล่นภาพเคลื่อนไหวบนจอ LyMini'
          : '✅ Video saved & playing on LyMini!';

        await new Promise(r => setTimeout(r, 800));
        loadStatusAndMedia();
        // Resume preview loop in browser
        srcVideo.play().then(() => videoLoop()).catch(() => {});
      }
    } catch (err) {
      uploadMsg.innerText = (curLang === 'th')
        ? '❌ เกิดข้อผิดพลาดในการบันทึก กรุณาลองใหม่อีกครั้ง'
        : '❌ Save failed, please try again';
      alert(uploadMsg.innerText);
    } finally {
      hideBusyModal();
      isUploadingMedia = false;
    }
  };

  // --- Render PromptPay QR in About Tab ---
  let qrLoaded = false;
  async function loadQrCanvas() {
    if (qrLoaded) return;
    try {
      const r = await fetch('/api/qr.bin');
      const buf = new Uint8Array(await r.arrayBuffer());
      const qcv = document.getElementById('qrCanvas');
      const qctx = qcv.getContext('2d');
      const imgData = qctx.createImageData(112, 112);
      for (let i = 0; i < 112 * 112; i++) {
        const c = buf[i];
        const r3 = (c >> 5) & 0x07;
        const g3 = (c >> 2) & 0x07;
        const b2 = c & 0x03;
        imgData.data[i * 4 + 0] = (r3 * 255) / 7;
        imgData.data[i * 4 + 1] = (g3 * 255) / 7;
        imgData.data[i * 4 + 2] = (b2 * 255) / 3;
        imgData.data[i * 4 + 3] = 255;
      }
      qctx.putImageData(imgData, 0, 0);
      qrLoaded = true;
    } catch (e) {}
  }

  document.getElementById('btnShowQrEsp').onclick = async () => {
    await fetch('/api/show_qr', { method: 'POST' });
  };

  async function loadStatusAndMedia() {
    try {
      const r = await fetch('/api/status');
      const st = await r.json();
      document.getElementById('battBadge').innerText = (st.usb ? '⚡ USB ' : '🔋 ') + st.batt + '%';
      document.getElementById('statusText').innerHTML = `IP: <b>${st.ip}</b> | <b>http://lymini.local</b>`;
      document.getElementById('fsInfo').innerText = `${st.fsUsedKB} / ${st.fsTotalKB} KB`;

      document.getElementById('chkClock').checked      = (st.idleMask & 1) !== 0;
      document.getElementById('chkWeather').checked    = (st.idleMask & 2) !== 0;
      document.getElementById('chkPhoto').checked      = (st.idleMask & 4) !== 0;
      document.getElementById('chkVideo').checked      = (st.idleMask & 8) !== 0;
      document.getElementById('chkAnalog').checked     = (st.idleMask & 16) !== 0;
      document.getElementById('chkFlip').checked       = (st.idleMask & 32) !== 0;
      document.getElementById('chkAboutQr').checked    = true; // Always checked & locked
      document.getElementById('chkClkWeather').checked = (st.idleMask & 128) !== 0;

      document.getElementById('chkRandom').checked      = st.idleRandom === 1;
      document.getElementById('chkEcoVideo').checked    = st.ecoVideo === 1;
      document.getElementById('inpIdleTime').value      = st.idleTimeout;
      if (st.ecoTimeout !== undefined) {
        document.getElementById('inpEcoTime').value     = st.ecoTimeout;
      }
      if (st.sleepTimeout !== undefined) {
        document.getElementById('inpSleepTime').value   = Math.round(st.sleepTimeout / 60);
      }
      document.getElementById('inpIdleCycle').value     = st.idleCycle;

      rngBright.value = st.bright;
      briBadge.innerText = st.bright + '%';
      if (st.idleBright !== undefined) {
        rngIdleBright.value = st.idleBright;
        idleBriBadge.innerText = st.idleBright + '%';
      }
      if (st.ecoBright !== undefined) {
        rngEcoBright.value = st.ecoBright;
        ecoBriBadge.innerText = st.ecoBright + '%';
      }
      if (st.version) {
        document.getElementById('curFwVer').innerText = 'v' + st.version;
        document.getElementById('topFwVer').innerText = 'v' + st.version;
        document.title = 'LyMini Arcade & Smart TV v' + st.version;
        const ab = document.getElementById('abDescText');
        if (ab) ab.innerText = 'LyMini ESP32-C3 1.44" Retro Arcade & Smart Idle TV v' + st.version + ' by Zotix';
      }

      document.getElementById('selRotation').value   = st.rot;
      document.getElementById('chkBtnRot').checked   = st.btnRot === 1;
      document.getElementById('selColorBgr').value   = st.bgr;
      document.getElementById('chkNightEn').checked  = st.nightEn === 1;
      document.getElementById('inpNightStart').value = st.nightStart;
      document.getElementById('inpNightEnd').value   = st.nightEnd;

      // Multi-location weather cities
      const cBox = document.getElementById('cityListBox');
      cBox.innerHTML = '';
      (st.cities || []).forEach((c, idx) => {
        const div = document.createElement('div');
        div.className = 'wifi-item';
        const info = c.valid ? `${c.temp}°C (${c.hum}%)` : '⏳ Syncing...';
        const delBtn = (st.cities.length > 1) ? `<button class="sm red" onclick="delCity(${idx})">🗑️</button>` : '';
        div.innerHTML = `
          <span style="flex:1;">🌍 <b>${c.name}</b> — <span style="color:var(--accent)">${info}</span></span>
          <button class="sm purple" onclick="previewIdle(7, ${idx})">👁️ Clock+WT</button>
          <button class="sm purple" onclick="previewIdle(1, ${idx})">👁️ WT</button>
          ${delBtn}
        `;
        cBox.appendChild(div);
      });

      const wBox = document.getElementById('savedWifiList');
      wBox.innerHTML = '';
      (st.wifiList || []).forEach((w, idx) => {
        const div = document.createElement('div');
        div.className = 'wifi-item';
        div.innerHTML = `<span>📶 <b>${w}</b></span>
          <button class="sm red" onclick="delWifi(${idx})">🗑️</button>`;
        wBox.appendChild(div);
      });

      const mBox = document.getElementById('mediaList');
      mBox.innerHTML = '';
      (st.media || []).forEach(item => {
        const row = document.createElement('div');
        row.className = 'media-row';
        const icon = item.path.endsWith('.mvid') ? '🎬' : '🖼️';
        row.innerHTML = `
          <input type="checkbox" ${item.enabled ? 'checked' : ''} onchange="toggleMedia('${item.name}', this.checked)" />
          <span class="media-name">${icon} ${item.name} <small style="color:#64748b">(${item.kb}KB)</small></span>
          <button class="sm purple" onclick="showOnEsp('${item.path}')">👁️</button>
          <button class="sm red" onclick="deleteMedia('${item.path}')">🗑️</button>
        `;
        mBox.appendChild(row);
      });
    } catch (e) {}
  }

  window.showOnEsp = async (path) => {
    await fetch('/api/show?path=' + encodeURIComponent(path), { method: 'POST' });
  };
  window.toggleMedia = async (name, en) => {
    await fetch(`/api/toggle_media?name=${encodeURIComponent(name)}&en=${en ? 1 : 0}`, { method: 'POST' });
  };
  window.deleteMedia = async (path) => {
    if (!confirm('Delete ' + path + ' ?')) return;
    await fetch('/api/delete_media?path=' + encodeURIComponent(path), { method: 'POST' });
    loadStatusAndMedia();
  };
  window.delWifi = async (idx) => {
    await fetch('/api/del_wifi?idx=' + idx, { method: 'POST' });
    loadStatusAndMedia();
  };
  window.delCity = async (idx) => {
    await fetch('/api/del_city?idx=' + idx, { method: 'POST' });
    loadStatusAndMedia();
  };

  document.getElementById('btnAddCity').onclick = async () => {
    const c = document.getElementById('inpNewCity').value.trim();
    if (!c) return;
    document.getElementById('inpNewCity').value = '';
    await fetch('/api/add_city?city=' + encodeURIComponent(c), { method: 'POST' });
    loadStatusAndMedia();
  };

  let toastTimer = null;
  function showToast(msg) {
    const t = document.getElementById('autoSaveToast');
    if (!t) return;
    t.innerText = msg;
    t.classList.add('show');
    clearTimeout(toastTimer);
    toastTimer = setTimeout(() => t.classList.remove('show'), 1800);
  }

  window.saveAllSettings = async (silent = false) => {
    // Bit 6 (64 = PromptPay QR) is always forced ON!
    let mask = 64;
    if (document.getElementById('chkClock').checked)      mask |= 1;
    if (document.getElementById('chkWeather').checked)    mask |= 2;
    if (document.getElementById('chkPhoto').checked)      mask |= 4;
    if (document.getElementById('chkVideo').checked)      mask |= 8;
    if (document.getElementById('chkAnalog').checked)     mask |= 16;
    if (document.getElementById('chkFlip').checked)       mask |= 32;
    if (document.getElementById('chkClkWeather').checked) mask |= 128;

    const params = new URLSearchParams({
      idlemask: mask,
      idlernd: document.getElementById('chkRandom').checked ? 1 : 0,
      ecovid: document.getElementById('chkEcoVideo').checked ? 1 : 0,
      idletime: document.getElementById('inpIdleTime').value,
      ecotime: document.getElementById('inpEcoTime').value,
      sleeptime: Math.round(Number(document.getElementById('inpSleepTime').value || 0) * 60),
      idlecyc: document.getElementById('inpIdleCycle').value,
      bright: rngBright.value,
      idlebri: rngIdleBright.value,
      ecobri: rngEcoBright.value,
      rot: document.getElementById('selRotation').value,
      btnrot: document.getElementById('chkBtnRot').checked ? 1 : 0,
      bgr: document.getElementById('selColorBgr').value,
      night_en: document.getElementById('chkNightEn').checked ? 1 : 0,
      night_st: document.getElementById('inpNightStart').value,
      night_ed: document.getElementById('inpNightEnd').value
    });
    showToast(curLang === 'th' ? '⏳ กำลังบันทึก...' : '⏳ Saving...');
    try {
      await fetch('/api/settings?' + params.toString(), { method: 'POST' });
      showToast(curLang === 'th' ? '✅ บันทึกอัตโนมัติแล้ว' : '✅ Auto-saved');
    } catch (e) {
      showToast(curLang === 'th' ? '❌ บันทึกไม่สำเร็จ' : '❌ Save failed');
    }
    loadStatusAndMedia();
  };

  let autoSaveTimer = null;
  function scheduleAutoSave(delayMs) {
    clearTimeout(autoSaveTimer);
    autoSaveTimer = setTimeout(() => saveAllSettings(true), delayMs);
  }
  ['chkClock','chkWeather','chkPhoto','chkVideo','chkAnalog','chkFlip','chkClkWeather',
   'chkRandom','chkEcoVideo','selRotation','chkBtnRot','selColorBgr','chkNightEn'].forEach(id => {
    const el = document.getElementById(id);
    if (el) el.addEventListener('change', () => scheduleAutoSave(80));
  });
  ['inpIdleTime','inpEcoTime','inpSleepTime','inpIdleCycle','inpNightStart','inpNightEnd'].forEach(id => {
    const el = document.getElementById(id);
    if (el) {
      el.addEventListener('input', () => scheduleAutoSave(450));
      el.addEventListener('change', () => scheduleAutoSave(80));
    }
  });

  document.getElementById('btnRefreshWeather').onclick = async () => {
    await fetch('/api/refresh_weather', { method: 'POST' });
    setTimeout(loadStatusAndMedia, 1200);
  };

  document.getElementById('btnScanWifi').onclick = async () => {
    const box = document.getElementById('scanResults');
    box.innerHTML = '<div class="hint">Scanning Wi-Fi...</div>';
    const r = await fetch('/api/scan_wifi');
    const nets = await r.json();
    box.innerHTML = '';
    nets.forEach(n => {
      const d = document.createElement('div');
      d.className = 'wifi-item';
      d.style.cursor = 'pointer';
      d.innerHTML = `<span>📡 <b>${n.ssid}</b> (${n.rssi} dBm)</span><span class="badge">Select</span>`;
      d.onclick = () => { document.getElementById('inpSsid').value = n.ssid; };
      box.appendChild(d);
    });
  };

  document.getElementById('btnSaveWifi').onclick = async () => {
    const ssid = document.getElementById('inpSsid').value.trim();
    const pass = document.getElementById('inpPass').value;
    if (!ssid) return;
    const params = new URLSearchParams({ ssid, pass });
    await fetch('/api/save_wifi?' + params.toString(), { method: 'POST' });
    alert(curLang === 'th' ? '✅ บันทึก Wi-Fi เรียบร้อยแล้ว!' : '✅ Wi-Fi saved!');
    loadStatusAndMedia();
  };

  // --- OTA Check & Online Update ---
  document.getElementById('btnCheckOta').onclick = async () => {
    const msg = document.getElementById('otaOnlineMsg');
    msg.innerText = '🔍 Checking GitHub Releases...';
    try {
      const r = await fetch('/api/ota_check');
      const d = await r.json();
      if (!d.ok) {
        msg.innerText = '❌ Check failed: ' + (d.error || 'Unknown error');
      } else if (d.hasUpdate) {
        msg.innerText = `✨ New version v${d.latest} available! (Current: v${d.current})`;
        if (d.binUrl) document.getElementById('inpOtaUrl').value = d.binUrl;
      } else {
        msg.innerText = `✅ Up to date (Current: v${d.current}, Latest: v${d.latest})`;
      }
    } catch (e) {
      msg.innerText = '❌ Network error checking OTA';
    }
  };

  document.getElementById('btnRunOnlineOta').onclick = async () => {
    const msg = document.getElementById('otaOnlineMsg');
    const url = document.getElementById('inpOtaUrl').value.trim();
    if (!confirm('Start online OTA update? Device will reboot automatically when done.')) return;
    msg.innerText = '⏳ Downloading & flashing firmware... Do not power off!';
    try {
      const r = await fetch('/api/ota_online' + (url ? ('?url=' + encodeURIComponent(url)) : ''), { method: 'POST' });
      const d = await r.json();
      if (d.ok) {
        msg.innerText = '✅ Update complete! Rebooting...';
      } else {
        msg.innerText = '❌ OTA failed: ' + (d.error || 'Error');
      }
    } catch (e) {
      msg.innerText = '🔄 Rebooting or connection closed...';
    }
  };

  // --- Manual OTA .bin Upload ---
  document.getElementById('btnUploadOta').onclick = () => {
    const file = document.getElementById('otaFileInput').files[0];
    const msg = document.getElementById('otaUploadMsg');
    if (!file) { msg.innerText = '⚠️ Select a .bin firmware file first.'; return; }
    if (!confirm('Flash ' + file.name + ' (' + Math.round(file.size / 1024) + ' KB)?')) return;
    const xhr = new XMLHttpRequest();
    xhr.open('POST', '/api/ota');
    xhr.upload.onprogress = (ev) => {
      if (ev.lengthComputable) {
        const pct = Math.round((ev.loaded * 100) / ev.total);
        msg.innerText = `⏳ Uploading & Flashing: ${pct}%`;
      }
    };
    xhr.onload = () => {
      msg.innerText = (xhr.status === 200) ? '✅ OTA Complete! Rebooting...' : '❌ OTA Failed!';
    };
    xhr.onerror = () => { msg.innerText = '❌ Upload error!'; };
    const fd = new FormData();
    fd.append('firmware', file);
    xhr.send(fd);
  };

  applyLang();
  loadStatusAndMedia();
</script>
</body>
</html>
)rawliteral";
