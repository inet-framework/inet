//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;

namespace inet;

class TlvOptionBase extends cObject
{
    @packetData;
    short type = -1;
    short length = 0;   // total length of option
}

class TlvOptionRaw extends TlvOptionBase
{
    char bytes[];
}

class TlvOptions extends cObject
{
    @packetData;
    TlvOptionBase *tlvOption[] @owned;
}

cplusplus(TlvOptions) {{
  public:
    /*
     * Removes option from optionVector and returns option when removed, otherwise returns nullptr.
     */
    TlvOptionBase *dropTlvOption(TlvOptionBase *option);

    /*
     * Removes all options or first only option where type is the specified type.
     */
    void deleteOptionByType(int type, bool firstOnly = true);

    /*
     * Calculate and returns the total length of all stored options in bytes
     */
    virtual int getLength() const;

    /*
     * Find the first option with specified type. Search started with element at firstPos.
     * Returns the position of found option, or return -1 if not found
     */
    int findByType(short int type, int firstPos=0) const;
}}
