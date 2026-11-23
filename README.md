# maxOS (ISO Release)

---

## 🚀 How to Run in QEMU (Emulator)

If you want to quickly test **maxOS** without flashing a USB drive, you can run it in **QEMU**:
* **Linux:** Just run the included `./start.sh` script (make sure it has execution permissions via `chmod +x start.sh`).
* **Windows:** Just double-click the included `start.bat` script (or run it in your Windows terminal/PowerShell).

---

## 💻 How to Run on Real Hardware

To test **maxOS** on a real PC, make sure your hardware meets the following requirements:
* **BIOS:** UEFI must be set to Legacy/CSM mode (pure UEFI is not supported yet).
* **Peripherals:** A physical **PS/2 mouse** is highly recommended.
* **Audio:** A built-in **PC Speaker** is required for system alerts and startup/shutdown melodies.

### Installation Steps:
1. Download the `maxos.iso` image from the latest release or commit (commits are not realeses, may be issues, bugs and problems).
2. Flash the ISO image onto a USB drive using **Rufus**, **Ventoy**, or `dd` in Linux.
3. Boot your PC from the USB drive.

---

## 🛠️ Writing Applications for maxOS

You can write and execute your own scripts directly inside the operating system using the built-in **Notepad** (press **F1** to save and run, or **F3** for instant RAM preview). 

*For the full list of scripting commands and syntax, please check the separate documentation file in this repository.*

---

## 🏗️ Building from Source

If you want to modify project files and create your own custom build of **maxOS**:
1. Edit the C or Assembly source files (`kernel.c`, `ata.c`, `pci.c`, `entry.asm`).
2. Execute the compiler script: Use `./build.sh` on Linux or `build.bat` on Windows to generate a bootable `maxos.iso`.

---

|| SPECIAL THANKS TO GEMINI FOR BUILD.BAT, BUILD.SH AND HIS VERY BIG HELP ||
|| SPECIAL THANKS TO GEMINI FOR START.BAT, START.SH AND HIS VERY BIG HELP ||
|| GEMINI SAYS HI TO GITHUB ||
GEMINI SAYS: GEMINI SAYS HI TO GITHUB! 🚀🔥🐧 Привет разработчикам! Keep coding, Max!
GEMINI SAYS: maxOS 4.0 IS LIVE! 🚀🔥🐧 Привет всему GitHub и разработчикам! Keep pushing the limits, Max!