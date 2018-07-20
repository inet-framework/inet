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

#ifndef __INET_IDEALRADIO_H
#define __INET_IDEALRADIO_H

#include "inet/physicallayer/common/packetlevel/Radio.h"

namespace inet {

namespace physicallayer {

class INET_API UnitDiskRadio : public Radio
{
  protected:
    virtual void encapsulate(Packet *packet) const override;
    virtual void decapsulate(Packet *packet) const override;

  public:
    UnitDiskRadio();
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IDEALRADIO_H

