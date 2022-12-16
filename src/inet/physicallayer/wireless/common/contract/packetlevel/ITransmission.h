//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITRANSMISSION_H
#define __INET_ITRANSMISSION_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/Units.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IAntennaGain.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioSignal.h"

namespace inet {
namespace physicallayer {

class IRadio;
class IRadioMedium;

/**
 * This interface represents the transmission of a radio signal. There's one
 * instance per transmission of this interface that is sent to all potential
 * receiver radios in a separate signal instance.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API ITransmission : public IPrintableObject
{
  protected:
    uint64_t& nextId = SIMULATION_SHARED_COUNTER(nextId);

  public:
    /**
     * Returns an identifier for this transmission which is globally unique
     * for the whole lifetime of the simulation among all transmissions.
     */
    virtual int getId() const = 0;

    /**
     * Returns the transmitter that transmitted this radio signal on the radio
     * channel. This function may return nullptr.
     */
    virtual const IRadio *getTransmitter() const = 0;

    /**
     * Returns the transmitter's id that transmitted this radio signal on the
     * radio channel.
     */
    virtual int getTransmitterId() const = 0;

    /**
     * Returns the gain of the transmitting antenna.
     */
    virtual const IAntennaGain *getTransmitterAntennaGain() const = 0;

    /**
     * Returns the radio medium used for this transmission. This function
     * never return nullptr.
     */
    virtual const IRadioMedium *getMedium() const = 0;

    /**
     * Returns the packet corresponding to this transmission. This function
     * never returns nullptr.
     */
    virtual const Packet *getPacket() const = 0;

    /**
     * Returns the time when the transmitter started this transmission. It is
     * the start of the first bit's transmission.
     */
    virtual const simtime_t getStartTime() const = 0;

    /**
     * Returns the time when the transmitter ended this transmission. It is
     * the end of the last bit's transmission.
     */
    virtual const simtime_t getEndTime() const = 0;

    virtual const simtime_t getStartTime(IRadioSignal::SignalPart part) const = 0;
    virtual const simtime_t getEndTime(IRadioSignal::SignalPart part) const = 0;

    virtual const simtime_t getPreambleStartTime() const = 0;
    virtual const simtime_t getPreambleEndTime() const = 0;
    virtual const simtime_t getHeaderStartTime() const = 0;
    virtual const simtime_t getHeaderEndTime() const = 0;
    virtual const simtime_t getDataStartTime() const = 0;
    virtual const simtime_t getDataEndTime() const = 0;

    /**
     * Returns the total length of this transmission.
     */
    virtual const simtime_t getDuration() const = 0;

    /**
     * Returns the length of the provided part of this transmission.
     */
    virtual const simtime_t getDuration(IRadioSignal::SignalPart part) const = 0;

    /**
     * Returns the length of the preamble part of this transmission.
     */
    virtual const simtime_t getPreambleDuration() const = 0;

    /**
     * Returns the length of the header part of this transmission.
     */
    virtual const simtime_t getHeaderDuration() const = 0;

    /**
     * Returns the length of the data part of this transmission.
     */
    virtual const simtime_t getDataDuration() const = 0;

    /**
     * Returns the antenna's position when the transmitter started this transmission.
     */
    virtual const Coord& getStartPosition() const = 0;

    /**
     * Returns the antenna's position when the transmitter ended this transmission.
     */
    virtual const Coord& getEndPosition() const = 0;

    /**
     * Returns the antenna's orientation when the transmitter started this transmission.
     */
    virtual const Quaternion& getStartOrientation() const = 0;

    /**
     * Returns the antenna's orientation when the transmitter ended this transmission.
     */
    virtual const Quaternion& getEndOrientation() const = 0;

    /**
     * Returns the analog model of the transmitted signal.
     */
    virtual const ITransmissionAnalogModel *getAnalogModel() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

