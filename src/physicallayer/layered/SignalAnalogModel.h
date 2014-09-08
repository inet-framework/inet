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

#ifndef __INET_SIGNALANALOGMODEL_H
#define __INET_SIGNALANALOGMODEL_H

#include "ISignalAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API SignalAnalogModel : public virtual ISignalAnalogModel
{
  protected:
    const simtime_t duration;

  public:
    SignalAnalogModel(const simtime_t duration) :
        duration(duration)
    {}

    virtual void printToStream(std::ostream &stream) const;

    virtual const simtime_t getDuration() const { return duration; }
};

class INET_API ScalarAnalogModel : public SignalAnalogModel
{
  protected:
    const W power;
    const Hz carrierFrequency;
    const Hz bandwidth;

  public:
    ScalarAnalogModel(const simtime_t duration, W power, Hz carrierFrequency, Hz bandwidth) :
        SignalAnalogModel(duration),
        power(power),
        carrierFrequency(carrierFrequency),
        bandwidth(bandwidth)
    {}

    virtual void printToStream(std::ostream &stream) const;

    virtual W getPower() const { return power; }
    virtual Hz getCarrierFrequency() const { return carrierFrequency; }
    virtual Hz getBandwidth() const { return bandwidth; }
};

class INET_API ScalarTransmissionAnalogModel : public ScalarAnalogModel, public virtual ITransmissionAnalogModel
{
  public:
    ScalarTransmissionAnalogModel(const simtime_t duration, W power, Hz carrierFrequency, Hz bandwidth) :
        ScalarAnalogModel(duration, power, carrierFrequency, bandwidth)
    {}
};

class INET_API ScalarReceptionAnalogModel : public ScalarAnalogModel, public virtual IReceptionAnalogModel
{
  protected:
    const double snir;

  public:
    ScalarReceptionAnalogModel(const simtime_t duration, W power, Hz carrierFrequency, Hz bandwidth, double snir) :
        ScalarAnalogModel(duration, power, carrierFrequency, bandwidth),
        snir(snir)
    {}

    /**
     * Returns the signal to noise plus interference ratio.
     */
    virtual double getSNIR() const { return snir; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SIGNALANALOGMODEL_H
