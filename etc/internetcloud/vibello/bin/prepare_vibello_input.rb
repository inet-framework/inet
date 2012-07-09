#!/usr/bin/ruby

require 'fileutils'
require 'bin/database'


# Drop outliers where latency exceeds this value (milliseconds)
MAX_LATENCY=1000

# Minimum number of connections required for each client. Should be at least number of GNP dimensions plus one.
# May be lowered for testing purposes e.g. hand-crafted input files.
MIN_CONNECTIVITY=3

# Generate input files for vibello (computation of GNP coordinates)
def prepare_vibello_input

# Clients that connected to more than two servers
$db.do("DROP TABLE IF EXISTS super")
$db.do("CREATE TABLE super(
 client_ip_id INT UNSIGNED PRIMARY KEY,
 degree INT UNSIGNED NOT NULL )")

#insert into super select client_ip_id, count(distinct server_name) as c from data group by client_ip_id having c>2 order by c asc;
$db.do("INSERT INTO super SELECT client_ip_id, COUNT(DISTINCT server_id) AS c FROM tests #{ MAX_LATENCY && MAX_LATENCY>0 ? "WHERE latency < #{MAX_LATENCY}" : ""} GROUP BY client_ip_id #{ MIN_CONNECTIVITY && MIN_CONNECTIVITY > 1 ? "HAVING c>=#{MIN_CONNECTIVITY} " : "" } ORDER BY c ASC")

# Minimum latency for each client-server-pair 
$db.do("DROP TABLE IF EXISTS min_lat")
$db.do("CREATE TABLE min_lat(
    client_ip_id INT UNSIGNED NOT NULL,
    server_id INT NOT NULL,
    min_latency INT NOT NULL)")


$db.do("INSERT INTO min_lat SELECT client_ip_id, server_id, MIN(latency) FROM super LEFT JOIN tests USING(client_ip_id) GROUP BY client_ip_id, server_id")
#Query OK, 237367 rows affected (7 min 23.64 sec)
#Records: 237367  Duplicates: 0  Warnings: 0

$db.do("DELETE FROM min_lat WHERE min_latency >= 1000")


# All measurents resulting in minimum latencies (min_lat)
$db.do("DROP TABLE IF EXISTS ml_tests")
$db.do("CREATE TABLE ml_tests (
    test_id INT NOT NULL PRIMARY KEY,
    test_date DATETIME NOT NULL,
    client_ip_id INT UNSIGNED NOT NULL,
    download_kbps INT NOT NULL,
    upload_kbps INT NOT NULL,
    latency INT NOT NULL,
    server_id INT NOT NULL,
    user_agent_id INT,
    isp_id INT)")


$db.do("INSERT IGNORE INTO ml_tests 
    SELECT tests.* FROM min_lat LEFT JOIN tests ON(tests.client_ip_id=min_lat.client_ip_id AND tests.server_id=min_lat.server_id AND tests.latency=min_lat.min_latency)")
$db.do("ALTER TABLE ml_tests ADD INDEX(client_ip_id)")

tmpnam=Time.now.to_i.to_s(16)


# Minimum latency for each client
$db.do("DROP TABLE IF EXISTS min_lat2")
$db.do("CREATE TABLE min_lat2(
    client_ip_id INT UNSIGNED PRIMARY KEY,
    min_latency INT NOT NULL)") # in ms

$db.do('INSERT INTO min_lat2 SELECT client_ip_id, MIN(latency) FROM ml_tests GROUP BY client_ip_id')

$db.do("DROP TABLE IF EXISTS access_latencies")
$db.do('CREATE TABLE access_latencies(
    client_ip_id INT UNSIGNED PRIMARY KEY,
    access_latency INT NOT NULL,
    wire_access_latency INT)') # latencies in ms

$db.do('SET @C_FIBRE_M_PER_MS=299792.458/1.5') # Speed of light in fibre, in meters per millisecond
$db.do('SET @M_PER_MILE=1609.344') # meters per mile
$db.do('SET @MS_PER_2MILES=2*@M_PER_MILE/@C_FIBRE_M_PER_MS') # time in milliseconds it takes light to travel 2 miles

$db.do('INSERT IGNORE INTO access_latencies(client_ip_id, access_latency)
   SELECT min_lat2.client_ip_id, min_lat2.min_latency - distance.miles_between * @MS_PER_2MILES
   FROM min_lat2 LEFT JOIN ml_tests ON(ml_tests.client_ip_id=min_lat2.client_ip_id AND ml_tests.latency=min_lat2.min_latency) LEFT JOIN distance ON(ml_tests.client_ip_id=distance.client_ip_id AND ml_tests.server_id=distance.server_id)')

# Discard hosts with negative access latencies
#$db.do('DELETE access_latencies, super, min_lat, ml_tests, min_lat2 FROM access_latencies LEFT JOIN (super, min_lat, ml_tests, min_lat2) ON (access_latencies.client_ip_id=super.client_ip_id AND access_latencies.client_ip_id=min_lat.client_ip_id AND access_latencies.client_ip_id=ml_tests.client_ip_id AND access_latencies.client_ip_id=min_lat2.client_ip_id) WHERE access_latency < 0')
$db.do('DELETE FROM access_latencies WHERE access_latency < 0')
# Delete orphans
$db.do('DELETE super    FROM super    LEFT JOIN access_latencies USING(client_ip_id) WHERE access_latencies.client_ip_id IS NULL')
$db.do('DELETE min_lat  FROM min_lat  LEFT JOIN access_latencies USING(client_ip_id) WHERE access_latencies.client_ip_id IS NULL')
$db.do('DELETE ml_tests FROM ml_tests LEFT JOIN access_latencies USING(client_ip_id) WHERE access_latencies.client_ip_id IS NULL')
$db.do('DELETE min_lat2 FROM min_lat2 LEFT JOIN access_latencies USING(client_ip_id) WHERE access_latencies.client_ip_id IS NULL')

#FileUtils.rm_rf '/tmp/hosts.csv'
$db.do("SELECT access_latencies.client_ip_id, accesspoints.latitude, accesspoints.longitude
  INTO OUTFILE '/tmp/hosts-#{tmpnam}.csv'
  FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '\"'
  LINES TERMINATED BY '\\n'
  FROM access_latencies LEFT JOIN accesspoints USING(client_ip_id)")

#FileUtils.rm_rf '/tmp/latencies-#{tmpnam}.csv'
$db.do("SELECT client_ip_id, server_id, (latency-access_latency) as lat INTO OUTFILE '/tmp/latencies-#{tmpnam}.csv'
  FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '\"'
  LINES TERMINATED BY '\\n'
  FROM ml_tests INNER JOIN access_latencies USING(client_ip_id) HAVING lat>=0")

FileUtils.cp "/tmp/latencies-#{tmpnam}.csv", 'data/latencies.csv'
FileUtils.cp "/tmp/hosts-#{tmpnam}.csv", 'data/hosts.csv'

end

prepare_vibello_input

