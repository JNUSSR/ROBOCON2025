#ifndef __BERXEL_HAWK_DEFINES_H__
#define __BERXEL_HAWK_DEFINES_H__
//
#include <stdint.h>
//#include "BerxelHawkFrame.h"
//
namespace berxel
{

class BerxelHawkFrame;
#define MAX_SN_SIZE 32

typedef enum {
	BERXEL_HAWK_PIXEL_TYPE_IMAGE_RGB24                = 0x00,
	BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_12I_4D           = 0x01, 
	BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D           = 0x02, 
	BERXEL_HAWK_PIXEL_TYPE_IR_16BIT                   = 0x03,
	BERXEL_HAWK_PIXEL_INVALID_TYPE					  = 0xff,
}BerxelHawkPixelType ;



typedef enum {
	BERXEL_HAWK_COLOR_STREAM							= 0x01,
	BERXEL_HAWK_DEPTH_STREAM							= 0x02,
	BERXEL_HAWK_IR_STREAM								= 0x04,
	BERXEL_HAWK_LIGHT_IR_STREAM							= 0x20,
	BERXEL_HAWK_INVALID_STREAM							= 0xff,
}BerxelHawkStreamType;



typedef enum 
{
	BERXEL_HAWK_SINGULAR_STREAM_FLAG_MODE				= 0x01,	
	BERXEL_HAWK_MIX_STREAM_FLAG_MODE				    = 0x02,
	BERXEL_HAWK_MIX_HD_STREAM_FLAG_MODE				    = 0x03,
	BERXEL_HAWK_MIX_QVGA_STREAM_FLAG_MODE				= 0x04,	

}BerxelHawkStreamFlagMode;


typedef enum {
	BERXEL_HAWK_DEVICE_CONNECT 	  = 0x00,
	BERXEL_HAWK_DEVICE_DISCONNECT = 0x01
} BerxelHawkDeviceStatus;

typedef enum {
	BERXEL_HAWK_DEVICE_ISOC_MODE = 0x01,
	BERXEL_HAWK_DEVICE_BULK_MODE = 0x02,
}BerxelHawkUVCMode;

typedef enum {
	BERXEL_HAWK_SYNC_TIME_REALTIME = 0x00,
	BERXEL_HAWK_SYNC_TIME_MONITOR_RAW = 0x01,
}BerxelHawkSyncTimeType;

typedef enum {
	BERXEL_HAWK_UPGRADE_START					  = 0x00,
	BERXEL_HAWK_UPGRADE_ENTER_DFU_MODE_FAILED     = 0x01,
	BERXEL_HAWK_UPGRADE_ENTER_DFU_SUCCESS		  = 0x02,
	BERXEL_HAWK_UPGRADE_DOWNLOAD_FILE_SUCCEED	  = 0x03,
	BERXEL_HAWK_UPGRADE_DOWNLOAD_FILE_FAILED	  = 0x04,
	BERXEL_HAWK_UPGRADE_PROCESSING		          = 0x05,
	BERXEL_HAWK_UPGRADE_SUCCESS				      = 0x06,
	BERXEL_HAWK_UPGRADE_FAILED				      = 0x07,

}BERXEL_HAWK_UPGRADE_STATUS;

typedef enum
{
	BERXEL_HAWK_DEVICE_UNKNOW = 0x00,
	BERXEL_HAWK_DEVICE_UVC = 0x01,
	BERXEL_HAWK_DEVICE_NET_100M = 0x02,
	BERXEL_HAWK_DEVICE_NET_1000M = 0x03,
	BERXEL_HAWK_DEVICE_SONIX = 0x04,
}BERXEL_DEVICE_TYPE;

typedef enum
{
	BERXEL_HAWK_UVC_ISOC_BANDWIDTH_DEFAULT = 0x01,
	BERXEL_HAWK_UVC_ISOC_BANDWIDTH_24MB = 0x02,
	BERXEL_HAWK_UVC_ISOC_BANDWIDTH_16MB = 0x03,
	BERXEL_HAWK_UVC_ISOC_BANDWIDTH_8MB = 0x04,
	BERXEL_HAWK_UVC_ISOC_BANDWIDTH_4MB = 0x05,
	BERXEL_HAWK_UVC_ISOC_BANDWIDTH_2MB = 0x06,
	BERXEL_HAWK_UVC_ISOC_BANDWIDTH_1MB = 0x07,
}BerxelHawkDeviceBandwidth;

typedef struct _BerxelHawkDeviceInfo {
	uint16_t vendorId;
	uint16_t productId;
	uint32_t deviceNum;
	uint32_t deviceType;
	uint32_t devBus;
	// uint32_t devPort;
	char	 devicePort[32];
	char     serialNumber[MAX_SN_SIZE];
	char     deviceAddress[255];
	char	 deviceLocation[255];
} BerxelHawkDeviceInfo;

typedef struct _BerxelHawkDeviceTemperature
{
	float rxTemperature;
	float pcbTemperature;
	float radTemperature;
	int32_t tx_tec_id;
	int32_t rx_tex_id;
}BerxelHawkDeviceTemperature;

typedef struct _BerxelFrameMode {
	BerxelHawkPixelType pixelType;
	int16_t resolutionX;
	int16_t resolutionY;
	int8_t  framerate;
} BerxelHawkStreamFrameMode;


#pragma pack (push, 1)

typedef struct _BerxelSdkVersion {
	uint16_t major;
	uint16_t minor;
	uint16_t revision;
} BerxelHawkSdkVersion;

typedef struct _BerxelFwVersion {
	uint16_t major;
	uint16_t minor;
	uint16_t revision;
	char chipVersion[64];
} BerxelHawkFwVersion;



typedef struct _BerxelHwVersion {
	uint16_t major;
	uint16_t minor;
	uint16_t revision;
} BerxelHawkHwVersion;

typedef struct {
	BerxelHawkSdkVersion sdkVersion;
	BerxelHawkFwVersion  fwVersion;
	BerxelHawkHwVersion  hwVersion;
} BerxelHawkVersions;


typedef struct _BerxelHawkPoint3D
{
	float x;
	float y;
	float z;
}BerxelHawkPoint3D;


typedef struct _BerxelHawkPoint2D
{
	uint32_t x;
	uint32_t y;

}BerxelHawkPoint2D;


typedef struct _BerxelHawkCameraIntrinsic
{
	float fxParam;  
	float fyParam;  
	float cxParam;  
	float cyParam;  
	float k1Param;  
	float k2Param; 
	float p1Param; 
	float p2Param; 
	float k3Param;  
}BerxelHawkCameraIntrinsic;


typedef struct _BerxelHawkIntrinsicInfo
{
	int8_t   colorIntrinsicParams[36];       //36 bytes 9个float
	int8_t   irIntrinsicParams[36];          //36 bytes 9个float
	int8_t   liteIrIntrinsicParams[36];      //36 bytes 9个float
	int8_t   rotateIntrinsicParams[36];      //36 bytes 9个float
	int8_t   translationIntrinsicParams[12]; //12 bytes 3个float
	
}BerxelHawkDeviceIntrinsicParams;

typedef struct _BerxelHawkNetParams
{
	uint8_t  static_ip;    //0 dhcp client, 1 static ip
	uint32_t ip_addr;
	uint32_t net_mask; //子网掩码
	uint32_t gw_addr;	//网关
	uint32_t dns_addr;
	uint8_t  dhcps_enable; //dhcp server, 0 disable 1 enable
	uint32_t dhcps_saddr;	//dhcp server a 服务端地址
	uint32_t dhcps_eaddr;	//dhcp server e 服务端地址
	uint8_t  reserved[256 - 27];
	uint8_t  user_setting;
}BerxelHawkNetParams;


typedef struct _BerxelHawkFilterRange {
	int32_t leftX;
	int32_t rightX;
	int32_t groundY;

} BerxelHawkFilterRange;

#pragma pack(pop)

#if defined(_WIN32)
typedef void (_stdcall * BerxelHawkNewFrameCallBack) (BerxelHawkStreamType streamType, BerxelHawkFrame* pFrame, void* pUserData);
typedef void (_stdcall * BerxelHawkDeviceStatusChangeCallback) (const char* deviceAddress, const char* deviceSerialNumber, BerxelHawkDeviceStatus deviceState, void* pUserData);
typedef void (_stdcall * BerxelHawkUpgradeProcessCallBack) (BERXEL_HAWK_UPGRADE_STATUS statusID, float progress,   void* pUserData);
#else
typedef void (* BerxelHawkNewFrameCallBack) (BerxelHawkStreamType streamType,BerxelHawkFrame* pFrame,  void* pUserData);
typedef void (* BerxelHawkDeviceStatusChangeCallback) (const char* deviceAddress, const char* deviceSerialNumber, BerxelHawkDeviceStatus deviceState, void* pUserData);
typedef void (* BerxelHawkUpgradeProcessCallBack) (BERXEL_HAWK_UPGRADE_STATUS statusID, float progress, void* pUserData);
#endif

const uint32_t BerxelHawkColorGainTable[] = {
	100,101,103,104,106,107,109,110,112,114,115,117,118,120,121,123,125,126,
	128,129,131,132,134,135,137,139,140,142,143,145,146,148,150,151,153,154,156,
	157,159,160,162,164,165,167,168,170,171,173,175,176,178,179,181,182,184,
	185,187,189,190,192,193,195,196,198,200,203,206,209,212,215,218,221,225,228,
	231,234,237,240,243,246,250,253,256,259,262,265,268,272,276,280,284,289,
	293,297,301,306,310,314,318,323,327,331,335,340,344,348,352,357,361,365,369,
	374,378,382,386,391,395,399,403,408,412,416,420,425,429,433,437,442,446,
	450,454,459,463,467,471,476,480,484,488,493,497,501,505,510,514,518,522,527,
	531,535,539,544,552,561,569,578,586,595,603,612,620,629,637,646,654,663,
	671,680,688,697,705,714,722,731,739,748,756,765,773,782,790,799,807,816,824,
	833,841,850,858,867,875,884,892,901,909,918,926,935,943,952,960,969,977,
	986,994,1003,1011,1020,1028,1037,1045,1054,1062,1071,1079,1088,1105,1122,1139,
	1156,1173,1190,1207,1224,1241,1258,1275,1292,1309,1326,1343,1360,1377,1394,
	1411,1428,1445,1462,1479,1496,1513,1530,1547,1564,1581,1598,1615,1632,1649,1666,
	1683,1700,1717,1734,1751,1768,1785,1802,1819,1836,1853,1870,1887,1904,1921,
	1938,1955,1972,1989,2006,2023,2040,2057,2074,2091,2108,2125,2142,2159,2176,2210,
	2244,2278,2312,2346,2380,2414,2448,2482,2516,2550,2584,2618,2652,2686,2720,
	2754,2788,2822,2856,2890,2924,2958,2992,3026,3060,3094,3128,3162,3196,3230,3264,
	3298,3332,3366,3400,3434,3468,3502,3536,3570,3604,3638,3672,3706,3740,3774,
	3808,3842,3876,3910,3944,3978,4012,4046,4080,4114,4148,4182,4216,4250,4284,4318,
};

}
//
#endif
