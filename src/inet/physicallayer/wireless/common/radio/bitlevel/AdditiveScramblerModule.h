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

#ifndef __INET_ADDITIVESCRAMBLERMODULE_H
#define __INET_ADDITIVESCRAMBLERMODULE_H

#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambler.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScrambling.h"

namespace inet {
namespace physicallayer {

class INET_API AdditiveScramblerModule : public cSimpleModule, public IScrambler
{
  protected:
    const AdditiveScrambler *scrambler;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("This module doesn't handle self messages"); }

  public:
    virtual ~AdditiveScramblerModule();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual BitVector scramble(const BitVector& bits) const override { return scrambler->scramble(bits); }
    virtual BitVector descramble(const BitVector& bits) const override { return scrambler->descramble(bits); }
    virtual const AdditiveScrambling *getScrambling() const override { return scrambler->getScrambling(); }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

