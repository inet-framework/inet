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

def setupHost(host, netns, tap_intf, ip):
    "Creates TAP interface in the host OS, assigns IP address to it, creates Mininet host and attaches TAP interface to it."
    info( '*** Adding host', host, 'to Mininet network', '\n' )
    python_host = net.addHost(host)
    info( '*** Adding tap interface', tap_intf, 'in host OS', '\n' )
    subprocess.run(['bash', '-c', f'sudo tunctl -t {tap_intf} -u $SAVED_USER -g $SAVED_GROUP && sudo ip link set dev {tap_intf} up'])
    info( '*** Checking', tap_intf, '\n' )
    checkIntf( tap_intf )
    info( '*** Adding hardware interface', tap_intf, 'to Mininet host', host, '\n' )
    _intf = Intf( tap_intf, node=python_host, ip=ip )
    info( '*** Attaching mininet tap interface to host OS tap interface', tap_intf, '\n' )
    host_pid = python_host.pid
    subprocess.run(['bash', '-c', f'sudo ip netns attach {netns} {host_pid}'])
    return python_host


setLogLevel( 'info' )

# delete all network namespaces
subprocess.run(['sudo', 'ip', '-all', 'netns', 'delete'])

info( '*** Creating network\n' )
net = Mininet( topo=LinearTopo( k=0, n=3 ), controller=None)

# creating hosts
host0 = setupHost(host='h0', netns='host0', tap_intf='tap0', ip='192.168.2.1/30')
host1 = setupHost(host='h1', netns='host1', tap_intf='tap1', ip='192.168.3.1/30')
host2 = setupHost(host='h2', netns='host2', tap_intf='tap2', ip='192.168.4.1/30')

info( '*** Starting Mininet network', '\n' )
net.start()

# remove default routes
host0.cmd("ip route del 192.168.2.0/30")
host1.cmd("ip route del 192.168.3.0/30")
host2.cmd("ip route del 192.168.4.0/30")

info( '*** Starting Babel daemons', '\n' )
host0.cmd("""babeld -I babel0.pid -S babel-state0 -r -w -h 2 -M 0 -C 'reflect-kernel-metric true' -C 'interface tap0 channel 1 link-quality false' &""")
host1.cmd("""babeld -I babel1.pid -S babel-state1 -r -w -h 2 -M 0 -C 'reflect-kernel-metric true' -C 'interface tap1 channel 1 link-quality false' &""")
host2.cmd("""babeld -I babel2.pid -S babel-state2 -r -w -h 2 -M 0 -C 'reflect-kernel-metric true' -C 'interface tap2 channel 1 link-quality false' &""")

info( '*** Starting ping loop', '\n' )
host0.cmd("bash -c 'while ! ping 192.168.4.1; do sleep 1; done' &")

# watch routing tables
host0.cmd("""xterm -e "watch -n 0.1 'ip route'" &""")
host1.cmd("""xterm -e "watch -n 0.1 'ip route'" &""")
host2.cmd("""xterm -e "watch -n 0.1 'ip route'" &""")

# uncomment to run wireshark on host
#python_host0.cmd("wireshark -i tap0 -k &")
#python_host1.cmd("wireshark -i tap1 -k &")
#python_host2.cmd("wireshark -i tap2 -k &")

info( '*** Starting INET', '\n' )
user = os.getenv('SAVED_USER')
group = os.getenv('SAVED_GROUP')
saved_path = os.getenv('SAVED_PATH')
saved_env = os.environ.copy()
saved_env["PATH"] = saved_path
# Note: Mininet runs as root, but we want to run the simulation as the normal user. We use 'sudo' to specify the user.
# We also need to restore the original PATH which is overwritten by 'sudo'.
subprocess.run(['sudo -E -u $SAVED_USER bash -c "export PATH=$SAVED_PATH && inet"'], shell=True)

info( '*** Starting Mininet command-line interface', '\n' )
CLI( net )

# delete all network namespaces
subprocess.run(['sudo', 'ip', '-all', 'netns', 'delete'])

net.stop()
