//
// Copyright (C) 2013 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_NARROWBANDRADIOBASE_H
#define __INET_NARROWBANDRADIOBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IModulation.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/Radio.h"

namespace inet {

namespace physicallayer {

class INET_API NarrowbandRadioBase : public Radio
{
  protected:
    void handleUpperCommand(cMessage *message) override;

  public:
    NarrowbandRadioBase();

    virtual void setModulation(const IModulation *newModulation);
    virtual void setCenterFrequency(Hz newCenterFrequency);
    virtual void setBandwidth(Hz newBandwidth);
};

} // namespace physicallayer

} // namespace inet

#endif

