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

#ifndef __INET_CONVOLUTIONALCODERMODULE_H_
#define __INET_CONVOLUTIONALCODERMODULE_H_

#include "ConvolutionalCoder.h"

namespace inet {
namespace physicallayer {

class ConvolutionalCoderModule : public cSimpleModule, public IFECEncoder
{
    protected:
        ConvolutionalCoder *convolutionalCoder;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }

    public:
        virtual BitVector encode(const BitVector& informationBits) const { return convolutionalCoder->encode(informationBits); }
        virtual BitVector decode(const BitVector& encodedBits) const { return convolutionalCoder->decode(encodedBits); }
        virtual const IForwardErrorCorrection *getConvolutionalCode() const {  return convolutionalCoder->getConvolutionalCode(); }
        ~ConvolutionalCoderModule();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_CONVOLUTIONALCODERMODULE_H_ */
