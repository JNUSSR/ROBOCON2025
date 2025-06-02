#ifndef __BERXEL_CONTEXT_H__
#define __BERXEL_CONTEXT_H__

#include <BerxelHawkPlatform.h>
#include <BerxelHawkDefines.h>


namespace berxel
{

class BerxelHawkDevice;

class BERXEL_HAWK_API_EXPORT BerxelHawkContext
{
public:
    BerxelHawkContext();
    static BerxelHawkContext* getBerxelContext();
	static BerxelHawkContext* getBerxelContext(const char* strLogPath);
    static void destroyBerxelContext(BerxelHawkContext* &pBerxelContext);
public:
    virtual int32_t getDeviceList(BerxelHawkDeviceInfo** pDeviceList, uint32_t* pDeviceCount) = 0;
    virtual BerxelHawkDevice* openDevice(const BerxelHawkDeviceInfo deviceInfo) = 0;
    virtual int32_t closeDevice(BerxelHawkDevice* hawkDevice) = 0;
    virtual int32_t setDeviceStateCallback(BerxelHawkDeviceStatusChangeCallback callback, void* pData) = 0;
protected:
    virtual ~BerxelHawkContext();
};

}

#endif
