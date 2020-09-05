#pragma once


// REG_LGYFB_CNT
#define LGYFB_ENABLE             (1u)
#define LGYFB_VSCALE_E           (1u<<1)
#define LGYFB_HSCALE_E           (1u<<2)
#define LGYFB_SPATIAL_DITHER_E   (1u<<4) // Unset behaves like weight 0xCCCC in both pattern regs.
#define LGYFB_TEMPORAL_DITHER_E  (1u<<5) // Unset behaves like weight 0xCCCC in both pattern regs.
#define LGYFB_OUT_FMT_8888       (0u)
#define LGYFB_OUT_FMT_8880       (1u<<8)
#define LGYFB_OUT_FMT_5551       (2u<<8)
#define LGYFB_OUT_FMT_5650       (3u<<8)
#define LGYFB_ROT_NONE           (0u)
#define LGYFB_ROT_90CW           (1u<<10)
#define LGYFB_ROT_180CW          (2u<<10)
#define LGYFB_ROT_270CW          (3u<<10)
#define LGYFB_OUT_SWIZZLE        (1u<<12)
#define LGYFB_DMA_E              (1u<<15)
#define LGYFB_IN_FMT             (1u<<16) // Use input format but this bit does nothing?

// REG_LGYFB_SIZE width and hight
#define LGYFB_SIZE(w, h)     (((h) - 1)<<16 | ((w) - 1))

// REG_LGYFB_STAT and REG_LGYFB_IRQ
#define LGYFB_IRQ_DMA_REQ    (1u)
#define LGYFB_IRQ_BUF_ERR    (1u<<1) // FIFO overrun?
#define LGYFB_IRQ_VBLANK     (1u<<2)
#define LGYFB_IRQ_MASK       (LGYFB_IRQ_VBLANK | LGYFB_IRQ_BUF_ERR | LGYFB_IRQ_DMA_REQ)
#define LGYFB_OUT_LINE(reg)  ((reg)>>16) // STAT only



void LGYFB_init(void);
void LGYFB_deinit(void);

#ifndef NDEBUG
void LGYFB_dbgDumpFrame(void);
#endif
