cplusplus {{
/*
 * Copyright (C) 2008 
 * DSDV simple example for INET (add-on)
 * Version 1.0
 * Diogo Ant�o & Pedro Menezes
 * Instituto Superior T�cnico
 * Lisboa - Portugal
 * This version and newer version can be found at http://dsdv.has.it
 * This code was written while assisting the course "Redes m�veis e sem fios" http://comp.ist.utl.pt/ec-cm
 * Autorization to use and modify this code not needed :P
 * The authors hope it will be useful to help understand how
 * INET and OMNET++ works(more specifically INET 20061020 and omnet++ 3.3).
 * Also we hope it will help in the developing of routing protocols using INET.
*/
#include "IPv4Address.h"

}}


class noncobject IPv4Address;

packet DSDV_HelloMessage
{
    IPv4Address srcIPAddress; //terminal que gerou o pacote
    unsigned int sequencenumber;//sequence number created by the source node(node's ID = srcIPAddress) of the hello msg and associated with that node
    IPv4Address nextIPAddress; //terminal que re-/transmite o pacote
    int hopdistance; //distancia em hops/custo
}

