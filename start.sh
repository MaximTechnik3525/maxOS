echo "============================================="
echo " maxOS запускается в QEMU!"
echo "============================================="
qemu-system-i386 -hda maxos.iso -m 256M -vga std -machine pc,acpi=off,pcspk-audiodev=snd0 -device sb16,audiodev=snd0 -audiodev alsa,id=snd0 -netdev user,id=net0 -device rtl8139,netdev=net0