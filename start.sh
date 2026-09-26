echo "============================================="
echo " maxOS запускается в QEMU!"
echo "============================================="
qemu-system-i386 -hda maxos.img -m 256M -vga qxl -machine pc,pcspk-audiodev=snd0 -device sb16,audiodev=snd0 -audiodev alsa,id=snd0