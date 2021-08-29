//数据管理模块 视频增删改查
#include <iostream>
#include <mysql/mysql.h>
//#include <json/json.h>
#include<jsoncpp/json/json.h>   
#include <mutex>
#include <fstream>
namespace vod_system{
#define MYSQL_HOST "127.0.0.1"
#define MYSQL_USER "root"
#define MYSQL_PASS NULL
#define MYSQL_NAME "vod_system_hlc"

  //几个通用接口
  static  MYSQL *MysqlInit()//初始化MQYSQL 
  {
    MYSQL*mysql=mysql_init(NULL);
    if(mysql==NULL){
      std::cout<<"mysql init failed\n";
      return NULL;
    }
    if(mysql_real_connect(mysql,MYSQL_HOST,MYSQL_USER,MYSQL_PASS,MYSQL_NAME,0,NULL,0)==NULL){
      std::cout<<mysql_error(mysql)<<std::endl;
      mysql_close(mysql);
      return NULL;
    }
    if(mysql_set_character_set(mysql,"utf8")!=0){
      std::cout<<mysql_error(mysql)<<std::endl;
      mysql_close(mysql);
      return NULL;
    }
    return mysql;
  }

  static void MysqlRelease(MYSQL*mysql){
    if(mysql!=NULL){
      mysql_close(mysql);
    }
    return;
  }

  static bool  MysqlQuery(MYSQL *mysql,const std::string  sql) //句柄 ，执行语句
  {
    int ret =mysql_query(mysql,sql.c_str());
    if(ret!=0){
      std::cout<<sql<<std::endl;
      std::cout<<mysql_error(mysql)<<std::endl;
      return false;

    }
    return true;
  }

  class TableVod{
    private:
      MYSQL *_mysql;
      std::mutex _mutex;
    public:
      TableVod(){
        //句柄初始化，连接服务器...
        _mysql=MysqlInit();
        if(_mysql==NULL){
          exit(0);
        }
      }
      ~TableVod(){
        //句柄销毁
        MysqlRelease(_mysql);
      }
      bool Insetrt(const Json::Value &video) //组织语句 把视频数据insert进数据库
      {
        const char *name=video["name"].asCString();
        const  char *vdesc=video["vdesc"].asCString();
        const char *video_url=video["video_url"].asCString();
        const char *image_url=video["image_url"].asCString();
        char sql[4096]={0};
#define VIDEO_INSERT "insert tb_video values(NULL,'%s','%s','%s','%s',now());"
        sprintf(sql,VIDEO_INSERT,name,vdesc,video_url,image_url);//按照格式组织后面的数据存入sql中
        return MysqlQuery(_mysql,sql);
      }

      bool Delete(int  video_id)//删除视频信息
      {
#define VIDEO_DELETE "delete from tb_video where id =%d;"
        char sql[4096]={0};
        sprintf(sql,VIDEO_DELETE,video_id);
        return MysqlQuery(_mysql,sql);
        printf("delete success\n");
      }

      bool Update(int video_id,Json::Value &video)//更新视频信息，只修改名称和描述
      {
#define  VIDEO_UPDATE "update tb_video set name='%s',vdesc='%s' where id=%d;"
        char sql[8192]={0};
        sprintf(sql,VIDEO_UPDATE,video["name"].asCString(),video["vdesc"].asCString(),video_id);
        return MysqlQuery(_mysql,sql);
      }      

      bool GetAll(Json::Value *video){
#define VIDEO_GETALL "select *from tb_video;"
        _mutex.lock();
        bool ret =MysqlQuery(_mysql,VIDEO_GETALL);//要保证执行语句和获取结果集这两步骤为线程安全的--加锁
        if(ret==false){
          _mutex.unlock();
          return false;
        }
        MYSQL_RES *res =mysql_store_result(_mysql);
        _mutex.unlock();

        if(res==NULL){
          std::cout<<"store result failed!\n";
          return false;
        }
        int num=mysql_num_rows(res);
        for(int i=0;i<num;++i){
          MYSQL_ROW row=mysql_fetch_row(res);
          Json::Value val;
          val["id"]=std::stoi(row[0]);
          val["name"]=row[1];
          val["vdesc"]=row[2];
          val["video_url"]=row[3];
          val["image_url"]=row[4];
          val["ctime"]=row[5];
          video->append(val);//添加数组元素
        }
        mysql_free_result(res);//释放结果集
        return true;
      }


      bool GetOne(int video_id,Json::Value *video){
#define VIDEO_GETONE "select *from tb_video where id = %d;"
        char sql_str[8192]={0};
        sprintf(sql_str,VIDEO_GETONE,video_id);
        _mutex.lock();
        bool ret =MysqlQuery(_mysql,sql_str);
        if(ret==false){
          _mutex.unlock();
          return false;
        }
        MYSQL_RES *res=mysql_store_result(_mysql);
        _mutex.unlock();

        if(res==NULL){
          std::cout<<mysql_error(_mysql)<<std::endl;
          mysql_free_result(res);
          return false;
        }
        int num_row=mysql_num_rows(res);
        if(num_row!=1){  //获取的结果不等于一个则说明获取出错。
          std::cout<<"getone result error\n";
          mysql_free_result(res);
          return false;
        }
        MYSQL_ROW row =mysql_fetch_row(res);
        (*video)["id"]=video_id;
        (*video)["name"]=row[1];
        (*video)["vdesc"]=row[2];
        (*video)["video_url"]=row[3];
        (*video)["image_url"]=row[4];
        (*video)["ctime"]=row[5];

        return true;
      }
  };


  class Util{
    public:
      static bool WriteFile(const std::string &name,const std::string  &content){                               
        std::ofstream of;   //输出流 
        of.open(name,std::ios::binary); //创建fstream对象,其文件名为file.name        
        if(!of.is_open()){    
          std::cout<<"open file failed!\n"<<std::endl;    
          return false;    
        }    
        of.write(content.c_str(),content.size());//将file的内容写入到新创的of对象的文件中。   
        if(!of.good()){
          std::cout<<"write file faileed!\n"<<std::endl;
          return false;                                
        }                  
        of.close();
        return true;   
      }      
      static bool ReadFile(const std::string& name,std::string* body){
        std::ifstream ifile;
        ifile.open(name,std::ios::binary);
        if(!ifile.is_open()){
          std::cout<<"open "<<name<<" failed!"<<std::endl;
          ifile.close();
          return false;
        }
        ifile.seekg(0,std::ios::end);
        uint64_t length = ifile.tellg();
        ifile.seekg(0,std::ios::beg);
        body->resize(length);
        ifile.read(&(*body)[0],length);
        if(!ifile.good()){
          std::cout<<"read "<<name<<" failed!"<<std::endl;
          ifile.close();
          return false;
        }
        ifile.close();
        return true;
      }

  };
}

