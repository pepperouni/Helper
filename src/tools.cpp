#include <iostream>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sstream>
#include <stdio.h>
#include <algorithm>
#include <iomanip>
#include <signal.h>
#include <stdint.h>


#include <vector>
#include <string>
typedef std::vector<std::string> typeList;

//tools v2

void endprocess(){ 
    //kill(getpid(), SIGTERM);
    exit(0);
    return;
}

///Strings

std::string str_at(std::string Input,unsigned x){
	if (x>Input.length())
		return "";
	std::string output = Input;
	output = output.at(x);
	return output;
}

bool is_number(std::string number){
	static const std::string numbers[]={
		"0","1","2","3","4","5",
		"6","7","8","9","."
	};
	
	/* remove signs */
	if (str_at(number,0)=="+" or str_at(number,0)=="-")
		number.erase(0,1);

	if (number.length()==0)
		return 0;
	uint8_t size = sizeof(numbers)/sizeof(std::string);
	bool n=0;
	for (unsigned i=0; i<number.length(); i++){
		n=0;
		for (unsigned e=0; e<size; e++){
			if (str_at(number,i)==numbers[e])
				n=1;
		}
		if (n==0)
			return 0;
	}
    return 1;

}

std::string remove_spaces(std::string Input){
	std::string in = Input;
	while (in.find(" ")!=-1){
		in.erase(in.find(" "),1);
	}
	return in;
}

std::string remove_endline(std::string Input){
	std::string InputString = Input;
	std::string Buffer;
	if (InputString.length()>0){
		Buffer = str_at(InputString,InputString.length()-1);
		while ( (Buffer=="\n" or Buffer=="\r" )  && InputString.length()>0 ){
			InputString.erase(InputString.length()-1,1);
			if (InputString.length()==0)
				break;
			Buffer = InputString.at(InputString.length()-1);
		}		
	}
	return InputString;
}

std::string remove_coincidences(std::string Input,std::string c){
	std::string out = Input;
	for (unsigned i=0; i<out.length(); i++){
		if (i<out.length()-1){
			while (i<out.length()-1){
				if (str_at(out,i)!=c or str_at(out,i+1)!=c)
					break;
				out.erase(i,1);
			}
		}
	}
	return out;
}


std::string remove_marks(std::string Input,std::string Exception){
    std::string out = Input;
    std::string Marks[] = {
        "!","?",",","'",".",";",":","(",")","[","]",
        "*","@","#","$","%","^","&","-","_","+","=",
    };
    uint8_t n = (sizeof(Marks)/sizeof(std::string));
    for (uint8_t i=0; i<n; i++){
		if ( out.find(Marks[i],0)!=-1 && Exception.find(Marks[i],0)==-1){
				out.erase(out.find(Marks[i],0),1);
		}
    }
    return out;
}

bool is_string_upper(std::string Input){
	unsigned n = 0;
	char c;
	for (unsigned i=0; i<Input.length(); i++){
		c=Input.at(i);
		if (isupper(c))
			n++;
	}
	if (n==Input.length())
		return 1;
	return 0;
}


std::string string_to_upper(std::string Input){
	std::string Output=Input;
	std::transform(Output.begin(), Output.end(),Output.begin(), ::toupper);
	return Output;
}

std::string string_to_lower(std::string Input){
	std::string Output=Input;
	std::transform(Output.begin(), Output.end(),Output.begin(), ::tolower);
	return Output;
}


long long int string_to_int (std::string string){
    return atoll(string.c_str());
}

double string_to_float(std::string s){
   std::istringstream i(s);
   double x;
   if (!(i >> std::fixed  >> std::setprecision(11) >> x))
     return 0;
   return x;
}

std::string float_to_string (double Float){
	std::ostringstream ss;
	ss << std::fixed << std::setprecision(11) << Float;
	std::string s(ss.str());
	for (unsigned i=s.length()-1; i>0; i--){
		if (str_at(s,i)=="."){
			s.erase(i,1);
			break;
		}
		if (str_at(s,i)!="0")
			break;
		if (str_at(s,i)=="0"){
			s.erase(i,1);
		}
	}
	return s;
}

std::string int_to_string (long long int number){
    char buffer[30];
	sprintf(buffer,"%lld",number);
	std::string out = buffer;
    return out;
}

std::string substring (std::string string,uint16_t start,uint16_t end){
    if (start<string.length()){
        std::string substring = "";
        for (unsigned i=start; i<=end; i++){
            substring+=string.at(i);
        }
        return substring;
    }
    return "";
}

void split_words(std::string input,std::string split,typeList *list){
	if ( (*list).size()!=0 ){
		(*list).clear();
	}
	std::string whole = remove_coincidences(input,split);
	unsigned e = 0;
	for (unsigned i=0; i<whole.length(); i++){
		if (str_at(whole,i)==split or i==whole.length()-1){
			if (i==0 and i!=(whole.length()-1)){
				e=i+1;
				continue;
			}
			unsigned h = i;
			if (i<whole.length()-1)
				h--;
			(*list).push_back(substring(whole,e,h));
			e=i+1;
		}
	}
	if ((*list).size()>0){
		std::string last = (*list)[(*list).size()-1];
		if (str_at(last,last.length()-1)==split)
			(*list)[(*list).size()-1].erase(last.length()-1,1);
	}
	return;
}

//MATH
int64_t factorial(int n){
    uint64_t h=1;
    int unsigned i=0;
    int t=n;
    if (t<0)
        t=t*-1;
    for (i=1;i<=t;i++)
        h=h*i;
    if (n<0)
        h=h*-1;
    return h;
}

float fractional (float number){
    return (float)number-((int)number);
}


int64_t ceil_int (double number){
    return (int64_t)number+1;
}

int64_t floor_int (double number){
    return  (int64_t)number;
}

int64_t round_int (double number){
    if (fractional(number) >= 0.5){
        return (int)number+1;
    }else
    if (fractional(number) < 0.5){
        return (int)number;
    }
}

float positive(float n){
	if (n<0)
		return n*-1;
	return n;
}

int64_t positivei(int64_t n){
	if (n<0)
		return n*-1;
	return n;
}

float negative(float n){
	if (n>0)
		return n*-1;
	return n;
}

float inversive(float n){
	return n*-1;	
}

int random_int(int max,int min){
	static unsigned int c,d,h,seed;
    static int result;	
    const static int unsigned m=8;		//accuracy
    size_t cOrig,dOrig;						//c and d origin
    max++;
    if (min<0)
        max+=(min*-1);
    dOrig=(size_t)(&result);		
    //place c and d possible origins
    if (dOrig<time(NULL))
        cOrig=time(NULL)-dOrig;
    else
    if (dOrig>time(NULL))
        cOrig=dOrig-time(NULL);
    //reset seed
    seed=0;
    //set seed
    for (h=0; h<m; h++){
        ///Re-Normalize c and d
        d=(size_t)(&c)+c+(h^m)+d; 
        c=(time(NULL)%(d+c))+h;
        ///Adding c and d to seed
        seed=seed+c+max+d;
        ///Normalize c and d
        d=d%dOrig;
        c=c%cOrig;
    }
    //get
    seed=(seed%(c+d+max))+max;
    result=0;
    result=seed%max;
    max--;
    if (result<min)
        result+=(min);
    if (min<0)
        result+=min;
    if (result>max)
        result = (max+1);
    return result;
}

///CONSOLE
void color_set(std::string color){
    if ("white"==color){ 	//White color
        std::cout << "\033[0m";
    }else
    if ("yellow"==color){ 	//Yellow color
        std::cout << "\033[0;33m";
    }else
    if ("red"==color){ 		//Red color
        std::cout << "\033[0;31m";
    }else
    if ("dkred"==color){ 	//Dark Red color
        std::cout << "\033[0;31m";
    }else
    if ("grey"==color){ 	//Grey color
        std::cout << "\033[0;37m";
    }else
    if ("green"==color){ 	//Green color
        std::cout << "\033[0;32m";
    }else
    if ("blue"==color){ 	//Blue color
        std::cout << "\033[0;34m";
    }else
    if ("brown"==color){ 	//Brown color
        std::cout << "\033[0;33m";
    }else
    if ("magenta"==color){ 	//Magenta color
        std::cout << "\033[0;35m";
    }else
    if ("cyan"==color){ 	//Cyan color
        std::cout << "\033[0;36m";
    }
    return;
}


//Output a message with title
bool out_message(std::string message,std::string title,std::string color){
	if (message.length()==0 or title.length()==0)
		return 0;
	color_set(color);
	std::cout << "[" << title << "]: ";
	color_set("white");
	std::cout << message << std::endl;
    return 1;
}

///Time
std::string seconds2hours(long long int Seconds){
	unsigned int hours = Seconds/3600;
	float fractpartminutes=fractional(Seconds/3600.f);
	unsigned int minutes = fractpartminutes*60;
	float fractpartseconds=fractional(fractpartminutes*60);
	unsigned int seconds = round_int(fractpartseconds*60);
	std::string output="";
	if (hours){
		output+=int_to_string(hours)+" hour";
		if (hours>1){
			output+="s";
		}
		if (minutes)
			output+=", ";
	}
	if (minutes or (hours and seconds)){
		output+=int_to_string(minutes)+" minute";
		if (minutes>1)
			output+="s";
		if (seconds)
			output+=" and ";
	}
	if (seconds or (!hours and !minutes)){
		output+=int_to_string(seconds)+" second";
		if (seconds>1){
			output+="s";
		}
	}
	return output;
}
unsigned int date_to_days(int M,int D,int Y){
    return (367 * Y) - (7 * (Y + ((M + 9)/12))/4) - (3 * (((Y + (M - 9)/7)/100) + 1)/4)+ ((275 * M)/9) + D + 1721028;
}

unsigned int get_tocks(){ //Returns seconds since the epoch
    timeval tima;
    gettimeofday(&tima, 0);
    return (tima.tv_sec);
}
uint64_t get_ticks(){ 	//Returns milliseconds since the epoch
    timeval tima;
    gettimeofday(&tima, 0);
    return (tima.tv_sec)*1000+(tima.tv_usec) / 1000;
}

void suspend(int unsigned time){
    usleep(time*1000);
}

std::string time_get(std::string element){ //CurrentTime
    time_t currentTime;
    time (&currentTime);
    struct tm * ptm= localtime(&currentTime);
    std::string final;
    for (unsigned i=0; i<element.length(); i++){
		std::string c = str_at(element,i);
		if (c=="d")
			final+=int_to_string(ptm->tm_mday);
		if (c=="m")
			final+=int_to_string(ptm->tm_mon);
		if (c=="y")
			final+=int_to_string(ptm->tm_year);
		if (c=="h")
			final+=int_to_string(ptm->tm_hour);
		if (c=="mi")
			final+=int_to_string(ptm->tm_min);
		if (c=="s")
			final+=int_to_string(ptm->tm_sec);
		if (c=="a" or c=="all")
			final+=ctime(&currentTime);
	}
    return final;
}

///FILE
std::string filename_get(std::string File){
	std::string Buffer;
	for (uint8_t Index=File.length()-1;Index>0;Index--){
		Buffer=File.at(Index);
		if (Buffer=="/"){
			return substring(File,Index+1,File.length()-1);
		}
	}
	return File;
}
std::string fileformat_get(std::string Filename){
	std::string Buffer;
	std::string File = filename_get(Filename);
	for (uint8_t Index=File.length()-1;Index>0;Index--){
		Buffer=File.at(Index);
		if (Buffer=="."){
			return substring(File,Index,File.length()-1);
		}
	}
	return File;
}
bool files_find(std::string dir,std::string type,std::string format,bool r,bool fp,typeList *list){
	
	//dir: Directory where you want to look for file/folder
	//type: file(for files), dir(for directories) or all(for both)
	//format: desired formats to look for (.txt|.mp3|.mp4) several, (.txt) just one, if everything then "", empty.
	//recusively: looks for only in the root of the specified dir or every foldr in the folder (1 rly, 0 not recusively)
	//fp: place the list of files with fp or just the filename (1 fp, 0 filename only)
	//list
	//Example: files_find("/path/to/dir","file",".txt",0,0,&Vector,N);
	
	
    DIR *Directory;
    std::string FilenameGet; 			   //Filename
    std::string DirectoryGet; 			   //Directory + Filename
    struct dirent *ent;
    struct stat buf;
    
    //Format defines what kind of specific format must be file to be accepted (.txt, .mp3)
    //syntax: ".mp3|.txt|.whatever"
    bool Formats=0;
    //Check if format is empty, if it is, then its going to accept anykind of filename
    if (format!="")
		Formats=1;
    std::vector <std::string> AcceptedFragments;
    uint8_t AcceptedFragmentsN=0;
    if (Formats==1){
		uint8_t Index=0;
		uint8_t Last=0;
		std::string Buffer;
		bool FoundAnything=0;
		for (Index=0;Index<format.length();Index++){
			Buffer = format.at(Index);
			if (Buffer=="|" or Index==(format.length()-1)){
				if (Index==(format.length()-1))
					Index++;
				AcceptedFragments.push_back(substring(format,Last,Index-1));
				AcceptedFragmentsN++;
				FoundAnything=1;
				Last=Index+1;
			}
		}		
		if (FoundAnything==0)
			Formats=0;
	}
    
    //Type defines what kind of file/dir is going to find
    uint8_t AcceptedType=0; //0 = Files, 1 = Folders, 2 = File and Folder
    if (type=="file"){
		AcceptedType=0;
	}else
	if (type=="folder" or type=="dir"){
		AcceptedType=1;
	}else
	if (type=="all" or type=="every"){
		AcceptedType=2;
	}
	//Open dir
    Directory=opendir( dir.c_str() );
    //Check if the dir if not null
    if (Directory!=NULL){
		std::string Filename;
		//Do a loop until it ends looking for files
        while ( (ent = readdir (Directory)) ) {
            Filename = ent->d_name;
            //skip . and .. 
            if ( (Filename!=".") && (Filename!="..")  ){
                FilenameGet=ent->d_name; 	//Get filename
                DirectoryGet+=dir;	//Add Directory
                DirectoryGet+=FilenameGet;	//Add Filename
                stat(DirectoryGet.c_str(), &buf);
                //Directory
                if (S_ISDIR(buf.st_mode)) {
					if (AcceptedType==1 or AcceptedType==2){
						if (fp==0){
							(*list).push_back(FilenameGet);	
						}else{
							(*list).push_back(DirectoryGet);
						}
					}
					if (r==1){
						files_find(DirectoryGet+"/",type,format,r,fp,list);
					}
                }else
                //Normal File
                if (S_ISREG(buf.st_mode)) {
					if (AcceptedType==0 or AcceptedType==2){
						bool Acceptable=0;
						if (Formats==0)
							Acceptable=1;
						if (Formats==1){
							for (uint8_t Index=0;Index<AcceptedFragmentsN;Index++){
								if (fileformat_get(FilenameGet)==AcceptedFragments[Index]){
									Acceptable=1;
									break;
								}
							}
						}		
						if (Acceptable==1){			
							if (fp==0){
								(*list).push_back(FilenameGet);
							}else{
								(*list).push_back(DirectoryGet);
							}
						}
					}
                }
				DirectoryGet="";
            }
        }
    }else{
        return 0; //Return error
    }
    closedir (Directory);
    return 1;
}

bool file_exists(std::string Directory){ // Check if the file exists
    struct stat target_type;
    stat(Directory.c_str(), &target_type);
    if (S_ISREG(target_type.st_mode)) {
        return 1;
    }
    return 0;
}
bool directory_exists(std::string Directory){ //Check if the directory exists
    struct stat target_type;
    stat(Directory.c_str(), &target_type);
    if (S_ISDIR(target_type.st_mode)) {
        return 1;
    }
    return 0;
}
