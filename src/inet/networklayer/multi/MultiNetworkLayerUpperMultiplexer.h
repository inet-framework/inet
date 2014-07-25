//
// Copyright (C) 2013 Opensim Ltd.
// Author: Levente Meszaros
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

#ifndef __INET_MULTINETWORKLAYERUPPERMULTIPLEXER_H
#define __INET_MULTINETWORKLAYERUPPERMULTIPLEXER_H

#include "inet/common/INETDefs.h"

namespace inet {

class MultiNetworkLayerUpperMultiplexer : public cSimpleModule
{
  public:
    MultiNetworkLayerUpperMultiplexer() {}
    virtual ~MultiNetworkLayerUpperMultiplexer() {}

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *message);
    int getProtocolCount();
    int getProtocolIndex(cMessage *message);
};

} // namespace inet

#endif // ifndef __INET_MULTINETWORKLAYERUPPERMULTIPLEXER_H

