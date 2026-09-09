все будет сделано с помощью сил ии так что хер тебе а не пуш в основной репозиторий 

idk (v566): Added new command, theme 9 - neon red trail. Fixes.
To run on real PC: recommended PS/2 mouse, PC speaker and BIOS. You need to write maxOS.img on device (for example on USB flash drive) and boot with it.
You can write your own programm for maxOS right in the system. This is list of avaivable commands:
theme1, theme2, theme3, theme4, theme5, theme6 - changes system theme.
format - formates system virtual maxFS disk.
clear - redraws the scene and closes windows.
winrght, winlft, winup, windwn - moves the main window by 50 pixels in 4 directions.
speaker - enables pc speaker for 250ms on 750gHz.
scrblack, scrwhite - fills the screen in black or white color.
stbusy, stfree, stcrit - changes system status of input.
printtext - prints text of file on ~center of screen.
drawwin - draws the custom programm window.
shtdwn - shutting down the work of system.
sleep - makes delay for 2s.
errscr - running the error screen.
trailon, trailoff - enables and disables cursor trail.

### maxOS 2.0 New Features:
- **64-Bit Long Mode (x86_64)**: 4-level PML4 paging with 2MB huge pages, AMD64 System V ABI, Long Mode GDT.
- **ATA Hard Disk Driver (`ata.c` / `ata.h`)**: Direct low-level sector reading & writing via ATA PIO (LBA28).
- **maxFS 2.0 Persistent Filesystem (`maxfs.c` / `maxfs.h`)**: Superblock, Inodes, Sector Allocation Bitmap, persistent storage for files.
- **GUI System Setup & Installer v2.0 (`installer.c` / `installer.h`)**: Install maxOS directly to hard disk with MBR bootloader and pre-seeded .maxP applications.
- **Custom MBR Bootloader (`boot_mbr.asm`)**: Boots maxOS directly from HDD (`./build.sh --boot-hdd`).
- **Desktop Environment & Taskbar (`taskbar.c` / `taskbar.h`)**: 
  - Desktop icons on left side.
  - Start Menu popup with app launcher, themes, and shutdown.
  - Bottom Taskbar with active app tab (click tab or press `F10` to minimize/restore).
  - System Tray with `[x64]` badge, `[HDD]` mount indicator, and real-time RTC clock.
- **Modular Applications & `.maxP` Format (`maxp.c` / `maxp.h`)**:
  - `notepad.maxP` (`notepad.c`): GUI Text Editor 2.0 with color palette, custom filenames, file picker.
  - `explorer.maxP` (`explorer.c`): Graphical file manager with support for opening `.maxP` files and text files.
  - `calc.maxP` (`calc.c`): 3D GUI Calculator with arithmetic operations and LCD display.
  - `sysinfo.maxP` (`sysinfo.c`): System hardware & CPU/RAM/VBE inspection.
  - `pong.maxP` (`pong.c`): Retro Pong arcade game.
  - `install.maxP` (`installer.c`): Disk setup and system installer.

If you want create own build, you can edit project files, and run compilers (build.bat for Windows, build.sh for Linux).
- `./build.sh` — build and run in QEMU with ISO and HDD.
- `./build.sh --boot-hdd` — run maxOS directly from hard disk.
- `./build.sh --no-run` — build without launching QEMU.

|| SPECIAL THANKS TO GEMINI FOR BUILD.BAT, BUILD.SH AND HIS VERY BIG HELP ||
|| GEMINI SAYS HI TO GITHUB ||
GEMINI SAYS: GEMINI SAYS HI TO GITHUB! 🚀🔥🐧
