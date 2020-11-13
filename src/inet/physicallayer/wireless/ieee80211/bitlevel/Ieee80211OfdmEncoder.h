//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE80211OFDMENCODER_H
#define __INET_IEEE80211OFDMENCODER_H

#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IEncoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IFecCoder.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IInterleaver.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IScrambler.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalBitModel.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/SignalPacketModel.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmCode.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211OfdmEncoder : public IEncoder
{
  protected:
    const IFecCoder *convolutionalCoder = nullptr;
    const IInterleaver *interleaver = nullptr;
    const IScrambler *scrambler = nullptr;
    const Ieee80211OfdmCode *code = nullptr;

  public:
    Ieee80211OfdmEncoder(const Ieee80211OfdmCode *code);
    ~Ieee80211OfdmEncoder();

    virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const override;
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    const Ieee80211OfdmCode *getCode() const override { return code; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

