#ifndef _DBUTIL_H_
#define _DBUTIL_H_

#include "iDB.h"

class DBUtil
{
public:
    static void dumpTable(iDB* db, const string tblname);
    static string addCondParamPair(string name[], string val[], int length);
    static string addParam(string& cmd, string pval);
    static string escapeCmd(string& cmd);
};

#endif
