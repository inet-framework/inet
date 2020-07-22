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

#ifndef __INET_CPROGRESS_H
#define __INET_CPROGRESS_H

#include "inet/common/INETDefs.h"

namespace inet {

class SIM_API cProgress : public cMessage
{
  friend class cSimpleModule;

  public:
    enum ProgressKind {
        PACKET_START,
        PACKET_END,
        PACKET_PROGRESS
    };

  protected:
    cPacket *packet = nullptr;
    double datarate = std::numeric_limits<double>::quiet_NaN();
    int bitPosition = -1;
    simtime_t timePosition = -1;
    int extraProcessableBitLength = 0;
    simtime_t extraProcessableDuration = 0;

  private:
    void copy(const cProgress& orig);

  public:
    cProgress(const char *name = nullptr, int kind = 0) : cMessage(name, kind) { }
    cProgress(const cProgress& orig) : cMessage(orig) { copy(orig); }

    virtual cProgress *dup() const override { return new cProgress(*this); }

    virtual void parsimPack(cCommBuffer *buffer) const override;
    virtual void parsimUnpack(cCommBuffer *buffer) override;

    cPacket *getPacket() const;
    cPacket *removePacket();
    void setPacket(cPacket *packet);

    double getDatarate() const { return datarate; }
    void setDatarate(double datarate) { this->datarate = datarate; }

    int getBitPosition() const { return bitPosition; }
    void setBitPosition(int bitPosition) { this->bitPosition = bitPosition; }

    simtime_t getTimePosition() const { return timePosition; }
    void setTimePosition(simtime_t timePosition) { this->timePosition = timePosition; }

    int getExtraProcessableBitLength() const { return extraProcessableBitLength; }
    void setExtraProcessableBitLength(int extraProcessableBitLength) { this->extraProcessableBitLength = extraProcessableBitLength; }

    simtime_t getExtraProcessableDuration() const { return extraProcessableDuration; }
    void setExtraProcessableDuration(simtime_t extraProcessableDuration) { this->extraProcessableDuration = extraProcessableDuration; }
};

} // namespace inet

#endif // ifndef __INET_CPROGRESS_H

