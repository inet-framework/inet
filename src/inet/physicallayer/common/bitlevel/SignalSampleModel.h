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

#ifndef __INET_SIGNALSAMPLEMODEL_H
#define __INET_SIGNALSAMPLEMODEL_H

#include "inet/physicallayer/contract/bitlevel/ISignalSampleModel.h"

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

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

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

#endif // ifndef __INET_SIGNALSAMPLEMODEL_H

