#include <iostream>
#include "tools.h"


//Output a message with title
bool OuputMessage(std::string message,std::string title,std::string color){
	if (!message.length() or !title.length())
		return 0;
	color_set(color.c_str());
	std::cout << "[" << title << "]: ";
	color_set("white");
	std::cout << message << std::endl;
    return 1;
}
