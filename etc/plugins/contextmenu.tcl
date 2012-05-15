extendContextMenu {
   {"INET: Interfaces"             "**"  "**interfaceTable.idToInterface"  "*vector*"}
   {"INET: IP Routing Table"       "**"  "**routingTable.routes"           "*vector*"}
   {"INET: IP Multicast Routes"    "**"  "**routingTable.multicastRoutes"  "*vector*"}
   {"INET: IPv6 Routing Table"     "**"  "**routingTable6.routeList"       "*vector*"}
   {"INET: IPv6 Destination Cache" "**"  "**routingTable6.destCache"       "*map*"   }
   {"INET: ARP cache"              "**"  "**arp.arpCache"                  "*map*"   }
   {"INET: TCP connections"        "**"  "**tcp.tcpAppConnMap"             "*map*"   }
   {"INET: TED database"           "**"  "**ted.ted"                       "*vector*"}
   {"INET: LIB table"              "**"  "**libTable.lib"                  "*vector*"}

   {"INET: Interfaces"             "**.interfaceTable"  "idToInterface"   "*vector*"}
   {"INET: IP Routing Table"       "**.routingTable"    "routes"          "*vector*"}
   {"INET: IP Multicast Routes"    "**.routingTable"    "multicastRoutes" "*vector*"}
   {"INET: IPv6 Routing Table"     "**.routingTable6"   "routeList"       "*vector*"}
   {"INET: IPv6 Destination Cache" "**.routingTable6"   "destCache"       "*map*"   }
   {"INET: ARP cache"              "**.arp"             "arpCache"        "*map*"   }
   {"INET: TCP connections"        "**.tcp"             "tcpAppConnMap"   "*map*"   }
   {"INET: TED database"           "**.ted"             "ted"             "*vector*"}
   {"INET: LIB table"              "**.libTable"        "lib"             "*vector*"}

   {"INET: OSPF areas"             "**"         "**ospf.areas"            "*vector*"}
   {"INET: OSPF areas"             "**.ospf"    "areas"                   "*vector*"}
   {"INET: OSPF ASExternalLSA"     "**"         "**ospf.asExternalLSAs"   "*vector*"}
   {"INET: OSPF ASExternalLSA"     "**.ospf"    "asExternalLSAs"          "*vector*"}
   {"INET: OSPF RoutingTable"      "**"         "**ospf.routingTable"     "*vector*"}
   {"INET: OSPF RoutingTable"      "**.ospf"    "routingTable"            "*vector*"}
}

