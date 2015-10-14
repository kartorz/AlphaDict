
#include <boost/algorithm/string/replace.hpp>

#include "Log.h"
#include "DBUtil.h"

void DBUtil::dumpTable(iDB* db, const string tblname)
{
    vector< vector<string> > table = db->queryTable(tblname);
    for (int row = 0; row < table.size(); row++)
        for (int column = 0; column < table[row].size(); column++)
            g_sysLog(LOG_DEBUG, "{dumpTable} row: [%d], column: [%d], value: [%s]\n",row, column, table[row][column].c_str());
}

string DBUtil::addCondParamPair(string key[], string val[], int length)
{
    // SELECT * FROM tbl  WHERE col1 == 'v1'  AND col2 == 'v2'
    string cond = "WHERE ";
    for (int i = 0; i< length; i++) {
        cond += key[i] + " == " + "\'" + val[i] + "\'";
        if (i != length -1) cond += " AND";
    }
    return cond;
}

// [cmd]: Use ? as a placehold, eg: WHERE key1 == ? AND key2  == ?  
string DBUtil::addParam(string& cmd, string pval)
{
    string search = "\?";
    boost::algorithm::replace_first(cmd, search, pval);

    return cmd;
}

string DBUtil::escapeCmd(string& cmd)
{
    string search = "'";
    string format = "''";
    boost::algorithm::replace_all(cmd, search, format);
    
    search = "\"";
    format = "\"\"";
    boost::algorithm::replace_all(cmd, search, format);

    return cmd;
}
