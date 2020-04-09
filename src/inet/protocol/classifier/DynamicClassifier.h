//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_DYNAMICCLASSIFIER_H
#define __INET_DYNAMICCLASSIFIER_H

#include "inet/queueing/classifier/PacketClassifier.h"

namespace inet {

using namespace inet::queueing;

class INET_API DynamicClassifier : public PacketClassifier
{
  protected:
    const char *submoduleName = nullptr;
    cModuleType *moduleType = nullptr;
    std::map<int, int> classIndexToGateItMap;

  protected:
    virtual void initialize(int stage) override;
    virtual int classifyPacket(Packet *packet) override;
};

} // namespace inet

#endif // ifndef __INET_DYNAMICCLASSIFIER_H

