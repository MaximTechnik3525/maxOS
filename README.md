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

### maxOS 3.2 (RedCycle Multi-Instance & Binary Executables) Features:
- **Standalone Binary Application Format (`MAXB` / `.bin`)**:
  - Executable binaries (`notepad.bin`, `calc.bin`, `pong.bin`, `explorer.bin`, `sysinfo.bin`, `mem.bin`, `stress.bin`, `installer.bin`) compiled into `build/*.bin` and embedded into maxFS (`src/apps/maxb.h`).
  - Standardized 68-byte binary executable header with magic `MAXB` (`0x4258414D`), versioning, icon metadata, state size, and entry hook offsets.
  - Automated binary builder tool (`tools/build_apps.py`).
  - Native raw sector binary loader in maxFS (`maxfs_read_binary` / `maxfs_write_binary`).
- **Multi-Instance Application Architecture**:
  - Run multiple instances of the same application simultaneously (e.g. multiple Notepads, multiple Calculators, multiple Pong games).
  - Per-instance isolated memory state pools (`instance_state_pool[16][4096]`) preventing variable collisions.
  - Automatic window cascading / staggering with dynamic title assignment (`Notepad #1`, `Notepad #2`, `Calculator #1`, etc.).
  - Real-time window z-ordering: active window rendered on top, clicking any background window brings it immediately to front.
  - Dynamic multi-instance taskbar: each running instance has its own dedicated 3D tab with live focus indicator.
  - Global `Tab` shortcut (Alt-Tab style) cycles through all open instances with instant focus transfer.
- **64-Bit Long Mode (x86_64)**: 4-level PML4 paging with 2MB huge pages, AMD64 System V ABI, Long Mode GDT (`src/boot/entry.asm`).
- **64-Bit Long Mode (x86_64)**: 4-level PML4 paging with 2MB huge pages, AMD64 System V ABI, Long Mode GDT (`src/boot/entry.asm`).
- **Preemptive Task Scheduler (Путь B)**: Full hardware timer-driven preemptive multitasking (`src/kernel/task.c` / `task.h`). Round-Robin 20ms time slices, complete 15-register CPU trap frame preservation, SSE/FPU FXSAVE/FXRSTOR context switching, and per-process 32KB kernel & user stacks.
- **Simultaneous Multi-Process Execution**: Multiple applications run at the same time without losing state or closing each other.
- **Ring 3 Protected User Mode**: TSS and SYSCALL/SYSRET architecture isolating user apps from kernel space (`src/kernel/user.c`). Non-blocking `SYS_SLEEP`, `SYS_YIELD`, `SYS_SPAWN`, `SYS_KILL`, and `SYS_TASKLIST`.
- **Multi-Tab Taskbar & Quick Switcher**: Dynamic taskbar with individual 3D tabs for all running processes. Press `Tab` to cycle between running windows (Alt-Tab style), `F1` or `Win` to open Start Menu from anywhere.
- **Interactive Task Manager (`mem.maxP`)**: Switch between Memory Monitor and Live Process Table (press `t`). Displays real PIDs, process names, privilege levels (Ring 0 / Ring 3), CPU time, and supports terminating tasks (`k`).
- **ATA Hard Disk Driver (`src/drivers/ata.c` / `ata.h`)**: Direct low-level sector reading & writing via ATA PIO (LBA28).
- **maxFS 2.0 Persistent Filesystem (`src/fs/maxfs.c` / `maxfs.h`)**: Superblock, Inodes, Sector Allocation Bitmap, persistent storage for files.
- **GUI System Setup & Installer v3.1 (`src/apps/installer.c` / `installer.h`)**: Install maxOS directly to hard disk with MBR bootloader and pre-seeded .maxP applications.
- **Custom MBR Bootloader (`src/boot/boot_mbr.asm`)**: Boots maxOS directly from HDD (`./build.sh --boot-hdd`).
- **Modular Applications & `.maxP` Format (`src/apps/maxp.c` / `maxp.h`)**:
  - `notepad.maxP` (`src/apps/notepad.c`): GUI Text Editor 3.1 with color palette, custom filenames, file picker.
  - `explorer.maxP` (`src/apps/explorer.c`): Graphical file manager 3.1 with support for opening `.maxP` files and text files.
  - `calc.maxP` (`src/apps/calc.c`): 3D GUI Calculator 3.1 with floating-point arithmetic and LCD display.
  - `sysinfo.maxP` (`src/apps/sysinfo.c`): System hardware & CPU/RAM/VBE inspection.
  - `mem.maxP` (`src/apps/mem.c`): RAM usage monitor & interactive Preemptive Task Manager.
  - `stress.maxP` (`src/apps/mem.c`): RAM hardware stress test, memory cell integrity validation & live bandwidth benchmark.
  - `pong.maxP` (`src/apps/pong.c`): Retro Pong arcade game with smooth paddle physics.
  - `install.maxP` (`src/apps/installer.c`): Disk setup and system installer.

### Project Structure:
```
maxOS/
├── .gitignore
├── README.md
├── build.sh
├── maxos.iso
├── maxos_disk.img
├── build/                 # Intermediate build objects (*.o, *.bin)
├── iso/                   # ISO layout for GRUB
└── src/
    ├── boot/              # Bootloaders (MBR, Multiboot, 64-bit trampoline)
    ├── kernel/            # Kernel entry, graphics, taskbar, desktop
    ├── drivers/           # ATA hard disk driver
    ├── fs/                # maxFS 2.0 filesystem
    ├── apps/              # .maxP modular application suite
    └── linker.ld          # ELF64 linker script
```

If you want create own build, you can edit project files, and run compilers (build.bat for Windows, build.sh for Linux).
- `./build.sh` — build and run in QEMU with ISO and HDD.
- `./build.sh --boot-hdd` — run maxOS directly from hard disk.
- `./build.sh --no-run` — build without launching QEMU.
- `./build.sh --clean` — clean build directory.

|| SPECIAL THANKS TO GEMINI FOR BUILD.BAT, BUILD.SH AND HIS VERY BIG HELP ||
|| GEMINI SAYS HI TO GITHUB ||
GEMINI SAYS: GEMINI SAYS HI TO GITHUB! 🚀🔥🐧
