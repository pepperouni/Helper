#ifndef CORE_H
	#define CORE_H
	#include "types.h"
	void coreStart();
	void coreSetrtdir(std::string newDirectory);
	void coreInputProcess(typeMessage *MessageQueue);
	void coreAddLog(std::string message);
	void coreSaveSettings();
	bool coreRefreshUserData(std::string Username);
	int coreAddUser(std::string Username,std::string Host,std::string Nick);
	int coreAddUserTMP(std::string Username,std::string Host,std::string Nick);
	bool coreUserDelete(std::string username);
	bool coreLoadUsers();
	void coreLoadLists();
	void coreSolvertdir();
	bool coreSolveNickCollision(std::string nick);
	
	// Users
	int coreUserID(std::string Username);
	int coreUserIDfromNick(std::string Nick);
	
	void coreEnd();
	void coreRestart();
	
	int is_ignore_host(std::string host);
	bool ignore_host(std::string host);
	bool unignore_host(std::string host);
#endif
