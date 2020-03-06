//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ISIGNALPACKETMODEL_H
#define __INET_ISIGNALPACKETMODEL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IFecCoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IInterleaver.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IScrambler.h"

namespace inet {

namespace physicallayer {

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the packet domain.
 */
class INET_API ISignalPacketModel : public IPrintableObject
{
  public:
    /**
     * Returns the packet that includes the PHY frame header and PHY frame data parts.
     */
    virtual const Packet *getPacket() const = 0;

    /**
     * Returns the net bitrate (datarate) of the PHY frame header part.
     */
    virtual bps getHeaderNetBitrate() const = 0;

    /**
    * Returns the net bitrate (datarate) of the PHY frame data part.
    */
    virtual bps getDataNetBitrate() const = 0;
};

class INET_API ITransmissionPacketModel : public virtual ISignalPacketModel
{
};

class INET_API IReceptionPacketModel : public virtual ISignalPacketModel
{
  public:
    virtual double getPacketErrorRate() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

