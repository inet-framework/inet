///
/// @file   csqliteoutputscalarmgr.h
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2012-12-07
///
/// @brief  Declares cSQLiteOutputScalarManager class that writes output scalars
///         into SQLite databases.
///
/// @remarks Copyright (C) 2012 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#ifndef __SQLITEOUTPUTSCALARMGR_H
#define __SQLITEOUTPUTSCALARMGR_H

#include <sqlite3.h>
#include <omnetpp.h>


/**
 * An Output Scalar Manager that writes into an SQLite database instead of
 * a .sca file.
 *
 * It expects to find two tables in the database: SRUN and SCALAR.
 * SRUN corresponds to the "run" lines in the normal .sca files,
 * and SCALAR to the "scalar" (data) lines.
 *
 * <pre>
 * CREATE TABLE srun (
 *      id INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
 *      runnumber INT NOT NULL,
 *      network VARCHAR(80) NOT NULL,
 *      date TIMESTAMP NOT NULL
 * );
 *
 * CREATE TABLE scalar (
 *      runid INT  NOT NULL,
 *      module VARCHAR(200) NOT NULL,
 *      name VARCHAR(80) NOT NULL,
 *      value DOUBLE PRECISION NOT NULL,
 *      KEY (runid,module,name),
 *      FOREIGN KEY (runid) REFERENCES run(id)
 * ) ENGINE = MYISAM;
 * </pre>
 *
 * Additional config parameters:
 *
 * <pre>
 * [General]
 * mysqloutputscalarmanager-commit-freq = <integer> # COMMIT every n INSERTs, default=10
 * mysqloutputscalarmanager-connectionname = <string> # look for connect parameters in the given object
 * </pre>
 *
 * @ingroup Envir
 */
class cSQLiteOutputScalarManager : public cOutputScalarManager
{
  protected:
    sqlite3 *db;

    // we COMMIT after every commitFreq INSERT statements
    int commitFreq;
    int insertCount;

    // database id (run.id) of current run
    long runId;

    bool initialized;

    // prepared statements and their parameter bindings
    sqlite3_stmt *insertScalarStmt;

    // these variables are bound to the above bind parameters
    int runidBuf;
    char moduleBuf[201];
    unsigned long moduleLength;
    char nameBuf[81];
    unsigned long nameLength;
    double valueBuf;

  protected:
    void openDB();
    void closeDB();
    void beginTransaction();
    void endTransaction();
    void insertRunIntoDB();

  public:
    /** @name Constructors, destructor */
    //@{

    /**
     * Constructor.
     */
    explicit cSQLiteOutputScalarManager();

    /**
     * Destructor.
     */
    virtual ~cSQLiteOutputScalarManager();
    //@}

    /** @name Controlling the beginning and end of collecting data. */
    //@{

    /**
     * Opens collecting. Called at the beginning of a simulation run.
     */
    virtual void startRun();

    /**
     * Closes collecting. Called at the end of a simulation run.
     */
    virtual void endRun();

    /** @name Scalar statistics */
    //@{

    /**
     * Records a double scalar result into the scalar result file.
     */
    virtual void recordScalar(cComponent *component, const char *name, double value, opp_string_map *attributes=NULL);

    /**
     * Records a histogram or statistic object into the scalar result file.
     */
    virtual void recordStatistic(cComponent *component, const char *name, cStatistic *statistic, opp_string_map *attributes=NULL);

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

