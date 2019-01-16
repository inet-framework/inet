/**
 * Copyright (C) 2007
 * Faqir Zarrar Yousaf
 * Communication Networks Institute, University of Dortmund, Germany.
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
 */

#ifndef __INET_BINDINGUPDATELIST_H
#define __INET_BINDINGUPDATELIST_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"

namespace inet {

// Foreign declarations:
class InterfaceEntry;

// used for the RR tokens
#define UNDEFINED_TOKEN             0
#define UNDEFINED_COOKIE            0
#define UNDEFINED_BIND_AUTH_DATA    0
#define HO_COOKIE                   11
#define HO_TOKEN                    1101
#define CO_COOKIE                   21
#define CO_TOKEN                    2101

#define PRE_BINDING_EXPIRY          2 // amount of seconds before BUL expiry that indicate that a binding will shortly expiry

class INET_API BindingUpdateList : public cSimpleModule
{
  public:
//###########################Declaration of BUL and BC added by Zarrar Yousaf @ CNI Uni Dortmund on 04.06.07######

    // 21.07.08 - CB
    enum MobilityState {
        NONE = 0,
        RR,
        RR_COMPLETE,
        REGISTER,
        REGISTERED,
        DEREGISTER
    };

    class BindingUpdateListEntry
    {
      public:
        Ipv6Address destAddress;    // is the address of the HA or the CN to which the MN has sent a BU message; reference to the key in the map
        Ipv6Address homeAddress;    // Home address of the MN for which that BU was sent.
        Ipv6Address careOfAddress;    // MN's CoA. With this entry teh MN can determine whether it has sent a BU to the destination node with its CoA or not.
        uint bindingLifetime;    // the initial value of the lifetime field sent in the BU to which this entry corresponds
        simtime_t bindingExpiry;    // the time at which the lifetime of the binding expires
        //uint remainingLifetime;    //initialised from bindingLifetime and is decremented until it reaches zero
        uint sequenceNumber;    // the max value of the seq # sent in the previous BU.
        simtime_t sentTime;    // the time at which that particular BU was sent. recorded from simTime(). Used to implement rate limiting restrcition for sending BU.
        //simtime_t nextBUTx; // the time to send the next BU. NOT EXACTLY CLEAR
        bool BAck;    //not part of RFC. Indicates whether the correpsonding BU has received a valid BAck or not. True if Ack'ed. By Default it is FALSE.

        // this part is for return routability procedure // 27.08.07 - CB
        /* The time at which a Home Test Init or Care-of Test Init message
           was last sent to this destination, as needed to implement the rate
           limiting restriction for the return routability procedure. */
        simtime_t sentHoTI, sentCoTI;

        /* The state of any retransmissions needed for this return
           routability procedure.  This state includes the time remaining
           until the next retransmission attempt and the current state of the
           exponential back-off mechanism for retransmissions. */
        //simtime_t sendNext; // FIXME huh?

        /* Cookie values used in the Home Test Init and Care-of Test Init
           messages. */
        int cookieHoTI, cookieCoTI;

        /* Home and care-of keygen tokens received from the correspondent
           node.*/
        int tokenH, tokenC;

        /* Home and care-of nonce indices received from the correspondent
           node. */
        // not used
        /* The time at which each of the tokens and nonces were received from
           the correspondent node, as needed to implement reuse while moving. */
        // this information is stored in the retransmission timer

        // this state information is used for CN bindings
        MobilityState state;

        virtual ~BindingUpdateListEntry() {};
    };

    friend std::ostream& operator<<(std::ostream& os, const BindingUpdateListEntry& bul);
    typedef std::map<Ipv6Address, BindingUpdateListEntry> BindingUpdateList6;
    BindingUpdateList6 bindingUpdateList;

  public:
    BindingUpdateList();
    virtual ~BindingUpdateList();

  protected:
    virtual void initialize() override;

    /**
     * Raises an error.
     */
    virtual void handleMessage(cMessage *) override;

  public:
    /**
     * Sets entry in the Binding Update List with provided values. If entry does not yet exist, a new one is created.
     */
    virtual void addOrUpdateBUL(const Ipv6Address& dest, const Ipv6Address& hoa,
            const Ipv6Address& coa, const uint lifetime, const uint seq, const simtime_t buSentTime);    //,const simtime_t& nextBUSentTime );

    /**
     * Creates a new entry in the BUL for the provided address.
     */
    virtual BindingUpdateList::BindingUpdateListEntry *createBULEntry(const Ipv6Address& dest);

    /**
     * Initializes the values of a BUL entry to initial values.
     * Called by addOrUpdateBUL() if new entry is created.
     */
    virtual void initializeBUValues(BindingUpdateListEntry& entry);    // 28.08.07 - CB

    /**
     * Sets HoTI and/or CoTI values (transmission time, etc.) for the BUL entry.
     */
    virtual void addOrUpdateBUL(const Ipv6Address& dest, const Ipv6Address& hoa,
            simtime_t sentTime, int cookie, bool isHoTI);    // BU for HoTI/CoTI

    /**
     * Returns the BUL entry for a certain destination address.
     */
    virtual BindingUpdateList::BindingUpdateListEntry *lookup(const Ipv6Address& dest);    // checks whether BU exists for given address on provided interface

    /**
     * Similiar to lookup(), but with the difference that this method always returns
     * a valid BUL entry. If none existed prior to the call, a new entry is created.
     */
    virtual BindingUpdateList::BindingUpdateListEntry *fetch(const Ipv6Address& dest);    // checks whether BU exists for given address on provided interface

    //
    // The following methods are related to RR stuff.
    //

    /**
     * Returns the current mobility state for the CN identified by the provided IP address.
     */
    virtual MobilityState getMobilityState(const Ipv6Address& dest) const;

    /**
     * Sets the mobility state to provided state for the CN identified by the provided IP address.
     */
    virtual void setMobilityState(const Ipv6Address& dest, BindingUpdateList::MobilityState state);

    /**
     * Generates the Binding Authorization Data based on a certain destination address and CoA.
     */
    virtual int generateBAuthData(const Ipv6Address& dest, const Ipv6Address& CoA);

    /**
     * Generates the key Kbm from home and care-of keygen token.
     * For now, this return the sum of both tokens.
     */
    virtual int generateKey(int homeToken, int careOfToken, const Ipv6Address& CoA);

    /**
     * Generates a home token from the provided parameters.
     * Returns a static value for now.
     */
    virtual int generateHomeToken(const Ipv6Address& HoA, int nonce);

    /**
     * Generates a care-of token from the provided parameters.
     * Returns a static value for now.
     */
    virtual int generateCareOfToken(const Ipv6Address& CoA, int nonce);

    /**
     * Resets the token to UNDEFINED.
     */
    virtual void resetHomeToken(const Ipv6Address& dest, const Ipv6Address& hoa);

    /**
     * Resets the token to UNDEFINED.
     */
    virtual void resetCareOfToken(const Ipv6Address& dest, const Ipv6Address& hoa);

    /**
     * Returns true if a home keygen token is available.
     */
    virtual bool isHomeTokenAvailable(const Ipv6Address& dest, InterfaceEntry *ie);

    /**
     * Returns true if a care-of keygen token is available.
     */
    virtual bool isCareOfTokenAvailable(const Ipv6Address& dest, InterfaceEntry *ie);

    //
    // Additional methods
    //

    /**
     * Checks whether there exists an entry in the BUL for the given
     * destination address.
     */
    virtual bool isInBindingUpdateList(const Ipv6Address& dest) const;    // 10.9.07 - CB

    /**
     * Returns the last used sequence number for the given dest. address.
     */
    virtual uint getSequenceNumber(const Ipv6Address& dest);    // 10.9.07 - CB

    /**
     * Returns the CoA that was registered for the provided dest. address.
     */
    virtual const Ipv6Address& getCoA(const Ipv6Address& dest);    // 24.9.07 - CB

    /**
     * Checks whether there exists an entry in the BUL for the given
     * destination address and home address.
     */
    virtual bool isInBindingUpdateList(const Ipv6Address& dest, const Ipv6Address& HoA);    // 20.9.07 - CB

    /**
     * Returns true if a binding has been acknowledged and it's lifetime
     * has not yet expired.
     */
    virtual bool isValidBinding(const Ipv6Address& dest);

    /**
     * Returns true if a binding is about to expire.
     */
    virtual bool isBindingAboutToExpire(const Ipv6Address& dest);

    /**
     * Returns true if a binding update has been sent to and acknowledged by
     * the provided destination address and the lifetime has not yet expired.
     */
    virtual bool sentBindingUpdate(const Ipv6Address& dest);

    /**
     * Deletes an entry from the binding update list for the provided
     * destination address.
     */
    virtual void removeBinding(const Ipv6Address& dest);

    /**
     * Sets the state of the binding cache entry to "not usable".
     * Resets the BAck flag to false, etc.
     */
    virtual void suspendBinding(const Ipv6Address& dest);

    /**
     * These two methods indicate whether a CoTI or HoTI message
     * has been recently sent to the CN identified by parameter dest.
     */
    virtual bool recentlySentCOTI(const Ipv6Address& dest, InterfaceEntry *ie);
    virtual bool recentlySentHOTI(const Ipv6Address& dest, InterfaceEntry *ie);

  protected:
    /**
     * Resets binding lifetime, tokens, etc. of the BUL entry.
     */
    virtual void resetBindingCacheEntry(BindingUpdateListEntry& entry);
};

} // namespace inet

#endif // ifndef __INET_BINDINGUPDATELIST_H

