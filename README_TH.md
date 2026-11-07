<div align="center">

# LyMini 🕹️📺

**เฟิร์มแวร์เกมอาร์เคดเรโทร 9-in-1 และจอสมาร์ททีวีจำลอง สำหรับ ESP32-C3 1.44" ST7735 (128×128)**

[ 🇬🇧 English ](README.md) | [ 🇹🇭 ภาษาไทย ](README_TH.md)

[![License: Non-Commercial](https://img.shields.io/badge/License-Non--Commercial-red.svg)](LICENSE)
[![Platform: ESP32-C3](https://img.shields.io/badge/Platform-ESP32--C3-yellow.svg)](https://www.espressif.com/en/products/socs/esp32-c3)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)
[![Version: v0.2.2](https://img.shields.io/badge/Version-v0.2.2-green.svg)](https://github.com/zotix/LyMini/releases)

</div>

---

## 📖 ภาพรวม (Overview)

**LyMini** คือเฟิร์มแวร์ **เกมอาร์เคด 9-in-1 และโหมดพักหน้าจอ Smart Idle TV** ที่ออกแบบมาเพื่อบอร์ด **ESP32-C3 จอสี 1.44 นิ้ว ST7735 (128×128)** โดยเฉพาะ

ทำงานด้วยระบบ Framebuffer 16-bit RGB565 ความเร็วสูงผ่านฮาร์ดแวร์ SPI 40 MHz ภาพลื่นไหลไม่กะพริบ มาพร้อมเกมคลาสสิก 9 เกม, เมนูปรับแสงหน้าจอบนตัวเครื่อง, โหมดพักหน้าจออัจฉริยะ 8 รูปแบบ, หน้าเว็บจัดการ 2 ภาษา (`http://lymini.local`) ที่รองรับการครอปรูปและวิดีโอจาก iOS/Android พร้อมสตรีมภาพสดขึ้นจอทันที

---

## ✨ ฟีเจอร์หลัก (Key Features)

- 🎮 **9 เกมเรโทรในตัว (พร้อมภาพปกกราฟิกในหน้าเมนู)**:
  1. **Chrome Dino** — ไดโนเสาร์กระโดดข้ามกระบองเพชร
  2. **Cyber Pong** — ตีปิงปองนีออนดวลกับ AI
  3. **Space Runner** — ยานอวกาศหลบและยิงอุกกาบาต
  4. **Car Dodge** — ขับรถซิ่งหลบรถบนถนน 3 เลน
  5. **Retro Snake** — เกมงูกินแอปเปิลบังคับเลี้ยวซ้าย/ขวา
  6. **Timberman** — จามขวานตัดไม้หลบกิ่งไม้
  7. **Cyber Tetris** — เกมต่อบล็อกคลาสสิก
  8. **Neon Breakout** — ตีลูกบอลทำลายกำแพงอิฐสีนีออน
  9. **Cyber Bomber** — บอมเบอร์แมนวางระเบิดทำลายกำแพงอิฐและศัตรู
- 📺 **โหมดพักหน้าจออัจฉริยะ 8 รูปแบบ (Smart Idle TV)**:
  - **🕒 นาฬิกาดิจิทัล**, **🕰️ นาฬิกาเข็ม (Analog)**, **🔢 นาฬิกาป้ายพลิก (Flip Clock)**, **🌡️ สภาพอากาศหลายเมือง**, **🕒+🌡️ นาฬิกา + สภาพอากาศในหน้าเดียว**, **🖼️ กรอบรูปดิจิทัล (`.rgb`)**, **🎬 วิดีโอแอนิเมชัน (`.mvid`)**, และ **☕ สนับสนุนค่ากาแฟ (PromptPay QR)**
  - **กดดูตัวอย่างบนจอจริง (`👁️`)**: กดเลือกโหมดหรือเมืองจากหน้าเว็บเพื่อให้แสดงบนหน้าจอจริงของบอร์ดได้ทันที
- 🌐 **หน้าเว็บตั้งค่า 2 ภาษา ไทย/อังกฤษ (`http://lymini.local`)**:
  - เชื่อมต่อ Wi-Fi **`LyMini-AP`** (ไม่ต้องใช้รหัสผ่าน) หน้าเว็บตั้งค่าจะเด้งขึ้นมาอัตโนมัติ (Captive Portal)
  - **🔆 ปรับความสว่างสูงสุดแบบ Auto-Update**: เลื่อนแถบปุ๊บแสงหน้าจอเปลี่ยนตามทันที และทำหน้าที่เป็นเพดานความสว่างสูงสุด
  - **🔄 หมุนหน้าจอได้ 4 ทิศทาง (เริ่มต้น 270°)**: พร้อมระบบสลับปุ่มซ้าย/ขวาตามการหมุนหน้าจออัตโนมัติ
  - **🌙 ตั้งเวลาปิดไฟหน้าจอกลางคืน**: ปิดไฟ Backlight (`0%`) อัตโนมัติเมื่อถึงเวลานอน และตื่นทันทีเมื่อกดปุ่ม

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
