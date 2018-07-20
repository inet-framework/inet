//
// Copyright (C) 2010 Helene Lageber
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

#ifndef __INET_BGPUPDATE_H
#define __INET_BGPUPDATE_H

#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"

namespace inet {

namespace bgp {

class INET_API BgpUpdateMessage : public BgpUpdateMessage_Base
{
  protected:
    unsigned short computePathAttributesBytes(const BgpUpdatePathAttributeList& pathAttrs);

  public:
    BgpUpdateMessage() : BgpUpdateMessage_Base() {}
    virtual BgpUpdateMessage *dup() const override { return new BgpUpdateMessage(*this); }
    void setWithdrawnRoutesArraySize(size_t size) override;
    void setPathAttributeList(const BgpUpdatePathAttributeList& pathAttributeList_var);
    void setNLRI(const BgpUpdateNlri& NLRI_var) override;
};

} // namespace bgp

} // namespace inet

#endif // ifndef __INET_BGPUPDATE_H

