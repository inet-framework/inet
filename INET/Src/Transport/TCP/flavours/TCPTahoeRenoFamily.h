//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __TCPTAHOERENOFAMILY_H
#define __TCPTAHOERENOFAMILY_H

#include <omnetpp.h>
#include "TCPBaseAlg.h"


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

#endif


