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

#ifndef __NEMOBINDINGUPDATELIST_H__
#define __NEMOBINDINGUPDATELIST_H__


#include "INETDefs.h"

#include "IPv6Address.h"

// Foreign declarations:
class InterfaceEntry;

#define PRE_BINDING_EXPIRY          2 // amount of seconds before BUL expiry that indicate that a binding will shortly expiry

// fayruz 27.01.2015

class INET_API NemoBindingUpdateList : public cSimpleModule
{
  public:
//###########################Declaration of BUL and BC added by Zarrar Yousaf @ CNI Uni Dortmund on 04.06.07######

    // 21.07.08 - CB
    enum MobilityState
    {
        NONE = 0,
        //RR,               Nemo BS doesn't require optimization
        //RR_COMPLETE,
        REGISTER,
        REGISTERED,
        DEREGISTER
    };

    class NemoBindingUpdateListEntry
    {
      public:
        IPv6Address destAddress; // is the address of the HA or the CN to which the MN has sent a BU message; reference to the key in the map
        IPv6Address homeAddress; // Home address of the MN for which that BU was sent.
        IPv6Address careOfAddress; // MN's CoA. With this entry the MN can determine whether it has sent a BU to the destination node with its CoA or not.
        uint bindingLifetime;   // the initial value of the lifetime field sent in the BU to which this entry corresponds
        simtime_t bindingExpiry; // the time at which the lifetime of the binding expires
        //uint remainingLifetime;    //initialised from bindingLifetime and is decremented until it reaches zero
        uint sequenceNumber; // the max value of the seq # sent in the previous BU.
        simtime_t sentTime; // the time at which that particular BU was sent. recorded from simTime(). Used to implement rate limiting restrcition for sending BU.
        //simtime_t nextBUTx; // the time to send the next BU. NOT EXACTLY CLEAR
        bool BAck; //not part of RFC. Indicates whether the correpsonding BU has received a valid BAck or not. True if Ack'ed. By Default it is FALSE.

        // fayruz 04.02.2015 . adding new field on data structure - RFC 3963 section 5.1
        IPv6Address prefixInfo; //implicit mode? should be configured manually?
        bool mobileRouter;

        // this state information is used for CN bindings
        MobilityState state;

        virtual ~NemoBindingUpdateListEntry() {};
    };

    friend std::ostream& operator<<(std::ostream& os, const NemoBindingUpdateListEntry& bul);
    typedef std::map<IPv6Address,NemoBindingUpdateListEntry> NemoBindingUpdateList6;
    NemoBindingUpdateList6 nemoBindingUpdateList;

  public:
    NemoBindingUpdateList();
    virtual ~NemoBindingUpdateList();

  protected:
    virtual void initialize();

    /**
     * Raises an error.
     */
    virtual void handleMessage(cMessage *);

  public:
    /**
     * Sets entry in the Binding Update List with provided values. If entry does not yet exist, a new one is created.
     */
    virtual void addOrUpdateBUL(const IPv6Address& dest, const IPv6Address& hoa,
           const IPv6Address& coa, const uint lifetime, const uint seq, const simtime_t buSentTime, const bool mR, const IPv6Address& prefix); //,const simtime_t& nextBUSentTime );

    /**
     * Creates a new entry in the BUL for the provided address.
     */
    virtual NemoBindingUpdateList::NemoBindingUpdateListEntry* createBULEntry(const IPv6Address& dest);

    /**
     * Initializes the values of a BUL entry to initial values.
     * Called by addOrUpdateBUL() if new entry is created.
     */
    virtual void initializeBUValues(NemoBindingUpdateListEntry& entry); // 28.08.07 - CB

    /**
     * Returns the BUL entry for a certain destination address.
     */
    virtual NemoBindingUpdateList::NemoBindingUpdateListEntry* lookup(const IPv6Address& dest); // checks whether BU exists for given address on provided interface

    /**
     * Similiar to lookup(), but with the difference that this method always returns
     * a valid BUL entry. If none existed prior to the call, a new entry is created.
     */
    virtual NemoBindingUpdateList::NemoBindingUpdateListEntry* fetch(const IPv6Address& dest); // checks whether BU exists for given address on provided interface

    //
    // The following methods are related to RR stuff.
    //

    /**
     * Returns the current mobility state for the CN identified by the provided IP address.
     */
    virtual MobilityState getMobilityState(const IPv6Address& dest) const;

    /**
     * Sets the mobility state to provided state for the CN identified by the provided IP address.
     */
    virtual void setMobilityState(const IPv6Address& dest, NemoBindingUpdateList::MobilityState state);

    /**
     * Checks whether there exists an entry in the BUL for the given
     * destination address.
     */
    virtual bool isInBindingUpdateList(const IPv6Address& dest) const; // 10.9.07 - CB

    /**
     * Returns the last used sequence number for the given dest. address.
     */
    virtual uint getSequenceNumber(const IPv6Address& dest); // 10.9.07 - CB

    /**
     * Returns the CoA that was registered for the provided dest. address.
     */
    virtual const IPv6Address& getCoA(const IPv6Address& dest); // 24.9.07 - CB

    /**
     * Checks whether there exists an entry in the BUL for the given
     * destination address and home address.
     */
    virtual bool isInBindingUpdateList(const IPv6Address& dest, const IPv6Address& HoA); // 20.9.07 - CB

    /**
     * Returns true if a binding has been acknowledged and it's lifetime
     * has not yet expired.
     */
    virtual bool isValidBinding(const IPv6Address& dest);

    /**
     * Returns true if a binding is about to expire.
     */
    virtual bool isBindingAboutToExpire(const IPv6Address& dest);

    /**
     * Returns true if a binding update has been sent to and acknowledged by
     * the provided destination address and the lifetime has not yet expired.
     */
    virtual bool sentBindingUpdate(const IPv6Address& dest);

    /**
     * Deletes an entry from the binding update list for the provided
     * destination address.
     */
    virtual void removeBinding(const IPv6Address& dest);

    /**
     * Sets the state of the binding cache entry to "not usable".
     * Resets the BAck flag to false, etc.
     */
    virtual void suspendBinding(const IPv6Address& dest);

    //get mobileRouter Flag from binding cache entry (???) fayruz 27.02.2015
    virtual bool getMobileRouterFlag(const IPv6Address& dest);

  protected:
    /**
     * Resets binding lifetime, tokens, etc. of the BUL entry.
     */
    virtual void resetBindingCacheEntry(NemoBindingUpdateListEntry& entry);
};

#endif

