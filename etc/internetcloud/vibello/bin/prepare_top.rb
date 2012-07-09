#!/usr/bin/ruby

require 'bin/database'

def top

$db.do("DROP TABLE IF EXISTS top")
$db.do("CREATE TABLE top(
  client_ip_id INT UNSIGNED PRIMARY KEY,
  download_kbps INT NOT NULL,
  upload_kbps INT NOT NULL,
  latency INT NOT NULL
)")

$db.do("INSERT INTO top
  SELECT client_ip_id, MAX(download_kbps), MAX(upload_kbps), MIN(latency) FROM data GROUP BY client_ip_id")


FileUtils.rm_rf '/tmp/bandwidths.csv'
$db.do("SELECT 'client_ip_id', 'client_country_code', 'is_isp', 'upload_kbps', 'download_kbps', 'latency'
UNION
SELECT client_ip_id, client_country_code, is_isp, MAX(upload_kbps), MAX(download_kbps), MIN(latency)
  INTO OUTFILE '/tmp/bandwidths.csv'
    FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '\"' LINES TERMINATED BY '\\n'
FROM data GROUP BY client_ip_id")


$db.do("CREATE TABLE closest_server(
client_ip_id INT UNSIGNED PRIMARY KEY,
miles_between DOUBLE NOT NULL
)")

$db.do("INSERT INTO closest_server SELECT client_ip_id, MIN(miles_between) FROM super LEFT JOIN data USING (client_ip_id) GROUP BY client_ip_id")
#Query OK, 56780 rows affected (4 min 43.62 sec)

$db.do("ALTER TABLE closest_server
  ADD COLUMN server_name VARCHAR(20),
  ADD COLUMN download_kbps INT,
  ADD COLUMN upload_kbps INT")

$db.do("UPDATE closest_server LEFT JOIN data USING(client_ip_id, miles_between) SET closest_server.server_name = data.server_name, closest_server.download_kbps = data.download_kbps, closest_server.upload_kbps = data.upload_kbps")

FileUtils.rm_rf '/tmp/bandwidth_distance.csv'
$db.do("SELECT 'down','up','miles','cs_down','cs_up','cs_miles', 'max_up'
UNION
SELECT d.download_kbps, d.upload_kbps, d.miles_between, cs.download_kbps, cs.upload_kbps, cs.miles_between, top.upload_kbps
INTO OUTFILE '/tmp/bandwidth_distance.csv'
  FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '\"' LINES TERMINATED BY '\\n'
 FROM closest_server AS cs LEFT JOIN data AS d USING(client_ip_id) LEFT JOIN top USING(client_ip_id) WHERE d.server_name <> cs.server_name")

end


top
