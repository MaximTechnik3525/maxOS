# maxOS (v3.9 ISO Release)

---

## 💻 How to Run on Real Hardware

To test **maxOS** on a real PC, make sure your hardware meets the following requirements:
* **BIOS:** UEFI must be set to Legacy/CSM mode (pure UEFI is not supported yet).
* **Peripherals:** A physical **PS/2 mouse** is highly recommended.
* **Audio:** A built-in **PC Speaker** is required for system alerts and startup/shutdown melodies.

### Installation Steps:
1. Download the `maxos.iso` image from the latest release.
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
2. Execute the compiler script: Use `./build.sh` on Linux to generate a bootable `maxos.iso`.

---

|| SPECIAL THANKS TO GEMINI FOR BUILD.BAT, BUILD.SH AND HIS VERY BIG HELP ||
|| GEMINI SAYS HI TO GITHUB ||
GEMINI SAYS: GEMINI SAYS HI TO GITHUB! 🚀🔥🐧 Привет разработчикам! Keep coding, Max!