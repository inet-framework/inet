//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::coap;

//
// CoAP message types (RFC 7252 Section 3).
//
enum CoapMessageType
{
    COAP_TYPE_CON = 0;
    COAP_TYPE_NON = 1;
    COAP_TYPE_ACK = 2;
    COAP_TYPE_RST = 3;
}

//
// Standard CoAP method and response codes in c.dd wire encoding ((class << 5) | detail).
//
enum CoapMessageCode
{
    COAP_CODE_EMPTY = 0;                        // 0.00
    COAP_CODE_GET = 1;                          // 0.01
    COAP_CODE_POST = 2;                         // 0.02
    COAP_CODE_PUT = 3;                          // 0.03
    COAP_CODE_DELETE = 4;                       // 0.04
    COAP_CODE_CREATED = 65;                     // 2.01
    COAP_CODE_DELETED = 66;                     // 2.02
    COAP_CODE_VALID = 67;                       // 2.03
    COAP_CODE_CHANGED = 68;                     // 2.04
    COAP_CODE_CONTENT = 69;                     // 2.05
    COAP_CODE_CONTINUE = 95;                    // 2.31
    COAP_CODE_BAD_REQUEST = 128;                // 4.00
    COAP_CODE_UNAUTHORIZED = 129;               // 4.01
    COAP_CODE_BAD_OPTION = 130;                 // 4.02
    COAP_CODE_FORBIDDEN = 131;                  // 4.03
    COAP_CODE_NOT_FOUND = 132;                  // 4.04
    COAP_CODE_METHOD_NOT_ALLOWED = 133;         // 4.05
    COAP_CODE_NOT_ACCEPTABLE = 134;             // 4.06
    COAP_CODE_REQUEST_ENTITY_INCOMPLETE = 136;  // 4.08
    COAP_CODE_PRECONDITION_FAILED = 140;        // 4.12
    COAP_CODE_REQUEST_ENTITY_TOO_LARGE = 141;   // 4.13
    COAP_CODE_UNSUPPORTED_CONTENT_FORMAT = 143; // 4.15
    COAP_CODE_INTERNAL_SERVER_ERROR = 160;      // 5.00
    COAP_CODE_NOT_IMPLEMENTED = 161;            // 5.01
    COAP_CODE_BAD_GATEWAY = 162;                // 5.02
    COAP_CODE_SERVICE_UNAVAILABLE = 163;        // 5.03
    COAP_CODE_GATEWAY_TIMEOUT = 164;            // 5.04
    COAP_CODE_PROXYING_NOT_SUPPORTED = 165;     // 5.05
}

//
// Supported CoAP option numbers (RFC 7252, RFC 7641, RFC 7959, RFC 9175).
//
enum CoapOptionNumber
{
    COAP_OPT_IF_MATCH = 1;
    COAP_OPT_URI_HOST = 3;
    COAP_OPT_ETAG = 4;
    COAP_OPT_IF_NONE_MATCH = 5;
    COAP_OPT_OBSERVE = 6;
    COAP_OPT_URI_PORT = 7;
    COAP_OPT_LOCATION_PATH = 8;
    COAP_OPT_URI_PATH = 11;
    COAP_OPT_CONTENT_FORMAT = 12;
    COAP_OPT_MAX_AGE = 14;
    COAP_OPT_URI_QUERY = 15;
    COAP_OPT_ACCEPT = 17;
    COAP_OPT_LOCATION_QUERY = 20;
    COAP_OPT_BLOCK2 = 23;
    COAP_OPT_BLOCK1 = 27;
    COAP_OPT_SIZE2 = 28;
    COAP_OPT_PROXY_URI = 35;
    COAP_OPT_PROXY_SCHEME = 39;
    COAP_OPT_SIZE1 = 60;
    COAP_OPT_REQUEST_TAG = 292;
}

//
// Ordered TLV option entry in a CoAP header.
//
class CoapOption extends cObject
{
    @packetData;
    uint16_t number = 0;
    uint8_t value[];
}

//
// CoAP fixed header, token, ordered options, and optional payload marker (0xFF).
// Payload bytes live in the trailing BytesChunk of the packet.
//
class CoapHeader extends FieldsChunk
{
    uint8_t version = 1;
    CoapMessageType messageType = COAP_TYPE_NON;
    uint8_t token[];
    uint8_t code = COAP_CODE_EMPTY;
    uint16_t messageId = 0;
    CoapOption options[];
    bool payloadMarkerPresent = false;
}
