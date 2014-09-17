//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IVISITOR_H_
#define __INET_IVISITOR_H_

#include "inet/common/INETDefs.h"

namespace inet {

// This is the interface for data structure visitors
class IVisitor
{
    public:
        virtual void visit(const cObject *) const = 0;
        virtual ~IVisitor() {}
};

} /* namespace inet */

#endif /* __INET_IVISITOR_H_ */
