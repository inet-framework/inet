//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

