//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_IRSVPCLASSIFIER_H
#define __INET_IRSVPCLASSIFIER_H

#include "inet/common/INETDefs.h"
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

#endif // ifndef __INET_IRSVPCLASSIFIER_H

