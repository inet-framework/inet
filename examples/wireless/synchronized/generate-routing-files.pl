for ($i = 0; $i < 100; $i++)
{
  open(FILE, ">node$i.mrt");
  print FILE "ifconfig:

# interface 0 to router
name: wlan0  inet_addr: 172.0.0.$i   MTU: 1500
Groups: 224.0.0.1

ifconfigend.

route:

default:        *      0.0.0.0         G   0   wlan0
224.0.0.0       *      255.0.0.0       G   0   wlan0

routeend.
";
  close(FILE);
}