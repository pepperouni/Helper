#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>

#include "output.h"
#include "types.h"
#include "irc.h"
#include "core.h"
#include "commandline.h"
#include "tools.h"

extern ConfigData Config;

std::string getReceiver(typeMessage *Message){
	if (Message->Host=="helper.manual.input"){
		return "helper.manual.input";
	}
	if (Message->Type){
		return Message->Nick;
	}
	return Message->Channel;
}

/* Alme implementation */
std::string math_engine(std::string input);


bool isIdentified(typeUser *User,typeMessage *Message){
	if (User->Identified)
		return 1;
	std::string pr;
	pr = "You %redmust%reset be identified to use this command. If you already are, try again.";
	ircSendMessage(pr,getReceiver(Message));
	return 0;
}

bool requieredArgn(unsigned n, unsigned re_n,typeMessage *Message){
	if (n>=re_n)
		return 1;
	ircSendMessage("Not enough arguments("+int_to_string(re_n)+").",getReceiver(Message));
	return 0;	
}

bool requieredUserlevel(uint8_t lv,typeUser *User,typeMessage *Message){
	if (User->userLevel>=lv)
		return 1;
	ircSendMessage(
		"You need userlevel ("+int_to_string(lv)+") or more to run this command. Yours: "+
		int_to_string(User->userLevel)+".",
		getReceiver(Message)
	);
	return 0;
}

std::string PythonScript(std::string script,std::string input){
	std::string out = "#ERROR";
	char buff[256];	
	FILE *in;
	in = popen(std::string("python "+Config.rtdir+"/scripts/"+script+" '"+input+"'").c_str(), "r");
	while(fgets(buff, sizeof(buff), in)!=NULL){
		out = buff;
		out = remove_endline(out);
		break;
	}
	return out;
}


std::string float_to_cash(int64_t money){
	std::string output="";
	std::string entry = int_to_string(positivei(money));
	
	//if (entry.find(".")==-1)
		//entry+=".00";
	//std::string decimal = substring(entry,entry.find("."),entry.length()-1);
	std::string whole 	= entry;//substring(entry,0,entry.find(".")-1);
	if (string_to_int(whole)<1000){
		std::string c ="%green";
		if (money<=0)
			c = "%red";
		return c+"$"+whole+"%reset";
	}
	for (unsigned i=0; i<whole.length(); i++){
		unsigned c = whole.length()-1-i;
		output=str_at(whole,c)+output;
		
		if ( (i+1)%3==0 and c>0){
			output=","+output;
		}
	}
	output.insert(0,"$");
	if (money<=0){
		output.insert(0,"%red");
		output+="%reset";
	}else{
		output.insert(0,"%green");
		output+="%reset";
	}	
			
	return output;
}

int64_t cash_to_float(std::string money){
	//std::cout << "cash_to_float " << %redmoney%reset << std::endl;
	std::string output = money;
	while (output.find(",")!=-1)
		output.erase(output.find(","),1);
	while (output.find("$")!=-1)
		output.erase(output.find("$"),1);
	return string_to_int(output);
}

#define pwdLevel5 std::string("level5")
#define pwdLevel4 std::string("level4")
#define pwdLevel3 std::string("level3")

#define paycheck_time 21600	//Every six hours
#define paycheck_pay 300.00	//$300.00

int8_t Identify(std::string password){
	if (pwdLevel5==password)
		return 5;
	if (pwdLevel4==password)
		return 4;
	if (pwdLevel3==password)
		return 3;
	return -1;
}

void moneyAdd(typeUser *User, unsigned m){
	User->Money+=m;
	coreRefreshUserData(User->Username);
}

void moneyRemove(typeUser *User, unsigned m){
	if (User->Money-m<0){
		User->Money=0;
	}else{
		User->Money-=m;
	}
	coreRefreshUserData(User->Username);
}

bool Buy(uint64_t price,std::string name,typeUser *User,typeMessage *Message){
	if (User->Money<price){
		std::string m; 
		m = "You don't have enough %redmoney%reset to Buy a "+name+". ";
		m += name+"s are "+float_to_cash(price)+" each one.";	
		ircSendMessage(m,getReceiver(Message));
		return 0;
	}
	moneyRemove(User,price);
	std::string m = "You have bought one "+name+" for "+float_to_cash(price)+".";
	ircSendNOTICE(m,User->Nick);
	return 1;
}

void FlagInterpreter(typeList &Arguments,typeMessage *Message){
	typeUser &User = Config.Users[ coreUserID(Message->Username) ];
	
	for (int i=0; i<Arguments.size(); i++){
		if (Arguments[i].length()==0){
			Arguments.erase(Arguments.begin()+i);
			i = -1;
			continue;
		}
		if (str_at(Arguments[i],0)!="%" or Arguments[i].length()<2){
			continue;
		}
		/* Switch username for nick */
		if (str_at(Arguments[i],1)=="@" and Arguments[i].length()>2){
			std::string n = substring(Arguments[i],2,Arguments[i].length()-1);
			int u = coreUserID(n);
			if (u==-1){
				Arguments[i] = "#ERROR:UKNOWN_USERNAME";
				continue;
			}
			Arguments[i] = Config.Users[u].Nick;
			i = -1;
			continue;
		}
		/* Remove specific strings */
		if (str_at(Arguments[i],1)=="-" and Arguments[i].length()>2){
			std::string n = substring(Arguments[i],2,Arguments[i].length()-1);
			Arguments.erase(Arguments.begin()+i);
			for (unsigned x=0; x<Arguments.size(); x++){
				int h = string_to_lower(Arguments[x]).find(string_to_lower(n));
				if (h==-1)
					continue;
				Arguments[x].erase(h,n.length());
				if (Arguments[x].length()==0)
					Arguments.erase(Arguments.begin()+x);
				x = -1;
			}
			i = -1;
			continue;
		}
		/* Add all the users */
		if (str_at(Arguments[i],1)=="*"){
			if (User.userLevel<3){
				ircSendMessage("You must be rootUser to use this flag.",getReceiver(Message));
				Arguments[i] = tNull;
				continue;
			}
			uint8_t t = 0;	/* all */
			
			if (Arguments[i].length()>2){
				/* All */
				if (string_to_lower(str_at(Arguments[i],2))=="a"){
					t = 0;
				}
				/* Registered */
				if (string_to_lower(str_at(Arguments[i],2))=="r"){
					t = 1;
				}
				/* Temporal */
				if (string_to_lower(str_at(Arguments[i],2))=="t"){
					t = 2;
				}
				/* Identified */
				if (string_to_lower(str_at(Arguments[i],2))=="i"){
					t = 3;
				}
				/* In the current Channel */
				if (string_to_lower(str_at(Arguments[i],2))=="c"){
					t = 4;
				}
			}
			Arguments.erase(Arguments.begin()+i);
			for(unsigned x=0; x<Config.Users.size(); x++){
				if (t==1 and Config.Users[x].Temporal){
					continue;
				}
				if (t==2 and !Config.Users[x].Temporal){
					continue;
				}
				if (t==3 and !Config.Users[x].Identified){
					continue;
				}
				Arguments.insert(Arguments.begin()+i,Config.Users[x].Nick);
			}
			i = -1;
			continue;
		}
	}
	
	return;
}

#define maxnicks 5

bool nicksCheck(unsigned n,typeMessage *Message){
	if (n>maxnicks){
		ircSendMessage("Too many nicks!",getReceiver(Message));
		return 1;
	}
	return 0;
}

void CommandLine(typeMessage *Message){
	typeUser &User = Config.Users[ coreUserID(Message->Username) ];
	if (!Message->Message.length())
		return;
	
	std::string Command;
	typeList Arguments;
	
	std::string Ctype="!";
	if (Message->Type==0){
		Ctype = Config.Channels[channelID(Message->Channel)].Cmdprefix;
	}
	
	split_words(Message->Message," ",&Arguments);
	
	Command = string_to_lower(Arguments[0]);
	Arguments.erase(Arguments.begin());
	
	static const std::string Commands[] = {
		Ctype+"paycheck",Ctype+"dice",Ctype+"scissors",Ctype+"rock",Ctype+"paper",
		Ctype+"yiff",Ctype+"math",Ctype+"transfer",Ctype+"bet",Ctype+"dice",Ctype+"who",
		Ctype+"coin",Ctype+"money",Ctype+"help",Ctype+"kick",Ctype+"listen",Ctype+"insult",
		Ctype+"quote",Ctype+"trivia",Ctype+"active",Ctype+"told",Ctype+"cmdprefix",
		Ctype+"say",Ctype+"channels",Ctype+"exit",Ctype+"time",Ctype+"host",
		Ctype+"identify",Ctype+"logout",Ctype+"join",Ctype+"part",Ctype+"unignore",
		Ctype+"ignore",	Ctype+"name",Ctype+"imitate",Ctype+"users"
	};
	static const unsigned CommandN = sizeof(Commands)/sizeof(std::string);
	
	if (User.AutoKick_Ignored and User.Ignore){
		if (Message->Type){
			return;
		}
		int chn = channelID(Message->Channel);
		if (chn==-1)
			return;
		if (!Config.Channels[chn].OP){
			return;
		}
		for (unsigned i=0; i<CommandN; i++){
			if (Command==Commands[i]){
				ircKick(User.Nick,Message->Channel);
				return;
			}
		}
	}
	
	FlagInterpreter(Arguments,Message);

	/*** Buy commands ***/
	if (Command==Ctype+"autokick"){
		if (!requieredUserlevel(4,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		int ID = coreUserIDfromNick(Arguments[0]);
		if (ID==-1){
			ircSendMessage("Unknown nick.",getReceiver(Message));
			return;
		}
		if (Config.Users[ID].AutoKick_Ignored){
			ircSendMessage(Arguments[0]+" is no longer kicked.",getReceiver(Message));
			Config.Users[ID].AutoKick_Ignored = 0;
			return;
		}
		if (!Config.Users[ID].Ignore){
			ircSendMessage("Can't autokick an unignored nick.",getReceiver(Message));
			return;
		}
		Config.Users[ID].AutoKick_Ignored = 1;
		ircSendMessage(Arguments[0]+" is now autokicked.",getReceiver(Message));
		return;
	}
	/* !trivia */
	if (Command==Ctype+"trivia"){
		if (!isIdentified(&User,Message))
			return;
		unsigned n = random_int(Config.Trivia.size()-1,0);
		if (!Buy(Config.Prices.Trivia,"trivia",&User,Message))
			return;
		if (Arguments.size()>0){
			if (is_number(Arguments[0]) and string_to_int(Arguments[0])>0 and
			string_to_int(Arguments[0])<=Config.Trivia.size()){
				n = string_to_int(Arguments[0])-1;
			}
		}
		ircSendMessage(Config.Trivia[n],getReceiver(Message));
	}
	
	/* !insult */
	if (Command==Ctype+"insult"){
		if (!isIdentified(&User,Message))
			return;
		if (Arguments.size()==0)
			Arguments.push_back(User.Nick);
		if (User.userLevel<3 and Arguments.size()>3)
			Arguments.erase(Arguments.begin()+3,Arguments.end());
		User.cMSG--;
		for (unsigned i=0; i<Arguments.size(); i++){
			if (string_to_lower(Arguments[i])==string_to_lower(Config.clientName)){
				Arguments[i]=User.Nick;
			}
			if (!Buy(Config.Prices.Insult,"insult",&User,Message))
				break;
			std::string Insult = Config.Insults[ random_int(Config.Insults.size()-1,0) ];
			while (1){
				bool f = 0;
				if (Insult.find("%TARGET")!=-1){
					unsigned d = Insult.find("%TARGET");
					Insult.erase(d,std::string("%TARGET").length());
					Insult.insert(d,"%red"+Arguments[i]+"%reset");
					f=1;
				}
				if (Insult.find("%LAUGH")!=-1){
					std::string Laughs[] = {
						"LOL","HAHAHAHA",
						"Pffhaahhaah","Lel",
						"Huehueehue","HUEHUEHUE",
						"Umad:)","xD",":)","x)",
						"xDD","Le "+Config.clientName+" face xD"
					};
					std::string laugh = Laughs[ random_int(sizeof(Laughs)/sizeof(std::string)-1,0)  ];
					unsigned d = Insult.find("%LAUGH");
					Insult.erase(d,std::string("%LAUGH").length());
					Insult.insert(d,"%blue"+laugh+"%reset");
					f=1;
				}				
				if (!f)
					break;
			}
			User.cMSG++;
			ircSendMessage(Insult,getReceiver(Message));
		}
	}
	/* !yiff */
	if (Command==Ctype+"yiff"){
		if (!isIdentified(&User,Message))
			return;
		if (Arguments.size()==0)
			Arguments.push_back(User.Nick);
		if (User.userLevel<3 and Arguments.size()>3)
			Arguments.erase(Arguments.begin()+3,Arguments.end());
		if (nicksCheck(Arguments.size(),Message))
			return;
		User.cMSG--;
		for (unsigned i=0; i<Arguments.size(); i++){
			if (!Buy(Config.Prices.Yiff,"yiff",&User,Message))
				break;
			std::string Yiff = Config.Yiff[  random_int(Config.Yiff.size()-1,0) ];;
			while (1){
				bool f = 0;
				if (Yiff.find("%SELF")!=-1){
					unsigned d = Yiff.find("%SELF");
					Yiff.erase(d,std::string("%SELF").length());
					Yiff.insert(d,"%blue"+Config.clientName+"%reset");
					f=1;
				}
				if (Yiff.find("%OWNER")!=-1){
					unsigned d = Yiff.find("%OWNER");
					Yiff.erase(d,std::string("%OWNER").length());
					Yiff.insert(d,"%red"+User.Nick+"%reset");
					f=1;
				}
				if (Yiff.find("%TARGET")!=-1){
					unsigned d = Yiff.find("%TARGET");
					Yiff.erase(d,std::string("%TARGET").length());
					Yiff.insert(d,"%violet"+Arguments[i]+"%reset");
					f=1;
				}
				if (Yiff.find("%CHANNEL")!=-1){
					unsigned d = Yiff.find("%CHANNEL");
					Yiff.erase(d,std::string("%CHANNEL").length());
					Yiff.insert(d,"%orange"+Message->Channel+"%reset");
					f=1;
				}				
				if (!f)
					break;
			}
			User.cMSG++;
			ircSendME(Yiff,getReceiver(Message));
		}
	}
	
	/* !trivia */
	if (Command==Ctype+"quote"){
		if (!isIdentified(&User,Message))
			return;
		unsigned n = random_int(Config.Quotes.size()-1,0);
		
		if (!Buy(Config.Prices.Quote,"quote",&User,Message))
			return;		
		
		for (unsigned i=0; i<Config.Quotes.size(); i++){
			if (Arguments.size()==0)
				break;
			bool f = 1;
			for (unsigned h=0; h<Arguments.size(); h++){
				if (string_to_lower(Config.Quotes[i]).find(string_to_lower(Arguments[h]))==-1){
					f = 0;
					continue;
				}
			}
			if (f){
				n = i;
				break;
			}
		}
		ircSendMessage(Config.Quotes[n],getReceiver(Message));
	}
	if (Command==Ctype+"shrink"){
		if (!requieredUserlevel(4,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		std::string out = PythonScript("shrinker.py",Arguments[0]);
		if (out!="#ERROR"){
			ircSendMessage(out,getReceiver(Message));
		}
		return;
	}
	if (Command==Ctype+"title"){
		if (!requieredUserlevel(4,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		std::string out = PythonScript("title.py",Arguments[0]);
		if (out!="#ERROR"){
			ircSendMessage(out,getReceiver(Message));
		}
		return;
	}
	/* !kick */
	if (Command==Ctype+"kick"){
		if (!isIdentified(&User,Message) or Arguments.size()==0)
			return;
		if (Message->Type){
			ircSendMessage("You can't Buy kicks via PM.",getReceiver(Message));
		}
		ircWhois(Config.clientName);
		int chn = channelID(Message->Channel);
		if (chn==-1)
			return;
		if (!Config.Channels[chn].OP){
			ircSendMessage("I'm not +o, I can't kick anyone.",getReceiver(Message));
			return;
		}
		if (string_to_lower(Arguments[0])==string_to_lower(Config.clientName)){
			ircSendMessage("You can't kick me.",getReceiver(Message));
			ircKick(Arguments[0],Message->Channel);	
			return;
		}
		if (coreUserIDfromNick(Arguments[0])==-1){
			ircSendMessage("I don't know who's that.",getReceiver(Message));
			return;
		}
		if (!Buy(Config.Prices.Kick,"kick",&User,Message))
			return;
		ircSendME("kicks "+Arguments[0]+" out of "+Message->Channel+".",getReceiver(Message));
		ircKick(Arguments[0],Message->Channel);
	}
	
		/* !told */
	if (Command==Ctype+"told"){
		if (Arguments.size()==0)
			Arguments.push_back(User.Nick);
		if (User.userLevel<3 and Arguments.size()>3)
			Arguments.erase(Arguments.begin()+3,Arguments.end());
		if (nicksCheck(Arguments.size(),Message))
			return;
		User.cMSG--;
		for (unsigned i=0; i<Arguments.size(); i++){
			if (string_to_lower(Arguments[i])==string_to_lower(Config.clientName)){
				Arguments[i]=User.Nick;
			}
			if (!Buy(Config.Prices.Told,"told",&User,Message))
				break;
			std::string Status = Config.Told[  random_int(Config.Told.size()-1,0) ];
			std::string m;
			User.cMSG++;
			m = Arguments[i]+"'s status: [ ] Not told | \002[x] TOLD\002 | %red"+Status+"%reset.";
			ircSendMessage(m,getReceiver(Message));
		}
	}
	
	/*** %redmoney%reset commands ***/
	
	if (Command==Ctype+"money" or Command==Ctype+"bank" or Command==Ctype+"balance"){
		if (!isIdentified(&User,Message))
			return;
		ircSendMessage("You currently have "+float_to_cash(User.Money)+
		" in the bank.",getReceiver(Message));
		if (User.Money<=0){
			ircSendMessage("%blueLOL%reset, you are broke!",getReceiver(Message));
		}
	}
	
	if (Command==Ctype+"pay" or Command==Ctype+"paycheck" or Command==Ctype+"salary"){
		if (!isIdentified(&User,Message))
			return;
		if ((get_tocks()-User.PaycheckT)>paycheck_time){
			unsigned newpay = paycheck_pay+round_int(paycheck_pay*(1/((float)random_int(10,1))));
			User.PaycheckT = get_tocks();
			moneyAdd(&User,newpay);
			ircSendMessage("You've received "+float_to_cash(newpay)+
			" for your paycheck. Now you have "+float_to_cash(User.Money)+". "+
			"Come back again in "+seconds2hours(paycheck_time)+" for your next paycheck.",getReceiver(Message));
		}else{
			ircSendMessage(std::string("Your next paycheck will be in ")+
			seconds2hours(paycheck_time-(get_tocks()-User.PaycheckT))+".",getReceiver(Message));
		}
	}
	
	/* !transfer */
	if (Command==Ctype+"transfer" or (Command==Ctype+"give" and User.userLevel<5)){
		if (!isIdentified(&User,Message))
			return;
		if (!requieredArgn(Arguments.size(),2,Message))
			return;
		int ID = coreUserIDfromNick(Arguments[0]);
		if (ID==-1){
			ircSendMessage("Unknown nick.",getReceiver(Message));
			return;
		}
		if (Config.Users[ID].Ignore){
			ircSendMessage("You can't transfer %redmoney%reset to someone being ignored.",getReceiver(Message));
			return;
		}
		if (Config.Users[ID].Host==User.Host){
			ircSendMessage("You can't transfer %redmoney%reset to a user "+
			std::string("who has the same Hostname than you."),getReceiver(Message));
			return;
		}
		std::string m = int_to_string( cash_to_float(Arguments[1]) );
		if (!(is_number(Arguments[1]) and string_to_int(Arguments[1])>0)){
			ircSendMessage("Invalid quantity.",getReceiver(Message));
			return;
		}
		if (string_to_int(m)>User.Money){
			ircSendMessage("You can't give more %redmoney%reset than you have!",getReceiver(Message));
			return;
		}
		moneyRemove(&User,string_to_int(m));
		moneyAdd(&Config.Users[ID],string_to_int(m));
		ircSendMessage("Transfered "+float_to_cash(string_to_int(m))+" to "+
		Arguments[0]+"'s account. Now you have "+float_to_cash(User.Money)+"."
		,getReceiver(Message));
	}
	
	/*** Bet commands ***/
	if (Command==Ctype+"coin"){
		if (!isIdentified(&User,Message))
			return;
		unsigned his = random_int(1,0);
		if (Arguments.size()>0){
			if ( is_number(Arguments[0]) and
				string_to_int(Arguments[0])>=0 and string_to_int(Arguments[0])<=1 ){
				his = string_to_int(Arguments[0]);
			}
			if (string_to_lower(Arguments[0])=="heads" or
				string_to_lower(Arguments[0])=="head"
				){
				his=0;
			}
			if (string_to_lower(Arguments[0])=="tails" or
				string_to_lower(Arguments[0])=="tail"
				){
				his=1;
			}
		}
		unsigned win = random_int(10,1);
		if (User.Money<10){
			std::string m = "You need at least "+float_to_cash(10)+" to play Coin.";
			ircSendMessage(m,getReceiver(Message));
			return;
		}
		
		if (Arguments.size()>1){
			std::string mh = int_to_string(cash_to_float(Arguments[1]));
			if (is_number(mh) and string_to_int(mh)>0 ){
				if (string_to_int(mh)>User.Money){
					ircSendMessage("Are you trying to trick me, mate?"+
					std::string(" You can't bet more %redmoney%reset than you have!"),getReceiver(Message));
					return;
				}
				if (string_to_int(mh)>100){
					ircSendMessage("You can't bet more than "+float_to_cash(100)
					+" in Coin.",getReceiver(Message));
					return;
				}
				win = string_to_int(mh);
			}
		}
		
		unsigned n = random_int(1,0);
		
		std::string Coin[] = { "Heads","Tails" };
		
		std::string out = "Flipping the coin... "+Coin[n]+"!";
		if (n==his){
			moneyAdd(&User,win);
			out+=" You chose "+Coin[his]+". You win "+float_to_cash(win)+"!";
		}else{
			moneyRemove(&User,win);
			out+=" You chose "+Coin[his]+". You lose "+float_to_cash(negative(win))+".";
		}
		out+=" Now you have "+float_to_cash(User.Money)+".";
		ircSendMessage(out,getReceiver(Message));
		
	}
	/* dice */
	if (Command==Ctype+"dice"){
		if (!isIdentified(&User,Message))
			return;
		unsigned his = random_int(6,1);
		if (Arguments.size()>0){
			if ( is_number(Arguments[0]) and
				string_to_int(Arguments[0])>0 and string_to_int(Arguments[0])<7 ){
				his = string_to_int(Arguments[0]);
			}
		}
		unsigned win = random_int(10,1);
		if (User.Money<10){
			std::string m = "You need at least "+float_to_cash(10)+" to play Dice.";
			ircSendMessage(m,getReceiver(Message));
			return;
		}
		if (Arguments.size()>1){
			std::string mh = int_to_string(cash_to_float(Arguments[1]));
			if (is_number(mh) and string_to_int(mh)>0 ){
				if (string_to_int(mh)>User.Money){
					ircSendMessage("Are you trying to trick me, mate?"+
					std::string(" You can't bet more %redmoney%reset than you have!"),getReceiver(Message));
					return;
				}
				win = string_to_int(mh);
			}
		}
		unsigned n = random_int(6,1);
		std::string out = "Rolling the dice... "+int_to_string(n)+"!";
		if (n==his){
			moneyAdd(&User,win);
			out+=" You chose "+int_to_string(his)+". You win "+float_to_cash(win)+"!";
		}else{
			moneyRemove(&User,win);
			out+=" You chose "+int_to_string(his)+". You lose "+float_to_cash(negative(win))+".";
		}
		out+=" Now you have "+float_to_cash(User.Money)+".";
		ircSendMessage(out,getReceiver(Message));
	}
	/* !bet */
	if (Command==Ctype+"bet"){
		if (!isIdentified(&User,Message))
			return;
		if (Arguments.size()==0){
			ircSendMessage("Place how much are you willing to bet!",getReceiver(Message));
			return;
		}
		Arguments[0] = int_to_string( cash_to_float(Arguments[0]) );
		if (!(is_number(Arguments[0]) and string_to_int(Arguments[0])>0)){
			ircSendMessage("Invalid quantity.",getReceiver(Message));
			return;
		}
		int64_t bet = string_to_int(Arguments[0]);
		if (bet>User.Money){
			ircSendMessage("Are you trying to trick me, mate?"+
			std::string(" You can't bet more %redmoney%reset than you have!"),getReceiver(Message));
			return;
		}
		moneyRemove(&User,bet);
		unsigned n1 = random_int(20,1);
		if((!(n1%2)) and (n1!=2)){
			bet*=2.0;
			moneyAdd(&User,bet);
			ircSendMessage("Congratulations, you've won "+
			float_to_cash(bet/2.0)+"! Now you have "+
			float_to_cash(User.Money)+".",getReceiver(Message));
		}else{
			ircSendMessage(std::string("Sorry, you've lost. Now you have ")+
			float_to_cash(User.Money)+".",getReceiver(Message));
		}
		return;
	}
	/* scissors */
	if (Command==Ctype+"scissors" or Command==Ctype+"rock" or Command==Ctype+"paper"){
		if (!isIdentified(&User,Message))
			return;
		uint8_t hishand;
		if (Command==Ctype+"scissors"){
			hishand=0;
		}else
		if (Command==Ctype+"rock"){
			hishand=1;
		}else
		if (Command==Ctype+"paper"){
			hishand=2;
		}
		std::string h[] = {"Scissors","Rock","Paper"};
		uint8_t yourhand = random_int(2,0);
		unsigned winprize = random_int(10,1);
		if (User.Money<10){
			ircSendMessage("You need at least $10 to play Scissors.",getReceiver(Message));
			return;
		}
		if (Arguments.size()>0){
			std::string mh = int_to_string(cash_to_float(Arguments[0]));
			if (is_number(mh) and string_to_int(mh)>0 ){
				if (string_to_int(mh)>User.Money){
					ircSendMessage("Are you trying to trick me, mate?"+
					std::string(" You can't bet more %redmoney%reset than you have!"),getReceiver(Message));
					return;
				}
				winprize = string_to_int(mh);
			}
		}
		std::string line;
		line=h[yourhand]+".";
		if (hishand==yourhand){
			line += " Draw.";
		}else
		if ( 
			(hishand==0 and yourhand==1) or
			(hishand==1 and yourhand==2) or
			(hishand==2 and yourhand==0)
			){
			moneyRemove(&User,winprize);
			line += " You lose "+float_to_cash(negative(winprize))+". Now you have "
			+float_to_cash(User.Money)+".";
		}else
		if (
			(hishand==0 and yourhand==2) or
			(hishand==1 and yourhand==0) or
			(hishand==2 and yourhand==1)
			){
			moneyAdd(&User,winprize);
			line += " You win "+float_to_cash(winprize)+". Now you have "
			+float_to_cash(User.Money)+".";
		}
		ircSendMessage(line,getReceiver(Message));
	}
	
	/*** Fun/Useful commands ***/
	
	/* !math */
	if (Command==Ctype+"m" or Command==Ctype+"math" or Command==Ctype+"alme"){
		std::string in;
		for (unsigned i=0; i<Arguments.size(); i++){
			in+=Arguments[i];
			if (i<Arguments.size()-1)
				in+=" ";
		}
		in = math_engine(in);
		if (in.size()>0)
			ircSendMessage(in,getReceiver(Message));
	}

	/* !help */
	if (Command==Ctype+"help"){
		ircSendMessage("Helper 2.0",User.Nick);
		std::string out = "Commands for "+Message->Channel+": ";
		if (Message->Type){
			out = "Commands: ";
		}
		for(unsigned i=0; i<CommandN; i++){
			out+=Commands[i];
			if (i<CommandN-1){
				out+=" ";
			}
		}
		out += " | Total: "+int_to_string(CommandN)+".";
		ircSendMessage(out,User.Nick);
	}
	/* !enable/!disable */
	if (Command==Ctype+"enable" or Command==Ctype+"disable"){
		bool en = 1;
		if (Command==Ctype+"disable"){
			en = 0;
		}
		if (!requieredUserlevel(4,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		/* shrinker */
		std::string nwhat=tNull;

		if(Arguments[0]=="shrinker"){
			nwhat="Shrinker";		
		}else
		if(Arguments[0]=="title"){
			nwhat="Title";		
		}
			
		if (nwhat==tNull)
			return;

		std::string chan = Message->Channel;
		if (Arguments.size()>1){
			chan = Arguments[1];
		}
		int chann = channelID(chan);
		if (chann==-1){
			ircSendMessage("Unknown channel.",getReceiver(Message));
			return;
		}
		
		bool *what = NULL;

		if (nwhat=="Shrinker"){
			what = &Config.Channels[chann].URLShrinker;
		}
		
		if (nwhat=="Title"){
			what = &Config.Channels[chann].WebTitle;
		}
		
		if (en){
			if ((*what)){
				ircSendMessage(nwhat+" is already enabled on "+chan+".",getReceiver(Message));
			}else
				ircSendMessage(nwhat+" enabled on "+chan+".",getReceiver(Message));
			(*what) = 1; 
			return;
		}
		
		if (!(*what)){
			ircSendMessage(nwhat+" is already disabled on "+chan+".",getReceiver(Message));
		}else
			ircSendMessage(nwhat+" disabled on "+chan+".",getReceiver(Message));
			
		(*what) = 0; 
		return;
	}
	/* !listen */
	if (Command==Ctype+"listen"){
        std::string x[] = {
			"Yes?","Yes, sir?","What is it, sir?", "Right here, sir.",
			"I am here, sir.","Yes, "+User.Nick+".","What is it, "+User.Nick+"?",
			"Right here, "+User.Nick+".","Sir.", "I am here, "+User.Nick+"."
		};
		unsigned n = sizeof(x)/sizeof(std::string);
		n = random_int(n-1,0);
		ircSendMessage(x[n],getReceiver(Message));
	}
	/* !active */
	if (Command==Ctype+"active"){
		User.cMSG--;
		if (nicksCheck(Arguments.size(),Message))
			return;
		for (unsigned i=0; i<Arguments.size(); i++){
			int ID = coreUserIDfromNick(Arguments[i]);
			if (ID==-1)
				continue;
			User.cMSG++;
			std::string What = "active";
			if (Config.Users[ID].LastActive=="Join"){
				What="joining to "+Config.Users[ID].Channel;
			}else
			if (Config.Users[ID].LastActive=="Quit"){
				What="quitting";
			}else
			if (Config.Users[ID].LastActive=="Message"){
				What="messaging on "+Config.Users[ID].Channel;
				if (Config.Users[ID].Channel=="PM")
					What="messaging me through PM";
			}else
			if (Config.Users[ID].LastActive=="Part"){
				What="parting from "+Config.Users[ID].Channel;
			}else
			if (Config.Users[ID].LastActive=="Mode"){
				What="changing a mode on "+Config.Users[ID].Channel;
			}else
			if (Config.Users[ID].LastActive=="Nick"){
				What="changing nick from '"+Config.Users[ID].Channel+"' to '"+Config.Users[ID].Nick+"'";
			}
			ircSendMessage("I saw "+Arguments[i]+" "+What+" "+
			seconds2hours(get_tocks()-Config.Users[ID].LastActivity)+" ago.",
			getReceiver(Message));
		}
	}
	
	/*** Administrative commands */
	if (Command==Ctype+"say" or Command==Ctype+"talk"){
		if (User.userLevel<2 or Arguments.size()==0)
			return;
		std::string Whole="";
		for (unsigned i=0; i<Arguments.size(); i++){
			Whole+=Arguments[i];
			if (i<Arguments.size()-1)
				Whole+=" ";
		}
		ircSendMessage(Whole,getReceiver(Message));
		
	}
	if (Command==Ctype+"channels" and User.userLevel==5){
		if (!requieredUserlevel(3,&User,Message))
			return;
		std::string all;
		for(unsigned i=0; i<Config.Channels.size(); i++){
			all+=Config.Channels[i].Channel;
			if (i<Config.Channels.size()-1){
				all+=" | ";
			}
		}
		if (!Config.Channels.size()){
			all="Currently on no channel";
		}
		all+=".";
		ircSendMessage(all,getReceiver(Message));
	}
	/* Remove */
	if (Command==Ctype+"remove"){
		if (User.userLevel<5)
			return;
		unsigned c = 0;
		for (unsigned i=0; i<Arguments.size(); i++){
			int ID = coreUserIDfromNick(Arguments[i]);
			if (ID==-1)
				continue;
			if (coreUserDelete(Config.Users[ID].Username)){
				c++;
				if (Arguments.size()<=5){
					ircSendMessage(Arguments[i]+" removed.",getReceiver(Message));
				}
			}
		}
		if (c>0 and Arguments.size()>5){
			ircSendMessage(int_to_string(c)+" users removed.",getReceiver(Message));
		}
		return;
	}
	/* !give */
	if (Command==Ctype+"give" and User.userLevel==5){
		if (!requieredArgn(Arguments.size(),2,Message))
			return;
		int ID = coreUserIDfromNick(Arguments[0]);
		if (ID==-1){
			ircSendMessage("Unknown nick.",getReceiver(Message));
			return;
		}
		std::string m = int_to_string( cash_to_float(Arguments[1]) );
		if (!(is_number(m) and string_to_int(m)!=0)){
			ircSendMessage("Invalid quantity.",getReceiver(Message));
			return;
		}
		if (string_to_int(m)>0){
			moneyAdd(&Config.Users[ID],string_to_int(m));
			ircSendMessage("Added "+float_to_cash(string_to_int(m))+" to "+
			Arguments[0]+"'s account.",getReceiver(Message));
			return;
		}	
		moneyRemove(&Config.Users[ID],positive(string_to_int(m)));
		ircSendMessage("Removed "+float_to_cash(string_to_int(m))+" from "+
		Arguments[0]+"'s account.",getReceiver(Message));
		return;
	}
	/* !restart */
	if (Command==Ctype+"restart" or Command==Ctype+"reboot"){
		if (!requieredUserlevel(5,&User,Message))
			return;
		coreRestart();
		return;
	}
	/* !exit */
	if (Command==Ctype+"exit" or Command==Ctype+"die"){
		if (!requieredUserlevel(5,&User,Message))
			return;
		coreEnd();
	}
	/* !time */
	if (Command==Ctype+"time"){
		if (!requieredUserlevel(2,&User,Message))
			return;
		std::string up = seconds2hours(get_tocks()-Config.Uptime);
		ircSendMessage("Uptime: "+up+".",getReceiver(Message));
		return;
	}
	if (Command==Ctype+"host"){
		if (!requieredUserlevel(4,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		int ID = coreUserIDfromNick(Arguments[0]);
		if (ID==-1){
			ircSendMessage("Unknown nick.",getReceiver(Message));
			return;
		}	
		ircSendMessage(Config.Users[ID].Host,getReceiver(Message));
	}
	/* !Identify */
	if (Command==Ctype+"Identify"){
		if (!requieredArgn(Arguments.size(),1,Message))
			return;
		if (Arguments[0]==pwdLevel5){
			User.userLevel = 5;
			ircSendMessage("You are now userLevel 5.",getReceiver(Message));
			coreRefreshUserData(User.Username);
		}else
		if (Arguments[0]==pwdLevel4){
			User.userLevel = 4;
			ircSendMessage("You are now userLevel 4.",getReceiver(Message));
			coreRefreshUserData(User.Username);
		}else
		if (Arguments[0]==pwdLevel3){
			User.userLevel = 3;
			ircSendMessage("You are now userLevel 3.",getReceiver(Message));
			coreRefreshUserData(User.Username);
		}
		return;
	}
	/* !logout */
	if (Command==Ctype+"logout" or Command==Ctype+"unIdentify"){
		if (User.userLevel<2)
			return;
		User.userLevel=1;
		coreRefreshUserData(User.Username);
		ircSendMessage("You are now userLevel 1.",getReceiver(Message));
		return;
	}
	if (Command==Ctype+"whois"){
		//ircWhois(Arguments[0]);
	}
	/* !who */
	if (Command==Ctype+"who"){
		if (Arguments.size()==0)
			Arguments.push_back(User.Nick);
		if (User.userLevel<3 and Arguments.size()>3)
			Arguments.erase(Arguments.begin()+3,Arguments.end());
		if (nicksCheck(Arguments.size(),Message))
			return;
		User.cMSG--;
		for (unsigned i=0; i<Arguments.size(); i++){
			int ID = coreUserIDfromNick(Arguments[i]);
			if (ID==-1){
				ircSendMessage("Unknown nick.",getReceiver(Message));
				continue;
			}
			typeUser &cUser = Config.Users[ID];
			std::string Whole=cUser.Nick;
			Whole+=" | "+int_to_string(cUser.Lines)+" lines";
			Whole+=" | Last time seen active "+	seconds2hours(get_tocks()-Config.Users[ID].LastActivity)+" ago";
			if (isChannel (Config.Users[ID].Channel))
				Whole += " on "+Config.Users[ID].Channel;
			Whole+=" | Bank: "+float_to_cash(cUser.Money)+".";
			Whole+=" | userLevel: "+int_to_string(Config.Users[ID].userLevel);

			if (Config.Users[ID].Ignore)
				Whole+=" | Currently being ignored ";

			if (!Config.Users[ID].Identified)
				Whole+=" | Not identified ";

			if (Config.Users[ID].Temporal)
				Whole+=" | Temporal user(Identify! Don't be lazy)";				
			
			User.cMSG++;
			ircSendMessage(Whole,getReceiver(Message));
		}
		return;
	}
	/* !join */
	if (Command==Ctype+"join"){
		if (!requieredUserlevel(2,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		for (unsigned i=0; i<Arguments.size(); i++){
			if (ircJoinChannel(Arguments[i])!=0)
				ircSendMessage("Couldn't join to "+Arguments[i]+".",getReceiver(Message));
		}
		return;
	}
	/* !part */
	if (Command==Ctype+"part"){
		if (Arguments.size()==0 and Message->Type==0)
			Arguments.push_back(Message->Channel);
		if (!requieredUserlevel(2,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
			for (unsigned i=0; i<Arguments.size(); i++){
				if (ircPartChannel(Arguments[i])!=0)
					ircSendMessage("Couldn't part from "+Arguments[i]+".",getReceiver(Message));
			}
		return;
	}
	/* !imitate */
	if (Command==Ctype+"imitate"){
		if (!requieredUserlevel(4,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		int ID = coreUserIDfromNick(Arguments[0]);
		if (ID==-1){
			ircSendMessage("Unknown nick.",getReceiver(Message));
			return;
		}
		if (Arguments.size()>1){
			Config.Users[ID].ImitateTo = Arguments[1];
		}
		std::string out = "";
		if (Config.Users[ID].Imitate){
			out = Config.Users[ID].Nick+" is no longer imitated.";
			Config.Users[ID].ImitateTo=tNull;
		}else{
			out = Config.Users[ID].Nick+" is now imitated.";
			if (Config.Users[ID].ImitateTo!=tNull){
				out += " Output: "+Config.Users[ID].ImitateTo+".";
			}
		}
		ircSendMessage(out,getReceiver(Message));
		Config.Users[ID].Imitate = !Config.Users[ID].Imitate;
		return;
	}
	/* !users */
	if (Command==Ctype+"users"){
		User.cMSG--;
		if (Arguments.size()==0)
			Arguments.push_back("asdf");
		if (User.userLevel<3 and Arguments.size()>3)
			Arguments.erase(Arguments.begin()+3,Arguments.end());
		for (unsigned i=0; i<Arguments.size(); i++){
			int8_t t = 0;	//All
			std::string type = "user";
			if (Arguments[i]=="temporal" or Arguments[i]=="tmp"){
				t = 1;	//Temporal users only
				type = "temporal user";
			}
			if (Arguments[i]=="registered" or Arguments[i]=="register"){
				t = 2;	//Registered users only
				type = "registered user";
			}
			if (Arguments[i]=="identified" or Arguments[i]=="Identify"){
				t = 3;	//Identified users only
				type = "identified user";
			}
			if (Arguments[i]=="rootuser" or Arguments[i]=="rtuser"){
				t = 4;	//rootUsers
				type = "rootuser";
			}
			User.cMSG++;
			unsigned c = 0;
			for (unsigned i=0; i<Config.Users.size(); i++){
				typeUser &C = Config.Users[i];
				if (t==1 and !C.Temporal){
					continue;
				}
				if (t==2 and C.Temporal){
					continue;
				}
				if (t==3 and !C.Identified){
					continue;
				}
				if (t==4 and C.userLevel<3){
					continue;
				}
				c++;
			}
			type = type+std::string(c>1 ? "s" : "");
			std::string o;
			o = "There "+std::string(c>1 ? "are" : "is" )+" currently "+std::string( c>0 ? int_to_string(c) : "no" ) +" "+type+".";
			ircSendMessage(o,getReceiver(Message));	
		}
		return;
	}
	/* !reload */
	if (Command==Ctype+"reload"){
		if (!requieredUserlevel(3,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		if (Arguments[0]=="list" or Arguments[0]=="lists"){
			coreLoadLists();
			ircSendMessage("Lists reloaded.",getReceiver(Message));
			return;
		}
	}
	/* !unignore */
	if (Command==Ctype+"unignore"){
		if (!requieredUserlevel(2,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		
		for (unsigned i=0; i<Arguments.size(); i++){
			if (str_at(Arguments[i],0)!="@")
				continue;
			Arguments[i].erase(0,1);
			if (unignore_host(Arguments[i])){
				ircSendMessage(Arguments[i]+" is no longer ignored.",getReceiver(Message));
			}else{
				ircSendMessage(Arguments[i]+" is not being ignored.",getReceiver(Message));
			}
			Arguments.erase(Arguments.begin()+i);
		}
		unsigned  c = 0;
		for (unsigned i=0; i<Arguments.size(); i++){
			int ID = coreUserIDfromNick(Arguments[i]);
			if (ID==-1)
				continue;
			if (Config.Users[ID].Ignore==1){
				Config.Users[ID].Ignore=0;
				Config.Users[ID].autoUnignore=0;
				Config.Users[ID].cMSG=0;
				Config.Users[ID].autoTimeout=0;
				coreRefreshUserData(Config.Users[ID].Username);
				c++;
				if (Arguments.size()<=5){
					ircSendMessage(
						Arguments[i]+" is no longer ignored.",
						getReceiver(Message)
					);
				}
				/* jump to next step */
				continue;
			}
			if (Arguments.size()<=5){	
				ircSendMessage(Arguments[i]+" is not being ignored.",getReceiver(Message));
			}
		}
		if (c>0 and Arguments.size()>5){
			ircSendMessage(int_to_string(c)+" users are no longer ignored.",getReceiver(Message));
		}
		return;
	}
	/* !ignore */
	if (Command==Ctype+"ignore"){
		if (!requieredUserlevel(2,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		for (unsigned i=0; i<Arguments.size(); i++){
			if (str_at(Arguments[i],0)!="@")
				continue;
			Arguments[i].erase(0,1);
			if (ignore_host(Arguments[i])){
				ircSendMessage(Arguments[i]+" is now ignored.",getReceiver(Message));
			}else{
				ircSendMessage(Arguments[i]+" is already being ignored.",getReceiver(Message));
			}
			Arguments.erase(Arguments.begin()+i);
		}
		unsigned  c = 0;
		for (unsigned i=0; i<Arguments.size(); i++){
			int ID = coreUserIDfromNick(Arguments[i]);
			if (ID==-1)
				continue;
			if (Config.Users[ID].userLevel>User.userLevel){
				ircSendMessage(std::string("You cannot use this command on someone with a")+
				" higher Userlevel("+int_to_string(Config.Users[ID].userLevel)+
				") than you("+int_to_string(User.userLevel)+").",getReceiver(Message));
				continue;
			}
			Config.Users[ID].Ignore=1;
			Config.Users[ID].autoUnignore=0;
			Config.Users[ID].cMSG=0;
			Config.Users[ID].autoTimeout=0;
			coreRefreshUserData(Config.Users[ID].Username);
			c++;
			if (Arguments.size()<=5){
				ircSendMessage(Arguments[i]+" is now ignored.",getReceiver(Message));
			}
		}
		if (c>0 and Arguments.size()>5){
			ircSendMessage(int_to_string(c)+" users are now ignored.",getReceiver(Message));
		}
		return;
	}
	/* !prefix */
	if (Command==Ctype+"cmdprefix"){
		if (!requieredUserlevel(5,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		if (Message->Type){
			ircSendMessage("This only works with channels.",getReceiver(Message));
			return;
		}
		if (Arguments[0]!="~" and Arguments[0]!="!" and Arguments[0]!="."){
			ircSendMessage("Only acceptable prefixes: '~', '!' and '.'.",getReceiver(Message));
			return;
		}
		Config.Channels[channelID(Message->Channel)].Cmdprefix = Arguments[0];
		ircSendMessage("All commands on "+Message->Channel+" are now called with '"
		+Arguments[0]+"'.",getReceiver(Message));
		return;
	}
	/* !name */
	if (Command==Ctype+"name"){
		if (!requieredUserlevel(5,&User,Message) or
			!requieredArgn(Arguments.size(),1,Message))
			return;
		ircChangeNick(Arguments[0]);
		return;
	}
	
	return;
}
