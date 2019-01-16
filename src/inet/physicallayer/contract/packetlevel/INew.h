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

#ifndef __INET_NEW_H
#define __INET_NEW_H

#include "inet/common/Units.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"

namespace inet {
namespace newphysicallayer {

using namespace inet::units::values;

class ITransmissionMedium
{
};

class ISignalSource
{
};

class ISignalSink
{
};

class INoiseSource : public ISignalSource
{
};

class ISignalProbe : public ISignalSink
{
};

class ITransmitter
{
};

class IReceiver
{
};

class ITransceiver : public ISignalSource, public ISignalSink
{
    virtual ITransmitter *getTransmitter() = 0;
    virtual IReceiver *getReceiver() = 0;
};

class IPowerSpectralDensity
{
    virtual simtime_t getStartTime() = 0;
    virtual simtime_t getEndTime() = 0;

    virtual Hz getStartFrequency() = 0;
    virtual Hz getEndFrequency() = 0;

    virtual W getPower(simtime_t time, Hz frequency) = 0;
};

class IPowerDirectionalSelectivity
{
    virtual double getGain(Quaternion direction) = 0;
};

class ISignalDeparture
{
    virtual ISignalSource *getSource() = 0;

    virtual IPowerSpectralDensity *getPowerSpectralDensity() = 0;
    virtual IPowerDirectionalSelectivity *getPowerDirectionalSelectivity() = 0;

    virtual Coord getDeparturePosition() = 0;
    virtual Quaternion getDepartureOrientation() = 0;
};

class ISignalArrival
{
    virtual ISignalDeparture *getDeparture() = 0;
    virtual ISignalSink *getSink() = 0;

    virtual IPowerSpectralDensity *getPowerSpectralDensity() = 0;
    virtual IPowerDirectionalSelectivity *getPowerDirectionalSelectivity() = 0;

    virtual Coord getArrivalPosition() = 0;
    virtual Quaternion getArrivalOrientation() = 0;
};

class ISignalTransmission : public ISignalDeparture
{
    virtual Packet *getPacket() = 0;
    virtual ITransceiver *getTransmitter() = 0;
};

class ISignalReception : public ISignalArrival
{
    virtual ISignalTransmission *getTransmission() = 0;
    virtual ITransceiver *getReceiver() = 0;
};

} // namespace newphysicallayer
} // namespace inet

#endif // ifndef __INET_INEW_H

