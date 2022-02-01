//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/serializer/ChunkSerializer.h"

namespace inet {

b ChunkSerializer::totalSerializedLength = b(0);
b ChunkSerializer::totalDeserializedLength = b(0);

} // namespace

