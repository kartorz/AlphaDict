#ifndef _SQLITEDB_H_
#define _SQLITEDB_H_

#include <vector>
#include <map>
#include <sqlite3.h>

#include "iDB.h"
#include "MutexLock.h"

class SqliteDB : public iDB
{
public:
    SqliteDB(const string& dbfile);
    SqliteDB();
    ~SqliteDB();

    virtual bool open(const string& dbfile);

    virtual vector< vector<string> > queryTable(const string tblname);

    virtual vector< vector<string> > queryTable(const string tblname, const string colname[], const int collen, const string cond = "");
    
    virtual bool createTable(const string tblname, const string colname[], const string coltype[], const int length);

    virtual bool rowExist(const string tblname, string keyName[], string keyVal[], int keyLen);

    virtual bool insertTable(const string tblname, const string colname[], const string colval[], const int length);

    virtual bool updateTable(const string tblname, const string colname[], const string colval[], const int length, string cond = "");

    virtual bool writeTable(const string tblname, const vector<string>& col, const vector<string>& val, string cond = "");

    virtual bool deleteTable(const string tblname, const  string key, const string val);

    virtual bool deleteTable(const string tblname, const  string cond = "");

    virtual const string vendor() { return "sqlite"; }
    
    virtual const string suffix() { return ".sqlite"; }

private:
    bool execSql(string& sqlcmd);
    static MutexCriticalSection s_queryCS;
    static int queryCallback(void *NotUsed, int argc, char **argv, char **azColName);
    static vector< vector<string> > s_tblTemp;
    sqlite3 *m_db;
};

#endif
