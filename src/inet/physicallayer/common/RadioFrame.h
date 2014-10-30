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

#ifndef __INET_RADIOFRAME_H
#define __INET_RADIOFRAME_H

#include "inet/physicallayer/contract/IRadioFrame.h"

namespace inet {

namespace physicallayer {

class INET_API RadioFrame : public cPacket, public IRadioFrame
{
  protected:
    const ITransmission *transmission;

  public:
    RadioFrame(const ITransmission *transmission);
    RadioFrame(const RadioFrame& other);

    virtual RadioFrame *dup() const { return new RadioFrame(*this); }

    virtual void printToStream(std::ostream& stream) const;

    virtual const ITransmission *getTransmission() const { return transmission; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_RADIOFRAME_H

