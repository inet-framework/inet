#!/usr/bin/ruby

require 'bin/database'
require 'fileutils'
SED='1  s/\(.*\)/INSERT INTO `gnp_coords` VALUES /
$  { s/\(.*\)/(\1);/; q }
1! s/\(.*\)/(\1),/'

# Generate input files for vibello (computation of GNP coordinates)
def load_vibello_results

dimensions=`head -n 1 data/ehosts.csv`.count(',')

$db.do("DROP TABLE IF EXISTS gnp_coords")
stmt="CREATE TABLE gnp_coords(
  client_ip_id INT UNSIGNED PRIMARY KEY"
(0...dimensions).each{|d| stmt << ",\n  c#{d} DOUBLE NOT NULL"  }
stmt << ")"

$db.do(stmt)

system("sed '#{SED}' data/ehosts.csv | mysql -u #{DB_USER} '--password=#{DB_PASSWORD}' #{DB_DB}") || abort

end

load_vibello_results

