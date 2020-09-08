#include <math.h>
#include <stdatomic.h>
#include "types.h"
#include "arm11/hardware/lgyfb.h"
#include "arm11/hardware/interrupt.h"
#include "hardware/corelink_dma-330.h"
#include "arm11/hardware/lcd.h"
#include "lgyfb_dma330.h"
#include "kevent.h"


#define LGYFB_TOP_REGS_BASE      (IO_MEM_ARM9_ARM11 + 0x11000)
#define REG_LGYFB_TOP_CNT        *((vu32*)(LGYFB_TOP_REGS_BASE + 0x000))
#define REG_LGYFB_TOP_SIZE       *((vu32*)(LGYFB_TOP_REGS_BASE + 0x004))
#define REG_LGYFB_TOP_STAT       *((vu32*)(LGYFB_TOP_REGS_BASE + 0x008))
#define REG_LGYFB_TOP_IRQ        *((vu32*)(LGYFB_TOP_REGS_BASE + 0x00C))
#define REG_LGYFB_TOP_FLUSH      *((vu32*)(LGYFB_TOP_REGS_BASE + 0x010)) // Write 0 to flush LgyFb FIFO.
#define REG_LGYFB_TOP_ALPHA      *((vu32*)(LGYFB_TOP_REGS_BASE + 0x020))
#define REG_LGYFB_TOP_UNK_F0     *((vu32*)(LGYFB_TOP_REGS_BASE + 0x0F0))
#define REG_LGYFB_TOP_DITHPATT0   ((vu32*)(LGYFB_TOP_REGS_BASE + 0x100)) // 2 u32 regs with 4x2 pattern bits (mask 0xCCCC) each.
#define REG_LGYFB_TOP_DITHPATT1   ((vu32*)(LGYFB_TOP_REGS_BASE + 0x108)) // 2 u32 regs with 4x2 pattern bits (mask 0xCCCC) each.

#define REG_LGYFB_TOP_V_LEN      *((vu32*)(LGYFB_TOP_REGS_BASE + 0x200))
#define REG_LGYFB_TOP_V_PATT     *((vu32*)(LGYFB_TOP_REGS_BASE + 0x204))
#define REG_LGYFB_TOP_V_ARRAY0    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x240)) // 8 regs.
#define REG_LGYFB_TOP_V_ARRAY1    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x260)) // 8 regs.
#define REG_LGYFB_TOP_V_ARRAY2    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x280)) // 8 regs.
#define REG_LGYFB_TOP_V_ARRAY3    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x2A0)) // 8 regs.
#define REG_LGYFB_TOP_V_ARRAY4    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x2C0)) // 8 regs.
#define REG_LGYFB_TOP_V_ARRAY5    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x2E0)) // 8 regs.
#define REG_LGYFB_TOP_V_MATRIX    ((vu32 (*)[8])(LGYFB_TOP_REGS_BASE + 0x240)) // 6 * 8 regs.
#define REG_LGYFB_TOP_H_LEN      *((vu32*)(LGYFB_TOP_REGS_BASE + 0x300))
#define REG_LGYFB_TOP_H_PATT     *((vu32*)(LGYFB_TOP_REGS_BASE + 0x304))
#define REG_LGYFB_TOP_H_ARRAY0    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x340)) // 8 regs.
#define REG_LGYFB_TOP_H_ARRAY1    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x360)) // 8 regs.
#define REG_LGYFB_TOP_H_ARRAY2    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x380)) // 8 regs.
#define REG_LGYFB_TOP_H_ARRAY3    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x3A0)) // 8 regs.
#define REG_LGYFB_TOP_H_ARRAY4    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x3C0)) // 8 regs.
#define REG_LGYFB_TOP_H_ARRAY5    ((vu32*)(LGYFB_TOP_REGS_BASE + 0x3E0)) // 8 regs.
#define REG_LGYFB_TOP_H_MATRIX    ((vu32 (*)[8])(LGYFB_TOP_REGS_BASE + 0x340)) // 6 * 8 regs.

#define LGYFB_TOP_FIFO           *((const vu32*)(0x10311000))


#define LGYFB_BOT_REGS_BASE      (IO_MEM_ARM9_ARM11 + 0x10000)
#define REG_LGYFB_BOT_CNT        *((vu32*)(LGYFB_BOT_REGS_BASE + 0x000))
#define REG_LGYFB_BOT_SIZE       *((vu32*)(LGYFB_BOT_REGS_BASE + 0x004))
#define REG_LGYFB_BOT_STAT       *((vu32*)(LGYFB_BOT_REGS_BASE + 0x008))
#define REG_LGYFB_BOT_IRQ        *((vu32*)(LGYFB_BOT_REGS_BASE + 0x00C))
#define REG_LGYFB_BOT_FLUSH      *((vu32*)(LGYFB_BOT_REGS_BASE + 0x010)) // Write 0 to flush LgyFb FIFO.
#define REG_LGYFB_BOT_ALPHA      *((vu32*)(LGYFB_BOT_REGS_BASE + 0x020)) // 8 bit alpha for all pixels.
#define REG_LGYFB_BOT_UNK_F0     *((vu32*)(LGYFB_BOT_REGS_BASE + 0x0F0))
#define REG_LGYFB_BOT_DITHPATT0   ((vu32*)(LGYFB_BOT_REGS_BASE + 0x100)) // 2 u32 regs with 4x2 pattern bits (mask 0xCCCC) each.
#define REG_LGYFB_BOT_DITHPATT1   ((vu32*)(LGYFB_BOT_REGS_BASE + 0x108)) // 2 u32 regs with 4x2 pattern bits (mask 0xCCCC) each.

#define REG_LGYFB_BOT_V_LEN      *((vu32*)(LGYFB_BOT_REGS_BASE + 0x200))
#define REG_LGYFB_BOT_V_PATT     *((vu32*)(LGYFB_BOT_REGS_BASE + 0x204))
#define REG_LGYFB_BOT_V_ARRAY0    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x240)) // 8 regs.
#define REG_LGYFB_BOT_V_ARRAY1    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x260)) // 8 regs.
#define REG_LGYFB_BOT_V_ARRAY2    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x280)) // 8 regs.
#define REG_LGYFB_BOT_V_ARRAY3    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x2A0)) // 8 regs.
#define REG_LGYFB_BOT_V_ARRAY4    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x2C0)) // 8 regs.
#define REG_LGYFB_BOT_V_ARRAY5    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x2E0)) // 8 regs.
#define REG_LGYFB_BOT_V_MATRIX    ((vu32 (*)[8])(LGYFB_BOT_REGS_BASE + 0x240)) // 6 * 8 regs.
#define REG_LGYFB_BOT_H_LEN      *((vu32*)(LGYFB_BOT_REGS_BASE + 0x300))
#define REG_LGYFB_BOT_H_PATT     *((vu32*)(LGYFB_BOT_REGS_BASE + 0x304))
#define REG_LGYFB_BOT_H_ARRAY0    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x340)) // 8 regs.
#define REG_LGYFB_BOT_H_ARRAY1    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x360)) // 8 regs.
#define REG_LGYFB_BOT_H_ARRAY2    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x380)) // 8 regs.
#define REG_LGYFB_BOT_H_ARRAY3    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x3A0)) // 8 regs.
#define REG_LGYFB_BOT_H_ARRAY4    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x3C0)) // 8 regs.
#define REG_LGYFB_BOT_H_ARRAY5    ((vu32*)(LGYFB_BOT_REGS_BASE + 0x3E0)) // 8 regs.
#define REG_LGYFB_BOT_H_MATRIX    ((vu32 (*)[8])(LGYFB_BOT_REGS_BASE + 0x340)) // 6 * 8 regs.

#define LGYFB_BOT_FIFO           *((const vu32*)(0x10310000))


static KEvent g_frameReadyEvent = NULL;



static void lgyFbDmaIrqHandler(UNUSED u32 intSource)
{
	DMA330_ackIrq(0);
	DMA330_run(0, program);

	// We can't match the GBA refreshrate exactly so keep the LCDs around 90%
	// ahead of the GBA output which gives us a time window of around 1.6 ms to
	// render the frame and hopefully reduces output lag as much as possible.
	u32 vtotal;
	if(REG_LCD_PDC0_VPOS > 414 - 41) vtotal = 415; // Slower than GBA.
	else                             vtotal = 414; // Faster than GBA.
	REG_LCD_PDC0_VTOTAL = vtotal;

	signalEvent(g_frameReadyEvent, false);
}

static void setScaleMatrixTop(u32 len, u32 patt, const s16 *const matrix)
{
	REG_LGYFB_TOP_V_LEN = len - 1;
	REG_LGYFB_TOP_V_PATT = patt;
	REG_LGYFB_TOP_H_LEN = len - 1;
	REG_LGYFB_TOP_H_PATT = patt;

	for(u32 y = 0; y < 6; y++)
	{
		for(u32 x = 0; x < len; x++)
		{
			const s16 tmp = matrix[len * y + x];

			// Correct the color range using the scale matrix hardware.
			// For example when converting RGB555 to RGB8 LgyFb lazily shifts the 5 bits up
			// so 0b00011111 becomes 0b11111000. This creates wrong spacing between colors.
			// TODO: What is the "+ 8" good for?
			REG_LGYFB_TOP_V_MATRIX[y][x] = tmp * 0xFF / 0xF8 + 8;
			REG_LGYFB_TOP_H_MATRIX[y][x] = tmp + 8;
		}
	}
}

void LGYFB_init(const KEvent frameReadyEvent)
{
	if(DMA330_run(0, program)) return;

	g_frameReadyEvent = (KEvent)frameReadyEvent;

	//REG_LGYFB_TOP_SIZE  = LGYFB_SIZE(240u, 160u);
	REG_LGYFB_TOP_SIZE  = LGYFB_SIZE(360u, 240u);
	REG_LGYFB_TOP_STAT  = LGYFB_IRQ_MASK;
	REG_LGYFB_TOP_IRQ   = 0;
	REG_LGYFB_TOP_ALPHA = 0xFF;

	/*
	 * Limitations:
	 * First pattern bit must be 1 and last 0 (for V-scale) or it loses sync with the DS/GBA input.
	 *
	 * Matrix ranges:
	 * in[-3] -1024-1023 (0xFC00-0x03FF)
	 * in[-2] -4096-4095 (0xF000-0x0FFF)
	 * in[-1] -32768-32767 (0x8000-0x7FFF)
	 * in[0]  -32768-32767 (0x8000-0x7FFF)
	 * in[1]  -4096-4095 (0xF000-0x0FFF)
	 * in[2]  -1024-1023 (0xFC00-0x03FF)
	 *
	 * Note: At scanline start the in FIFO is all filled with the first pixel.
	 */
	static const s16 scaleMatrix[6 * 6] =
	{
		// Original from AGB_FIRM.
		/*     0,      0,      0,      0,      0,      0, // in[-3]
		     0,      0,      0,      0,      0,      0, // in[-2]
		     0, 0x2000, 0x4000,      0, 0x2000, 0x4000, // in[-1]
		0x4000, 0x2000,      0, 0x4000, 0x2000,      0, // in[0]
		     0,      0,      0,      0,      0,      0, // in[1]
		     0,      0,      0,      0,      0,      0*/  // in[2]
		// out[0] out[1] out[2]  out[3]  out[4]  out[5]  out[6]  out[7]

		// Razor sharp (pixel duplication).
		/*     0,      0,      0,      0,      0,      0,
		     0,      0,      0,      0,      0,      0,
		     0,      0, 0x4000,      0,      0, 0x4000,
		0x4000, 0x4000,      0, 0x4000, 0x4000,      0,
		     0,      0,      0,      0,      0,      0,
		     0,      0,      0,      0,      0,      0*/

		// Sharp interpolated.
		     0,      0,      0,      0,      0,      0,
		     0,      0,      0,      0,      0,      0,
		     0,      0, 0x2000,      0,      0, 0x2000,
		0x4000, 0x4000, 0x2000, 0x4000, 0x4000, 0x2000,
		     0,      0,      0,      0,      0,      0,
		     0,      0,      0,      0,      0,      0
	};
	setScaleMatrixTop(6, 0b00011011, scaleMatrix);

	// With RGB8 output solid red and blue are converted to 0xF8 and green to 0xFA.
	// The green bias exists on the whole range of green colors.
	// Some results:
	// RGBA8:   Same as RGB8 but with useless alpha component.
	// RGB8:    Observed best format. Invisible dithering and best color accuracy.
	// RGB565:  A little visible dithering. Good color accuracy.
	// RGB5551: Lots of visible dithering. Good color accuracy (a little worse than 565).
	REG_LGYFB_TOP_CNT = LGYFB_DMA_E | LGYFB_OUT_SWIZZLE | LGYFB_OUT_FMT_8880 |
	                    LGYFB_HSCALE_E | LGYFB_VSCALE_E | LGYFB_ENABLE;

	IRQ_registerIsr(IRQ_CDMA_EVENT0, 13, 0, lgyFbDmaIrqHandler);
}

void LGYFB_deinit(void)
{
	REG_LGYFB_TOP_CNT = 0;

	DMA330_kill(0);

	IRQ_unregisterIsr(IRQ_CDMA_EVENT0);
	g_frameReadyEvent = NULL;
}

#ifndef NDEBUG
#include "fsutil.h"
/*void LGYFB_dbgDumpFrame(void)
{
	GX_displayTransfer((u32*)0x18200000, 160u<<16 | 256u, (u32*)0x18400000, 160u<<16 | 256u, 1u<<12 | 1u<<8);
	GFX_waitForEvent(GFX_EVENT_PPF, false);
	fsQuickWrite((void*)0x18400000, "sdmc:/lgyfb_dbg_frame.bgr", 256 * 160 * 3);*/
	/*GX_displayTransfer((u32*)0x18200000, 240u<<16 | 512u, (u32*)0x18400000, 240u<<16 | 512u, 1u<<12 | 1u<<8);
	GFX_waitForEvent(GFX_EVENT_PPF, false);
	fsQuickWrite((void*)0x18400000, "sdmc:/lgyfb_dbg_frame.bgr", 512 * 240 * 3);
}*/
#endif
