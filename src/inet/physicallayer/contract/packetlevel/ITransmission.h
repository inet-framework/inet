//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_ITRANSMISSION_H
#define __INET_ITRANSMISSION_H

#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/EulerAngles.h"
#include "inet/common/Units.h"
#include "inet/physicallayer/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"
#include "inet/physicallayer/contract/packetlevel/IRadioSignal.h"

namespace inet {

namespace physicallayer {

class IRadio;

/**
 * This interface represents the transmission of a radio signal. There's one
 * instance per transmission of this interface that is sent to all potential
 * receiver radios in a separate radio frame instance.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API ITransmission : public IPrintableObject
{
  protected:
    static int nextId;

  public:
    /**
     * Returns an identifier for this transmission which is globally unique
     * for the whole lifetime of the simulation among all transmissions.
     */
    virtual int getId() const = 0;

    /**
     * Returns the transmitter that transmitted this radio signal on the radio
     * channel. This function never returns nullptr.
     */
    virtual const IRadio *getTransmitter() const = 0;

    /**
     * Returns the PHY frame corresponding to this transmission. This function
     * may return nullptr.
     */
    virtual const cPacket *getPhyFrame() const = 0;

    /**
     * Returns the MAC frame corresponding to this transmission. This function
     * never returns nullptr.
     */
    virtual const cPacket *getMacFrame() const = 0;

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
    virtual const Coord getStartPosition() const = 0;

    /**
     * Returns the antenna's position when the transmitter ended this transmission.
     */
    virtual const Coord getEndPosition() const = 0;

    /**
     * Returns the antenna's orientation when the transmitter started this transmission.
     */
    virtual const EulerAngles getStartOrientation() const = 0;

    /**
     * Returns the antenna's orientation when the transmitter ended this transmission.
     */
    virtual const EulerAngles getEndOrientation() const = 0;

    /**
     * Returns the analog model of the transmitted signal.
     */
    virtual const ITransmissionAnalogModel *getAnalogModel() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ITRANSMISSION_H

