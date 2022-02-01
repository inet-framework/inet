//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKSYMBOL_H
#define __INET_APSKSYMBOL_H

//#include "inet/common/Complex.h"
#include <complex>

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISymbol.h"

namespace inet {

namespace physicallayer {

class INET_API ApskSymbol : public std::complex<double>, public ISymbol
{
  public:
    ApskSymbol() : std::complex<double>() {}
    ApskSymbol(const double& r) : std::complex<double>(r) {}
    ApskSymbol(const double& q, const double& i) : std::complex<double>(q, i) {}
    ApskSymbol(const std::complex<double>& w) : std::complex<double>(w) {}
};

} // namespace physicallayer

} // namespace inet

#endif

