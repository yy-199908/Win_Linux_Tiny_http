#ifndef YYServer_H
#define YYServer_H
#define BUFFER_SIZE 1024
#include<sys/types.h>
#include<sys/stat.h>
#include <iostream>
#include <string>

#include <fstream>
#ifdef _WIN32
#include <Winsock2.h>
#pragma comment(lib,"WS2_32.lib")
#include <thread>
#include <Windows.h>
static DWORD WINAPI thread_main(LPVOID arg);
#else 

#include <signal.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<stdint.h>
#include<unistd.h>
#include<ctype.h>
#include<errno.h>
#include<string.h>
#include <strings.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<pthread.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#define USER_LIMIT 100
#define MAX_EVENT_NUMBER 100



 void* worker(void* arg);
#endif



class YYServer
{
public:

	

	/*实现网络的初始化
	返回socket
	参数为指定端口：默认为80*/
	int startup(unsigned short *port);

	/*
	主函数：死循环 等待客户发起访问

	*/
	void Main();

	
	~YYServer();
	YYServer();


#ifdef _WIN32
	/*成员函数调用时，默认输入有一个this指针，所以不能直接用作线程回调函数
	因为线程传入的参数没有this指针
	解决办法是将其设置为静态成员函数，不属于某个对象，属于整个类，没有this指针
	但是这样就不能调用非静态的成员变量*/
	
#else
	 int setnonblocking(int fd);
	 void addfd(int fd);
	 void delfd( int fd);
	

#endif // _WIN32

	//从指定的客户端socket读取一行数据保存到buf，返回实际读取到的字节数
	 int get_line(int clientfd, char* buf, int size);
	void error_die(const char* err);
	 std::string get_path(int clientfd,std::string& res);
	 void unimplement(int clientfd);
	 void not_found(int clientfd);
	//http响应头信息
	 void headers(int clientfd,std::string type);
	//发送文件
	 void Clean_recv(int client);
	 int recive(int clientfd,char* buf, int len);
	 int send_to(int clientfd,const char* buf, int len);
	 void cat(int client,std::fstream &fp);
	 void server_file(int client,std::string &path);
	 void execute_cgi(int client, std::string path, std::string method, std::string query_string);
	 void bad_request(int client);
	 std::string replaceStr(std::string str, const char* replaceValue, const char* destValue);
	 void cannot_execute(int client);
private:
	



};
#endif

