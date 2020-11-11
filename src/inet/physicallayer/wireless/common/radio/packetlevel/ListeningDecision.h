//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_LISTENINGDECISION_H
#define __INET_LISTENINGDECISION_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IListeningDecision.h"

namespace inet {
namespace physicallayer {

class INET_API ListeningDecision : public IListeningDecision, public cObject
{
  protected:
    const IListening *listening;
    const bool isListeningPossible_;

  public:
    ListeningDecision(const IListening *listening, bool isListeningPossible_);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const IListening *getListening() const override { return listening; }

    virtual bool isListeningPossible() const override { return isListeningPossible_; }
};

} // namespace physicallayer
} // namespace inet

#endif

