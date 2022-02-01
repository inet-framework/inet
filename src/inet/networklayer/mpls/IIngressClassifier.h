//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IINGRESSCLASSIFIER_H
#define __INET_IINGRESSCLASSIFIER_H

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/mpls/LibTable.h"

namespace inet {

/**
 * This is an abstract interface for packet classifiers in MPLS ingress routers.
 * The ~Mpls module holds a pointer to an ~IIngressClassifier object, and uses it to
 * classify Ipv4 datagrams and find the right label-switched path for them.
 *
 * A known sub-interface is ~IRsvpClassifier.
 *
 * Known concrete classifier classes are the ~Ldp module class and (via ~IRsvpClassifier)
 * RSVP_TE's RsvpClassifier module class.
 */
class INET_API IIngressClassifier
{
  public:
    virtual ~IIngressClassifier() {}

    /**
     * The packet argument is an input parameter, the rest (outLabel,
     * outInterface, color) are output parameters only.
     *
     * In subclasses, this function should be implemented to determine the forwarding
     * equivalence class for the Ipv4 datagram passed, and map it to an outLabel
     * and outInterface.
     *
     * The color parameter (which can be set to an arbitrary value) will
     * only be used for the NAM trace if one will be recorded.
     */
    virtual bool lookupLabel(Packet *packet, LabelOpVector& outLabel, std::string& outInterface, int& color) = 0;
};

} // namespace inet

#endif

