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

#ifndef __INET_ENCODER_H
#define __INET_ENCODER_H

#include "IEncoder.h"
#include "ISerializer.h"
#include "IForwardErrorCorrection.h"
#include "IScrambler.h"
#include "IInterleaver.h"
#include "SignalPacketModel.h"
#include "SignalBitModel.h"

namespace inet {

namespace physicallayer {

class INET_API Encoder : public IEncoder, public cSimpleModule
{
  protected:
    double bitRate;
    int headerBitLength;
    const ISerializer *serializer;
    const IScrambler *scrambler;
    const IForwardErrorCorrection *forwardErrorCorrection;
    const IInterleaver *interleaver;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg) { cRuntimeError("This module doesn't handle self messages."); }

  public:
    virtual const ITransmissionBitModel *encode(const ITransmissionPacketModel *packetModel) const;
    void printToStream(std::ostream& stream) const { stream << "Layered Encoder"; } // TODO
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ENCODER_H
