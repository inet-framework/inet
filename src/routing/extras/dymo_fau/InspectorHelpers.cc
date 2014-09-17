/*
 *  Copyright (C) 2005 Mohamed Louizi
 *  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "inet/common/INETDefs.h"

#include "inet/routing/extras/dymo_fau/InspectorHelpers.h"


using namespace std;

namespace inet {

namespace inetmanet {

ostream& operator<<(ostream& os, const std::vector<DYMO_AddressBlock>& abs)
{
    os << "{" << std::endl;
    for (std::vector<DYMO_AddressBlock>::const_iterator i = abs.begin(); i != abs.end(); i++)
    {
        const DYMO_AddressBlock& ab = *i;
        os << "  " << ab << std::endl;
    }
    os << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DYMO_AddressBlock& ab)
{
    os << "{";

    if (ab.hasAddress())
    {
        os << "Address=" << ab.getAddress();
        int id = ab.getAddress();
        cModule *m = simulation.getModule(id);
        if (m) os << " (" << m->getFullName() << ")";
    }

    os << ", ";

    if (ab.hasPrefix())
    {
        os << "Prefix=" << ab.getPrefix();
    }

    os << ", ";

    if (ab.hasDist())
    {
        os << "Dist=" << ab.getDist();
    }

    os << ", ";

    if (ab.hasSeqNum())
    {
        os << "SeqNum=" << ab.getSeqNum();
    }

    os << "}";
    return os;
}

} // namespace inetmanet

} // namespace inet

