//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_PCAPRECORDER_H
#define __INET_PCAPRECORDER_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/common/packet/recorder/PacketDump.h"
#include "inet/common/packet/recorder/PcapWriter.h"

namespace inet {

enum PcapLinkType {
    LINKTYPE_INVALID = -1,
    /** BSD loopback encapsulation */
    LINKTYPE_NULL = 0,
    /** IEEE 802.3 Ethernet */
    LINKTYPE_ETHERNET = 1,
    /** AX.25 packet */
    LINKTYPE_AX25 = 3,
    /** IEEE 802.5 Token Ring */
    LINKTYPE_IEEE802_5 = 6,
    /** ARCNET Data Packets */
    LINKTYPE_ARCNET_BSD = 7,
    /** SLIP, encapsulated with a LINKTYPE_SLIP header */
    LINKTYPE_SLIP = 8,
    /** PPP, as per RFC 1661 and RFC 1662 */
    LINKTYPE_PPP = 9,
    /** FDDI, as specified by ANSI INCITS 239-1994 */
    LINKTYPE_FDDI = 10,
    /** Raw IP */
    LINKTYPE_DLT_RAW1 = 12,
    /** Raw IP (OpenBSD) */
    LINKTYPE_DLT_RAW2 = 14,
    /** PPP in HDLC-like framing, as per RFC 1662, or Cisco PPP with HDLC framing, as per section 4.3.1 of RFC 1547 */
    LINKTYPE_PPP_HDLC = 50,
    /** PPPoE */
    LINKTYPE_PPP_ETHER = 51,
    /** RFC 1483 LLC/SNAP-encapsulated ATM */
    LINKTYPE_ATM_RFC1483 = 100,
    /** Raw IP */
    LINKTYPE_RAW = 101,
    /** Cisco PPP with HDLC framing */
    LINKTYPE_C_HDLC = 104,
    /** IEEE 802.11 wireless LAN */
    LINKTYPE_IEEE802_11 = 105,
    /** Frame Relay */
    LINKTYPE_FRELAY = 107,
    /** OpenBSD loopback encapsulation */
    LINKTYPE_LOOP = 108,
    /** Linux "cooked" capture encapsulation */
    LINKTYPE_LINUX_SLL = 113,
    /** Apple LocalTalk */
    LINKTYPE_LTALK = 114,
    /** OpenBSD pflog */
    LINKTYPE_PFLOG = 117,
    /** Prism monitor mode information followed by an 802.11 header */
    LINKTYPE_IEEE802_11_PRISM = 119,
    /** RFC 2625 IP-over-Fibre Channel */
    LINKTYPE_IP_OVER_FC = 122,
    /** ATM traffic, encapsulated as per the scheme used by SunATM devices */
    LINKTYPE_SUNATM = 123,
    /** Radiotap link-layer information followed by an 802.11 header */
    LINKTYPE_IEEE802_11_RADIOTAP = 127,
    /** ARCNET Data Packets, as described by the ARCNET Trade Association standard ATA 878.1-1999 */
    LINKTYPE_ARCNET_LINUX = 129,
    /** Apple IP-over-IEEE 1394 cooked header */
    LINKTYPE_APPLE_IP_OVER_IEEE1394 = 138,
    /** Signaling System 7 Message Transfer Part Level 2 */
    LINKTYPE_MTP2_WITH_PHDR = 139,
    /** Signaling System 7 Message Transfer Part Level 2 */
    LINKTYPE_MTP2 = 140,
    /** Signaling System 7 Message Transfer Part Level 3 */
    LINKTYPE_MTP3 = 141,
    /** Signaling System 7 Signalling Connection Control Part */
    LINKTYPE_SCCP = 142,
    /** Signaling System 7 Signalling Connection Control Part */
    LINKTYPE_DOCSIS = 143,
    /** Linux-IrDA packets */
    LINKTYPE_LINUX_IRDA = 144,
    /** AVS monitor mode information followed by an 802.11 header */
    LINKTYPE_IEEE802_11_AVS = 163,
    /** BACnet MS/TP frames */
    LINKTYPE_BACNET_MS_TP = 165,
    /** PPP in HDLC-like encapsulation, like LINKTYPE_PPP_HDLC, but with the 0xff address byte replaced by a direction indication - 0x00 for incoming and 0x01 for outgoing */
    LINKTYPE_PPP_PPPD = 166,
    /** General Packet Radio Service Logical Link Control */
    LINKTYPE_GPRS_LLC = 169,
    /** Transparent-mapped generic framing procedure */
    LINKTYPE_GPF_T = 170,
    /** Frame-mapped generic framing procedure */
    LINKTYPE_GPF_F = 171,
    /** Link Access Procedures on the D Channel (LAPD) frames */
    LINKTYPE_LINUX_LAPD = 177,
    /** Bluetooth HCI UART transport layer */
    LINKTYPE_BLUETOOTH_HCI_H4 = 187,
    /** USB packets, beginning with a Linux USB header */
    LINKTYPE_USB_LINUX = 189,
    /** Per-Packet Information information */
    LINKTYPE_PPI = 192,
    /** IEEE 802.15.4 wireless Personal Area Network */
    LINKTYPE_IEEE802_15_4 = 195,
    /** Various link-layer types, with a pseudo-header, for SITA */
    LINKTYPE_SITA = 196,
    /** Various link-layer types, with a pseudo-header, for Endace DAG cards; encapsulates Endace ERF record */
    LINKTYPE_ERF = 197,
    /** Bluetooth HCI UART transport layer */
    LINKTYPE_BLUETOOTH_HCI_H4_WITH_PHDR = 201,
    /** AX.25 packet, with a 1-byte KISS header containing a type indicator */
    LINKTYPE_AX25_KISS = 202,
    /** Link Access Procedures on the D Channel (LAPD) frames */
    LINKTYPE_LAPD = 203,
    /** PPP, as per RFC 1661 and RFC 1662, preceded with a one-byte pseudo-header with a zero value meaning "received by this host" and a non-zero value meaning  "sent by this host" */
    LINKTYPE_PPP_WITH_DIR = 204,
    /** Cisco PPP with HDLC framing */
    LINKTYPE_C_HDLC_WITH_DIR = 205,
    /** Frame Relay */
    LINKTYPE_FRELAY_WITH_DIR = 206,
    /** IPMB over an I2C circuit */
    LINKTYPE_IPMB_LINUX = 209,
    /** IEEE 802.15.4 wireless Personal Area Network */
    LINKTYPE_IEEE802_15_4_NONASK_PHY = 215,
    /** USB packets, beginning with a Linux USB header */
    LINKTYPE_USB_LINUX_MMAPPED = 220,
    /** Fibre Channel FC-2 frames, beginning with a Frame_Header */
    LINKTYPE_FC_2 = 224,
    /** Fibre Channel FC-2 frames */
    LINKTYPE_FC_2_WITH_FRAME_DELIMS = 225,
    /** Solaris ipnet pseudo-header */
    LINKTYPE_IPNET = 226,
    /** CAN (Controller Area Network) frames, with a pseudo-header as supplied by Linux SocketCAN */
    LINKTYPE_CAN_SOCKETCAN = 227,
    /** Raw IPv4; the packet begins with an IPv4 header */
    LINKTYPE_IPV4 = 228,
    /** Raw IPv6; the packet begins with an IPv6 header */
    LINKTYPE_IPV6 = 229,
    /** IEEE 802.15.4 wireless Personal Area Network, without the FCS at the end of the frame */
    LINKTYPE_IEEE802_15_4_NOFCS = 230,
    /** Raw D-Bus messages, starting with the endianness flag, followed by the message type, etc., but without the authentication handshake before the message sequence */
    LINKTYPE_DBUS = 231,
    /** DVB-CI (DVB Common Interface for communication between a PC Card module and a DVB receiver), with the message format specified by the PCAP format for DVB-CI specification */
    LINKTYPE_DVB_CI = 235,
    /** Variant of 3GPP TS 27.010 multiplexing protocol (similar to, but not the same as, 27.010) */
    LINKTYPE_MUX27010 = 236,
    /** D_PDUs as described by NATO standard STANAG 5066, starting with the synchronization sequence, and including both header and data CRCs */
    LINKTYPE_STANAG_5066_D_PDU = 237,
    /** Linux netlink NETLINK NFLOG socket log messages */
    LINKTYPE_NFLOG = 239,
    /** Pseudo-header for Hilscher Gesellschaft für Systemautomation mbH netANALYZER devices, followed by an Ethernet frame, beginning with the MAC header and ending with the FCS */
    LINKTYPE_NETANALYZER = 240,
    /** Pseudo-header for Hilscher Gesellschaft für Systemautomation mbH netANALYZER devices, followed by an Ethernet frame, beginning with the preamble, SFD, and MAC header, and ending with the FCS */
    LINKTYPE_NETANALYZER_TRANSPARENT = 241,
    /** IP-over-InfiniBand, as specified by RFC 4391 section 6 */
    LINKTYPE_IPOIB = 242,
    /** MPEG-2 Transport Stream transport packets, as specified by ISO 13818-1/ITU-T Recommendation H.222.0 */
    LINKTYPE_MPEG_2_TS = 243,
    /** Pseudo-header for ng4T GmbH's UMTS Iub/Iur-over-ATM and Iub/Iur-over-IP format as used by their ng40 protocol tester */
    LINKTYPE_NG40 = 244,
    /** Pseudo-header for NFC LLCP packet captures, followed by frame data for the LLCP Protocol as specified by NFCForum-TS-LLCP_1.1 */
    LINKTYPE_NFC_LLCP = 245,
    /** Raw InfiniBand frames, starting with the Local Routing Header */
    LINKTYPE_INFINIBAND = 247,
    /** SCTP packets, as defined by RFC 4960, with no lower-level protocols such as IPv4 or IPv6 */
    LINKTYPE_SCTP = 248,
    /** USB packets, beginning with a USBPcap header */
    LINKTYPE_USBPCAP = 249,
    /** Serial-line packet header for the Schweitzer Engineering Laboratories "RTAC" product */
    LINKTYPE_RTAC_SERIAL = 250,
    /** Bluetooth Low Energy air interface Link Layer packets */
    LINKTYPE_BLUETOOTH_LE_LL = 251,
    /** Linux Netlink capture encapsulation */
    LINKTYPE_NETLINK = 253,
    /** Bluetooth Linux Monitor encapsulation of traffic for the BlueZ stack */
    LINKTYPE_BLUETOOTH_LINUX_MONITOR = 254,
    /** Bluetooth Basic Rate and Enhanced Data Rate baseband packets */
    LINKTYPE_BLUETOOTH_BREDR_BB = 255,
    /** Bluetooth Low Energy link-layer packets */
    LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR = 256,
    /** PROFIBUS data link layer packets, as specified by IEC standard 61158-6-3 */
    LINKTYPE_PROFIBUS_DL = 257,
    /** Apple PKTAP capture encapsulation */
    LINKTYPE_PKTAP = 258,
    /** Ethernet-over-passive-optical-network packets */
    LINKTYPE_EPON = 259,
    /** IPMI trace packets, as specified by Table 3-20 "Trace Data Block Format" in the PICMG HPM.2 specification */
    LINKTYPE_IPMI_HPM_2 = 260,
    /** Per Joshua Wright <jwright@hasborg.com>, formats for Z-Wave RF profiles R1 and R2 captures */
    LINKTYPE_ZWAVE_R1_R2 = 261,
    /** Per Joshua Wright <jwright@hasborg.com>, formats for Z-Wave RF profile R3 captures */
    LINKTYPE_ZWAVE_R3 = 262,
    /** Formats for WattStopper Digital Lighting Management (DLM) and Legrand Nitoo Open protocol common packet structure captures */
    LINKTYPE_WATTSTOPPER_DLM = 263,
    /** Messages between ISO 14443 contactless smartcards (Proximity Integrated Circuit Card, PICC) and card readers (Proximity Coupling Device, PCD), with the message format specified by the PCAP format for ISO14443 specification */
    LINKTYPE_ISO_14443 = 264
};

/**
 * Dumps every packet using the PcapWriter and PacketDump classes
 */
class INET_API PcapRecorder : public cSimpleModule, protected cListener
{
  public:
    class INET_API IHelper {
      public:
        virtual ~IHelper() { }

        /// returns pcapLinkType for given protocol or returns LINKTYPE_INVALID. Protocol storable as or convertable to pcapLinkType.
        virtual PcapLinkType protocolToLinkType(const Protocol *protocol) const = 0;

        /// returns true when the protocol storable as pcapLinkType without conversion.
        virtual bool matchesLinkType(PcapLinkType pcapLinkType, const Protocol *protocol) const = 0;

        /// Create a new Packet or return nullptr. The new packet contains the original packet converted to pcapLinkType format.
        virtual Packet *tryConvertToLinkType(const Packet* packet, PcapLinkType pcapLinkType, const Protocol *protocol) const = 0;
    };
  protected:
    typedef std::map<simsignal_t, bool> SignalList;
    std::vector<const Protocol *> dumpProtocols;
    SignalList signalList;
    PacketDump packetDumper;
    PcapWriter pcapWriter;
    unsigned int snaplen = 0;
    bool dumpBadFrames = false;
    PacketFilter packetFilter;
    int numRecorded = 0;
    PcapLinkType pcapLinkType = LINKTYPE_INVALID;
    bool recordPcap = false;
    std::vector<IHelper *> helpers;

    static simsignal_t packetRecordedSignal;

  public:
    PcapRecorder();
    virtual ~PcapRecorder();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual void updateDisplayString() const;
    virtual void finish() override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void recordPacket(const cPacket *msg, bool l2r);
    virtual bool matchesLinkType(const Protocol *protocol) const;
    virtual Packet *tryConvertToLinkType(const Packet* packet, const Protocol *protocol) const;
    virtual PcapLinkType protocolToLinkType(const Protocol *protocol) const;
};

} // namespace inet

#endif // ifndef __INET_PCAPRECORDER_H

