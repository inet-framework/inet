//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::tls;

// TLS 1.3 handshake bodies. Extensions are stored as their encoded vector.
class TlsClientHello extends FieldsChunk
{
    uint16_t legacyVersion = 0x0303;
    uint8_t random[32];
    uint8_t sessionId[];
    uint16_t cipherSuites[];
    uint8_t compressionMethods[];
    uint8_t extensions[];
}

class TlsServerHello extends FieldsChunk
{
    uint16_t legacyVersion = 0x0303;
    uint8_t random[32];
    uint8_t sessionId[];
    uint16_t cipherSuite = 0x1301;
    uint8_t compressionMethod = 0;
    uint8_t extensions[];
}

// These byte arrays contain the encoded TLS vectors' contents; their length
// prefixes are supplied by the serializer, not by the association.
class TlsEncryptedExtensions extends FieldsChunk
{
    uint8_t extensions[];
}

class TlsCertificateRequest extends FieldsChunk
{
    uint8_t requestContext[];
    uint8_t extensions[];
}

class TlsCertificate extends FieldsChunk
{
    uint8_t requestContext[];
    // Encoded CertificateEntry sequence, including each entry's lengths.
    uint8_t certificateList[];
}

class TlsCertificateVerify extends FieldsChunk
{
    uint16_t signatureScheme = 0;
    uint8_t signature[];
}

class TlsFinished extends FieldsChunk
{
    uint8_t verifyData[];
}

class TlsKeyUpdate extends FieldsChunk
{
    chunkLength = B(1);
    uint8_t requestUpdate = 0;
}
