#pragma once

#include "INETDefs.h"
#include "IGMP.h"
#include "ModuleAccess.h"

class INET_API IGMPAccess : public ModuleAccess<IGMP>
{
public:
	IGMPAccess() : ModuleAccess<IGMP>("igmp") {}
};
