/******************************************************************************
* CopyRIGHT(C):
* FileName:      bsp_gps.c
* Author:        huangjl
* Date：        11-22-2013
* OverView:ublox interface for configuration
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include "bsp_gps.h"
#include "bspapp_com.h"

UBLOX_SData gUbloxSData;

void Set_Speed(int fd, int speed)
{ 
	int i;  
	int status;  
	struct termios Opt; 
	tcgetattr(fd, &Opt);
	
	tcflush(fd, TCIOFLUSH);
	cfsetispeed(&Opt, speed);
	cfsetospeed(&Opt, speed);
	status = tcsetattr(fd, TCSANOW, &Opt);
	if (status != 0) 
	{  
		perror("tcsetattr fd");
		return;
	}  
	tcflush(fd,TCIOFLUSH);
}

int gps_series_fd = -1;
int SetgpsSerial()
{
   	struct termios Opt;

	gps_series_fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
 	if (gps_series_fd == -1)  
	{  
		perror("Can't Open Serial Port\n"); 
		return APP_ERROR;  
	}
	
	if(tcgetattr(gps_series_fd, &Opt) != 0)
	{
		perror("tcgetattr fd\n");
		return APP_ERROR;
	}

	Opt.c_cflag &= ~CSIZE;
	Opt.c_cflag |= CS8;

	Opt.c_cflag |= (CLOCAL | CREAD);

	Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	Opt.c_oflag &= ~OPOST;
	Opt.c_oflag &= ~(ONLCR | OCRNL); 

	Opt.c_iflag &= ~(ICRNL | INLCR);
	Opt.c_iflag &= ~(IXON | IXOFF | IXANY); 

	tcflush(gps_series_fd, TCIFLUSH);
	Opt.c_cc[VTIME] = 0; 
	Opt.c_cc[VMIN] = 0; 

	if(tcsetattr(gps_series_fd, TCSANOW, &Opt) != 0)
	{
		perror("tcsetattr g_IntGpsUartFd\n");
		return APP_ERROR;
	}

//	Set_Speed(gps_series_fd, 9600);
	return APP_OK;  
}

UINT32 UART3_SendData(UINT8 *SendBuf, UINT32 len)
{
	int nWriteByte =0;
	
	nWriteByte = write(gps_series_fd, SendBuf, len);

	if(nWriteByte != len)
	{  
		LOG_DEBUG("writing length error!.....write count=%d, all length=%d\n", nWriteByte, len); 
		return ERROR;  
	}	
	return nWriteByte;
}

#define 			GPS_MAX_CMD_LEN  	400

UINT32 Uart3Read (UINT8 *buffer, int count)
{
	int nReadByte =0;
	int delay = 0;
	int readcnt = 0;


#if 0
    while(delay++ < 0x100000)
    {
       if(count >=GPS_MAX_CMD_LEN)
       		break;
       	
       if((U3LSR & 0x01) == 0x01) 
       {
         	buffer[ncnt++] = (uint8)U3RBR;     
       }      
    }

#endif

    while(delay++ < 0x100000)
    {
    	nReadByte = read(gps_series_fd, buffer+readcnt, count-readcnt);
		readcnt +=nReadByte;
		if(readcnt >= count)
			break;
    }
	return readcnt;
}

int Ublox_ExeCommand(UINT8 *msg, const UINT8 length, char *pOutPutResponse, UINT8 resplen)

{  
    UINT8 response[GPS_MAX_RESP_LENGTH];
    int i, m, len, count;
    BOOL done;
    int retry  = GPS_SEND_RECEIVE_RETRY_COUNT;
    int status = OK;
   
    done = FALSE;
    do {
        status = OK;

        // Send the command to the GPS
        len = length;
        count = UART3_SendData((UINT8*)msg, len);
        
        if(count!=len)
        {
            LOG_DEBUG("\r\ngps writing len error!.\n");
            status = ERR_GPS_TIMED_OUT;
            break;
        }  

        if(resplen==0)//不需要等返回值
            done = TRUE;

        while (!done)
        {
           count = Uart3Read (response, resplen);

           if (count == 0) 
           {
               status = ERR_GPS_TIMED_OUT;
               break;
           }
            
            for(i=0; i<count; i++)//找到应答命令头
            {
                if(response[i]==0xb5)
                    break;
            }
            if((i==count)||((count-i)<= 8))//没有找到或长度不对，丢弃
            {
               status = ERR_GPS_TIMED_OUT;
               break;
            } 
            
            for(m=i; m<4; m++)
            {
                if(response[m]!=msg[m])
                    break;
            }
            if ((m==4)||((response[i]==0xb5)&&(response[i+1]==0x62)&&(response[i+2]==0x05)&&(response[i+3]==0x01)))//是查询应答
            {
                len = response[5+i] * 0x100 + response[4+i];
                if((count-i)<len)//没有取到全部消息，丢弃
                {
                    status = ERR_GPS_TIMED_OUT;
                    break;
                }
                done   = TRUE;
                status = OK;
                if (NULL != pOutPutResponse)
                {                    
                    memcpy(pOutPutResponse, &response[i], len+8);
                }                
            }
            else
            {
                status = ERR_GPS_TIMED_OUT;    
                break;
            }           
        } // !done getting response
    // Send the command again if we timed-out
    } while ((!done) && (status == ERR_GPS_TIMED_OUT) && retry--);
    return (status);
}

struct T_GpsAllData gUbloxGpsAllData;

STATUS gps_NAV_TimeUTC()
{
    UINT8 data[] = {0xB5, 0x62, 0x01, 0x21, 0, 0, 0x22, 0x67};
    char dataRsp[GPS_MAX_RESP_LENGTH];
    int status ;

    memset(dataRsp, 0, GPS_MAX_RESP_LENGTH);
    status = Ublox_ExeCommand(data, 8, (char*)dataRsp, 20+6);
    if(status==OK)
    {        
        char *pchar = &dataRsp[6];
        if(pchar[19]&0x07 == 0x07)//utc valid
        {
            gUbloxGpsAllData.Year = pchar[13]*0x100+pchar[12];//小端模式
            gUbloxGpsAllData.Month = pchar[14];
            gUbloxGpsAllData.Day = pchar[15];
            gUbloxGpsAllData.Hour = pchar[16];
            gUbloxGpsAllData.Minute = pchar[17];
            gUbloxGpsAllData.Second = pchar[18];

            LOG_DEBUG("[tGPS] gps_NAV_TimeUTC, msg is valid"); 
			return TRUE;
        }
    }
    return FALSE;   
}

STATUS gps_Close_GGA()
{
    UINT8 data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00
, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x23};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = Ublox_ExeCommand(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        	LOG_DEBUG("[tGPS] gps_Close_GGA, return failure!\n");         
    }
    else
           	LOG_DEBUG("[tGPS] gps_Close_GGA, SUCC!\n"); 
           	 
    return status; 
}

/*
*关闭NMEA的GLL命令
*/
STATUS gps_Close_GLL()
{
    UINT8 data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00
, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = Ublox_ExeCommand(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        LOG_DEBUG("[tGPS] gps_Close_GLL, return failure!\n");         
    }
    else
        LOG_DEBUG("[tGPS] gps_Close_GLL, SUCC!\n");         

    return status; 
}

/*
*关闭NMEA的GSA命令
*/
STATUS gps_Close_GSA()
{
    UINT8 data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00
, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31 };
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = Ublox_ExeCommand(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        LOG_DEBUG("[tGPS] gps_Close_GSAL, return failure!\n");         
    }
     else
        LOG_DEBUG("[tGPS] gps_Close_GSAL, SUCC!\n"); 
            
    return status; 
}

/*
*关闭NMEA的GSV命令
*/
STATUS gps_Close_GSV()
{
    UINT8 data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00
, 0x00, 0x00, 0x00, 0x00, 0x02, 0x38};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = Ublox_ExeCommand(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        LOG_DEBUG("[tGPS] gps_Close_GSV, return failure!\n");         
    }
    else
            LOG_DEBUG("[tGPS] gps_Close_GSV, SUCC\n");   
    return status; 
}

/*
*关闭NMEA的RMC命令
*/
STATUS gps_Close_RMC()
{
    UINT8 data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3F};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = Ublox_ExeCommand(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        	LOG_DEBUG("[tGPS] gps_Close_RMC, return failure!\n");         
    }
    else
            LOG_DEBUG("[tGPS] gps_Close_RMC, SUCC\n");         
    return status; 
}

/*
*关闭NMEA的VTG命令
*/
STATUS gps_Close_VTG()
{
    UINT8 data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x46};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = Ublox_ExeCommand(data, 16, dataRsp, 0);
    if(status!=OK)//failure, try again
    {
        LOG_DEBUG("[tGPS] gps_Close_UTG, return failure!\n");         
    }
    else
           LOG_DEBUG("[tGPS] gps_Close_UTG, SUCC\n"); 
           
    return status; 
}

/*
*关闭NMEA的ZDA命令
*/
STATUS gps_Close_ZDA()
{
    UINT8 data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5B};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = Ublox_ExeCommand(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        LOG_DEBUG("[tGPS] gps_Close_ZDA, return failure!\n");         
    }
    else
          LOG_DEBUG("[tGPS] gps_Close_ZDA, SUCC\n"); 
          
    return status; 
}
/*
*关闭SABS命令
*/
STATUS gps_Close_SABS()
{
    UINT8 data[] = {0xB5, 0x62, 0x06, 0x16, 0x08, 0x00, 0x00, 0x01, 0x01, 0x00
, 0x00, 0x00, 0x00, 0x00, 0x26, 0x97};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = Ublox_ExeCommand(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        LOG_DEBUG("[tGPS] gps_Close_SABS, return failure!\n");         
    }
    else
            LOG_DEBUG("[tGPS] gps_Close_SABS,SUCC!\n");       
    return status; 
}

//配置10ms
STATUS gps_Cfg_Tp1() 
{   
    UINT8 data[] = {
    0xb5, 0x62, 0x06, 0x31, 0x20, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 
    0xe8, 0x03, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0xb8, 0x0b, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0xf7, 0x00, 
    0x00, 0x00, 0x3d, 0x02};
    
    char dataRsp[GPS_MAX_RESP_LENGTH];
    int status;
//    printf("in gps_Cfg_Tp1dddddd\n");
    status = Ublox_ExeCommand(data, 40, dataRsp, 10);//10--->0
//    printf("in gps_Cfg_Tp1bbbbb\n");

    if(status!=OK)
    {
//        printf("in gps_Cfg_Tp1 failed\n");

        LOG_DEBUG("[tGPS] open gps_Cfg_Tp1, return failure!\n"); 
        return FALSE;
    }
    else if((dataRsp[2]==5)&&(dataRsp[3]==1))//ACK
    {
//        printf("in gps_Cfg_Tp1 succ\n");

        LOG_DEBUG("[tGPS] open gps_Cfg_Tp1, return OK\n"); 
        return TRUE;
    }
    else
        return FALSE;
    
}		

STATUS gps_Cfg_Tp2() 
{
   
    UINT8 data[] ={
    0xb5, 0x62, 0x06, 0x31, 0x20, 0x00, 
    0x01, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0f, 0x00, 0x40, 0x42, 0x0f, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x4c, 0x1d, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0xf7, 0x00, 0x00, 
    0x00, 0x62, 0xec};
   
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;
    status = Ublox_ExeCommand(data, 40, dataRsp, 10);
    if(status!=OK)
    {
        LOG_DEBUG("[tGPS] open gps_Cfg_Tp2, return failure!\n"); 
        return FALSE;
    }
    else if((dataRsp[2]==5)&&(dataRsp[3]==1))//ACK
    {
        LOG_DEBUG("[tGPS] open gps_Cfg_Tp2, return OK\n"); 
        return TRUE;
    }
    else
        return FALSE;

}


STATUS gps_MON_HW()
{
    UINT8 data[] = {0xB5, 0x62, 0x0A, 0x09, 0x00, 0x00, 0x13, 0x43 };
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status = ERROR;
	UINT8 ante_sta = 0, aPower_sta = 0;
    status = Ublox_ExeCommand(data, 8, dataRsp, 68+6);
    if(status==OK)
    {
        char *pInfo = &dataRsp[6];
        ante_sta = pInfo[20];
        aPower_sta = pInfo[21];
		
	if(ante_sta == 2 && aPower_sta== 1)
			return (status=OK);
		else
			return (status=ERROR);
    }
	else
	{
        LOG_DEBUG("[tGPS] gps_MON_HW, return failure!\n"); 
	}
    return status; 
}

STATUS gps_NAV_SVInfo(int *ntrackS)
{
    UINT8 data[10] = {0xB5, 0x62, 0x01, 0x30, 0x00, 0x00, 0x31, 0x94};
    char dataRsp[GPS_MAX_RESP_LENGTH];
    int status, i;
//    char numCh;
    int visiableS, trackS;
    int numChlen = 0, quality =0;
    visiableS = 0;
    trackS = 0;
    status = Ublox_ExeCommand(data, 8, dataRsp, 200);
    if(status==OK)
    {        
        UINT8*pInfo = (UINT8*)&dataRsp[6];//小端模式
        int len = 0;
        
        len += 4;
	  	numChlen = pInfo[len];
        len += 4;

        for(i=0; i<numChlen; i++)
        {
        
           len += 3;
           quality = pInfo[len];
	       len += 9; 
            if(quality>=3)//Signal detected but unusable, track
            {
                visiableS++;
                if(quality>=4)//lock
                {
                    trackS++;
                }
            }
        }
    }
	*ntrackS = trackS;
    return visiableS; 
}

STATUS get_gps_NAV_SVInfo()
{
	int lockntrack, vis;
	vis = gps_NAV_SVInfo(&lockntrack);

	LOG("get_gps_NAV_SVInfo-->visable:%d,lock:%d\n", vis, lockntrack);
	return TRUE;
}

STATUS gps_NAV_SVInfo_Prt()
{
    UINT8 data[10] = {0xB5, 0x62, 0x01, 0x30, 0x00, 0x00, 0x31, 0x94};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status, i;
    char numCh;
    int visiableS, trackS;
    visiableS = 0;
    trackS = 0;
    status = Ublox_ExeCommand(data, 8, dataRsp, 200);
    printf("\r\nPlease waiting for research for satellite.............\n");
    
    if(status==OK)
    {        
        UINT8*pInfo = (UINT8*)&dataRsp[6];//小端模式
        int len = 0;
        
 //  gUbloxSData.svInfo.iTow = pInfo[len+3]*0x1000000 + pInfo[len+2]*0x10000 + pInfo[len+1]*0x100 +pInfo[len];
       	gUbloxSData.svInfo.iTow = pInfo[len+3]<<24 + pInfo[len+2]<<16 + pInfo[len+1]<<8 +pInfo[len];

        len += 4;
        gUbloxSData.svInfo.numCh = pInfo[len];
        len += 1;
        gUbloxSData.svInfo.globalFlags = pInfo[len];
        len += 1;
  //  gUbloxSData.svInfo.res2 = pInfo[len+1]*0x100 +pInfo[len];
        gUbloxSData.svInfo.res2 = pInfo[len+1]<<8 +pInfo[len];
        
        printf("\r\n ####GPS number: %d ,and following is these satellite information\n####\n",gUbloxSData.svInfo.numCh);
        len += 2;
        
        for(i=0; i<gUbloxSData.svInfo.numCh; i++)
        {
     	   printf("\r\n-------------------------..\n");   
     	   gUbloxSData.svInfo.sInfo[i].chn = pInfo[len];
        	   printf("\r\n gps[%d] channel number: %d \n",i,pInfo[len]);

             
            len += 1;
            gUbloxSData.svInfo.sInfo[i].svid = pInfo[len];
        	   printf("\r\n gps[%d] satellite id: %d :.\n",i,pInfo[len]);
             
            len += 1;
            gUbloxSData.svInfo.sInfo[i].flags = pInfo[len];
            len += 1;
            gUbloxSData.svInfo.sInfo[i].quality = pInfo[len];
        	   printf("\r\n gps[%d] satellite quality: %d :.\n",i,pInfo[len]);
            
            len += 1;
            gUbloxSData.svInfo.sInfo[i].cno_db = pInfo[len];
         	   printf("\r\n gps[%d] Carrier to noise ratio: %d dbhz\n",i,pInfo[len]);
     	   
            len += 1;
            gUbloxSData.svInfo.sInfo[i].elev = pInfo[len];
            len += 1;
     //    gUbloxSData.svInfo.sInfo[i].azim = pInfo[len] * 0x100 + pInfo[len] ;
             gUbloxSData.svInfo.sInfo[i].azim = pInfo[len]<<8 + pInfo[len] ;

            len += 2;
     //       gUbloxSData.svInfo.sInfo[i].prRes = pInfo[len+3]*0x1000000 + pInfo[len+2]*0x10000 + pInfo[len+1]*0x100 +pInfo[len];
            gUbloxSData.svInfo.sInfo[i].prRes = pInfo[len+3]<<24 + pInfo[len+2]<<16 + pInfo[len+1]<<8 +pInfo[len];

            len += 4;
            if(gUbloxSData.svInfo.sInfo[i].quality>=3)//Signal detected but unusable, track
            {
                visiableS++;
                if(gUbloxSData.svInfo.sInfo[i].quality>=4)//lock
                {
                    trackS++;
                }
            }
            
        }
		
		printf("\r\n[GPS] VisiableS Satellite number: %d \n",visiableS);
		
		printf("\r\n[GPS] Tracked Satellite number: %d \n",visiableS);
		
     	printf("\r\n#########################################################\n");

        gUbloxGpsAllData.VisibleSatellites = visiableS;
        gUbloxGpsAllData.TrackedSatellites = trackS;
    }
    return visiableS; 
}

STATUS Config_Ublox()
{    
    BOOL status1;
    int count = 0;
    if(count < 1)
    {  
        gps_Close_GGA();
        usleep(1000);
        gps_Close_GLL();
        usleep(1000);
        gps_Close_GSA();
        usleep(1000);
        gps_Close_GSV();
        usleep(1000);
        gps_Close_RMC();
        usleep(1000);
        gps_Close_VTG();
        usleep(1000);
        gps_Close_ZDA();
        usleep(1000);
        gps_Close_SABS(); 
        usleep(1000);
        count++;
    }
    
    usleep(80000);
    status1 = gps_Cfg_Tp1();
    if(status1== FALSE)
        return FALSE;

    usleep(80000);
    status1 = gps_Cfg_Tp2();

    if(status1== FALSE )
        return FALSE;

	return TRUE;
}

