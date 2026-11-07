#pragma once
#include <Arduino.h>

const char PAGE_INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>Spotpear Mini Arcade & TV v0.2.1</title>
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
  * { box-sizing: border-box; }
  body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 10px; }
  .container { max-width: 720px; margin: 0 auto; }
  h1 { font-size: 18px; margin: 0 0 4px 0; color: #fff; }
  h2 { font-size: 15px; margin: 0 0 10px 0; color: var(--accent); }
  .topbar { display: flex; flex-wrap: wrap; gap: 8px; align-items: center; justify-content: space-between; margin-bottom: 10px; padding: 10px 12px; background: var(--card); border: 1px solid var(--border); border-radius: 10px; }
  .badge { display: inline-block; padding: 3px 8px; border-radius: 12px; font-size: 11px; font-weight: 600; background: #1e293b; color: var(--accent); }
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
  button {
    background: var(--accent); color: #090d14; border: none; padding: 10px 14px; border-radius: 6px;
    font-weight: 700; cursor: pointer; margin-top: 8px; width: 100%; font-size: 14px;
  }
  button.green { background: var(--green); color: #fff; }
  button.purple { background: var(--purple); color: #fff; }
  button.red { background: var(--red); color: #fff; }
  button.gold { background: var(--gold); color: #090d14; }
  button.sm { width: auto; padding: 6px 10px; margin: 0; font-size: 12px; }
  .row { display: flex; gap: 8px; align-items: center; margin-top: 8px; }
  .row > * { flex: 1; }
  .cropper-wrap { display: flex; flex-direction: column; align-items: center; background: #080b11; border: 1px dashed var(--border); border-radius: 8px; padding: 12px; margin-top: 10px; position: relative; }
  canvas.preview-cv { border: 2px solid var(--accent); border-radius: 6px; width: 256px; height: 256px; image-rendering: pixelated; background: #000; touch-action: none; cursor: grab; }
  .chk-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; margin-top: 8px; }
  .chk-item { display: flex; align-items: center; gap: 8px; background: #0b0f17; padding: 8px 10px; border-radius: 6px; border: 1px solid var(--border); font-size: 13px; color: #fff; }
  .chk-item input { width: 18px; height: 18px; margin: 0; }
  .media-list { margin-top: 10px; display: flex; flex-direction: column; gap: 6px; }
  .media-row { display: flex; align-items: center; justify-content: space-between; gap: 8px; background: #0b0f17; padding: 8px 10px; border-radius: 6px; border: 1px solid var(--border); font-size: 13px; }
  .media-name { flex: 1; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; color: #fff; }
  .wifi-item { display: flex; justify-content: space-between; align-items: center; background: #0b0f17; padding: 7px 10px; border-radius: 6px; margin-top: 6px; border: 1px solid var(--border); font-size: 13px; }
  .hint { font-size: 12px; color: var(--muted); margin: 4px 0 0 0; }
  /* Off-screen video element so iOS Safari WebKit hardware decoder actually renders frames! */
  #srcVideo { position: fixed; left: -9999px; top: 0; width: 64px; height: 64px; opacity: 0.01; pointer-events: none; z-index: -1; }
</style>
</head>
<body>
<div class="container">
  <div class="topbar">
    <div>
      <h1>🕹️ Spotpear Mini Arcade & TV <span class="badge">v0.2.1</span></h1>
      <div class="hint" id="statusText">IP: <b>192.168.4.1</b> | <b>http://mini.local</b></div>
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
    <button type="button" class="tab-btn" onclick="switchTab(2)" data-i18n="tab_display">3. 🖥️ หน้าจอ & กลางคืน</button>
    <button type="button" class="tab-btn" onclick="switchTab(3)" data-i18n="tab_wifi">4. 📶 Wi-Fi & อากาศ</button>
    <button type="button" class="tab-btn" onclick="switchTab(4)" data-i18n="tab_about">5. ☕ เกี่ยวกับ & กาแฟ</button>
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
          <input type="checkbox" id="chkAutoLive" checked />
          <span data-i18n="st_autolive">👁️ Live View บนจอ ESP32 อัตโนมัติ ขณะลาก/ปรับแต่งภาพ</span>
        </div>
        <div class="row" style="width:100%;">
          <button type="button" class="purple" id="btnLiveNow" data-i18n="st_btn_live">👁️ ส่งขึ้นจอ ESP ทันที</button>
          <button type="button" class="green" id="btnUploadMedia" data-i18n="st_btn_save">🚀 บันทึกลงเครื่อง & แสดงผล</button>
        </div>
        <div id="uploadMsg" class="hint" style="margin-top:8px; color:var(--gold); font-weight:600;"></div>
      </div>
    </div>
  </div>

  <!-- TAB 1: Idle Mode & Media Gallery -->
  <div class="tab-pane" id="tab1">
    <div class="card">
      <h2 data-i18n="id_title">🎲 ตั้งค่าโหมดพักหน้าจอ (Smart Idle Display)</h2>
      <p class="hint" data-i18n="id_smart_hint">✨ ระบบอัจฉริยะ: หากยังไม่ได้ซิงค์เวลาจะข้ามหน้านาฬิกาอัตโนมัติ และหากยังไม่มีข้อมูลสภาพอากาศจะข้ามหน้าสภาพอากาศอัตโนมัติ</p>
      <label data-i18n="id_select">เลือกหน้าจอที่จะแสดงเมื่อพักหน้าจอ (เปิด/ปิดแยกอิสระได้):</label>
      <div class="chk-grid">
        <label class="chk-item"><input type="checkbox" id="chkClock" checked /> <span data-i18n="id_clk_dig">🕒 นาฬิกาดิจิทัล (Digital)</span></label>
        <label class="chk-item"><input type="checkbox" id="chkAnalog" checked /> <span data-i18n="id_clk_ana">🕰️ นาฬิกาเข็ม (Analog)</span></label>
        <label class="chk-item"><input type="checkbox" id="chkFlip" checked /> <span data-i18n="id_clk_flp">🔢 นาฬิกาพลิก (Flip Clock)</span></label>
        <label class="chk-item"><input type="checkbox" id="chkWeather" checked /> <span data-i18n="id_weather">🌡️ สภาพอากาศ (Weather)</span></label>
        <label class="chk-item"><input type="checkbox" id="chkPhoto" checked /> <span data-i18n="id_photo">🖼️ รูปภาพ (Photos)</span></label>
        <label class="chk-item"><input type="checkbox" id="chkVideo" checked /> <span data-i18n="id_video">🎬 วิดีโอ (Videos)</span></label>
        <label class="chk-item" style="grid-column: span 2;"><input type="checkbox" id="chkAboutQr" checked /> <span data-i18n="id_qr">☕ Buy Me a Coffee (PromptPay QR Code)</span></label>
      </div>

      <div class="chk-item" style="margin-top:8px; border-color:var(--gold);">
        <input type="checkbox" id="chkLockQr" />
        <span style="color:var(--gold); font-weight:700;" data-i18n="id_lock_qr">🔒 ล็อกหน้าจอ Idle ให้แสดงเฉพาะ PromptPay QR Code ตลอดเวลา</span>
      </div>

      <div class="chk-item" style="margin-top:8px;">
        <input type="checkbox" id="chkRandom" checked />
        <span data-i18n="id_random">🔀 Random Mode (สุ่มลำดับหน้าจอ และสุ่มรูปภาพ/วิดีโอ)</span>
      </div>

      <div class="row">
        <div>
          <label data-i18n="id_timeout">เข้า Idle เมื่อไม่กดปุ่ม (วินาที):</label>
          <input type="number" id="inpIdleTime" min="15" max="600" value="60" />
        </div>
        <div>
          <label data-i18n="id_cycle">สลับหน้าทุกๆ (วินาที):</label>
          <input type="number" id="inpIdleCycle" min="5" max="120" value="12" />
        </div>
      </div>
      <button type="button" class="green" onclick="saveAllSettings()" data-i18n="btn_save">💾 บันทึกการตั้งค่า</button>
    </div>

    <div class="card">
      <div style="display:flex; justify-content:space-between; align-items:center;">
        <h2 style="margin:0;" data-i18n="gal_title">📂 คลังภาพและวิดีโอในบอร์ด (LittleFS)</h2>
        <span id="fsInfo" class="badge">-- KB</span>
      </div>
      <p class="hint" data-i18n="gal_hint">ติ๊กถูกเพื่อเปิดแสดงในรอบ Idle หรือกด 👁️ เพื่อยิงขึ้นจอ ESP32 ทันที</p>
      <div id="mediaList" class="media-list">Loading...</div>
    </div>
  </div>

  <!-- TAB 2: Display Rotation, Color Fix & Night Schedule -->
  <div class="tab-pane" id="tab2">
    <div class="card">
      <h2 data-i18n="dp_title">🖥️ ตั้งค่าหน้าจอ การหมุนจอ และโหมดกลางคืน</h2>
      <div class="row">
        <div>
          <label data-i18n="dp_rot">การหมุนหน้าจอ (Screen Rotation):</label>
          <select id="selRotation">
            <option value="0">0° (Portrait)</option>
            <option value="1" selected>90° (Landscape - ค่าเริ่มต้น)</option>
            <option value="2">180° (Inverted Portrait)</option>
            <option value="3">270° (Inverted Landscape)</option>
          </select>
        </div>
        <div>
          <label data-i18n="dp_color">ลำดับสีจอ (Color Order):</label>
          <select id="selColorBgr">
            <option value="1" selected>BGR (มาตรฐาน ST7735 แก้สีแดง/ฟ้า)</option>
            <option value="0">RGB (ทางเลือก)</option>
          </select>
        </div>
      </div>

      <label data-i18n="dp_bright">ความสว่างจอปกติ (10% - 100%):</label>
      <input type="number" id="inpBright" min="10" max="100" value="85" />

      <hr style="border:none; border-top:1px solid var(--border); margin:14px 0;">

      <h2 data-i18n="nt_title">🌙 ตั้งเวลาปิดไฟหน้าจอกลางคืน (Night Mode Screen-Off)</h2>
      <p class="hint" data-i18n="nt_desc">เมื่ออยู่ในโหมดพักหน้าจอ (Idle) และถึงช่วงเวลาที่กำหนด หน้าจอจะปิดไฟ Backlight (0%) อัตโนมัติ และตื่นทันทีเมื่อกดปุ่มใดๆ</p>
      <div class="chk-item" style="margin-top:8px;">
        <input type="checkbox" id="chkNightEn" />
        <span data-i18n="nt_enable">🌙 เปิดใช้งานตั้งเวลาปิดหน้าจอกลางคืน</span>
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

      <button type="button" class="green" onclick="saveAllSettings()" data-i18n="btn_save">💾 บันทึกการตั้งค่า</button>
    </div>
  </div>

  <!-- TAB 3: Wi-Fi & Weather -->
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
      <h2 data-i18n="wt_title">🌡️ ตั้งค่าเมืองสภาพอากาศ (Open-Meteo Weather)</h2>
      <div id="weatherStatusBox" class="hint" style="margin-bottom:8px; color:var(--accent); font-weight:600;"></div>
      <label data-i18n="wt_city">ชื่อเมือง (ภาษาอังกฤษ เช่น Bangkok, Chiang Mai, Tokyo):</label>
      <input type="text" id="inpCity" value="Bangkok" />
      <div class="row">
        <button type="button" class="green" onclick="saveAllSettings()" data-i18n="btn_save">💾 บันทึกการตั้งค่า</button>
        <button type="button" class="purple" id="btnRefreshWeather" data-i18n="wt_refresh">🔄 ดึงข้อมูลอากาศเดี๋ยวนี้</button>
      </div>
    </div>
  </div>

  <!-- TAB 4: About Me & Buy Me a Coffee (PromptPay QR) -->
  <div class="tab-pane" id="tab4">
    <div class="card" style="text-align:center;">
      <h2 data-i18n="ab_title">☕ เกี่ยวกับโปรเจกต์ & เลี้ยงกาแฟผู้พัฒนา (PromptPay QR)</h2>
      <p class="hint" data-i18n="ab_desc">Spotpear ESP32-C3 1.44" Mini Arcade & Smart Idle TV v0.2.1 (9 เกมเรโทร + ระบบจัดการพลังงาน + Live Media Studio)</p>

      <div style="margin: 14px auto; display:inline-block; padding:12px; background:#fff; border-radius:12px;">
        <canvas id="qrCanvas" width="112" height="112" style="width:224px; height:224px; image-rendering:pixelated; display:block;"></canvas>
      </div>
      <div style="font-weight:700; color:var(--gold); margin-bottom:8px;" data-i18n="ab_scan">📱 สแกน PromptPay QR เพื่อสนับสนุนค่ากาแฟผู้พัฒนา</div>

      <div class="chk-item" style="justify-content:center; margin:10px 0; border-color:var(--gold);">
        <input type="checkbox" id="chkLockQrAbout" onchange="document.getElementById('chkLockQr').checked=this.checked; saveAllSettings();" />
        <span style="color:var(--gold); font-weight:700;" data-i18n="id_lock_qr">🔒 ล็อกหน้าจอ Idle ให้แสดงเฉพาะ PromptPay QR Code ตลอดเวลา</span>
      </div>

      <div class="row">
        <button type="button" class="gold" id="btnShowQrEsp" data-i18n="ab_show_esp">👁️ แสดง QR Code บนจอ ESP32 ทันที</button>
      </div>
    </div>
  </div>
</div>

<script>
  // --- Bilingual Dictionary (TH / EN) ---
  let curLang = localStorage.getItem('mini_lang') || 'th';
  const i18n = {
    th: {
      tab_studio: '1. ✂️ สตูดิโอ',
      tab_idle: '2. 🎲 พักหน้าจอ & คลัง',
      tab_display: '3. 🖥️ หน้าจอ & กลางคืน',
      tab_wifi: '4. 📶 Wi-Fi & อากาศ',
      tab_about: '5. ☕ เกี่ยวกับ & กาแฟ',
      st_title: '✂️ 128×128 Media Studio (iOS / Android / PC + Live View)',
      st_desc: 'รองรับรูปภาพ (.JPG, .PNG, .WEBP, .HEIC) และวิดีโอจาก iPhone/มือถือ (.MOV, .MP4) ลากนิ้วเพื่อเลื่อนกรอบ หรือถ่าง 2 นิ้วเพื่อซูม',
      st_pan_hint: '💡 ลากบนกรอบภาพเพื่อจัดตำแหน่ง (Pan) / ถ่าง 2 นิ้วเพื่อซูม',
      st_rot: 'หมุน 90°',
      st_reset: 'รีเซ็ต',
      st_autolive: '👁️ Live View บนจอ ESP32 อัตโนมัติ ขณะลาก/ปรับแต่งภาพ',
      st_btn_live: '👁️ ส่งขึ้นจอ ESP ทันที',
      st_btn_save: '🚀 บันทึกลงเครื่อง & แสดงผล',
      id_title: '🎲 ตั้งค่าโหมดพักหน้าจอ (Smart Idle Display)',
      id_smart_hint: '✨ ระบบอัจฉริยะ: หากไม่มีข้อมูลเวลาจะข้ามหน้านาฬิกา และหากไม่มีข้อมูลอากาศจะข้ามหน้าสภาพอากาศอัตโนมัติ',
      id_select: 'เลือกหน้าจอที่จะแสดงเมื่อพักหน้าจอ (เปิด/ปิดแยกอิสระได้):',
      id_clk_dig: '🕒 นาฬิกาดิจิทัล (Digital)',
      id_clk_ana: '🕰️ นาฬิกาเข็ม (Analog)',
      id_clk_flp: '🔢 นาฬิกาพลิก (Flip Clock)',
      id_weather: '🌡️ สภาพอากาศ (Weather)',
      id_photo: '🖼️ รูปภาพ (Photos)',
      id_video: '🎬 วิดีโอ (Videos)',
      id_qr: '☕ Buy Me a Coffee (PromptPay QR Code)',
      id_lock_qr: '🔒 ล็อกหน้าจอ Idle ให้แสดงเฉพาะ PromptPay QR Code ตลอดเวลา',
      id_random: '🔀 Random Mode (สุ่มลำดับหน้าจอ และสุ่มรูปภาพ/วิดีโอ)',
      id_timeout: 'เข้า Idle เมื่อไม่กดปุ่ม (วินาที):',
      id_cycle: 'สลับหน้าทุกๆ (วินาที):',
      gal_title: '📂 คลังภาพและวิดีโอในบอร์ด (LittleFS)',
      gal_hint: 'ติ๊กถูกเพื่อเปิดแสดงในรอบ Idle หรือกด 👁️ เพื่อยิงขึ้นจอ ESP32 ทันที',
      dp_title: '🖥️ ตั้งค่าหน้าจอ การหมุนจอ และโหมดกลางคืน',
      dp_rot: 'การหมุนหน้าจอ (Screen Rotation):',
      dp_color: 'ลำดับสีจอ (Color Order):',
      dp_bright: 'ความสว่างจอปกติ (10% - 100%):',
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
      wt_title: '🌡️ ตั้งค่าเมืองสภาพอากาศ (Open-Meteo Weather)',
      wt_city: 'ชื่อเมือง (ภาษาอังกฤษ เช่น Bangkok, Chiang Mai, Tokyo):',
      wt_refresh: '🔄 ดึงข้อมูลอากาศเดี๋ยวนี้',
      ab_title: '☕ เกี่ยวกับโปรเจกต์ & เลี้ยงกาแฟผู้พัฒนา (PromptPay QR)',
      ab_desc: 'Spotpear ESP32-C3 1.44" Mini Arcade & Smart Idle TV v0.2.1 (9 เกมเรโทร + ระบบประหยัดพลังงาน + Live Studio)',
      ab_scan: '📱 สแกน PromptPay QR เพื่อสนับสนุนค่ากาแฟผู้พัฒนา',
      ab_show_esp: '👁️ แสดง QR Code บนจอ ESP32 ทันที',
      btn_save: '💾 บันทึกการตั้งค่า'
    },
    en: {
      tab_studio: '1. ✂️ Studio',
      tab_idle: '2. 🎲 Idle & Gallery',
      tab_display: '3. 🖥️ Display & Night',
      tab_wifi: '4. 📶 Wi-Fi & Weather',
      tab_about: '5. ☕ About & Coffee',
      st_title: '✂️ 128×128 Media Studio (iOS / Android / PC + Live View)',
      st_desc: 'Supports photos (.JPG, .PNG, .WEBP, .HEIC) and iPhone/mobile videos (.MOV, .MP4). Drag to pan or pinch to zoom.',
      st_pan_hint: '💡 Drag on canvas to pan / Pinch with 2 fingers to zoom',
      st_rot: 'Rotate 90°',
      st_reset: 'Reset',
      st_autolive: '👁️ Auto Live View on ESP32 while dragging/editing',
      st_btn_live: '👁️ Push to ESP Now',
      st_btn_save: '🚀 Save to Flash & Show',
      id_title: '🎲 Smart Idle Display Settings',
      id_smart_hint: '✨ Smart Filter: Clock screens are hidden until NTP time syncs; Weather screen is hidden until weather data is fetched.',
      id_select: 'Select screens to include in Idle rotation:',
      id_clk_dig: '🕒 Digital Clock',
      id_clk_ana: '🕰️ Analog Clock',
      id_clk_flp: '🔢 Flip Clock',
      id_weather: '🌡️ Weather & Temp',
      id_photo: '🖼️ Photo Frame',
      id_video: '🎬 Video / TV',
      id_qr: '☕ Buy Me a Coffee (PromptPay QR)',
      id_lock_qr: '🔒 Lock Idle Screen on PromptPay QR Code only',
      id_random: '🔀 Random Mode (Shuffle screens, photos & videos)',
      id_timeout: 'Enter Idle after inactivity (sec):',
      id_cycle: 'Cycle screen every (sec):',
      gal_title: '📂 On-Board Media Gallery (LittleFS)',
      gal_hint: 'Check to enable in Idle rotation, or click 👁️ to display immediately on ESP32',
      dp_title: '🖥️ Screen Rotation, Color & Brightness',
      dp_rot: 'Screen Rotation:',
      dp_color: 'Display Color Order:',
      dp_bright: 'Normal Brightness (10% - 100%):',
      nt_title: '🌙 Night Mode Screen-Off Schedule',
      nt_desc: 'Automatically turns off LCD backlight (0%) during scheduled night hours while in Idle mode. Press any button to wake.',
      nt_enable: '🌙 Enable Night Mode Screen-Off Schedule',
      nt_start: 'Turn Off Hour (0-23):',
      nt_end: 'Wake Up Hour (0-23):',
      wf_title: '📶 Persistent Multi-WiFi Setup (Up to 3 Networks)',
      wf_scan: '🔍 Scan Nearby Wi-Fi Networks',
      wf_ssid: 'Wi-Fi SSID:',
      wf_pass: 'Wi-Fi Password:',
      wf_save: '💾 Save Wi-Fi & Connect',
      wt_title: '🌡️ Weather Location (Open-Meteo)',
      wt_city: 'City Name (e.g. Bangkok, Chiang Mai, Tokyo):',
      wt_refresh: '🔄 Fetch Weather Now',
      ab_title: '☕ About Me & Buy Me a Coffee (PromptPay QR)',
      ab_desc: 'Spotpear ESP32-C3 1.44" Mini Arcade & Smart Idle TV v0.2.1 (9 Retro Games + Power Manager + Live Studio)',
      ab_scan: '📱 Scan PromptPay QR Code to Buy Me a Coffee',
      ab_show_esp: '👁️ Show QR Code on ESP32 Screen Now',
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
    localStorage.setItem('mini_lang', curLang);
    applyLang();
  };

  window.switchTab = (idx) => {
    document.querySelectorAll('.tab-btn').forEach((b, i) => b.classList.toggle('active', i === idx));
    document.querySelectorAll('.tab-pane').forEach((p, i) => p.classList.toggle('active', i === idx));
    if (idx === 4) loadQrCanvas();
  };

  // --- Studio Canvas & iOS Video Fix ---
  const canvas = document.getElementById('cropCanvas');
  const ctx = canvas.getContext('2d');
  const mediaInput = document.getElementById('mediaInput');
  const srcVideo = document.getElementById('srcVideo');
  const uploadMsg = document.getElementById('uploadMsg');

  let mediaType = 'none';
  let currentImg = null;
  let zoom = 1.0, rotation = 0, panX = 0, panY = 0;
  let isDragging = false, startX = 0, startY = 0, lastPinchDist = 0;
  let liveBusy = false, liveQueued = false;
  let animReq = null;

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

  function renderSourceToCanvas(source, sw, sh) {
    if (!sw || !sh) return;
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
  }

  function renderCanvas() {
    if (mediaType === 'image' && currentImg) {
      renderSourceToCanvas(currentImg, currentImg.width, currentImg.height);
      if (document.getElementById('chkAutoLive').checked) triggerLiveFrame();
    } else if (mediaType === 'video') {
      const vw = srcVideo.videoWidth || 128;
      const vh = srcVideo.videoHeight || 128;
      renderSourceToCanvas(srcVideo, vw, vh);
      if (document.getElementById('chkAutoLive').checked) triggerLiveFrame();
    }
  }

  function videoLoop() {
    if (mediaType === 'video' && !srcVideo.paused && !srcVideo.ended) {
      renderSourceToCanvas(srcVideo, srcVideo.videoWidth || 128, srcVideo.videoHeight || 128);
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
      uploadMsg.innerText = (curLang === 'th') ? '⏳ กำลังถอดรหัสวิดีโอ iOS/มือถือ...' : '⏳ Decoding video...';
      const url = URL.createObjectURL(file);
      srcVideo.src = url;
      srcVideo.muted = true;
      srcVideo.playsInline = true;
      srcVideo.load();

      // iOS Safari WebKit requires seeking to a non-zero timestamp or playing briefly to decode first frame!
      const onLoadedMeta = () => {
        srcVideo.removeEventListener('loadedmetadata', onLoadedMeta);
        const targetT = Math.min(0.1, (srcVideo.duration || 1.0) * 0.1);
        srcVideo.currentTime = targetT;
      };
      const onFirstSeek = () => {
        srcVideo.removeEventListener('seeked', onFirstSeek);
        renderCanvas();
        uploadMsg.innerText = (curLang === 'th') ? '🎬 โหลดเฟรมวิดีโอสำเร็จ! ปรับมุมกล้องแล้วกดบันทึกได้เลย' : '🎬 Video ready! Adjust crop & click Save.';
        srcVideo.play().then(() => {
          videoLoop();
        }).catch(() => {
          // Even if autoplay is blocked by Low Power Mode on iOS, seeked frame is already drawn on canvas!
          renderCanvas();
        });
      };
      srcVideo.addEventListener('loadedmetadata', onLoadedMeta);
      srcVideo.addEventListener('seeked', onFirstSeek);
    } else {
      mediaType = 'image';
      uploadMsg.innerText = (curLang === 'th') ? '⏳ กำลังโหลดรูปภาพ...' : '⏳ Loading image...';
      const url = URL.createObjectURL(file);
      const img = new Image();
      img.onload = () => {
        currentImg = img;
        renderCanvas();
        uploadMsg.innerText = (curLang === 'th') ? '🖼️ โหลดรูปสำเร็จ! ลากจัดตำแหน่งได้เลย' : '🖼️ Image ready!';
      };
      img.onerror = () => {
        uploadMsg.innerText = '⚠️ Error loading image format';
      };
      img.src = url;
    }
  });

  // Mouse & Touch Gestures
  canvas.addEventListener('mousedown', (e) => { isDragging = true; startX = e.clientX - panX; startY = e.clientY - panY; });
  window.addEventListener('mousemove', (e) => {
    if (!isDragging) return;
    panX = e.clientX - startX; panY = e.clientY - startY;
    renderCanvas();
  });
  window.addEventListener('mouseup', () => { isDragging = false; });

  canvas.addEventListener('touchstart', (e) => {
    e.preventDefault();
    if (e.touches.length === 1) {
      isDragging = true;
      startX = e.touches[0].clientX - panX;
      startY = e.touches[0].clientY - panY;
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
      panX = e.touches[0].clientX - startX;
      panY = e.touches[0].clientY - startY;
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

  function canvasToRgb332() {
    const d = ctx.getImageData(0, 0, 128, 128).data;
    const out = new Uint8Array(128 * 128);
    for (let i = 0, j = 0; i < d.length; i += 4, j++) {
      out[j] = (d[i] & 0xE0) | ((d[i + 1] & 0xE0) >> 3) | (d[i + 2] >> 6);
    }
    return out;
  }

  function canvasToRgb565() {
    const d = ctx.getImageData(0, 0, 128, 128).data;
    const out = new Uint8Array(128 * 128 * 2);
    let idx = 0;
    for (let i = 0; i < d.length; i += 4) {
      const c = ((d[i] & 0xF8) << 8) | ((d[i + 1] & 0xFC) << 3) | (d[i + 2] >> 3);
      out[idx++] = (c >> 8) & 0xFF;
      out[idx++] = c & 0xFF;
    }
    return out;
  }

  function triggerLiveFrame() {
    if (liveBusy) { liveQueued = true; return; }
    liveBusy = true;
    const buf = canvasToRgb332();
    const fd = new FormData();
    fd.append('frame', new Blob([buf], { type: 'application/octet-stream' }), 'live.bin');
    fetch('/api/liveframe', { method: 'POST', body: fd }).finally(() => {
      liveBusy = false;
      if (liveQueued) { liveQueued = false; triggerLiveFrame(); }
    });
  }

  document.getElementById('btnLiveNow').onclick = () => {
    triggerLiveFrame();
    uploadMsg.innerText = '✅ Sent to ESP32 screen!';
  };

  document.getElementById('btnUploadMedia').onclick = async () => {
    if (mediaType === 'none') {
      uploadMsg.innerText = (curLang === 'th') ? '⚠️ กรุณาเลือกรูปหรือวิดีโอก่อนครับ' : '⚠️ Please select a photo or video first';
      return;
    }

    if (mediaType === 'image') {
      uploadMsg.innerText = '⏳ Uploading 128x128 RGB565...';
      const buf = canvasToRgb565();
      const fname = 'img_' + (Date.now() % 100000) + '.rgb';
      const fd = new FormData();
      fd.append('file', new Blob([buf], { type: 'application/octet-stream' }), fname);
      const res = await fetch('/api/upload?folder=photos', { method: 'POST', body: fd });
      if (res.ok) {
        uploadMsg.innerText = '✅ Uploaded & displayed on ESP32!';
        loadStatusAndMedia();
      }
    } else if (mediaType === 'video') {
      if (animReq) cancelAnimationFrame(animReq);
      srcVideo.pause();
      const frames = 12;
      const dur = Math.min(Math.max(srcVideo.duration || 2.0, 0.5), 4.0);
      const totalBytes = 8 + frames * 128 * 128;
      const mvid = new Uint8Array(totalBytes);
      mvid.set([77, 86, 49, 1, 128, 128, 12, frames], 0);

      for (let f = 0; f < frames; f++) {
        const targetTime = Math.min((f * dur) / frames + 0.05, (srcVideo.duration || dur) - 0.01);
        await new Promise(r => {
          let done = false;
          const finish = () => { if (!done) { done = true; srcVideo.removeEventListener('seeked', finish); setTimeout(r, 45); } };
          srcVideo.addEventListener('seeked', finish);
          srcVideo.currentTime = Math.max(0.01, targetTime);
          setTimeout(finish, 350);
        });
        renderSourceToCanvas(srcVideo, srcVideo.videoWidth || 128, srcVideo.videoHeight || 128);
        const fbuf = canvasToRgb332();
        mvid.set(fbuf, 8 + f * 16384);
        uploadMsg.innerText = `🎬 Frame ${f + 1}/${frames}...`;
      }

      uploadMsg.innerText = '🚀 Uploading video to ESP32...';
      const fname = 'vid_' + (Date.now() % 100000) + '.mvid';
      const fd = new FormData();
      fd.append('file', new Blob([mvid], { type: 'application/octet-stream' }), fname);
      const res = await fetch('/api/upload?folder=videos', { method: 'POST', body: fd });
      if (res.ok) {
        uploadMsg.innerText = '✅ Video uploaded & playing on ESP32!';
        loadStatusAndMedia();
      }
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
      document.getElementById('statusText').innerHTML = `IP: <b>${st.ip}</b> | <b>http://mini.local</b>`;
      document.getElementById('fsInfo').innerText = `${st.fsUsedKB} / ${st.fsTotalKB} KB`;

      document.getElementById('chkClock').checked   = (st.idleMask & 1) !== 0;
      document.getElementById('chkWeather').checked = (st.idleMask & 2) !== 0;
      document.getElementById('chkPhoto').checked   = (st.idleMask & 4) !== 0;
      document.getElementById('chkVideo').checked   = (st.idleMask & 8) !== 0;
      document.getElementById('chkAnalog').checked  = (st.idleMask & 16) !== 0;
      document.getElementById('chkFlip').checked    = (st.idleMask & 32) !== 0;
      document.getElementById('chkAboutQr').checked = (st.idleMask & 64) !== 0;

      document.getElementById('chkLockQr').checked      = st.idleLockQr === 1;
      document.getElementById('chkLockQrAbout').checked = st.idleLockQr === 1;
      document.getElementById('chkRandom').checked      = st.idleRandom === 1;
      document.getElementById('inpIdleTime').value      = st.idleTimeout;
      document.getElementById('inpIdleCycle').value     = st.idleCycle;
      document.getElementById('inpBright').value        = st.bright;
      document.getElementById('selRotation').value      = st.rot;
      document.getElementById('selColorBgr').value      = st.bgr;
      document.getElementById('chkNightEn').checked     = st.nightEn === 1;
      document.getElementById('inpNightStart').value    = st.nightStart;
      document.getElementById('inpNightEnd').value      = st.nightEnd;
      document.getElementById('inpCity').value          = st.city;

      const wStat = document.getElementById('weatherStatusBox');
      if (st.weatherValid) {
        wStat.innerText = `✅ ${st.city}: ${st.tempC}°C | Humidity: ${st.humid}%`;
      } else {
        wStat.innerText = (curLang === 'th')
          ? 'ℹ️ ยังไม่มีข้อมูลสภาพอากาศ (ต้องเชื่อมต่อ Wi-Fi บ้านที่มีอินเทอร์เน็ตก่อน หน้าสภาพอากาศจึงจะแสดงใน Idle)'
          : 'ℹ️ Weather not yet synced (Connect to home Wi-Fi with internet first)';
      }

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

  window.saveAllSettings = async () => {
    let mask = 0;
    if (document.getElementById('chkClock').checked)   mask |= 1;
    if (document.getElementById('chkWeather').checked) mask |= 2;
    if (document.getElementById('chkPhoto').checked)   mask |= 4;
    if (document.getElementById('chkVideo').checked)   mask |= 8;
    if (document.getElementById('chkAnalog').checked)  mask |= 16;
    if (document.getElementById('chkFlip').checked)    mask |= 32;
    if (document.getElementById('chkAboutQr').checked) mask |= 64;
    if (mask === 0) mask = 127;

    const params = new URLSearchParams({
      idlemask: mask,
      idlernd: document.getElementById('chkRandom').checked ? 1 : 0,
      lockqr: document.getElementById('chkLockQr').checked ? 1 : 0,
      idletime: document.getElementById('inpIdleTime').value,
      idlecyc: document.getElementById('inpIdleCycle').value,
      bright: document.getElementById('inpBright').value,
      rot: document.getElementById('selRotation').value,
      bgr: document.getElementById('selColorBgr').value,
      night_en: document.getElementById('chkNightEn').checked ? 1 : 0,
      night_st: document.getElementById('inpNightStart').value,
      night_ed: document.getElementById('inpNightEnd').value,
      city: document.getElementById('inpCity').value
    });
    await fetch('/api/settings?' + params.toString(), { method: 'POST' });
    alert(curLang === 'th' ? '✅ บันทึกการตั้งค่าเรียบร้อยแล้ว!' : '✅ Settings saved!');
    loadStatusAndMedia();
  };

  document.getElementById('btnRefreshWeather').onclick = async () => {
    const c = document.getElementById('inpCity').value.trim();
    await fetch('/api/refresh_weather?city=' + encodeURIComponent(c), { method: 'POST' });
    setTimeout(loadStatusAndMedia, 1500);
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

  applyLang();
  loadStatusAndMedia();
</script>
</body>
</html>
)rawliteral";
