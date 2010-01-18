//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_TCPTAHOERENOFAMILY_OLD_H
#define __INET_TCPTAHOERENOFAMILY_OLD_H

#include <omnetpp.h>
#include "TCPBaseAlg_old.h"

namespace tcp_old {

/**
 * State variables for TCPTahoeRenoFamily.
 */
class INET_API TCPTahoeRenoFamilyStateVariables : public TCPBaseAlgStateVariables
{
  public:
    TCPTahoeRenoFamilyStateVariables();
    virtual std::string info() const;
    virtual std::string detailedInfo() const;

    uint ssthresh;  ///< slow start threshold
};


/**
 * Provides utility functions to implement TCPTahoe, TCPReno and TCPNewReno.
 * (TCPVegas should inherit from TCPBaseAlg instead of this one.)
 */
class INET_API TCPTahoeRenoFamily : public TCPBaseAlg
{
  protected:
    TCPTahoeRenoFamilyStateVariables *&state; // alias to TCLAlgorithm's 'state'

  public:
    /** Ctor */
    TCPTahoeRenoFamily();
};

}
#endif


