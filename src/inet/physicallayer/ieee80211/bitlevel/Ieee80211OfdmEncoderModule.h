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

#ifndef __INET_IEEE80211OFDMENCODERMODULE_H
#define __INET_IEEE80211OFDMENCODERMODULE_H

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmEncoder.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211OfdmEncoderModule : public IEncoder, public cSimpleModule
{
  protected:
    const Ieee80211OfdmEncoder *encoder = nullptr;
    const IScrambler *scrambler = nullptr;
    const IFecCoder *convolutionalCoder = nullptr;
    const IInterleaver *interleaver = nullptr;
    const Ieee80211OfdmCode *code = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle self messages"); }

  public:
    virtual ~Ieee80211OfdmEncoderModule();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override { return encoder->printToStream(stream, level); }
    const Ieee80211OfdmCode *getCode() const override { return code; }
    virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const override;
};
} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211OFDMENCODERMODULE_H

