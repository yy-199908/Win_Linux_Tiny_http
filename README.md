# 基于tinyhttp的双系统web服务器

## 项目特点

- 包含get和post两种应答方法
- 采用REACTOR模式
- 采用子线程和STDOUT重定向实现cgi服务器
- windows版本基于Visual Studio编写



## 文件

- ./HTTP httpserver源代码
- ./cgi cgi服务器vs项目
- ./HTTP/htdocs 网页资源文件
  - cgi为linux系统下cgi服务器可执行文件
  - cgi.exe为windows系统下可执行文件由 ./cgi 输出
- 文件执行路径必须是./HTTP





## 执行

- windows执行 ./HTTP/HTTP.exe
- linux执行 make  ./httpd



## 仍需改进，有时间再做

- log系统

- 可以结合YYMsql与mysql交互

- 不会写perl等前端脚本，cgi是个拼接html字符串的c++程序

- linux下proactor模式的实现

- 高并发场景(ps:多线程加多进程应该是够用一些) 

- 考虑到每个连接只有在连接时发送请求和回复然后立即关闭，不存在第二次同连接通信情况，所以没有用epoll等i\o复用。

  但是http 1.1 好像存在keep alive字段，可以尝试epoll监听长连接，而不是每次调用新连接

## 遇到的问题

- 虚拟机中运行程序，在虚拟机中访问127.0.0.1能够成功访问。但是windows下访问其ip老是出现浏览器发送请求服务器没accept到的情况。理论上说不存在填满accept等待队列的情况，目前暂未解决
- WINDOWS下的各种调用真的复杂啊，比如CreateProcess传入的cmd必须是宽字节指针，操，搞了尼玛半天
- 总之很多问题，解决了之后也做了详细注释，，，，
