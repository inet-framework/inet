from inet.documentation import *
from inet.simulation import *

def generate_tsn_charts():
    # TODO
    generate_charts(inet_project, path_filter = "trafficshaping")

def generate_tsn_ned_documentation():
    generate_ned_documentation([
        "/inet/src/inet/linklayer/configurator",
        "/inet/src/inet/linklayer/ethernet",
        "/inet/src/inet/linklayer/vlan",
        "/inet/src/inet/physicallayer",
        "MARKER",
        "/inet/src/inet/applications",
        "/inet/src/inet/common",
        "/inet/src/inet/emulation",
        "/inet/src/inet/environment",
        "/inet/src/inet/linklayer/acking",
        "/inet/src/inet/linklayer/base",
        "/inet/src/inet/linklayer/bmac",
        "/inet/src/inet/linklayer/common",
        "/inet/src/inet/linklayer/contract",
        "/inet/src/inet/linklayer/csmaca",
        "/inet/src/inet/linklayer/ethernet/base",
        "/inet/src/inet/linklayer/ethernet/basic",
        "/inet/src/inet/linklayer/ieee80211",
        "/inet/src/inet/linklayer/ieee8021d",
        "/inet/src/inet/linklayer/ieee802154",
        "/inet/src/inet/linklayer/ieee8021ae",
        "/inet/src/inet/linklayer/ieee8022",
        "/inet/src/inet/linklayer/lmac",
        "/inet/src/inet/linklayer/loopback",
        "/inet/src/inet/linklayer/ppp",
        "/inet/src/inet/linklayer/shortcut",
        "/inet/src/inet/linklayer/tun",
        "/inet/src/inet/linklayer/virtual",
        "/inet/src/inet/linklayer/xmac",
        "/inet/src/inet/mobility",
        "/inet/src/inet/networklayer",
        "/inet/src/inet/networks",
        "/inet/src/inet/node/aodv",
        "/inet/src/inet/node/bgp",
        "/inet/src/inet/node/dsdv",
        "/inet/src/inet/node/dymo",
        "/inet/src/inet/node/eigrp",
        "/inet/src/inet/node/gpsr",
        "/inet/src/inet/node/httptools",
        "/inet/src/inet/node/internetcloud",
        "/inet/src/inet/node/ipv6",
        "/inet/src/inet/node/mpls",
        "/inet/src/inet/node/ospfv2",
        "/inet/src/inet/node/ospfv3",
        "/inet/src/inet/node/packetdrill",
        "/inet/src/inet/node/rip",
        "/inet/src/inet/node/rtp",
        "/inet/src/inet/node/wireless",
        "/inet/src/inet/node/xmipv6",
        "/inet/src/inet/physicallayer/common",
        "/inet/src/inet/physicallayer/wired/common",
        "/inet/src/inet/physicallayer/wireless",
        "/inet/src/inet/power",
        "/inet/src/inet/protocolelement/acknowledgement",
        "/inet/src/inet/protocolelement/aggregation",
        "/inet/src/inet/protocolelement/checksum",
        "/inet/src/inet/protocolelement/common",
        "/inet/src/inet/protocolelement/contract",
        "/inet/src/inet/protocolelement/dispatching",
        "/inet/src/inet/protocolelement/forwarding",
        "/inet/src/inet/protocolelement/fragmentation",
        "/inet/src/inet/protocolelement/lifetime",
        "/inet/src/inet/protocolelement/measurement",
        "/inet/src/inet/protocolelement/ordering",
        "/inet/src/inet/protocolelement/selectivity",
        "/inet/src/inet/protocolelement/service",
        "/inet/src/inet/protocolelement/socket",
        "/inet/src/inet/protocolelement/trafficconditioner",
        "/inet/src/inet/protocolelement/transceiver",
        "/inet/src/inet/queueing/compat",
        "/inet/src/inet/queueing/flow",
        "/inet/src/inet/queueing/function",
        "/inet/src/inet/queueing/marker",
        "/inet/src/inet/routing",
        "/inet/src/inet/transportlayer",
        "/inet/src/inet/visualizer",
        "/inet/examples",
        "/inet/showcases",
        "/inet/tutorials"])

def generate_tsn_html_documentation():
    generate_html_documentation()

def upload_tsn_documentation():
    upload_html_documentation("~/httpdocs/tmp/tsn")

def open_tsn_documentation():
    open_html_documentation("showcases/tsn/index.html")

def generate_tsn_documentation():
    generate_tsn_charts()
    generate_tsn_ned_documentation()
    generate_tsn_html_documentation()
    upload_tsn_documentation()
    open_tsn_documentation()
