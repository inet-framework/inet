extendContextMenu {
   {"INET: Interfaces"             "**"  "**interfaceTable.interfaces"     "*vector*"}
   {"INET: IP Routing Table"       "**"  "**routingTable.routes"           "*vector*"}
   {"INET: IP Multicast Routes"    "**"  "**routingTable.multicastRoutes"  "*vector*"}
   {"INET: IPv6 Routing Table"     "**"  "**routingTable6.routeList"       "*vector*"}
   {"INET: IPv6 Destination Cache" "**"  "**routingTable6.destCache"       "*map*"   }
   {"INET: ARP cache"              "**"  "**arp.arpCache"                  "*map*"   }
   {"INET: TCP connections"        "**"  "**tcp.tcpAppConnMap"             "*map*"   }
   {"INET: TED database"           "**"  "**ted.ted"                       "*vector*"}
   {"INET: LIB table"              "**"  "**libTable.lib"                  "*vector*"}

   {"INET: Interfaces"             "**.interfaceTable"  "interfaces"      "*vector*"}
   {"INET: IP Routing Table"       "**.routingTable"    "routes"          "*vector*"}
   {"INET: IP Multicast Routes"    "**.routingTable"    "multicastRoutes" "*vector*"}
   {"INET: IPv6 Routing Table"     "**.routingTable6"   "routeList"       "*vector*"}
   {"INET: IPv6 Destination Cache" "**.routingTable6"   "destCache"       "*map*"   }
   {"INET: ARP cache"              "**.arp"             "arpCache"        "*map*"   }
   {"INET: TCP connections"        "**.tcp"             "tcpAppConnMap"   "*map*"   }
   {"INET: TED database"           "**.ted"             "ted"             "*vector*"}
   {"INET: LIB table"              "**.libTable"        "lib"             "*vector*"}
}

