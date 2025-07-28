#include "removecontrol.h"

ChannelParameters G_ChannelParam = {0};
ProcessedChannelValue G_Value = {0};

void RemoveControl_Init(void)
{
    G_ChannelParam.Lx_Min = 1000;
    G_ChannelParam.Lx_Mid = 1500;
    G_ChannelParam.Lx_Max = 2000;
    G_ChannelParam.Ly_Min = 1000;
    G_ChannelParam.Ly_Max = 2000;
    G_ChannelParam.Rx_Min = 1000;
    G_ChannelParam.Rx_Mid = 1500;
    G_ChannelParam.Rx_Max = 2000;
    G_ChannelParam.Ry_Min = 1000;
    G_ChannelParam.Ry_Mid = 1500;
    G_ChannelParam.Ry_Max = 2000;
    G_ChannelParam.VRA_Min = 1000;
    G_ChannelParam.VRA_Max = 2000;
    G_ChannelParam.VRB_Min = 1000;
    G_ChannelParam.VRB_Max = 2000;
    G_ChannelParam.SWA_Close = 1000;
    G_ChannelParam.SWA_Open = 2000;
    G_ChannelParam.SWB_Close = 1000;
    G_ChannelParam.SWB_Open = 2000;
    G_ChannelParam.SWC_Close = 1000;
    G_ChannelParam.SWC_First = 1500;
    G_ChannelParam.SWC_Second = 2000;
    G_ChannelParam.SWD_Close = 1000;
    G_ChannelParam.SWD_Open = 2000;
}

float RemoveControl_math(uint16_t mid,uint16_t min,uint16_t max,uint16_t channeldata)
{
    if (mid == 0)
    {
        return (float)(channeldata - min) / (float)(max - min);
    }
    else
    {
        return channeldata - mid >= 0 ? (float)(channeldata - mid) / (float)(max - mid) : (float)(channeldata - mid) / (float)(mid - min);
    }
}

void RemoveControl_ProcessData(uint16_t channeldata[14])
{
    float alpha = 0.4f;
    static ProcessedChannelValue G_Value_Last;
    G_Value.Lx = alpha * 100 * RemoveControl_math(G_ChannelParam.Lx_Mid,G_ChannelParam.Lx_Min,G_ChannelParam.Lx_Max,channeldata[LXChannel - 1]) + (1 - alpha) * G_Value_Last.Lx;
    G_Value.Ly = alpha * 100 * RemoveControl_math(0,G_ChannelParam.Ly_Min,G_ChannelParam.Ly_Max,channeldata[LYChannel - 1]) + (1 - alpha) * G_Value_Last.Ly;
    G_Value.Rx = alpha * 100 * RemoveControl_math(G_ChannelParam.Rx_Mid,G_ChannelParam.Rx_Min,G_ChannelParam.Rx_Max,channeldata[RXChannel - 1]) + (1 - alpha) * G_Value_Last.Rx;
    G_Value.Ry = alpha * 100 * RemoveControl_math(G_ChannelParam.Ry_Mid,G_ChannelParam.Ry_Min,G_ChannelParam.Ry_Max,channeldata[RYChannel - 1]) + (1 - alpha) * G_Value_Last.Ry;
    G_Value.VRA = alpha * 100 * RemoveControl_math(0,G_ChannelParam.VRA_Min,G_ChannelParam.VRA_Max,channeldata[VRAChannel - 1]) + (1 - alpha) * G_Value_Last.VRA;
    G_Value.VRB = alpha * 100 * RemoveControl_math(0,G_ChannelParam.VRB_Min,G_ChannelParam.VRB_Max,channeldata[VRBChannel - 1]) + (1 - alpha) * G_Value_Last.VRB;

    G_Value_Last.Lx = G_Value.Lx;
    G_Value_Last.Ly = G_Value.Ly;
    G_Value_Last.Rx = G_Value.Rx;
    G_Value_Last.Ry = G_Value.Ry;
    G_Value_Last.VRA = G_Value.VRA;
    G_Value_Last.VRB = G_Value.VRB;

    if (channeldata[SWAChannel - 1] == G_ChannelParam.SWA_Close)
        G_Value.SWA = 0;
    else if (channeldata[SWAChannel - 1] == G_ChannelParam.SWA_Open)
        G_Value.SWA = 1;

    if (channeldata[SWBChannel - 1] == G_ChannelParam.SWB_Close)
        G_Value.SWB = 0;
    else if (channeldata[SWBChannel - 1] == G_ChannelParam.SWB_Open)
        G_Value.SWB = 1;

    if (channeldata[SWCChannel - 1] == G_ChannelParam.SWC_Close)
        G_Value.SWC = 0;
    else if (channeldata[SWCChannel - 1] == G_ChannelParam.SWC_First)
        G_Value.SWC = 1;
    else if (channeldata[SWCChannel - 1] == G_ChannelParam.SWC_Second)
        G_Value.SWC = 2;

    if (channeldata[SWDChannel - 1] == G_ChannelParam.SWD_Close)
        G_Value.SWD = 0;
    else if (channeldata[SWDChannel - 1] == G_ChannelParam.SWD_Open)
        G_Value.SWD = 1;
}

