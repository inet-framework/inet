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

#ifndef __INET_PIMROUTE_H
#define __INET_PIMROUTE_H

#include "IPv4Route.h"

#include "PIMTimer_m.h"
#include "InterfaceEntry.h"

/**
 * PIM IPv4 multicast route in IRoutingTable.
 */
class INET_API PIMMulticastRoute : public IPv4MulticastRoute
{
    public:
        /** Route flags. Added to each route. */
        enum flag
        {
            NO_FLAG,
            D,              /**< Dense */
            S,              /**< Sparse */
            C,              /**< Connected */
            P,              /**< Pruned */
            A,              /**< Source is directly connected */
            F,              /**< Register flag*/
            T               /**< SPT bit*/
        };

        /** States of each outgoing interface. E.g.: Forward/Dense. */
        enum intState
        {
            Densemode = 1,
            Sparsemode = 2,
            Forward,
            Pruned
        };

        /** Assert States of each outgoing interface. */
        enum AssertState
        {
            NoInfo = 0,
            Winner = 1,
            Loser = 2
        };

        /**  Register machine States. */
        enum RegisterState
        {
          NoInfoRS = 0,
          Join = 1,
          Prune = 2,
          JoinPending = 3
        };

        /**
         * @brief Structure of incoming interface.
         * @details E.g.: GigabitEthernet1/4, RPF nbr 10.10.51.145
         */
        struct AnsaInInterface : public InInterface
        {
            int         intId;              /**< Interface ID */
            IPv4Address nextHop;            /**< RF neighbor */
            AnsaInInterface(InterfaceEntry *intPtr, int intId, IPv4Address nextHop)
                : InInterface(intPtr), intId(intId), nextHop(nextHop) {}
        };

        /**
         * @brief Structure of outgoing interface.
         * @details E.g.: Ethernet0, Forward/Sparse, 5:29:15/0:02:57
         */
        struct AnsaOutInterface : public OutInterface
        {
            int                     intId;              /**< Interface ID */
            intState                forwarding;         /**< Forward or Pruned */
            intState                mode;               /**< Dense, Sparse, ... */
            PIMpt                   *pruneTimer;        /**< Pointer to PIM Prune Timer*/
            PIMet                   *expiryTimer;       /**< Pointer to PIM Expiry Timer*/
            AssertState             assert;             /**< Assert state. */
            RegisterState           regState;           /**< Register state. */
            bool                    shRegTun;           /**< Show interface which is also register tunnel interface*/
            AnsaOutInterface(InterfaceEntry *intPtr)
                : OutInterface(intPtr, false) {}

            virtual bool isEnabled() { return forwarding != Pruned; }
        };

    private:
        IPv4Address                 RP;                     /**< Randevous point */
        std::vector<flag>           flags;                  /**< Route flags */
        // timers
        PIMgrt                      *grt;                   /**< Pointer to Graft Retry Timer*/
        PIMsat                      *sat;                   /**< Pointer to Source Active Timer*/
        PIMsrt                      *srt;                   /**< Pointer to State Refresh Timer*/
        PIMkat                      *kat;                   /**< Pointer to Keep Alive timer for PIM-SM*/
        PIMrst                      *rst;                   /**< Pointer to Register-stop timer for PIM-SM*/
        PIMet                       *et;                    /**< Pointer to Expiry timer for PIM-SM*/
        PIMjt                       *jt;                    /**< Pointer to Join timer*/
        PIMppt                      *ppt;                   /**< Pointer to Prune Pending Timer*/

        //Originated from destination.Ensures loop freeness.
        unsigned int sequencenumber;
        //Time of routing table entry creation
        simtime_t installtime;

    public:
        PIMMulticastRoute();                                                 /**< Set all pointers to null */
        virtual ~PIMMulticastRoute() {}
        virtual std::string info() const;

    public:
        void setRP(IPv4Address RP)  {this->RP = RP;}                        /**< Set RP IP address */
        void setGrt (PIMgrt *grt)   {this->grt = grt;}                      /**< Set pointer to PimGraftTimer */
        void setSat (PIMsat *sat)   {this->sat = sat;}                      /**< Set pointer to PimSourceActiveTimer */
        void setSrt (PIMsrt *srt)   {this->srt = srt;}                      /**< Set pointer to PimStateRefreshTimer */
        void setKat (PIMkat *kat)   {this->kat = kat;}                      /**< Set pointer to KeepAliveTimer */
        void setRst (PIMrst *rst)   {this->rst = rst;}                      /**< Set pointer to RegisterStopTimer */
        void setEt  (PIMet *et)     {this->et = et;}                        /**< Set pointer to ExpiryTimer */
        void setJt  (PIMjt *jt)     {this->jt = jt;}                        /**< Set pointer to JoinTimer */
        void setPpt  (PIMppt *ppt)  {this->ppt = ppt;}                      /**< Set pointer to PrunePendingTimer */

        void setFlags(std::vector<flag> flags)  {this->flags = flags;}      /**< Set vector of flags (flag) */
        bool isFlagSet(flag fl);                                            /**< Returns if flag is set to entry or not*/
        void addFlag(flag fl);                                              /**< Add flag to ineterface */
        void addFlags(flag fl1, flag fl2, flag fl3,flag fl4);               /**< Add flags to ineterface */
        void removeFlag(flag fl);                                           /**< Remove flag from ineterface */

        void setInInt(InterfaceEntry *interfacePtr, int intId, IPv4Address nextHop)     /**< Set information about incoming interface*/
        {
            setInInterface(new AnsaInInterface(interfacePtr, intId, nextHop));
        }
        void addOutIntFull(InterfaceEntry *intPtr, int intId, intState forwading, intState mode,
                            PIMpt *pruneTimer, PIMet *expiryTimer, AssertState assert, RegisterState regState, bool show);

        void setAddresses(IPv4Address multOrigin, IPv4Address multGroup, IPv4Address RP);

        void setRegStatus(int intId, RegisterState regState);                                                           /**< set register status to given interface*/
        RegisterState getRegStatus(int intId);

        bool isRpf(int intId){ return getInInterface() && intId == getAnsaInInterface()->intId; }                    /**< Returns if given interface is RPF or not*/
        bool isOilistNull();                                                                                            /**< Returns true if list of outgoing interfaces is empty, otherwise false*/

        IPv4Address   getRP() const {return RP;}                            /**< Get RP IP address */
        PIMgrt*     getGrt() const {return grt;}                            /**< Get pointer to PimGraftTimer */
        PIMsat*     getSat() const {return sat;}                            /**< Get pointer to PimSourceActiveTimer */
        PIMsrt*     getSrt() const {return srt;}                            /**< Get pointer to PimStateRefreshTimer */
        PIMkat*     getKat() const {return kat;}                            /**< Get pointer to KeepAliveTimer */
        PIMrst*     getRst() const {return rst;}                            /**< Get pointer to RegisterStopTimer */
        PIMet*      getEt()  const {return et;}                             /**< Get pointer to ExpiryTimer */
        PIMjt*      getJt()  const {return jt;}                             /**< Get pointer to JoinTimer */
        PIMppt*     getPpt()  const {return ppt;}                           /**< Get pointer to PrunePendingTimer */
        std::vector<flag> getFlags() const {return flags;}                  /**< Get list of route flags */

        // get incoming interface
        AnsaInInterface *getAnsaInInterface() const { AnsaInInterface *p = dynamic_cast<AnsaInInterface*>(getInInterface()); ASSERT(p); return p; }
        InterfaceEntry* getInIntPtr() const {return getInInterface() ? getAnsaInInterface()->getInterface() : NULL;}          /**< Get pointer to incoming interface*/
        int             getInIntId() const {return getInInterface() ? getAnsaInInterface()->intId : 0;}            /**< Get ID of incoming interface*/
        IPv4Address       getInIntNextHop() const {return getInInterface() ? getAnsaInInterface()->nextHop : IPv4Address();}     /**< Get IP address of next hop for incoming interface*/

        // get outgoing interface
        AnsaOutInterface *getAnsaOutInterface(unsigned int k) const { AnsaOutInterface *outIf = dynamic_cast<AnsaOutInterface*>(getOutInterface(k)); ASSERT(outIf); return outIf; }
        int             getOutIdByIntId(int intId);                         /**< Get sequence number of outgoing interface with given interface ID*/
        bool            outIntExist(int intId);                             /**< Return true if interface intId exist, otherwise return false*/

        simtime_t getInstallTime() const {return installtime;}
        void setInstallTime(simtime_t time) {installtime = time;}
        void setSequencenumber(int i){sequencenumber =i;}
        unsigned int getSequencenumber() const {return sequencenumber;}
};

#endif
