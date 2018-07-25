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

package inet.networklayer.rsvpte;

import inet.networklayer.mpls.IIngressClassifier;

//
// Table-base ingress classifier.
//
// The XML config file has the following format. <fectable>
// is the root element. <fectable> may contain any number of
// <fecentry> elements. <fecentry> may contain the following
// child elements that effectively serve as attributes:
// <id>, <destination>, <source>, <label>, <tunnel_id>, <lspid>.
//
simple RsvpClassifier like IIngressClassifier
{
    parameters:
        string routingTableModule;
        string libTableModule;
        string rsvpModule;
        xml config = default(xml("<fectable/>"));
        @display("i=block/classifier");
}

