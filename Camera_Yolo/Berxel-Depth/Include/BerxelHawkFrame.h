#ifndef __BERXEL_HAWK_FRAME_H__
#define __BERXEL_HAWK_FRAME_H__

#include <BerxelHawkPlatform.h>
#include <BerxelHawkDefines.h>

namespace berxel
{

class BERXEL_HAWK_API_EXPORT BerxelHawkFrame
{
public:
    BerxelHawkFrame();
    virtual ~BerxelHawkFrame();

public:
    virtual BerxelHawkPixelType getPixelType() = 0;
    virtual BerxelHawkStreamType getStreamType() = 0;
    virtual uint32_t getFrameIndex() = 0;
    virtual uint64_t getTimeStamp() = 0;
    virtual uint32_t getFPS() = 0;
    virtual uint32_t getWidth() = 0;
    virtual uint32_t getHeight() = 0;
    virtual void*    getData() = 0;
    virtual uint32_t getDataSize() = 0;
	virtual int32_t  getTxTemperature() = 0;
};



}

#endif
