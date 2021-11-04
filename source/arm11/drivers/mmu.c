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

#include "types.h"
#include "mem_map.h"
#include "fb_assert.h"
#include "arm11/drivers/scu.h"
#include "mmio.h"
#include "arm.h"


// Mem permissions. Bit 5 (APX), 1-0 (AP[1:0]), all other padding.
#define PERM_NA                              (0b000000u)
#define PERM_PRIV_RW_USR_NA                  (0b000001u)
#define PERM_PRIV_RW_USR_RO                  (0b000010u)
#define PERM_PRIV_RW_USR_RW                  (0b000011u)
// These 2 don't work for supersections because no APX bit.
#define PERM_PRIV_RO_USR_NA                  (0b100001u)
#define PERM_PRIV_RO_USR_RO                  (0b100010u)

// Predefined mem attributes. Bit 12-10 (TEX[2:0]), 1 (C), 0 (B), all other padding.
// All of these count for outer and inner.
#define ATTR_STRONGLY_ORDERED                (0b0000000000000u) // Always shared
#define ATTR_SHARED_DEVICE                   (0b0000000000001u)
#define ATTR_NORM_WRITE_TROUGH_NO_ALLOC      (0b0000000000010u) // Behaves as noncacheable on ARM11 MPCore.
#define ATTR_NORM_WRITE_BACK_NO_ALLOC        (0b0000000000011u) // Behaves as write-back write-allocate.
#define ATTR_NORM_NONCACHABLE                (0b0010000000000u)
#define ATTR_NORM_WRITE_BACK_ALLOC           (0b0010000000011u)
#define ATTR_NONSHARED_DEVICE                (0b0100000000000u)

// Policies for custom normal memory attributes.
#define POLI_NONCACHABLE_UNBUFFERED          (0b00u)
#define POLI_WRITE_BACK_ALLOC_BUFFERED       (0b01u)
#define POLI_WRITE_THROUGH_NO_ALLOC_BUFFERED (0b10u) // Behaves as noncacheable on ARM11 MPCore.
#define POLI_WRITE_BACK_NO_ALLOC_BUFFERED    (0b11u) // Behaves as write-back write-allocate.

// Make custom normal memory attributes.
#define CUSTOM_ATTR(outer, inner)            (1u<<12 | (outer)<<10 | (inner))

// Converts the attribute bits from L1 format to L2 format.
// Required for mmuMapPages().
#define L1_TO_L2(attr)                       (((attr)>>6 | (attr)) & 0x73)


typedef struct
{
	u32 l1[4096];
	u32 l2PrivReg[256]; // L2 table for MPCore private region
	u32 l2Axiwram[256]; // L2 table for AXIWRAM
	u32 l2Boot11[256];  // L2 table for boot11 (high vectors)
} MmuTables;
static MmuTables *const g_mmuTables = (MmuTables*)A11_MMU_TABLES_BASE;



/**
 * @brief      Maps up to 256 16 MiB sections of memory. Domain is always 0.
 *
 * @param[in]  va      The virtual address base. Must be aligned to 16 MiB.
 * @param[in]  pa      The physical address base. Must be aligned to 16 MiB.
 * @param[in]  num     The number of sections to map.
 * @param[in]  access  The access permission bits.
 * @param[in]  xn      If this memory should be marked as execute never.
 * @param[in]  attr    Other attribute bits like caching.
 */
static void mmuMapSupersections(u32 va, u32 pa, u32 num, u8 access, bool xn, u32 attr)
{
	fb_assert(!(va & ~0xFF000000));
	fb_assert(!(pa & ~0xFF000000));
	fb_assert(num < 256);

	u32 *const l1Table = g_mmuTables->l1;
	for(u32 i = 0; i < 0x1000000 * num; i += 0x1000000)
	{
		const u32 l1Ss = (va + i)>>20;
		for(u32 n = 0; n < 16; n++)
		{
			l1Table[l1Ss + n] = (pa + i) | 1u<<18 | access<<10 | xn<<4 | attr<<2 | 0b10u;
		}
	}
}

/**
 * @brief      Maps up to 4096 1 MiB sections of memory.
 *
 * @param[in]  va      The virtual address base. Must be aligned to 1 MiB.
 * @param[in]  pa      The physical address base. Must be aligned to 1 MiB.
 * @param[in]  num     The number of sections to map.
 * @param[in]  shared  If the sections are shared memory.
 * @param[in]  access  The access permission bits.
 * @param[in]  domain  One of the 16 possible domains.
 * @param[in]  xn      If this memory should be marked as execute never.
 * @param[in]  attr    Other attribute bits like caching.
 */
static void mmuMapSections(u32 va, u32 pa, u32 num, bool shared, u8 access, u8 domain, bool xn, u32 attr)
{
	fb_assert(!(va & ~0xFFF00000));
	fb_assert(!(pa & ~0xFFF00000));
	fb_assert(num < 4096);

	u32 *const l1Table = g_mmuTables->l1;
	for(u32 i = 0; i < 0x100000 * num; i += 0x100000)
	{
		l1Table[(va + i)>>20] = (pa + i) | shared<<16 | access<<10 |
		                        domain<<5 | xn<<4 | attr<<2 | 0b10u;
	}
}

/**
 * @brief      Maps up to 256 4 KiB pages of memory.
 * @brief      The mapped range must not cross the next section.
 *
 * @param[in]  va       The virtual address base. Must be aligned to 4 KiB.
 * @param[in]  pa       The physical address base. Must be aligned to 4 KiB.
 * @param[in]  num      The number of pages to map.
 * @param      l2Table  The L2 MMU table address base for this mapping.
 * @param[in]  shared   If the pages are shared memory.
 * @param[in]  access   The access permission bits.
 * @param[in]  domain   One of the 16 possible domains.
 * @param[in]  xn       If this memory should be marked as execute never.
 * @param[in]  attr     Other attribute bits like caching.
 */
static void mmuMapPages(u32 va, u32 pa, u32 num, u32 *const l2Table, bool shared, u8 access, u8 domain, bool xn, u32 attr)
{
	fb_assert(!(va & ~0xFFFFF000));
	fb_assert(!(pa & ~0xFFFFF000));
	fb_assert(num < 256);
	fb_assert(!((u32)l2Table & ~0xFFFFFC00));

	for(u32 i = 0; i < 0x1000 * num; i += 0x1000)
	{
		l2Table[(va + i)>>12 & 0xFF] = ((pa + i) & 0xFFFFF000) | shared<<10 | access<<4 | attr<<2 | 0b10u | xn;
	}

	g_mmuTables->l1[va>>20] = (u32)l2Table | domain<<5 | 0b01u;
}

void setupMmu(void)
{
	// FCSE PID Register (FCSE PID = 0)
	// Note: This must be 0 before disabling the MMU otherwise UB
	__setFcsepidr(0);
	// Context ID Register (ASID = 0, PROCID = 0)
	__setCidr(0);
	// TTBR0 address shared page table walk and outer cachable write-through, no allocate on write
	__setTtbr0((u32)g_mmuTables->l1 | 0x12);
	// Use the 16 KiB L1 table only
	__setTtbcr(0);
	// Domain 0 = client, remaining domains all = no access
	__setDacr(1);


	static volatile bool syncFlag = false;
	if(!__getCpuId())
	{
		// Clear L1 and L2 tables
		iomemset((u32*)g_mmuTables, 0, sizeof(MmuTables));

		// IO mem mapping
		mmuMapSections(IO_MEM_ARM9_ARM11, IO_MEM_ARM9_ARM11, 4, true,
		               PERM_PRIV_RW_USR_NA, 0, true, ATTR_SHARED_DEVICE);

		// MPCore private region mapping
		mmuMapPages(MPCORE_PRIV_REG_BASE, MPCORE_PRIV_REG_BASE, 2,
		            g_mmuTables->l2PrivReg, false, PERM_PRIV_RW_USR_NA,
		            0, true, L1_TO_L2(ATTR_SHARED_DEVICE));

		// VRAM mapping
		mmuMapSections(VRAM_BASE, VRAM_BASE, 6, true, PERM_PRIV_RW_USR_NA, 0,
		               true, ATTR_NORM_WRITE_TROUGH_NO_ALLOC);

		// AXIWRAM core 0/1 stack mapping
		mmuMapPages(A11_C0_STACK_START, A11_C0_STACK_START, 4, g_mmuTables->l2Axiwram,
		            true, PERM_PRIV_RW_USR_NA, 0, true, L1_TO_L2(ATTR_NORM_WRITE_BACK_ALLOC));

		// AXIWRAM MMU table mapping
		const u32 mmuTablesPages = ((sizeof(MmuTables) + 0xFFFu) & ~0xFFFu) / 0x1000;
		mmuMapPages((u32)g_mmuTables, (u32)g_mmuTables, mmuTablesPages, g_mmuTables->l2Axiwram, true,
		            PERM_PRIV_RO_USR_NA, 0, true, L1_TO_L2(ATTR_NORM_WRITE_TROUGH_NO_ALLOC));

		extern const u32 __start__[];
		extern const u32 __text_pages__[];
		extern const u32 __rodata_start__[];
		extern const u32 __rodata_pages__[];
		extern const u32 __data_start__[];
		const u32 dataPages = (AXIWRAM_BASE + AXIWRAM_SIZE - (u32)__data_start__) / 0x1000;

		// text
		mmuMapPages((u32)__start__, (u32)__start__, (u32)__text_pages__,
		            g_mmuTables->l2Axiwram, true, PERM_PRIV_RO_USR_NA, 0, false,
		            L1_TO_L2(ATTR_NORM_WRITE_BACK_ALLOC));
		// rodata
		mmuMapPages((u32)__rodata_start__, (u32)__rodata_start__, (u32)__rodata_pages__,
		            g_mmuTables->l2Axiwram, true, PERM_PRIV_RO_USR_NA, 0, true,
		            L1_TO_L2(ATTR_NORM_WRITE_BACK_ALLOC));
		// data, bss and heap
		mmuMapPages((u32)__data_start__, (u32)__data_start__, dataPages,
		            g_mmuTables->l2Axiwram, true, PERM_PRIV_RW_USR_NA, 0, true,
		            L1_TO_L2(ATTR_NORM_WRITE_BACK_ALLOC));

		// FCRAM with New 3DS extension
		mmuMapSupersections(FCRAM_BASE, FCRAM_BASE, 16, PERM_PRIV_RW_USR_NA, true,
		                    ATTR_NORM_WRITE_BACK_ALLOC);

		// Map fastboot executable start to boot11 mirror (exception vectors)
		mmuMapPages(BOOT11_MIRROR2, (u32)__start__, 1, g_mmuTables->l2Boot11, true,
		            PERM_PRIV_RO_USR_NA, 0, false, L1_TO_L2(ATTR_NORM_WRITE_BACK_ALLOC));

		// Invalidate tag RAMs before enabling SMP as recommended by the MPCore doc.
		// TODO: Disable SCU on poweroff/reboot/FIRM launch.
		Scu *const scu = getScuRegs();
		scu->ctrl = SCU_CTRL_RST_VAL;          // Disable SCU and parity checking. Access to all aliases.
		scu->inval_all = SCU_WAY_ALL;          // Invalidate SCU tag RAMs of all CPUs.
		scu->ctrl = SCU_CTRL_RST_VAL | SCU_EN; // Enable SCU.

		syncFlag = true;
		__sev();
	}
	else while(!syncFlag) __wfe();


	// Invalidate TLB (Unified TLB operation)
	__asm__ volatile("mcr p15, 0, %0, c8, c7, 0" : : "r" (0) : "memory");
	__dsb();


	// Enable Return stack, Dynamic branch prediction, Static branch prediction,
	// Instruction folding and SMP mode: the CPU is taking part in coherency
	__setAcr(__getAcr() | 0x2F);

	// Enable MMU, D-Cache, Program flow prediction,
	// I-Cache, high exception vectors, Unaligned data access,
	// subpage AP bits disabled
	__setCr(__getCr() | 0xC03805);

	// Invalidate Both Caches. Also flushes the branch target cache
	__asm__ volatile("mcr p15, 0, %0, c7, c7, 0" : : "r" (0) : "memory");
	__dsb();
	__isb();
}
