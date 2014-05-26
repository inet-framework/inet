///
/// @file   csqliteoutputvectormgr.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2012-12-05
///
/// @brief  Declares cSQLiteOutputVectorManager class that writes output vectors
///         into SQLite databases.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __SQLITEOUTPUTVECTORMGR_H
#define __SQLITEOUTPUTVECTORMGR_H

#include <sqlite3.h>
#include <omnetpp.h>
#include "intervals.h"


/**
 * An Output Vector Manager that writes into an SQLite database instead of
 * a .vec file.
 *
 * It expects to find two tables in the database: VECTOR and VECDATA.
 * VECTOR corresponds to the "vector" lines in the normal .vec files,
 * and VECDATA to the data lines.
 *
 * <pre>
 * CREATE TABLE vrun (
 *      id INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
 *      runnumber INT NOT NULL,
 *      network VARCHAR(80) NOT NULL,
 *      date TIMESTAMP NOT NULL
 * );
 *
 * CREATE TABLE vector (
 *      id INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
 *      runid INT NOT NULL,
 *      module VARCHAR(200) NOT NULL,
 *      name VARCHAR(80) NOT NULL,
 *      FOREIGN KEY (runid) REFERENCES vrun(id)
 * );
 *
 * CREATE TABLE vecattr (
 *      vectorid INT NOT NULL,
 *      name VARCHAR(200) NOT NULL,
 *      value VARCHAR(200) NOT NULL,
 *      FOREIGN KEY (vectorid) REFERENCES vector(id)
 * );
 *
 * CREATE TABLE vecdata (
 *      vectorid INT NOT NULL,
 *      time DOUBLE PRECISION NOT NULL,
 *      value DOUBLE PRECISION NOT NULL,
 *      FOREIGN KEY (vectorid) REFERENCES vector(id)
 * ) ENGINE = MYISAM;
 * </pre>
 *
 * Additional config parameters:
 *
 * <pre>
 * [General]
 * sqliteoutputvectormanager-commit-freq = <integer> # COMMIT every n INSERTs, default=50
 * sqliteoutputscalarmanager-connectionname = <string> # look for connect parameters in the given object
 * </pre>
 *
 * @ingroup Envir
 */
class cSQLiteOutputVectorManager : public cOutputVectorManager
{
  protected:
    struct sVectorData {
       long id;             // vector ID
       opp_string modulename; // module of cOutVector object
       opp_string vectorname; // cOutVector object name
       opp_string_map attributes; // vector attributes
       bool initialised;    // true if the "label" line is already written out
       bool enabled;        // write to the output file can be enabled/disabled
       bool recordEventNumbers;  // write to the output file can be enabled/disabled
       Intervals intervals; // write begins at starttime
    };

    // the database connection
    sqlite3 *db;

    // database id (vrun.id) of current run
    long runId;

    bool initialized;

    // we COMMIT after every commitFreq INSERT statements
    int commitFreq;
    int insertCount;

    // prepared statements and their parameter bindings
    sqlite3_stmt *insertVectorStmt;
    /* SQLITE_BIND insVectorBind[3]; */
    sqlite3_stmt *insertVecAttrStmt;
    /* SQLITE_BIND insVecAttrBind[3]; */
    sqlite3_stmt *insertVecdataStmt;
    /* SQLITE_BIND insVecdataBind[3]; */

    // these variables are bound to the above bind parameters
    int runidBuf;
    char moduleBuf[201];
    unsigned long moduleLength;
    char nameBuf[81];
    unsigned long nameLength;
    int vectoridBuf;
    double timeBuf;
    double valueBuf;
    char vecAttrNameBuf[201];
    unsigned long vecAttrNameLength;
    char vecAttrValueBuf[201];
    unsigned long vecAttrValueLength;

  protected:
    void openDB();
    void closeDB();
    void beginTransaction();
    void endTransaction();
    void insertRunIntoDB();
    void initVector(sVectorData *vp);
    virtual sVectorData *createVectorData();

  public:
    /** @name Constructors, destructor */
    //@{

    /**
     * Constructor.
     */
    explicit cSQLiteOutputVectorManager();

    /**
     * Destructor. Closes the output file if it is still open.
     */
    virtual ~cSQLiteOutputVectorManager();
    //@}

    /** @name Redefined cOutputVectorManager member functions. */
    //@{

    /**
     * Deletes output vector file if exists (left over from previous runs).
     * The file is not yet opened, it is done inside registerVector() on demand.
     */
    virtual void startRun();

    /**
     * Closes the output file.
     */
    virtual void endRun();

    /**
     * Registers a vector and returns a handle.
     */
    virtual void *registerVector(const char *modulename, const char *vectorname);

    /**
     * Deregisters the output vector.
     */
    virtual void deregisterVector(void *vectorhandle);

    /**
     * Sets an attribute of an output vector.
     */
    virtual void setVectorAttribute(void *vectorhandle, const char *name, const char *value);

    /**
     * Writes the (time, value) pair into the output file.
     */
    virtual bool record(void *vectorhandle, simtime_t t, double value);

    /**
     * Returns NULL, because this class doesn't use a file.
     */
    const char *getFileName() const {return NULL;}

    /**
     * Performs a database commit.
     */
    virtual void flush();
    //@}
};

#endif

