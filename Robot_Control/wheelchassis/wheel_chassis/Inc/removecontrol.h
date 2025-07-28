#ifndef REMOVECONTROL_H
#define REMOVECONTROL_H

#include "main.h"

#define LXChannel 4
#define LYChannel 3
#define RXChannel 1
#define RYChannel 2
#define VRAChannel 5
#define VRBChannel 6
#define SWAChannel 7
#define SWBChannel 8
#define SWCChannel 9
#define SWDChannel 10

typedef struct
{
    uint16_t Lx_Min;
    uint16_t Lx_Mid;
    uint16_t Lx_Max;
    uint16_t Ly_Min;
    uint16_t Ly_Max;
    uint16_t Rx_Min;
    uint16_t Rx_Mid;
    uint16_t Rx_Max;
    uint16_t Ry_Min;
    uint16_t Ry_Mid;
    uint16_t Ry_Max;
    uint16_t VRA_Min;
    uint16_t VRA_Max;
    uint16_t VRB_Min;
    uint16_t VRB_Max;
    uint16_t SWA_Close;
    uint16_t SWA_Open;
    uint16_t SWB_Close;
    uint16_t SWB_Open;
    uint16_t SWC_Close;
    uint16_t SWC_First;
    uint16_t SWC_Second;
    uint16_t SWD_Close;
    uint16_t SWD_Open;
    
}ChannelParameters;

typedef struct
{
    float Lx;//-100~0~100
    float Ly;//0~100
    float Rx;//-100~0~100
    float Ry;//-100~0~100
    float VRA;//0~100
    float VRB;//0~100
    uint8_t SWA;//0,1
    uint8_t SWB;//0,1
    uint8_t SWC;//0,1,2
    uint8_t SWD;//0,1

}ProcessedChannelValue;

void RemoveControl_Init(void);
void RemoveControl_ProcessData(uint16_t channeldata[14]);

#endif //REMOVECONTROL_H
