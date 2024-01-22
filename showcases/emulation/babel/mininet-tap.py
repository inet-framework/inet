import re
import sys
import subprocess
import os

from sys import exit  # pylint: disable=redefined-builtin

from mininet.cli import CLI
from mininet.log import setLogLevel, info, error
from mininet.net import Mininet
from mininet.link import Intf
from mininet.topo import LinearTopo
from mininet.util import quietRun


def checkIntf( intf ):
    "Make sure intf exists and is not configured."
    config = quietRun( 'ifconfig %s 2>/dev/null' % intf, shell=True )
    if not config:
        error( 'Error:', intf, 'does not exist!\n' )
        exit( 1 )
    ips = re.findall( r'\d+\.\d+\.\d+\.\d+', config )
    if ips:
        error( 'Error:', intf, 'has an IP address,'
               'and is probably in use!\n' )
        exit( 1 )


setLogLevel( 'info' )

subprocess.run(['sudo', 'ip', '-all', 'netns', 'delete'])

info( '*** Creating network\n' )
net = Mininet( topo=LinearTopo( k=0, n=3 ), controller=None)

host0 = net.addHost('h1')
host1 = net.addHost('h2')
host2 = net.addHost('h3')

# add tap interfaces
subprocess.run(['bash', '-c', 'sudo tunctl -t tap0 -u $SAVED_USER -g $SAVED_GROUP && sudo ip link set dev tap0 up'])
subprocess.run(['bash', '-c', 'sudo tunctl -t tap1 -u $SAVED_USER -g $SAVED_GROUP && sudo ip link set dev tap1 up'])
subprocess.run(['bash', '-c', 'sudo tunctl -t tap2 -u $SAVED_USER -g $SAVED_GROUP && sudo ip link set dev tap2 up'])

info( '*** Checking', 'tap0', '\n' )
checkIntf( 'tap0' )

info( '*** Checking', 'tap1', '\n' )
checkIntf( 'tap1' )

info( '*** Checking', 'tap2', '\n' )
checkIntf( 'tap2' )

info( '*** Adding hardware interface', 'tap0', 'to host', host0.name, '\n' )
_intf = Intf( 'tap0', node=host0, ip='192.168.2.1/30' )

info( '*** Adding hardware interface', 'tap1', 'to host', host1.name, '\n' )
_intf = Intf( 'tap1', node=host1, ip='192.168.3.1/30' )

info( '*** Adding hardware interface', 'tap2', 'to host', host2.name, '\n' )
_intf = Intf( 'tap2', node=host2, ip='192.168.4.1/30' )

pid0 = net.hosts[0].pid
pid1 = net.hosts[1].pid
pid2 = net.hosts[2].pid

cmd0 = """sudo ip netns attach host0 """ + str(pid0)
cmd1 = """sudo ip netns attach host1 """ + str(pid1)
cmd2 = """sudo ip netns attach host2 """ + str(pid2)

subprocess.run(['bash', '-c', str(cmd0)])
subprocess.run(['bash', '-c', str(cmd1)])
subprocess.run(['bash', '-c', str(cmd2)])

net.start()

# remove default routes
host0.cmd("ip route del 192.168.2.0/30")
host1.cmd("ip route del 192.168.3.0/30")
host2.cmd("ip route del 192.168.4.0/30")

# start babel daemons on hosts
host0.cmd("""babeld -I babel0.pid -S babel-state0 -r -w -h 2 -M 0 -C 'reflect-kernel-metric true' -C 'interface tap0 channel 1 link-quality false' &""")
host1.cmd("""babeld -I babel1.pid -S babel-state1 -r -w -h 2 -M 0 -C 'reflect-kernel-metric true' -C 'interface tap1 channel 1 link-quality false' &""")
host2.cmd("""babeld -I babel2.pid -S babel-state2 -r -w -h 2 -M 0 -C 'reflect-kernel-metric true' -C 'interface tap2 channel 1 link-quality false' &""")

# start ping loop
host0.cmd("bash -c 'while ! ping 192.168.4.1; do sleep 1; done' &")

# watch routing tables
host0.cmd("""xterm -e "watch -n 0.1 'ip route'" &""")
host1.cmd("""xterm -e "watch -n 0.1 'ip route'" &""")
host2.cmd("""xterm -e "watch -n 0.1 'ip route'" &""")

# uncomment to run wireshark on host
#host0.cmd("wireshark -i tap0 -k &")
#host1.cmd("wireshark -i tap1 -k &")
#host2.cmd("wireshark -i tap2 -k &")

# start inet
user = os.getenv('SAVED_USER')
group = os.getenv('SAVED_GROUP')
saved_path = os.getenv('SAVED_PATH')
saved_env = os.environ.copy()
saved_env["PATH"] = saved_path
# Note: Mininet runs as root, but we want to run the simulation as the normal user. We use 'sudo' to specify the user.
# We also need to restore the original PATH which is overwritten by 'sudo'.
subprocess.run(['sudo -E -u $SAVED_USER bash -c "export PATH=$SAVED_PATH && inet"'], shell=True)

CLI( net )

subprocess.run(['sudo', 'ip', '-all', 'netns', 'delete'])

net.stop()
