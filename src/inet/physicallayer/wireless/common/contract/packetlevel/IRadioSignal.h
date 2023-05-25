//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRADIOSIGNAL_H
#define __INET_IRADIOSIGNAL_H

#include "inet/common/IPrintableObject.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/math/Functions.h"

namespace inet {
namespace physicallayer {

class INET_API IRadioSignal: public virtual IPrintableObject
{
  public:
    /**
     * This enumeration specifies a part of a radio signal.
     */
    enum SignalPart {
        SIGNAL_PART_NONE = -1,
        SIGNAL_PART_WHOLE,
        SIGNAL_PART_PREAMBLE,
        SIGNAL_PART_HEADER,
        SIGNAL_PART_DATA
    };

  protected:
    /**
     * The enumeration registered for signal part.
     */
    static cEnum *signalPartEnum;

  public:
    /**
     * Returns the name of the provided signal part.
     */
    static const char *getSignalPartName(SignalPart signalPart);
};

class INET_API INarrowbandSignal : public virtual IRadioSignal
{
  public:
    virtual Hz getCenterFrequency() const = 0;
    virtual Hz getBandwidth() const = 0;

    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const = 0;
};

class INET_API IScalarSignal : public virtual IRadioSignal
{
  public:
    virtual W getPower() const = 0;
};

class INET_API IDimensionalSignal : public virtual IRadioSignal
{
  public:
    virtual const Ptr<const math::IFunction<WpHz, math::Domain<simsec, Hz>>>& getPower() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif

