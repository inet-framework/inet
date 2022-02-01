//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIGNAL_H
#define __INET_SIGNAL_H

#include "inet/common/IPrintableObject.h"

namespace inet {
namespace physicallayer {

class INET_API Signal : public cPacket, public IPrintableObject
{
  public:
    explicit Signal(const char *name = nullptr, short kind = 0, int64_t bitLength = 0);
    Signal(const Signal& other);

    virtual Signal *dup() const override { return new Signal(*this); }

    virtual const char *getFullName() const override;

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual std::string str() const override;
};

} // namespace physicallayer
} // namespace inet

#endif

