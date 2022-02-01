//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211BITDOMAINTEST_H_
#define __INET_IEEE80211BITDOMAINTEST_H_

#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCoderModule.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaverModule.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/AdditiveScramblerModule.h"
#include "inet/common/ModuleAccess.h"

using namespace inet::physicallayer;

namespace inet {

class INET_API Ieee80211BitDomainTest : public cSimpleModule
{
    protected:
        AdditiveScramblerModule *scrambler;
        Ieee80211OfdmInterleaverModule *interleaver;
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
