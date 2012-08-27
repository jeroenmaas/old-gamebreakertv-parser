/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2011 MaNGOSZero <http://github.com/mangoszero/mangoszero/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "stdafx.h"
#include "MysqlConnection.h"
#include "QueryResultMysql.h"
#include "Log.h"

MySQLConnection::~MySQLConnection()
{
    mysql_close(mMysql);
}

bool MySQLConnection::Initialize(std::string host, std::string user, std::string password, std::string name, uint32 port)
{
    MYSQL * mysqlInit = mysql_init(NULL);
    if (!mysqlInit)
    {
        return false;
    }

    mysql_options(mysqlInit,MYSQL_SET_CHARSET_NAME,"utf8");

    mMysql = mysql_real_connect(mysqlInit, host.c_str(), user.c_str(),
        password.c_str(), name.c_str(), 3306, NULL, 0);

    if (mMysql)
    {
        /*----------SET AUTOCOMMIT ON---------*/
        // It seems mysql 5.0.x have enabled this feature
        // by default. In crash case you can lose data!!!
        // So better to turn this off
        // ---
        // This is wrong since mangos use transactions,
        // autocommit is turned of during it.
        // Setting it to on makes atomic updates work
        // ---
        // LEAVE 'AUTOCOMMIT' MODE ALWAYS ENABLED!!!
        // W/O IT EVEN 'SELECT' QUERIES WOULD REQUIRE TO BE WRAPPED INTO 'START TRANSACTION'<>'COMMIT' CLAUSES!!!
        mysql_autocommit(mMysql, 1);
        /*-------------------------------------*/

        // set connection properties to UTF8 to properly handle locales for different
        // server configs - core sends data in UTF8, so MySQL must expect UTF8 too
        Execute("SET NAMES `utf8`");
        Execute("SET CHARACTER SET `utf8`");

        return true;
    }
    else
    {
        sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "Couldn't connect to mysql.", mysql_error(mysqlInit), NULL);
        mysql_close(mysqlInit);
        return false;
    }
}

bool MySQLConnection::PExecute(const char * format,...)
{
    if (!format)
        return false;

    va_list ap;
    char szQuery [32*1024];
    va_start(ap, format);
    int res = vsnprintf( szQuery, 32*1024, format, ap );
    va_end(ap);

    if(res==-1)
        return false;

    return Execute(szQuery);
}

bool MySQLConnection::Execute(const char* sql)
{
    if (!mMysql)
        return false;

    {
        if(mysql_query(mMysql, sql))
        {
            return false;
        }
    }

    return true;
}

QueryResult* MySQLConnection::Query(const char *sql)
{
    MYSQL_RES *result = NULL;
    MYSQL_FIELD *fields = NULL;
    uint64 rowCount = 0;
    uint32 fieldCount = 0;

    if(!_Query(sql,&result,&fields,&rowCount,&fieldCount))
        return NULL;

    QueryResultMysql *queryResult = new QueryResultMysql(result, fields, rowCount, fieldCount);

    queryResult->NextRow();
    return queryResult;
}

QueryResult* MySQLConnection::PQuery(const char * format,...)
{
    if (!format)
        return false;

    va_list ap;
    char szQuery [32*1024];
    va_start(ap, format);
    int res = vsnprintf( szQuery, 32*1024, format, ap );
    va_end(ap);

    if(res==-1)
        return NULL;

    return Query(szQuery);
}

bool MySQLConnection::_Query(const char *sql, MYSQL_RES **pResult, MYSQL_FIELD **pFields, uint64* pRowCount, uint32* pFieldCount)
{
    if (!mMysql)
        return 0;

    if(mysql_query(mMysql, sql))
        return false;

    *pResult = mysql_store_result(mMysql);
    *pRowCount = mysql_affected_rows(mMysql);
    *pFieldCount = mysql_field_count(mMysql);

    if (!*pResult )
        return false;

    if (!*pRowCount)
    {
        mysql_free_result(*pResult);
        return false;
    }

    *pFields = mysql_fetch_fields(*pResult);
    return true;
}