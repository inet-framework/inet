//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __MAC_IIEEE80211MACIMMEDIATETX_H_
#define __MAC_IIEEE80211MACIMMEDIATETX_H_

#include "inet/common/INETDefs.h"
#include "IIeee80211MacTx.h"

namespace inet {

namespace ieee80211 {

class IIeee80211MacImmediateTx
{
    public:
        typedef IIeee80211MacTx::ICallback ICallback;
        virtual ~IIeee80211MacImmediateTx() {}
        virtual void transmitImmediateFrame(Ieee80211Frame *frame, simtime_t ifs, ICallback *completionCallback) = 0;
        virtual void radioTransmissionFinished() = 0;
};

}

} //namespace

#endif
