#include "httplib.h"
#include"db.hpp"
#include<jsoncpp/json/json.h>   
#include <boost/algorithm/string.hpp>
using namespace httplib;

#define WWWROOT  "./wwwroot"
vod_system::TableVod* tb_video;

void VideoDelete(const Request &req,Response &rsp){
  //删除不仅要删除数据库中数据，还要删除实际上传的文件和图片资源；
  //请求路径 req.path=/video/id
  //1.获取视频id
  int video_id =std::stoi(req.matches[1]);

  //2.从数据库中获取到对应视频信息
  Json::Value json_rsp;
  Json::FastWriter writer;
  Json::Value video;
  bool ret =tb_video->GetOne(video_id,&video);
  if(ret==false){
    std::cout<<"mysql get video info failed!"<<std::endl;
    rsp.status=500;//500表示服务器内部错误
    json_rsp["result"]=false;
    json_rsp["reason"]= "mysql get video info failed!";
    rsp.body=writer.write(json_rsp);//将错误信息序列化为json_rsp 写入响应正文
    rsp.set_header("Content-Type","application/json");//设置头部信息 正文类型：Json
    return;
  }
  //3.组织删除视频文件和封面图片文件
  std::string vpath=WWWROOT+video["video_url"].asString(); //视频实际路径
  std::string ipath=WWWROOT+video["image_url"].asString(); //图片实际路径
  unlink(vpath.c_str());
  unlink(ipath.c_str());

  //4.删除数据库中的数据  使用unlink(const char * pathname)删除指定文件；
  bool retd =tb_video->Delete(video_id);
  if(retd==false){
    std::cout<<"mysql delete video failed!"<<std::endl;
    rsp.status=500;
    return ;
  }

  //5.设置相应信息  200-ok
  rsp.status=200;//也可以不给值 rsp.status默认为200：ok
}

void VideoUpdate(const Request &req,Response &rsp){
  //只修改视频name 和 desc
  int video_id =std::stoi(req.matches[1]);
  Json::Value video;
  Json::Reader reader;
  bool ret =reader.parse(req.body,video);//将body中json格式字符串反序列化存入video中
  if(ret==false){
    std::cout<<"updata video : parse video json failed!\n";
    rsp.status=400;//客户端出错；bad Request
    return;
  }
  ret = tb_video->Update(video_id,video);
  if(ret==false){
    std::cout<<"updata video : parse video json failed!\n";
    rsp.status=400;//客户端出错；bad Request
    return;
  }
  return;
}

void VideoGetAll(const Request &req,Response &rsp){
  //从数据库中获取所有视屏信息组织成rsp正文
  Json::Value videos;
  Json::FastWriter writer;
  bool ret =tb_video->GetAll(&videos);
  if(ret==false){
    std::cout<<"getall video :mysql operation failed!\n";
    rsp.status=500;
    return;
  }
  rsp.body=writer.write(videos);//将获取到的所有视频信息写入响应正文
  rsp.set_header("Content-Type","application/json");//设置头部信息   正文类型：Json
  rsp.status=200;
  return;
}

void VideoGetOne(const Request &req,Response &rsp){

  int video_id =std::stoi(req.matches[1]);//获取视频ID

  Json::Value video;
  Json::FastWriter writer;
  bool ret =tb_video->GetOne(video_id,&video);
  if(ret==false){
    std::cout<<"getone video :mysql operation failed!\n";
    rsp.status=500;
    return;
  }
  rsp.body=writer.write(video);//将获取到的所有视频信息写入响应正文
  rsp.set_header("Content-Type","application/json");//设置头部信息   正文类型：Json
  rsp.status=200;
  return;
}

#define VIDEO_PATH  "/video/"
#define IMAGE_PATH "/image/"
void VideoUpload(const Request &req,Response &rsp){//上传i
  //获取前端上传的信息
  //获取video_name
  auto ret =req.has_file("video_name");//根据video_name字段判断是否有相应的文件上传请求。         
  if(ret ==false){
    std::cout<<"have no video_name"<<std::endl;
    rsp.status=400;//请求错误
    return;
  }
  const auto& file = req.get_file_value("video_name");//获取上传的文件信息。

  //获取video_desc
  ret =req.has_file("video_desc");//根据video_desc字段判断是否有相应的文件上传请求
  if(ret ==false){
    std::cout<<"have no video_desc"<<std::endl;
    rsp.status=400;//请求错误 
    return;
  }
  const auto& file1= req.get_file_value("video_desc");//获取上传的文件信息。
  //获取视频文件
  ret =req.has_file("video_file");//根据video_file字段判断是否有相应的文件上传请求
  if(ret ==false){
    std::cout<<"have no video_file"<<std::endl;
    rsp.status=400;//请求错误                
    return;
  }
  const auto& file2= req.get_file_value("video_file");//获取上传的文件信息。

  //获取封面文件
  ret =req.has_file("image_file");//根据image_file字段判断是否有相应的文件上传请求
  if(ret ==false){
    std::cout<<"have no image_file"<<std::endl;
    rsp.status=400;//请求错误  
    return;
  }
  const auto& file3= req.get_file_value("image_file");//获取上传的文件信息。
  const std::string &vname=file.content;
  const std::string &vdesc=file1.content;
  const std::string &vfile=file2.filename;//视频文件较大 拷贝占内存用引用。
  const std::string &vcont=file2.content;
  const std::string &ifile=file3.filename;
  const std::string &icont=file3.content;

  std::string vurl = VIDEO_PATH+file2.filename;
  std::string iurl = IMAGE_PATH+file3.filename;

  vod_system:: Util::WriteFile(WWWROOT+vurl,file2.content);//文件写入
  vod_system:: Util::WriteFile(WWWROOT+iurl,file3.content);
  //组织数据
  Json::Value video;
  video["name"]=vname;
  video["vdesc"]=vdesc;
  video["video_url"]=vurl;
  video["image_url"]=iurl;
  bool reti = tb_video->Insetrt(video);//上传到数据库
  if(reti==false){
    rsp.status=500;
    std::cout<<"insert video :mysql operation failed!\n";
    return ;
  }
  rsp.set_redirect("/");//上传后刷新界面
  rsp.set_header("Content_Type","text/html");
}

void VideoPlay(const Request &rep,Response &rsp){
  Json::Value video;
  int video_id = std::stoi(rep.matches[1]);
  bool ret = tb_video->GetOne(video_id,&video);
  if(!ret){
    std::cout<<"getone video:mysql operation failed!"<<std::endl;
    rsp.status = 500;
    return;
  }
  std::string oldstr = "{{video_url}}";
  std::string newstr =  video["video_url"].asString();
  std::string play_html = "./wwwroot/single-video.html";
  vod_system::Util::ReadFile(play_html, &rsp.body);
  boost::algorithm::replace_all(rsp.body, oldstr, newstr);
  rsp.set_header("Content-Type","text/html");
  return;
}

int  main(){
  // test();
  tb_video=new vod_system::TableVod();//实例化一个TableVod对象；
  Server srv;
  srv.set_base_dir("./wwwroot");

  //动态数据请求
  //正则表达式， \d表示匹配一个数字字符， +表示匹配字符一次或者多次。
  //R"(string)"  表示去除括号中字符串中每个字符的特殊含义，使用原始string的含有。
  //(\d+)正则表达式中捕捉匹配到的括号中数字，匹配到的数据存储在Request类下的Match matches;中。
  srv.Delete(R"(/video/(\d+))",VideoDelete);
  srv.Put(R"(/video/(\d+))",VideoUpdate);//修改
  srv.Get(R"(/video)",VideoGetAll);
  srv.Get(R"(/video/(\d+))",VideoGetOne);
  srv.Post(R"(/video)",VideoUpload);//新增视屏
  srv.Get(R"(/play/(\d+))",VideoPlay);
  srv.listen("0.0.0.0",9000);

  return 0;
}
