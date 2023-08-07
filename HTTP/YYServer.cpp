#include "YYServer.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <chrono>


int epollfd = -1;
int listenfd = -1;


using namespace std;
#ifdef _WIN32
#define PRINTF(client,str) printf("ThreadID: %d  client-%d [%s-%d]: "#str" = %s\n",GetCurrentThreadId(),client,__func__,__LINE__,str);
 DWORD  WINAPI thread_main(LPVOID arg)
{
	 //读取一行数据 GET / HTTP/1.1\n

	 YYServer*my=new YYServer();
	char buf[BUFFER_SIZE]= { 0 };
	int  client = (int)arg;
	int numchar= my->get_line(client,buf,sizeof(buf));
	if (numchar == 0)
	{
		my->Clean_recv(client);
		closesocket(client);
		return 0;
	}
	

	PRINTF(client, buf);
	 //字符串分割 res[0]="GET"  res[1]="/" res[2]="HTTP/1.1"
	vector<string> res;
	istringstream ss(buf);
	string token;
	while (ss >> token) res.push_back(token);


	if (!strcmp(res[0].c_str(), "GET"))
	{

		auto pos = res[1].find('?');
		if (pos == string::npos)
		{
			my->Clean_recv(client);
			string path = my->get_path(client, res[1]);
			if (path == "not_found")
			{
				my->not_found(client);
				closesocket(client);
				return 0;
			}
			my->server_file(client, path);
		}
		else
		{
			string query_string = res[1].substr(pos + 1, res[1].size() - pos - 1);
			res[1] = res[1].substr(0, pos);
			my->Clean_recv(client);
			string path = my->get_path(client, res[1]);
			if (path == "not_found")
			{
				my->not_found(client);
				closesocket(client);
				return 0;
			}
			my->execute_cgi(client, path, res[0], query_string);

		}





		
		closesocket(client);

		return 0;

	}
	else if (!strcmp(res[0].c_str(), "POST"))
	{

		string path = my->get_path(client, res[1]);

		if (path == "not_found")
		{
			my->not_found(client);
			closesocket(client);
			return 0;
		}

		my->execute_cgi(client, path, res[0], "");
	
		closesocket(client);

		return 0;
	}
	else
	{
		my->Clean_recv(client);
		my->unimplement(client);

	
		closesocket(client);
		return 0;

	}





	return 0;
}






#else 
#define PRINTF(client,str) printf("client-%d [%s-%d]: "#str" = %s\n",client,__func__,__LINE__,str);





int YYServer::setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}


void  YYServer::addfd( int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	//setnonblocking(fd);
}


void  YYServer::delfd( int fd)
{
	epoll_event event;
	event.data.fd = fd;
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event);

}


 void* worker(void* arg)
{
	 char buf[BUFFER_SIZE];
	 YYServer* my = new YYServer();
	 int client = *((int*)arg);
	 
	 int numchar = my->get_line(client, buf, sizeof(buf));
	 if (numchar == 0)
	 {
		 my->Clean_recv(client);
		 close(client);
		 return 0;
	 }
	 PRINTF(client,buf);
	 //字符串分割 res[0]="GET"  res[1]="/" res[2]="HTTP/1.1"
	 vector<string> res;
	 istringstream ss(buf);
	 string token;
	 while (ss >> token) res.push_back(token);

	 if (!strcmp(res[0].c_str(), "GET"))
	 {
		 
		 auto pos = res[1].find('?');
		 if (pos == string::npos)
		 {
			 my->Clean_recv(client);
			 string path = my->get_path(client, res[1]);
			 if (path == "not_found")
			 {
				 my->not_found(client);
				 close(client);
				 return nullptr;
			 }
			 my->server_file(client, path);
			 close(client);
		 }
		 else
		 {
			 string query_string = res[1].substr(pos+1,res[1].size()-pos-1);
			 res[1] = res[1].substr(0, pos);
			 my->Clean_recv(client);
			 string path = my->get_path(client, res[1]);
			 if (path == "not_found")
			 {
				 my->not_found(client);
				 close(client);
				 return nullptr;
			 }
			 my->execute_cgi(client, path, res[0], query_string);
			 close(client);
		 }
	 }
	 else if (!strcmp(res[0].c_str(), "POST"))
	 {
		
		 string path = my->get_path(client, res[1]);
		
		 if (path == "not_found")
		 {
			 my->not_found(client);
			 close(client);
			 return nullptr;
		 }
		 
		 my->execute_cgi(client, path, res[0], "");
		
		 close(client);
		 
		 return nullptr;
	 }
	 else
	 {
		 my->Clean_recv(client);
		 my->unimplement(client);
		
		
		 close(client);
		 return nullptr;

	 }





	 return 0;
}

#endif





 int YYServer::recive(int client,char* buf, int len)
 {
#ifdef _WIN32
	 int n=recv(client,buf,len,0);
#else
	 int n = recv(client, buf, len, 0);
	 if (!n)
	 {
		// delfd(client);
		 close(client);
		 exit(0);
	 }
#endif
	 return n;


 }
 int YYServer::send_to(int client,const char* buf, int len)
 {
#ifdef _WIN32
	 int n=send(client, buf, len, 0);
#else
	 int n = send(client, buf, len, 0);
	 if (!n)
	 {
		// delfd(client);
		 close(client);
	 }
#endif
	 return n;
 }



void YYServer::error_die(const char* err)
{
	cerr << err <<" error!" << endl;
	
}


YYServer::YYServer()
{}
YYServer::~YYServer()
{}
int YYServer::startup(unsigned short *port)
{
	if (*port < 0)
	{
		error_die("port < 0");
		return -1;
	}

	
	//1.windows需要网络初始化
#ifdef _WIN32
	WSADATA data;
	int ret=WSAStartup(
		MAKEWORD(1,1),//1.1版本协议
				&data);
	if (ret)
	{
		error_die("WSAStartup");
		return -1;
	}
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	int ret = 0;
#endif

	//2.创建套接字
	
	if (fd == -1) {
		error_die("socket");
	}
	//3.设置套接字属性
	//(预防套接字假死)  设置套接字端口可复用
	int opt = 1;
	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
	if (ret == -1) error_die("setsockopt");


	//4.绑定套接字与网络地址
	sockaddr_in server_addr;
	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(*port);
	server_addr.sin_addr.s_addr = 0;//可访问的ip，表示所有ip可访问
	ret = bind(fd, (const sockaddr*)&server_addr, sizeof(server_addr));
	if (ret < 0) error_die("bind");
	//5.创建监听队列
	

	ret = listen(fd,10);
	if (ret < 0) error_die("listen");


	if (*port == 0)//动态获取端口号
	{
		
		#ifdef _WIN32
		int len = sizeof(server_addr);
		#else 
		socklen_t len = sizeof(server_addr);
		#endif
		ret = getsockname(fd, (sockaddr*)&server_addr, &len);
		if(ret<0) error_die("getsockname");
		*port = server_addr.sin_port;
	}

	listenfd = fd;
	return fd;
}



void YYServer::Main()
{
	if (listenfd < 0) {
		error_die("You must startup net frist!");
		return;
	}
#ifdef _WIN32
	//window不支持epoll,使用REACTOR模式，主线程只负责accept，子线程负责读写
	sockaddr_in client_addr;
	int len = sizeof(client_addr);
	memset(&client_addr, '\0', len);
	int fd = -1;

	while (1)
	{
		  fd= accept(listenfd, (sockaddr*)&client_addr, &len);
		if (fd == -1)
		{
			error_die("accept");
		}
		//创建一个新的线程，用于接待client
		

		DWORD thread_id = 0;
		CreateThread(0, 0, thread_main, (LPVOID)fd, 0, &thread_id);


	}
	closesocket(listenfd);
#else 
	//linux使用PREACTOR 模式，主线程负责所有io操作，子线程负责业务逻辑


	/*sigset_t sigt,osig_t;
	sigemptyset(&sigt);
	sigaddset(&sigt, SIGPIPE);
	sigprocmask(SIG_SETMASK,&sigt,&osig_t);*/

	/*epoll_event events[MAX_EVENT_NUMBER];
	epollfd = epoll_create(1);
	if(epollfd==-1) error_die("epoll_create");
	addfd(listenfd);*/
	int i = 0;
	while (1)
	{
		//int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		//if (ret < 0)
		//{
		//
		//	error_die("epoll_wait"); 
		//	break;
		//}
		//
		//for (int i = 0; i < ret; i++)
		//{
		//	int sockfd = events[i].data.fd;
		//	if (sockfd == listenfd)//若是listenfd有事件，则是新连接
		//	{
		//		struct sockaddr_in client_address;
		//		socklen_t client_addrlength = sizeof(client_address);
		//		int client = accept(listenfd, (struct sockaddr*)&client_address, &
		//			client_addrlength);
		//		/*对每个非监听文件描述符都注册EPOLLONESHOT事件*/
		//		addfd(client);
		//		pthread_t thread;
		//		pthread_create(&thread, NULL, worker, (void*)&client);
		//		
		//	}
		//	
		//	
		//}

		struct sockaddr_in client_address;
		int len = sizeof(client_address);
		memset(&client_address, '\0', len);
		socklen_t client_addrlength = sizeof(client_address);
		int client = accept(listenfd, (struct sockaddr*)&client_address, &
			client_addrlength);
		/*对每个非监听文件描述符都注册EPOLLONESHOT事件*/
		//addfd(client);
		pthread_t thread;
		pthread_create(&thread, NULL, worker, (void*)&client);
		i++;
		cout << "已经连接数:" <<i<< endl;
				

	}

	close(listenfd);

#endif // 
	
}


int YYServer::get_line(int client,char* buf, int size)
{
	char c = '\0';
	int index = 0;
	//http 协议以\r\n结尾
	while (index<size-1 and c!='\n')
	{

		int n = recv( client,&c, 1,0);
		if (n > 0)
		{
			if (c == '\r')
			{
				n = recv(client, &c, 1, MSG_PEEK);//不读出来，瞄一眼（缓冲区仍然保留该字符）
				
				if (n > 0 and c == '\n')
				{
					recv(client, &c, 1, 0);
	
				}
				else
				{
					c = '\n';
				}
			}
			buf[index++] = c;
		}
		else 
		{
			/*if(errno== EAGAIN or errno== EINTR)
			else
			close()*/
			break;
		}
	}
	buf[index] = '\0';

	return index;
}


void YYServer::Clean_recv(int client)
{
	char buf[BUFFER_SIZE] = { 0 };
	int numchar = 1;
	while (numchar > 0 and strcmp(buf, "\n"))
	{
		numchar = get_line(client, buf, sizeof(buf));

		if (numchar == -1)
		{
			printf("remote close!\n");
			return;
		}
	}
}





string YYServer::get_path(int client,string &res)
{
#ifdef _WIN32
	string url = "htdocs";
#else 
	string url = "./htdocs";
#endif
	
	if (res == "/")
	{
		url += "/index.html";
	}
	else
	{
		url += res;
	}
	struct stat status;
	if (stat(url.c_str(), &status) == -1)
	{
		return "not_found";
	}
	if ((status.st_mode & S_IFMT) == S_IFDIR)
	{
		url += "/index.html";
	}
	return url;
}




 void YYServer::server_file(int client,string& path)
{
	 //清除残留未读取数据
	
	 //分割字符串获得文件类型
	 
	 bool img = 0;
	 stringstream ss(path);
	 fstream fp;
	 string str;
	 while (getline(ss,str,'.'))
	 {}
	 string type;
	 if (str == "css") type = "text/css";
	 else if(str=="jpg")
		 type = "image/jpeg";

	 else if (str == "png")type = "image/png";
	 else if (str == "js")type = "application/x-javascript";
	 else if (str == "html")type = "text/html";
	 if (str == "html")
	 
		  fp.open(path.c_str(), ios::in);
	 else
	 {
		 fp.open(path.c_str(), ios::in | ios::binary);
		 img = true;
	 }
	
	 if (!fp.is_open())
	 {
		 cout << "open"<<path<<" failed!" << endl;
		 return;
	 }

	 //http头
	 
	 headers(client,type);
	 cat(client,fp);
	 fp.close();
	 
	 printf( "send %s ok!\n",path.c_str());
 }



 void YYServer::unimplement(int clientfd)
 {
	 //


	 string head="HTTP / 1.0 501 Method Not Implemented\r\n";
	 send(clientfd, head.c_str(), head.size(), 0);
		head= "Server: qitiandasheng/1.0\r\n";
		send(clientfd, head.c_str(), head.size(), 0);
		head = "Content-Type: text/html\r\n";
	 send(clientfd, head.c_str(), head.size(), 0);
	 head = "\r\n";
	 send(clientfd, head.c_str(), head.size(), 0);
	 head = "<HTML><HEAD><TITLE>Method Not Implemented\r\n";
	 send(clientfd, head.c_str(), head.size(), 0);
	 head = "</TITLE></HEAD>\r\n";
	 send(clientfd, head.c_str(), head.size(), 0);
	 head = "<BODY><P>HTTP request method not supported.\r\n";
	 send(clientfd, head.c_str(), head.size(), 0);
	 head = "</BODY></HTML>\r\n";
	 send(clientfd, head.c_str(), head.size(), 0);
 }

 void YYServer::not_found(int clientfd)
 {
	 string header = "HTTP/1.1 404 Not Found\r\n";
	 int ret = send_to(clientfd, header.c_str(), header.size());


	 header = "Server: qitiandasheng/1.0\r\n";
	 ret = send_to(clientfd, header.c_str(), header.size());


	 auto now = std::chrono::system_clock::now();
	 //通过不同精度获取相差的毫秒数
	 uint64_t dis_millseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
		 - std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * 1000;
	 time_t tt = std::chrono::system_clock::to_time_t(now);
	 struct tm time_tm;
#ifdef _WIN32
	 localtime_s(&time_tm, &tt);
#else 
	 localtime_r(&tt, &time_tm);
#endif
	 stringstream ss;
	 ss << time_tm.tm_year + 1900 << "-" << time_tm.tm_mon + 1 << "-" << time_tm.tm_mday;
	 ss << " " << time_tm.tm_hour << ":" << time_tm.tm_min << ":" << time_tm.tm_sec;
	 header = "Date: ";
	 header += ss.str();
	 header += "\r\n";
	 ret = send_to(clientfd, header.c_str(), header.size());
	 header = "Content-type:text/html\r\n";
	 ret = send_to(clientfd, header.c_str(), header.size());
	 if (ret == 0) return;
	 header = "\r\n";
	 ret = send_to(clientfd, header.c_str(), header.size());
	 if (ret == 0) return;
	 
	  fstream fp("./htdocs/404.html",ios::in);
	  if (!fp.is_open()) return;
	  cat(clientfd, fp);
	  printf("Not found\n");;

 }


 void YYServer::headers(int clientfd,string type)
 {
	
	string header = "HTTP/1.1 200 OK\r\n";
	int ret = send_to(clientfd,header.c_str(),header.size());


	header = "Server: qitiandasheng/1.0\r\n";
	 ret = send_to( clientfd,header.c_str(), header.size());


	auto now = std::chrono::system_clock::now();
	//通过不同精度获取相差的毫秒数
	uint64_t dis_millseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
		- std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * 1000;
	time_t tt = std::chrono::system_clock::to_time_t(now);
	struct tm time_tm;
	#ifdef _WIN32
		localtime_s(&time_tm,&tt);
	#else 
		localtime_r( &tt, &time_tm);
	#endif
	stringstream ss;
	ss << time_tm.tm_year + 1900 << "-" << time_tm.tm_mon + 1 << "-" << time_tm.tm_mday;
	ss << " " << time_tm.tm_hour << ":" << time_tm.tm_min << ":" << time_tm.tm_sec;
	header = "Date: ";
	header += ss.str();
	header += "\r\n";
	 ret = send_to( clientfd,header.c_str(), header.size());
	header = "Content-type:";
	header += type;
	header += "\r\n";
		
	 ret = send_to( clientfd,header.c_str(), header.size());
	 if (ret == 0) return;
	header = "\r\n";
	ret = send_to( clientfd,header.c_str(), header.size());
	if (ret == 0) return;

	
 }

void YYServer::cat(int client,fstream& fp)
 {
	//一次发4096个字节
	long long count = 0;
	fp.seekg(0, fp.end);
	int length = fp.tellg();
	fp.seekg(0, fp.beg);
	char *pic=new char[length];
	int ret = 0;
	fp.read(pic, length);//如果使用fp>>line，会忽略空格
	ret = send_to(client,pic, length);
	count += length;
	delete [] pic;
	printf("send %d byte data!\n",count);
	
 }



string YYServer::replaceStr(string str, const char* replaceValue, const char* destValue)
{
	int pos = -1;
	while ((pos = str.find(replaceValue)) >= 0)
	{
		//LOGD("pos :%d",pos);
		str.erase(pos, strlen(replaceValue));
		if (destValue != NULL)
		{
			str.insert(pos, destValue);
		}
		pos = -1;
	}
	return str;
}


void YYServer::execute_cgi(int client, string path, string method, string query_string)
{
	char buf[1024];
	int pid;
	int numchar = 1;
	int content_length = -1;
	
	int status;
	char c;
	string cmd;
	vector<string> cmds;
	if (method == "GET")
	{
		Clean_recv(client);
	}
	else if (method == "POST")
	{
		numchar = get_line(client, buf, sizeof(buf));
		while (numchar > 0 and strcmp(buf, "\n"))
		{
			buf[15] = '\0';
			if (strcmp(buf, "Content-Length:") == 0)
				content_length = atoi(&buf[16]);
			numchar = get_line(client, buf, sizeof(buf));
		}


		if (content_length == -1) {
			bad_request(client);
			return;
		}

	}
	for (int i = 0; i < content_length; i++) {
		recv(client, &c, 1, 0);
		cmd += c;
	}
	cmd = replaceStr(cmd, "%40", "@");
	cmd = replaceStr(cmd, "%2B", "+");
	stringstream ss(cmd);
	while (getline(ss, cmd, '&')) cmds.push_back(cmd);
	char* argv[cmds.size() + 1];

	for (int i = 0; i < cmds.size(); i++)
	{
		argv[i] = (char*)cmds[i].c_str();
		cout << argv[i];
	}
		
	argv[cmds.size()] = NULL;


	
#ifdef _WIN32
	HANDLE cgi_output[2];

	SECURITY_ATTRIBUTES la;
	la.nLength = sizeof(la);
	la.bInheritHandle = true;
	la.lpSecurityDescriptor = 0;
	BOOL pipe_ok = CreatePipe(&cgi_output[0], &cgi_output[1], &la, 0);
	if (pipe_ok == false)
	{
		
		cannot_execute(client);
		return;
	}

	


	cmd = path;
	for (int i = 0; i < cmds.size(); i++)
	{
		cmd += " ";
		cmd += cmds[i];
		
	}
	cmd = replaceStr(cmd,"/","\\");
	//必须要将执行命令转化为双字节字符串
	char* ptr = (char*)cmd.c_str();
	int pSize = MultiByteToWideChar(CP_OEMCP, 0, ptr, strlen(ptr) + 1, NULL, 0);
	wchar_t* L_cmd = new wchar_t[pSize];
	//第二次调用将单字节字符串转换成双字节字符串
	MultiByteToWideChar(CP_OEMCP, 0, ptr, strlen(ptr) + 1, L_cmd, pSize);
	

	
	PROCESS_INFORMATION pi= { 0 };
	STARTUPINFO si = { 0 };
	
	si.cb = 0;
	si.hStdOutput = cgi_output[1];
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	pipe_ok=CreateProcess(NULL, L_cmd, 0, 0, TRUE, 0, 0, 0, &si, &pi);
	if (pipe_ok == false)
	{
		cout << "\ncreate process false\n";
		cout << GetLastError() << endl;
		
		cannot_execute(client);
		return;
	}
	char buff[1024] = {0};
	DWORD size;
	headers(client, "text/html");
	
	
	if (ReadFile(cgi_output[0], buff, sizeof(buff), &size, NULL))
	{
		
		int ret=send(client, buff, sizeof(buff), 0);
		cout << buff;
		if (ret)
		{
			cout << "send ok!" << endl;
		}
	}
	

	delete[] L_cmd;
	


#else
	int cgi_output[2];
	int cgi_input[2];
		if (pipe(cgi_output) < 0)
		{
			cannot_execute(client);
			return;
		}
		if (pipe(cgi_input) < 0)
		{
			cannot_execute(client);
			return;
		}

		if ((pid = fork()) < 0) {
			cannot_execute(client);
			return;
		}

		
		//cgi_server(client,pid,path,method,query_string,content_length);
		

		if (pid == 0)  /* child: CGI script */
		{
			char meth_env[255];
			char query_env[255];
			char length_env[255];

			dup2(cgi_output[1], 1);
			dup2(cgi_input[0], 0);
			
			close(cgi_output[0]);
			close(cgi_input[1]);
			sprintf(meth_env, "REQUEST_METHOD=%s", method);
			putenv(meth_env);
			if (strcasecmp(method.c_str(), "GET") == 0) {
				sprintf(query_env, "QUERY_STRING=%s", query_string);
				putenv(query_env);
			}
			else {   /* POST */
				sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
				putenv(length_env);
			}
			
			
			execv(path.c_str(), argv);
		
			close(cgi_output[1]);
			close(cgi_input[0]);
			exit(0);
		}
		else {    /* parent */
		
			
			close(cgi_output[1]);
			close(cgi_input[0]);
			headers(client, "text/html");
			while (read(cgi_output[0], &c, 1) > 0)
			{
				
				send(client, &c, 1, 0);
			}
				

			close(cgi_output[0]);
			close(cgi_input[1]);
			waitpid(pid, &status, 0);
		}
		


#endif
		
	
}





void YYServer::bad_request(int client)
{
	headers(client, "text/html");
	string buf;
	buf = "HTTP/1.0 400 BAD REQUEST\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf="Content-type: text/html\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf="\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf="<P>Your browser sent a bad request, ";
	send(client, buf.c_str(), buf.size(), 0);
	buf="such as a POST without a Content-Length.\r\n";
	send(client, buf.c_str(), buf.size(), 0);
}


void YYServer::cannot_execute(int client)
{
	headers(client, "text/html");
	string buf;
	buf = "HTTP/1.0 500 Internal Server Error\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "Content-type: text/html\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "\r\n";
	send(client, buf.c_str(), buf.size(), 0);
	buf = "<P>Error prohibited CGI execution.\r\n";
	send(client, buf.c_str(), buf.size(), 0);

}