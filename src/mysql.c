#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include<mysql/mysql.h>

int  main(){
  MYSQL *mysql=NULL;
  //1.初始化MYSQL句柄MYSQL *mysql_init(MYSQL *mysql)；
  mysql=mysql_init(NULL);
  if(mysql==NULL){
    printf("mysql init error\n");
    return -1;
  }
  //2.连接服务器
  //MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user,
  // const char *passwd,const char *db, unsigned int port,const char *unix_socket,   unsigned long client_flag);
  ////mysql--初始化完成的句柄
  ////host---连接的mysql服务器的地址
  ////user---连接的服务器的用户名
  ////passwd-连接的服务器的密码
  ////db ----默认选择的数据库名称
  ////port---连接的服务器的端口： 默认0是3306端口
  ////unix_socket---通信管道文件或者socket文件，通常置NULL
  ////client_flag---客户端标志位，通常置0
  ////返回值：成功返回句柄，失败返回NULL
  if(mysql_real_connect(mysql,"127.0.0.1","root","hlc","hu130052"
        "vod_system",0,NULL,0)==NULL){
    printf("mysql connect failed:%s\n",mysql_error(mysql));
    return -1;
  }

  //3. 设置字符集int mysql_set_character_set(MYSQL *mysql, const char *csname)
  ////mysql--初始化完成的句柄
  ////csname--字符集名称，通常："utf8"
  ////返回值：成功返回0， 失败返回非0；
  int ret;
  ret= mysql_set_character_set(mysql,"utf-8");
  if(ret!=0){
    printf("set character failed:%s\n",mysql_error(mysql));
    return -1;
  }
  //4.选择操作的数据库：int mysql_select_db(MYSQL *mysql, const char *db)
  ////mysql--初始化完成的句柄
  ////db-----要切换选择的数据库名称
  ////返回值：成功返回0， 失败返回非0；
  mysql_select_db(mysql,"vod_system");
  //5.执行sql语句：
  //int mysql_query(MYSQL *mysql, const char *stmt_str)
  ////mysql--初始化完成的句柄
  ////stmt_str--要执行的sql语句
  ////返回值：成功返回0， 失败返回非0；

  //char *insert="update from tb_video values(NULL,'变形金刚'，'婆媳伦理片','./video/popo.mp4','./image/xifu.jpg',now())";
  char* select="select * from tb_video;";
  ret =mysql_query(mysql,select);
  if(ret!=0){
    printf("query sql failed: %s\n",mysql_error(mysql));
    return -1;
  }

  //6.保存查询结果到本地：MYSQL_RES *mysql_store_result(MYSQL *mysql)
  ////mysql--初始化完成的句柄
  ////返回值：成功返回结果集的指针， 失败返回NULL
  MYSQL_RES *res=mysql_store_result(mysql);
  if(res==NULL){
    printf("store result  failed: %s\n",mysql_error(mysql));
    return -1;
  }
  //7.获取结果集中的行数与列数：
  //uint64_t mysql_num_rows(MYSQL_RES *result)；
  ////result--保存到本地的结果集地址
  ////返回值：结果集中数据的条数；
  //unsigned int mysql_num_fields(MYSQL_RES *result)
  ////result--保存到本地的结果集地址
  ////返回值：结果集中每一条数据的列数；
  int row_num=mysql_num_rows(res);
  int col_num=mysql_num_fields(res);
  for(int i=0;i<row_num;++i){
    //8.逐条遍历获取结果集
    //MYSQL_ROW mysql_fetch_row(MYSQL_RES *result)
    ////result--保存到本地的结果集地址
    ////返回值：实际上是一个char **的指针，将每一条数据做成了字符串指针数组
    //row[0]-第0列 row[1]-第1列
    ////并且这个接口会保存当前读取结果位置，每次获取的都是下一条数据
    MYSQL_ROW row =mysql_fetch_row(res);
    for(int j=0;j<col_num;++j){
      printf("%s\t",row[j]);
    }
    printf("\n");
  }
  //释放结果集
  mysql_free_result(res);
  mysql_close(mysql);
  return 0;

}
