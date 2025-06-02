#ifndef __BERXEL_HAWK_DEVICE_H__
#define __BERXEL_HAWK_DEVICE_H__

#include <map>
#include <vector>
#include <BerxelHawkPlatform.h>
#include <BerxelHawkDefines.h>
#include <BerxelHawkFrame.h>


namespace berxel
{
class BERXEL_HAWK_API_EXPORT BerxelHawkDevice
{
public:
    BerxelHawkDevice();
    virtual ~BerxelHawkDevice();

public:
	virtual int32_t setStreamFlagMode(BerxelHawkStreamFlagMode flagMode) = 0;
    virtual int32_t startStreams(int streamFlags) = 0;
	virtual int32_t startStreams(int streamFlags, BerxelHawkNewFrameCallBack callBack, void* pUserData) = 0;
    virtual int32_t stopStreams(int streamFlags) = 0;
    virtual int32_t getSupportFrameModes(BerxelHawkStreamType streamType, const BerxelHawkStreamFrameMode** pModes, uint32_t* pCount) = 0;
    virtual int32_t setFrameMode(BerxelHawkStreamType streamType, BerxelHawkStreamFrameMode *mFrameMode) = 0;
    virtual int32_t getCurrentFrameMode(BerxelHawkStreamType streamType, BerxelHawkStreamFrameMode* frameMode) = 0;
    virtual int32_t readColorFrame(BerxelHawkFrame* &pFrame,int32_t timeout = 30) = 0;
	virtual int32_t readDepthFrame(BerxelHawkFrame* &pFrame,int32_t timeout = 30) = 0;
	virtual int32_t readIrFrame(BerxelHawkFrame* &pFrame,int32_t timeout = 30) = 0;
	virtual int32_t readLightIrFrame( BerxelHawkFrame* &pSteamFrame,int32_t timeout = 30) = 0;
    virtual int32_t releaseFrame(BerxelHawkFrame* &pHawFrame) = 0;
    virtual int32_t startUpgrade(BerxelHawkUpgradeProcessCallBack pCallbacks, void* pUserData, const char* pFwFilePath) = 0;
	virtual int32_t convertDepthToPointCloud(BerxelHawkFrame* pFrame , float factor, BerxelHawkPoint3D* pPointClouds, bool bPointCloudWall = false) = 0;
	virtual int32_t getVersion(BerxelHawkVersions* versions) = 0;
	virtual int32_t getCurrentDeviceInfo(BerxelHawkDeviceInfo* pDeviceInfo) = 0;
	virtual int32_t getCameraIntriscParams(BerxelHawkCameraIntrinsic *pParams ) = 0;
	virtual int32_t getDeviceIntriscParams(BerxelHawkDeviceIntrinsicParams *pParams ) = 0;
	virtual int32_t setStreamMirror(bool bNeedMirror) = 0;
	virtual int32_t setRegistrationEnable(bool bEnable) = 0;
	virtual int32_t setFactoryBurnEnable(bool bEnable) = 0;
	virtual int32_t setTemperatureCompensationEnable(bool bEnable) = 0;
	virtual int32_t setFrameSync(bool bEnable) = 0;
	virtual int32_t setFillHoleStatus(bool bEnable) = 0;
	virtual int32_t setDenoiseStatus(bool bEnable) = 0;
	virtual int32_t setSystemClock() = 0;
	virtual int32_t getSystemClock(uint32_t *sec, uint32_t* usec) = 0;
	virtual int32_t setNetParams(void* pData, uint32_t dataSize) = 0;
	virtual int32_t getNetParams(void* pData, uint32_t needDataSize) = 0;
	virtual int32_t setTemperatureParams(void* pData, uint32_t dataSize) = 0;
	virtual int32_t getTemperatureParams(void* pData,  uint32_t *dataSize) = 0;
	virtual int32_t openNoiseFilter(bool bEnable) = 0;
	virtual int32_t setDepthCloseRangeDefaultGainAndExposure() = 0;
	virtual int32_t setEdgeOptimizationStatus(bool bEnable) = 0;
	virtual int32_t setDepthRemoteMode(bool bEnable) = 0;
	virtual int32_t getDeviceHighPrecisionTemperature(float* temperature) = 0;
	virtual int32_t getDeviceRxHighPrecisionTemperature(BerxelHawkDeviceTemperature* temperature) = 0;
	virtual int32_t getDeviceTemperature(int32_t* temperature) = 0;
	virtual int32_t setAdbMode(bool bOpen) = 0;
	virtual int32_t setSafetyMode(bool bOpen) = 0;
	virtual int32_t setSafetyDistance(uint32_t nDistance) = 0; 
	virtual int32_t getSafetyDistance(uint32_t* nDistance) = 0; 
	virtual int32_t setArParams(void* pData, uint32_t dataSize, uint32_t Offset) = 0;
	virtual int32_t setCameraIntrisc(void* pData, uint32_t dataSize, uint32_t Offset) = 0;
	virtual int32_t setFactoryBurnFirmwareHawk300(void* pData, uint32_t dataSize, uint32_t Offset) = 0;
	virtual int32_t getArParams(void* pData, uint32_t needDataSize, uint32_t Offset) = 0;
	virtual int32_t getCameraIntrisc(void* pData, uint32_t dneeDataSize, uint32_t Offset) = 0;
	virtual int32_t resetUsb() = 0;
	virtual int32_t setChipDenoiseStatus(bool bEnable) = 0;
	virtual int32_t setSonixSerialNumber(void* pData, uint32_t dataSize) = 0;
	virtual int32_t getSonixSerialNumber(void* pData, uint32_t dataSize) = 0;
	virtual int32_t setColorExposureGain(uint32_t exposureTime, uint32_t gain) = 0;
	virtual int32_t getColorExposureGain(uint32_t* exposureTime, uint32_t* gain, uint8_t* aeStatus) = 0;
	virtual int32_t enableColorAutoExposure() = 0;
	virtual int32_t reconnectNetDevice() = 0;
	virtual int32_t setDepthElectricCurrent(uint32_t value) = 0;
	virtual int32_t getDepthElectricCurrent(uint32_t* value) = 0;
	virtual int32_t setDepthExposure(uint32_t value) = 0;
	virtual int32_t getDepthExposure(uint32_t* value) = 0;
	virtual int32_t setDepthGain(uint32_t value) = 0;
	virtual int32_t getDepthGain(uint32_t* value) = 0;
	virtual int32_t setDepthAEStatus(bool bEnable) = 0;
	virtual int32_t getDepthAEStatus(uint32_t* value) = 0;
	virtual int32_t enableDeviceSlaveMode(bool bEnable) = 0;
	virtual int32_t getDeviceMasterSlaveMode(uint32_t* value) = 0;
	virtual int32_t setColorQuality(uint32_t nValue) = 0;
	virtual int32_t getColorQuality(uint32_t* value) = 0;
	virtual int32_t setUserGpioCtrl(uint32_t gpio_nr, uint32_t gpio_number) = 0;
	virtual int32_t getUserGpioCtrl(uint32_t gpio_nr, uint32_t* gpio_number) = 0;
	virtual int32_t setCurveFilteringStatus(bool bEnable) = 0;
	virtual int32_t setDeviceBandwidth(BerxelHawkDeviceBandwidth bandwidth) = 0;
	virtual int32_t setStreamStatus(bool bEnable) = 0;
	virtual int32_t setBindCpuCore(uint32_t pData[], uint32_t nSize) = 0;
	virtual int32_t setDepthConfidence(uint32_t nValue) = 0;
	virtual int32_t getDepthConfidence(uint32_t* value) = 0;
	virtual int32_t enableHightFpsMode(bool bEnable) = 0;
	virtual int32_t setDepthTxStatus(bool bEnable) = 0;
	virtual int32_t setMaxDepthValue(uint32_t nValue) = 0;
	virtual int32_t setFilterGroundValue(BerxelHawkFilterRange range) = 0;

	virtual int32_t setTemperatureParamsFilePath(const char* filePath) = 0;
	virtual int32_t setDepthAEGainRange(uint32_t min, uint32_t max) = 0;
	virtual int32_t getDepthAEGainRange(uint32_t* min, uint32_t* max) = 0;
	virtual int32_t setDepthAEExposureRange(uint32_t min, uint32_t mid, uint32_t max) = 0;
	virtual int32_t getDepthAEExposureRange(uint32_t* min, uint32_t* mid, uint32_t* max) = 0;
	virtual int32_t getDeviceLogSize(uint32_t* size) = 0;
	virtual	int32_t getDeviceLog(void* pData, uint32_t dataSize) = 0;
	//virtual int32_t setDevHwThremalStatus(bool bEnable) = 0;
	virtual int32_t setDeviceTransferMode(BerxelHawkUVCMode mode) = 0;
	virtual int32_t setTemporalDenoiseStatus(bool bEnable) = 0;
	virtual int32_t setSpatialDenoiseStatus(bool bEnable) = 0;
	virtual int32_t setSyncHostTime(BerxelHawkSyncTimeType type) = 0;


	virtual int32_t rebootDevice() = 0;
	virtual bool	deviceSupportHwTempCompenSation() = 0;
	virtual int32_t setDevHwAutoTemCtrlStatus(bool bEnable) = 0;
	virtual int32_t getDevHwAutoTemCtrlStatus(uint32_t* value) = 0;
	virtual int32_t enableDecodeColorData(bool bEnable) = 0;

	virtual int32_t setDeviceWatchDogStatus(bool bEnable) = 0;
	virtual int32_t getDeviceWatchDogStatus(uint32_t* nValue) = 0;

	virtual int32_t setDeviceDepthGTFlash() = 0;
	virtual int32_t getDeviceDepthGTFlash(uint32_t* nValue) = 0;

	virtual int32_t setDeviceCDCStatus(bool bEnable) = 0;
	virtual int32_t getDeviceCDCStatus(uint32_t* nValue) = 0;

	virtual int32_t transparentTransmission(uint8_t* pSend, uint32_t nSendSize, uint8_t* pRecv, uint32_t* nRecvSize) = 0;
};

}

#endif
