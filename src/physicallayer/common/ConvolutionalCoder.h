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

#ifndef __INET_CONVOLUTIONALCODER_H_
#define __INET_CONVOLUTIONALCODER_H_

#include "inet/common/INETDefs.h"
#include "inet/common/BitVector.h"
#include "inet/common/ShortBitVector.h"
#include <vector>
#include <queue>

namespace inet {
namespace physicallayer {

/*
 * This class implements a feedforward (k/n)-convolutional encoder/decoder.
 * The decoder implements hard-decision Viterbi algorithm with Hamming-distance
 * metric.
 * By default, it supports the following code rates often used by IEEE802.11 PHY:
 *
 *  - k = 1, n = 2 with constraint length 7, generator polynomials: (133)_8 = (1011011)_2,
 *                                                                  (171)_8 = (1111001)_2
 * Higher code rates are achieved by puncturing:
 *
 *  - k = 2, n = 3 with puncturing matrix: |1 1|
 *                                         |1 0|
 *
 *  - k = 3, n = 4 with puncturing matrix: |1 1 0|
 *                                         |1 0 1|
 *
 * We use industry-standard generator polynomials.
 * The encoder and decoder algorithm can handle arbitrary (k/n) code rates
 * and constraint lengths, in this case, you have to define your own transfer
 * function matrix and puncturing matrix.
 *
 * References:
 *  [1]  Encoder implementation based on this lecture note: http://ecee.colorado.edu/~mathys/ecen5682/slides/conv99.pdf
 *  [2]  Decoder implementation based on this description: http://www.ee.unb.ca/tervo/ee4253/convolution3.shtml
 *  [3]  Generator polynomials can be found in 18.3.5.6 Convolutional encoder, IEEE Part 11: Wireless LAN Medium Access Control
    (MAC) and Physical Layer (PHY) Specifications
    [4]  Puncturing matrices came from http://en.wikipedia.org/wiki/Convolutional_code#Punctured_convolutional_codes
 */
class ConvolutionalCoder : public cSimpleModule
{
    public:
        typedef std::vector<std::vector<ShortBitVector> > ShortBitVectorMatrix;

        class TrellisGraphNode
        {
            public:
                int symbol;
                int state;
                int prevState;
                int comulativeHammingDistance;
                int numberOfErrors;
                unsigned int depth;
                TrellisGraphNode() : symbol(0), state(0), prevState(0), comulativeHammingDistance(0), numberOfErrors(0), depth(0) {}
                TrellisGraphNode(int symbol, int state, int prevState, int hammingDistance, int numberOfErrors, unsigned int depth) :
                    symbol(symbol), state(state), prevState(prevState), comulativeHammingDistance(hammingDistance), numberOfErrors(numberOfErrors), depth(depth) {};
        };

    protected:
        const char *mode;
        unsigned int codeRateParamaterK, codeRateParamaterN; // these define the k/n code rate
        unsigned int codeRatePuncturingK, codeRatePuncturingN; // the k/n code rate after puncturing
        int memorySizeSum; // sum of memorySizes
        std::vector<int> memorySizes; // constraintLengths[i] - 1 for all i
        std::vector<int> constraintLengths; // the delay for the encoder's k input bit streams
        int numberOfStates; // 2^memorySizeSum
        int numberOfInputSymbols; // 2^k, where k is the parameter from k/n
        int numberOfOutputSymbols; // 2^n where n is the parameter from k/n
        ShortBitVectorMatrix transferFunctionMatrix; // matrix of the generator polynomials
        std::vector<ShortBitVector> puncturingMatrix; // defines the puncturing method
        int **inputSymbols; // maps a (state, outputSymbol) pair to the corresponding input symbol
        ShortBitVector **outputSymbols; // maps a (state, inputSymbol) pair to the corresponding output symbol
        ShortBitVector *decimalToInputSymbol; // maps an inputSymbol (in decimal) to its ShortBitVector representation
        ShortBitVector *decimalToOutputSymbol; // maps an outputSymbol (in decimal) to its ShortBitVector representation
        int **stateTransitions; // maps a (state, inputSymbol) pair to the corresponding next state
        unsigned char ***hammingDistanceLookupTable; // lookup table for Hamming distances, the three dimensions are: [outputSymbol, outputSymbol, excludedBits]
        std::vector<std::vector<TrellisGraphNode> > trellisGraph; // the decoder's trellis graph

    protected:
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }
        inline bool eXOR(bool alpha, bool beta) const
        {
            return (alpha || beta) && !(alpha && beta);
        }
        void setTransferFunctionMatrix(std::vector<std::vector<int> >& transferFMatrix);
        ShortBitVector inputSymbolToOutputSymbol(const ShortBitVector& state, const ShortBitVector& inputSymbol) const;
        bool modulo2Adder(const ShortBitVector& shiftRegisters, const ShortBitVector& generatorPolynomial) const;
        ShortBitVector giveNextOutputSymbol(const BitVector& encodedBits, int decodedLength, const BitVector& isPunctured, ShortBitVector& excludedFromHammingDistance) const;
        BitVector puncturing(const BitVector& informationBits) const;
        BitVector depuncturing(const BitVector& decodedBits, BitVector& isPunctured) const;
        BitVector getPuncturedIndices(unsigned int length) const;
        inline unsigned int computeHammingDistance(const ShortBitVector& u, const ShortBitVector& excludedBits, const ShortBitVector& w) const
        {
#ifndef NDEBUG
            if (u.isUndef() || w.isUndef())
                throw cRuntimeError("You can't compute the Hamming distance between undefined BitVectors");
            if (u.getSize() != w.getSize())
                throw cRuntimeError("You can't compute Hamming distance between two vectors with different sizes");
#endif
            return hammingDistanceLookupTable[u.toDecimal()][w.toDecimal()][excludedBits.toDecimal()];
        }
        void updateTrellisGraph(TrellisGraphNode **trellisGraph, unsigned int time, const ShortBitVector& outputSymbol, const ShortBitVector& excludedFromHammingDistance) const;
        bool isCompletelyDecoded(unsigned int encodedLength, unsigned int decodedLength) const;
        void initParameters();
        void memoryAllocations();
        void computeHammingDistanceLookupTable();
        void computeMemorySizes();
        void computeMemorySizeSum();
        void computeNumberOfStates();
        void computeNumberOfInputAndOutputSymbols();
        void computeStateTransitions();
        void computeOutputAndInputSymbols();
        void computeDecimalToOutputSymbolVector();
        void printStateTransitions() const;
        void printOutputs() const;
        void printTransferFunctionMatrix() const;
        void parseMatrix(const char *strMatrix, std::vector<std::vector<int> >& matrix) const;
        void parseVector(const char *strVector, std::vector<int>& vector) const;
        void convertToShortBitVectorMatrix(std::vector<std::vector<int> >& matrix, std::vector<ShortBitVector>& boolMatrix) const;
        ShortBitVector octalToBinary(int octalNum, int fixedSize) const;
        BitVector traversePath(const TrellisGraphNode& bestNode, TrellisGraphNode **bestPaths) const;

    public:

        BitVector encode(const BitVector& informationBits) const;
        BitVector decode(const BitVector& encodedBits) const;

        /*
         * Getters for the encoder's/decoder's parameters
         */
        unsigned int getMemorySizeSum() const { return memorySizeSum; }
        const std::vector<int>& getConstraintLengthVector() const { return constraintLengths; }
        const ShortBitVectorMatrix& getTransferFunctionMatrix() const { return transferFunctionMatrix; }
        const std::vector<ShortBitVector>& getPuncturingMatrix() const { return puncturingMatrix; }
        int getNumberOfStates() const { return numberOfStates; }
        int getNumberOfOutputSymbols() const { return numberOfOutputSymbols; }
        int getNumberOfInputSymbols() const { return numberOfInputSymbols; }

        /*
         * Getters for the code's state transition table and output table
         */
        const int** getStateTransitionTable() const { return (const int**)stateTransitions; }
        const int** getOutputTable() const { return (const int**)inputSymbols; }

        ~ConvolutionalCoder();
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_CONVOLUTIONALCODER_H_ */
