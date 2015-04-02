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
            convCoder = getModuleFromPar<ConvolutionalCoderModule>(par("convolutionalCoderModule"), this);
        else if(!strcmp(testType, "interleaver"))
            interleaver = getModuleFromPar<Ieee80211OFDMInterleaverModule>(par("interleaverModule"), this);
        else if(!strcmp(testType, "scrambler"))
            scrambler = getModuleFromPar<AdditiveScramblerModule>(par("scramblerModule"), this);
        else if (!strcmp(testType, "all"))
        {
            convCoder = getModuleFromPar<ConvolutionalCoderModule>(par("convolutionalCoderModule"), this);
            interleaver = getModuleFromPar<Ieee80211OFDMInterleaverModule>(par("interleaverModule"), this);
            scrambler = getModuleFromPar<AdditiveScramblerModule>(par("scramblerModule"), this);
        }
        else
            throw cRuntimeError("Unknown (= %s) test type", testType);
    }
    else if (stage == INITSTAGE_LAST)
    {
        const char *testFile = par("testFile");
        fileStream = new std::ifstream(testFile);
        if (!fileStream->is_open())
            throw cRuntimeError("Cannot open the test file: %s", testFile);
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
    srand(time(NULL));
    std::string strInput;
    while (*fileStream >> strInput)
    {
        BitVector input = BitVector(strInput.c_str());
        BitVector encoded;
        encoded = convCoder->encode(input);
        int numOfErrors = numberOfRandomErrors;
        while (numOfErrors--)
        {
            int pos = rand() % encoded.getSize();
            encoded.toggleBit(pos);
        }
        BitVector decoded = convCoder->decode(encoded).first;
        if (input != decoded)
            EV_DETAIL << "Convolutional Coder test has failed" << endl;
    }
}

void Ieee80211BitDomainTest::testScrambler() const
{
    std::string strInput;
    while (*fileStream >> strInput)
    {
        BitVector input = BitVector(strInput.c_str());
        BitVector scrambledInput = scrambler->scramble(input);
        if (input != scrambler->descramble(scrambledInput))
            EV_DETAIL << "Descrambling has failed" << endl;
    }
}

void Ieee80211BitDomainTest::testInterleaver() const
{
    std::string strInput;
    while (*fileStream >> strInput)
    {
        BitVector input = BitVector(strInput.c_str());
        BitVector interleavedInput = interleaver->interleave(input);
        if (interleaver->deinterleave(interleavedInput) != input)
            EV_DETAIL << "Deinterleaving has failed" << endl;
    }
}

void Ieee80211BitDomainTest::testIeee80211BitDomain() const
{

    std::string strInput;
    *fileStream >> strInput;
    BitVector input = BitVector(strInput.c_str());
//    EV_DETAIL << "The scrambling sequence is: " << scrambler->getScramblingSequcene() << endl;
    BitVector scrambledInput = scrambler->scramble(input);
    BitVector bccEncodedInput = convCoder->encode(scrambledInput);
    BitVector interleavedInput = interleaver->interleave(bccEncodedInput);
    BitVector deinterleavedInput = interleaver->deinterleave(interleavedInput);
    if (bccEncodedInput != deinterleavedInput)
        EV_DETAIL << "Deinterleaving has failed" << endl;
    BitVector bccDecodedInput = convCoder->decode(deinterleavedInput).first;
    if (bccDecodedInput != scrambledInput)
        EV_DETAIL << "BCC decoding has failed" << endl;
    BitVector descrambledInput = scrambler->descramble(bccDecodedInput); // Note: scrambling and descrambling are the same operations
    if (descrambledInput != input)
        EV_DETAIL << "Descrambling has failed" << endl;
}

} /* namespace inet */
