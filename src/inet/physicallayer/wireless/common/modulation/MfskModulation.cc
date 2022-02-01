//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/modulation/MfskModulation.h"

namespace inet {

namespace physicallayer {

MfskModulation::MfskModulation(unsigned int codeWordSize) :
    codeWordSize(codeWordSize)
{
}

std::ostream& MfskModulation::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "MFSKModulaiton";
}

double MfskModulation::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    throw cRuntimeError("Not implemented yet");
}

double MfskModulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const
{
    throw cRuntimeError("Not implemented yet");
}

} // namespace physicallayer

} // namespace inet

