#ifndef __DATABASEDESCRIPTIONHANDLER_HPP__
#define __DATABASEDESCRIPTIONHANDLER_HPP__

#include "IMessageHandler.h"

namespace OSPF {

class DatabaseDescriptionHandler : public IMessageHandler {
private:
    bool ProcessDDPacket (OSPFDatabaseDescriptionPacket* ddPacket, Interface* intf, Neighbor* neighbor, bool inExchangeStart);

public:
    DatabaseDescriptionHandler (Router* containingRouter);

    void ProcessPacket (OSPFPacket* packet, Interface* intf, Neighbor* neighbor);
};

} // namespace OSPF

#endif // __DATABASEDESCRIPTIONHANDLER_HPP__

