#ifndef _DBFACTORY_H_
#define _DBFACTORY_H_

#include "iDB.h"
#include "SqliteDB.h"

class DBFactory
{
public:
    enum ARGUMENT
    {
        CURRENT_TIME,
        CURRENT_DATE,
        CURRENT_TIMESTAMP,
    };

    static iDB* create(const string& dbfile)
    {
        static iDB* db = new SqliteDB(dbfile);
        return db;
    }

    static iDB* create()
    {
        static iDB* db = new SqliteDB();
        return db;
    }
};

#endif
