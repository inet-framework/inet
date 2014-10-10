//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_ISIGNALPACKETMODEL_H
#define __INET_ISIGNALPACKETMODEL_H

#include "inet/physicallayer/contract/IPrintableObject.h"
#include "inet/physicallayer/contract/IFECCoder.h"
#include "inet/physicallayer/contract/IScrambler.h"
#include "inet/physicallayer/contract/IInterleaver.h"

namespace inet {

namespace physicallayer {

/**
 * This purely virtual interface provides an abstraction for different radio
 * signal models in the packet domain.
 */
class INET_API ISignalPacketModel : public IPrintableObject
{
  public:
    virtual const cPacket *getPacket() const = 0;
};

class INET_API ITransmissionPacketModel : public virtual ISignalPacketModel
{
};

class INET_API IReceptionPacketModel : public virtual ISignalPacketModel
{
  public:

    /*
     *
     */
    virtual const IForwardErrorCorrection *getForwardErrorCorrection() const = 0;
    /*
     *
     */
    virtual const IScrambling *getScrambling() const = 0;
    /*
     *
     */
    virtual const IInterleaving *getInterleaving() const = 0;
    /**
     * Returns the packet error rate (probability).
     */
    virtual double getPER() const = 0;

    /**
     * Returns true if the packet is actually free of errors.
     */
    virtual bool isPacketErrorless() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ISIGNALPACKETMODEL_H
