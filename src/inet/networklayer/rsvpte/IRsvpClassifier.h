//
// Copyright (C) 2005 Vojtech Janota
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_IRSVPCLASSIFIER_H
#define __INET_IRSVPCLASSIFIER_H

#include "inet/networklayer/mpls/IIngressClassifier.h"
#include "inet/networklayer/rsvpte/IntServ_m.h"

namespace inet {

/**
 * TODO
 */
class INET_API IRsvpClassifier : public IIngressClassifier
{
  public:
    virtual ~IRsvpClassifier() {}

    virtual void bind(const SessionObj& session, const SenderTemplateObj& sender, int inLabel) = 0;
};

} // namespace inet

#endif

