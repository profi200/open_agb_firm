#include <stdatomic.h>
#include "types.h"
#include "arm11/hardware/lgyfb.h"
#include "arm11/hardware/interrupt.h"
#include "hardware/corelink_dma-330.h"
#include "hardware/gfx.h"
#include "lgyfb_dma330.h"


#define REG_LGYFB_TOP_CNT    *((vu32*)(0x10111000))
#define REG_LGYFB_TOP_SIZE   *((vu32*)(0x10111004))
#define REG_LGYFB_TOP_STAT   *((vu32*)(0x10111008))
#define REG_LGYFB_TOP_IRQ    *((vu32*)(0x1011100C))
#define REG_LGYFB_TOP_ALPHA  *((vu32*)(0x10111020))
#define REG_LGYFB_TOP_UNK_F0 *((vu32*)(0x101110F0))
// TODO: Add the missing regs.
#define LGYFB_TOP_FIFO       *((vu32*)(0x10311000))

#define REG_LGYFB_BOT_CNT    *((vu32*)(0x10110000))
#define REG_LGYFB_BOT_SIZE   *((vu32*)(0x10110004))
#define REG_LGYFB_BOT_STAT   *((vu32*)(0x10110008))
#define REG_LGYFB_BOT_IRQ    *((vu32*)(0x1011000C))
#define REG_LGYFB_BOT_ALPHA  *((vu32*)(0x10110020)) // 8 bit alpha for all pixels.
#define REG_LGYFB_BOT_UNK_F0 *((vu32*)(0x101100F0))
// TODO: Add the missing regs.
#define LGYFB_BOT_FIFO       *((vu32*)(0x10310000))


static bool flag = false;



static void lgyFbDmaIrqHandler(UNUSED u32 intSource)
{
	DMA330_ackIrq(0);
	atomic_store_explicit(&flag, true, memory_order_relaxed);
}

void LGYFB_init(void)
{
	if(DMA330_run(0, program)) return;

	REG_LGYFB_TOP_SIZE  = LGYFB_SIZE(160u, 240u);
	REG_LGYFB_TOP_STAT  = LGYFB_IRQ_MASK;
	REG_LGYFB_TOP_IRQ   = 0;
	REG_LGYFB_TOP_ALPHA = 0xFF;
	REG_LGYFB_TOP_CNT   = LGYFB_DMA_E | /*LGYFB_OUT_SWIZZLE |*/ LGYFB_OUT_FMT_5551 | LGYFB_ENABLE;

	IRQ_registerHandler(IRQ_CDMA_EVENT0, 13, 0, true, lgyFbDmaIrqHandler);
}

void LGYFB_processFrame(void)
{
	if(atomic_load_explicit(&flag, memory_order_relaxed))
	{
		atomic_store_explicit(&flag, false, memory_order_relaxed);

		// TODO: Do this with the GPU.
		u16 *fb = GFX_getFramebuffer(SCREEN_TOP) + (160 * 2) + (80 * 2 * 240);
		const u64 *data = (u64*)RENDERBUF_TOP;
		for(u32 y = 0; y < 160; y++)
		{
			for(u32 x = 0; x < 240; x += 4)
			{
				u64 tmp = data[x / 4];

				fb[x * 240] = tmp;
				fb[(x + 1) * 240] = tmp>>16;
				fb[(x + 2) * 240] = tmp>>32;
				fb[(x + 3) * 240] = tmp>>48;
			}

			fb--;
			data += 240 / 4;
		}

		// CDMA takes some cycles to get to stopped state so we will use this time
		// to do the frame rotation and restart it later.
		DMA330_run(0, program);
		GFX_swapFramebufs();
	}
}

void LGYFB_deinit(void)
{
	REG_LGYFB_TOP_CNT = 0;

	DMA330_kill(0);

	IRQ_unregisterHandler(IRQ_CDMA_EVENT0);
}
