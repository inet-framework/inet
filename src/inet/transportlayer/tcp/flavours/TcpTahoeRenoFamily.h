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

#ifndef __INET_TCPTAHOERENOFAMILY_H
#define __INET_TCPTAHOERENOFAMILY_H

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/tcp/flavours/TcpBaseAlg.h"

namespace inet {
namespace tcp {

/**
 * State variables for TcpTahoeRenoFamily.
 */
class INET_API TcpTahoeRenoFamilyStateVariables : public TcpBaseAlgStateVariables
{
  public:
    TcpTahoeRenoFamilyStateVariables();
    virtual std::string str() const override;
    virtual std::string detailedInfo() const override;
    virtual void setSendQueueLimit(uint32 newLimit);

    uint32 ssthresh;    ///< slow start threshold
};

/**
 * Provides utility functions to implement TcpTahoe, TcpReno and TcpNewReno.
 * (TcpVegas should inherit from TcpBaseAlg instead of this one.)
 */
class INET_API TcpTahoeRenoFamily : public TcpBaseAlg
{
  protected:
    TcpTahoeRenoFamilyStateVariables *& state;    // alias to TcpAlgorithm's 'state'

  public:
    /** Ctor */
    TcpTahoeRenoFamily();
};

} // namespace tcp
} // namespace inet

#endif // ifndef __INET_TCPTAHOERENOFAMILY_H

