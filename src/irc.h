#ifndef IRC_H
	#define IRC_H
	#include "types.h"
	void ircStartup();
	uint8_t ircSendMessage(std::string Message,std::string Destionation);
	uint8_t ircSendME(std::string Message,std::string Destionation);
	bool ircChangeNick(std::string name);
	uint8_t ircJoinChannel(std::string Channel);
	uint8_t ircPartChannel(std::string Channel);
	uint8_t ircWhois(std::string User);
	void ircEnd(std::string Message);
	int16_t channelID(std::string Channel);
	bool isChannel(std::string Channel);
	void ircKick(std::string Nick,std::string Channel);
	void ircWho(std::string Target);
	uint8_t ircSendNOTICE(std::string Message,std::string Destionation);
#endif
