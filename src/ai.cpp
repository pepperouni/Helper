#include <iostream>
#include <string>
#include <fstream>
#include "irc.h"
#include "tools.h"
#include "core.h"
#include "ai.h"

extern ConfigData Config;

std::string getReceiver(typeMessage *Message);
std::string PythonScript(std::string script,std::string input);

std::string dictionary(std::string w){
	std::string file =  Config.rtdir+"/lists/dictionary.txt";
	std::string error = "#ERROR";
	if (!file_exists(file))
		return error;
	std::ifstream DictionaryFile;
	DictionaryFile.open(file.c_str());
	std::string buffer;
	std::string out = error;
	while (std::getline(DictionaryFile,buffer)){
		if (!(string_to_upper(w)==remove_marks(buffer,"")))
			continue;
		std::string newout = "";
		while (std::getline(DictionaryFile,buffer)){
			buffer = remove_endline(buffer);
			if (is_string_upper(buffer) and buffer!="")
				break;
			newout+=buffer+" ";
		}
		out = newout;
		break;
	}
	DictionaryFile.close();
	return out;
}

bool ignore_check(typeMessage *Message){
	typeUser &User = Config.Users[ coreUserID(Message->Username) ];
	
	std::string mUser = Message->Nick+"("+Message->Username+"@"+Message->Host+")";
	
	/* Unignore */
	if (User.autoUnignore and User.Ignore){
		if (get_tocks()-User.autoTimeout>Config.Ignore_timeout){
			User.autoUnignore=0;
			User.cMSG=0;
			User.Ignore=0;
		}
	}
	
	/* Drop message if he's being ignored */
	if (User.Ignore){
		coreAddLog(mUser+" tried to reach Helper but he is ignored.");
		return 1;
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
		ircSendMessage(pr,Message->Nick);
	}
	
	/* Ignore */
	if (User.cMSG>Max_mpm){
		std::string pr;
		pr = "You are now being ignore for spamming. You'll be unignored in ";
		pr += seconds2hours(Config.Ignore_timeout)+".";
		ircSendMessage(pr,Message->Nick);
		ignore_host(User.Host);
		User.autoUnignore=1;
		User.Ignore=1;
		User.autoTimeout=get_tocks();
		coreRefreshUserData(Message->Username);
		return 1;
	}
	
	/* Timeout, resets counters */
	if (get_tocks()-User.cStrTimeout>60){
		User.cMSG=0;
		User.cStrTimeout=get_tocks();
	}
	return 0;
}

void aiProcessInput(typeMessage *Message){
	typeUser &User = Config.Users[ coreUserID(Message->Username) ];
	if ((User.userLevel<5 and Message->Type) or User.Ignore or is_ignore_host(User.Host)!=-1)
		return;
	if (!Message->Message.length())
		return;
	
	typeList Entry;
	split_words(Message->Message," ",&Entry);
	
	std::string clnm = string_to_lower(Config.clientName);
	
	if (Entry.size()==0)
		return;
		
	unsigned x = channelID(Message->Channel);
	
	if (Config.Channels[x].URLShrinker or Config.Channels[x].WebTitle){
		for (unsigned i=0; i<Entry.size(); i++){
			bool isurl = 0;
			if (Entry[i].find("http://")==0 or Entry[i].find("https://")==0 or
			Entry[i].find("www.")==0)
				isurl=1;
			
			if (!isurl){
				continue;
			}
			
			std::string output	= "";
			std::string shrunk	= tNull;
			std::string title	= tNull;
			if (Entry[i].length()>90 and Config.Channels[x].URLShrinker){
				shrunk	= PythonScript("shrinker.py",Entry[i]);
			}
			if (Config.Channels[x].WebTitle){
				title	= PythonScript("title.py",Entry[i]);
			}				
			if (title!=tNull and title!="#ERROR"){
				output	+= title;
			}
			if (shrunk!=tNull and shrunk!="#ERROR"){
				if (output.length()>0){
					output+=" | ";
				}
				output+=shrunk;
			}
			if (output.length()>0 and output!=tNull and output!="#ERROR"){
				ircSendMessage(output,getReceiver(Message));
			}
			break;
		}

	}
	
	for (unsigned i=0; i<Entry.size(); i++){
		Entry[i]=remove_marks(string_to_lower(Entry[i]),"");
	}	
	
	/* Salute */
	if (Entry[0]==clnm and Entry.size()==1){
		if (ignore_check(Message))
			return;
		ircSendMessage(Message->Nick+": Yes?",getReceiver(Message));
		return;
	}
	/* what is */
	//if (Entry[0]==clnm and Entry[1]=="what" and User.userLevel>3){
		//if (Entry.size()!=4)
			//return;
		//if (Entry[2]!="is" and Entry[2]!="are")
			//return;
		///* Ignore */
		//if (User.cMSG>Config.Max_mpm and User.userLevel<2){
			//std::string pr;
			//pr = "You are now being ignore for spamming. You'll be unignored in ";
			//pr += seconds2hours(Config.Ignore_timeout)+".";
			//ircSendMessage(pr,Message->Nick);
			//User.autoUnignore=1;
			//User.Ignore=1;
			//User.autoTimeout=get_tocks();
			//return;
		//}
		//std::string out = dictionary(Entry[3]);
		//if (out=="#ERROR")
			//out = "I have no idea.";
		//std::string who = getReceiver(Message);
		//if (out.length()>1600)
			//out = substring(out,0,1600);
		//ircSendMessage(out,who);
		//User.cMSG+=round_int(Config.Max_mpm/2.0)+1;
		//return;
	//}
	/* 8Ball */
	if (Entry[0]=="8ball"){
		ignore_check(Message);
		if (ignore_check(Message))
			return;
		static const std::string answers[]={
			"Most likely","Very doubtful","Yes definitely","Yes","No","Outlook so so",
			"A definite yes","Who knows","Looking good","Not now","Go for it",
			"Are you kidding?","Forget about it","I have my doubts","Yes, in due time",
			"You will have to wait","Absolutely","My sources say no"
		};
		static const uint8_t answersn = sizeof(answers)/sizeof(std::string);
		ircSendMessage(answers[random_int(answersn-1,0)],getReceiver(Message));
		User.cMSG++;
		return;
	}
	
	/* or questions */
	if (Entry[0]==clnm and Entry.size()>2){
		if (ignore_check(Message))
			return;
		unsigned last = 1;
		bool f = 0;
		typeList Options;
		for (unsigned i=2; i<Entry.size(); i++){
			if (!( (Entry[i]=="or" and i<Entry.size()-1) or i==Entry.size()-1))
				continue;
			f = 1;
			if ((Entry[i]=="or" and Entry[i-1]=="or"))
				return;
			if (i<Entry.size()-1 and Entry[i]=="or"){
				if (Entry[i+1]=="or")
					return;
			}
			std::string option = "";
			for (unsigned x = last; x<=i; x++){
				if (Entry[x]=="or")
					continue;
				option += Entry[x];
				option += " ";
			}
			last=i+1;
			Options.push_back(remove_coincidences(option," "));
		}
		if (!f)
			return;
		ircSendMessage(Options[random_int(Options.size()-1,0)],getReceiver(Message));
		User.cMSG++;
		return;
	}
	
}
