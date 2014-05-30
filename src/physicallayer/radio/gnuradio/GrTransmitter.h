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

#ifndef __INET_GRTRANSMITTER_H_
#define __INET_GRTRANSMITTER_H_

#include <gnuradio/gr_complex.h>
#include "bbn_transmitter.h"
#include "IRadioSignalTransmission.h"
#include "IRadioSignalTransmitter.h"
#include "ImplementationBase.h"
#include "GrSignal.h"

class INET_API GrTransmitter : public cCompoundModule, public IRadioSignalTransmitter
{
        bbn_transmitter_sptr transmitter;
    public:
        GrTransmitter();
        virtual ~GrTransmitter();
        virtual const IRadioSignalTransmission *createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const;
        virtual void printToStream(std::ostream&) const;
        virtual W getMaxPower() const { return W(NaN); }
};

#endif
