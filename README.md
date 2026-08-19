Update 2.3: fixed theme 6, new theme - inversion, new commands.
To run on qemu: if you are on Linux, start qemu with this command "qemu-system-i386 -vga cirrus -audiodev alsa,id=snd0 -machine pcspk-audiodev=snd0 -display default,full-screen=on -fda 'path to maxOS IMG'". if you are on Windows, start with qemu with this command "qemu-system-i386 -vga cirrus -machine pcspk-audiodev=audio0 -audiodev sdl,id=audio0 -display default,full-screen=on -fda 'path to maxOS IMG'".
To run on real PC: recommended PS/2 mouse and BIOS. You need to write maxOS.img on device (for example on USB flash drive) and boot with it.
You can write your own programm for maxOS right in the system. This is list of avaivable commands:
theme1, theme2, theme3, theme4, theme5, theme6 - changes system theme.
format - formates system virtual maxFS disk.
clear - redraws the scene and closes windows.
winrght, winlft, winup, windwn - moves the main window by 50 pixels in 4 directions.
spkr - enables pc speaker for 250ms on 750gHz.
scrblck, scrwht - fills the screen in black or white color.
stbusy, stfree - changes system status of input.
If you want create own build, you can edit project files, and run compilers (build.bat for Windows, build.sh for Linux).

|| SPECIAL THANKS TO GEMINI FOR BUILD.BAT, BUILD.SH AND HIS VERY BIG HELP ||
|| GEMINI SAYS HI TO GITHUB ||
GEMINI SAYS: GEMINI SAYS HI TO GITHUB! 🚀🔥🐧
