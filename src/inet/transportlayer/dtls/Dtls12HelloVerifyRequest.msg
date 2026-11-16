// SPDX-License-Identifier: LGPL-3.0-or-later

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::dtls;

class Dtls12HelloVerifyRequest extends FieldsChunk
{
    uint16_t serverVersion = 0xfeff;
    uint8_t cookie[];
}

// One opaque modeled certificate, or an empty client certificate list.
class Dtls12Certificate extends FieldsChunk
{
    uint8_t certificate[];
}

class Dtls12PskServerKeyExchange extends FieldsChunk
{
    uint8_t identityHint[];
}

class Dtls12PskClientKeyExchange extends FieldsChunk
{
    uint8_t identity[];
}

class Dtls12EcdheServerKeyExchange extends FieldsChunk
{
    uint16_t namedGroup = 0x0017;
    uint8_t publicKey[];
    uint16_t signatureScheme = 0x0403;
    uint8_t signature[];
}

class Dtls12EcdheClientKeyExchange extends FieldsChunk
{
    uint8_t publicKey[];
}

class Dtls12CertificateRequest extends FieldsChunk
{
    uint8_t certificateTypes[];
    uint16_t signatureSchemes[];
    // Encoded length-prefixed DistinguishedName sequence; selected profile is empty.
    uint8_t certificateAuthorities[];
}

class Dtls12ServerHelloDone extends FieldsChunk
{
    chunkLength = B(0);
}

class Dtls12ChangeCipherSpec extends FieldsChunk
{
    chunkLength = B(1);
    uint8_t value = 1;
}

class Dtls12ExplicitNonce extends FieldsChunk
{
    chunkLength = B(8);
    uint64_t value = 0;
}
