/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "types.h"
#include "drivers/toshsd.h"
#include "arm.h"
#include "drivers/mmc/sdmmc.h"
#include "arm9/drivers/ndma.h"
#include "drivers/cache.h"



/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	(void)pdrv;

	/*DSTATUS dstat = 0; // TODO: Testing.
	if(!TOSHSD_cardDetected())
	{
		// Assumes IRQs are on and the controller IRQ is the only one that fires.
		// TODO: Better strategy.
		__wfi();
		dstat = (TOSHSD_cardDetected() == true ? 0u : STA_NODISK);
	}

	return dstat;*/
	return (TOSHSD_cardDetected() == true ? 0u : STA_NODISK);
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive number to identify the drive */
)
{
	(void)pdrv;

	return (SDMMC_init(SDMMC_DEV_CARD) == RES_OK ? 0u : STA_NOINIT);
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	(void)pdrv;
	// TODO: fatfs will use memcpy() for sizes under 512 bytes
	//       but larger transfers run into this check if unaligned.
	if((uintptr_t)buff % 4u) return RES_PARERR;

	// Warning! Flush before transfer only works on ARM9 (no speculative prefetching)!
	flushDCacheRange(buff, 512u * count);

	NdmaCh *const ndmaCh = getNdmaChRegs(5);
	do
	{
		const u16 blockCount = (count > 0xFFFFu ? 0xFFFFu : count);
		ndmaCh->sad  = (u32)getToshsdFifo(getToshsdRegs(1)); // TODO: SDMMC dev to FIFO function.
		ndmaCh->dad  = (u32)buff;
		ndmaCh->tcnt = (u32)blockCount<<7;
		ndmaCh->wcnt = 512u / 4;
		ndmaCh->bcnt = NDMA_FASTEST;
		ndmaCh->cnt  = NDMA_EN | NDMA_START_TOSHSD3 | NDMA_TCNT_MODE |
		               NDMA_BURST(64u / 4) | NDMA_SAD_FIX | NDMA_DAD_INC;

		if(SDMMC_readSectors(SDMMC_DEV_CARD, sector, NULL, blockCount))
		{
			ndmaCh->cnt = 0; // Stop DMA on error.
			return RES_ERROR;
		}

		buff += 512u * blockCount;
		sector += blockCount;
		count -= blockCount;
	} while(count > 0u);

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	(void)pdrv;
	// TODO: fatfs will use memcpy() for sizes under 512 bytes
	//       but larger transfers run into this check if unaligned.
	if((uintptr_t)buff % 4u) return RES_PARERR;

	flushDCacheRange(buff, 512u * count);

	NdmaCh *const ndmaCh = getNdmaChRegs(5);
	do
	{
		const u16 blockCount = (count > 0xFFFFu ? 0xFFFFu : count);
		ndmaCh->sad  = (const u32)buff;
		ndmaCh->dad  = (u32)getToshsdFifo(getToshsdRegs(1)); // TODO: SDMMC dev to FIFO function.
		ndmaCh->tcnt = (u32)blockCount<<7;
		ndmaCh->wcnt = 512u / 4;
		ndmaCh->bcnt = NDMA_FASTEST;
		ndmaCh->cnt  = NDMA_EN | NDMA_START_TOSHSD3 | NDMA_TCNT_MODE |
		               NDMA_BURST(64u / 4) | NDMA_SAD_INC | NDMA_DAD_FIX;

		if(SDMMC_writeSectors(SDMMC_DEV_CARD, sector, NULL, blockCount))
		{
			ndmaCh->cnt = 0; // Stop DMA on error.
			return RES_ERROR;
		}

		buff += 512u * blockCount;
		sector += blockCount;
		count -= blockCount;
	} while(count > 0u);

	return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	(void)pdrv;

	DRESULT res = RES_OK;
	switch(cmd)
	{
		case GET_SECTOR_COUNT:
			*(DWORD*)buff = SDMMC_getSectors(SDMMC_DEV_CARD);
			break;
		case GET_SECTOR_SIZE:
			*(WORD*)buff = 512u;
			break;
		case GET_BLOCK_SIZE:
			*(DWORD*)buff = 0x100u; // Default to 128 KiB. TODO: Get this from the driver.
		case CTRL_TRIM:
			// TODO: Implement this.
		case CTRL_SYNC:
			break;
		default:
			res = RES_PARERR;
	}

	return res;
}
