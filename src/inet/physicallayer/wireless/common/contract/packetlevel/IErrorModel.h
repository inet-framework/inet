//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IERRORMODEL_H
#define __INET_IERRORMODEL_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/ISnir.h"

namespace inet {

namespace physicallayer {

/**
 * The error model describes how the signal to noise ratio affects the amount of
 * errors at the receiver. The main purpose of this model is to determine whether
 * if the received packet has errors or not. It also computes various physical
 * layer indications for higher layers such as packet error rate, bit error rate,
 * and symbol error rate.
 */
class INET_API IErrorModel : public virtual IPrintableObject
{
  public:
    virtual Packet *computeCorruptedPacket(const ISnir *snir) const = 0;

    /**
     * Returns the packet error rate based on SNIR, modulation, FEC encoding
     * and any other physical layer characteristics.
     */
    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const = 0;

    /**
     * Returns the probability that a region of the provided signal part has at
     * least one error. The region has the provided number of content bits, and
     * the transmitter sends it from startTime to endTime. The rate uses the SNIR
     * of that interval. The regions of a part share the bits that the rate of
     * the part counts. Thus, at a constant SNIR, the rates of the regions of the
     * header and the data give the rate of the whole signal. Only the header
     * part and the data part have regions.
     */
    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part, b length, simtime_t startTime, simtime_t endTime) const = 0;

    /**
     * Returns the bit error rate based on SNIR, modulation, FEC encoding
     * and any other physical layer characteristics.
     */
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const = 0;

    /**
     * Returns the error rate of one content bit of a region of the provided
     * signal part; see the region rate of computePacketErrorRate(). The region
     * rate is equal to 1 - (1 - bit error rate)^length.
     */
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part, b length, simtime_t startTime, simtime_t endTime) const = 0;

    /**
     * Returns the symbol error rate based on SNIR, modulation, and any other
     * physical layer characteristics.
     */
    virtual double computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

