/*
 * MrpInterfaceData.h
 *
 *  Created on: 22.06.2023
 *      Author: Daniel Zeitler
 */

#ifndef __INET_MRPINTERFACEDATA_H
#define __INET_MRPINTERFACEDATA_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

/**
 * Per-interface data needed by the MRP protocol.
 */
class INET_API MrpInterfaceData : public InterfaceProtocolData
{
  public:
    enum PortRole : uint16_t { PRIMARY, SECONDARY, INTERCONNECTION, NOTASSIGNED };

    enum PortState : uint16_t { BLOCKED, FORWARDING, DISABLED };

    class INET_API PortInfo {
      public:
        PortState state;
        PortRole role;
        simtime_t continuityCheckInterval; // The time interval between the generation of Link-Check-Frames
        simtime_t nextUpdate;
        bool continuityCheck;
        uint16_t cfmEndpointID = 1;
        std::string cfmName = "CFM CCM 000000000000";
        unsigned int lostPDU;
        unsigned int portNum; // The number of the switch port (i.e. EthernetSwitch ethg[] gate index).

      public:
        PortInfo();
    };

  protected:
    PortInfo portData;

  public:
    MrpInterfaceData();

    virtual std::string str() const override;
    virtual std::string detailedInfo() const;

    bool isForwarding() const { return portData.state == FORWARDING; }

    simtime_t getContinuityCheckInterval() const { return portData.continuityCheckInterval; }

    void setContinuityCheckInterval(simtime_t ContinuityCheckInterval) { portData.continuityCheckInterval = ContinuityCheckInterval; }

    PortRole getRole() const { return portData.role; }

    void setRole(PortRole role) { portData.role = role; }

    PortState getState() const { return portData.state; }

    void setState(PortState state) { portData.state = state; }

    bool getContinuityCheck() const { return portData.continuityCheck; }
    void setContinuityCheck(bool continuityCheck) { portData.continuityCheck = continuityCheck; }

    unsigned int getLostPDU() const { return portData.lostPDU; }

    void setLostPDU(unsigned int lostPDU) { portData.lostPDU = lostPDU; }

    const char *getRoleName() const { return getRoleName(getRole()); }
    const char *getStateName() const { return getStateName(getState()); }

    static const char *getRoleName(PortRole role);
    static const char *getStateName(PortState state);

    simtime_t getNextUpdate() const { return portData.nextUpdate; }

    void setNextUpdate(simtime_t nextUpdate) { portData.nextUpdate = nextUpdate; }

    uint16_t getCfmEndpointID() const { return portData.cfmEndpointID; }
    void setCfmEndpointID(uint16_t cfmEndpointID) { portData.cfmEndpointID = cfmEndpointID; }

    std::string getCfmName() { return portData.cfmName; }
    void setCfmName( std::string cfmName) { portData.cfmName = cfmName; }

    unsigned int getPortNum() const { return portData.portNum; }
    void setPortNum(unsigned int portNum) { portData.portNum = portNum; }
};

} // namespace inet
#endif /* INET_LINKLAYER_CONFIGURATOR_MRPINTERFACEDATA_H_ */
