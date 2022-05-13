//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_INEW_H
#define __INET_INEW_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/Units.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/common/packet/Packet.h"

namespace inet {
namespace newphysicallayer {

using namespace inet::units::values;

class INET_API ITransmissionMedium
{
};

class INET_API ISignalSource
{
};

class INET_API ISignalSink
{
};

class INET_API INoiseSource : public ISignalSource
{
};

class INET_API ISignalProbe : public ISignalSink
{
};

class INET_API ITransmitter
{
};

class INET_API IReceiver
{
};

class INET_API ITransceiver : public ISignalSource, public ISignalSink
{
    virtual ITransmitter *getTransmitter() = 0;
    virtual IReceiver *getReceiver() = 0;
};

class INET_API IPowerSpectralDensity
{
    virtual simtime_t getStartTime() = 0;
    virtual simtime_t getEndTime() = 0;

    virtual Hz getStartFrequency() = 0;
    virtual Hz getEndFrequency() = 0;

    virtual W getPower(simtime_t time, Hz frequency) = 0;
};

class INET_API IPowerDirectionalSelectivity
{
    virtual double getGain(Quaternion direction) = 0;
};

class INET_API ISignalDeparture
{
    virtual ISignalSource *getSource() = 0;

    virtual IPowerSpectralDensity *getPowerSpectralDensity() = 0;
    virtual IPowerDirectionalSelectivity *getPowerDirectionalSelectivity() = 0;

    virtual Coord getDeparturePosition() = 0;
    virtual Quaternion getDepartureOrientation() = 0;
};

class INET_API ISignalArrival
{
    virtual ISignalDeparture *getDeparture() = 0;
    virtual ISignalSink *getSink() = 0;

    virtual IPowerSpectralDensity *getPowerSpectralDensity() = 0;
    virtual IPowerDirectionalSelectivity *getPowerDirectionalSelectivity() = 0;

    virtual Coord getArrivalPosition() = 0;
    virtual Quaternion getArrivalOrientation() = 0;
};

class INET_API ISignalTransmission : public ISignalDeparture
{
    virtual Packet *getPacket() = 0;
    virtual ITransceiver *getTransmitter() = 0;
};

class INET_API ISignalReception : public ISignalArrival
{
    virtual ISignalTransmission *getTransmission() = 0;
    virtual ITransceiver *getReceiver() = 0;
};

} // namespace newphysicallayer
} // namespace inet

#endif

