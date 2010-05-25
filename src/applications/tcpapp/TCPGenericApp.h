//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_TCPGENERICAPP_H
#define __INET_TCPGENERICAPP_H

#include <omnetpp.h>

#include "INETDefs.h"
#include "TCPCommand_m.h"


/**
 * Generic TCP application.
 */
class INET_API TCPGenericApp : public cSimpleModule
{
  public:
    TCPGenericApp(unsigned stacksize = 0);
    virtual ~TCPGenericApp() {};
  protected:
    TCPDataTransferMode tcpDataTransferMode;

  protected:
    virtual void initialize();

    /**
     * Read "dataTransferMode" parameter from ini/ned, and set tcpDataTransferMode member value
     *
     * Generate runtime error when parameter is missing or value is invalid.
     */
    void readTransferModePar();

    void setTransferMode(TCPDataTransferMode newmodeP) { tcpDataTransferMode = newmodeP; };

    TCPDataTransferMode getTransferMode() { return tcpDataTransferMode; };
};

#endif
