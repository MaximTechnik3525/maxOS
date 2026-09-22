# maxOS (v3.8 Placeholder)

### What's New in v3.8:
* **ATA/IDE Hardware Driver & maxFS2:** Fully integrated a custom ATA/IDE PIO driver. Data is now written directly into the OS disk image (`maxos.img`), and the system has been upgraded to **maxFS2** with a limit of up to **10 files**.
* **Dynamic File Explorer:** The File Explorer now reads metadata directly from the HDD sectors. It calculates file sizes in bytes and displays rows strictly for existing files.
* **Full-Screen Cursor Freedom:** The cursor can now move smoothly across the entire 1024x768 screen canvas, even when system windows are active.
* **Keyboard Special Characters:** Added support for printing `-`, `+`, and `=` special characters directly inside the Notepad.
* **System Profiler Overhaul:** Overhauled the old CPU utility into an architecture monitor that displays CPUID data, total RAM, bootloader info, and disk status.
* **Escape Window Closing:** All open application window frames can now be closed by pressing the **Escape (Esc)** key.
* **Bug Fixes & Improvements:** General performance optimizations, stability patches, and cleaner core code.

---

## 💻 How to Run on Real Hardware

To test **maxOS** on a real PC, make sure your hardware meets the following requirements:
* **BIOS:** UEFI must be set to Legacy/CSM mode (pure UEFI is not supported yet).
* **Peripherals:** A physical **PS/2 mouse** is highly recommended.
* **Audio:** A built-in **PC Speaker** is required for system alerts and new startup/shutdown melodies.

### Installation Steps:
1. Download the `maxOS.img` binary from the latest release.
2. Flash the image onto a USB drive using a tool like **Rufus** (in DD mode) or `dd` in Linux.
3. Boot your PC from the USB drive.

---

## 🛠️ Writing Applications for maxOS

You can write and execute your own scripts directly inside the operating system. To do this, simply list your commands inside a `.mapp` file.

### Available Commands:
* **Text & Utilities:**
  * `printstring` — Directly prints a text string on the graphics memory buffer.
* **Execution & Loops:**
  * `waitkey` — Pauses script execution and waits for any keyboard key press.
  * `sleep` — Introduces a 2-second system delay/wait.
  * `repeat0`, `repeat5`, `repeat10`, `repeat50`, `repeat100` — Repeats the following block of code the specified number of times.
* **Disk Utility:** `format` — Formats the HDD disk sectors according to the **maxFS2** system specifications.
* **Themes:** `theme1` to `theme10` — Switch between 10 different system color palettes (including the newly balanced Theme 10 Dark Space Mode).
* **Window Controls:** 
  * `redraw` — Redraws the entire desktop workspace and refreshes active windows.
  * `drawwin` — Renders a custom application window container.
  * `winr`, `winl`, `winu`, `wind` — Moves the main window frame in the respective direction (Right, Left, Up, Down).
* **Graphics & UI:**
  * `scrblack`, `scrwhite` — Fills the entire screen with pure black or white color.
  * `trailon`, `trailoff` — Toggles the mouse cursor trail effect on or off.
* **System Status:** `stbusy`, `stfree`, `stcrit` — Manually changes the OS kernel execution states (Busy, Free, Critical).
* **Hardware & Sound:** `speaker` — Fires the physical PC speaker at 750 Hz for 250ms.
* **Power & Diagnostics:**
  * `errscr` — Forces the system to trigger the custom OS error crash screen.
  * `shutdown` — Safely powers off the machine and turns off system routines.

---

## 🏗️ Building from Source

If you want to modify project files and create your own custom build of **maxOS**:
1. Edit the C or Assembly source files.
2. Execute the compiler script: Use `build.sh` on Linux.

---

|| SPECIAL THANKS TO GEMINI FOR BUILD.BAT, BUILD.SH AND HIS VERY BIG HELP ||
|| GEMINI SAYS HI TO GITHUB ||
GEMINI SAYS: GEMINI SAYS HI TO GITHUB! 🚀🔥🐧 Привет разработчикам! Keep coding, Max!
