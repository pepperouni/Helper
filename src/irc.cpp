#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

#include "core.h"
#include "tools.h"
#include "types.h"
#include "output.h"


extern ConfigData Config;

pthread_t ircThread;

bool GotConnected=0;
bool InChannel=0;

bool gotMOTD=0;

bool IRCReconnect=1;

int irc_socket;

/* Some unnecesary buffers, I may remove them later */
char buf[513];
char rbuf[513];
char sbuf[513];

bool SocketConnected=0;

unsigned lastmessage = get_tocks();
unsigned currentmessagen = 0;

/* Custom SEND, to avaoid flood */
int senddata(std::string data){
	/* Anti-flood */
	if (currentmessagen>15 and get_tocks()-lastmessage<2){
		coreAddLog("Dropped message '"+data+"' to avoid flood.");
		return -1;
	}
	if (get_tocks()-lastmessage>=2){
		currentmessagen=0;
	}
	int status = send(irc_socket,data.c_str(),strlen(data.c_str()),0);
	coreAddLog("Send: "+remove_endline(data));
	currentmessagen++;
	lastmessage = get_tocks();
	return status;
}

/* CONNECT TO SERVER */
bool ircConnect(){
	
    struct addrinfo hints;
    struct addrinfo *results;
    
    /* Setting up */
    memset(&hints, 0, sizeof(struct addrinfo));
    
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    int gai_err = getaddrinfo( Config.ircServer.c_str(), Config.ircPort.c_str(), &hints, &results);
    /* Error getting getaddrinfo */
    if ( gai_err != 0 ) {
		coreAddLog("IRC: "+std::string(gai_strerror(gai_err)));
		OuputMessage(gai_strerror(gai_err),"Socket Error","red");
        return 0;
    }
    /* Socket */
    irc_socket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    OuputMessage(std::string("Connecting to ")+Config.ircServer.c_str()+" port: "+Config.ircPort.c_str(),"IRC","cyan");
	coreAddLog(std::string("IRC: Connecting to ")+Config.ircServer.c_str()+" port: "+Config.ircPort.c_str());
    /* Error with the socket */
    if ( irc_socket == -1 ){
		OuputMessage("Socket problem ","Error","red");
		coreAddLog(std::string("IRC: Socket problem ")+Config.ircServer.c_str()+" port: "+Config.ircPort.c_str());
		return 0;
	}
	SocketConnected=1;	
	/* Connecting... */
    if ( connect( irc_socket, results->ai_addr, results->ai_addrlen ) == -1 ){
		//Error with the connection
		OuputMessage("Couldn't connect","Error","red");
		coreAddLog(std::string("IRC: Couldn't connect to ")+Config.ircServer.c_str()+" port: "+Config.ircPort.c_str());
		return 0;
	}
	/* Send user data */
	char buffer[513];
    sprintf(buffer,"NICK %s\r\nUSER %s tolmoon tolsun :%s\r\n", Config.clientName.c_str(), Config.clientName.c_str(), Config.clientName.c_str() );
    if ( send(irc_socket, buffer, strlen(buffer), 0) == -1){
		OuputMessage("Failure trying to send username/nick","Error","red");
		coreAddLog("IRC: Failure trying to send username/nick");		
		return 0;
	}
    memset(buffer, 0, 513);
    freeaddrinfo(results);  
    OuputMessage(std::string("Connected to ")+Config.ircServer.c_str()+" port: "+Config.ircPort.c_str(),"IRC","cyan");
    coreAddLog(std::string("Connected to ")+Config.ircServer.c_str()+" port: "+Config.ircPort.c_str());		    
    return 1;
}
/* Change color flags with their IRC color code equivalent */
std::string applyColors(std::string in){
	std::string input = in;
	std::string colors[] = {
		"1","0","8","4","7","14",
		"3","2","5","6","12","\017"
	};
	std::string color_ns[] = {
		"%black","%white","%yellow","%red","%orange","%grey",
		"%green","%blue","%brown","%violet","%cyan","%reset"
	};
	unsigned n = sizeof(color_ns)/sizeof(std::string);
	for (unsigned i=0; i<n; i++){
		int j = input.find(color_ns[i]);
		if (j==-1)
			continue;
		input.erase(j,color_ns[i].length());
		input.insert(j,colors[i]);
		i = 0;
	}
	return input;
}
/* Check if this channel is in the channel list */
bool isChannel(std::string Channel){
	for (uint8_t i=0; i<Config.Channels.size(); i++){
		if (string_to_lower(Channel)==string_to_lower(Config.Channels[i].Channel))
			return 1;
	}
	return 0;
}

/* Get channel's ID from channel list */
int16_t channelID(std::string Channel){
	for (uint8_t i=0;i<Config.Channels.size();i++){
		if (string_to_lower(Channel)==string_to_lower(Config.Channels[i].Channel))
			return i;
	}
	return -1;
}

/* Remove a channel from the channel list */
bool channelRemove(std::string chan){
	int x = channelID(chan);
	if (x==-1)
		return 0;
	Config.Channels.erase(Config.Channels.begin()+x);
	coreSaveSettings();
	return 1;
}

/* Add a channel to the channel list */
bool channelAdd(std::string chan){
	int x = channelID(chan);
	if (x!=-1)
		return 0;
	unsigned n = Config.Channels.size();
	Config.Channels.push_back(typeChannel());
	Config.Channels[n].Channel=chan;
	Config.Channels[n].Cmdprefix="!";
	Config.Channels[n].OP=0;
	Config.Channels[n].URLShrinker=0;
	Config.Channels[n].WebTitle=0;
	coreSaveSettings();
	return 1;
}
/* Send NICK */
bool ircChangeNick(std::string name) {
	char buffer[513];
    sprintf(buffer, "NICK %s\r\n", name.c_str());
    if ( senddata(buffer) == -1)
		return 0;
	Config.clientName=name;	
	coreAddLog("Nick is now "+name);	
    memset(buffer, 0, 513);
    coreSaveSettings();
    return 1;
}
/* Send PART */
uint8_t ircPartChannel(std::string Channel){
	if (!isChannel(Channel))
		return 1;
	char buffer[513];
	sprintf(buffer, "PART %s\r\n", Channel.c_str());
	if (senddata(buffer) == -1)
		return 2;
	channelRemove(Channel);
	OuputMessage("Parted from the channel "+Channel,"IRC","cyan");
	coreAddLog("Parted from the channel "+Channel);	
	memset(buffer, 0, 513);
    return 0;
}
/* Send WHO */
void ircWho(std::string Target){
	char buffer[513];
    sprintf(buffer, "WHO %s\r\n", Target.c_str());
    senddata(buffer);
    return;
}

/* Join channel */
uint8_t ircJoinChannel(std::string Channel){
	if (isChannel(Channel))
		return 1;
	char buffer[513];
    sprintf(buffer, "JOIN %s\r\n", Channel.c_str());
    if ( senddata(buffer) == -1)
		return 2;
	channelAdd(Channel);
	OuputMessage("Joined to the channel "+Channel,"IRC","cyan");
	coreAddLog("Joined to the channel "+Channel);	
    memset(buffer, 0, 513);
    ircWho(Channel);
    return 0;
}

/* This doesn't add the channel to the channel list, just send JOIN to the server */
uint8_t ircJoinChannelManual(std::string Channel){
	char buffer[513];
    sprintf(buffer, "JOIN %s\r\n", Channel.c_str());
    if ( send(irc_socket, buffer, strlen(buffer),0) == -1)
		return 2;
	OuputMessage("Joined to the channel "+Channel,"IRC","cyan");
	coreAddLog("Joined to the channel "+Channel);	
    memset(buffer, 0, 513);
    ircWho(Channel);
    return 0;
}

/* Send WHOIS */
void ircWhois(std::string nick){
	char buffer[513];
    sprintf(buffer, "WHOIS %s\r\n", nick.c_str());
    senddata(buffer);
    return;
}

void processNICK(std::string input){
	typeList Split;
	split_words(input," ",&Split);
	if (Split.size()<9)
		return;
	
	std::string Username 	= string_to_lower(Split[4]);
	std::string Host 		= Split[5];
	std::string Nick 		= Split[7];
	if (Username.find("~")!=-1){
		Username.erase(Username.find("~"),1);
	}
	if (Nick==Config.clientName)
		return;
	std::string ModeW = Split[8];
	bool Registered=0;
	typeList Mode;
	for (unsigned i=0; i<ModeW.length(); i++){
		if (str_at(ModeW,i)=="r"){
			Registered=1;
		}
		Mode.push_back(str_at(ModeW,i));
	}
	int ID = coreUserID(Username);
	if (ID!=-1){
		typeUser &User = Config.Users[ID];
		if (Registered and User.Temporal){
			User.Temporal = 0;
			coreRefreshUserData(User.Username);
		}
		coreSolveNickCollision(User.Nick);
		User.Nick=Nick;
		User.Host=Host;
		User.Identified=Registered;
		return;
	}
	if (!Registered){
		coreAddUserTMP(Username,Host,Nick);
		return;
	}
	coreAddUser(Username,Host,Nick);
}

void processWHOIS(std::string input){
	typeList all;
	split_words(input," ",&all);
	if (all.size()<5)
		return;
	/* Channels */
	int x = input.find(":",1);
	if (x==-1)
		return;
	std::string whole = substring(input,x+1,input.length()-1);
	if (all[3]!=Config.clientName){
		return;
	}
	typeList ch;
	split_words(whole," ",&ch);
	for (unsigned i=0; i<ch.size(); i++){
		int x = channelID(ch[i]);
		if (x!=-1){
			Config.Channels[x].OP=0;
			continue;
		}
		if (str_at(ch[i],0)!="@")
			continue;
		x = channelID( substring(ch[i],1,ch[i].length()-1) );
		if (x==-1)
			return;
		Config.Channels[x].OP=1;
	}
}

void processMODE(std::string input){
	typeList all;
	split_words(input," ",&all);
	if (all.size()<5)
		return;
	std::string Channel = all[2];
	std::string Mode 	= all[3];
	std::string Nick 	= all[4];
	if (Nick==Config.clientName){
		int x = channelID(Channel);
		if (x==-1)
			return;
		if (Mode=="+o"){
			Config.Channels[x].OP=1;
		}else
		if (Mode=="-o"){
			Config.Channels[x].OP=0;
		}
	}
	return;
}

bool getIdent(std::string Ident,typeList *elements){
	/* Get nick */
	int x = Ident.find("!");
	if (str_at(Ident,x+1)!="~"){
		Ident.insert(x+1,"~");
	}
	if (x==-1){
		return 0;
	}
	elements->push_back(substring(Ident,0,x-1));
	int z = Ident.find("@");
	if (z==-1){
		return 0;
	}
	elements->push_back(substring(Ident,x+2,z-1));
	elements->push_back(substring(Ident,z+1,Ident.length()-1));
	return 1;
}

void processPrivMSG(typeList &Split){
	typeList Ident;
	if (!getIdent(substring(Split[0],1,Split[0].length()-1),&Ident) or Split.size()<4)
		return;
	
	std::string Origin = Split[2];
	if (Split[3].length()<=1)
		return;
	
	std::string Message = substring(Split[3],1,Split[3].length()-1);
	if (Split.size()>4)
		Message+=" ";
	for (unsigned i=4; i<Split.size();i++){
		Message+=Split[i];
		if (i<Split.size()-1)
			Message+=" ";
	}
	bool isPM = ( string_to_lower(Origin)==string_to_lower(Config.clientName) ? 1 : 0 );
	std::string mUser = Ident[0]+"("+Ident[1]+"@"+Ident[2]+")";
	std::string lmessage = mUser+": '"+Message;
	if (isPM)
		lmessage += "' from PM";
	else
		lmessage += "' from "+Origin;
	int ID = coreUserID(Ident[1]);
	if (ID!=-1){
		if (!Config.Users[ID].Identified){
			lmessage+= "(~i)";	//Unidentified
		}
		Config.Users[ID].Channel = (isPM ? "PM" : Origin);
		Config.Users[ID].LastActive = "Message";
		coreRefreshUserData(Config.Users[ID].Username);
	}else{
		lmessage+= "(~e)";	//Unexisting
	}
	
	coreAddLog(lmessage);
		
	std::cout << Ident[0] << ": '" << Message << "' from ";
	if(isPM)
		std::cout << "PM";
	else
		std::cout << Origin;
	std::cout << std::endl;
	typeMessage ManualMessage;
	ManualMessage.Host=Ident[2];
	ManualMessage.Nick=Ident[0];
	ManualMessage.Type=isPM;
	ManualMessage.Channel=Origin;
	ManualMessage.Message=Message;
	ManualMessage.Username=string_to_lower(Ident[1]);
	coreInputProcess(&ManualMessage);
}

/* Get data from the server */
bool ircGet() {
	fd_set fds;
	struct timeval tv;
	int ret;

	tv.tv_sec = 240;
	tv.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(irc_socket, &fds);

	ret = select(irc_socket + 1, &fds, NULL, NULL, &tv);
	//Socket timeout
	if (!ret)	
		return 0;
	int recv_bytes;
    memset(rbuf, 0, 513);
    //receive data
    recv_bytes = recv(irc_socket, rbuf, sizeof(rbuf), 0);
   // std::cout << "'" << rbuf << "'";
    //Socket error
    if (recv_bytes <= 0) 
        return 0;
    //std::cout << "'" << rbuf << "'" << std::endl;
    //std::cout <<rbuf << std::endl;
    char *tok;
    //Respond to Ping
    if (rbuf[0] == 'P' && rbuf[1] == 'I') {
        tok = strtok(rbuf, "PING :");
        sprintf(buf, "PONG %s", tok-1 );
        send(irc_socket, buf, strlen(buf), 0);
        coreAddLog("IRC: PONG!");	
        memset(buf, 0, 513);
    }   
    if (gotMOTD==0){
		if (remove_endline(std::string(rbuf)).find("/MOTD")!=-1){
			OuputMessage("Ready to join in a channel","IRC","cyan");
			coreAddLog("IRC: Ready to join in a channel");
			gotMOTD=1;	
		}
	}
	std::string m = rbuf;
	typeList Lines;
	while (1){
		int i = m.find("\r");
		if (i==-1)
			break;
		m.erase(i,1);
	}
	split_words(m,"\n",&Lines);
	for (unsigned i=0; i<Lines.size(); i++){
		if (Lines[i].size()==0)
			continue;
		//std::cout << "'" << Lines[i] << "'" << std::endl;
		if (str_at(Lines[i],0)!=":")
			continue;
		typeList Split;
		split_words(Lines[i]," ",&Split);
		if (Split.size()==0)
			continue;
		/* KICK */
		if (Split[1]=="KICK"){	
			if (string_to_lower(Split[3])!=string_to_lower(Config.clientName))
				return 1;
			OuputMessage("Helper was kicked from "+Split[2]+".","Attention","red");
			coreAddLog("IRC: Helper was kicked from "+Split[2]+".");
			channelRemove(Split[2]);
			continue;
		}
		/* QUIT */
		if (Split[1]=="QUIT" or Split[1]=="SQUIT"){
			typeList Ident;
			if (!getIdent(substring(Split[0],1,Split[0].length()-1),&Ident) or Split.size()<3)
				return 1;
			int ID = coreUserID(Ident[1]);
			if (ID==-1)
				return 1;
			Config.Users[ID].Identified = 0;
			Config.Users[ID].Channel = tNull;
			Config.Users[ID].LastActive = "Quit";
			coreRefreshUserData(Config.Users[ID].Username);
			continue;
		}
		/* NICK */
		if (Split[1]=="NICK"){
			typeList Ident;
			if (!getIdent(substring(Split[0],1,Split[0].length()-1),&Ident) or Split.size()<3)
				continue;
			if (Ident[0]==Config.clientName){
				ircChangeNick("helper"+int_to_string(random_int(10000,1000)));
				continue;
			}
			int ID = coreUserID(Ident[1]);
			if (ID==-1)
				continue;
			Config.Users[ID].Channel = Ident[0];
			Config.Users[ID].LastActive = "Nick";
			coreSolveNickCollision(Config.Users[ID].Nick);
			Config.Users[ID].Nick = substring(Split[2],1,Split[2].length()-1);
			coreRefreshUserData(Config.Users[ID].Username);
			continue;
		}
		/* PART */
		if (Split[1]=="PART"){
			typeList Ident;
			if (!getIdent(substring(Split[0],1,Split[0].length()-1),&Ident) or Split.size()<3)
				continue;
			int ID = coreUserID(Ident[1]);
			if (ID==-1)
				continue;
			Config.Users[ID].Channel = Split[2];
			Config.Users[ID].LastActive = "Part";
			coreRefreshUserData(Config.Users[ID].Username);
			continue;
		}
		/* JOIN */
		if (Split[1]=="JOIN"){
			typeList Ident;
			if (!getIdent(substring(Split[0],1,Split[0].length()-1),&Ident) or Split.size()<3)
				continue;
			processNICK(Ident[0]);
			ircWho(Ident[0]);
			int ID = coreUserID(Ident[1]);
			if (ID==-1){
				continue;
			}
			Config.Users[ID].Channel = substring(Split[2],1,Split[2].length()-1);
			Config.Users[ID].LastActive = "Join";
			coreRefreshUserData(Config.Users[ID].Username);
			continue;
		}
		/* PRIVMSG */
		if (Split[1]=="PRIVMSG"){	
			processPrivMSG(Split);
			continue;
		}
		/* MODE */
		if (Split[1]=="MODE"){
			processMODE(Lines[i]);
			typeList Ident;
			if (!getIdent(substring(Split[0],1,Split[0].length()-1),&Ident) or Split.size()<3)
				continue;
			int ID = coreUserID(Ident[1]);
			if (ID==-1)
				continue;
			Config.Users[ID].Channel = Split[2];
			Config.Users[ID].LastActive = "Mode";
			coreRefreshUserData(Config.Users[ID].Username);
			continue;
		}
		/* WHOIS */
		if (Split[1]=="319"){	
			processWHOIS(Lines[i]);
			continue;
		}
		/* Get users */
		if (Split[1]=="352"){	
			processNICK(Lines[i]);
			continue;
		}
		/* Change nick */
		if (Split[1]=="432"){
			ircChangeNick("helper"+int_to_string(random_int(10000,1000)));
			continue;
		}
		/* Change nick */
		if (Split[1]=="433"){
			ircChangeNick(Config.clientName+int_to_string(random_int(9,0)));
			continue;
		}
		/* Needs to be identified */
		if (Split[1]=="477"){
			if (channelRemove(Split[3])){
				std::string m = "Couldn't join to "+Split[3]+": ";
				m += "You need to Identify to a registered nick to join that channel.";
				coreAddLog("IRC: "+m);	
				OuputMessage(m,"IRC","red");
			}	
			continue;
		}
	}
    return 1;
}

/* Send KICK */
void ircKick(std::string Nick,std::string Channel){	
	std::string Kick="KICK "+Channel+" "+Nick+"\r\n";
	senddata(Kick);
}

/* Send a raw: ircSendRaw("PRIVMSG Canape :Good bot"); */
/* \r\n is automatically added */
uint8_t ircSendRaw(std::string Raw){
	if (Raw.length()<1){
		coreAddLog("'"+Raw+"' is too short to be sent as raw.");
		return 1;		
	}
	if (Raw.length()>412){
		coreAddLog("'"+Raw+"' is too long to be sent as raw.");
		return 1;
	}
	if ( senddata(Raw+"\r\n") == -1)
		return 2;
	return 0;
}
/* Send PRIVMSG */
uint8_t ircSendMessage(std::string Message,std::string Destionation){
	std::cout << Config.clientName << ": " << Message << std::endl;
	if (Destionation=="helper.manual.input"){
		return 0;
	}
	std::string Destiny = Destionation;
	std::string sendMessage = applyColors(Message);
	if ((Config.enableIRC==0 && GotConnected==0 && gotMOTD==0) or 
	Destionation==tNull or sendMessage.length()<1)
		return 1;	
	//Split message
	while (sendMessage.length()>421){
		ircSendMessage(substring(sendMessage,0,420),Destionation);
		sendMessage.erase(0,420);
	}
	std::cout << Config.clientName << ": " << sendMessage << std::endl;
	std::string MessageSend=std::string( "PRIVMSG "+Destiny+" :"+sendMessage+"\r\n" );
	
	if ( senddata(MessageSend) == -1)
		return 2;
	return 0;

}
/* send NOTICE */
uint8_t ircSendNOTICE(std::string Message,std::string Destionation){
	std::cout << Config.clientName << ": " << Message << std::endl;
	if (Destionation=="helper.manual.input"){
		return 0;
	}
	std::string Destiny = Destionation;
	std::string sendMessage = applyColors(Message);
	
	if ((Config.enableIRC==0 && GotConnected==0 && gotMOTD==0) or 
	Destionation==tNull or sendMessage.length()<1)
		return 1;	
	//Split message
	while (sendMessage.length()>421){
		ircSendNOTICE(substring(sendMessage,0,420),Destionation);
		sendMessage.erase(0,420);
	}
	std::string MessageSend=std::string( "NOTICE "+Destiny+" :"+sendMessage+"\r\n" );
	
	if ( senddata(MessageSend) == -1)
		return 2;
	return 0;
}
/* Send /me  */
uint8_t ircSendME(std::string Message,std::string Destionation){
	std::cout << Config.clientName << ": " << Message << std::endl;
	if (Destionation=="helper.manual.input"){
		return 0;
	}
	std::string Destiny = Destionation;
	std::string sendMessage = applyColors(Message);
		
	if ((Config.enableIRC==0 && GotConnected==0 && gotMOTD==0) or 
	Destionation==tNull or sendMessage.length()<1)
		return 1;
	//Split message
	while (sendMessage.length()>421){
		ircSendME(substring(sendMessage,0,420),Destionation);
		sendMessage.erase(0,420);
	}
	std::string MessageSend=std::string( "PRIVMSG "+Destiny+" :\001ACTION "+sendMessage+"\001\r\n" );
	
	if ( senddata(MessageSend) == -1)
		return 2;
	return 0;

}

bool AutoJoined=0;

void ircEnd(std::string Message){
	if (GotConnected==1){
		std::string Exit="QUIT :"+Message+"\r\n";
		send(irc_socket, Exit.c_str(), strlen(Exit.c_str()),0);
	}
	if (SocketConnected==1){
		close(irc_socket);
		coreAddLog("IRC: Disconnected.");
		OuputMessage("Disconnected.","IRC","cyan");
	}
	GotConnected=0;
	InChannel=0;
	AutoJoined=0;
	gotMOTD=0;
	SocketConnected=0;
	if (Config.enableIRC==1){
		Config.enableIRC=0;
		coreAddLog("IRC: Disabled.");
		OuputMessage("Disabled.","IRC","cyan");
	}
	return;
}

/* IRC's Thread */
void *ircMain(void *p){
    while(1){
		/* If IRC is not enabled, suspend for 50 milliseconds */
		if (Config.enableIRC==0)
			suspend(50);
		/* If it is, try to connect */
        if (Config.enableIRC==1 && GotConnected==0 && Config.IRCAutoConnect==1){
			if (ircConnect()==1){
				GotConnected=1;
			}else{
				coreAddLog("IRC: Couldn't connect. Retrying in 10 seconds...");	
				OuputMessage("Couldn't connect. Retrying in 10 seconds...","IRC","red");
				suspend(10000);
			}
			suspend(50);
        }
        /* MAIN (When connected) */
        if (Config.enableIRC==1 && GotConnected==1){ 
			if (ircGet()==0){
				/* timeout */
				coreAddLog("IRC: Socket timeout.");	
				OuputMessage("IRC's socket timeout.","IRC","red");
				ircEnd("");
				/* re-enable if IRCReconnect is true */
				if (IRCReconnect==1){
					coreAddLog("IRC: Aut-reconnect in 10 seconds...");	
					OuputMessage("Auto-reconnect in 10 seconds...","IRC","red");
					suspend(10000);
					coreAddLog("IRC: Re-enabled IRC.");	
					OuputMessage("Re-enabled IRC.","IRC","red");
					Config.enableIRC=1;
				}

			}
			/* Ready to join a channel */
			if (gotMOTD==1 and AutoJoined==0){
				for (uint8_t i=0;i<Config.Channels.size();i++){
					ircJoinChannelManual(Config.Channels[i].Channel);
				}
				if (!Config.Channels.size()){
					coreAddLog("IRC: No channels found.");	
					OuputMessage("No channels found.","IRC","yellow");
				}
				AutoJoined=1;
			}
		}
    }
}

/* Start up */
void ircStartup(){
    coreAddLog("IRC started");
    pthread_create( &ircThread, NULL, ircMain,NULL);
    return;
}
