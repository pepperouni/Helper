#ifndef TOOLS_H
	#define TOOLS_H
	#include <stdint.h>
	#ifndef TYPES_H
		#include <vector>
		#include <string>
		typedef std::vector<std::string> typeList; 
	#endif

	void endprocess();
	//Time
	unsigned int get_tocks();
	std::string seconds2hours(long long int Seconds);
	uint64_t get_ticks();
	void suspend(unsigned Sleeptime);
	std::string time_get(std::string element);
	
	//Strings
	bool is_number(std::string number);
	
	long long int string_to_int (std::string string);
	std::string int_to_string (long long int number);
	
	std::string substring (std::string string,uint16_t start,uint16_t end);
	void split_words(std::string input,std::string splitter,typeList *list);
	std::string string_to_upper(std::string Input);
	std::string string_to_lower(std::string Input);
	std::string remove_marks(std::string Input,std::string Exception);
	double string_to_float(std::string number);
	std::string float_to_string(double Float);
	std::string remove_coincidences(std::string Input,std::string coinc);
	std::string remove_endline(std::string Input);
	bool is_string_upper(std::string Input);
	std::string str_at(std::string Input,unsigned x);
	std::string remove_spaces(std::string Input);
	float fractional (float number);
	int64_t positivei(int64_t n);
	
	//MATH
	int random_int(int max,int min);
	float fractional (float number);
	int64_t round_int (double number);
	int64_t floor_int (double number);
	int64_t ceil_int (double number);
	int64_t factorial(int n);
	float negative(float n);
	float positive(float n);
	float inversive(float n);
	
	//Console
	void color_set(std::string color);
	bool out_message(std::string message,std::string title,std::string color);
	
	//Files
	bool directory_exists(std::string Directory);
	bool file_exists(std::string Directory);
	std::string filename_get(std::string File);
	std::string fileformat_get(std::string Filename);
	bool files_find(std::string dir,std::string type,std::string format,bool r,bool fp,typeList *list);

#endif
