//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONVOLUTIONALCODER_H
#define __INET_CONVOLUTIONALCODER_H

#include <queue>
#include <vector>

#include "inet/common/BitVector.h"
#include "inet/common/ShortBitVector.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IFecCoder.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCode.h"

namespace inet {
namespace physicallayer {

/*
 * This class implements a feedforward (k/n)-convolutional encoder/decoder.
 * The decoder implements hard-decision Viterbi algorithm with Hamming-distance
 * metric.
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
class INET_API ConvolutionalCoder : public IFecCoder
{
  public:
    typedef std::vector<std::vector<ShortBitVector>> ShortBitVectorMatrix;

    class INET_API TrellisGraphNode {
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
    std::vector<std::vector<TrellisGraphNode>> trellisGraph; // the decoder's trellis graph
    const ConvolutionalCode *convolutionalCode; // this info class holds information related to this encoder

  protected:
    inline bool eXOR(bool alpha, bool beta) const { return alpha != beta; }
    void setTransferFunctionMatrix(std::vector<std::vector<int>>& transferFMatrix);
    ShortBitVector inputSymbolToOutputSymbol(const ShortBitVector& state, const ShortBitVector& inputSymbol) const;
    bool modulo2Adder(const ShortBitVector& shiftRegisters, const ShortBitVector& generatorPolynomial) const;
    ShortBitVector giveNextOutputSymbol(const BitVector& encodedBits, int decodedLength, const BitVector& isPunctured, ShortBitVector& excludedFromHammingDistance) const;
    BitVector puncturing(const BitVector& informationBits) const;
    BitVector depuncturing(const BitVector& decodedBits, BitVector& isPunctured) const;
    BitVector getPuncturedIndices(unsigned int length) const;
    inline unsigned int computeHammingDistance(const ShortBitVector& u, const ShortBitVector& excludedBits, const ShortBitVector& w) const
    {
#ifndef NDEBUG
        if (u.isEmpty() || w.isEmpty())
            throw cRuntimeError("You can't compute the Hamming distance between undefined BitVectors");
        if (u.getSize() != w.getSize())
            throw cRuntimeError("You can't compute Hamming distance between two vectors with different sizes");
#endif // ifndef NDEBUG
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
    void parseMatrix(const char *strMatrix, std::vector<std::vector<int>>& matrix) const;
    void parseVector(const char *strVector, std::vector<int>& vector) const;
    void convertToShortBitVectorMatrix(std::vector<std::vector<int>>& matrix, std::vector<ShortBitVector>& boolMatrix) const;
    ShortBitVector octalToBinary(int octalNum, int fixedSize) const;
    int octalToDec(int octalNum) const;
    std::pair<BitVector, bool> traversePath(const TrellisGraphNode& bestNode, TrellisGraphNode **bestPaths, bool isTruncatedMode) const;

  public:
    ConvolutionalCoder(const ConvolutionalCode *convolutionalCode);
    ~ConvolutionalCoder();

    BitVector encode(const BitVector& informationBits) const override;
    std::pair<BitVector, bool> decode(const BitVector& encodedBits) const override;
    const ConvolutionalCode *getForwardErrorCorrection() const override { return convolutionalCode; }

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
    const int **getStateTransitionTable() const { return (const int **)stateTransitions; }
    const int **getOutputTable() const { return (const int **)inputSymbols; }

    /* IPrintable object */
    std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};
} /* namespace physicallayer */
} /* namespace inet */

#endif

