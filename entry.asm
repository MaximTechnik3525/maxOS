; Константы для Multiboot
MODULEALIGN equ  1 << 0             ; выравнивание модулей
MEMINFO     equ  1 << 1             ; карта памяти
VIDEO_MODE  equ  1 << 2             ; флаг, говорящий что мы принимаем видеорежим от загрузчика

FLAGS       equ  MODULEALIGN | MEMINFO | VIDEO_MODE
MAGIC       equ  0x1BADB002         ; "магическое число"
CHECKSUM    equ -(MAGIC + FLAGS)    ; контрольная сумма

section .multiboot_header
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    
    ; Мы оставляем 5 пустых полей (так называемый aout_kludge), чтобы сдвинуть 
    ; параметры графики на правильные байты, иначе спецификация Multiboot ломается
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    
    ; Запрос режима (GRUB расценит это как подтверждение gfxpayload)
    dd 0    ; mode_type (0 = линейный графический фреймбуфер)
    dd 1024 ; width
    dd 768  ; height
    dd 16   ; depth

section .text
global _start
_start:
    push ebx
    extern kmain
    call kmain
    
.halt:
    hlt
    jmp .halt
