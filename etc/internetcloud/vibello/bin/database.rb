require 'yaml'
conf = open('database.conf') {|f| YAML.load(f) }
db=conf['database']
DB_ADAPTER=db['adapter']
DB_DB=db['database']
DB_USER=db['username']
DB_PASSWORD=db['password']

require 'dbi'

$dbh = DBI.connect("DBI:#{DB_ADAPTER}:#{DB_DB}", DB_USER, DB_PASSWORD)

class DB
attr_reader :dbh
def initialize(_dbh)
 @dbh = _dbh
end
def do(stmt)
puts "#{stmt};"
start = Time.new
rows = dbh.do(stmt)
stop = Time.new
puts "# #{rows} rows (#{'%.3f'%(stop-start)} sec)\n\n"
end
end

$db = DB.new($dbh)
