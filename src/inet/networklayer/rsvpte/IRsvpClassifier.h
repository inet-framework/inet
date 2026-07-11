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

    // C7 (make-before-break, RFC 3209 Section 4.6.4): move every FEC binding currently
    // pointing at oldSender over to newSender with its already-installed inLabel. bind()
    // above cannot do this by itself: FEC entries are bound to a specific (session, sender)
    // pair (typically configured once, statically, via XML with the ORIGINAL Lsp_Id), so
    // once a make-before-break replacement gets a fresh Lsp_Id, an ordinary bind() call for
    // the new sender matches no existing binding at all. This rebind is the actual ingress
    // traffic cutover point from the old LSP to its replacement.
    virtual void rebind(const SessionObj& session, const SenderTemplateObj& oldSender, const SenderTemplateObj& newSender, int inLabel) = 0;
};

} // namespace inet

#endif

