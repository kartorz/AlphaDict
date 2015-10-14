#ifndef _IDB_H_
#define _IDB_H_

#include <vector>
#include <string>

using namespace std;


class iDB 
{
public: 
    virtual bool open(const string& dbfile) = 0;

    // Read all records from table /tblname/.
    virtual vector< vector<string> > queryTable(const string tblname) = 0;

    // Read records of /colname/ from table /tblname/ where /cond/ meets.
    // [cond]: condition clause, eg: "WHERE col1 == 'v1'" 
    virtual vector< vector<string> > queryTable(const string tblname, const string colname[], const int collen, const string cond = "") = 0;

    // Check a row with /keys/ if exist. 
    virtual bool rowExist(const string tblname, string keyName[], string keyVal[], int keyLen) = 0;

    // string nm[] = {"name", "title"};
    // string type[] = {"TEXT PRIMARY KEY", "TEXT"};
    // createTable("article", nm, type, 2);
    virtual bool createTable(const string tblname, const string colname[], const string coltype[], const int length) = 0;

    // Inseart a row to table /tblname/
    virtual bool insertTable(const string tblname, const string colname[], const string colval[], const int length) = 0;

    // Modify a table table /tblname/ with the value /val/ according to column /col/ when /cond/ meets.
    virtual bool updateTable(const string tblname, const string colname[], const string colval[], const int length, string cond = "") = 0;

    // If cond meets, then modify the table /tblname/, or inseart a row to the table.
    virtual bool writeTable(const string tblname, const vector<string>& col, const vector<string>& val, string cond = "") = 0;

    virtual bool deleteTable(const string tblname, const string key, const string val) = 0;

    virtual bool deleteTable(const string tblname, const string cond = "") = 0;

    virtual const string vendor() = 0;
    
    virtual const string suffix() = 0;
};

#endif
