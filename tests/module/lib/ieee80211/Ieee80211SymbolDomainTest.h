//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211SYMBOLMODELTEST_H
#define __INET_IEEE80211SYMBOLMODELTEST_H

#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmEncoderModule.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmModulatorModule.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDemodulatorModule.h"
#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmDecoderModule.h"

using namespace inet::physicallayer;

namespace inet {

class INET_API Ieee80211SymbolDomainTest : public cSimpleModule
{
    protected:
        Ieee80211OfdmEncoderModule *ieee80211OFDMSignalEncoder;
        Ieee80211OfdmEncoderModule *ieee80211OFDMDataEncoder;
        Ieee80211OfdmModulatorModule *ieee80211OFDMSignalModulator;
        Ieee80211OfdmModulatorModule *ieee80211OFDMDataModulator;
        Ieee80211OfdmDemodulatorModule *ieee80211OFDMSignalDemodulator;
        Ieee80211OfdmDemodulatorModule *ieee80211OFDMDataDemodulator;
        Ieee80211OfdmDecoderModule *ieee80211OFDMSignalDecoder;
        Ieee80211OfdmDecoderModule *ieee80211OFDMDataDecoder;
        BitVector input;

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }
        void parseInput(const char *fileName);

    public:
        void test() const;
};

} /* namespace inet */

#endif /* __INET_IEEE80211SYMBOLMODELTEST_H */

