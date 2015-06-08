// Copyright (c) Fayruz Rahma 2015

#ifndef _PREFIXTABLE_H_
#define _PREFIXTABLE_H_

#include "INETDefs.h"

#include "IPv6Address.h"

#include "ModuleAccess.h"

class INET_API PrefixTable : public cSimpleModule
{
private:
    struct PrefixTableEntry
    {
        /*
         * RFC 3969 sec 6.1.2
         * The Home Agent SHOULD be able to prevent a Mobile Router from
         * claiming Mobile Network Prefixes belonging to another Mobile Router.
         */

        // The Home Address of the Mobile Router. This field is used as the
        // key for searching the pre-configured Prefix Table.
        // IPv6Address HomeAddress; deliberately commented. this is stored as the key to the entry

        //The Mobile Network Prefix of the Mobile Router associated with the
        // Home Address.
        IPv6Address mobileNetworkPrefix;
    };

    typedef std::map<IPv6Address,PrefixTableEntry> PrefixTable6; // The IPv6 Address KEY of this map is the Home Address of the MR
    PrefixTable6 prefixTable;

    friend std::ostream& operator<<(std::ostream& os, const PrefixTableEntry& pte);

public:
    PrefixTable();
    virtual ~PrefixTable();

protected:
    virtual void initialize();

    /*
     * Raises an error. this module does not handle message
     */
    virtual void handleMessage(cMessage *);

public:
    /*
     * Sets Prefix Table Entry (PTE) with provided values.
     * If PTE does not yet exist, a new one will be created.
     */
    void addOrUpdatePT(const IPv6Address& HoA, const IPv6Address& prefix);

    /*
     * Check whether there is an entry in the PT for the given HoA
     */
    bool isInPrefixTable(const IPv6Address& HoA) const;

    /*
     * Delete the entry from prefix table with the provided HoA
     */
    void deleteEntry(IPv6Address& HoA);

    /*
     * Returns the prefix for the given HoA
     */
    IPv6Address getPrefix(const IPv6Address& HoA) const;

    /*
     * Returns the last prefix on the table
     */
    IPv6Address getLastPrefix() const;
};

/**
 * Gives access to the PrefixTable instance within the Home Agent / Mobile Router.
 */

class INET_API PrefixTableAccess : public ModuleAccess<PrefixTable>
{
  public:
    PrefixTableAccess() : ModuleAccess<PrefixTable>("prefixTable") {}
};

#endif
