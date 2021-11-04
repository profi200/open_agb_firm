#pragma once

#include <assert.h>
#include "types.h"


// For simplicity we will name the accessible 2 controllers 1 and 2.
// The real controller number is in the comment.
#ifdef _3DS
#ifdef ARM9
#define TOSHSD1_REGS_BASE (0x10006000u) // Controller 1.
#define TOSHSD2_REGS_BASE (0x10007000u) // Controller 3. Remappable.
#elif ARM11
#define TOSHSD1_REGS_BASE (0x10122000u) // Controller 2.
#define TOSHSD2_REGS_BASE (0x10100000u) // Controller 3. Remappable.
#endif // #ifdef ARM9

#define TOSHSD_HCLK       (67027964u) // In Hz.

#elif TWL

#define TOSHSD1_REGS_BASE (0x04004800u) // Controller 1.
#define TOSHSD2_REGS_BASE (0x04004A00u) // Controller 2.

#define TOSHSD_HCLK       (33513982u) // In Hz.
#endif // #ifdef _3DS

typedef struct
{
	vu16 sd_cmd;              // 0x000
	vu16 sd_portsel;          // 0x002
	vu32 sd_arg;              // 0x004 SD_ARG0 and SD_ARG1 combined.
	vu16 sd_stop;             // 0x008
	vu16 sd_blockcount;       // 0x00A
	const vu32 sd_resp[4];    // 0x00C SD_RESP0-7 16 bit reg pairs combined.
	vu32 sd_status;           // 0x01C SD_STATUS1 and SD_STATUS2 combined.
	vu32 sd_status_mask;      // 0x020 SD_STATUS1_MASK and SD_STATUS2_MASK combined.
	vu16 sd_clk_ctrl;         // 0x024
	vu16 sd_blocklen;         // 0x026
	vu16 sd_option;           // 0x028 Data timeout: 0x2000<<12 / (67027964 / 2) = 1.001206959 sec. Card detect: 0x400<<9 / 67027964 = 0.007821929 sec.
	u8 _0x2a[2];
	const vu32 sd_err_status; // 0x02C SD_ERR_STATUS1 and SD_ERR_STATUS2 combined.
	vu16 sd_fifo;             // 0x030
	u8 _0x32[2];
	vu16 sdio_mode;           // 0x034
	vu16 sdio_status;         // 0x036
	vu16 sdio_status_mask;    // 0x038
	u8 _0x3a[0x9e];
	vu16 dma_ext_mode;        // 0x0D8
	u8 _0xda[6];
	vu16 soft_rst;            // 0x0E0
	const vu16 revision;      // 0x0E2 Controller version/revision?
	u8 _0xe4[0xe];
	vu16 unkF2;               // 0x0F2 Power related? Default 0. Other values do nothing?
	vu16 unkF4;               // 0x0F4 Unknwon. SDIO IRQ related.
	const vu16 ext_wrprot;    // 0x0F6 Apparently for eMMC.
	vu16 ext_cdet;            // 0x0F8 Card detect status.
	vu16 ext_cdet_dat3;       // 0x0FA DAT3 card detect status.
	vu16 ext_cdet_mask;       // 0x0FC Card detect mask (IRQ).
	vu16 ext_cdet_dat3_mask;  // 0x0FE DAT3 card detect mask (IRQ).
	vu16 sd_fifo32_cnt;       // 0x100
	u8 _0x102[2];
	vu16 sd_blocklen32;       // 0x104
	u8 _0x106[2];
	vu16 sd_blockcount32;     // 0x108
	u8 _0x10a[2];
	vu32 sd_fifo32;           // 0x10C Note: This is in the FIFO region on ARM11 (3DS).
} Toshsd;
static_assert(offsetof(Toshsd, sd_fifo32) == 0x10C, "Error: Member sd_fifo32 of Toshsd is not at offset 0x10C!");

ALWAYS_INLINE Toshsd* getToshsdRegs(u8 controller)
{
	Toshsd *regs;
	switch(controller)
	{
		case 0:
			regs = (Toshsd*)TOSHSD1_REGS_BASE;
			break;
		case 1:
			regs = (Toshsd*)TOSHSD2_REGS_BASE;
			break;
		default:
			regs = (Toshsd*)NULL;
	}

	return regs;
}

ALWAYS_INLINE vu32* getToshsdFifo(Toshsd *const regs)
{
#if (_3DS && ARM11)
	return (vu32*)((uintptr_t)regs + 0x200000u);
#else
	return &regs->sd_fifo32;
#endif // #if (_3DS && ARM11)
}


// REG_SD_CMD
// Bit 0-5 command index.
#define CMD_ACMD                 (1u<<6)        // Application command.
#define CMD_RESP_AUTO            (0u)           // Response type auto. Really?
#define CMD_RESP_NONE            (3u<<8)        // Response type none.
#define CMD_RESP_R1              (4u<<8)        // Response type R1 48 bit.
#define CMD_RESP_R5              (CMD_RESP_R1)  // Response type R5 48 bit.
#define CMD_RESP_R6              (CMD_RESP_R1)  // Response type R6 48 bit.
#define CMD_RESP_R7              (CMD_RESP_R1)  // Response type R7 48 bit.
#define CMD_RESP_R1b             (5u<<8)        // Response type R1b 48 bit + busy.
#define CMD_RESP_R5b             (CMD_RESP_R1b) // Response type R5b 48 bit + busy.
#define CMD_RESP_R2              (6u<<8)        // Response type R2 136 bit.
#define CMD_RESP_R3              (7u<<8)        // Response type R3 48 bit OCR without CRC.
#define CMD_RESP_R4              (CMD_RESP_R3)  // Response type R4 48 bit OCR without CRC.
#define CMD_RESP_MASK            (CMD_RESP_R3)
#define CMD_DT                   (1u<<11)       // Data transfer enable.
#define CMD_DIR_W                (0u)           // Data transfer direction write.
#define CMD_DIR_R                (1u<<12)       // Data transfer direction read.
#define CMD_MBT                  (1u<<13)       // Multi block transfer (auto STOP_TRANSMISSION).
#define CMD_SEC_SDIO             (1u<<14)       // Security/SDIO command.

// REG_SD_PORTSEL
#define PORTSEL_P0               (0u) // Controller port 0.
#define PORTSEL_P1               (1u) // Controller port 1.
#define PORTSEL_P2               (2u) // Controller port 2.
#define PORTSEL_P3               (3u) // Controller port 3.
#define PORTSEL_MASK             (PORTSEL_P3)
// Bit 8-9 number of supported ports?
#define PORTSEL_UNK10            (1u<<10) // Unknown writable bit 10?

// REG_SD_STOP
#define STOP_STOP                (1u)    // Stop/abort a transfer.
#define STOP_AUTO_STOP           (1u<<8) // Automatically send CMD12 on block transfer end.

// REG_SD_STATUS1/2 and REG_SD_STATUS1/2_MASK
// (M) = Maskable bit. 1 = disabled.
// Unmaskable bits act as status only, don't trigger IRQs and can't be acknowledged.
#define STATUS_RESP_END          (1u)     // (M) Response end.
#define STATUS_DATA_END          (1u<<2)  // (M) Data transfer end (triggers after last block).
#define STATUS_REMOVE            (1u<<3)  // (M) Card got removed.
#define STATUS_INSERT            (1u<<4)  // (M) Card got inserted. Set at the same time as DETECT.
#define STATUS_DETECT            (1u<<5)  // Card detect status (SD_OPTION detection timer). 1 = inserted.
#define STATUS_NO_WRPROT         (1u<<7)  // Write protection slider unlocked (low).
#define STATUS_DAT3_REMOVE       (1u<<8)  // (M) Card DAT3 got removed (low).
#define STATUS_DAT3_INSERT       (1u<<9)  // (M) Card DAT3 got inserted (high).
#define STATUS_DAT3_DETECT       (1u<<10) // Card DAT3 status. 1 = inserted.
#define STATUS_ERR_CMD_IDX       (1u<<16) // (M) Bad CMD index in response.
#define STATUS_ERR_CRC           (1u<<17) // (M) Bad CRC in response.
#define STATUS_ERR_STOP_BIT      (1u<<18) // (M) Stop bit error. Failed to recognize response frame end?
#define STATUS_ERR_DATA_TMOUT    (1u<<19) // (M) Response data timeout.
#define STATUS_ERR_RX_OVERF      (1u<<20) // (M) Receive FIFO overflow.
#define STATUS_ERR_TX_UNDERF     (1u<<21) // (M) Send FIFO underflow.
#define STATUS_ERR_CMD_TMOUT     (1u<<22) // (M) Response start bit timeout.
#define STATUS_SD_BUSY           (1u<<23) // SD card signals busy if this bit is 0 (DAT0 held low).
#define STATUS_RX_RDY            (1u<<24) // (M) FIFO ready for read.
#define STATUS_TX_REQ            (1u<<25) // (M) FIFO write request.
// Bit 27 is maskable. Purpose unknown.
// Bit 29 exists (not maskable). Unknown purpose.
#define STATUS_CMD_BUSY          (1u<<30) // Command register busy.
#define STATUS_ERR_ILL_ACC       (1u<<31) // (M) Illegal access error. TODO: What does that mean?

#define STATUS_MASK_ALL          (STATUS_ERR_ILL_ACC | (1u<<27) | STATUS_TX_REQ | STATUS_RX_RDY | \
                                  STATUS_ERR_CMD_TMOUT | STATUS_ERR_TX_UNDERF | STATUS_ERR_RX_OVERF | \
                                  STATUS_ERR_DATA_TMOUT | STATUS_ERR_STOP_BIT | STATUS_ERR_CRC | \
                                  STATUS_ERR_CMD_IDX | STATUS_DAT3_INSERT | STATUS_DAT3_REMOVE | \
                                  STATUS_INSERT | STATUS_REMOVE | STATUS_DATA_END | STATUS_RESP_END)
#define STATUS_MASK_DEFAULT      ((1u<<27) | STATUS_TX_REQ | STATUS_RX_RDY | \
                                  STATUS_DAT3_INSERT | STATUS_DAT3_REMOVE | STATUS_DATA_END)
#define STATUS_MASK_ERR          (STATUS_ERR_ILL_ACC | STATUS_ERR_CMD_TMOUT | STATUS_ERR_TX_UNDERF | \
                                  STATUS_ERR_RX_OVERF | STATUS_ERR_DATA_TMOUT | STATUS_ERR_STOP_BIT | \
                                  STATUS_ERR_CRC | STATUS_ERR_CMD_IDX)

// REG_SD_CLK_CTRL
#define SD_CLK_DIV_2             (0u)    // Clock divider 2.
#define SD_CLK_DIV_4             (1u)    // Clock divider 4.
#define SD_CLK_DIV_8             (1u<<1) // Clock divider 8.
#define SD_CLK_DIV_16            (1u<<2) // Clock divider 16.
#define SD_CLK_DIV_32            (1u<<3) // Clock divider 32.
#define SD_CLK_DIV_64            (1u<<4) // Clock divider 64.
#define SD_CLK_DIV_128           (1u<<5) // Clock divider 128.
#define SD_CLK_DIV_256           (1u<<6) // Clock divider 256.
#define SD_CLK_DIV_512           (1u<<7) // Clock divider 512.
#define SD_CLK_EN                (1u<<8) // Clock enable.
#define SD_CLK_AUTO_OFF          (1u<<9) // Disables clock on idle.
// Bit 10 is writable... at least according to gbatek (can't confirm). Purpose unknown.

// REG_SD_OPTION
// TODO: Bit 0-3 card detect timer 0x400<<x HCLKs. 0xF timer test.
// TODO: Bit 4-7 data timeout 0x2000<<x (HCLK / divider). 0xF timeout test.
#define OPTION_UNK14             (1u<<14) // "no C2 module" What the fuck is a C2 module?
#define OPTION_BUS_WIDTH4        (0u)     // 4 bit bus width.
#define OPTION_BUS_WIDTH1        (1u<<15) // 1 bit bus width.

// REG_SD_ERR_STATUS1/2
// TODO: Are all of these actually supported on this controller?
#define ERR_RESP_CMD_IDX         (1u)     // Manual command index error in response.
#define ERR_RESP_CMD12_IDX       (1u<<1)  // Auto command index error in response.
#define ERR_RESP_STOP_BIT        (1u<<2)  // Manual command response stop bit error.
#define ERR_RESP_STOP_BIT_CMD12  (1u<<3)  // Auto command response stop bit error.
#define ERR_STOP_BIT_DATA_READ   (1u<<4)  // Stop bit error in read data.
#define ERR_STOP_BIT_WR_CRC      (1u<<5)  // Stop bit error for write CRC status. What the hell does that mean?
#define ERR_CMD_RESP_CRC         (1u<<8)  // Manual command response CRC error.
#define ERR_CMD12_RESP_CRC       (1u<<9)  // Auto command response CRC error.
#define ERR_DATA_READ_CRC        (1u<<10) // CRC error for read data.
#define ERR_WR_CRC_STAT          (1u<<11) // "CRC error for Write CRC status for a write command". What the hell does that mean?
// Bit 13 always 1.
#define ERR_CMD_RESP_TMOUT       (1u<<16) // Manual command response timeout.
#define ERR_CMD12_RESP_TMOUT     (1u<<17) // Auto command response timeout.
// TODO: Add the correct remaining ones.

// REG_SDIO_MODE

// REG_SDIO_STATUS and REG_SDIO_STATUS_MASK

// REG_DMA_EXT_MODE
#define DMA_EXT_CPU_MODE         (0u)    // Disables DMA requests. Actually also turns off the 32 bit FIFO.
#define DMA_EXT_DMA_MODE         (1u<<1) // Enables DMA requests.
#define DMA_EXT_UNK5             (1u<<5) // "Buffer status mode"?

// REG_SOFT_RST
#define SOFT_RST_RST             (0u) // Reset.
#define SOFT_RST_NORST           (1u) // No reset.

// REG_EXT_WRPROT
// 1 = Write protected unlike SD_STATUS.
#define EXT_WRPROT_PORT1         (1u)
#define EXT_WRPROT_PORT2         (1u<<1)
#define EXT_WRPROT_PORT3         (1u<<2)

// REG_EXT_CDET and REG_EXT_CDET_MASK
// (M) = Maskable bit. 1 = disabled (no IRQ).
#define EXT_CDET_P1_REMOVE       (1u)    // (M) Port 1 card got removed.
#define EXT_CDET_P1_INSERT       (1u<<1) // (M) Port 1 card got inserted. TODO: With detection timer?
#define EXT_CDET_P1_DETECT       (1u<<2) // Port 1 card detect status. 1 = inserted. TODO: With detection timer?
#define EXT_CDET_P2_REMOVE       (1u<<3) // (M) Port 2 card got removed.
#define EXT_CDET_P2_INSERT       (1u<<4) // (M) Port 2 card got inserted. TODO: With detection timer?
#define EXT_CDET_P2_DETECT       (1u<<5) // Port 2 card detect status. 1 = inserted. TODO: With detection timer?
#define EXT_CDET_P3_REMOVE       (1u<<6) // (M) Port 3 card got removed.
#define EXT_CDET_P3_INSERT       (1u<<7) // (M) Port 3 card got inserted. TODO: With detection timer?
#define EXT_CDET_P3_DETECT       (1u<<8) // Port 3 card detect status. 1 = inserted. TODO: With detection timer?

#define EXT_CDET_MASK_ALL        (EXT_CDET_P3_INSERT | EXT_CDET_P3_REMOVE | EXT_CDET_P2_INSERT | \
                                  EXT_CDET_P2_REMOVE | EXT_CDET_P1_INSERT | EXT_CDET_P1_REMOVE)

// REG_EXT_CDET_DAT3 and REG_EXT_CDET_DAT3_MASK
// (M) = Maskable bit. 1 = disabled (no IRQ).
#define EXT_CDET_DAT3_P1_REMOVE  (1u)    // (M) Port 1 card DAT3 got removed (low).
#define EXT_CDET_DAT3_P1_INSERT  (1u<<1) // (M) Port 1 card DAT3 got inserted (high).
#define EXT_CDET_DAT3_P1_DETECT  (1u<<2) // Port 1 card DAT3 status. 1 = inserted.
#define EXT_CDET_DAT3_P2_REMOVE  (1u<<3) // (M) Port 2 card DAT3 got removed (low).
#define EXT_CDET_DAT3_P2_INSERT  (1u<<4) // (M) Port 2 card DAT3 got inserted (high).
#define EXT_CDET_DAT3_P2_DETECT  (1u<<5) // Port 2 card DAT3 status. 1 = inserted.
#define EXT_CDET_DAT3_P3_REMOVE  (1u<<6) // (M) Port 3 card DAT3 got removed (low).
#define EXT_CDET_DAT3_P3_INSERT  (1u<<7) // (M) Port 3 card DAT3 got inserted (high).
#define EXT_CDET_DAT3_P3_DETECT  (1u<<8) // Port 3 card DAT3 status. 1 = inserted.

#define EXT_CDET_DAT3_MASK_ALL   (EXT_CDET_DAT3_P3_INSERT | EXT_CDET_DAT3_P3_REMOVE | EXT_CDET_DAT3_P2_INSERT | \
                                  EXT_CDET_DAT3_P2_REMOVE | EXT_CDET_DAT3_P1_INSERT | EXT_CDET_DAT3_P1_REMOVE)

// REG_SD_FIFO32_CNT
#define FIFO32_UNK1              (1u)     // Unknown bit.
#define FIFO32_EN                (1u<<1)  // Enables the 32 bit FIFO.
#define FIFO32_FULL              (1u<<8)  // FIFO is full.
#define FIFO32_NOT_EMPTY         (1u<<9)  // FIFO is not empty. Inverted bit. 0 means empty.
#define FIFO32_CLEAR             (1u<<10) // Clears the FIFO.
#define FIFO32_FULL_IE           (1u<<11) // FIFO full IRQ enable.
#define FIFO32_NOT_EMPTY_IE      (1u<<12) // FIFO not empty IRQ enable.


typedef struct
{
	u8 portNum;
	u16 sd_clk_ctrl;
	u16 sd_blocklen; // Also sd_blocklen32.
	u16 sd_option;
	u32 *buf;
	u16 blocks;
	u32 resp[4]; // Little endian, MSB first.
} ToshsdPort;


// See status defines in regs/toshsd.h for details.
#define TSD_ERR_CMD_IDX     (1u<<16)
#define TSD_ERR_CRC         (1u<<17)
#define TSD_ERR_STOP_BIT    (1u<<18)
#define TSD_ERR_DATA_TMOUT  (1u<<19)
#define TSD_ERR_RX_OVERF    (1u<<20)
#define TSD_ERR_TX_UNDERF   (1u<<21)
#define TSD_ERR_CMD_TMOUT   (1u<<22)
#define TSD_ERR_ILL_ACC     (1u<<31)



void TOSHSD_init(void);
void TOSHSD_deinit(void);
void TOSHSD_initPort(ToshsdPort *const port, u8 portNum);
bool TOSHSD_cardDetected(void);
bool TOSHSD_cardSliderUnlocked(void);
void TOSHSD_setClock(ToshsdPort *const port, u16 clk);
u32 TOSHSD_sendCommand(ToshsdPort *const port, u16 cmd, u32 arg);

ALWAYS_INLINE void TOSHSD_setBlockLen(ToshsdPort *const port, u16 blockLen)
{
	if(blockLen > 512)       blockLen = 512;
	if(blockLen < 16)        blockLen = 0;
	if((blockLen % 16) != 0) blockLen = 0;

	port->sd_blocklen = blockLen;
}

ALWAYS_INLINE void TOSHSD_setBusWidth(ToshsdPort *const port, u8 width)
{
	// TODO: Make this more readable.
	if(width == 4) port->sd_option = 1u<<14 | 0xE9;
	else           port->sd_option = 1u<<15 | 1u<<14 | 0xE9u;
}

ALWAYS_INLINE void TOSHSD_setBuffer(ToshsdPort *const port, u32 *buf, u16 blocks)
{
	port->buf    = buf;
	port->blocks = blocks;
}

#ifdef ARM11
void TOSHSD_dbgPrint(ToshsdPort *const port);
#endif
