global contextmenurules
set contextmenurules(keys) {}

proc extendContextMenu {rules} {
    global contextmenurules

    set i [llength $contextmenurules(keys)]
    foreach line $rules {
       lappend contextmenurules(keys) $i
       if {[llength $line]!=4} {
           set rulename "\"[lindex $line 0]\""
           tk_messageBox -type ok -icon info -title Info -message "Context menu inspector rule $rulename should contain 4 items, ignoring."
       } else {
           set contextmenurules($i,label)   [lindex $line 0]
           set contextmenurules($i,context) [lindex $line 1]
           set contextmenurules($i,name)    [lindex $line 2]
           set contextmenurules($i,class)   [lindex $line 3]
       }
       incr i
    }
}

proc popup_insp_menu {ptr X Y} {
    global contextmenurules

    if {$ptr==""} return

    # create popup menu
    catch {destroy .popup}
    menu .popup -tearoff 0

    # add inspector types supported by the object
    set insptypes [opp_supported_insp_types $ptr]
    foreach type $insptypes {
       .popup add command -label "$type..." -command "opp_inspect $ptr \{$type\}"
    }

    # add "run until" menu items
    set baseclass [opp_getobjectbaseclass $ptr]
    if {$baseclass=="cSimpleModule" || $baseclass=="cCompoundModule"} {
        set w ".$ptr-0"  ;#hack
        .popup add separator
        .popup add command -label "Run until next event in this module" -command "runsimulation_local $w normal"
        .popup add command -label "Fast run until next event in this module" -command "runsimulation_local $w fast"
    }

    # add further menu items
    set name [opp_getobjectfullpath $ptr]
    set allcategories "mqsgvo"
    set first 1
    foreach key $contextmenurules(keys) {
       # check context matches
       if {$contextmenurules($key,context)!="" && [opp_getobjectfullname $ptr]!=$contextmenurules($key,context)} {
           continue
       }
       # check we have such object
       set objlist [opp_getsubobjectsfilt $ptr $allcategories $contextmenurules($key,class) "$name.$contextmenurules($key,name)" 1 ""]
       if {$objlist!={}} {
           if {$first} {
               set first 0
               .popup add separator
           }
           .popup add command -label "$contextmenurules($key,label)..." -command "inspect_contextmenurules $ptr $key"
       }
    }
    .popup post $X $Y
}

proc inspect_contextmenurules {ptr key} {
    global contextmenurules
    set allcategories "mqsgvo"
    set name [opp_getobjectfullpath $ptr]
    set objlist [opp_getsubobjectsfilt $ptr $allcategories $contextmenurules($key,class) "$name.$contextmenurules($key,name)" 100 ""]
    if {[llength $objlist] > 5} {
        tk_messageBox -type ok -icon info -title Info -message "This matches [llength $objlist]+ objects, opening inspectors only for the first five."
        set objlist [lrange $objlist 0 4]
    }
    foreach objptr $objlist {
        opp_inspect $objptr "(default)"
    }
}

extendContextMenu {
   {"INET: Interfaces"             ""  "**interfaceTable.interfaces"     "*vector*"}
   {"INET: IP Routing Table"       ""  "**routingTable.routes"           "*vector*"}
   {"INET: IP Multicast Routes"    ""  "**routingTable.multicastRoutes"  "*vector*"}
   {"INET: IPv6 Routing Table"     ""  "**routingTable6.routeList"       "*vector*"}
   {"INET: IPv6 Destination Cache" ""  "**routingTable6.destCache"       "*map*"   }
   {"INET: ARP cache"              ""  "**arp.arpCache"                  "*map*"   }
   {"INET: TCP connections"        ""  "**tcp.tcpAppConnMap"             "*map*"   }

   {"INET: Interfaces"             "interfaceTable"  "interfaces"      "*vector*"}
   {"INET: IP Routing Table"       "routingTable"    "routes"          "*vector*"}
   {"INET: IP Multicast Routes"    "routingTable"    "multicastRoutes" "*vector*"}
   {"INET: IPv6 Routing Table"     "routingTable6"   "routeList"       "*vector*"}
   {"INET: IPv6 Destination Cache" "routingTable6"   "destCache"       "*map*"   }
   {"INET: ARP cache"              "arp"             "arpCache"        "*map*"   }
   {"INET: TCP connections"        "tcp"             "tcpAppConnMap"   "*map*"   }
}

