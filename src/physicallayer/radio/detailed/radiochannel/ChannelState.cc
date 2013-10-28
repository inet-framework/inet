#include "ChannelState.h"

bool ChannelState::isIdle() const
{
    return idle;
}

double ChannelState::getRSSI() const
{
    return rssi;
}

