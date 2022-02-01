//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIGNALSAMPLEMODEL_H
#define __INET_SIGNALSAMPLEMODEL_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSampleModel.h"

namespace inet {
namespace physicallayer {

class INET_API SignalSampleModel : public virtual ISignalSampleModel
{
  protected:
    const int sampleLength;
    const double sampleRate;
    const std::vector<W> *samples;

  public:
    SignalSampleModel(int sampleLength, double sampleRate, const std::vector<W> *samples);
    virtual ~SignalSampleModel();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual int getSampleLength() const override { return sampleLength; }
    virtual double getSampleRate() const override { return sampleRate; }
    virtual const std::vector<W> *getSamples() const override { return samples; }
};

class INET_API TransmissionSampleModel : public SignalSampleModel, public virtual ITransmissionSampleModel
{
  public:
    TransmissionSampleModel(int sampleLength, double sampleRate, const std::vector<W> *samples);
};

class INET_API ReceptionSampleModel : public SignalSampleModel, public virtual IReceptionSampleModel
{
  protected:
    const W rssi;

  public:
    ReceptionSampleModel(int sampleLength, double sampleRate, const std::vector<W> *samples, W rssi);

    virtual const W getRSSI() const override { return rssi; }
};

} // namespace physicallayer
} // namespace inet

#endif

