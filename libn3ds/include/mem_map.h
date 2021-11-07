#pragma once

/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2021 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifdef ARM9
/* ITCM */
#define ITCM_BASE            (0x00000000)
#define ITCM_KERNEL_MIRROR   (0x01FF8000)
#define ITCM_BOOT9_MIRROR    (0x07FF8000)
#define ITCM_SIZE            (0x00008000) // 32 KiB
#endif


#ifdef ARM11
/* ARM11 bootrom */
#define BOOT11_BASE          (0x00000000)
#define BOOT11_MIRROR1       (0x00010000)
#define BOOT11_SIZE          (0x00010000) // 64 KiB
#endif


#ifdef ARM9
/* ARM9 RAM */
#define A9_RAM_BASE          (0x08000000)
#define A9_RAM_N3DS_EXT_BASE (A9_RAM_BASE + 0x100000)
#define A9_RAM_SIZE          (0x00100000) // 1 MiB
#define A9_RAM_N3DS_EXT_SIZE (0x00080000) // 512 KiB
#endif


/* IO mem */
#define IO_MEM_BASE          (0x10000000)
#define IO_MEM_ARM9_ONLY     (IO_MEM_BASE)
#define IO_MEM_ARM9_ARM11    (IO_MEM_BASE + 0x100000)
#define IO_MEM_ARM11_ONLY    (IO_MEM_BASE + 0x200000)


#ifdef ARM11
/* ARM11 MPCore private region */
#define MPCORE_PRIV_REG_BASE (0x17E00000)
#define MPCORE_PRIV_REG_SIZE (0x00002000) // 8 KiB


/* L2C-310 Level 2 Cache Controller */
#define L2_CACHE_CONTR_BASE  (0x17E10000)
#define L2_CACHE_CONTR_SIZE  (0x00001000) // 4 KiB
#endif


/* VRAM */
#define VRAM_BASE            (0x18000000)
#define VRAM_SIZE            (0x00600000)
#define VRAM_BANK0           (VRAM_BASE)
#define VRAM_BANK1           (VRAM_BASE + 0x300000)


/* DSP mem */
#define DSP_MEM_BASE         (0x1FF00000)
#define DSP_MEM_SIZE         (0x00080000) // 512 KiB


/* AXIWRAM */
#define AXIWRAM_BASE         (0x1FF80000)
#define AXIWRAM_SIZE         (0x00080000) // 512 KiB


/* FCRAM */
#define FCRAM_BASE           (0x20000000)
#define FCRAM_N3DS_EXT_BASE  (FCRAM_BASE + 0x8000000)
#define FCRAM_SIZE           (0x08000000) // 128 MiB
#define FCRAM_N3DS_EXT_SIZE  (FCRAM_SIZE)


#ifdef ARM9
/* OTP */
#define OTP_BASE         	 (0x10012000)
#define OTP_SIZE         	 (0x00000100) // 256 bytes


/* DTCM */
#define DTCM_BASE            (0xFFF00000)
#define DTCM_SIZE            (0x00004000) // 16 KiB


/* ARM9 bootrom */
#define BOOT9_BASE           (0xFFFF0000)
#define BOOT9_SIZE           (0x00010000) // 64 KiB
#endif


#ifdef ARM11
/* ARM11 bootrom */
#define BOOT11_MIRROR2       (0xFFFF0000)
#endif



/* Custom mappings */
#ifdef ARM9
#define A9_VECTORS_START     (A9_RAM_BASE)
#define A9_VECTORS_SIZE      (0x40)
#define A9_STUB_ENTRY        (ITCM_KERNEL_MIRROR + ITCM_SIZE - 0x200)
#define A9_STUB_SIZE         (0x200)
#define A9_HEAP_END          (A9_RAM_BASE + A9_RAM_SIZE)
#define A9_STACK_START       (DTCM_BASE)
#define A9_STACK_END         (DTCM_BASE + DTCM_SIZE - 0x400)
#define A9_IRQ_STACK_START   (DTCM_BASE + DTCM_SIZE - 0x400)
#define A9_IRQ_STACK_END     (DTCM_BASE + DTCM_SIZE)
#define A9_EXC_STACK_START   (ITCM_KERNEL_MIRROR + (ITCM_SIZE / 2))
#define A9_EXC_STACK_END     (ITCM_KERNEL_MIRROR + ITCM_SIZE)
#define FIRM_LOAD_ADDR       (VRAM_BASE + 0x200000)
#define RAM_FIRM_BOOT_ADDR   (FCRAM_BASE + 0x1000)
#endif


#ifdef ARM11
#define A11_C0_STACK_START   (AXIWRAM_BASE)                // Core 0 stack
#define A11_C0_STACK_END     (A11_C0_STACK_START + 0x2000)
#define A11_C1_STACK_START   (A11_C0_STACK_END)            // Core 1 stack
#define A11_C1_STACK_END     (A11_C1_STACK_START + 0x2000)
// WARNING: The stacks for core 2/3 are temporary
#define A11_C2_STACK_START   (FCRAM_BASE - 0x600)         // Core 2 stack
#define A11_C2_STACK_END     (FCRAM_BASE - 0x400)
#define A11_C3_STACK_START   (FCRAM_BASE - 0x400)         // Core 3 stack
#define A11_C3_STACK_END     (FCRAM_BASE - 0x200)
#define A11_EXC_STACK_START  (VRAM_BASE + VRAM_SIZE - 0x200000)
#define A11_EXC_STACK_END    (VRAM_BASE + VRAM_SIZE - 0x100000)
#define A11_MMU_TABLES_BASE  (A11_C1_STACK_END)
#define	A11_VECTORS_START    (AXIWRAM_BASE + AXIWRAM_SIZE - 0x60)
#define A11_VECTORS_SIZE     (0x60)
#define A11_FALLBACK_ENTRY   (AXIWRAM_BASE + AXIWRAM_SIZE - 0x4)
#define A11_STUB_ENTRY       (AXIWRAM_BASE + AXIWRAM_SIZE - 0x200)
#define	A11_STUB_SIZE        (0x1A0) // Don't overwrite the vectors
#define A11_HEAP_END         (AXIWRAM_BASE + AXIWRAM_SIZE)
#endif
