#ifndef TYPES_H
	#define TYPES_H
	#include <string>
	#include <vector>
	#include <stdint.h>
	static std::string tNull = "#null";
	//typeUser
	struct typeUser{
		//User data
		std::string Nick;				//User nick
		std::string Username;			//User name
		std::string Host;				//User host
		//User status
		bool Temporal;					//Temporal user
		std::string Channel;			//Last active channel
		uint8_t userLevel;				//User level 1-5. 3 and further are rootUsers.
		bool Ignore;					//Ignore the user or not
		unsigned LastActivity;			//Last time the user was active
		std::string LastActive;			//Last active
		unsigned Lines;					//Lines Number
		unsigned PaycheckT;				//Lastpaycheck
		std::string MostUsedWord;		//Most used word
		unsigned AvgWrdsPerLine;		//Average number of words per line
		bool Identified;
		//RP
		int64_t Money;					//Money for Ropeplay
		//Auto-unignore
		bool autoUnignore;
		unsigned autoTimeout;
		//Messaging control flow
		bool AutoKick_Ignored;
		unsigned cMSG;
		unsigned cStrTimeout;
		bool Imitate;
		std::string ImitateTo;
	};
	//typeList
	typedef std::vector<std::string> typeList; 
	//typeIdent
	struct typeIdent{
		std::string Nick;
		std::string Username;
		std::string Host;
	};
	//typeMessage
	struct typeMessage{
		std::string Nick;
		std::string Username;
		std::string Host;
		std::string Message;
		std::string Channel;
		bool Type; 					//0 = Channel message type, 1 = Private Message
	};
	//typeChannel
	struct typeChannel{
		std::string Channel;
		std::string Cmdprefix;
		typeList Users;
		bool OP;	// Operator
		/* Features */
		bool URLShrinker;
		bool WebTitle;
		bool AntiSpam;
		bool AntiSpamN;
		
	};
	//Price tags
	struct p_tag{
		unsigned Yiff;
		unsigned Told;
		unsigned Insult;
		unsigned Trivia;
		unsigned Quote;
		unsigned Kick;
	};
	//Config
	struct ConfigData {
		unsigned Ignore_timeout;	//Ignore global timeout
		unsigned Max_mpm;			//Max messages per minute
		std::string Binary;
		std::string Parameters;
		std::string rtdir;
		std::string Logfilename;
		uint64_t Uptime;
		bool enableIRC;				//IRC client
		bool enMinput;				//Manual Input (console input)
		bool enableLog;				//Log
		//Ignore hosts
		typeList IgnoreHosts;
		//IRC
		std::string clientName;
		std::string ircServer;
		std::string ircPort;
		std::vector <typeChannel> Channels;
		std::string ircNick;
		bool IRCAutoJoin;	
		bool IRCAutoConnect;
		//Prices
		p_tag Prices;
		//Users
		std::vector <typeUser> Users;
		//Yiffes
		typeList Yiff;
		//Trivia
		typeList Trivia;
		//Told
		typeList Told;
		//Insults
		typeList Insults;
		//Insults
		typeList Quotes;
	}; 
#endif
