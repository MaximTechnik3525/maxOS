# Поддержка UEFI и Унифицированный Видеодрайвер в maxOS v4.0 EventUpdate

## 1. Введение и цели

maxOS v4.0 EventUpdate получила полную поддержку архитектуры **Dual-Boot (Legacy BIOS + UEFI x86_64)** и аппаратно-независимый **Унифицированный видеодрайвер (Unified Video Driver)**.

Образ `maxos.iso` является гибридным носителем:
* При загрузке на старых ПК или в стандартном режиме эмулятора используется **Legacy BIOS (VBE)**.
* При загрузке на современных материнских платах с UEFI или в QEMU с прошивкой OVMF используется **UEFI GOP (Graphics Output Protocol)**.
* Ядро и графическая среда функционируют идентично в обоих режимах без изменения пользовательских приложений (.maxP).

---

## 2. Архитектура унифицированного видеодрайвера

Ранее ядро maxOS напрямую адресовало 16-битный видео-буфер (`gfx_memory`, формат RGB565) с фиксированным шагом строки. В UEFI GOP стандартным и обязательным форматом является 32-битный цвет (BGRA8888 или RGBA8888), а физический адрес фреймбуфера динамически определяется прошивкой (например, в диапазоне `0x80000000`–`0xFD000000`).

Для прозрачной работы графической среды создан драйвер:
* **Заголовочный файл**: `src/drivers/video.h`
* **Реализация**: `src/drivers/video.c`

### Структура состояния драйвера (`video_driver_t`):
```c
typedef struct {
    unsigned char* fb;             // Линейный адрес фреймбуфера
    unsigned int width;            // Ширина в пикселях (1024)
    unsigned int height;           // Высота в пикселях (768)
    unsigned int pitch;            // Длина строки в байтах (bytes per scanline)
    unsigned char bpp;             // Глубина цвета: 16, 24, 32 бита
    unsigned char format;          // VIDEO_FORMAT_RGB565, BGRA8888, RGBA8888, etc.
    unsigned char red_pos, red_size;
    unsigned char green_pos, green_size;
    unsigned char blue_pos, blue_size;
    int is_uefi;                   // 1 — запуск под UEFI, 0 — Legacy BIOS
    const char* mode_name;         // Строковое имя текущего видеорежима
} video_driver_t;
```

### Поддерживаемые форматы пикселей:
1. `VIDEO_FORMAT_RGB565` (16-bpp, стандарт BIOS VESA/VBE)
2. `VIDEO_FORMAT_BGRA8888` (32-bpp, стандарт UEFI GOP на большинстве видеокарт)
3. `VIDEO_FORMAT_RGBA8888` (32-bpp с прямым порядком байт)
4. `VIDEO_FORMAT_RGB888` / `BGR888` (24-bpp)

### Ключевые функции драйвера:
* `video_init(multiboot_info)` — автоматический парсинг полей Multiboot 1 фреймбуфера (адрес, pitch, ширина, высота, bpp, маски каналов), определение типа прошивки (BIOS/UEFI).
* `video_rgb565_to_native(c565)` / `video_native_to_rgb565(native)` — быстрое математическое преобразование палитры 16-бит <-> 32-бит в реальном времени.
* `video_put_pixel(x, y, color)` / `video_get_pixel(x, y)` — безопасный вывод и чтение точки с защитой границ экрана.
* `video_draw_rect(x, y, w, h, color)` — оптимизированная блочная заливка прямоугольников для 16- и 32-битных режимов.
* `video_clear(color)` — полная очистка экрана текущим цветом.
* `video_scroll(x, y, w, h, lines, fill_color)` — аппаратный скроллинг фрагментов экрана для консоли Verbose Boot.
* `video_render_wallpaper(c1, c2)` — отрисовка шахматных обоев рабочего стола.
* `video_draw_cursor(x, y, c1, c2)` / `video_restore_cursor()` — неразрушающий отрисовщик аппаратного курсора мыши (сохраняет и восстанавливает подлежащие пиксели независимо от bpp).

---

## 3. Загрузочная подсистема GRUB и Dual-Boot ISO

Конфигурация `iso/boot/grub/grub.cfg` динамически определяет платформу:
```grub
if [ "${grub_platform}" = "efi" ]; then
    insmod efi_gop
    insmod efi_uga
else
    insmod vbe
    insmod vga
    insmod video_bochs
    insmod video_cirrus
fi

set default=0
set timeout=3
set gfxmode=1024x768x32,1024x768x16,1024x768,auto
set gfxpayload=keep
```

При сборке ISO через `grub-mkrescue` включаются модули:
* `/usr/lib/grub/i386-pc` — для El Torito BIOS MBR/hybrid загрузки.
* `/usr/lib/grub/x86_64-efi` — для создания EFI системного раздела `efi.img` с файлом `/efi/boot/bootx64.efi`.

---

## 4. Особенности адаптации ядра под UEFI

1. **Определение физической памяти (PMM / Memory Map)**:
   Под UEFI GRUB флаг `mbi->flags & 0x01` (`mem_lower`/`mem_upper`) может быть не заполнен, так как прошивка не предоставляет традиционный BIOS INT 12h/15h. Вместо этого анализируется Multiboot Memory Map (флаг `mbi->flags & (1 << 6)`), где суммируются все дескрипторы свободной системной памяти (тип 1).
2. **Адресация фреймбуфера выше 2 ГБ**:
   Фреймбуфер UEFI GOP часто располагается по адресам выше `0x80000000` (вплоть до `0xFD000000`). Первичные страницы отображения ядра корректно покрывают эту область памяти.
3. **Определение режима прошивки**:
   Под Legacy BIOS GRUB передает флаг `boot_device` (бит 1 в `mbi->flags`). Под UEFI GRUB этот флаг сброшен, что позволяет ядру точно определить режим (`is_uefi`) и отображать корректную информацию в Verbose Boot и SysInfo.

---

## 5. Инструкция по сборке и запуску

### Зависимости:
```bash
sudo apt-get update
sudo apt-get install -y gcc nasm qemu-system-x86 grub-pc-bin grub-efi-amd64-bin mtools xorriso ovmf
```

### Команды запуска через `./build.sh`:
* `./build.sh` — сборка и запуск в стандартном режиме Legacy BIOS.
* `./build.sh --uefi` — сборка и запуск в чистом режиме **UEFI x86_64** с прошивкой OVMF.
* `./build.sh --uefi-verbose` — запуск под UEFI с экраном подробной диагностики (Verbose Boot).
* `./build.sh --verbose` — запуск под BIOS с экраном подробной диагностики.
* `./build.sh --sata` — запуск с контроллером AHCI SATA.
* `./build.sh --no-run` — только сборка ядра и образа `maxos.iso`.

---

## 6. План дальнейших работ (Roadmap)

- [x] Унифицированный видеодрайвер для VBE (16-bpp) и GOP (32-bpp/24-bpp).
- [x] Поддержка Dual-Boot ISO (BIOS + UEFI) через GRUB.
- [x] Парсинг карты памяти EFI (MMAP) для менеджера физических страниц (PMM).
- [ ] Чтение таблицы конфигурации ACPI через RSDP / EFI System Table для расширенного управления питанием.
- [ ] Собственный UEFI Application Loader (`bootx64.efi` на C/EDK2) без зависимости от GRUB Multiboot1.
- [ ] Динамическое переключение видеорежимов на лету (GOP SetMode).
