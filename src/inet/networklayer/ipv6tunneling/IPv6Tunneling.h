/**
 * Copyright (C) 2007
 * Christian Bauer
 * Institute of Communications and Navigation, German Aerospace Center (DLR)

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * @file IPv6Tunneling.h
 * @brief Manage IP tunnels (RFC 2473) and Type 2 Routing Header/Home Address Option based routing as specified in MIPv6 (RFC 3775)

 * @author Christian
 * @date 12.06.07
 */

#ifndef __INET_IPV6TUNNELING_H
#define __INET_IPV6TUNNELING_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

// Foreign declarations:
class IInterfaceTable;
class IPv6Datagram;
class IPv6RoutingTable;

/**
 * Management of IP tunnels.
 */
class INET_API IPv6Tunneling : public cSimpleModule, public ILifecycle
{
  public:
    enum TunnelType {
        INVALID = 0,
        SPLIT,
        NON_SPLIT,
        NORMAL,    // either split or non-split
        T2RH,
        HA_OPT,
        MOBILITY    // either T2RH or HA_OPT
    };

  protected:
    IInterfaceTable *ift = nullptr;
    IPv6RoutingTable *rt = nullptr;

    struct Tunnel
    {
        Tunnel(const IPv6Address& entry = IPv6Address::UNSPECIFIED_ADDRESS,
                const IPv6Address& exit = IPv6Address::UNSPECIFIED_ADDRESS,
                const IPv6Address& destTrigger = IPv6Address::UNSPECIFIED_ADDRESS);
        //~Tunnel();

        bool operator==(const Tunnel& rhs)
        {
            return entry == rhs.entry && exit == rhs.exit && destTrigger == rhs.destTrigger;
        }

        // entry point of tunnel
        IPv6Address entry;

        // exit point of tunnel
        IPv6Address exit;

        // hoplimit (0 for default)
        int hopLimit = 0;

        // traffic class (0 for default)
        int trafficClass = 0;

        // flowLabel (0 for default)
        int flowLabel = 0;

        // the Path MTU of the tunnel (not used)
        int tunnelMTU = 0;

        /**
         * Specifies the type of the tunnel
         * * split tunnel
         * * non-split tunnel
         * * type 2 routing header pseudo tunnel for communication with MNs (RFC 3775)
         * * home address option header pseudo tunnel for communication with CNs (RFC 3775)
         */
        TunnelType tunnelType = INVALID;

        // if this address is set, the tunnel is actually a split tunnel, where only
        // packets with a certain destination get forwarded
        // if it's value is the unspecified address, this is a normal tunnel over which
        // (nearly) everything will get routed
        IPv6Address destTrigger;

        bool isTriggerPrefix = false;
    };

    typedef std::map<int, struct Tunnel> Tunnels;
    typedef Tunnels::const_iterator TI;

    struct equalTunnel : public std::binary_function<Tunnels::value_type, Tunnels::value_type, bool>
    {
        bool operator()(const Tunnels::value_type& lhs, const Tunnels::value_type& rhs) const
        {
            return (lhs.second.entry == rhs.second.entry) &&
                   (lhs.second.exit == rhs.second.exit) &&
                   (lhs.second.destTrigger == rhs.second.destTrigger);
        }
    };

    // Tunnels are stored here indexed by vIfIndex
    Tunnels tunnels;

    // The lowest vIfIndex assigned so far. Virtual ifIndexes are assigned downwards.
    int vIfIndexTop = 0;

    // number of tunnels which are not split tunnels
    int noOfNonSplitTunnels = 0;

  public:
    IPv6Tunneling();
    //virtual ~IPv6Tunneling();

    /**
     * Initialize tunnel manager.
     */
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /**
     * Receive messages from IPv6 module and encapsulate/decapsulate them.
     */
    virtual void handleMessage(cMessage *msg) override;

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    /**
     * Creates a tunnel with given entry and exit point, which will be used for datagrams
     * destined for destTrigger. Type of tunnel (normal tunnel, mobility related pseudo-tunnel)
     * is determined by first parameter.
     * Returns virtual interface index.
     */
    int createTunnel(TunnelType tunnelType, const IPv6Address& src, const IPv6Address& dest,
            const IPv6Address& destTrigger = IPv6Address::UNSPECIFIED_ADDRESS);

    /**
     * Creates a pseudo-tunnel for use with either Type 2 Routing Header or Home Address Option
     * with given entry and exit point, which will be used for datagrams destined for destTrigger.
     * Returns virtual interface index.
     */
    //int createPseudoTunnel(const IPv6Address& src, const IPv6Address& dest,
    //        const IPv6Address& destTrigger, int tunnelType);

    /**
     * Remove tunnel and the associated entries from destination cache
     */
    bool destroyTunnel(const IPv6Address& src, const IPv6Address& dest, const IPv6Address& destTrigger);

    /**
     * Remove all tunnels with provided entry point.
     */
    void destroyTunnels(const IPv6Address& entry);

    /**
     * Remove the tunnel with the provided entry and exit point.
     */
    void destroyTunnel(const IPv6Address& entry, const IPv6Address& exit);

    /**
     * Remove the tunnel with the provided exit point and trigger.
     */
    void destroyTunnelForExitAndTrigger(const IPv6Address& exit, const IPv6Address& trigger);

    /**
     * Remove the tunnel with the provided entry point and trigger.
     */
    void destroyTunnelForEntryAndTrigger(const IPv6Address& entry, const IPv6Address& trigger);

    /**
     * Remove the tunnel with the provided destination trigger.
     */
    void destroyTunnelFromTrigger(const IPv6Address& trigger);

    /**
     * Returns the virtual interface identifier for the tunnel which has the provided
     * destination as destination trigger.
     *
     * This is done by first looking at the split tunnels; if no split
     * tunnels are found, a prefix matching on the non-split tunnels is then performed.
     * In case both searches do not return a search hit, a value of -1 is returned.
     */
    int getVIfIndexForDest(const IPv6Address& destAddress);

    /**
     * This method is equivalent for getVIfIndexForDest() except that it
     * only searches for either "normal" or mobility tunnels
     */
    virtual int getVIfIndexForDest(const IPv6Address& destAddress, TunnelType tunnelType);

    /**
     * This method is equivalent for getVIfIndexForDest() except that it
     * only searches for pseudo tunnels (T2RH, etc.).
     */
    //int getVIfIndexForDestForPseudoTunnel(const IPv6Address& destAddress);

    /**
     * Check if there exists a tunnel with exit equal to the provided address.
     */
    bool isTunnelExit(const IPv6Address& exit);    // 11.9.07 - CB

    /**
     * Returns the type of the tunnels: non-split, split, T2RH, ...
     */
    //TunnelType getTunnelType(const int vIfIndex);

  protected:
    /**
     * Returns the vIfIndex of tunnel if found, 0 otherwise.
     */
    int findTunnel(const IPv6Address& src, const IPv6Address& dest, const IPv6Address& destTrigger) const;

    /**
     * Encapsulate a datagram with tunnel headers.
     *
     * Attaches a Type 2 Routing Header in the control info if the datagram is routed over an
     * appropriate RH2 pseudo tunnel.
     */
    void encapsulateDatagram(IPv6Datagram *dgram);

    /**
     * Strip tunnel headers from datagram
     */
    void decapsulateDatagram(IPv6Datagram *dgram);

    friend std::ostream& operator<<(std::ostream& os, const IPv6Tunneling::Tunnel& tun);

  private:
    /**
     * Search through all tunnels and locate one entry which is anything but a non-split tunnel
     * and has a destination trigger for the provided address.
     */
    int lookupTunnels(const IPv6Address& dest);

    /**
     * Search through all tunnels and locate one entry which is a non-split tunnel
     * (later on this could be exteded to searching for a tunnel that has a prefix
     *  matching the provided address).
     */
    int doPrefixMatch(const IPv6Address& dest);

    /**
     * Reset the vIfIndex to its starting value if no tunnels exist anymore.
     */
    inline void resetVIfIndex() { if (tunnels.size() == 0) vIfIndexTop = INT_MAX; };
};

} // namespace inet

#endif // ifndef __INET_IPV6TUNNELING_H

