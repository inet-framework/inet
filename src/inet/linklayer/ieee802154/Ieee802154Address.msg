// SPDX-License-Identifier: LGPL-3.0-or-later
//
import inet.common.INETDefs;
cplusplus {{
#include "inet/linklayer/ieee802154/Ieee802154Address.h"
}}
namespace inet;
class Ieee802154Address
{
    @existingClass;
    @opaque;
    @toString(.str());
    @fromString(inet::Ieee802154Address($));
}
