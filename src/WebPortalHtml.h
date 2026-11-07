#pragma once
#include <Arduino.h>

const char PAGE_INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Spotpear Mini TV & Arcade Portal</title>
<style>
  :root {
    --bg: #0d1117;
    --card: #161b22;
    --border: #30363d;
    --text: #c9d1d9;
    --accent: #58a6ff;
    --green: #2ea043;
    --red: #da3633;
    --gold: #f1e05a;
  }
  body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 16px; }
  .container { max-width: 680px; margin: 0 auto; }
  h1, h2, h3 { color: #fff; margin-top: 0; }
  .card { background: var(--card); border: 1px solid var(--border); border-radius: 8px; padding: 18px; margin-bottom: 20px; box-shadow: 0 4px 12px rgba(0,0,0,0.3); }
  label { display: block; margin-top: 10px; font-weight: 600; font-size: 13px; color: #8b949e; }
  input[type=text], input[type=password], input[type=number], select { width: 100%; box-sizing: border-box; background: #0d1117; border: 1px solid var(--border); color: #fff; padding: 10px; border-radius: 6px; margin-top: 4px; }
  button { background: var(--accent); color: #fff; border: none; padding: 10px 16px; border-radius: 6px; font-weight: 600; cursor: pointer; margin-top: 14px; width: 100%; }
  button.green { background: var(--green); }
  button:hover { opacity: 0.9; }
  .cropper-box { display: flex; flex-direction: column; align-items: center; justify-content: center; margin-top: 14px; border: 2px dashed var(--border); border-radius: 8px; padding: 12px; background: #090d13; }
  canvas { border: 2px solid var(--accent); border-radius: 4px; image-rendering: pixelated; margin-top: 10px; width: 256px; height: 256px; background: #000; }
  .controls-row { display: flex; gap: 8px; width: 100%; margin-top: 10px; }
  .status-badge { display: inline-block; padding: 4px 8px; border-radius: 12px; font-size: 12px; font-weight: 600; }
  .badge-online { background: #1f6feb; color: #fff; }
</style>
</head>
<body>
<div class="container">
  <h1>🕹️ Spotpear Mini Arcade & TV</h1>
  <p><span class="status-badge badge-online">Connected</span> Domain: <b>mini.local</b></p>

  <!-- Image & Video Cropper Studio -->
  <div class="card">
    <h2>✂️ 128x128 Media Studio (Crop & Upload)</h2>
    <p>Select any image (.png, .jpg, .webp) or GIF/Video. It will be cropped and resized to exact 128x128 pixels!</p>
    <input type="file" id="mediaInput" accept="image/*,video/*" />
    
    <div class="cropper-box">
      <canvas id="cropCanvas" width="128" height="128"></canvas>
      <div class="controls-row">
        <button id="btnZoomIn" type="button">Zoom +</button>
        <button id="btnZoomOut" type="button">Zoom -</button>
        <button id="btnRotate" type="button">Rotate 90°</button>
      </div>
      <button class="green" id="btnUploadMedia" type="button">🚀 Upload to ESP32-C3 Screen</button>
      <p id="uploadMsg" style="font-size: 13px; margin-top: 8px;"></p>
    </div>
  </div>

  <!-- Settings Card -->
  <div class="card">
    <h2>⚙️ Device & Wi-Fi Settings</h2>
    <form action="/api/settings" method="POST">
      <label>Wi-Fi Network Name (SSID):</label>
      <input type="text" name="ssid" placeholder="Enter Home Wi-Fi" />

      <label>Wi-Fi Password:</label>
      <input type="password" name="pass" placeholder="Enter Wi-Fi Password" />

      <label>Weather City Name:</label>
      <input type="text" name="city" value="Bangkok" />

      <label>Screen Brightness (%):</label>
      <input type="number" name="bright" min="10" max="100" value="85" />

      <label>Idle Sleep Timeout (seconds):</label>
      <input type="number" name="idletime" min="10" max="600" value="60" />

      <button type="submit">💾 Save & Restart Wi-Fi</button>
    </form>
  </div>

  <!-- Score Reset -->
  <div class="card">
    <h2>🏆 High Scores</h2>
    <form action="/api/resetscores" method="POST">
      <button style="background: var(--red);" type="submit" onclick="return confirm('Reset all game scores?')">🗑️ Reset All High Scores</button>
    </form>
  </div>
</div>

<script>
  const canvas = document.getElementById('cropCanvas');
  const ctx = canvas.getContext('2d');
  const mediaInput = document.getElementById('mediaInput');
  let currentImg = null;
  let zoom = 1.0;
  let rotation = 0;
  let panX = 0, panY = 0;

  // Draw default guide
  ctx.fillStyle = '#111';
  ctx.fillRect(0, 0, 128, 128);
  ctx.fillStyle = '#58a6ff';
  ctx.font = '12px sans-serif';
  ctx.textAlign = 'center';
  ctx.fillText('128 x 128 Preview', 64, 68);

  mediaInput.addEventListener('change', (e) => {
    const file = e.target.files[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = (event) => {
      const img = new Image();
      img.onload = () => {
        currentImg = img;
        zoom = 1.0;
        rotation = 0;
        panX = 0;
        panY = 0;
        renderCanvas();
      };
      img.src = event.target.result;
    };
    reader.readAsDataURL(file);
  });

  function renderCanvas() {
    if (!currentImg) return;
    ctx.fillStyle = '#000';
    ctx.fillRect(0, 0, 128, 128);
    ctx.save();
    ctx.translate(64, 64);
    ctx.rotate(rotation * Math.PI / 180);
    ctx.scale(zoom, zoom);
    const aspect = currentImg.width / currentImg.height;
    let dw = 128, dh = 128;
    if (aspect > 1) { dh = 128; dw = 128 * aspect; }
    else { dw = 128; dh = 128 / aspect; }
    ctx.drawImage(currentImg, -dw/2 + panX, -dh/2 + panY, dw, dh);
    ctx.restore();
  }

  document.getElementById('btnZoomIn').onclick = () => { zoom *= 1.15; renderCanvas(); };
  document.getElementById('btnZoomOut').onclick = () => { zoom /= 1.15; renderCanvas(); };
  document.getElementById('btnRotate').onclick = () => { rotation = (rotation + 90) % 360; renderCanvas(); };

  // Convert canvas to RGB565 and upload to ESP32
  document.getElementById('btnUploadMedia').onclick = () => {
    const msg = document.getElementById('uploadMsg');
    msg.innerText = "Converting to 16-bit RGB565...";
    const imgData = ctx.getImageData(0, 0, 128, 128);
    const pixels = imgData.data;
    const rgb565Buf = new Uint8Array(128 * 128 * 2);

    let idx = 0;
    for (let i = 0; i < pixels.length; i += 4) {
      const r = pixels[i];
      const g = pixels[i + 1];
      const b = pixels[i + 2];
      const rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
      rgb565Buf[idx++] = (rgb565 >> 8) & 0xFF;
      rgb565Buf[idx++] = rgb565 & 0xFF;
    }

    const blob = new Blob([rgb565Buf], { type: 'application/octet-stream' });
    const formData = new FormData();
    formData.append('file', blob, 'photo_' + Date.now() + '.rgb');

    msg.innerText = "Uploading to http://mini.local...";
    fetch('/api/upload', { method: 'POST', body: formData })
      .then(res => res.text())
      .then(txt => { msg.innerText = "✅ Successfully uploaded to screen!"; })
      .catch(err => { msg.innerText = "❌ Upload failed: " + err; });
  };
</script>
</body>
</html>
)rawliteral";
