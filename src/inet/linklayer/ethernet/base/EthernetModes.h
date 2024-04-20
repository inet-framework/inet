//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETMODES_H
#define __INET_ETHERNETMODES_H

#include "inet/common/INETDefs.h"
#include "inet/common/Units.h"

namespace inet {

using namespace inet::units::values;

/**
 * Base class for Ethernet MAC implementations.
 */
class INET_API EthernetModes
{
  public:
    struct EthernetMode {
        double bitrate;
        // for half-duplex operation:
        short int maxFrameCountInBurst;
        B maxBytesInBurst; // including IFG and preamble, etc.
        B halfDuplexFrameMinBytes; // minimal frame length in half-duplex mode; -1 means half duplex is not supported
        B frameInBurstMinBytes; // minimal frame length in burst mode, after first frame
        uint16_t slotBitLength; // slot time specified in bitcount
        uint16_t csmaMaxPropagationDelayInBits; // used for detecting longer cables than allowed, maxPropagationDelay [s] = csmaMaxPropagationDelayInBits [b] / bitrate [bps]
    };

  protected:
    enum {
        NUM_OF_ETHERNETMODES = 11
    };

    // MAC constants for bitrates and modes
    static const EthernetMode ethernetModes[NUM_OF_ETHERNETMODES];

  public:
    static const EthernetMode nullEthernetMode;

  public:
    static EthernetModes::EthernetMode getEthernetMode(double txRate, bool allowNonstandardBitrate = false);
};

} // namespace inet

#endif
