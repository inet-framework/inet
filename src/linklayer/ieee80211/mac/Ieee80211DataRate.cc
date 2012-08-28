// Copyright (C) 2012 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// @author: Zoltan Bojthe
//

#include <algorithm>

#include "Ieee80211DataRate.h"

#include "WifiMode.h"


/* Bit rates for 802.11b/g/a/p.
 * Must be ordered by mode, bitrate.
 */
const Ieee80211DescriptorData Ieee80211Descriptor::data[] =
{
    {'a',  6000000, WifiModulationType::GetOfdmRate6Mbps()},
    {'a',  9000000, WifiModulationType::GetOfdmRate9Mbps()},
    {'a', 12000000, WifiModulationType::GetOfdmRate12Mbps()},
    {'a', 18000000, WifiModulationType::GetOfdmRate18Mbps()},
    {'a', 24000000, WifiModulationType::GetOfdmRate24Mbps()},
    {'a', 36000000, WifiModulationType::GetOfdmRate36Mbps()},
    {'a', 48000000, WifiModulationType::GetOfdmRate48Mbps()},
    {'a', 54000000, WifiModulationType::GetOfdmRate54Mbps()},

    {'b',  1000000, WifiModulationType::GetDsssRate1Mbps()},
    {'b',  2000000, WifiModulationType::GetDsssRate2Mbps()},
    {'b',  5500000, WifiModulationType::GetDsssRate5_5Mbps()},
    {'b', 11000000, WifiModulationType::GetDsssRate11Mbps()},

    {'g',  1000000, WifiModulationType::GetDsssRate1Mbps()},
    {'g',  2000000, WifiModulationType::GetDsssRate2Mbps()},
    {'g',  5500000, WifiModulationType::GetDsssRate5_5Mbps()},
    {'g',  6000000, WifiModulationType::GetErpOfdmRate6Mbps()},
    {'g',  9000000, WifiModulationType::GetErpOfdmRate9Mbps()},
    {'g', 11000000, WifiModulationType::GetDsssRate11Mbps()},
    {'g', 12000000, WifiModulationType::GetErpOfdmRate12Mbps()},
    {'g', 18000000, WifiModulationType::GetErpOfdmRate18Mbps()},
    {'g', 24000000, WifiModulationType::GetErpOfdmRate24Mbps()},
    {'g', 36000000, WifiModulationType::GetErpOfdmRate36Mbps()},
    {'g', 48000000, WifiModulationType::GetErpOfdmRate48Mbps()},
    {'g', 54000000, WifiModulationType::GetErpOfdmRate54Mbps()},

    {'p',  3000000, WifiModulationType::GetOfdmRate3MbpsBW10MHz()},
    {'p',  4500000, WifiModulationType::GetOfdmRate4_5MbpsBW10MHz()},
    {'p',  6000000, WifiModulationType::GetOfdmRate6MbpsBW10MHz()},
    {'p',  9000000, WifiModulationType::GetOfdmRate9MbpsBW10MHz()},
    {'p', 12000000, WifiModulationType::GetOfdmRate12MbpsBW10MHz()},
    {'p', 18000000, WifiModulationType::GetOfdmRate18MbpsBW10MHz()},
    {'p', 24000000, WifiModulationType::GetOfdmRate24MbpsBW10MHz()},
    {'p', 27000000, WifiModulationType::GetOfdmRate27MbpsBW10MHz()},
};

const int Ieee80211Descriptor::descriptorSize = sizeof(Ieee80211Descriptor::data)/sizeof(Ieee80211Descriptor::data[0]);

#if 0
// linear search

int Ieee80211Descriptor::findIdx(char mode, double bitrate)
{
    for (int i=0; i<descriptorSize; i++)
    {
        if (data[i].mode == mode)
        {
            if (data[i].bitrate == bitrate)
                return i;
            if (data[i].bitrate > bitrate)
                break;
        }
        if (data[i].mode > mode)
            break;
    }
    return -1;
}

int Ieee80211Descriptor::getMinIdx(char mode)
{
    for (int i=0; i<descriptorSize; i++)
    {
        if (data[i].mode == mode)
            return i;
        if (data[i].mode > mode)
            break;
    }
    throw cRuntimeError("mode '%c' not valid", mode);
}

int Ieee80211Descriptor::getMaxIdx(char mode)
{
    int idx = -1;
    for (int i=0; i<descriptorSize; i++)
    {
        if (data[i].mode == mode)
            idx = i;
        if (data[i].mode > mode)
            break;
    }
    if (idx == -1)
        throw cRuntimeError("mode '%c' not valid", mode);
    return idx;
}

#else

namespace
{
    bool ieee80211DescriptorCompareModeBitrate(const Ieee80211DescriptorData& a, const Ieee80211DescriptorData& b)
    {
        // return a < b;
        return (a.mode < b.mode) || ((a.mode == b.mode) && (a.bitrate < b.bitrate));
    }

    bool ieee80211DescriptorCompareMode(const Ieee80211DescriptorData& a, const Ieee80211DescriptorData& b)
    {
        // return a < b;
        return (a.mode < b.mode);
    }
}

int Ieee80211Descriptor::findIdx(char mode, double bitrate)
{
    Ieee80211DescriptorData d;
    d.mode = mode;
    d.bitrate = bitrate;

    const Ieee80211DescriptorData *found = std::lower_bound(data, &data[descriptorSize], d, ieee80211DescriptorCompareModeBitrate);
    if (found->mode == mode && found->bitrate == bitrate)
        return (int)(found - data);
    return -1;
}

int Ieee80211Descriptor::getMinIdx(char mode)
{
    Ieee80211DescriptorData d;
    d.mode = mode;

    const Ieee80211DescriptorData *found = std::lower_bound(data, &data[descriptorSize], d, ieee80211DescriptorCompareMode);
    if (found->mode == mode)
        return (int)(found - data);
    throw cRuntimeError("mode '%c' not valid", mode);
}

int Ieee80211Descriptor::getMaxIdx(char mode)
{
    Ieee80211DescriptorData d;
    d.mode = mode;

    const Ieee80211DescriptorData *found = std::upper_bound(data, &data[descriptorSize], d, ieee80211DescriptorCompareMode);
    if (found > data)
        --found;
    if (found->mode == mode)
        return (int)(found - data);
    throw cRuntimeError("mode '%c' not valid", mode);
}

#endif


int Ieee80211Descriptor::getIdx(char mode, double bitrate)
{
    int idx = findIdx(mode, bitrate);
    if (idx == -1)
        throw cRuntimeError("mode '%c':%g bps not valid", mode, bitrate);
    return idx;
}

bool Ieee80211Descriptor::incIdx(int& idx)
{
    ASSERT(idx >= 0 && idx < descriptorSize);

    if (data[idx].mode != data[idx+1].mode)
        return false;
    ++idx;
    return true;
}

bool Ieee80211Descriptor::decIdx(int& idx)
{
    ASSERT(idx >= 0 && idx < descriptorSize);

    if (idx==0 || data[idx].mode != data[idx-1].mode)
        return false;
    --idx;
    return true;
}

const Ieee80211DescriptorData& Ieee80211Descriptor::getDescriptor(int idx)
{
    ASSERT(idx >= 0 && idx < descriptorSize);

    return data[idx];
}


