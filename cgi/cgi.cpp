#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
using namespace std;
int main(int argc, char* argv[])
{
	
#ifdef _WIN32
	stringstream ss;
	vector<string> cmd;
	if (argc == 1) return 0;
	string c;
	for (int i = 1; i < argc; i++)
	{
		c = argv[i];
		cmd.push_back(c);
	}
#else 
	stringstream ss;
	vector<string> cmd;
	if (argc == 0) return 0;
	string c;
	for (int i = 0; i < argc; i++)
	{
		c = argv[i];
		cmd.push_back(c);
	}
#endif 
	

	if (cmd.size()==1)
	{
		string color,color_up;
		int pos = cmd[0].find('=');
		color = cmd[0].substr(pos+1,cmd[0].size()-1-pos);
		color_up = color;
		transform(color_up.begin(), color_up.end(), color_up.begin(), ::toupper);

		string html="<html>\n<head>\n<title>";
		html += color_up;
		html += "</title>\n<meta http - equiv = \"Content-Type\" content = \"text/html; charset=iso-8859-1\"/>\n</head>\n<body bgcolor = ";
		html += color;
		html += ">\n<h1>This is ";
		html += color;
		html += "</h1>\n</body>\n</html>";
		std::cout << html << std::endl;

	}
	else if (cmd.size() == 3)
	{
		string company,email, post;
		int pos = cmd[0].find('=');
		company = cmd[0].substr(pos + 1, cmd[0].size() - 1 - pos);
		pos = cmd[1].find('=');
		email = cmd[1].substr(pos + 1, cmd[1].size() - 1 - pos);
		pos = cmd[2].find('=');
		post = cmd[2].substr(pos + 1, cmd[2].size() - 1 - pos);

		string html = "<html>\n<head>\n<meta charset = \"gb2312\" / >\n<title>THANKS</title>\n";
		html += "</head>\n<body>\n<div style = \"margin:0 auto; background:red; width:800px;\">\n";
		html += "<img src = \"images/head.png\">\n<h1 style = \"margin:20px; \">\n";
		html += "×ð¾´µÄÁìµ¼£º<br/>&nbsp; &nbsp; &nbsp; &nbsp; ÄúºÃ£¡ÏûÏ¢ÒÑÊÕµ½£¬¸ÐÐ»ÄúµÄÑûÇë¡£<br/>\n";
		html += "&nbsp; &nbsp; &nbsp; &nbsp; ÎÒ¶Ô¹ó¹«Ë¾µÄ";
		html += post;
		html += "Èí¼þ¹¤³ÌÊ¦Ö°Î»ºÜ¸ÐÐËÈ¤¡£ <br/>\n&nbsp; &nbsp; &nbsp; &nbsp;";
		html += company;
		html += "ÊÇÒ»¼ÒºÜ°ôµÄ¹«Ë¾£¬Ï£ÍûÓÐ»ú»á¼ÓÈëÄúµÄÍÅ¶Ó,ÎÒ»áÈÏÕæ¿¼ÂÇÄúµÄÑûÇë£¬ÔÚ3ÈÕÄÚ·¢ËÍµç×ÓÓÊ¼þµ½ÄúµÄÓÊÏä(";
		html += email;
		html += ").<br/><br/>\n";

		auto now = std::chrono::system_clock::now();
		//Í¨¹ý²»Í¬¾«¶È»ñÈ¡Ïà²îµÄºÁÃëÊý
		uint64_t dis_millseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
			- std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() * 1000;
		time_t tt = std::chrono::system_clock::to_time_t(now);
		struct tm time_tm;
#ifdef _WIN32
		localtime_s(&time_tm, &tt);
#else 
		localtime_r(&tt, &time_tm);
#endif

		html += "&nbsp; &nbsp; &nbsp; &nbsp;";
		html += to_string(time_tm.tm_year + 1900);
		html += "Äê";
		html += to_string(time_tm.tm_mon + 1);
		html+= "ÔÂ";
		html += to_string(time_tm.tm_mday);
		html += "ÈÕ ";
		html += to_string(time_tm.tm_hour);
		html += ":";
		html += to_string(time_tm.tm_min);
		html += "  Rock<br/> </br>\n";
		html += "</h1>\n</div>\n</body>\n</html>";
		std::cout << html << std::endl;

		return 0;
	}


		
}