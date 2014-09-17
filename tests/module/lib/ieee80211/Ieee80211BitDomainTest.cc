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

#include "Ieee80211BitDomainTest.h"
#include "inet/common/BitVector.h"
#include "inet/common/ShortBitVector.h"
#include <fstream>

namespace inet {

Define_Module(Ieee80211BitDomainTest);

void Ieee80211BitDomainTest::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        convCoder = NULL;
        interleaver = NULL;
        scrambler = NULL;
        testType = par("testType");
        if (!strcmp(testType,"convCoder"))
            convCoder = getModuleFromPar<ConvolutionalCoder>(par("convolutionalCoderModule"), this);
        else if(!strcmp(testType, "interleaver"))
            interleaver = getModuleFromPar<Ieee80211Interleaver>(par("interleaverModule"), this);
        else if(!strcmp(testType, "scrambler"))
            scrambler = getModuleFromPar<Ieee80211Scrambler>(par("scramblerModule"), this);
        else if (!strcmp(testType, "all"))
        {
            convCoder = getModuleFromPar<ConvolutionalCoder>(par("convolutionalCoderModule"), this);
            interleaver = getModuleFromPar<Ieee80211Interleaver>(par("interleaverModule"), this);
            scrambler = getModuleFromPar<Ieee80211Scrambler>(par("scramblerModule"), this);
        }
        else
            throw cRuntimeError("Unknown (= %s) test type", testType);
        const char *testFile = par("testFile");
        fileStream = new std::ifstream(testFile);
        if (!fileStream->is_open())
            throw cRuntimeError("Cannot open the test file: %s", testFile);
    }
    else if (stage == INITSTAGE_LAST)
    {
        int numberOfRandomErrors = par("numberOfRandomErrors");
        if (!strcmp(testType,"convCoder"))
            testConvolutionalCoder(numberOfRandomErrors);
        else if(!strcmp(testType, "interleaver"))
            testInterleaver();
        else if(!strcmp(testType, "scrambler"))
            testScrambler();
        else if (!strcmp(testType, "all"))
            testIeee80211BitDomain();
    }
}

void Ieee80211BitDomainTest::testConvolutionalCoder(unsigned int numberOfRandomErrors) const
{
    fileStream->clear();
    fileStream->seekg(0, std::ios::beg);
    srand(time(NULL));
    std::string line;
    while (*fileStream >> line)
    {
        BitVector input(line.c_str());
        BitVector encoded;
        encoded = convCoder->encode(input);
        int numOfErrors = numberOfRandomErrors;
        while (numOfErrors--)
        {
            int pos = rand() % encoded.getSize();
            encoded.toggleBit(pos);
        }
        BitVector decoded = convCoder->decode(encoded);
        if (input != decoded)
            EV_DETAIL << "Convolutional Coder test has failed" << endl;
    }
}

void Ieee80211BitDomainTest::testScrambler() const
{
    fileStream->clear();
    fileStream->seekg(0, std::ios::beg);
    std::string line;
    while (*fileStream >> line)
    {
        BitVector input(line.c_str());
        BitVector scrambledInput = scrambler->scrambling(input);
        if (input != scrambler->scrambling(scrambledInput))
            EV_DETAIL << "Descrambling has failed" << endl;
    }
}

void Ieee80211BitDomainTest::testInterleaver() const
{
    fileStream->clear();
    fileStream->seekg(0, std::ios::beg);
    std::string line;
    while (*fileStream >> line)
    {
        BitVector input(line.c_str());
        BitVector interleavedInput = interleaver->interleaving(input);
        if (interleaver->deinterleaving(interleavedInput) != input)
            EV_DETAIL << "Deinterleaving has failed" << endl;
    }
}

void Ieee80211BitDomainTest::testIeee80211BitDomain() const
{
    fileStream->clear();
    fileStream->seekg(0, std::ios::beg);
    std::string line;
    EV_DETAIL << "The scrambling sequence is: " << scrambler->getScramblingSequcene() << endl;
    while (*fileStream >> line)
    {
        BitVector input(line.c_str());
        BitVector scrambledInput = scrambler->scrambling(input);
        BitVector bccEncodedInput = convCoder->encode(scrambledInput);
        BitVector interleavedInput = interleaver->interleaving(bccEncodedInput);
        BitVector deinterleavedInput = interleaver->deinterleaving(interleavedInput);
        if (bccEncodedInput != deinterleavedInput)
            EV_DETAIL << "Deinterleaving has failed" << endl;
        BitVector bccDecodedInput = convCoder->decode(deinterleavedInput);
        if (bccDecodedInput != scrambledInput)
            EV_DETAIL << "BCC decoding has failed" << endl;
        BitVector descrambledInput = scrambler->scrambling(bccDecodedInput); // Note: scrambling and descrambling are the same operations
        if (descrambledInput != input)
            EV_DETAIL << "Descrambling has failed" << endl;
    }
}

Ieee80211BitDomainTest::~Ieee80211BitDomainTest()
{
    fileStream->close();
    delete fileStream;
}

} /* namespace inet */
