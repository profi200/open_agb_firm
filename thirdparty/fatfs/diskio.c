/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "types.h"
#include "arm9/hardware/sdmmc.h"



/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	(void)pdrv;

	DSTATUS stat = 0;

	//if(!(sdmmc_read16(REG_SDSTATUS0) & TMIO_STAT0_SIGSTATE)) stat = STA_NODISK;

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

	sdmmc_init();
	if(SD_Init() != 0) stat = STA_NOINIT;

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	(void)pdrv;

	DRESULT res = RES_OK;

	if(sdmmc_sdcard_readsectors(sector, count, buff) != 0) res = RES_ERROR;

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	(void)pdrv;

	DRESULT res = RES_OK;

	if(sdmmc_sdcard_writesectors(sector, count, buff) != 0) res = RES_ERROR;

	return res;
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
			{
				*(DWORD*)buff = getMMCDevice(1)->total_size;
				break;
			}
			res = RES_NOTRDY;
			break;
		case GET_SECTOR_SIZE:
			*(WORD*)buff = 512;
			break;
		case GET_BLOCK_SIZE:
			*(DWORD*)buff = 0x100; // Default to 128 KiB.
		case CTRL_TRIM:
		case CTRL_SYNC:
			break;
		default:
			res = RES_PARERR;
	}

	return res;
}

