# maxOS (TextExplorer v3.6)

### What's New in v3.6:
* **Advanced Text Input:** Rewritten text handling engine, fully powering the new **Notepad app**.
* **File Explorer Upgrade:** Enhanced file management, now featuring full support for disk formatting options.
* **Kernel Stability:** Various internal fixes and minor stability improvements for the OS kernel.
* **General Improvements:** Polished existing code structure and optimized internal functions.

---

## 💻 How to Run on Real Hardware

To test **maxOS** on a real PC, make sure your hardware meets the following requirements:
* **BIOS:** UEFI must be set to Legacy/CSM mode (pure UEFI is not supported yet).
* **Peripherals:** A physical **PS/2 mouse** is highly recommended.
* **Audio:** A built-in **PC Speaker** is required for sound effects.

### Installation Steps:
1. Download the `maxOS.img` binary from the latest release.
2. Flash the image onto a USB drive using a tool like **Rufus** (in DD mode) or `dd` in Linux.
3. Boot your PC from the USB drive.

---

## 🛠️ Writing Applications for maxOS

You can write and execute your own scripts directly inside the operating system. To do this, simply list your commands inside a `.mapp` file. 

### Available Commands:
* **Text & Utilities:**
  * `notepad` — Launches the text editor with advanced text input handling.
  * `printtext` — Prints the contents of a file near the center of the screen.
* **Execution & Loops:**
  * `waitkey` — Pauses script execution and waits for a keyboard key press.
  * `repeat0`, `repeat5`, `repeat10`, `repeat50`, `repeat100` — Repeats the following block of code the specified number of times.
* **Disk Utility:** `format` — Formats the virtual `maxFS` file system disk.
* **Themes:** `theme1` to `theme9` — Switch between 9 different system color palettes.
* **Window Controls:** 
  * `clear` — Redraws the desktop environment and closes all active windows.
  * `drawwin` — Renders a custom application window.
  * `winrght`, `winlft`, `winup`, `windwn` — Moves the main window by 50 pixels in the respective direction.
* **Graphics & UI:**
  * `scrblack`, `scrwhite` — Fills the entire screen with pure black or white.
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
1. Edit the C or Assembly source files.
2. Execute the compiler script: Use `build.sh` on Linux.

---

|| SPECIAL THANKS TO GEMINI FOR BUILD.BAT, BUILD.SH AND HIS VERY BIG HELP ||
|| GEMINI SAYS HI TO GITHUB ||
GEMINI SAYS: GEMINI SAYS HI TO GITHUB! 🚀🔥🐧 Привет разработчикам! Keep coding!
