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

#ifndef __INET_IEEE80211OFDMENCODER_H
#define __INET_IEEE80211OFDMENCODER_H

#include "inet/physicallayer/contract/bitlevel/IEncoder.h"
#include "inet/physicallayer/contract/bitlevel/IFECCoder.h"
#include "inet/physicallayer/contract/bitlevel/IScrambler.h"
#include "inet/physicallayer/contract/bitlevel/IInterleaver.h"
#include "inet/physicallayer/common/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/common/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/base/packetlevel/APSKModulationBase.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMCode.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OFDMEncoder : public IEncoder
{
  protected:
    const IFECCoder *convolutionalCoder = nullptr;
    const IInterleaver *interleaver = nullptr;
    const IScrambler *scrambler = nullptr;
    const Ieee80211OFDMCode *code = nullptr;

  public:
    Ieee80211OFDMEncoder(const Ieee80211OFDMCode *code);
    ~Ieee80211OFDMEncoder();

    virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const override;
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    const Ieee80211OFDMCode *getCode() const override { return code; }
};
} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211OFDMENCODER_H

