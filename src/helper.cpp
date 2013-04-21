#include <iostream>
#include <fstream>
#include "core.h"
#include "commandline.h"
#include "tools.h"
#include "types.h"

extern ConfigData Config;

int main (int argc, char* argv[]){
	/* Solve parameters */
	Config.Parameters="";
	Config.Binary = argv[0];
	for (unsigned i=1; i<argc; i++){
		Config.Parameters+=argv[i];
		if (i<argc-1)
			Config.Parameters+=" ";
	}
	typeList Parameters;
	split_words(Config.Parameters," ",&Parameters);
	/* look for -rtdir */
	for (unsigned i=0; i<Parameters.size(); i++){
		if (Parameters[i]=="-rtdir" && i<Parameters.size()-1){
			coreSetrtdir(Parameters[i+1]);
			break;
		}
	}
	/* Start core */
	coreStart();
	/* Pass Post-arguments */
	for (unsigned i=0; i<Parameters.size(); i++){
		if (Parameters[i]=="--enable-min"){ /* min = Manual Input */
			Config.enMinput=1;
			coreSaveSettings();
		}
		if (Parameters[i]=="--disable-min"){ 
			Config.enMinput=0;
			coreSaveSettings();
		}
		if (Parameters[i]=="--enable-log"){ 
			Config.enableLog=1;
			coreSaveSettings();
		}
		if (Parameters[i]=="--disable-log"){ 
			Config.enableLog=0;
			coreSaveSettings();
		}
		if (Parameters[i]=="--enable-irc"){ 
			Config.enableIRC=1;
			coreSaveSettings();
		}
		if (Parameters[i]=="--disable-irc"){ 
			Config.enableIRC=0;
			coreSaveSettings();
		}
	}

	/* Manual Input loop */
	std::string Buffer;
	typeMessage ManualMessage;
	ManualMessage.Message="";
	ManualMessage.Message="root";
	ManualMessage.Username="root";
	ManualMessage.Host="helper.manual.input";
	ManualMessage.Channel="#HELPER";
	ManualMessage.Type=1; //Treat it as a PM
	while (1){
		if (Config.enMinput==0){
			suspend(100);
			continue;
		}
		getline(std::cin,Buffer);
		if (Buffer!=""){
			ManualMessage.Message=Buffer;
			CommandLine(&ManualMessage);
		}
		
	}
	return 0;
}
