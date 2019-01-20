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

#ifndef __INET_APSKCODE_H
#define __INET_APSKCODE_H

#include "inet/physicallayer/common/bitlevel/ConvolutionalCode.h"
#include "inet/physicallayer/contract/bitlevel/ICode.h"
#include "inet/physicallayer/contract/bitlevel/IInterleaver.h"
#include "inet/physicallayer/contract/bitlevel/IScrambler.h"

namespace inet {

namespace physicallayer {

class INET_API ApskCode : public ICode
{
  protected:
    const ConvolutionalCode *convolutionalCode;
    const IInterleaving *interleaving;
    const IScrambling *scrambling;

  public:
    ApskCode(const ConvolutionalCode *convCode, const IInterleaving *interleaving, const IScrambling *scrambling);
    ~ApskCode();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    const ConvolutionalCode *getConvolutionalCode() const { return convolutionalCode; }
    const IInterleaving *getInterleaving() const { return interleaving; }
    const IScrambling *getScrambling() const { return scrambling; }
};
} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_APSKCODE_H */

