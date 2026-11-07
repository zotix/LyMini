#pragma once
#include <Arduino.h>

const char PAGE_INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>Spotpear Mini Arcade & TV v0.2.0</title>
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
  body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 12px; }
  .container { max-width: 680px; margin: 0 auto; }
  h1 { font-size: 20px; margin: 0 0 6px 0; color: #fff; }
  h2 { font-size: 16px; margin: 0 0 10px 0; color: var(--accent); }
  .topbar { display: flex; flex-wrap: wrap; gap: 8px; align-items: center; justify-content: space-between; margin-bottom: 14px; padding: 10px 12px; background: var(--card); border: 1px solid var(--border); border-radius: 10px; }
  .badge { display: inline-block; padding: 3px 8px; border-radius: 12px; font-size: 12px; font-weight: 600; background: #1e293b; color: var(--accent); }
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
  button.sm { width: auto; padding: 6px 10px; margin: 0; font-size: 12px; }
  .row { display: flex; gap: 8px; align-items: center; margin-top: 8px; }
  .row > * { flex: 1; }
  .cropper-wrap { display: flex; flex-direction: column; align-items: center; background: #080b11; border: 1px dashed var(--border); border-radius: 8px; padding: 12px; margin-top: 10px; }
  canvas { border: 2px solid var(--accent); border-radius: 6px; width: 256px; height: 256px; image-rendering: pixelated; background: #000; touch-action: none; cursor: grab; }
  .chk-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; margin-top: 8px; }
  .chk-item { display: flex; align-items: center; gap: 8px; background: #0b0f17; padding: 8px 10px; border-radius: 6px; border: 1px solid var(--border); font-size: 13px; color: #fff; }
  .chk-item input { width: 18px; height: 18px; margin: 0; }
  .media-list { margin-top: 10px; display: flex; flex-direction: column; gap: 6px; }
  .media-row { display: flex; align-items: center; justify-content: space-between; gap: 8px; background: #0b0f17; padding: 8px 10px; border-radius: 6px; border: 1px solid var(--border); font-size: 13px; }
  .media-name { flex: 1; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; color: #fff; }
  .wifi-item { display: flex; justify-content: space-between; align-items: center; background: #0b0f17; padding: 7px 10px; border-radius: 6px; margin-top: 6px; border: 1px solid var(--border); font-size: 13px; }
  .hint { font-size: 12px; color: var(--muted); margin: 4px 0 0 0; }
</style>
</head>
<body>
<div class="container">
  <div class="topbar">
    <div>
      <h1>🕹️ Spotpear Mini Arcade & TV <span class="badge">v0.2.0</span></h1>
      <div class="hint" id="statusText">Domain: <b>http://mini.local</b></div>
    </div>
    <div id="battBadge" class="badge">🔋 --%</div>
  </div>

  <!-- 1. 128x128 Image & iOS Video Cropper + Live View -->
  <div class="card">
    <h2>✂️ 128×128 Media Studio (iOS / Android / PC + Live View)</h2>
    <p class="hint">รองรับรูปภาพ (.JPG, .PNG, .WEBP, .HEIC) และวิดีโอจาก iPhone/มือถือ (.MOV, .MP4, .GIF) ลากนิ้วเพื่อเลื่อนกรอบ หรือถ่าง 2 นิ้วเพื่อซูม</p>
    <input type="file" id="mediaInput" accept="image/jpeg,image/png,image/webp,image/gif,image/heic,image/heif,video/mp4,video/quicktime,video/webm,video/x-m4v" style="margin-top:8px; width:100%;" />
    <video id="srcVideo" playsinline webkit-playsinline muted loop style="display:none;"></video>

    <div class="cropper-wrap">
      <canvas id="cropCanvas" width="128" height="128"></canvas>
      <div class="hint">💡 ลากบนกรอบภาพเพื่อจัดตำแหน่ง (Pan) / ถ่าง 2 นิ้วเพื่อซูม</div>
      <div class="row" style="width:100%;">
        <button type="button" id="btnZoomIn">Zoom +</button>
        <button type="button" id="btnZoomOut">Zoom -</button>
        <button type="button" id="btnRotate">หมุน 90°</button>
        <button type="button" id="btnResetCrop">รีเซ็ต</button>
      </div>
      <div class="chk-item" style="width:100%; margin-top:8px;">
        <input type="checkbox" id="chkAutoLive" checked />
        <span>👁️ <b>Live View บนจอ ESP32 อัตโนมัติ</b> ขณะลาก/ปรับแต่งภาพ</span>
      </div>
      <div class="row" style="width:100%;">
        <button type="button" class="purple" id="btnLiveNow">👁️ ส่งขึ้นจอ ESP ทันที</button>
        <button type="button" class="green" id="btnUploadMedia">🚀 บันทึกลงเครื่อง & แสดงผล</button>
      </div>
      <div id="uploadMsg" class="hint" style="margin-top:8px; color:var(--gold); font-weight:600;"></div>
    </div>
  </div>

  <!-- 2. Idle Screen Random & Enable/Disable Controls -->
  <div class="card">
    <h2>🎲 ตั้งค่าโหมดพักหน้าจอ (Idle Display & Random Mode)</h2>
    <label>เลือกสิ่งที่จะแสดงเมื่อพักหน้าจอ (เปิด/ปิดแยกอิสระได้):</label>
    <div class="chk-grid">
      <label class="chk-item"><input type="checkbox" id="chkClock" checked /> 🕒 นาฬิกา (Clock)</label>
      <label class="chk-item"><input type="checkbox" id="chkWeather" checked /> 🌡️ อุณหภูมิ (Weather)</label>
      <label class="chk-item"><input type="checkbox" id="chkPhoto" checked /> 🖼️ รูปภาพ (Photos)</label>
      <label class="chk-item"><input type="checkbox" id="chkVideo" checked /> 🎬 วิดีโอ (Videos)</label>
    </div>
    <div class="chk-item" style="margin-top:8px;">
      <input type="checkbox" id="chkRandom" checked />
      <span>🔀 <b>Random Mode (สุ่มลำดับหน้าจอ และสุ่มรูปภาพ/วิดีโอ)</b></span>
    </div>
    <div class="row">
      <div>
        <label>เข้า Idle เมื่อไม่กดปุ่ม (วินาที):</label>
        <input type="number" id="inpIdleTime" min="15" max="600" value="60" />
      </div>
      <div>
        <label>สลับหน้าทุกๆ (วินาที):</label>
        <input type="number" id="inpIdleCycle" min="5" max="120" value="12" />
      </div>
    </div>
    <div class="row">
      <div>
        <label>ความสว่างจอปกติ (%):</label>
        <input type="number" id="inpBright" min="10" max="100" value="85" />
      </div>
      <div>
        <label>เมืองสภาพอากาศ:</label>
        <input type="text" id="inpCity" value="Bangkok" />
      </div>
    </div>
    <button type="button" class="green" id="btnSaveIdle">💾 บันทึกตั้งค่าหน้าจอ & โหมด Idle</button>
  </div>

  <!-- 3. Media Gallery (Enable/Disable / Show Live / Delete) -->
  <div class="card">
    <div style="display:flex; justify-content:space-between; align-items:center;">
      <h2 style="margin:0;">📂 คลังภาพและวิดีโอในบอร์ด (LittleFS)</h2>
      <span id="fsInfo" class="badge">-- KB</span>
    </div>
    <p class="hint">ติ๊กถูกเพื่อเปิดแสดงในรอบ Idle หรือกด <b>👁️ แสดงทันที</b> เพื่อยิงขึ้นจอ ESP32</p>
    <div id="mediaList" class="media-list">กำลังโหลดรายการไฟล์...</div>
  </div>

  <!-- 4. Persistent Multi-WiFi Settings -->
  <div class="card">
    <h2>📶 ตั้งค่าและจำค่า Wi-Fi (บันทึกถาวรสูงสุด 3 เครือข่าย)</h2>
    <div id="savedWifiList"></div>
    <button type="button" id="btnScanWifi" style="background:#334155; color:#fff;">🔍 สแกนหา Wi-Fi รอบข้าง</button>
    <div id="scanResults"></div>
    <label>ชื่อ Wi-Fi (SSID):</label>
    <input type="text" id="inpSsid" placeholder="เลือกจากรายการสแกน หรือพิมพ์ชื่อ Wi-Fi" />
    <label>รหัสผ่าน Wi-Fi (Password):</label>
    <input type="password" id="inpPass" placeholder="รหัสผ่าน Wi-Fi" />
    <button type="button" id="btnSaveWifi">💾 บันทึก Wi-Fi และเชื่อมต่อ</button>
  </div>
</div>

<script>
  const canvas = document.getElementById('cropCanvas');
  const ctx = canvas.getContext('2d');
  const mediaInput = document.getElementById('mediaInput');
  const srcVideo = document.getElementById('srcVideo');
  const uploadMsg = document.getElementById('uploadMsg');

  let mediaType = 'none'; // 'image' or 'video'
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
    ctx.fillText('เลือกรูปหรือวิดีโอด้านบน', 64, 76);
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
      renderSourceToCanvas(srcVideo, srcVideo.videoWidth || 128, srcVideo.videoHeight || 128);
    }
  }

  function videoLoop() {
    if (mediaType === 'video' && !srcVideo.paused) {
      renderCanvas();
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
      uploadMsg.innerText = 'กำลังโหลดวิดีโอ (' + file.name + ')...';
      const url = URL.createObjectURL(file);
      srcVideo.src = url;
      srcVideo.onloadeddata = () => {
        srcVideo.play().then(() => {
          uploadMsg.innerText = '🎬 พรีวิววิดีโอพร้อม! ปรับมุมกล้องแล้วกดบันทึกได้เลย';
          videoLoop();
          if (document.getElementById('chkAutoLive').checked) setTimeout(triggerLiveFrame, 200);
        }).catch(() => {
          srcVideo.currentTime = 0.1;
          setTimeout(renderCanvas, 200);
        });
      };
    } else {
      mediaType = 'image';
      uploadMsg.innerText = 'กำลังโหลดรูปภาพ...';
      const url = URL.createObjectURL(file);
      const img = new Image();
      img.onload = () => {
        currentImg = img;
        renderCanvas();
        uploadMsg.innerText = '🖼️ โหลดรูปสำเร็จ! ลากจัดตำแหน่งได้เลย';
      };
      img.onerror = () => {
        uploadMsg.innerText = '⚠️ บน iOS กรุณาเลือกภาพเป็นมุมมองปกติ หรือใช้ Safari';
      };
      img.src = url;
    }
  });

  // Mouse & Touch Gestures (Pan & Pinch-to-Zoom for iOS/Android)
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

  // Real-Time Live View streaming to ESP32 screen (/api/liveframe)
  function triggerLiveFrame() {
    if (liveBusy) { liveQueued = true; return; }
    liveBusy = true;
    const buf = canvasToRgb332();
    const fd = new FormData();
    fd.append('frame', new Blob([buf], { type: 'application/octet-stream' }), 'live.bin');
    fetch('/api/liveframe', {
      method: 'POST',
      body: fd
    }).finally(() => {
      liveBusy = false;
      if (liveQueued) { liveQueued = false; triggerLiveFrame(); }
    });
  }

  document.getElementById('btnLiveNow').onclick = () => {
    uploadMsg.innerText = '👁️ กำลังส่งภาพขึ้นจอ ESP32...';
    triggerLiveFrame();
    setTimeout(() => { uploadMsg.innerText = '✅ แสดงภาพสดบนจอ ESP32 แล้ว!'; }, 200);
  };

  // Upload Photo (.rgb) or Multi-Frame Video (.mvid)
  document.getElementById('btnUploadMedia').onclick = async () => {
    if (mediaType === 'none') {
      uploadMsg.innerText = '⚠️ กรุณาเลือกรูปหรือวิดีโอก่อนครับ';
      return;
    }

    if (mediaType === 'image') {
      uploadMsg.innerText = '⏳ กำลังแปลงภาพ 128x128 RGB565 และอัปโหลด...';
      const buf = canvasToRgb565();
      const fname = 'img_' + (Date.now() % 100000) + '.rgb';
      const fd = new FormData();
      fd.append('file', new Blob([buf], { type: 'application/octet-stream' }), fname);
      const res = await fetch('/api/upload?folder=photos', { method: 'POST', body: fd });
      if (res.ok) {
        uploadMsg.innerText = '✅ อัปโหลดและแสดงผลบนจอสำเร็จ!';
        loadStatusAndMedia();
      } else {
        uploadMsg.innerText = '❌ อัปโหลดไม่สำเร็จ';
      }
    } else if (mediaType === 'video') {
      uploadMsg.innerText = '🎬 กำลังสกัดเฟรมวิดีโอ 12 เฟรม (128x128)...';
      srcVideo.pause();
      const frames = 12;
      const dur = Math.min(srcVideo.duration || 2.0, 4.0);
      const totalBytes = 8 + frames * 128 * 128;
      const mvid = new Uint8Array(totalBytes);
      // Header: 'M','V','1', format=1(RGB332), w=128, h=128, fps=12, frames=12
      mvid.set([77, 86, 49, 1, 128, 128, 12, frames], 0);

      for (let f = 0; f < frames; f++) {
        srcVideo.currentTime = (f * dur) / frames;
        await new Promise(r => {
          const h = () => { srcVideo.removeEventListener('seeked', h); r(); };
          srcVideo.addEventListener('seeked', h);
          setTimeout(r, 250);
        });
        renderSourceToCanvas(srcVideo, srcVideo.videoWidth || 128, srcVideo.videoHeight || 128);
        const fbuf = canvasToRgb332();
        mvid.set(fbuf, 8 + f * 16384);
        uploadMsg.innerText = `🎬 แปลงเฟรม ${f + 1}/${frames}...`;
      }

      uploadMsg.innerText = '🚀 กำลังอัปโหลดวิดีโอเข้าบอร์ด...';
      const fname = 'vid_' + (Date.now() % 100000) + '.mvid';
      const fd = new FormData();
      fd.append('file', new Blob([mvid], { type: 'application/octet-stream' }), fname);
      const res = await fetch('/api/upload?folder=videos', { method: 'POST', body: fd });
      if (res.ok) {
        uploadMsg.innerText = '✅ อัปโหลดวิดีโอและเล่นบนจอ ESP32 สำเร็จ!';
        srcVideo.play();
        loadStatusAndMedia();
      } else {
        uploadMsg.innerText = '❌ อัปโหลดวิดีโอไม่สำเร็จ';
      }
    }
  };

  async function loadStatusAndMedia() {
    try {
      const r = await fetch('/api/status');
      const st = await r.json();
      document.getElementById('battBadge').innerText = (st.usb ? '⚡ USB ' : '🔋 ') + st.batt + '%';
      document.getElementById('statusText').innerHTML = `IP: <b>${st.ip}</b> | Domain: <b>http://mini.local</b>`;
      document.getElementById('fsInfo').innerText = `${st.fsUsedKB} / ${st.fsTotalKB} KB`;

      document.getElementById('chkClock').checked   = (st.idleMask & 1) !== 0;
      document.getElementById('chkWeather').checked = (st.idleMask & 2) !== 0;
      document.getElementById('chkPhoto').checked   = (st.idleMask & 4) !== 0;
      document.getElementById('chkVideo').checked   = (st.idleMask & 8) !== 0;
      document.getElementById('chkRandom').checked  = st.idleRandom === 1;
      document.getElementById('inpIdleTime').value  = st.idleTimeout;
      document.getElementById('inpIdleCycle').value = st.idleCycle;
      document.getElementById('inpBright').value    = st.bright;
      document.getElementById('inpCity').value      = st.city;

      // Saved Wi-Fi list
      const wBox = document.getElementById('savedWifiList');
      wBox.innerHTML = '';
      (st.wifiList || []).forEach((w, idx) => {
        const div = document.createElement('div');
        div.className = 'wifi-item';
        div.innerHTML = `<span>📶 <b>${w}</b> (จำค่าแล้ว)</span>
          <button class="sm red" onclick="delWifi(${idx})">ลบ</button>`;
        wBox.appendChild(div);
      });

      // Media list
      const mBox = document.getElementById('mediaList');
      mBox.innerHTML = '';
      (st.media || []).forEach(item => {
        const row = document.createElement('div');
        row.className = 'media-row';
        const icon = item.path.endsWith('.mvid') ? '🎬' : '🖼️';
        row.innerHTML = `
          <input type="checkbox" ${item.enabled ? 'checked' : ''} onchange="toggleMedia('${item.name}', this.checked)" />
          <span class="media-name">${icon} ${item.name} <small style="color:#64748b">(${item.kb}KB)</small></span>
          <button class="sm purple" onclick="showOnEsp('${item.path}')">👁️ แสดงทันที</button>
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
    if (!confirm('ลบไฟล์ ' + path + ' ?')) return;
    await fetch('/api/delete_media?path=' + encodeURIComponent(path), { method: 'POST' });
    loadStatusAndMedia();
  };
  window.delWifi = async (idx) => {
    await fetch('/api/del_wifi?idx=' + idx, { method: 'POST' });
    loadStatusAndMedia();
  };

  document.getElementById('btnSaveIdle').onclick = async () => {
    let mask = 0;
    if (document.getElementById('chkClock').checked)   mask |= 1;
    if (document.getElementById('chkWeather').checked) mask |= 2;
    if (document.getElementById('chkPhoto').checked)   mask |= 4;
    if (document.getElementById('chkVideo').checked)   mask |= 8;
    if (mask === 0) mask = 15;

    const params = new URLSearchParams({
      idlemask: mask,
      idlernd: document.getElementById('chkRandom').checked ? 1 : 0,
      idletime: document.getElementById('inpIdleTime').value,
      idlecyc: document.getElementById('inpIdleCycle').value,
      bright: document.getElementById('inpBright').value,
      city: document.getElementById('inpCity').value
    });
    await fetch('/api/settings?' + params.toString(), { method: 'POST' });
    alert('บันทึกการตั้งค่าเรียบร้อยแล้ว!');
    loadStatusAndMedia();
  };

  document.getElementById('btnScanWifi').onclick = async () => {
    const box = document.getElementById('scanResults');
    box.innerHTML = '<div class="hint">กำลังสแกน Wi-Fi...</div>';
    const r = await fetch('/api/scan_wifi');
    const nets = await r.json();
    box.innerHTML = '';
    nets.forEach(n => {
      const d = document.createElement('div');
      d.className = 'wifi-item';
      d.style.cursor = 'pointer';
      d.innerHTML = `<span>📡 <b>${n.ssid}</b> (${n.rssi} dBm)</span><span class="badge">เลือก</span>`;
      d.onclick = () => { document.getElementById('inpSsid').value = n.ssid; };
      box.appendChild(d);
    });
  };

  document.getElementById('btnSaveWifi').onclick = async () => {
    const ssid = document.getElementById('inpSsid').value.trim();
    const pass = document.getElementById('inpPass').value;
    if (!ssid) { alert('กรุณาระบุชื่อ Wi-Fi'); return; }
    const params = new URLSearchParams({ ssid, pass });
    await fetch('/api/save_wifi?' + params.toString(), { method: 'POST' });
    alert('บันทึก Wi-Fi ลงหน่วยความจำถาวรเรียบร้อยแล้ว!');
    loadStatusAndMedia();
  };

  loadStatusAndMedia();
</script>
</body>
</html>
)rawliteral";
