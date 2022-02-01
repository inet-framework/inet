//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCoder.h"

#include "inet/common/BitVector.h"
#include "inet/common/ShortBitVector.h"

namespace inet {
namespace physicallayer {

std::ostream& ConvolutionalCoder::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ConvolutionalCoder";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(convolutionalCode, printFieldToString(convolutionalCode, level + 1, evFlags));
    return stream;
}

BitVector ConvolutionalCoder::getPuncturedIndices(unsigned int length) const
{
    BitVector isPunctured;
    isPunctured.appendBit(false, length);
    int puncturingMatrixSize = puncturingMatrix.at(0).getSize();
    for (unsigned int i = 0; i < codeRateParamaterN; i++) {
        int idx = 0;
        for (unsigned int j = 0; codeRateParamaterN * j + i < length; j++) {
            int place = idx % puncturingMatrixSize;
            bool stolen = !puncturingMatrix.at(i).getBit(place);
            isPunctured.setBit(codeRateParamaterN * j + i, stolen);
            idx++;
        }
    }
    return isPunctured;
}

BitVector ConvolutionalCoder::puncturing(const BitVector& informationBits) const
{
    BitVector puncturedInformationBits;
    BitVector isPunctured = getPuncturedIndices(informationBits.getSize());
    for (unsigned int i = 0; i < informationBits.getSize(); i++)
        if (!isPunctured.getBit(i))
            puncturedInformationBits.appendBit(informationBits.getBit(i));
    return puncturedInformationBits;
}

BitVector ConvolutionalCoder::depuncturing(const BitVector& decodedBits, BitVector& isPunctured) const
{
    ASSERT(decodedBits.getSize() % codeRatePuncturingN == 0);
    int encodedLength = (decodedBits.getSize() / codeRatePuncturingN) * codeRatePuncturingK;
    int depuncturedLength = (encodedLength / codeRateParamaterK) * codeRateParamaterN;
    isPunctured = getPuncturedIndices(depuncturedLength);
    BitVector depuncturedDecodedBits;
    int idx = 0;
    for (int i = 0; i < depuncturedLength; i++) {
        if (isPunctured.getBit(i))
            depuncturedDecodedBits.appendBit(false);
        else {
            depuncturedDecodedBits.appendBit(decodedBits.getBit(idx));
            idx++;
        }
    }
    return depuncturedDecodedBits;
}

ShortBitVector ConvolutionalCoder::inputSymbolToOutputSymbol(const ShortBitVector& state, const ShortBitVector& inputSymbol) const
{
    ShortBitVector outputSymbol;
    outputSymbol.appendBit(false, codeRateParamaterN);
    int shift = 0;
    for (int i = inputSymbol.getSize() - 1; i >= 0; i--) {
        ShortBitVector shiftRegisters;
        shiftRegisters.appendBit(inputSymbol.getBit(i));
        for (int k = shift; k < shift + memorySizes[i]; k++)
            shiftRegisters.appendBit((unsigned int)k >= state.getSize() ? false : state.getBit(k));
        const std::vector<ShortBitVector>& row = transferFunctionMatrix.at(i);
        for (unsigned int j = 0; j < row.size(); j++) {
            const ShortBitVector& generatorPolynomial = row.at(j);
            outputSymbol.setBit(j, eXOR(outputSymbol.getBit(j), modulo2Adder(shiftRegisters, generatorPolynomial)));
        }
        shift += memorySizes[i];
    }
    return outputSymbol;
}

bool ConvolutionalCoder::modulo2Adder(const ShortBitVector& shiftRegisters, const ShortBitVector& generatorPolynomial) const
{
    bool sum = false;
    for (unsigned int i = 0; i < generatorPolynomial.getSize(); i++) {
        if (generatorPolynomial.getBit(i))
            sum = eXOR(sum, shiftRegisters.getBit(i));
    }
    return sum;
}

void ConvolutionalCoder::computeOutputAndInputSymbols()
{
    for (int i = 0; i != numberOfInputSymbols; i++) {
        ShortBitVector inputSymbol(i, codeRateParamaterK);
        decimalToInputSymbol[inputSymbol.reverseToDecimal()] = inputSymbol;
    }
    for (int decState = 0; decState != numberOfStates; decState++) {
        ShortBitVector state(decState, memorySizeSum);
        for (int decInputSymbol = 0; decInputSymbol != numberOfInputSymbols; decInputSymbol++) {
            const ShortBitVector& inputSymbol = decimalToInputSymbol[decInputSymbol];
            ShortBitVector outputSymbol = inputSymbolToOutputSymbol(state, inputSymbol);
            int decOutputSymbol = outputSymbol.reverseToDecimal();
            int reverseDec = state.reverseToDecimal();
            outputSymbols[reverseDec][decInputSymbol] = outputSymbol;
            inputSymbols[reverseDec][decOutputSymbol] = decInputSymbol;
        }
    }
}

void ConvolutionalCoder::memoryAllocations()
{
    decimalToOutputSymbol = new ShortBitVector[numberOfOutputSymbols];
    outputSymbols = new ShortBitVector *[numberOfStates];
    decimalToInputSymbol = new ShortBitVector[numberOfInputSymbols];
    stateTransitions = new int *[numberOfStates];
    inputSymbols = new int *[numberOfStates];
    hammingDistanceLookupTable = new unsigned char **[numberOfOutputSymbols];
    for (int i = 0; i < numberOfOutputSymbols; i++) {
        hammingDistanceLookupTable[i] = new unsigned char *[numberOfOutputSymbols];
        for (int j = 0; j < numberOfOutputSymbols; j++)
            hammingDistanceLookupTable[i][j] = new unsigned char[numberOfOutputSymbols];
    }
    for (int i = 0; i < numberOfStates; i++) {
        stateTransitions[i] = new int[numberOfInputSymbols];
        inputSymbols[i] = new int[numberOfOutputSymbols];
        outputSymbols[i] = new ShortBitVector[numberOfInputSymbols];
        for (int j = 0; j < numberOfInputSymbols; j++) {
            stateTransitions[i][j] = -1;
            outputSymbols[i][j] = ShortBitVector();
        }
        for (int j = 0; j < numberOfOutputSymbols; j++)
            inputSymbols[i][j] = -1;
    }
}

void ConvolutionalCoder::computeMemorySizes()
{
    for (auto& elem : constraintLengths)
        memorySizes.push_back(elem - 1);
}

void ConvolutionalCoder::computeNumberOfStates()
{
    numberOfStates = (int)pow(2, memorySizeSum);
}

void ConvolutionalCoder::computeNumberOfInputAndOutputSymbols()
{
    numberOfInputSymbols = (int)pow(2, codeRateParamaterK);
    numberOfOutputSymbols = (int)pow(2, codeRateParamaterN);
}

void ConvolutionalCoder::computeMemorySizeSum()
{
    memorySizeSum = 0;
    for (auto& elem : memorySizes)
        memorySizeSum += elem;
}

void ConvolutionalCoder::computeHammingDistanceLookupTable()
{
    for (int i = 0; i < numberOfOutputSymbols; i++) {
        for (int j = 0; j < numberOfOutputSymbols; j++) {
            for (int k = 0; k < numberOfOutputSymbols; k++) {
                unsigned int hammingDistance = i ^ j;
                hammingDistance = hammingDistance & k;
                hammingDistance = hammingDistance - ((hammingDistance >> 1) & 0x55555555);
                hammingDistance = (hammingDistance & 0x33333333) + ((hammingDistance >> 2) & 0x33333333);
                hammingDistanceLookupTable[i][j][k] = (((hammingDistance + (hammingDistance >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
            }
        }
    }
}

void ConvolutionalCoder::initParameters()
{
    computeMemorySizes();
    computeMemorySizeSum();
    computeNumberOfStates();
    computeNumberOfInputAndOutputSymbols();
    memoryAllocations();
    computeOutputAndInputSymbols();
    computeStateTransitions();
    computeDecimalToOutputSymbolVector();
    computeHammingDistanceLookupTable();
}

BitVector ConvolutionalCoder::encode(const BitVector& informationBits) const
{
    EV_DEBUG << "Encoding the following bits: " << informationBits << endl;
    if (informationBits.getSize() % codeRateParamaterK)
        throw cRuntimeError("Length of informationBits must be a multiple of codeRateParamaterK = %d", codeRateParamaterK);
    BitVector encodedInformationBits;
    int state = 0;
    for (unsigned int i = 0; i < informationBits.getSize(); i += codeRateParamaterK) {
        ShortBitVector inputSymbol;
        for (unsigned int j = i; j < i + codeRateParamaterK; j++)
            inputSymbol.appendBit(informationBits.getBit(j));
        int inputSymbolDec = inputSymbol.reverseToDecimal();
        const ShortBitVector& encodedSymbol = outputSymbols[state][inputSymbolDec];
        state = stateTransitions[state][inputSymbolDec];
        for (unsigned int j = 0; j < encodedSymbol.getSize(); j++)
            encodedInformationBits.appendBit(encodedSymbol.getBit(j));
    }
    BitVector puncturedEncodedInformationBits = puncturing(encodedInformationBits);
    EV_DEBUG << "The encoded bits are: " << puncturedEncodedInformationBits << endl;
    return puncturedEncodedInformationBits;
}

void ConvolutionalCoder::printOutputs() const
{
    std::cout << "Outputs" << endl;
    for (int i = 0; i != numberOfStates; i++) {
        std::cout << outputSymbols[i][0].reverseToDecimal();
        for (int j = 1; j != numberOfInputSymbols; j++)
            std::cout << " " << outputSymbols[i][j].reverseToDecimal();
        std::cout << endl;
    }
    std::cout << "End Outputs" << endl;
}

void ConvolutionalCoder::printStateTransitions() const
{
    std::cout << "State transitions" << endl;
    for (int i = 0; i != numberOfStates; i++) {
        std::cout << stateTransitions[i][0];
        for (int j = 1; j != numberOfInputSymbols; j++)
            std::cout << " " << stateTransitions[i][j];
        std::cout << endl;
    }
    std::cout << "End State transitions" << endl;
}

void ConvolutionalCoder::computeStateTransitions()
{
    for (int decState = 0; decState != numberOfStates; decState++) {
        ShortBitVector state(decState, memorySizeSum);
        ShortBitVector shiftedState = state;
        shiftedState.leftShift(1);
        for (int is = 0; is != numberOfInputSymbols; is++) {
            ShortBitVector nextState = shiftedState;
            ShortBitVector inputSymbol(is, codeRateParamaterK);
            int shift = 0;
            for (int m = inputSymbol.getSize() - 1; m >= 0; m--) {
                nextState.setBit(shift, inputSymbol.getBit(m));
                shift += memorySizes.at(m);
            }
            stateTransitions[state.reverseToDecimal()][inputSymbol.reverseToDecimal()] = nextState.reverseToDecimal();
        }
    }
}

ShortBitVector ConvolutionalCoder::giveNextOutputSymbol(const BitVector& encodedBits, int decodedLength, const BitVector& isPunctured, ShortBitVector& countsOnHammingDistance) const
{
    if (isCompletelyDecoded(encodedBits.getSize(), decodedLength))
        return ShortBitVector();
    int pos = decodedLength / codeRateParamaterK;
    ShortBitVector nextSymbol;
    int idx = 0;
    for (unsigned int i = pos * codeRateParamaterN; i < (pos + 1) * codeRateParamaterN; i++) {
        if (isPunctured.getBit(i))
            countsOnHammingDistance.setBit(idx, false);
        nextSymbol.appendBit(encodedBits.getBit(i));
        idx++;
    }
    return nextSymbol;
}

bool ConvolutionalCoder::isCompletelyDecoded(unsigned int encodedLength, unsigned int decodedLength) const
{
    ASSERT(decodedLength % codeRateParamaterK == 0);
    unsigned int pos = decodedLength / codeRateParamaterK;
    return (pos + 1) * codeRateParamaterN > encodedLength;
}

void ConvolutionalCoder::parseMatrix(const char *strMatrix, std::vector<std::vector<int>>& matrix) const
{
    cStringTokenizer tokenizer(strMatrix, ";");
    while (tokenizer.hasMoreTokens()) {
        std::vector<int> row;
        parseVector(tokenizer.nextToken(), row);
        matrix.push_back(row);
    }
}

void ConvolutionalCoder::parseVector(const char *strVector, std::vector<int>& vector) const
{
    cStringTokenizer tokenizer(strVector);
    while (tokenizer.hasMoreTokens())
        vector.push_back(atoi(tokenizer.nextToken()));
}

void ConvolutionalCoder::convertToShortBitVectorMatrix(std::vector<std::vector<int>>& matrix, std::vector<ShortBitVector>& boolMatrix) const
{
    for (auto& elem : matrix) {
        std::vector<int> matrixRow = elem;
        ShortBitVector row;
        for (auto& matrixRow_j : matrixRow) {
            if (matrixRow_j == 1)
                row.appendBit(true);
            else if (matrixRow_j == 0)
                row.appendBit(false);
            else
                throw cRuntimeError("A bool matrix only contains 0-1 values");
        }
        boolMatrix.push_back(row);
    }
}

ShortBitVector ConvolutionalCoder::octalToBinary(int octalNum, int fixedSize) const
{
    unsigned int powerOfEight = 1;
    unsigned int decimal = 0;
    while (octalNum > 0) {
        unsigned int octalDigit = octalNum % 10;
        octalNum /= 10;
        decimal += octalDigit * powerOfEight;
        powerOfEight *= 8;
    }
    return ShortBitVector(decimal, fixedSize);
}

void ConvolutionalCoder::setTransferFunctionMatrix(std::vector<std::vector<int>>& transferFMatrix)
{
    for (unsigned int i = 0; i < transferFMatrix.size(); i++) {
        const std::vector<int>& row = transferFMatrix.at(i);
        std::vector<ShortBitVector> bitRow;
        for (auto& elem : row) {
            ShortBitVector bin = octalToBinary(elem, constraintLengths.at(i));
            ShortBitVector reverseBin;
            for (int k = bin.getSize() - 1; k >= 0; k--)
                reverseBin.appendBit(bin.getBit(k));
            bitRow.push_back(reverseBin);
        }
        transferFunctionMatrix.push_back(bitRow);
    }
}

void ConvolutionalCoder::printTransferFunctionMatrix() const
{
    std::cout << "Transfer function matrix" << endl;
    for (auto& elem : transferFunctionMatrix) {
        std::cout << elem.at(0);
        for (unsigned int j = 1; j < elem.size(); j++)
            std::cout << "," << elem.at(j);
        std::cout << endl;
    }
}

void ConvolutionalCoder::computeDecimalToOutputSymbolVector()
{
    for (int i = 0; i != numberOfOutputSymbols; i++) {
        ShortBitVector outputSymbol(i, codeRateParamaterN);
        decimalToOutputSymbol[outputSymbol.reverseToDecimal()] = outputSymbol;
    }
}

void ConvolutionalCoder::updateTrellisGraph(TrellisGraphNode **trellisGraph, unsigned int time, const ShortBitVector& outputSymbol, const ShortBitVector& excludedFromHammingDistance) const
{
    int tieBreakingCounter = 0;
    for (int prevState = 0; prevState != numberOfStates; prevState++) {
        const TrellisGraphNode& node = trellisGraph[prevState][time];
        for (int j = 0; j != numberOfOutputSymbols && node.state != -1; j++) {
            int feasibleDecodedSymbol = inputSymbols[prevState][j];
            if (feasibleDecodedSymbol != -1) {
                const ShortBitVector& otherOutputSymbol = decimalToOutputSymbol[j];
                int hammingDistance = computeHammingDistance(outputSymbol, excludedFromHammingDistance, otherOutputSymbol);
                int newState = stateTransitions[prevState][feasibleDecodedSymbol];
                int cumulativeHammingDistance = hammingDistance;
                if (node.comulativeHammingDistance != INT32_MAX)
                    cumulativeHammingDistance += node.comulativeHammingDistance;
                TrellisGraphNode& best = trellisGraph[newState][time + 1];
                bool replace = false;
                if (cumulativeHammingDistance == best.comulativeHammingDistance) {
                    tieBreakingCounter++;
                    if (RNGCONTEXT dblrand() < 1.0 / tieBreakingCounter)
                        replace = true;
                }
                else if (cumulativeHammingDistance < best.comulativeHammingDistance) {
                    tieBreakingCounter = 0;
                    replace = true;
                }
                if (replace) {
                    best.state = newState;
                    best.prevState = prevState;
                    best.depth = node.depth + 1;
                    best.comulativeHammingDistance = cumulativeHammingDistance;
                    if (hammingDistance > 0)
                        best.numberOfErrors = node.numberOfErrors + 1;
                    else
                        best.numberOfErrors = node.numberOfErrors;
                    best.symbol = feasibleDecodedSymbol;
                }
            }
        }
    }
}

std::pair<BitVector, bool> ConvolutionalCoder::traversePath(const TrellisGraphNode& bestNode, TrellisGraphNode **trellisGraph, bool isTruncatedMode) const
{
    if (!isTruncatedMode && bestNode.symbol == -1)
        return std::pair<BitVector, bool>(BitVector(), false);
    TrellisGraphNode path = bestNode;
    BitVector reverseDecodedBits;
    int depth = bestNode.depth;
    while (depth--) {
        const ShortBitVector& inputSymbol = decimalToInputSymbol[path.symbol];
        for (int i = inputSymbol.getSize() - 1; i >= 0; i--)
            reverseDecodedBits.appendBit(inputSymbol.getBit(i));
        path = trellisGraph[path.prevState][depth];
    }
    BitVector decodedBits;
    for (int i = reverseDecodedBits.getSize() - 1; i >= 0; i--)
        decodedBits.appendBit(reverseDecodedBits.getBit(i));
    return std::pair<BitVector, bool>(decodedBits, true);
}

std::pair<BitVector, bool> ConvolutionalCoder::decode(const BitVector& encodedBits) const
{
    BitVector isPunctured;
    BitVector depuncturedEncodedBits = depuncturing(encodedBits, isPunctured);
    unsigned int encodedBitsSize = depuncturedEncodedBits.getSize();
    if (encodedBitsSize % codeRateParamaterN != 0)
        throw cRuntimeError("Length of encodedBits must be a multiple of codeRateParamaterN = %d", codeRateParamaterN);
    bool isTruncatedMode = false;
    if (!strcmp(mode, "truncated"))
        isTruncatedMode = true;
    else if (!strcmp(mode, "terminated"))
        isTruncatedMode = false;
    else
        throw cRuntimeError("Unknown decodingMode = %s decodingMode", mode);
    TrellisGraphNode **trellisGraph;
    trellisGraph = new TrellisGraphNode *[numberOfStates];
    for (int i = 0; i != numberOfStates; i++) {
        int length = depuncturedEncodedBits.getSize() / codeRateParamaterN + 1;
        trellisGraph[i] = new TrellisGraphNode[length];
        for (int j = 0; j < length; j++)
            trellisGraph[i][j] = TrellisGraphNode(-1, -1, -1, INT32_MAX, 0, 0);
    }
    trellisGraph[0][0].state = 0;
    EV_DEBUG << "Decoding the following bits: " << depuncturedEncodedBits << endl;
    ShortBitVector countsOnHammingDistance;
    countsOnHammingDistance.appendBit(true, codeRateParamaterN);
    ShortBitVector nextOutputSymbol = giveNextOutputSymbol(depuncturedEncodedBits, 0, isPunctured, countsOnHammingDistance);
    unsigned int time = 0;
    while (!nextOutputSymbol.isEmpty()) {
        updateTrellisGraph(trellisGraph, time, nextOutputSymbol, countsOnHammingDistance);
        time++;
        countsOnHammingDistance = ShortBitVector();
        countsOnHammingDistance.appendBit(true, codeRateParamaterN);
        nextOutputSymbol = giveNextOutputSymbol(depuncturedEncodedBits, time * codeRateParamaterK, isPunctured, countsOnHammingDistance);
    }
    TrellisGraphNode bestNode;
    if (!isTruncatedMode)
        bestNode = trellisGraph[0][time];
    else {
        bestNode = trellisGraph[0][time];
        for (int i = 1; i != numberOfStates; i++) {
            TrellisGraphNode currentNode = trellisGraph[i][time];
            if (currentNode.symbol != -1 && currentNode.comulativeHammingDistance < bestNode.comulativeHammingDistance)
                bestNode = currentNode;
        }
    }
    std::pair<BitVector, bool> result = traversePath(bestNode, trellisGraph, isTruncatedMode);
    if (result.second) {
        EV_DEBUG << "Recovered message: " << result.first << endl;
        EV_DEBUG << "Number of errors: " << bestNode.numberOfErrors
                 << " Cumulative error (Hamming distance): " << bestNode.comulativeHammingDistance
                 << " End state: " << bestNode.state << endl;
    }
    else
        EV_DEBUG << "None of the paths in the trellis graph lead to the all-zeros state" << endl;
    for (int i = 0; i < numberOfStates; i++)
        delete[] trellisGraph[i];
    delete[] trellisGraph;
    return result;
}

ConvolutionalCoder::ConvolutionalCoder(const ConvolutionalCode *convolutionalCode) : convolutionalCode(convolutionalCode)
{
    const char *strTransferFunctionMatrix = convolutionalCode->getTransferFunctionMatrix();
    const char *strPuncturingMatrix = convolutionalCode->getPuncturingMatrix();
    const char *strConstraintLengthVector = convolutionalCode->getConstraintLengthVector();
    mode = convolutionalCode->getMode();
    if (strcmp(mode, "terminated") && strcmp(mode, "truncated"))
        throw cRuntimeError("Unknown (= %s ) coding mode", mode);
    codeRatePuncturingK = convolutionalCode->getCodeRatePuncturingK();
    codeRatePuncturingN = convolutionalCode->getCodeRatePuncturingN();
    parseVector(strConstraintLengthVector, constraintLengths);
    std::vector<std::vector<int>> pMatrix;
    parseMatrix(strPuncturingMatrix, pMatrix);
    convertToShortBitVectorMatrix(pMatrix, puncturingMatrix);
    std::vector<std::vector<int>> transferFMatrix;
    parseMatrix(strTransferFunctionMatrix, transferFMatrix);
    codeRateParamaterK = transferFMatrix.size();
    codeRateParamaterN = transferFMatrix.at(0).size();
    for (unsigned int i = 0; i < transferFMatrix.size(); i++) {
        for (unsigned int j = 0; j < transferFMatrix.at(i).size(); j++) {
            int decPolynom = octalToDec(transferFMatrix.at(i).at(j));
            int constraintLength = constraintLengths[i];
            if (decPolynom >= pow(2, constraintLength))
                throw cRuntimeError("Code size is greater then constraint length");
        }
    }
    // TODO check whether it is less than..
    for (unsigned int i = 0; i < codeRateParamaterK; i++)
        if (transferFMatrix.at(i).size() != codeRateParamaterN)
            throw cRuntimeError("Transfer function matrix must be a k-by-n matrix of octal numbers");
    if (transferFMatrix.size() != constraintLengths.size())
        throw cRuntimeError("Constraint length vector must be a 1-by-k vector of integers");
    if (puncturingMatrix.size() != codeRateParamaterN)
        throw cRuntimeError("Puncturing matrix must be a n-by-arbitrary bool matrix");
    setTransferFunctionMatrix(transferFMatrix);
    initParameters();
}

int ConvolutionalCoder::octalToDec(int octalNum) const
{
    int powersOfEight = 1;
    int decVal = 0;
    while (octalNum != 0) {
        int octDigit = octalNum % 10;
        decVal += octDigit * powersOfEight;
        octalNum /= 10;
        powersOfEight *= 8;
    }
    return decVal;
}

ConvolutionalCoder::~ConvolutionalCoder()
{
    for (int i = 0; i < numberOfOutputSymbols; i++) {
        for (int j = 0; j < numberOfOutputSymbols; j++)
            delete[] hammingDistanceLookupTable[i][j];
        delete[] hammingDistanceLookupTable[i];
    }
    delete[] hammingDistanceLookupTable;
    for (int i = 0; i < numberOfStates; i++) {
        delete[] stateTransitions[i];
        delete[] outputSymbols[i];
        delete[] inputSymbols[i];
    }
    delete[] stateTransitions;
    delete[] outputSymbols;
    delete[] inputSymbols;
    delete[] decimalToOutputSymbol;
    delete[] decimalToInputSymbol;
}

} /* namespace physicallayer */
} /* namespace inet */

