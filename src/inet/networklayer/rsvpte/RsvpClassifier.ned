//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.rsvpte;

import inet.common.SimpleModule;
import inet.networklayer.mpls.IIngressClassifier;

//
// Table-based ingress classifier.
//
// The XML config file has the following format. <fectable>
// is the root element. <fectable> may contain any number of
// <fecentry> elements. <fecentry> may contain the following
// child elements that effectively serve as attributes:
// <id>, <destination>, <source>, <label>, <tunnel_id>, <lspid>.
//
simple RsvpClassifier extends SimpleModule like IIngressClassifier
{
    parameters:
        @class(RsvpClassifier);
        string routingTableModule;
        string libTableModule;
        string rsvpModule;
        xml config = default(xml("<fectable/>"));
        @display("i=block/classifier");
}

