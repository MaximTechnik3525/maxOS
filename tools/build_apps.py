#!/usr/bin/env python3
import struct
import os

# maxb_header_t:
# uint32 magic (0x4258414D)
# uint16 version (1)
# uint16 app_type
# char name[24]
# char icon[8]
# uint16 icon_color
# uint16 flags
# uint32 state_size
# uint32 binary_size
# uint32 entry_point
# uint32 draw_offset
# uint32 click_offset
# uint32 key_offset
# uint32 tick_offset
# Total: 4 + 2 + 2 + 24 + 8 + 2 + 2 + 4 + 4 + 4 + 4 + 4 + 4 + 4 = 68 bytes

MAGIC = 0x4258414D # "MAXB"
VERSION = 1

APPS = [
    {
        "filename": "notepad.bin",
        "app_type": 1,
        "name": "Notepad",
        "icon": "NP",
        "color": 0x03EA,
        "flags": 0,
        "desc": "maxOS Text Editor v3.5 (Multi-instance)"
    },
    {
        "filename": "explorer.bin",
        "app_type": 2,
        "name": "Explorer",
        "icon": "EXP",
        "color": 0x24EE,
        "flags": 0,
        "desc": "maxFS File & Disk Manager"
    },
    {
        "filename": "calc.bin",
        "app_type": 3,
        "name": "Calculator",
        "icon": "CAL",
        "color": 0xF621,
        "flags": 0,
        "desc": "GUI Calculator 3.5 (Floating-Point)"
    },
    {
        "filename": "sysinfo.bin",
        "app_type": 4,
        "name": "SysInfo",
        "icon": "CPU",
        "color": 0x0DE5,
        "flags": 0,
        "desc": "x86_64 Long Mode System Hardware Info"
    },
    {
        "filename": "pong.bin",
        "app_type": 5,
        "name": "Pong",
        "icon": "PNG",
        "color": 0x7BEF,
        "flags": 1,
        "desc": "Retro Pong Arcade Game"
    },
    {
        "filename": "installer.bin",
        "app_type": 6,
        "name": "Installer",
        "icon": "INS",
        "color": 0x92E0,
        "flags": 0,
        "desc": "maxOS System Setup & HDD Installer"
    },
    {
        "filename": "mem.bin",
        "app_type": 7,
        "name": "Mem",
        "icon": "MEM",
        "color": 0x05E0,
        "flags": 0,
        "desc": "RAM Usage & Preemptive Task Manager"
    },
    {
        "filename": "stress.bin",
        "app_type": 8,
        "name": "StressTest",
        "icon": "RAM",
        "color": 0xF800,
        "flags": 1,
        "desc": "RAM Hardware Stress Test & Benchmark"
    }
]

os.makedirs("build", exist_ok=True)

asm_lines = [
    "; Auto-generated application binaries payload",
    "section .rodata",
]

for app in APPS:
    fname = app["filename"]
    bin_path = os.path.join("build", fname)
    
    # Pad payload to 512 bytes (1 sector)
    payload_desc = f"maxOS Binary Executable: {app['name']}\nDescription: {app['desc']}\n".encode('ascii')
    header_size = 68
    total_size = 512
    
    name_bytes = app["name"].encode('ascii')[:23] + b'\0'
    name_bytes = name_bytes.ljust(24, b'\0')
    icon_bytes = app["icon"].encode('ascii')[:7] + b'\0'
    icon_bytes = icon_bytes.ljust(8, b'\0')
    
    hdr = struct.pack(
        "<IHH24s8sHHIIIIIII",
        MAGIC,
        VERSION,
        app["app_type"],
        name_bytes,
        icon_bytes,
        app["color"],
        app["flags"],
        4096,        # state_size
        total_size,  # binary_size
        header_size, # entry_point
        header_size, # draw_offset
        header_size, # click_offset
        header_size, # key_offset
        header_size  # tick_offset
    )
    
    payload = hdr + payload_desc
    payload = payload.ljust(total_size, b'\0')
    
    with open(bin_path, "wb") as f:
        f.write(payload)
    
    sym_name = fname.replace(".", "_")
    asm_lines.append(f"global {sym_name}_data, {sym_name}_len")
    asm_lines.append(f"{sym_name}_data:")
    asm_lines.append(f'    incbin "build/{fname}"')
    asm_lines.append(f"{sym_name}_end:")
    asm_lines.append(f"{sym_name}_len: dd {sym_name}_end - {sym_name}_data")
    asm_lines.append("")

with open("build/app_binaries.asm", "w") as f:
    f.write("\n".join(asm_lines))

print(f"Successfully generated {len(APPS)} standalone application binaries and build/app_binaries.asm")
