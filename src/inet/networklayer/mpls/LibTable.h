//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_LIBTABLE_H
#define __INET_LIBTABLE_H

#include "inet/common/SimpleModule.h"
#include <string>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

namespace inet {

// RFC 3032 reserved label values (bottom of the 20-bit label space)
constexpr int IPV4_EXPLICIT_NULL_LABEL = 0;
constexpr int ROUTER_ALERT_LABEL = 1;
constexpr int IMPLICIT_NULL_LABEL = 3;
constexpr int RESERVED_LABEL_MAX = 15; // labels 4-15 are unassigned

enum LabelOpCode {
    PUSH_OPER,
    SWAP_OPER,
    POP_OPER
};

struct LabelOp
{
    int label;
    LabelOpCode optcode;
};

typedef std::vector<LabelOp> LabelOpVector;

/**
 * Represents the Label Information Base (LIB) for MPLS.
 */
class INET_API LibTable : public SimpleModule
{
  public:
    // sentinel inInterfaceId meaning "any incoming interface"
    static constexpr int ANY_INTERFACE = -1;

    struct LibEntry {
        int inLabel;
        int inInterfaceId;

        LabelOpVector outLabel;
        int outInterfaceId;

        // Optional precomputed backup path (e.g. a TI-LFA repair stack per RFC 9855),
        // dormant until activateBackup(inLabel, true) flips backupActive. Nothing in
        // this phase ever calls setBackup()/activateBackup() -- the fields exist purely
        // as inert data-plane plumbing for a later workstream phase. In-class
        // initializers are load-bearing here: unlike inLabel/inInterfaceId/outInterfaceId
        // (which every factory path below assigns explicitly), no existing factory path
        // touches these new fields, so an uninitialized bool would be indeterminate.
        LabelOpVector backupOutLabel;
        int backupOutInterfaceId = -1;
        bool backupActive = false;
    };

  protected:
    int maxLabel;
    std::vector<LibEntry> lib;
    ModuleRefByPar<IInterfaceTable> ift;

    static simsignal_t libEntryCountSignal;

  protected:
    // emits the current LIB size on the libEntryCount signal; call after any change to lib
    virtual void emitLibEntryCount();

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

    // static configuration
    virtual void readTableFromXML(const cXMLElement *libtable);

  public:
    // label management
    virtual bool resolveLabel(int inInterfaceId, int inLabel,
            LabelOpVector& outLabel, int& outInterfaceId);

    virtual int installLibEntry(int inLabel, int inInterfaceId, const LabelOpVector& outLabel,
            int outInterfaceId);

    // Reserve a fresh label without installing a full LIB entry. Used by LDP's
    // Downstream Unsolicited distribution mode under independent control (RFC 5036
    // Section 2.6): the label must be advertised to an upstream peer before the
    // downstream mapping needed to complete the swap is known. installReservedLabel()
    // later creates the actual entry for this same label once that mapping arrives.
    virtual int allocateLabel();

    // Creates a LIB entry for a label previously reserved via allocateLabel().
    // Unlike installLibEntry() -- which either allocates a fresh label
    // (inLabel==-1) or UPDATES an existing entry, throwing if none exists -- this
    // creates a new entry and throws if one with this inLabel already exists.
    virtual int installReservedLabel(int inLabel, int inInterfaceId, const LabelOpVector& outLabel,
            int outInterfaceId);

    virtual void removeLibEntry(int inLabel);

    // Like removeLibEntry(), but returns false instead of throwing when no entry
    // with inLabel exists -- e.g. a label reserved via allocateLabel() that was
    // advertised but never actually installed (independent control, downstream
    // mapping never arrived).
    virtual bool removeLibEntryIfExists(int inLabel);

    // Configures (or replaces) the backup outLabel/outInterfaceId of an existing entry,
    // found by inLabel, WITHOUT activating it (backupActive is left untouched). Throws
    // if no entry with inLabel exists -- like installLibEntry()'s update branch, this is
    // a configuration operation on an entry that must already exist, not an upsert.
    virtual void setBackup(int inLabel, const LabelOpVector& backupOutLabel, int backupOutInterfaceId);

    // Flips backupActive on the entry with the given inLabel: when true, resolveLabel()
    // returns the backup outLabel/outInterfaceId instead of the primary ones for that
    // entry. Returns false (does not throw) if no entry with inLabel exists, mirroring
    // removeLibEntryIfExists()'s no-op-on-miss convention -- callers on the activation
    // path (a future phase's failure/revert handler) may race a removed entry and
    // should not need to guard with a separate existence check.
    virtual bool activateBackup(int inLabel, bool active);

    // utility
    static LabelOpVector pushLabel(int label);
    static LabelOpVector swapLabel(int label);
    static LabelOpVector popLabel();
};

std::ostream& operator<<(std::ostream& os, const LibTable::LibEntry& lib);
std::ostream& operator<<(std::ostream& os, const LabelOpVector& label);

} // namespace inet

#endif

