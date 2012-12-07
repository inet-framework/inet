///
/// @file   csqliteoutputscalarmgr.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2012-12-07
///
/// @brief  Implements cSQLiteOutputScalarManager class that writes output scalars
///         into SQLite databases.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#include <assert.h>
#include "csqliteoutputscalarmgr.h"
//#include "oppsqliteutils.h"


// define prepared SQL statements
#define SQL_INSERT_SRUN     "INSERT INTO srun(runnumber,network) VALUES(?,?)"
#define SQL_INSERT_SCALAR  "INSERT INTO scalar(runid,module,name,value) VALUES(?,?,?,?)"


Register_Class(cSQLiteOutputScalarManager);


Register_GlobalConfigOption(CFGID_SQLITEOUTSCALARMGR_CONNECTIONNAME, "sqliteoutputscalarmanager-connectionname", CFG_STRING, "\"sqlite\"", "Object name of database connection parameters, default='sqlite'");
Register_GlobalConfigOption(CFGID_SQLITEOUTSCALARMGR_COMMIT_FREQ, "sqliteoutputscalarmanager-commit-freq", CFG_INT,  "10", "COMMIT every n INSERTs, default=10");


Register_PerObjectConfigOption(CFGID_SQLITE_SCALAR_DB, "sqlite-scalar-database", KIND_NONE, CFG_STRING, "\"\"", "Output scalar database name");


cSQLiteOutputScalarManager::cSQLiteOutputScalarManager()
{
    db = NULL;
    insertScalarStmt = NULL;
}

cSQLiteOutputScalarManager::~cSQLiteOutputScalarManager()
{
    if (db)
        closeDB();
}

void cSQLiteOutputScalarManager::openDB()
{
    EV << getClassName() << " connecting to SQLite database...";

    std::string cfgobj = ev.getConfig()->getAsString(CFGID_SQLITEOUTSCALARMGR_CONNECTIONNAME);
    if (cfgobj.empty())
        cfgobj = "sqlite";

    std::string db_name = (ev.getConfig())->getAsString(cfgobj.c_str(), CFGID_SQLITE_SCALAR_DB, NULL);

    // TODO: Implement option processing
    // unsigned int clientflag = (unsigned int) cfg->getAsInt(cfgobj, CFGID_SQLITE_CLIENTFLAG, 0);
    // bool usepipe = cfg->getAsBool(cfgobj, CFGID_SQLITE_USE_PIPE, false);
    // std::string optionsfile = cfg->getAsFilename(cfgobj, CFGID_SQLITE_OPT_FILE);

    // // set options, then connect
    // if (!optionsfile.empty())
    //     sqlite_options(db, SQLITE_READ_DEFAULT_FILE, optionsfile.c_str());
    // if (usepipe)
    //     sqlite_options(db, SQLITE_OPT_NAMED_PIPE, 0);

    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK)
        throw cRuntimeError("SQLite error: Failed to connect to database: %s", sqlite3_errmsg(db));

    EV << " OK\n";

    commitFreq = ev.getConfig()->getAsInt(CFGID_SQLITEOUTSCALARMGR_COMMIT_FREQ);
    insertCount = 0;

    // prepare SQL statements; note that SQLite doesn't support 'binding variables'
    if (sqlite3_prepare_v2(db, SQL_INSERT_SCALAR, strlen(SQL_INSERT_SCALAR)+1, &insertScalarStmt, NULL) != SQLITE_OK)
        throw cRuntimeError("SQLite error: prepare statement for 'insertScalarStmt' failed: %s", sqlite3_errmsg(db));

    // optimize the DB performance
    char *errMsg;
    sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &errMsg);
}

void cSQLiteOutputScalarManager::closeDB()
{
    if (insertScalarStmt) sqlite3_finalize(insertScalarStmt);
    insertScalarStmt = NULL;

    if (db) sqlite3_close(db);
    db = NULL;
}

void cSQLiteOutputScalarManager::beginTransaction()
{
    char *errMsg = NULL;
    
    if (sqlite3_exec(db, "BEGIN", NULL, NULL, &errMsg) != SQLITE_OK)
        throw cRuntimeError("SQLite error: BEGIN failed: %s", errMsg);
}

void cSQLiteOutputScalarManager::endTransaction()
{
    char *errMsg = NULL;
    
    if (sqlite3_exec(db, "COMMIT", NULL, NULL, &errMsg) != SQLITE_OK)
        throw cRuntimeError("SQLite error: COMMIT failed: %s", errMsg);
}

void cSQLiteOutputScalarManager::startRun()
{
    // clean up file from previous runs
    if (db)
        closeDB();
    openDB();
    initialized = false;
    runId = -1;
}

void cSQLiteOutputScalarManager::endRun()
{
    if (db)
    {
        endTransaction();
        closeDB();
    }
}

void cSQLiteOutputScalarManager::insertRunIntoDB()
{
    if (!initialized)
    {
        // insert run into the database
        sqlite3_stmt *stmt;

        sqlite3_prepare_v2(db, SQL_INSERT_SRUN, strlen(SQL_INSERT_SRUN)+1, &stmt, NULL);
        int runNumber = simulation.getActiveEnvir()->getConfigEx()->getActiveRunNumber();
        sqlite3_bind_int(stmt, 1, runNumber);
        std::string networkName = simulation.getNetworkType()->getName();
        sqlite3_bind_text(stmt, 2, networkName.c_str(), networkName.length()+1, NULL);        

        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw cRuntimeError("SQLite error: INSERT failed for 'srun' table: %s", sqlite3_errmsg(db));
        sqlite3_reset(stmt);

        // query the automatic runid from the newly inserted row
        runId = long(sqlite3_last_insert_rowid(db));

        // begin transaction for faster inserts
        beginTransaction();

        initialized = true;
    }
}

void cSQLiteOutputScalarManager::recordScalar(cComponent *component, const char *name, double value, opp_string_map *attributes)
{
    if (!initialized)
        insertRunIntoDB();

    // fill in prepared statement parameters, and fire off the statement
    sqlite3_bind_int(insertScalarStmt, 1, runId);
    sqlite3_bind_text(insertScalarStmt, 2, component->getFullPath().c_str(), strlen(component->getFullPath().c_str())+1, NULL);
    sqlite3_bind_text(insertScalarStmt, 3, name, strlen(name)+1, NULL);    
    sqlite3_bind_double(insertScalarStmt, 4, value);
    if (sqlite3_step(insertScalarStmt) != SQLITE_DONE)
        throw cRuntimeError("SQLite error: INSERT failed with 'insertScalarStmt': %s", sqlite3_errmsg(db));
    sqlite3_reset(insertScalarStmt);

    // commit every once in a while
    if (++insertCount == commitFreq)
    {
        insertCount = 0;
        endTransaction();
        beginTransaction();
    }
}

void cSQLiteOutputScalarManager::recordStatistic(cComponent *component, const char *name, cStatistic *statistic, opp_string_map *attributes)
{
    throw cRuntimeError("cSQLiteOutputScalarManager: recording cStatistics objects not supported yet");
}

void cSQLiteOutputScalarManager::flush()
{
    endTransaction();
}
