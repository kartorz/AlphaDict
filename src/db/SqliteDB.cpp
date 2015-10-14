/**
 *
 *
 **/

#include "SqliteDB.h"
#include "Log.h"
#include "DBUtil.h"

#define LOG  g_sysLog

MutexCriticalSection SqliteDB::s_queryCS = MutexCriticalSection();
vector< vector<string> > SqliteDB::s_tblTemp = vector< vector<string> >();

SqliteDB::SqliteDB(const string& dbfile)
{
    open(dbfile);
}

SqliteDB::SqliteDB() {}

SqliteDB::~SqliteDB()
{
    sqlite3_close(m_db);
}

bool SqliteDB::open(const string& dbfile)
{
   //int rc = sqlite3_open_v2(dbfile.c_str(), &m_db, SQLITE_OPEN_FULLMUTEX, NULL);
   int rc = sqlite3_open(dbfile.c_str(), &m_db);
   if (rc != 0) {
       LOG(LOG_ERROR, "{SqliteDB} can't open [%s], error code: [%d]\n", dbfile.c_str(), rc);

       sqlite3_close(m_db);
       return false;
   }
   return true;
}

int SqliteDB::queryCallback(void *NotUsed, int argc, char **argv, char **azColName)
{
    vector<string> row;
    row.reserve(argc);
    for(int i=0; i<argc; i++) {
        row.push_back(argv[i]);        
    }
    s_tblTemp.push_back(row);
    //LOG(LOG_DEBUG, "[queryCallback] push back a row\n");
    //LOG(LOG_DEBUG, "[queryCallback]: (%d, %s)\n\n\n", argc, s_tblTemp[0][0].c_str());
    return 0;
}

vector< vector<string> > SqliteDB::queryTable(const string tblname)
{
    char *zErrMsg = 0;
    string sqlcmd = "SELECT * FROM " + tblname;
    MutexLock lock(s_queryCS);
    s_tblTemp.clear();
    int rc = sqlite3_exec(m_db, sqlcmd.c_str(), queryCallback, 0, &zErrMsg);
    if (rc != 0) {
        LOG(LOG_ERROR, "[queryTable] error msg: [%s], error code:[%d], sqlcmd:[%s]\n", zErrMsg, rc, sqlcmd.c_str());
    }

    return s_tblTemp;
}

vector< vector<string> > SqliteDB::queryTable(const string tblname, const string colname[], const int collen, const string cond)
{
    char *zErrMsg = 0;
    string sqlcmd = "SELECT ";
    for (int i = 0; i < collen; i++) {
        sqlcmd += colname[i];
        if (i != collen -1) sqlcmd += ",";
    }
    sqlcmd += " FROM ";
    sqlcmd += tblname + " ";
    sqlcmd += cond;
    
    MutexLock lock(s_queryCS);
    s_tblTemp.clear();
    int rc = sqlite3_exec(m_db, sqlcmd.c_str(), queryCallback, 0, &zErrMsg);
    if (rc != 0) {
        LOG(LOG_ERROR, "[queryTable] error msg: [%s], error code:[%d], sqlcmd:[%s]\n", zErrMsg, rc, sqlcmd.c_str());
    }

    return s_tblTemp;
}

bool SqliteDB::rowExist(const string tblname, string keyName[], string keyVal[], int keyLen)
{
    char *zErrMsg = 0;
    string sqlcmd = "SELECT " + keyName[0] + +" FROM " + tblname + " ";
    sqlcmd += DBUtil::addCondParamPair(keyName, keyVal, keyLen);

    MutexLock lock(s_queryCS);
    s_tblTemp.clear();
    int rc = sqlite3_exec(m_db, sqlcmd.c_str(), queryCallback, 0, &zErrMsg);
    if (rc != 0) {
        LOG(LOG_ERROR, "[rowExist] error msg: [%s], error code:[%d], sqlcmd:[%s]\n", zErrMsg, rc, sqlcmd.c_str());
    }
    return !s_tblTemp.empty();
}

bool SqliteDB::createTable(const string tblname, const string colname[], const string coltype[], const int length)
{
    char *zErrMsg = 0;
    // CREATE TABLE tb_name(col1 TEXT PRIMARY KEY, col2 TEXT)
    string sqlcmd = "CREATE TABLE " + tblname;

    sqlcmd += "(";
    for (int i = 0; i < length; i++) {
        sqlcmd += colname[i] + " " + coltype[i];

        if (i != length -1) sqlcmd +=  ", ";
    }
    sqlcmd += ")";

    return execSql(sqlcmd);
}

bool SqliteDB::insertTable(const string tblname, const string colname[], const string colval[], const int length)
{
    // INSERT INTO  table (col1, col2) VALUES('val1', 'val2')
    char *zErrMsg = 0;
    string sqlcmd = "INSERT INTO " + tblname;

    sqlcmd += "(";
    for (int i = 0; i < length; i++) {
        sqlcmd += colname[i];
        if (i != length -1) sqlcmd +=  ", ";
    }
    sqlcmd += ")";

    sqlcmd += " VALUES(";
    for (int i = 0; i < length; i++) {
        string val =  (colval[i]);
        DBUtil::escapeCmd(val);        
        sqlcmd += "\'" + val + "\'";
        if (i != length -1) sqlcmd +=  ", ";
    }
    sqlcmd += ")";

    return execSql(sqlcmd);
}

bool SqliteDB::updateTable(const string tblname, const string colname[], const string colval[], const int length, string cond)
{
    // UPDATE tbl SET col1 = val1, cal2 = val2  WHERE key == xxx"
    string sqlcmd = "UPDATE " + tblname + " SET ";

    for (int i = 0; i < length; i++) {
        string val =  (colval[i]);
        DBUtil::escapeCmd(val);
        sqlcmd += colname[i] + " = " +  val;
        if (i != length -1) sqlcmd +=  ", ";
    }
    sqlcmd += " ";
    sqlcmd += cond;
    return execSql(sqlcmd);
}

bool SqliteDB::writeTable(const string tblname, const vector<string>& col, const vector<string>& val, string cond)
{
    return true;
}

bool SqliteDB::deleteTable(const string tblname, const string key, const string val)
{
    string sqlcmd = "DELETE FROM ? WHERE ? == '?'";
    DBUtil::addParam(sqlcmd, tblname);
    DBUtil::addParam(sqlcmd, key);
    DBUtil::addParam(sqlcmd, val);

    return execSql(sqlcmd);
}

bool SqliteDB::deleteTable(const string tblname, const string cond)
{
    string sqlcmd = "DELETE FROM " + tblname + " " + cond;

    return execSql(sqlcmd);
}

bool SqliteDB::execSql(string& sqlcmd)
{
    char *zErrMsg = 0;

    int rc = sqlite3_exec(m_db, sqlcmd.c_str(), NULL, 0, &zErrMsg);
    if (rc != 0) {
        LOG(LOG_ERROR, "[execSql] error msg: [%s], error code:[%d], cmd:[%s]\n", zErrMsg, rc, sqlcmd.c_str());
    }
    return rc == 0;
}
