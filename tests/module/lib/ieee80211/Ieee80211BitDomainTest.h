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

#ifndef __INET_IEEE80211BITDOMAINTEST_H_
#define __INET_IEEE80211BITDOMAINTEST_H_

#include "inet/physicallayer/common/bitlevel/ConvolutionalCoderModule.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OFDMInterleaverModule.h"
#include "inet/physicallayer/common/bitlevel/AdditiveScramblerModule.h"
#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"

using namespace inet::physicallayer;

namespace inet {

class INET_API Ieee80211BitDomainTest : public cSimpleModule
{
    protected:
        AdditiveScramblerModule *scrambler;
        Ieee80211OFDMInterleaverModule *interleaver;
        ConvolutionalCoderModule *convCoder;
        std::ifstream *fileStream;
        const char *testType;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }
        void testConvolutionalCoder(unsigned int numberOfRandomErrors) const;
        void testScrambler() const;
        void testInterleaver() const;
        void testIeee80211BitDomain() const;
};

} /* namespace inet */

#endif /* __INET_IEEE80211BITDOMAINTEST_H_ */
