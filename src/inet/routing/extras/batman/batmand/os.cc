/*
 * Copyright (C) 2006-2009 B.A.T.M.A.N. contributors:
 *
 * Simon Wunderlich, Axel Neumann, Marek Lindner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */


#include "inet/routing/extras/batman/batmand/batman.h"
#include "inet/routing/extras/batman/BatmanMain.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/routing/extras/base/ManetAddress.h"

namespace inet {

namespace inetmanet {

int8_t Batman::send_udp_packet(cPacket *packet_buff, int32_t packet_buff_len, const L3Address & destAdd, int32_t send_sock, BatmanIf *batman_if)
{
    if ((batman_if != nullptr) && (!batman_if->if_active))
    {
        delete packet_buff;
        return 0;
    }
    if (batman_if)
        sendToIp(packet_buff, BATMAN_PORT, destAdd, BATMAN_PORT, 1, par("broadcastDelay").doubleValue(), L3Address(batman_if->dev->ipv4Data()->getIPAddress()));
    else
        sendToIp(packet_buff, BATMAN_PORT, destAdd, BATMAN_PORT, 1, par("broadcastDelay").doubleValue(), L3Address());
    return 0;
}

//
//
// modification routing tables methods
//
//
void Batman::add_del_route(const L3Address &dest, uint8_t netmask, const L3Address &router, int32_t ifi, InterfaceEntry* dev, uint8_t rt_table, int8_t route_type, int8_t route_action)
{
    if (route_type != ROUTE_TYPE_UNICAST)
        return;
    int index = -1;
    for (int i=0; i<getNumInterfaces(); i++)
    {
        if (dev == this->getInterfaceEntry(i))
        {
            index = i;
            break;
        }
    }
    if (index < 0)
        return;

    L3Address nmask(IPv4Address::makeNetmask(netmask));
    if (route_action==ROUTE_DEL)
    {
       setRoute(dest, L3Address(), index, 0, nmask);
       return;
    }

    // if (route_action==ROUTE_ADD)
    setRoute(dest, router, index, -1, nmask);
}

int Batman::add_del_interface_rules(int8_t rule_action)
{
    if (isInMacLayer())
        return 1;
    int if_count = 1;
    for (int i=0; i<getNumInterfaces(); i++)
    {
        InterfaceEntry *ifr = getInterfaceEntry(i);

        if (ifr->ipv4Data()==nullptr) // no ipv4
            continue;
        if (ifr->isLoopback())      //FIXME What would be the correct conditions here? The isLoopback() used below.
            continue;
        if (!ifr->isUp())
            continue;

        L3Address addr(ifr->ipv4Data()->getIPAddress());
        L3Address netmask(ifr->ipv4Data()->getNetmask());
        uint8_t mask = ifr->ipv4Data()->getNetmask().getNetmaskLength();

        ManetNetworkAddress netaddr(addr, mask);// addr&netmask;
        BatmanIf *batman_if;

        L3Address ZERO;
        add_del_route(netaddr.getAddress(), mask, ZERO, 0, ifr, BATMAN_RT_TABLE_TUNNEL, ROUTE_TYPE_THROW, rule_action);

        if ((batman_if = is_batman_if(ifr))==nullptr)
            continue;

        add_del_rule(netaddr.getAddress(), mask, BATMAN_RT_TABLE_TUNNEL, (rule_action == RULE_DEL ? 0 : BATMAN_RT_PRIO_TUNNEL + if_count), nullptr, RULE_TYPE_SRC, rule_action);

        if (ifr->isLoopback())
            add_del_rule(L3Address(), 0, BATMAN_RT_TABLE_TUNNEL, BATMAN_RT_PRIO_TUNNEL, ifr, RULE_TYPE_IIF, rule_action);
        if_count++;
    }

    return 1;
}

void Batman::add_del_rule(const L3Address& network, uint8_t netmask, int8_t rt_table, uint32_t prio, InterfaceEntry *iif, int8_t rule_type, int8_t rule_action)
{
    return;
}

void Batman::deactivate_interface(BatmanIf *iface)
{
    iface->if_active = false;
    //iface->dev->setDown(true);
}

void Batman::activate_interface(BatmanIf *iface)
{
    iface->if_active = true;

    //iface->dev->setDown(true);
}

void Batman::check_active_inactive_interfaces(void)
{
    /* all available interfaces are deactive */
    for (auto & elem : if_list){
        BatmanIf* batman_if = elem;
        if ((batman_if->if_active) && (!batman_if->dev->isUp()))
        {
            deactivate_interface(batman_if);
            active_ifs--;
        }
        else if ((!batman_if->if_active) && (batman_if->dev->isUp()))
        {
            activate_interface(batman_if);
            active_ifs++;
        }
    }
}


void Batman::check_inactive_interfaces(void)
{
    /* all available interfaces are active */
    if (found_ifs == active_ifs)
        return;

    for (auto & elem : if_list){
        BatmanIf* batman_if = elem;

        if ((!batman_if->if_active) && (batman_if->dev->isUp()))
        {
            activate_interface(batman_if);
            active_ifs++;
        }
    }
}

void Batman::check_active_interfaces(void)
{
    /* all available interfaces are deactive */
    if (active_ifs == 0)
        return;
    for (auto & elem : if_list)
    {
        BatmanIf* batman_if = elem;
        if ((batman_if->if_active) && (!batman_if->dev->isUp()))
        {
            deactivate_interface(batman_if);
            active_ifs--;
        }
    }
}

simtime_t Batman::getTime()
{
    return simTime() + par("desynchronized");
}

} // namespace inetmanet

} // namespace inet

