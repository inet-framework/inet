///
/// @file   csqliteoutputvectormgr.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2012-12-05
///
/// @brief  Implements cSQLiteOutputVectorManager class that writes output vectors
///         into SQLite databases.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#include <assert.h>
#include "csqliteoutputvectormgr.h"
#include "fileoutvectormgr.h"
//#include "oppsqliteutils.h"


// define prepared SQL statements
#define SQL_INSERT_VRUN     "INSERT INTO vrun(runnumber,network) VALUES(?,?)"
#define SQL_INSERT_VECTOR   "INSERT INTO vector(runid,module,name) VALUES(?,?,?)"
#define SQL_INSERT_VECATTR  "INSERT INTO vecattr(vectorid,name,value) VALUES(?,?,?)"
#define SQL_INSERT_VECDATA  "INSERT INTO vecdata(vectorid,time,value) VALUES(?,?,?)"


Register_Class(cSQLiteOutputVectorManager);


Register_GlobalConfigOption(CFGID_SQLITEOUTVECTORMGR_CONNECTIONNAME, "sqliteoutputvectormanager-connectionname", CFG_STRING, "\"sqlite\"", "Object name of database connection parameters, default='sqlite'");
Register_GlobalConfigOption(CFGID_SQLITEOUTVECTORMGR_COMMIT_FREQ, "sqliteoutputvectormanager-commit-freq", CFG_INT, "50", "COMMIT every n INSERTs, default=50");


Register_PerObjectConfigOption(CFGID_SQLITE_VECTOR_DB, "sqlite-vector-database", KIND_NONE, CFG_STRING, "\"\"", "Output vector database name");
//Register_PerObjectConfigOption(CFGID_SQLITE_OPT_FILE,   "sqlite-options-file", KIND_NONE, CFG_FILENAME, "",      "Options file for sqlite server");


cSQLiteOutputVectorManager::cSQLiteOutputVectorManager()
{
    db = NULL;
    insertVectorStmt = insertVecdataStmt = NULL;
}

cSQLiteOutputVectorManager::~cSQLiteOutputVectorManager()
{
    if (db)
        closeDB();
}

void cSQLiteOutputVectorManager::openDB()
{
    EV << getClassName() << " connecting to an SQLite database ...";

    std::string cfgobj = ev.getConfig()->getAsString(CFGID_SQLITEOUTVECTORMGR_CONNECTIONNAME);
    if (cfgobj.empty())
        cfgobj = "sqlite";

    std::string db_name = (ev.getConfig())->getAsString(cfgobj.c_str(), CFGID_SQLITE_VECTOR_DB, NULL);

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
        throw cRuntimeError("Sqlite error: Failed to connect to database: %s", sqlite3_errmsg(db));

    EV << " OK\n";

    commitFreq = ev.getConfig()->getAsInt(CFGID_SQLITEOUTVECTORMGR_COMMIT_FREQ);
    insertCount = 0;

    // prepare SQL statements; note that SQLite doesn't support 'binding variables'
    if (sqlite3_prepare_v2(db, SQL_INSERT_VECTOR, strlen(SQL_INSERT_VECTOR)+1, &insertVectorStmt, NULL) != SQLITE_OK)
        throw cRuntimeError("SQLite error: prepare statement for 'insertVectorStmt' failed: %s", sqlite3_errmsg(db));
    if (sqlite3_prepare_v2(db, SQL_INSERT_VECATTR, strlen(SQL_INSERT_VECATTR)+1, &insertVecAttrStmt, NULL) != SQLITE_OK)
        throw cRuntimeError("SQLite error: prepare statement for 'insertVecAttrStmt' failed: %s", sqlite3_errmsg(db));
    if (sqlite3_prepare_v2(db, SQL_INSERT_VECDATA, strlen(SQL_INSERT_VECDATA)+1, &insertVecdataStmt, NULL) != SQLITE_OK)
        throw cRuntimeError("SQLite error: prepare statement for 'insertVecdataStmt' failed: %s", sqlite3_errmsg(db));

    // optimize the DB performance
    char *errMsg;
    sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &errMsg);
}

void cSQLiteOutputVectorManager::closeDB()
{
    if (insertVectorStmt) sqlite3_finalize(insertVectorStmt);
    if (insertVecAttrStmt) sqlite3_finalize(insertVecAttrStmt);
    if (insertVecdataStmt) sqlite3_finalize(insertVecdataStmt);
    insertVectorStmt = insertVecdataStmt = insertVecAttrStmt = NULL;

    if (db) sqlite3_close(db);
    db = NULL;
}

void cSQLiteOutputVectorManager::beginTransaction()
{
    char *errMsg = NULL;
    
    if (sqlite3_exec(db, "BEGIN", NULL, NULL, &errMsg) != SQLITE_OK)
        throw cRuntimeError("SQLite error: BEGIN failed: %s", errMsg);
}

void cSQLiteOutputVectorManager::endTransaction()
{
    char *errMsg = NULL;
    
    if (sqlite3_exec(db, "COMMIT", NULL, NULL, &errMsg) != SQLITE_OK)
        throw cRuntimeError("SQLite error: COMMIT failed: %s", errMsg);
}

void cSQLiteOutputVectorManager::initVector(sVectorData *vp)
{
    if (!initialized)
        insertRunIntoDB();

    // fill in prepared statement parameters, and fire off the statement
    sqlite3_bind_int(insertVectorStmt, 1, runId);
    sqlite3_bind_text(insertVectorStmt, 2, vp->modulename.c_str(), strlen(vp->modulename.c_str())+1, NULL);
    sqlite3_bind_text(insertVectorStmt, 3, vp->vectorname.c_str(), strlen(vp->vectorname.c_str())+1, NULL);
    if (sqlite3_step(insertVectorStmt) != SQLITE_DONE)
        throw cRuntimeError("SQLite error: INSERT failed with 'insertVectorStmt': %s", sqlite3_errmsg(db));
    sqlite3_reset(insertVectorStmt);    

    // query the automatic vectorid from the newly inserted row
    // Note: this INSERT must not be DELAYED, otherwise we get zero here.
    vp->id = long(sqlite3_last_insert_rowid(db));

    // write attributes:
    sqlite3_bind_int(insertVecAttrStmt, 1, vp->id);
    for (opp_string_map::iterator it=vp->attributes.begin(); it != vp->attributes.end(); ++it)
    {
        sqlite3_bind_text(insertVecAttrStmt, 2, it->first.c_str(), strlen(it->first.c_str())+1, NULL);
        sqlite3_bind_text(insertVecAttrStmt, 3, it->second.c_str(), strlen(it->second.c_str())+1, NULL);
        if (sqlite3_step(insertVecAttrStmt) != SQLITE_DONE)
            throw cRuntimeError("SQLite error: INSERT failed with 'insertVecAttrStmt': %s", sqlite3_errmsg(db));
        sqlite3_reset(insertVecAttrStmt);
    }

    vp->initialised = true;
}

void cSQLiteOutputVectorManager::startRun()
{
    // clean up file from previous runs
    if (db)
        closeDB();
    openDB();
    initialized = false;
}

void cSQLiteOutputVectorManager::endRun()
{
    if (db)
    {
        endTransaction();
        closeDB();
    }
}

void cSQLiteOutputVectorManager::insertRunIntoDB()
{
    if (!initialized)
    {   // insert run into the database
        sqlite3_stmt *stmt;

        sqlite3_prepare_v2(db, SQL_INSERT_VRUN, strlen(SQL_INSERT_VRUN)+1, &stmt, NULL);
        int runNumber = simulation.getActiveEnvir()->getConfigEx()->getActiveRunNumber();
        sqlite3_bind_int(stmt, 1, runNumber);
        std::string networkName = simulation.getNetworkType()->getName();
        sqlite3_bind_text(stmt, 2, networkName.c_str(), networkName.length()+1, NULL);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw cRuntimeError("SQLite error: INSERT failed for 'vrun' table: %s", sqlite3_errmsg(db));
        sqlite3_reset(stmt);

        // query the automatic runid from the newly inserted row
        runId = long(sqlite3_last_insert_rowid(db));

        // begin transaction for faster inserts
        beginTransaction();
        
        initialized = true;
    }
}

void *cSQLiteOutputVectorManager::registerVector(const char *modulename, const char *vectorname)
{
    // only create the data structure here -- we'll insert the entry into the
    // DB lazily, when first data gets written
    sVectorData *vp = createVectorData();
    vp->id = -1; // we'll get it from the database
    vp->initialised = false;
    vp->modulename = modulename;
    vp->vectorname = vectorname;

    cFileOutputVectorManager::getOutVectorConfig(modulename, vectorname,
                                                 vp->enabled, vp->recordEventNumbers, vp->intervals); //FIXME...
    return vp;
}

cSQLiteOutputVectorManager::sVectorData *cSQLiteOutputVectorManager::createVectorData()
{
    return new sVectorData;
}

void cSQLiteOutputVectorManager::deregisterVector(void *vectorhandle)
{
    sVectorData *vp = (sVectorData *)vectorhandle;
    delete vp;
}

void cSQLiteOutputVectorManager::setVectorAttribute(void *vectorhandle, const char *name, const char *value)
{
    ASSERT(vectorhandle != NULL);
    sVectorData *vp = (sVectorData *)vectorhandle;
    vp->attributes[name] = value;
}

bool cSQLiteOutputVectorManager::record(void *vectorhandle, simtime_t t, double value)
{
    sVectorData *vp = (sVectorData *)vectorhandle;

    if (!vp->enabled)
        return false;

    if (vp->intervals.contains(t))
    {
        if (!vp->initialised)
            initVector(vp);

        // fill in prepared statement parameters, and fire off the statement
        sqlite3_bind_int(insertVecdataStmt, 1, vp->id);
        sqlite3_bind_double(insertVecdataStmt, 2, SIMTIME_DBL(t));
        sqlite3_bind_double(insertVecdataStmt, 3, value);
        if (sqlite3_step(insertVecdataStmt) != SQLITE_DONE)
            throw cRuntimeError("SQLite error: INSERT failed with 'insertVecdataStmt': %s", sqlite3_errmsg(db));
        sqlite3_reset(insertVecdataStmt);

        // commit every once in a while
        if (++insertCount == commitFreq)
        {
            insertCount = 0;
            endTransaction();
            beginTransaction();
        }

        return true;
    }
    return false;
}

void cSQLiteOutputVectorManager::flush()
{
    endTransaction();
}

