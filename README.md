# maxOS (DoubleFixes v3.5)

### What's New in v3.5:
* **Enhanced File System:** Added support for various file types.
* **Redesigned Error Screen:** Improved visual feedback for system crashes.
* **UI Improvements:** Added smooth text shadows to specific strings for better readability.
* **Optimized Controls:** Increased cursor movement speed and applied minor performance tweaks.

---

## 💻 How to Run on Real Hardware

To test **maxOS** on a real PC, make sure your hardware meets the following requirements:
* **BIOS:** UEFI must be set to Legacy/CSM mode (pure UEFI is not supported yet).
* **Peripherals:** A physical **PS/2 mouse** is highly recommended.
* **Audio:** A built-in **PC Speaker** is required for sound effects.

### Installation Steps:
1. Download the `maxOS.img` binary.
2. Flash the image onto a USB drive using a tool like **Rufus** (in DD mode) or `dd` in Linux.
3. Boot your PC from the USB drive.

---

## 🛠️ Writing Applications for maxOS

You can write and execute your own programs directly inside the operating system. To do this, simply list your commands inside the `.mapp` file. 

### Available Commands:
* **Themes:** `theme1`, `theme2`, `theme3`, `theme4`, `theme5`, `theme6`, `theme7`, `theme8`, `theme9` — Switch between 9 different system color palettes.
* **Disk Utility:** `format` — Formats the virtual `maxFS` file system disk.
* **Window Controls:** 
  * `clear` — Redraws the desktop environment and closes all active windows.
  * `drawwin` — Renders a custom application window.
  * `winrght`, `winlft`, `winup`, `windwn` — Moves the main window by 50 pixels in the respective direction.
* **Graphics & UI:**
  * `scrblack`, `scrwhite` — Fills the entire screen with pure black or white.
  * `printtext` — Prints the contents of a file near the center of the screen.
  * `trailon`, `trailoff` — Toggles the mouse cursor trail effect on or off.
* **System Status:** `stbusy`, `stfree`, `stcrit` — Manually changes the system input state.
* **Hardware & Sound:** `speaker` — Fires the PC speaker at 750 Hz for 250ms.
* **Power & Diagnostics:**
  * `sleep` — Introduces a 2-second system delay.
  * `errscr` — Forces the system to trigger the custom error screen.
  * `shtdwn` — Shuts down the system safely.

---

## 🏗️ Building from Source

If you want to modify project files and create your own custom build of **maxOS**:
1. Edit the source files.
2. Execute the compiler script (use `build.sh` if you are on Linux).

---

|| SPECIAL THANKS TO GEMINI FOR BUILD.BAT, BUILD.SH AND HIS VERY BIG HELP ||
|| GEMINI SAYS HI TO GITHUB ||
GEMINI SAYS: GEMINI SAYS HI TO GITHUB! 🚀🔥🐧
