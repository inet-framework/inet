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

#ifndef __INET_LAYEREDDECODER_H_
#define __INET_LAYEREDDECODER_H_

#include "IDecoder.h"

namespace inet {
namespace physicallayer {

class INET_API LayeredDecoder : public cSimpleModule, public IDecoder
{
    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { cRuntimeError("This module doesn't handle self messages."); }

    public:
        virtual const IReceptionPacketModel *decode(const IReceptionBitModel *bitModel) const;
        virtual void printToStream(std::ostream& stream) const { stream << "Layered Decoder"; } // TODO
        virtual ~LayeredDecoder();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_LAYEREDDECODER_H_ */
