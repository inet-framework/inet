//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_APSKENCODER_H
#define __INET_APSKENCODER_H

#include "inet/physicallayer/apskradio/bitlevel/ApskCode.h"
#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/contract/bitlevel/IEncoder.h"
#include "inet/physicallayer/contract/bitlevel/IFecCoder.h"
#include "inet/physicallayer/contract/bitlevel/IInterleaver.h"
#include "inet/physicallayer/contract/bitlevel/IScrambler.h"

namespace inet {

namespace physicallayer {

class INET_API ApskEncoder : public IEncoder, public cSimpleModule
{
  protected:
    const ApskCode *code;
    const IScrambler *scrambler;
    const IFecCoder *fecEncoder;
    const IInterleaver *interleaver;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    ApskEncoder();
    virtual ~ApskEncoder();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual const ApskCode *getCode() const override { return code; }
    virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_APSKENCODER_H

