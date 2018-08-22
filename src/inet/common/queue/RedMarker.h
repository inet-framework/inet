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

#ifndef __INET_REDMARKER_H_
#define __INET_REDMARKER_H_



#include "inet/common/INETDefs.h"
#include "inet/common/queue/RedDropper.h"
#include "inet/common/packet/Packet.h"



namespace inet {


class INET_API RedMarker : public RedDropper
{
  public:
    RedMarker();

  protected:
    double *marks;
    bool markNext;

  protected:

    virtual ~RedMarker();
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    bool shouldMark(cPacket *packet);
    virtual bool shouldDrop(cPacket *packet) override;
    bool markPacket(Packet *packet);
};

} //namespace

#endif
