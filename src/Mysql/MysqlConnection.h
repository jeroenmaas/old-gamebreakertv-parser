#ifdef WIN32
#include <winsock2.h>
#include <mysql/mysql.h>
#else
#include <mysql.h>
#endif
#include "callback.h"
#include "common.h"
#include "Field.h"
#include "boost/serialization/singleton.hpp"

class MySQLConnection : public boost::serialization::singleton< MySQLConnection >
{
    public:
        MySQLConnection() : mMysql(NULL) {}
        ~MySQLConnection();

        bool Initialize(std::string host, std::string user, std::string password, std::string name, uint32 port);

        bool Execute(const char *sql);
        bool PExecute(const char *format,...);

        QueryResult* Query(const char *sql);
        QueryResult* PQuery(const char *format,...);

    private:
        bool _Query(const char *sql, MYSQL_RES **pResult, MYSQL_FIELD **pFields, uint64* pRowCount, uint32* pFieldCount);
        MYSQL *mMysql;
};

#define sDatabase MySQLConnection::get_mutable_instance()

class QueryResult
{
    public:
        QueryResult(uint64 rowCount, uint32 fieldCount)
            : mFieldCount(fieldCount), mRowCount(rowCount) {}

        virtual ~QueryResult() {}

        virtual bool NextRow() = 0;

        Field *Fetch() const { return mCurrentRow; }

        const Field & operator [] (int index) const { return mCurrentRow[index]; }

        uint32 GetFieldCount() const { return mFieldCount; }
        uint64 GetRowCount() const { return mRowCount; }

    protected:
        Field *mCurrentRow;
        uint32 mFieldCount;
        uint64 mRowCount;
};