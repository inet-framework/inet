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

#ifndef __INET_RSVPCLASSIFIER_H
#define __INET_RSVPCLASSIFIER_H

#include <omnetpp.h>

#include "IntServ.h"
#include "IClassifier.h"

/**
 * TODO
 */
class INET_API IRSVPClassifier : public IClassifier
{
  public:
    virtual ~IRSVPClassifier() {}

    virtual void bind(const SessionObj_t& session, const SenderTemplateObj_t& sender, int inLabel) = 0;
};

#endif
