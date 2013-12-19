//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Authors: Veronika Rybova, Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Jiri Trhlik (mailto:JiriTM@gmail.com)
//          Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#ifndef __INET_PIMMULTICASTROUTE_H
#define __INET_PIMMULTICASTROUTE_H

#include "IPv4Route.h"

#include "PIMTimer_m.h"
#include "InterfaceEntry.h"
#include "PIMInterfaceTable.h" // for PIMMode

/**
 * PIM IPv4 multicast route in IRoutingTable.
 */
class INET_API PIMMulticastRoute : public IPv4MulticastRoute
{
    public:
        /** Route flags. Added to each route. */
        enum Flag
        {
            NO_FLAG = 0,
            D       = 0x01,              /**< Dense */
            S       = 0x02,              /**< Sparse */
            C       = 0x04,              /**< Connected */ // XXX Are there any connected downstream receivers?
            P       = 0x08,              /**< Pruned */
            A       = 0x10,              /**< Source is directly connected */
            F       = 0x20,              /**< Register flag*/
            T       = 0x40               /**< SPT bit*/
        };

        /** States of each outgoing interface. */
        enum InterfaceState
        {
            Forward,
            Pruned
        };

        /** Assert States of each outgoing interface. */
        enum AssertState
        {
            AS_NO_INFO = 0,
            AS_WINNER = 1,
            AS_LOSER = 2
        };

        /**
         * @brief Structure of incoming interface.
         * @details E.g.: GigabitEthernet1/4, RPF nbr 10.10.51.145
         */
        struct PIMInInterface : public InInterface
        {
            IPv4Address nextHop;            /**< RF neighbor */

            PIMInInterface(InterfaceEntry *ie, IPv4Address nextHop)
                : InInterface(ie), nextHop(nextHop) {}
            int getInterfaceId() const { return ie->getInterfaceId(); }
        };

        /**
         * @brief Structure of outgoing interface.
         * @details E.g.: Ethernet0, Forward/Sparse, 5:29:15/0:02:57
         */
        struct PIMOutInterface : public OutInterface
        {
            InterfaceState          forwarding;         /**< Forward or Pruned */
            PIMInterface::PIMMode   mode;               /**< Dense, Sparse, ... */
            AssertState             assert;             /**< Assert state. */

            PIMOutInterface(InterfaceEntry *ie, PIMInterface::PIMMode mode)
                : OutInterface(ie, false), forwarding(Forward), mode(mode), assert(AS_NO_INFO) {}
            PIMOutInterface(InterfaceEntry *ie, InterfaceState forwarding, PIMInterface::PIMMode mode, AssertState assert)
                : OutInterface(ie, false), forwarding(forwarding), mode(mode), assert(assert) {}

            int getInterfaceId() const { return ie->getInterfaceId(); }
            virtual bool isEnabled() { return forwarding != Pruned; } // XXX should be: ((has neighbor and not pruned) or has listener) and not assert looser
        };

    protected:
        int flags;

        //Originated from destination.Ensures loop freeness.
        unsigned int sequencenumber;
        //Time of routing table entry creation
        simtime_t installtime;

    public:
        PIMMulticastRoute(IPv4Address origin, IPv4Address group);
        virtual ~PIMMulticastRoute() {}
        virtual std::string info() const;

        bool isFlagSet(Flag flag) const { return (flags & flag) != 0; }     /**< Returns if flag is set to entry or not*/
        void setFlags(int flags)   { this->flags |= flags; }                /**< Add flag to ineterface */
        void clearFlag(Flag flag)  { flags &= (~flag); }                   /**< Remove flag from ineterface */
        static std::string flagsToString(int flags);

        // get incoming interface
        PIMInInterface *getPIMInInterface() const { return getInInterface() ? check_and_cast<PIMInInterface*>(getInInterface()) : NULL; }

        // get outgoing interface
        PIMOutInterface *getPIMOutInterface(unsigned int k) const { return getOutInterface(k) ? check_and_cast<PIMOutInterface*>(getOutInterface(k)) : NULL; }
        PIMOutInterface *findOutInterfaceByInterfaceId(int interfaceId);
        bool isOilistNull();                                                /**< Returns true if list of outgoing interfaces is empty, otherwise false*/

        simtime_t getInstallTime() const {return installtime;}
        void setInstallTime(simtime_t time) {installtime = time;}
        void setSequencenumber(int i){sequencenumber =i;}
        unsigned int getSequencenumber() const {return sequencenumber;}
};

#endif
