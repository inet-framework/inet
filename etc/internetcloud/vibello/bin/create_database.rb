#!/usr/bin/ruby

require 'yaml'

conf = open('database.conf') {|f| YAML.load(f) }
DB=conf['database']
DB_DB=DB['database']
DB_USER=DB["username"]
DB_PASSWORD=DB["password"]

require 'dbi'

statements = ["DROP DATABASE IF EXISTS #{DB_DB}", "CREATE DATABASE #{DB_DB}", "GRANT ALL ON #{DB_DB}.* TO '#{DB_USER}'@'localhost' IDENTIFIED BY '#{DB_PASSWORD}'", "GRANT FILE ON *.* TO '#{DB_USER}'@'localhost"]

puts "I'd like to run the following statements as db root:\n"
statements.each{|st| puts "   #{st}"}
print "\nDB root password, please:"
DB_ROOT_PW=gets.chomp

$dbh = DBI.connect("DBI:#{DB['adapter']}:", 'root', DB_ROOT_PW) || fail

statements.each{|st| $dbh.do(st) || fail }
