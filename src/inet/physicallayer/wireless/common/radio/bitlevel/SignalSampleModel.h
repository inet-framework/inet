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
    const int headerSampleLength;
    const double headerSampleRate;
    const int dataSampleLength;
    const double dataSampleRate;
    const std::vector<W> *samples;

  public:
    SignalSampleModel(int headerSampleLength, double headerSampleRate, int dataSampleLength, double dataSampleRate, const std::vector<W> *samples);
    virtual ~SignalSampleModel();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual int getHeaderSampleLength() const override { return headerSampleLength; }
    virtual double getHeaderSampleRate() const override { return headerSampleRate; }
    virtual int getDataSampleLength() const override { return dataSampleLength; }
    virtual double getDataSampleRate() const override { return dataSampleRate; }
    virtual const std::vector<W> *getSamples() const override { return samples; }
};

class INET_API TransmissionSampleModel : public SignalSampleModel, public virtual ITransmissionSampleModel
{
  public:
    TransmissionSampleModel(int headerSampleLength, double headerSampleRate, int dataSampleLength, double dataSampleRate, const std::vector<W> *samples);
};

class INET_API ReceptionSampleModel : public SignalSampleModel, public virtual IReceptionSampleModel
{
  public:
    ReceptionSampleModel(int headerSampleLength, double headerSampleRate, int dataSampleLength, double dataSampleRate, const std::vector<W> *samples);
};

} // namespace physicallayer
} // namespace inet

#endif

