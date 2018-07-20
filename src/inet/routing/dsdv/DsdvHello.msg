//
// Copyright (C) 2008
// DSDV simple example for INET (add-on)
// Version 1.0
// Diogo Antio & Pedro Menezes
// Instituto Superior Tecnico
// Lisboa - Portugal
// This version and newer version can be found at http://dsdv.has.it
// This code was written while assisting the course "Redes moveis e sem fios" http://comp.ist.utl.pt/ec-cm
// Autorization to use and modify this code not needed :P
// The authors hope it will be useful to help understand how
// INET and OMNET++ works(more specifically INET 20061020 and omnet++ 3.3).
// Also we hope it will help in the developing of routing protocols using INET.
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.networklayer.contract.ipv4.Ipv4Address;

namespace inet;

class DsdvHello extends FieldsChunk
{
    Ipv4Address srcAddress; //terminal que gerou o pacote
    unsigned int sequencenumber;//sequence number created by the source node(node's ID = srcIPAddress) of the hello msg and associated with that node
    Ipv4Address nextAddress; //terminal que re-/transmite o pacote
    int hopdistance; //distancia em hops/custo
}

