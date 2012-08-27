#ifndef __IEEE80211_DATA_RATES_H__
#define __IEEE80211_DATA_RATES_H__

#include "INETDefs.h"

#include "ModulationType.h"


struct Ieee80211Descriptor
{
    char mode;
    double bitrate;
    ModulationType modulationType;
};

extern const Ieee80211Descriptor ieee80211Descriptor[];
int getIeee80211DescriptorIdx(char mode, double bitrate);

#endif
