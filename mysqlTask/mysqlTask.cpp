#include <signal.h>
#include <iostream>
#include <string>
#include <vector>
#include <workflow/WFFacilities.h>
#include <workflow/MySQLUtil.h>
#include <workflow/MySQLResult.h>
#include <workflow/mysql_types.h>
using std::cout;
using std::cerr;
using std::string;
using std::vector;

static WFFacilities::WaitGroup waitGroup(1);

void sighandler(int signum)
{
    cout << "signum = " << signum << "\n";
    cout << "done!\n";
    waitGroup.done();
}

void mysqlCallback(WFMySQLTask *mysqlTask)
{
    //1.错误检验
    if(mysqlTask->get_state() != WFT_STATE_SUCCESS){
        //系统层面报错, 权限 or 密码
        cerr << "error_msg = " << WFGlobal::get_error_string(mysqlTask->get_state(), mysqlTask->get_error()) << "\n";
        return;
    }
    protocol::MySQLResponse *resp = mysqlTask->get_resp();
    if(resp->get_packet_type() == MYSQL_PACKET_ERROR){
        //在SQL语句报错
        cerr << "error_code = " << resp->get_error_code() << ", error_msg = " << resp->get_error_msg() << "\n";
        return;
    }

    //2.处理执行结果
    protocol::MySQLResultCursor cursor(resp);
    do{
        //区分读还是写
        if(cursor.get_cursor_status() == MYSQL_STATUS_OK){
            //写类型的SQL语句
            cerr << "write \n";
            cerr << cursor.get_affected_rows() << " rows affected\n";
        }
        else if(cursor.get_cursor_status() == MYSQL_STATUS_GET_RESULT){
            //读类型的SQL语句
            cerr << "read \n";
            //读表头:列的信息 field
            const protocol::MySQLField * const * fieldArr;
            fieldArr = cursor.fetch_fields();
            for(int i = 0; i < cursor.get_field_count(); ++i){
                cerr << "db = " << fieldArr[i]->get_db()
                     << " table = " << fieldArr[i]->get_table()
                     << " name = " << fieldArr[i]->get_name()
                     << " type = " << datatype2str(fieldArr[i]->get_data_type()) << "\n";
            }
            //读表的内容:每一行每一列
            //bool fetch_all(vector<vector<MySQLCell>> &rows);
            vector<vector<protocol::MySQLCell>> rows;
            cursor.fetch_all(rows);
            for(auto &row:rows){
                for(auto &cell:row){
                    if(cell.is_int()){
                        cerr << cell.as_int();
                    }
                    else if(cell.is_string()){
                        cerr << cell.as_string();
                    }
                    else if(cell.is_datetime()){
                        cerr << cell.as_datetime();
                    }
                    cerr << "\t";
                }
                cerr << "\n";
            }
        }
    }while(cursor.next_result_set()); //mysql任务支持一个任务处理多个SQL语句
}

int main()
{
    signal(SIGINT, sighandler);
    
    //创建mysql任务, 客户端
    WFMySQLTask *mysqlTask = WFTaskFactory::create_mysql_task(
                    "mysql://root:123456@localhost", //123456是mysql的密码
                    1, mysqlCallback);
    //准备一个sql语句。一个mysqlTask可以处理多条sql语句。
    //不能用USE选择database, 每条指令就必须写清除表所在的database
    string sql = "insert into mycloud.tbl_user_token (user_name, user_token) values ('Caixukun', 'sing_dance_rap');";
    sql += "select * from mycloud.tbl_user_token;";
    mysqlTask->get_req()->set_query(sql);
    mysqlTask->start();
    waitGroup.wait();
    return 0;
}
