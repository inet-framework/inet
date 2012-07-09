#!/usr/bin/ruby

require 'tmpdir'
require 'bin/database'
TEMPDIR="#{Dir.tmpdir}/#{File.basename($0)}-#{Time.now.to_i}"

SED='1  s/\(.*\)/INSERT INTO `data` VALUES (\1),/
$  { s/\(.*\)/(\1);/; q }
1! s/\(.*\)/(\1),/'

# Load CSV file from ookla into database
def load(source)
$db.do("DROP TABLE IF EXISTS data")
$db.do("CREATE TABLE data (
    test_id INT NOT NULL PRIMARY KEY,
    test_date DATETIME NOT NULL,
    client_ip_id INT UNSIGNED NOT NULL,
    download_kbps INT NOT NULL,
    upload_kbps INT NOT NULL,
    latency INT NOT NULL,
    server_name VARCHAR(30),
    server_country VARCHAR(30),
    server_country_code VARCHAR(2),
    server_latitude DOUBLE,
    server_longitude DOUBLE,
    user_agent VARCHAR(120),
    isp_name VARCHAR(60),
    client_net_speed TINYINT,
    is_isp TINYINT,
    client_country VARCHAR(40),
    client_country_code VARCHAR(2),
    client_region_name VARCHAR(60),
    client_region_code VARCHAR(2),
    client_city VARCHAR(60),
    client_latitude DOUBLE,
    client_longitude DOUBLE,
    miles_between DOUBLE)")

#SED='1  s/\(.*\)/INSERT INTO `data` (\1) VALUES/
#$  { s/\(.*\)/(\1);/; q }
#1! s/\(.*\)/(\1),/'

#system("iconv -f ISO_8859-1 -t UTF-8 #{DATAFILE} | sed '#{SED}' | mysql -u #{DB_USER} '--password=#{DB_PASSWORD}' #{DB_DB}") || abort


FileUtils.mkdir(TEMPDIR) && FileUtils.chdir(TEMPDIR) do
puts "Splitting CSV..."
system("split --verbose -l 50000 #{source}") || abort
system("tail -n +2 xaa >xaa2") || abort
system("mv xaa2 xaa") || abort
Dir.glob("x*").sort.each do |file|
puts "Loading part #{file}"
system("iconv -f ISO_8859-1 -t UTF-8 #{file} | sed '#{SED}' | mysql -u #{DB_USER} '--password=#{DB_PASSWORD}' #{DB_DB}") || abort
end
end

$db.do("ALTER TABLE data ADD INDEX(client_ip_id)")
end

# Normalize ookla db schema
def normalize
$db.do("DROP TABLE IF EXISTS useragents")
$db.do("CREATE TABLE useragents (
    id INT PRIMARY KEY AUTO_INCREMENT,
    user_agent VARCHAR(120) UNIQUE KEY NOT NULL)")
$db.do("INSERT INTO useragents (user_agent) SELECT DISTINCT(user_agent) FROM data ORDER BY user_agent ASC")

$db.do("DROP TABLE IF EXISTS countries")
$db.do("CREATE TABLE countries (
    country_code VARCHAR(2) PRIMARY KEY,
    country_name VARCHAR(40) NOT NULL)")
$db.do("INSERT INTO countries SELECT DISTINCTROW client_country_code, client_country FROM data ORDER BY client_country_code ASC")

#$db.do("DROP TABLE IF EXISTS regions")
#$db.do("CREATE TABLE regions (
#    region_code VARCHAR(2) PRIMARY KEY,
#    region_name VARCHAR(60) NOT NULL)")
#$db.do("INSERT INTO regions SELECT DISTINCTROW client_region_code, client_region_name FROM data ORDER BY client_region_code ASC")

$db.do("DROP TABLE IF EXISTS isps")
$db.do("CREATE TABLE isps (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(60) UNIQUE KEY NOT NULL)")
$db.do("INSERT INTO isps (name) SELECT DISTINCT(isp_name) FROM data ORDER BY isp_name ASC")

$db.do("DROP TABLE IF EXISTS accesspoints")
$db.do("CREATE TABLE accesspoints (
    client_ip_id INT UNSIGNED PRIMARY KEY,
    isp_id INT,
    net_speed TINYINT,
    is_isp TINYINT,
    country_code VARCHAR(2),
    region_name VARCHAR(60),
    region_code VARCHAR(2),
    city VARCHAR(60),
    latitude DOUBLE,
    longitude DOUBLE)")

$db.do("INSERT INTO accesspoints SELECT client_ip_id, isps.id, client_net_speed, is_isp, client_country_code, client_region_name, client_region_code, client_city, client_latitude, client_longitude FROM data LEFT JOIN isps ON(isp_name=isps.name) GROUP BY client_ip_id")

$db.do("DROP TABLE IF EXISTS servers")
$db.do("CREATE TABLE servers (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(30),
    country_code VARCHAR(2),
    latitude DOUBLE,
    longitude DOUBLE)")
$db.do("ALTER TABLE servers ADD INDEX(name)")

$db.do("INSERT INTO servers (name, country_code, latitude, longitude) SELECT DISTINCTROW server_name, server_country_code, server_latitude, server_longitude FROM data ORDER BY server_name ASC")
#$db.do("INSERT INTO servers (name, country_code, latitude, longitude) SELECT DISTINCTROW server_name, server_country_code, server_latitude, server_longitude FROM data GROUP BY server_name ASC")

$db.do("DROP TABLE IF EXISTS tests")
$db.do("CREATE TABLE tests (
    test_id INT NOT NULL PRIMARY KEY,
    test_date DATETIME NOT NULL,
    client_ip_id INT UNSIGNED NOT NULL,
    download_kbps INT NOT NULL,
    upload_kbps INT NOT NULL,
    latency INT NOT NULL,
    server_id INT NOT NULL,
    user_agent_id INT,
    isp_id INT)")
#$db.do("INSERT INTO tests SELECT DISTINCTROW test_id, test_date, client_ip_id, download_kbps, upload_kbps, latency, servers.id, useragents.id, isps.id FROM data LEFT JOIN servers ON (data.server_name=servers.name AND data.server_country_code=servers.country_code AND data.server_latitude = servers.latitude AND data.server_longitude = servers.longitude) LEFT JOIN useragents ON data.user_agent=useragents.user_agent LEFT JOIN isps ON data.isp_name=isps.name ORDER BY server_country_code, server_name ASC")
$db.do("INSERT INTO tests SELECT test_id, test_date, client_ip_id, download_kbps, upload_kbps, latency, servers.id, useragents.id, isps.id FROM data LEFT JOIN servers ON (data.server_name=servers.name AND data.server_country_code=servers.country_code AND data.server_latitude = servers.latitude AND data.server_longitude = servers.longitude) LEFT JOIN useragents ON data.user_agent=useragents.user_agent LEFT JOIN isps ON data.isp_name=isps.name ORDER BY server_country_code, server_name ASC")

$db.do("ALTER TABLE tests ADD INDEX(client_ip_id)")

$db.do("DROP TABLE IF EXISTS distance")
$db.do("CREATE TABLE distance (
    client_ip_id INT UNSIGNED NOT NULL,
    server_id INT NOT NULL,
    miles_between DOUBLE,
    PRIMARY KEY (client_ip_id, server_id))")

$db.do("INSERT INTO distance SELECT DISTINCTROW client_ip_id, servers.id, miles_between FROM data LEFT JOIN servers ON (data.server_name=servers.name AND data.server_country_code=servers.country_code AND data.server_latitude = servers.latitude AND data.server_longitude = servers.longitude)")

end


case ARGV[0]
 when 'load'
  load(ARGV[1])
 when 'normalize'
  normalize
 else
  puts "Unknown command"
  exit 1
end

