#include "httplib.h"
#include <fstream>
using namespace httplib;
//httplib搭建简单服务器

void helloworld(const Request & req,Response &rsp){
  rsp.body="<html><body>Hello world</hl></body></html>";
  rsp.status=200;
  rsp.set_header("Content-Type","text/html");
}


bool WriteFile(std::string &name,std::string  &content){
  std::fstream of;
  of.open(name,std::ios::out|std::ios::binary|std::ios::trunc); //std::ios::out相当于write写的权限。//创建fstream对象,其文件名为file.name
  if(!of.is_open()){
    std::cout<<"open failed!\n"<<std::endl;
    return false;
  }
  of.write(content.c_str(),content.size());//将file的内容写入到新创的of对象的文件中。
  if(!of.good()){
    std::cout<<"write faileed!\n"<<std::endl;
    return false;
  }
  of.close();
  return true;
}

void upload(const Request & req,Response &rsp){
  //根据video_name字段判断是否有相应的文件上传请求。
//获取video_name
  auto ret =req.has_file("video_name");//根据video_name字段判断是否有相应的文件上传请求。
  if(ret ==false){
    std::cout<<"have no file"<<std::endl;
    return;
  }
  const auto& file = req.get_file_value("video_name");//获取上传的文件信息。
  std::string vname=file.content;

//获取video_desc
  ret =req.has_file("video_desc");//根据video_desc字段判断是否有相应的文件上传请求
  if(ret ==false){
    std::cout<<"have no file"<<std::endl;
    return;
  }
  const auto& file1= req.get_file_value("video_desc");//获取上传的文件信息。
  std::string vdesc=file1.content;


//获取视频文件
  ret =req.has_file("video_file");//根据video_file字段判断是否有相应的文件上传请求
  if(ret ==false){
    std::cout<<"have no file"<<std::endl;
    return;
  }
  const auto& file2= req.get_file_value("video_file");//获取上传的文件信息。
  std::string vfile=file2.filename;
  std::string vcont=file2.content;
  
//获取封面文件
  ret =req.has_file("image_file");//根据image_file字段判断是否有相应的文件上传请求
  if(ret ==false){
    std::cout<<"have no file"<<std::endl;
    return;
  }
   const auto& file3= req.get_file_value("image_file");//获取上传的文件信息。
  std::string ifile=file3.filename;
  std::string icont=file3.content;

//打印看效果
  std::cout<<"["<<vname<<"]\n";
  std::cout<<"["<<vdesc<<"]\n";
  std::cout<<"["<<vfile<<"]\n";
  std::cout<<"["<<vcont<<"]\n";
  std::cout<<"["<<ifile<<"]\n";
  std::cout<<"["<<icont<<"]\n";

}

int main(){

  //实例化一个Server对象，
  httplib::Server srv;
  srv.set_base_dir("./wwwroot");//设置静态资源相对根目录
  srv.Get("/hello",helloworld); // /hello是请求,helloworld为该对应的处理函数，http收到请求后调用该函数处理
  srv.Get("/hi",helloworld);

  srv.Post("/video",upload);
  srv.listen("0.0.0.0",9000);

  return 0;
}
