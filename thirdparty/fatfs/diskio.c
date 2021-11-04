/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
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
#include "drivers/mmc/sdmmc.h"
#include "arm9/drivers/ndma.h"
#include "arm.h"
#include "drivers/cache.h"



/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	(void)pdrv;

	DSTATUS stat = 0;

	if(!TOSHSD_cardDetected()) stat = STA_NODISK;

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive number to identify the drive */
)
{
	(void)pdrv;

	DSTATUS stat = 0;

	if(SDMMC_init(SDMMC_DEV_SLOT)) stat = STA_NOINIT;

	return stat;
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

	if((uintptr_t)buff & 3u) return RES_PARERR; // TODO: Find a solution for the alignment problem.

	// Warning! Flush before transfer only works on ARM9 (no speculative prefetching)!
	flushDCacheRange(buff, 512u * count);
	NdmaCh *const ndmaCh = getNdmaChRegs(5);
	ndmaCh->sad  = (u32)getToshsdFifo(getToshsdRegs(1));
	ndmaCh->dad  = (u32)buff;
	ndmaCh->tcnt = count<<7;
	ndmaCh->wcnt = 512 / 4;
	ndmaCh->bcnt = NDMA_FASTEST;
	ndmaCh->cnt  = NDMA_EN | NDMA_START_TOSHSD3 | NDMA_TCNT_MODE | // TODO: IRQ enable missing?
	               NDMA_BURST(64 / 4) | NDMA_SAD_FIX | NDMA_DAD_INC;
	if(SDMMC_readSectors(SDMMC_DEV_SLOT, sector, NULL /*(u32*)buff*/, count))
	{
		ndmaCh->cnt = 0; // Stop DMA on error.
		return RES_ERROR;
	}
	do
	{
		__wfi();
	} while(ndmaCh->cnt & NDMA_EN); // Wait needed (tested).

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

	if((uintptr_t)buff & 3u) return RES_PARERR; // TODO: Find a solution for the alignment problem.

	flushDCacheRange(buff, 512u * count);
	NdmaCh *const ndmaCh = getNdmaChRegs(5);
	ndmaCh->sad  = (u32)buff;
	ndmaCh->dad  = (u32)getToshsdFifo(getToshsdRegs(1));
	ndmaCh->tcnt = count<<7;
	ndmaCh->wcnt = 512 / 4;
	ndmaCh->bcnt = NDMA_FASTEST;
	ndmaCh->cnt  = NDMA_EN | NDMA_START_TOSHSD3 | NDMA_TCNT_MODE | // TODO: IRQ enable missing?
	               NDMA_BURST(64 / 4) | NDMA_SAD_INC | NDMA_DAD_FIX;
	if(SDMMC_writeSectors(SDMMC_DEV_SLOT, sector, NULL /*(u32*)buff*/, count))
	{
		ndmaCh->cnt = 0; // Stop DMA on error.
		return RES_ERROR;
	}
	do
	{
		__wfi();
	} while(ndmaCh->cnt & NDMA_EN); // Wait needed (tested).

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
			*(DWORD*)buff = SDMMC_getSectors(SDMMC_DEV_SLOT);
			break;
		case GET_SECTOR_SIZE:
			*(WORD*)buff = 512;
			break;
		case GET_BLOCK_SIZE:
			*(DWORD*)buff = 0x100; // Default to 128 KiB. TODO: Get this from the driver.
		case CTRL_TRIM:
			// TODO: Implement this.
		case CTRL_SYNC:
			break;
		default:
			res = RES_PARERR;
	}

	return res;
}
