//
// Copyright (C) 2015 Andras Varga
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
// Author: Andras Varga
//

#ifndef __INET_ITX_H
#define __INET_ITX_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class Ieee80211Frame;
class ITxCallback;

/**
 * Abstract interface for unconditionally transmitting a frame immediately
 * or after waiting for a specified inter-frame space (usually SIFS).
 */
class INET_API ITx
{
    public:
        virtual ~ITx() {}
        virtual void transmitFrame(Ieee80211Frame *frame, ITxCallback *txCallback) = 0;
        virtual void transmitFrame(Ieee80211Frame *frame, simtime_t ifs, ITxCallback *txCallback) = 0;
        virtual void radioTransmissionFinished() = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

