//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

cplusplus{{
#include "inet/physicallayer/base/packetlevel/PhysicalLayerDefs.h"
}}

namespace inet;

//
// This is an abstract base class that should not be directly added as a tag.
//
class SignalTagBase extends TagBase
{
}

//
// This is an abstract base class that should not be directly added as a tag.
//
class SignalPowerTagBase extends SignalTagBase
{
    W power = W(NaN); // specifies signal power in Watts
}

//
// This request determines the average analog signal power that should be used to transmit the packet.
// It may be present on a packet from the application to the phyisical layer.
//
class SignalPowerReq extends SignalPowerTagBase
{
}

//
// This indication specifies the average analog signal power that was detected during receiving the packet.
// It may be present on a packet from the phyiscal layer to the application.
//
class SignalPowerInd extends SignalPowerTagBase
{
}

//
// This is an abstract base class that should not be directly added as a tag.
//
class SignalBandTagBase extends SignalTagBase
{
    Hz centerFrequency = Hz(NaN); // carrier frequency in the range (0, +infinity) or NaN if not set.
    Hz bandwidth = Hz(NaN);        // bandwidth in the rage (0, +infinity) or NaN if not set.
}

//
// This request determines the signal band that should be used to transmit the packet.
// It may be present on a packet from the application to the phyisical layer.
//
class SignalBandReq extends SignalBandTagBase
{
}

//
// This indication specifies the signal band that was used to receive the packet.
// It may be present on a packet from the phyiscal layer to the application.
//
class SignalBandInd extends SignalBandTagBase
{
}

//
// This is an abstract base class that should not be directly added as a tag.
//
class SignalBitrateTagBase extends SignalTagBase
{
    bps preambleBitrate = bps(NaN); // preamble bitrate in the range (0, +infinity) or NaN if not set.
    bps headerBitrate = bps(NaN);   // header bitrate in the range (0, +infinity) or NaN if not set.
    bps dataBitrate = bps(NaN);     // data bitrate in the range (0, +infinity) or NaN if not set.
}

//
// This request determines the bitrates for various parts of the signal that should be used to transmit the packet.
// It may be present on a packet from the application to the phyisical layer.
//
class SignalBitrateReq extends SignalBitrateTagBase
{
}

//
// This indication specifies the bitrates for various parts of the signal that was used to receive the packet.
// It may be present on a packet from the phyiscal layer to the application.
//
class SignalBitrateInd extends SignalBitrateTagBase
{
}

//
// This indication specifies the signal to noise ratio that was detected during receiving the packet.
// It may be present on a packet from the phyiscal layer to the application.
//
class SnirInd extends SignalTagBase
{
    double minimumSnir = NaN; // minimum signal to noise plus interference ratio in the range (0, +infinity) or NaN if unknown. 
    double maximumSnir = NaN; // maximum signal to noise plus interference ratio in the range (0, +infinity) or NaN if unknown.
    double averageSnir = NaN; // average signal to noise plus interference ratio in the range (0, +infinity) or NaN if unknown.
}

//
// This indication specifies various error rates that was computed during receiving the packet.
// It may be present on a packet from the phyiscal layer to the application.
//
class ErrorRateInd extends SignalTagBase
{
    double packetErrorRate = NaN; // packet error rate (probability) in the range [0, 1] or NaN if unknown.
    double bitErrorRate = NaN;    // bit error rate (probability) in the range [0, 1] or NaN if unknown.
    double symbolErrorRate = NaN; // symbol error rate (probability) in the range [0, 1] or NaN if unknown.
}

//
// This indication specifies the timing of the received signal.
// It may be present on a packet from the phyiscal layer to the application.
//
class SignalTimeInd extends SignalTagBase
{
    simtime_t startTime;
    simtime_t endTime;
}
