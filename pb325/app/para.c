#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <litecore.h>
#include "system.h"
#include "para.h"
#include "alarm.h"
#include "meter.h"
#include "data.h"

//Private Defines
#define ICP_LOCK_ENABLE			0
#define ICP_MAGIC_WORD			0xFFFF5987

//Private Typedefs



//Private Consts
t_flash_dev icp_SfsDev = {
	FLASH_DEV_INT, 12, INTFLASH_BASE_ADR + 0x3C000,
};


//Private Variables
#if ICP_LOCK_ENABLE
static os_sem icp_sem;
#endif


//Private Macros
#if ICP_LOCK_ENABLE
#define icp_Lock()				rt_sem_take(&icp_sem, RT_WAITING_FOREVER)
#define icp_Unlock()			rt_sem_release(&icp_sem)
#else
#define icp_Lock()
#define icp_Unlock()
#endif


//Internal Functions
static int icp_Default(uint_t nAfn, uint_t nFn, uint_t nPn, uint8_t *pBuf, uint_t nLen)
{
	t_afn04_f3 *pF3;
	t_afn04_f33 *pF33;

	if (nAfn != 4)
		return -1;
	if (pBuf == NULL)
		return -1;

	memset(pBuf, 0, nLen);
	switch (nFn) {
	case 1:
		memcpy(pBuf, "\x14\x02\x1E\x30\x00\x05", nLen);
		break;
	case 3:
		pF3 = (t_afn04_f3 *)pBuf;
		pF3->ip1[0] = 10;
		pF3->ip1[1] = 11;
		pF3->ip1[2] = 50;
		pF3->ip1[3] = 154;
		pF3->port1 = 7916;
		pF3->ip2[0] = 0;
		pF3->ip2[1] = 0;
		pF3->ip2[2] = 0;
		pF3->ip2[3] = 0;
		pF3->port2 = 0;
		sprintf(pF3->apn, "mydyj.sc");
		break;
	case 5:
		break;
	case 8:
		memcpy(pBuf, "\x11\x78\x00\x05\x03\xFF\xFF\xFF", nLen);
		break;
	case 9:
		memcpy(pBuf, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x0B\x3E\x80\x80\x00\x00\x00\x00", nLen);
		break;
	case 21:
		memset(&pBuf[0], 3, 14);	//00:00-07:00（费率4）谷
		memset(&pBuf[14], 1, 8);	//07:00-11:00（费率2）峰
		memset(&pBuf[22], 2, 16);	//11:00-19:00（费率3）平
		memset(&pBuf[38], 1, 8);	//18:00-23:00（费率2）峰
		memset(&pBuf[46], 3, 2);	//23:00-00:00（费率4）谷
		pBuf[48] = 4;
		break;
	case 26:
		memcpy(pBuf, "\x20\x24\x80\x20\x00\x10\x00\x26\x02\x50\x00\x00\x18\x02\x50\x00\x00\x60\x00\x02\x50\x00\x00\x15\x00\x02\x50\x00\x00\x10\x00\x02\x50\x00\x00\x50\x01\x02\x50\x00\x00\x00\x01\x02\x50\x00\x00\x01\x02\x50\x00\x00\x01\x02\x50\x00\x05", nLen);
		break;
	case 28:
		memcpy(pBuf, "\x00\x08\x50\x09", nLen);
		break;
	case 33:
		pF33 = (t_afn04_f33 *)pBuf;
		switch (nPn) {
		case ECL_PORT_RS485:
		case ECL_PORT_PLC:
			pF33->span = 1;
			pF33->qty = 1;
			pF33->timeinfo[0] = 0x23500010;
			break;
		default:
			return -1;
		}
		break;
	case 34:
		memcpy(pBuf, "\x03\x01\x6B\x60\x09\x00\x00\x02\x6B\x60\x09\x00\x00\x1F\x6B\x60\x09\x00\x00", nLen);
		break;
	case 36:
		memcpy(pBuf, "\x00\x36\x6E\x01", nLen);
		break;
	case 59:
		memcpy(pBuf, "\x99\x99\xFF\x02", nLen);
		break;
	case 203:
		*pBuf = 0;
		break;
	default:
		return -1;
	}
	return icp_ParaWrite(nAfn, nFn, nPn, pBuf, nLen);
}








//External Functions
int icp_ParaRead(uint_t nAfn, uint_t nFn, uint_t nPn, void *pBuf, uint_t nLen)
{
	int res = -1;
	uint32_t nFnid;

	icp_Lock();
	nFnid = (nAfn << 12) | (nFn << 4);
	nFnid = (nFnid << 16) | nPn;
	if (sfs_Read(&icp_SfsDev, nFnid, pBuf) == SYS_R_OK)
		res = nLen;
	icp_Unlock();
	if (res < 0)
		res = icp_Default(nAfn, nFn, nPn, pBuf, nLen);
	return res;
}

int icp_ParaWrite(uint_t nAfn, uint_t nFn, uint_t nPn, const void *pBuf, uint_t nLen)
{
	uint32_t nFnid;

	icp_Lock();
	nFnid = (nAfn << 12) | (nFn << 4);
	nFnid = (nFnid << 16) | nPn;
	sfs_Write(&icp_SfsDev, nFnid, pBuf, nLen);
	icp_Unlock();
	return nLen;
}

int icp_MeterRead(uint_t nSn, t_afn04_f10 *p)
{

	return icp_ParaRead(4, 10, nSn, p, sizeof(t_afn04_f10));
}

void icp_MeterWrite(uint_t nSn, const t_afn04_f10 *p)
{

	icp_ParaWrite(4, 10, nSn, p, sizeof(t_afn04_f10));
}

int icp_Meter4Tn(uint_t nTn, t_afn04_f10 *p)
{
	uint_t i;

	if (nTn == TERMINAL) {
		p->port = ECL_PORT_ACM;
		return 1;
	}
	if (icp_MeterRead(nTn, p) > 0)
		if (p->tn == nTn)
			return nTn;
	for (i = 1; i < ECL_SN_MAX; i++) {
		if (icp_MeterRead(i, p) > 0)
			if (p->tn == nTn)
				break;
	}
	if (i < ECL_SN_MAX)
		return i;
	return 0;
}

uint_t icp_GetVersion()
{
	uint_t nVer = 0;

	icp_Lock();
	sfs_Read(&icp_SfsDev, ICP_MAGIC_WORD, &nVer);
	icp_Unlock();
	return nVer;
}

void icp_SetVersion()
{
	uint_t nVer = VER_SOFT;

	icp_Lock();
	sfs_Write(&icp_SfsDev, ICP_MAGIC_WORD, &nVer, 2);
	icp_Unlock();
}

void icp_Format()
{
	uint_t nVer = VER_SOFT;
	t_afn04_f85 xF85;

	icp_ParaRead(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
	icp_Lock();
	sfs_Init(&icp_SfsDev);
	sfs_Write(&icp_SfsDev, ICP_MAGIC_WORD, &nVer, 2);
	icp_Unlock();
	icp_ParaWrite(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
}

void icp_Clear()
{
	t_afn04_f1 xF1;
	t_afn04_f3 xF3;

	icp_ParaRead(4, 1, TERMINAL, &xF1, sizeof(t_afn04_f1));
	icp_ParaRead(4, 3, TERMINAL, &xF3, sizeof(t_afn04_f3));
	icp_Format();
	icp_ParaWrite(4, 1, TERMINAL, &xF1, sizeof(t_afn04_f1));
	icp_ParaWrite(4, 3, TERMINAL, &xF3, sizeof(t_afn04_f3));
}

void icp_Init()
{
	uint_t nVer;

#if ICP_LOCK_ENABLE
	rt_sem_init(&icp_sem, "sem_icp", 1, RT_IPC_FLAG_FIFO);
#endif
	if (sfs_Read(&icp_SfsDev, ICP_MAGIC_WORD, &nVer) != SYS_R_OK)
		icp_Format();
}

void icp_UdiskLoad(void)
{
	t_afn04_f1 xF1;
	t_afn04_f3 xF3;
	t_afn04_f85 xF85;
	char str[128];
	int fd;
	uint_t ulen, ui;

    BEEP(1);
	icp_ParaRead(4, 1, TERMINAL, &xF1, sizeof(t_afn04_f1));
	icp_ParaRead(4, 3, TERMINAL, &xF3, sizeof(t_afn04_f3));
	icp_ParaRead(4, 85, TERMINAL, &xF85, sizeof(t_afn04_f85));
	sprintf(str, FS_USBMSC_PATH"%04X%04X/pb325_cf.ini", xF85.area, xF85.addr);
	fd = fs_open(str, O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd >= 0) {
		fs_write(fd, str, sprintf(str, "[para]\r\nIP=%03d.%03d.%03d.%03d\r\nPORT=%05d\r\nHeartbeat=%02d\r\nAPN=%s", xF3.ip1[0], xF3.ip1[1], xF3.ip1[2], xF3.ip1[3], xF3.port1, xF1.span, xF3.apn));
		fs_close(fd);
	}
	fd = fs_open(FS_USBMSC_PATH"pb325_cf.ini", O_RDONLY, 0);
	if (fd >= 0) {
	    ulen = fs_read(fd, str, sizeof(str)) ; 
        xF1.span = bin2bcd8(atoi(&str[50]));
        xF3.ip1[0] = (atoi(&str[11]));
        xF3.ip1[1] = (atoi(&str[15]));
        xF3.ip1[2] = (atoi(&str[19]));
        xF3.ip1[3] = (atoi(&str[23]));
        xF3.port1 = (atoi(&str[33]));
        for(ui = 0; ui < (ulen-58); ui++)
            xF3.apn[ui] = str[58 + ui];
        xF3.apn[(ulen-58)] = 0;
    	icp_ParaWrite(4, 1, TERMINAL, &xF1, sizeof(t_afn04_f1));
    	icp_ParaWrite(4, 3, TERMINAL, &xF3, sizeof(t_afn04_f3));
		fs_close(fd);
	}
    os_thd_Sleep(1000);
    BEEP(0);
}

