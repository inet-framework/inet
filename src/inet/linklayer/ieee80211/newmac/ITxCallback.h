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

#ifndef __INET_ITXCALLBACK_H
#define __INET_ITXCALLBACK_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

/**
 * Tx processes use this interface to notify their callers that the
 * last frame transmission has been completed, or that the transmission
 * has had an internal collision with a higher priority Tx process (EDCA).
 *
 * @see IContentionTx and IImmediateTx
 */
class INET_API ITxCallback {
    public:
        virtual void transmissionComplete(int txIndex) = 0; // -1: immediate tx
        virtual void internalCollision(int txIndex) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

