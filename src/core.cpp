#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include "core.h"
#include "ai.h"
#include "irc.h"
#include "tools.h"
#include "commandline.h"
#include "output.h"
#include "types.h"


ConfigData Config;

void coreAddLog(std::string message);

#define ProgramName std::string("Helper")
#define Logfilename std::string("corelog.txt");


	// // // User Functions // // //
	
bool coreSolveNickCollision(std::string nick){
	bool F = 0;
	for (uint16_t i=0;i<Config.Users.size();i++){
		if (string_to_lower(nick)==string_to_lower(Config.Users[i].Nick)){
			Config.Users[i].Nick="";
			F = 1;
		}
	}
	return F;
}
int coreUserIDfromNick(std::string Nick){
	std::string nick=string_to_lower(Nick);
	for (uint16_t i=0;i<Config.Users.size();i++){
		if(string_to_lower(Config.Users[i].Nick)==nick){
			return i;
		}
	}
	return -1;	
}

int coreUserID(std::string Username){
	for (uint16_t i=0;i<Config.Users.size();i++){
		if(string_to_lower(Config.Users[i].Username)==string_to_lower(Username)){
			return i;
		}
	}
	return -1;
}



bool coreRefreshUserData(std::string Username){
	int ID = coreUserID(Username);
	if (ID==-1)
		return 0;
	typeUser &User = Config.Users[ID];
	if (User.Temporal)
		return 1;
	std::string f = Config.rtdir+"users/"+User.Username+".txt";
	std::ofstream File;
	User.LastActivity = get_tocks();
	File.open(f.c_str());
	File << User.Nick << std::endl;
	File << User.Username << std::endl;
	File << User.Host << std::endl;
	File << int_to_string(User.userLevel) << std::endl;
	File << int_to_string(User.Lines) << std::endl;
	File << int_to_string(User.Money) << std::endl;
	File << int_to_string(User.PaycheckT) << std::endl;
	File << int_to_string(User.LastActivity) << std::endl;
	File << User.Channel << std::endl;
	File << User.LastActive << std::endl;
	File << int_to_string(User.Ignore) << std::endl;
	File << int_to_string(User.autoUnignore) << std::endl;
	File << int_to_string(User.autoTimeout) << std::endl;
	File << int_to_string(User.cMSG) << std::endl;
	File << int_to_string(User.cStrTimeout) << std::endl;
	File.close();
	return 1;	
	
}
uint16_t coreUserAdd(typeUser *Data){
	uint16_t ID = Config.Users.size();
	Config.Users.push_back(typeUser());
	Config.Users[ID]=(*Data);
	return ID;
}

bool coreisrootUser(std::string Username){
	int ID=coreUserID(Username);
	if (ID==-1)
		return 0;
	if (Config.Users[ID].userLevel>=3)
		return 1;
	return 0;
}

bool coreUserDelete(std::string Username){
	int ID = coreUserID(Username);
	if (ID==-1)
		return 0;
	typeUser &User = Config.Users[ID];
	//Delete file (if he's not temporal)
	if (!User.Temporal){
		std::string f = Config.rtdir+"users/"+User.Username+".txt";
		unlink(f.c_str());
	}
	//Delete from memory
	Config.Users.erase(Config.Users.begin()+ID);
	return 1;
}
/* Add a user as temporal */
int coreAddUserTMP(std::string Username,std::string Host,std::string Nick){
	if (coreUserID(Username)!=-1)
		return -1;
	typeUser Data;
	Data.Temporal = 1;
	Data.Username=string_to_lower(Username);
	Data.Host=Host;
	Data.Ignore=0;
	Data.Nick=Nick;
	Data.userLevel=1;
	Data.LastActivity=get_tocks();
	Data.Lines=0;
	Data.Identified=0;
	Data.Money=200;
	Data.LastActive=tNull;
	Data.Channel=tNull;
	Data.cMSG = 0;
	Data.Imitate=0;
	Data.AutoKick_Ignored=0;
	Data.ImitateTo=tNull;
	coreSolveNickCollision(Data.Nick);
	Data.cStrTimeout = get_tocks();
	uint16_t ID = coreUserAdd(&Data);
	coreAddLog("Created user "+Username+".");
	//Create file
	coreRefreshUserData(Username);
	return ID;
}
/* Add a user */
int coreAddUser(std::string Username,std::string Host,std::string Nick){
	if (coreUserID(Username)!=-1)
		return -1;
	typeUser Data;
	Data.Temporal = 0;
	Data.Username=string_to_lower(Username);
	Data.Host=Host;
	Data.Ignore=0;
	Data.Nick=Nick;
	Data.userLevel=1;
	Data.LastActivity=get_tocks();
	Data.Lines=0;
	Data.Identified=1;
	Data.Money=200;
	Data.cMSG = 0;
	Data.LastActive=tNull;
	Data.Channel=tNull;
	Data.AutoKick_Ignored=0;
	Data.Imitate=0;
	Data.ImitateTo=tNull;
	coreSolveNickCollision(Data.Nick);
	Data.cStrTimeout = get_tocks();
	uint16_t ID = coreUserAdd(&Data);
	//Create file
	coreRefreshUserData(Username);
	return ID;
}

bool coreLoadUsers(){
	if (directory_exists(Config.rtdir+"users/")==0)
		return 0;
	typeList Files;
	files_find(Config.rtdir+"users/","file",".txt",0,0,&Files);
	if (Files.size()==0)
		return 0;
	for (unsigned i=0;i<Files.size();i++){
		std::ifstream File;
		std::string f = Config.rtdir+"users/"+Files[i];
		File.open(f.c_str());
		std::string Buffer;
		typeUser Holder;
		Holder.Identified=0;
		Holder.Temporal=0;
		Holder.Imitate=0;
		Holder.ImitateTo=tNull;
		Holder.LastActive=tNull;
		Holder.Channel=tNull;
		Holder.AutoKick_Ignored=0;
		/* Nick */
		std::getline(File,Buffer);
		Holder.Nick = Buffer;
		/* Username */
		std::getline(File,Buffer);
		Holder.Username = Buffer;
		/* Host */
		std::getline(File,Buffer);
		Holder.Host = Buffer;
		/* userLevel */
		std::getline(File,Buffer);
		Holder.userLevel = string_to_int(Buffer);
		/* Lines */
		std::getline(File,Buffer);
		Holder.Lines = string_to_int(Buffer);
		/* Money */
		std::getline(File,Buffer);
		Holder.Money = string_to_int(Buffer);
		/* Paycheck */
		std::getline(File,Buffer);
		Holder.PaycheckT = string_to_int(Buffer);
		/* Last active */
		std::getline(File,Buffer);
		Holder.LastActivity = string_to_int(Buffer);
		/* Last active Channel */
		std::getline(File,Buffer);
		Holder.Channel = Buffer;
		/* Last Activity */
		std::getline(File,Buffer);
		Holder.LastActive = Buffer;
		/* Ignore */
		std::getline(File,Buffer);
		Holder.Ignore = string_to_int(Buffer);;
		/* autoUnignore */
		std::getline(File,Buffer);
		Holder.autoUnignore = string_to_int(Buffer);
		/* autoTimeout */
		std::getline(File,Buffer);
		Holder.autoTimeout = string_to_int(Buffer);
		/* cMSG */
		std::getline(File,Buffer);
		Holder.cMSG = string_to_int(Buffer);
		/* cStrTimeout */
		std::getline(File,Buffer);
		Holder.cStrTimeout = string_to_int(Buffer);
		
		Config.Users.push_back(Holder);
		File.close();
	}
	coreAddLog("Loaded "+int_to_string(Config.Users.size())+" Users");
	return 1;
}

	// // // Input Processing // // //

int is_ignore_host(std::string host){
	for (unsigned i=0; i<Config.IgnoreHosts.size(); i++){
		std::string h = Config.IgnoreHosts[i];
		if (h==host or host.find(h)!=-1)
			return i;
	}
	return -1;
}

bool ignore_host(std::string host){
	if (is_ignore_host(host)!=-1)
		return 0;
	Config.IgnoreHosts.push_back(host);
	coreSaveSettings();
	return 1;
}

bool unignore_host(std::string host){
	int k = is_ignore_host(host);
	if (k!=-1){
		Config.IgnoreHosts.erase(Config.IgnoreHosts.begin()+k);
		coreSaveSettings();
		return 1;
	}
	return 0;
}
/* This function is defined in commandline.cpp */
std::string getReceiver(typeMessage *Message);

/* Process input as command */
void ProcessCMD(typeMessage *MessageQueue,int userID){
	std::string mUser = MessageQueue->Nick+"("+MessageQueue->Username+"@"+MessageQueue->Host+")";

	typeUser &User = Config.Users[userID];
	
	typeList W;
	split_words(MessageQueue->Message," ",&W);
	
	if (User.Temporal and MessageQueue->Type){
		coreAddLog("Dropped message from "+mUser+"(PM while temporal).");
		return;
	}
	
	/* Ignored? */
	if (W[0]=="!ignored?"){
		if (User.cMSG<1 and User.Ignore){
			ircSendMessage("Yes, you are ignored.",getReceiver(MessageQueue));	
			User.cMSG++;
			coreRefreshUserData(MessageQueue->Username);
			return;
		}
		if (!User.Ignore){
			ircSendMessage("No, you are not ignored.",getReceiver(MessageQueue));	
			User.cMSG++;
			coreRefreshUserData(MessageQueue->Username);
			return;
		}
	}
	
	/* Relog in case the rootuser changed his host and was y ignored */
	if (W.size()>1){
		if (W[0]=="!relog" and Identify(W[1])!=-1){
			unignore_host(MessageQueue->Host);
			ircSendMessage("You've identified and you are no longer ignored.",MessageQueue->Nick);	
			User.Host = MessageQueue->Host;
			User.Ignore = 0;
			coreRefreshUserData(MessageQueue->Username);
		}	
	}
	
	/* Unignore */
	if (User.autoUnignore and User.Ignore){
		if (get_tocks()-User.autoTimeout>Config.Ignore_timeout){
			unignore_host(User.Host);
			User.autoUnignore=0;
			User.cMSG=0;
			User.Ignore=0;
		}
	}
	
	/* Drop message if the message has an ignored host */
	if (is_ignore_host(MessageQueue->Host)!=-1){
		std::string log;
		log = mUser+" tried to reach Helper but he is ignored by host.";
		coreAddLog(log);
		return;
	}	

	if ( (User.Temporal or !User.Identified) and !User.cMSG%2 ){
		ircWho(MessageQueue->Nick);
	}

	if (User.Nick!=MessageQueue->Nick or User.Host!=MessageQueue->Host){
		return;
	}
	
	
	/* Drop message if the user is being ignored */
	if (User.Ignore and !User.AutoKick_Ignored){
		coreAddLog(mUser+" tried to reach Helper but he is ignored.");
		return;
	}
	
	/* Spam control */
	if (User.userLevel<3)
		User.cMSG++;
	else
		User.cMSG=0;
	
	unsigned Max_mpm = ( User.Temporal ? round_int(Config.Max_mpm*0.70) : Config.Max_mpm );
	
	/* Warning */
	if (User.cMSG==round_int(Max_mpm/2.0)){
		std::string pr;
		pr = "Slow down. Otherwise you'll be ignored as a penalty for ";
		pr += seconds2hours(Config.Ignore_timeout)+".";
		ircSendMessage(pr,MessageQueue->Nick);
	}
	
	/* Ignore */
	if (User.cMSG>Max_mpm){
		std::string pr;
		pr = "You are now being ignore for spamming. You'll be unignored in ";
		pr += seconds2hours(Config.Ignore_timeout)+".";
		ircSendMessage(pr,MessageQueue->Nick);
		ignore_host(User.Host);
		User.autoUnignore=1;
		User.Ignore=1;
		User.autoTimeout=get_tocks();
		coreRefreshUserData(MessageQueue->Username);
		return;
	}
	
	/* Timeout, resets counters */
	if (get_tocks()-User.cStrTimeout>60){
		User.cMSG=0;
		User.cStrTimeout=get_tocks();
	}
	
	coreRefreshUserData(MessageQueue->Username);
	/* Run command */
	CommandLine(MessageQueue);
	return;
}
/* Process input as natural language */
void ProcessNL(typeMessage *MessageQueue){
	aiProcessInput(MessageQueue);
	return;
}

/* Process input, decide what to do with it */
void coreInputProcess(typeMessage *MessageQueue){
	if (!MessageQueue->Message.length())
		return;
	
	
	int userID = coreUserID(MessageQueue->Username);
	 	
	/* Drop message if the user doesn't exist */
	if (userID==-1){
		return;
	}
	
	typeUser &User = Config.Users[userID];
	
	User.Lines++;
	
	if (User.Imitate){
		coreAddLog("Imitated "+MessageQueue->Nick+"!");
		std::string To = getReceiver(MessageQueue);
		std::string M = MessageQueue->Message;
		if (User.ImitateTo!=tNull){
			To = User.ImitateTo;
			M = "%blue["+MessageQueue->Nick+"]%reset '"+M+"' from ";
			std::string chan = "%yellow"+MessageQueue->Channel+"%reset";
			M += std::string( !MessageQueue->Type ? chan : "%greenPM")+"%reset.";
		}
		ircSendMessage(M,To);
	}
	
	
	std::string getMessage = MessageQueue->Message;
	
	if (str_at(getMessage,0)=="!" or str_at(getMessage,0)=="." or str_at(getMessage,0)=="~"){
		ProcessCMD(MessageQueue,userID);
	}else{
		ProcessNL(MessageQueue);
	}
	
	coreRefreshUserData(MessageQueue->Username);
	return;
}

	// // // Main core functions // // //


/* Solve Config.rtdir */
void coreSolvertdir(){
	//Load root.txt file
	if (file_exists("root.txt") and !directory_exists(Config.rtdir)){
		std::ifstream File;
		std::string Buffer;
		File.open("root.txt");
		std::getline(File,Buffer);
		File.close();
		Config.rtdir=Buffer;
	}
	
	/* ./ or / indicates that the Config.rtdir is the current path where the program is running */
	if (Config.rtdir=="./" or Config.rtdir=="/"){
		char CurrentPath[FILENAME_MAX];
		getcwd(CurrentPath, sizeof(CurrentPath));
		Config.rtdir=CurrentPath;
	}
		
	if (!directory_exists(Config.rtdir)){
		OuputMessage("rtdir was not specified. "+ProgramName+" Terminated","Error","red");
		endprocess();
	}
	//Add /
	Config.rtdir+="/";
	return;
}

/* change Config.rtdir */
void coreSetrtdir(std::string newDirectory){
	Config.rtdir=newDirectory;
	return;
}

/* Add to log */
void coreAddLog(std::string message){
	if (Config.enableLog==0)
		return;
	std::ofstream Logfile;
	
	std::string File = Config.rtdir+Logfilename;
	
	if (file_exists(File)==0){
		Logfile.open (File.c_str());
	}else{
		Logfile.open (File.c_str(),std::ios::out | std::ios::app);
	}
		
	std::string LogHeader;
	LogHeader="["+time_get("h")+":"+time_get("m")+":"+time_get("s")+"] "+message;
	Logfile << LogHeader << std::endl;
	Logfile.close();
	return;
}

/* Save current settings */
void coreSaveSettings(){
	std::ofstream File;
	std::string ConfigFile = Config.rtdir+"settings.txt";
	File.open(ConfigFile.c_str());
	File << "[enableIRC]" << std::endl;
	File << Config.enableIRC << std::endl;
	File << "[enableManualInput]" << std::endl;
	File << Config.enMinput << std::endl;	
	File << "[enableLog]" << std::endl;
	File << Config.enableLog << std::endl;		
	File << "[ClientName]" << std::endl;
	File << Config.clientName << std::endl;	
	File << "[ircServer]" << std::endl;
	File << Config.ircServer << std::endl;	
	File << "[ircPort]" << std::endl;
	File << Config.ircPort << std::endl;				
	File << "[IRCAutoJoin]" << std::endl;
	File << Config.IRCAutoJoin << std::endl;	
	File << "[IRCAutoConnect]" << std::endl;
	File << Config.IRCAutoConnect << std::endl;	
	File.close();
	ConfigFile = Config.rtdir+"channels.txt";
	File.open(ConfigFile.c_str());
	for (uint8_t i=0;i<Config.Channels.size();i++)
		File << Config.Channels[i].Channel << std::endl;
	File.close();
	ConfigFile = Config.rtdir+"hostbans.txt";
	File.open(ConfigFile.c_str());
	for (uint8_t i=0;i<Config.IgnoreHosts.size();i++)
		File << Config.IgnoreHosts[i] << std::endl;
	File.close();
	coreAddLog("Wrote settings.txt");
	coreAddLog("Wrote channels.txt");
	coreAddLog("Wrote hostbans.txt");
}
/* Load settings */
void coreLoadSettings(){
	std::ifstream File;
	std::string ConfigFile;
	std::string Buffer;
	//Load settings.txt
	ConfigFile = Config.rtdir+"settings.txt";
	if (file_exists(ConfigFile)==1){
		File.open(ConfigFile.c_str());
		while (std::getline(File,Buffer)){
			if (Buffer=="[enableIRC]"){
				std::getline(File,Buffer);
				Config.enableIRC = string_to_int(Buffer);
			}else
			if (Buffer=="[enableManualInput]"){
				std::getline(File,Buffer);
				Config.enMinput = string_to_int(Buffer);
			}else
			if (Buffer=="[enableLog]"){
				std::getline(File,Buffer);
				Config.enableLog = string_to_int(Buffer);
			}else
			if (Buffer=="[ClientName]"){
				std::getline(File,Buffer);
				Config.clientName = Buffer;
			}else
			if (Buffer=="[ircServer]"){
				std::getline(File,Buffer);
				Config.ircServer = Buffer;
			}else
			if (Buffer=="[ircPort]"){
				std::getline(File,Buffer);
				Config.ircPort = Buffer;
			}else
			if (Buffer=="[IRCAutoJoin]"){
				std::getline(File,Buffer);
				Config.IRCAutoJoin = string_to_int(Buffer);
			}else
			if (Buffer=="[IRCAutoConnect]"){
				std::getline(File,Buffer);
				Config.IRCAutoConnect = string_to_int(Buffer);
			}
		}
		File.close();
		coreAddLog("Loaded settings.txt");
	}else{
		OuputMessage("settings.txt was not found. Loaded default settings","Error","red");
		coreSaveSettings();
	}
	//Load channels.txt
	ConfigFile = Config.rtdir+"channels.txt";
	if (file_exists(ConfigFile)==1){
		File.open(ConfigFile.c_str());
		while (std::getline(File,Buffer) and Buffer.length()>0 and Buffer.find("#")!=-1){
			unsigned n = Config.Channels.size();
			Config.Channels.push_back(typeChannel());			
			Config.Channels[n].Channel=Buffer;
			Config.Channels[n].Cmdprefix="!";
			Config.Channels[n].OP=0;
			Config.Channels[n].URLShrinker=0;
			Config.Channels[n].WebTitle=0;
		}
		File.close();
		coreAddLog("Loaded channels.txt");
	}
	//Load hostbans.txt
	ConfigFile = Config.rtdir+"hostbans.txt";
	if (file_exists(ConfigFile)==1){
		File.open(ConfigFile.c_str());
		while (std::getline(File,Buffer) and Buffer.length()>0){
			ignore_host(Buffer);
		}
		File.close();
		coreAddLog("Loaded hostbans.txt");
	}
}
/* Load a typeList from a file */
int coreloadList(std::string name, typeList *Holder){
	if (Holder->size()>0){
		Holder->clear();
	}
	std::ifstream File;
	if (!file_exists(name)){
		coreAddLog("Failed to load '"+name+"'");
		return -1;
	}
	File.open(name.c_str());
	std::string Buffer;
	while (std::getline(File,Buffer)){
		if (Buffer.length()>0){
			Holder->push_back(Buffer);
		}		
	}
	return Holder->size();
}

void coreRestart(){
	ircEnd("BRB.");
	suspend(1);
	coreAddLog(ProgramName+" is about to restart");
	coreAddLog("Session time: "+ seconds2hours(get_tocks()-Config.Uptime));
	coreAddLog(string_to_upper(ProgramName)+" REBOOTING...");
	if (Config.Parameters.size()>0){
		execl(Config.Binary.c_str(),Config.Binary.c_str(), Config.Parameters.c_str(),NULL);
	}else{
		execl(Config.Binary.c_str(),Config.Binary.c_str(),NULL); 
	}
	kill(getpid(), SIGTERM);
	return;
}

void coreEnd(){
	ircEnd("Bye.");
	coreAddLog(ProgramName+" is about to exit");
	coreAddLog("Session time: "+ seconds2hours(get_tocks()-Config.Uptime));
	coreAddLog(string_to_upper(ProgramName)+" TERMINATED");
	endprocess();
	return;
}

void coreLoadLists(){
	/* Load Yiffes */
	coreloadList(Config.rtdir+"/lists/yiff.txt",&Config.Yiff);
	/* Load Trivias */
	coreloadList(Config.rtdir+"/lists/trivia.txt",&Config.Trivia);
	/* Load Tolds */
	coreloadList(Config.rtdir+"/lists/told.txt",&Config.Told);
	/* Load Insults */
	coreloadList(Config.rtdir+"/lists/insults.txt",&Config.Insults);
	/* Load Quotes */
	coreloadList(Config.rtdir+"/lists/quotes.txt",&Config.Quotes);
}

//Start core
void coreStart(){
	Config.Uptime = get_tocks();
	//Default Settings
	Config.Ignore_timeout=300; //300 seconds, 5 minutes
	Config.Max_mpm=15;
	Config.enableIRC=0;
	Config.enMinput=0;
	Config.enableLog=1;
	Config.clientName="Helper";
	Config.ircServer="irc.rizon.net";
	Config.ircPort="6667";
	Config.Channels.clear();
	Config.ircNick=Config.clientName;
	Config.IRCAutoJoin=0;	
	Config.IRCAutoConnect=0;	
	coreSolvertdir();
	coreAddLog(string_to_upper(ProgramName)+" STARTED");
	coreLoadSettings();
	coreLoadUsers();
	ircStartup();
	
	Config.Prices.Yiff=15;
	Config.Prices.Told=20;
	Config.Prices.Insult=20;
	Config.Prices.Trivia=10;
	Config.Prices.Quote=15;
	Config.Prices.Kick=5000;
	
	coreLoadLists();
	return;
}
