//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_NOISESCALARTRANSMITTER_H
#define __INET_NOISESCALARTRANSMITTER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmitter.h"

namespace inet {

namespace physicallayer {

class INET_API NoiseScalarTransmitter : public TransmitterBase
{
  protected:
    cPar *durationParameter = nullptr;
    cPar *centerFrequencyParameter = nullptr;
    cPar *bandwidthParameter = nullptr;
    cPar *powerParameter = nullptr;

  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

