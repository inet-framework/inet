#!/usr/bin/ruby

require 'bin/database'
require 'builder'
require 'fileutils'


def find_max_bw
puts "Determining maximum bandwidths for each client..."
$db.do("DROP TABLE IF EXISTS max_bw")
$db.do("CREATE TABLE max_bw (
     client_ip_id INT UNSIGNED PRIMARY KEY,
     download_kbps INT NOT NULL,
     upload_kbps INT NOT NULL)")
$db.do("INSERT INTO max_bw SELECT client_ip_id, MAX(download_kbps), MAX(upload_kbps) FROM access_latencies LEFT JOIN tests USING(client_ip_id) GROUP BY client_ip_id")
end

def compute_wire_access_latencies
puts "Determining wire access latencies..."

$db.do('SET @REQ_SIZE=477*8.0') # Bit size of a HTTP request packet
$db.do('SET @REP_SIZE=453*8.0') # Bit size of a HTTP reply packet

# Transmission delays are substracted from total access latency. kbps = bpms
$db.do("UPDATE access_latencies LEFT JOIN max_bw USING(client_ip_id) SET wire_access_latency=access_latency-@REQ_SIZE/(upload_kbps)-@REP_SIZE/(download_kbps)");

end

def write_xml

File.open("data/measured_data.xml", "w") do |file|
x = Builder::XmlMarkup.new(:target => file, :indent => 1)
x.instruct!
x.gnp {
x.GroupLookup {

# [[scheme name, outer select, inner select],...]
[
[ 'pinger_region', 'SELECT DISTINCT pinger_region FROM country_lookup INNER JOIN accesspoints USING(country_code) INNER JOIN gnp_coords USING(client_ip_id) ORDER BY pinger_region ASC', 'SELECT client_ip_id FROM country_lookup LEFT JOIN accesspoints USING(country_code) INNER JOIN gnp_coords USING(client_ip_id) WHERE pinger_region = ?' ],
[ 'country_code', 'SELECT DISTINCT country_code FROM accesspoints INNER JOIN gnp_coords USING(client_ip_id) ORDER BY country_code ASC', 'SELECT client_ip_id FROM accesspoints INNER JOIN gnp_coords USING(client_ip_id) WHERE country_code = ?' ],
#[ 'country_name', 'SELECT DISTINCT country_name FROM countries INNER JOIN accesspoints USING(country_code) INNER JOIN gnp_coords USING(client_ip_id) ORDER BY country_name ASC', 'SELECT client_ip_id FROM countries LEFT JOIN accesspoints USING(country_code) INNER JOIN gnp_coords USING(client_ip_id) WHERE country_name = ?' ],
[ 'region_name', 'SELECT DISTINCT region_name FROM accesspoints INNER JOIN gnp_coords USING(client_ip_id) ORDER BY region_name ASC', 'SELECT client_ip_id FROM accesspoints INNER JOIN gnp_coords USING(client_ip_id) WHERE region_name = ?' ]
].each do |scheme|
name, outer, inner = scheme[0], scheme[1], scheme[2]
puts "Writing GroupLookup by #{name}..."

 psth = $dbh.prepare(inner)
 $dbh.execute(outer) do |sth|
 sth.fetch_array do |row|
 group = row[0]
 p group
 psth.execute(group)
 ips = psth.fetch_all
x.Group(:id=>group, :maxsize=>ips.size) {

 ips.each_slice(1000) do |slice|
x.IPs(:value=>slice.join(','))
end
}
end
end
end

}

puts "Writing Hosts..."
x.Hosts {
 $dbh.execute("SELECT * FROM access_latencies LEFT JOIN accesspoints USING(client_ip_id) LEFT JOIN country_lookup USING(country_code) LEFT JOIN gnp_coords USING(client_ip_id) LEFT JOIN max_bw USING(client_ip_id)") do |sth|
 sth.fetch_hash do |row|
 coords = [] 
 d=0
 loop do
 c = row["c#{d}"]
 break unless c
 coords << c
 d += 1
 end

 # Latencies in PeerfactSim.KOM/gnplib are in ms; bandwidths are in bytes/s !
 x.Host(:ip=>row['client_ip_id'], :continentalArea=>row['pinger_region'], :countryCode=>row['country_code'], :region=>row['region_name'], :city=>row['city'], :isp=>row['is_isp'], :longitude=>row['longitude'], :latitude=>row['latitude'], :coordinates=>coords.join(','), :accessDelay=>row['wire_access_latency'], :bw_upstream=>row['upload_kbps'].to_f*(1000.0/8.0), :bw_downstream=>row['download_kbps'].to_f*(1000.0/8.0))
end
end
}

puts "Writing PingERLookup..."
x.PingErLookup {
 $dbh.execute("SELECT * FROM pinger_lookup ORDER BY from_region, to_region ASC") do |sth|
 sth.fetch_hash do |row|
  x.SummaryReport(:from=>row['from_region'], :to=>row['to_region'], :minimumRtt=>row['min_rtt'], :averageRtt=>row['avg_rtt'], :delayVariation=>row['delay_variation'], :packetLoss=>row['packet_loss'])
end
end
}

puts "Writing CountryLookup..."
x.CountryLookup {
 $dbh.execute("SELECT * FROM country_lookup LEFT JOIN countries USING(country_code) ORDER BY country_code ASC") do |sth|
 sth.fetch_hash do |row|
  x.CountryKey(:code=>row['country_code'], :countryGeoIP=>row['country_name'], :countryPingEr=>row['country_name'], :regionPingEr=>row['pinger_region'])
end
end
}

}
end

end

case ARGV[0]
 when 'find_max_bw'
  find_max_bw
 when 'compute_wire_access_latencies'
  compute_wire_access_latencies
 when 'write_xml'
  write_xml
 else
  puts "Unknown command"
  exit 1
end

