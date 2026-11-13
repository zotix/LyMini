<div align="center">

# LyMini 🕹️📺

**เฟิร์มแวร์เกมอาร์เคดเรโทร 18-in-1 และจอสมาร์ททีวีจำลอง สำหรับ ESP32-C3 1.44" ST7735 (128×128)**

[ 🇬🇧 English ](README.md) | [ 🇹🇭 ภาษาไทย ](README_TH.md)

[![License: Non-Commercial](https://img.shields.io/badge/License-Non--Commercial-red.svg)](LICENSE)
[![Platform: ESP32-C3](https://img.shields.io/badge/Platform-ESP32--C3-yellow.svg)](https://www.espressif.com/en/products/socs/esp32-c3)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)
[![Version: v0.3.4](https://img.shields.io/badge/Version-v0.3.4-green.svg)](https://github.com/zotix/LyMini/releases)

</div>

---

## 📖 ภาพรวม (Overview)

**LyMini** คือเฟิร์มแวร์ **เกมอาร์เคด 18-in-1 และโหมดพักหน้าจอ Smart Idle TV** ที่ออกแบบมาเพื่อบอร์ด **ESP32-C3 (Flash 16 MB) จอสี 1.44 นิ้ว ST7735 (128×128)** โดยเฉพาะ

ทำงานด้วยระบบ Framebuffer 16-bit RGB565 (ปรับจูน Gamma + 20 kHz PWM + Dirty-Frame Skip) มาพร้อม **18 เกมคลาสสิก** (เลือกเปิด/ปิดและจัดลำดับเกมได้), พื้นที่เก็บสื่อ **LittleFS 12 MB**, หน้าเว็บจัดการ 2 ภาษาที่บีบอัดด้วย **Gzip + ETag Cache + Add to Home Screen**, **OTA อัปเดตผ่าน GitHub**, และ **Light Sleep (`<1 mA`)**

---

## ✨ ฟีเจอร์หลัก (Key Features)

- 🎮 **18 เกมเรโทรในตัว (เลือกเปิด/ปิด และจัดลำดับได้จากหน้าเว็บ, ปรับกราฟิกและสมดุลเล่นง่ายขึ้น)**:
  1. **Chrome Dino** — ไดโนเสาร์กระโดดข้ามกระบองเพชร ท้องฟ้าไล่สีกลางวัน/กลางคืน ก้อนเมฆ และฝุ่นใต้เท้า
  2. **Cyber Pong** — ตีปิงปองนีออนดวลกับ AI พร้อมประกายไฟและตัวนับ Rally
  3. **Space Runner** — ยานอวกาศ 3 ชีวิต พร้อมไอเทมยิงกระจาย/Bomb และเอฟเฟกต์ระเบิด
  4. **Car Dodge** — ขับรถซิ่ง 3 เลน พร้อมไอเทมเกราะ Shield และต้นไม้ริมทาง
  5. **Retro Snake** — เกมงูเดินทะลุขอบได้ ไล่เฉดสีลำตัว พร้อมโบนัส Golden Apple
  6. **Timberman** — จามขวานตัดไม้พร้อมระบบ Combo ท้องฟ้าไล่สีและเศษไม้กระเด็น
  7. **Cyber Tetris** — เกมต่อบล็อก 3 มิติ พร้อมเงาบอกจุดตกและป้าย `CLEAR!`
  8. **Neon Breakout** — ตีลูกบอลทำลายอิฐแบบหลายด่าน มี 5 ชีวิต หางลูกบอล และอิฐ 3 มิติ
  9. **Cyber Bomber** — บอมเบอร์แมน 3 ชีวิต พร้อม Power-up เพิ่มระเบิด/ระยะไฟซ่อนในอิฐ
  10. **Flappy Bird** — นกกระพือปีกหลบท่อ 3 มิติในฉากพระอาทิตย์ตก ช่องท่อกว้างเล่นง่าย
  11. **2048 Puzzle** — เลื่อนรวมตัวเลข 3 มิติ พร้อมแถบแสดงแกนเลื่อนชัดเจน
  12. **Space Invader** — ยิงเอเลี่ยน 3 ชีวิต ยิงอัตโนมัติ พร้อมบังเกอร์กำบังและดาวกะพริบ
  13. **Pac-Mini** — เขาวงกตนีออน 13×13 พร้อม Power Pellet 4 มุมสำหรับกินผี
  14. **Simon Says** — เกมจำลำดับสีปุ่ม 3 มิติเรืองแสง พร้อมตัวนับสเต็ป
  15. **Whack-A-Mole** — ตีตัวตุ่นทองสวมมงกุฎใน 60 วิ พร้อมตัวเลขคะแนนลอย
  16. **Tower Stack** — ปล่อยบล็อก 3 มิติซ้อนหอคอยกลางท้องฟ้ายามค่ำคืน
  17. **Frogger** — กบ 3 ชีวิตข้ามถนนและท่อนซุงลายไม้กว้าง
  18. **Minesweeper** — เกมกู้ระเบิด 6×6 (4 ลูก) เปิดช่องแรกปลอดภัย 100%
- 📺 **โหมดพักหน้าจออัจฉริยะ 8 รูปแบบ (Smart Idle TV)**:
  - **🕒 นาฬิกาดิจิทัล**, **🕰️ นาฬิกาเข็ม (Analog)**, **🔢 นาฬิกาป้ายพลิก (Flip Clock)**, **🌡️ สภาพอากาศหลายเมือง**, **🕒+🌡️ นาฬิกา + สภาพอากาศ**, **🖼️ กรอบรูป (`.rgb`)**, **🎬 วิดีโอ (`.mvid` เปิด/ปิดการเล่นใน Eco Mode ได้)**, และ **☕ PromptPay QR**
- 🌐 **หน้าเว็บ Mobile-First 2 ภาษา (`http://lymini.local`)**:
  - **Gzip + `ETag` (`304`) Cache + Add to Home Screen**: ลดขนาดหน้าเว็บ ~78% โหลดซ้ำทันที และติดตั้งลงหน้าจอมือถือแบบเต็มจอได้
  - **แถบเมนูด้านล่างบนมือถือ & Auto-Save**: ใช้งานมือเดียวสะดวก และบันทึกการตั้งค่าอัตโนมัติไม่ต้องกดปุ่มบันทึก
  - **คลังสื่อ & ตัวอย่างภาพ/วิดีโอ**: แสดง Thumbnail ทันที (ดึง `.mvid` ด้วย `HTTP Range`) กดดูวิดีโอตัวอย่างแบบเล่นวนลูปบนเว็บได้ พร้อมแถบพื้นที่ LittleFS 12 MB
  - **จัดการเกมส์**: เปิด/ปิดและจัดลำดับเกมส์บนหน้า Launcher ได้ทันที
- 🚀 **OTA Firmware Update (Partition คู่บน Flash 16 MB)**:
  - **Web OTA**: เช็ก/อัปเดตจาก GitHub Releases หรืออัปโหลดไฟล์ `.bin` ในแท็บ 5
  - **On-Device OTA (`BOOT` ค้าง 10 วิ)**: เช็ก GitHub อัตโนมัติและแสดงเวอร์ชัน `Current` เทียบ `GitHub` ก่อนกดยืนยัน (`A`: อัปเดต, **กด `A` ค้าง 5 วิ**: Force Update, `B`/`BOOT`: ยกเลิก)
- ⚡ **ระบบประหยัดพลังงานหลายระดับ & Light Sleep**:
  - **ความสว่างเริ่มต้น**: **75% (ใช้งาน) / 45% (Idle) / 15% (Eco)** ปรับทีละ 5%
  - **CPU & Sleep**: **160 MHz** → **40 MHz** (Idle) → **20 MHz** (Eco) → **Light Sleep `<1 mA`** (กดปุ่มใดๆ เพื่อตื่น)
  - **กฎแบตเตอรี่ (เมื่อไม่ได้เสียบ USB)**: แบต `< 50%` เข้า **Eco Mode** อัตโนมัติ, แบต `< 20%` เข้า **Sleep Mode** อัตโนมัติ
- ℹ️ **เมนู About & Support ในหน้า Launcher**:
  - แสดงเลขเวอร์ชัน (**v0.3.4**) และกดปุ่ม **BOOT (OK)** เพื่อเปิดหน้า PromptPay QR Code (112×112)

---

## 🛒 สั่งซื้อฮาร์ดแวร์ (Where to Buy Hardware — Shopee)

- 📟 **บอร์ด ESP32-C3 1.44" LCD Mini Arcade / TV**:
  - Shopee: https://s.shopee.co.th/40hut3UXdh

---

## 📜 เงื่อนไขการใช้งาน (License Terms)

ลิขสิทธิ์ซอฟต์แวร์เป็นของ **© 2026 Zotix. All Rights Reserved.**

1. **ใช้งานส่วนตัว / ไม่แสวงหากำไร (Personal / Non-Commercial Use) — ฟรี 100%**
2. **ห้ามนำไปใช้ในเชิงพาณิชย์ (Commercial Use Prohibited)** เช่น ห้ามแฟลชลงบอร์ดเพื่อจำหน่ายบน Shopee, Lazada, TikTok Shop หรือร้านค้าใดๆ โดยไม่ได้รับอนุญาตเป็นลายลักษณ์อักษรจากผู้พัฒนา (ดูรายละเอียดใน [LICENSE](LICENSE))

---

## ☕ สนับสนุนผู้พัฒนา (Buy Me a Coffee)

หากคุณชื่นชอบโปรเจกต์ **LyMini** สามารถสแกน PromptPay QR Code ด้านล่างเพื่อเลี้ยงกาแฟผู้พัฒนาได้ครับ:

<div align="center">
  <img src="docs/images/promptpay_qr.png" width="220" alt="PromptPay QR Code">
  <br>
  <em>สแกนด้วยแอปธนาคารใดก็ได้เพื่อสนับสนุนผู้พัฒนา (Zotix) ☕</em>
</div>
