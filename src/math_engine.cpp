#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>
#include <math.h>
#include "types.h"
#include "tools.h"

#define PI 3.1415926535

struct element_type {
	std::string element;
	int8_t type;
};

typedef std::vector<element_type> item_type; 

struct variable_type {
	std::string variable;
	std::string name;
};

typedef std::vector<variable_type> variable_list; 

/*
	Element types:
	-1  =  Undefined/unexisting
	0   =  Operator 
	1   =  Constant (Numbers)
	2   =  Variable
*/

item_type items;
variable_list variables;

static std::string Operator[] = {
	/* symbol operators */
	"++","--","!=","+","-","*",
	"/","!","%%","==","=","^",
	"%","%log",	"<=","<",">=",">",
	"&","||","%","!n","#",
	"%sin","%cos","%tan",
	"%rt","%rnd","%ran","%trn",
	"%fl","%cl","%ln"
};

static std::string OperatorNL[] = {
	/* natural language operators */
	"and","or","xor","not",
	"plus","minus","times",
	"power","divided","divide","def",
	"as","by","define","of",
	"is","mod","factorial",
	"modulus","cos","cosinus",
	"sin","sinus","tan",
	"tangente","log","logarithm",
	"square","sqrt","the","rnd",
	"random","round","root","trunc",
	"truncate","floor","ceil","ln"
	
};

uint8_t Operatorn = sizeof(Operator)/sizeof(std::string);
uint8_t OperatorNLn = sizeof(OperatorNL)/sizeof(std::string);

/* 'is' functions */

bool is_uoperator(std::string input){
	/* is unary operator? */
	static std::string u_op[] = {
		"++","--","!","%sin","%cos","%tan",
		"%log","%sin","%sqrt","!n","%rnd","%ran",
		"%trn","%fl","%cl","%ln"
	};
	uint8_t n = sizeof(u_op)/sizeof(std::string);
	for (uint8_t i=0; i<n; i++){
		if (u_op[i]==input)
			return 1;
	}
	return 0;
}

bool is_operator(std::string input){
	for (uint8_t x=0; x<Operatorn; x++){
		if (input==Operator[x])
			return 1;
	}
	return 0;
}

bool is_error(std::string input){
	if (input.find("ERROR#",0)==-1)
		return 0;
	return 1;
}

bool is_variable(std::string input){
	for (uint16_t x=0; x<variables.size(); x++){
		if (input==variables[x].name)
			return 1;
	}	
	return 0;
}

bool is_operatorNL(std::string input){
	/* is NL operator? */
	for (uint8_t i=0; i<OperatorNLn; i++){
		if (OperatorNL[i]==input)
			return 1;
	}
	return 0;
}

/* item functions */

void item_add_at(std::string element,int8_t type,uint16_t at){
	if (at>=items.size())
		return;
	items.insert(items.begin()+at+1,element_type());
	items[at+1].element=element;
	items[at+1].type=type;
	return;
}

void item_add(std::string element,int8_t type){
	unsigned d = items.size();
	items.push_back(element_type());
	items[d].element=element;
	items[d].type=type;
	return;
}

bool item_remove(uint16_t id){
	if (id>items.size())
		return 0;
	items.erase(items.begin()+id);
	return 1;
}

bool item_set(uint16_t id,std::string element,int8_t type){
	if (id>items.size())
		return 0;
	items[id].element=element;
	items[id].type=type;
	return 1;
}


std::string item_get(uint16_t id){
	if (id>items.size())
		return "";
	return items[id].element;
}

int8_t item_get_t(uint16_t id){
	if (id>items.size())
		return -1;
	return items[id].type;
}

bool item_swap(uint16_t id,uint16_t id2){
	if (id>items.size() or id2>items.size())
		return 0;
	std::string melement = item_get(id);
	int8_t mid = item_get_t(id);
	item_set(id,item_get(id2),item_get_t(id2));
	item_set(id2,melement,mid);
	return 1;
}
/* variable functions */

std::string variable_get(uint16_t id){
	return variables[id].variable;
}

int variable_id(std::string name){
	for (uint16_t x=0; x<variables.size(); x++){
		if (name==variables[x].name)
			return x;
	}	
	return -1;
}

void variable_add(std::string name,std::string variable){
	if (is_variable(name)){
		variables[variable_id(name)].variable = variable;
		return;
	}
	unsigned d = variables.size();
	variables.push_back(variable_type());
	variables[d].name = name;
	variables[d].variable = variable;
	return;
}
/* check functions */

/*
  check if there's such a thing inside the input 
*/

bool check_operators(std::string input){
	for (uint8_t i=0; i<Operatorn; i++){
		if (input.find(Operator[i],0)!=-1)
			return 1;
	}
	return 0;
}

bool check_numbers(std::string input){
	for (unsigned i=0; i<input.length(); i++){
		if (is_number(str_at(input,i)))
			return 1;
	}
	return 0;
}

bool check_variables(std::string input){
	for (unsigned i=0; i<Operatorn; i++){
		if (input.find(Operator[i],0)!=-1)
			return 1;
	}
	return 0;
}

/* get functions */
std::string get_error(std::string input){
	/* get the error message from an error return type */
	if (input.find("ERROR#")!=-1)
		return substring(input,input.find("#",0)+1,input.length()-1);
	return input;
}

int get_operator_pos(std::string input, unsigned e){
	/* get the position of a symbol operator inside the input */
	for (unsigned i=e; i<input.length(); i++){
		std::string cur = str_at(input,i);
		for (uint8_t c=0; c<Operatorn; c++){
			if (str_at(Operator[c],0)==cur){
				if ( input.find(Operator[c],i)-i==0 )
					return input.find(Operator[c],i);
			}
		}
	}
	if (e<=input.length()-1)
		return input.length();
	else
		return -1;
}

std::string get_operator_wh(std::string input, unsigned e){
	/* get the operator */
	for (unsigned i=e; i<input.length(); i++){
		std::string cur = str_at(input,i);
		for (uint8_t c=0; c<Operatorn; c++){
			if (str_at(Operator[c],0)==cur){
				if ( input.find(Operator[c],i)-i==0 )
					return Operator[c];
			}
		}
	}
}

int8_t get_item_type(std::string input){
	if (is_operator(input))
		return 0;
	if (is_number(input))
		return 1;
	if (is_variable(input))
		return 2;
	return -1;	
}

void swap_variables(){
	/* swap variables name for their values */
	while (1){
		bool found = 0;
		for (uint16_t i=0; i<items.size(); i++){
			if (item_get_t(i)!=2)
				continue;
			found=1;
			std::string element = item_get(i);
			items[i].element = variable_get(variable_id(element));
			items[i].type = get_item_type(item_get(i));
		}
		if (!found)
			break;
	}
}

void NL_translate(){
	/* translates NL operators to their symbol-like equivalent */
	for (uint16_t i=0; i<items.size(); i++){

		std::string element = items[i].element;
		if (element=="square" and i<items.size()-1){
			if (item_get(i+1)=="root"){
				item_set(i,"%sqrt",0);
				item_remove(i+1);
				i=0;
				continue;
			}
		}
		
		if (element=="square"){
			item_set(i,"^",item_get_t(i));
			if (i<items.size()-1){
				item_swap(i,i+1);
				item_add_at("2",1,i+1);
				i=0;
				continue;
			}
		}
		
		if (element=="sinus" or element=="sin")
			item_set(i,"%sin",item_get_t(i));
			
		if (element=="cos" or element=="cosinus")
			item_set(i,"%cos",item_get_t(i));
			
		if (element=="log" or element=="logarithm")
			item_set(i,"%log",item_get_t(i));
			
		if (element=="tan" or element=="tangente")
			item_set(i,"%tan",item_get_t(i));
		
		if (element=="mod" or element=="modulus" )
			item_set(i,"%%",item_get_t(i));
		
		if (element=="ln")
			item_set(i,"%ln",item_get_t(i));

		if (element=="truncate" or element=="trunc")
			item_set(i,"%trn",item_get_t(i));

		if (element=="floor")
			item_set(i,"%fl",item_get_t(i));			

		if (element=="ceil")
			item_set(i,"%cl",item_get_t(i));	
			
		if (element=="round")
			item_set(i,"%ran",item_get_t(i));
			
		if (element=="random" or element=="rnd" )
			item_set(i,"%rnd",item_get_t(i));
			
		if (element=="power")
			item_set(i,"^",item_get_t(i));
			
		if (element=="divided" or element=="divide")
			item_set(i,"/",item_get_t(i));
			
		if (element=="plus")
			item_set(i,"+",item_get_t(i));

		if (element=="times")
			item_set(i,"*",item_get_t(i));

		if (element=="minus")
			item_set(i,"-",item_get_t(i));

		if (element=="and")
			item_set(i,"&",item_get_t(i));

		if (element=="not")
			item_set(i,"!n",item_get_t(i));

		if (element=="or")
			item_set(i,"||",item_get_t(i));

		if (element=="sqrt" or element=="root")
			item_set(i,"%sqrt",item_get_t(i));

		if (element=="xor")
			item_set(i,"%",item_get_t(i));


		if (element=="factorial"){
			item_set(i,"!",item_get_t(i));
			if (i<items.size()-1)
				item_swap(i,i+1);
		}
	}
}

/* arithmetic operations */
std::string addition(element_type *arg1,element_type *arg2){
	std::string arg1s = arg1->element;
	std::string arg2s = arg2->element;
	if (arg1->type!=arg2->type)
		return "ERROR#Addition of mismatching types '"
		+arg1->element+"' + '"+arg2->element+"'";
	if (is_number(arg1s)==0 or is_number(arg2s)==0)
		return "ERROR#Wrong arguments for an addition '"
		+arg1->element+"' + '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1s);
	double arg_2 = string_to_float(arg2s);
	return float_to_string( arg_1+arg_2 );
}

std::string subtraction(element_type *arg1,element_type *arg2){
	if (arg1->type!=arg2->type)
		return "ERROR#Subtraction of mismatching types '"
		+arg1->element+"' - '"+arg2->element+"'";
	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a subtraction '"
		+arg1->element+"' - '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	return float_to_string( arg_1-arg_2 );
}

std::string multiplication(element_type *arg1,element_type *arg2){
	if (arg1->type!=arg2->type)
		return "ERROR#Multiplication of mismatching types '"
		+arg1->element+"' * '"+arg2->element+"'";
	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a multiplication '"
		+arg1->element+"' * '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	return float_to_string( arg_1*arg_2 );
}

std::string division(element_type *arg1,element_type *arg2){
	if (arg1->type!=arg2->type)
		return "ERROR#Division of mismatching types '"
		+arg1->element+"' / '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a division '"
		+arg1->element+"' / '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	if (arg_2==0)
		return "ERROR#Division by zero";
	return float_to_string( arg_1/arg_2 );
}

std::string potentiation(element_type *arg1,element_type *arg2){
	if (arg1->type!=arg2->type)
		return "ERROR#Potentiation of mismatching types '"
		+arg1->element+"' ^ '"+arg2->element+"'";
	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a potentiation '"
		+arg1->element+"' ^ '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	return float_to_string( pow(arg_1,arg_2) );
}

/* return booleans as 'true' or 'false' */
std::string r_booltype(bool t){
	if (t)
		return "true";
	return "false";
}

/* logic operations */
std::string and_op(element_type *arg1,element_type *arg2){
	/* and */
	if (arg1->type!=arg2->type)
		return "ERROR#Logic operation of mismatching types '"
		+arg1->element+"' and '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a Logic operation '"
		+arg1->element+"' and '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	return r_booltype(((int)arg_1%2) and ((int)arg_2%2)) ;
}

std::string or_op(element_type *arg1,element_type *arg2){
	/* or */
	if (arg1->type!=arg2->type)
		return "ERROR#Logic operation of mismatching types '"
		+arg1->element+"' or '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a Logic operation '"
		+arg1->element+"' or '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	return r_booltype(((int)arg_1%2) or ((int)arg_2%2)) ;
}

std::string xor_op(element_type *arg1,element_type *arg2){
	/* xor */
	if (arg1->type!=arg2->type)
		return "ERROR#Logic operation of mismatching types '"
		+arg1->element+"' xor '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a Logic operation '"
		+arg1->element+"' xor '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	if (arg_1==arg_2)
		return r_booltype(0);
	return r_booltype(1);
}

std::string higher_op(element_type *arg1,element_type *arg2){
	/* > */
	if (arg1->type!=arg2->type)
		return "ERROR#Logic operation of mismatching types '"
		+arg1->element+"' > '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a Logic operation '"
		+arg1->element+"' > '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	if (arg_1>arg_2)
		return r_booltype(1);
	else
		return r_booltype(0);
}

std::string highereq_op(element_type *arg1,element_type *arg2){
	/* >= */
	if (arg1->type!=arg2->type)
		return "ERROR#Logic operation of mismatching types '"
		+arg1->element+"' >= '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a Logic operation '"
		+arg1->element+"' >= '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	if (arg_1>=arg_2)
		return r_booltype(1);
	else
		return r_booltype(0);
}

std::string lower_op(element_type *arg1,element_type *arg2){
	/* < */
	if (arg1->type!=arg2->type)
		return "ERROR#Logic operation of mismatching types '"
		+arg1->element+"' < '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a Logic operation '"
		+arg1->element+"' < '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	if (arg_1<arg_2)
		return r_booltype(1);
	else
		return r_booltype(0);
}

std::string lowereq_op(element_type *arg1,element_type *arg2){
	/* <= */
	if (arg1->type!=arg2->type)
		return "ERROR#Logic operation of mismatching types '"
		+arg1->element+"' <= '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a Logic operation '"
		+arg1->element+"' <= '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	if (arg_1<=arg_2)
		return r_booltype(1);
	else
		return r_booltype(0);
}

std::string eq_op(element_type *arg1,element_type *arg2){
	/* == */
	if (arg1->type!=arg2->type)
		return "ERROR#Logic operation of mismatching types '"
		+arg1->element+"' == '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a Logic operation '"
		+arg1->element+"' == '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	if (arg_1==arg_2)
		return r_booltype(1);
	else
		return r_booltype(0);
}

std::string neq_op(element_type *arg1,element_type *arg2){
	/* == */
	if (arg1->type!=arg2->type)
		return "ERROR#Logic operation of mismatching types '"
		+arg1->element+"' != '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a Logic operation '"
		+arg1->element+"' != '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	if (arg_1!=arg_2)
		return r_booltype(1);
	else
		return r_booltype(0);
}

std::string mod_op(element_type *arg1,element_type *arg2){
	/* modulus */
	if (arg1->type!=arg2->type)
		return "ERROR#Modulus of mismatching types '"
		+arg1->element+"' mod '"+arg2->element+"'";

	if (is_number(arg1->element)==0 or is_number(arg2->element)==0)
		return "ERROR#Wrong arguments for a Modulus '"
		+arg1->element+"' mod '"+arg2->element+"'";
	double arg_1 = string_to_float(arg1->element);
	double arg_2 = string_to_float(arg2->element);
	if (arg_2==0)
		return "ERROR#Division by zero";
	return int_to_string( (int)arg_1%(int)arg_2 );
}

void get_elements(std::string entry){
	typeList fragments;
	split_words(entry," ",&fragments);
	for (unsigned i=0; i<fragments.size(); i++){
		std::string current = fragments[i];
		if (check_operators(current) and
		(check_numbers(current) or check_variables(current))){
			unsigned e = 0;
			while (get_operator_pos(current,e)!=-1){
				unsigned m = get_operator_pos(current,e);
				unsigned h = m;
				if (h>0)
					h--;	
				std::string item = substring(current,e,h);
				if (is_operator(item) or item.length()==0){
					//lol
				}else
				if (is_variable(item)){
					item_add(item,2);
				}else
				if (is_operatorNL(item)){
					item_add(item,0);
				}else
				if (is_number(item)){
					item_add(item,1);
				}else{
					item_add(item,-1);
				}				
				if (current.length()!=m){
					m = m + get_operator_wh(current,e).length();
					item_add(get_operator_wh(current,e),0); 
				}
				e=m;
			}
		}else
		if (is_operatorNL(current)){
			item_add(current,0);
		}else
		if (is_number(current)==1){
			item_add(current,1);
		}else
		if (is_operator(current)==1){
			item_add(current,0);
		}else
		if (is_variable(current)){
			item_add(current,2);
		}else{
			item_add(current,-1);
		}
	}
	return;
}

std::string math_engine(std::string input){
	if (input.length()==0)
		return "";
	std::string entry = remove_coincidences(input," ");
	entry = remove_coincidences(input,";");
	/* add some costants */
	variable_add("one","1");
	variable_add("two","2");
	variable_add("three","3");
	variable_add("four","4");
	variable_add("five","5");
	variable_add("six","6");
	variable_add("seven","7");
	variable_add("eight","8");
	variable_add("nine","9");
	variable_add("zero","0");
	variable_add("pi","3.14159265359");
	variable_add("e","2.71828182846");
	variable_add("true","1");
	variable_add("false","0");
	variable_add("on","1");
	variable_add("off","0");
	/* swap [] for () */
	while (entry.find("[")!=-1){
		unsigned i = entry.find("[");
		entry.erase(i,1);
		entry.insert(i,"(");
	}
	while (entry.find("]")!=-1){
		unsigned i = entry.find("]");
		entry.erase(i,1);
		entry.insert(i,")");
	}
	/* semicolon */
	if (entry.find(";",0)!=-1 and entry.length()>1){
		while (1){
			int16_t m = entry.find(";",0);
			/* remove ';' from the start */
			if (m==0){
				entry.erase(0,1);
				continue;
			}
			/* break loop if there are no more semicolons */
			if (m==-1)
				break;
			/* get statement and run it */
			std::string s = substring(entry,0,m-1);
			math_engine(s);
			/* remove the already processed part */
			entry = substring(entry,m+1,entry.length()-1);
		}
	}
	/* parenthesis */
	while (entry.find("(",0)!=-1){
		uint16_t lp_n = 0;
		uint16_t rp_n = 0;
		for (uint16_t i=0; i<entry.length(); i++){
			if (str_at(entry,i)=="(")
				lp_n++;
			if (str_at(entry,i)==")")
				rp_n++;
		}
		if (rp_n>lp_n)
			return "Syntax error: Bad parenthesis";
		if (lp_n>rp_n){
			for (unsigned i=0; i<lp_n-rp_n; i++)
				entry+=")";
		}
		int16_t fp = -1;
		int16_t lp = -1;
		for (uint16_t i=0; i<entry.length(); i++){
			if (str_at(entry,i)=="("){
				fp=i;
			}else
			if (str_at(entry,i)==")"){
				lp=i;
				break;
			}
		}
		if (fp>lp or lp-fp<1 or lp<0)
			return "Syntax error: Bad parenthesis";
		std::string buffer = substring(entry,fp+1,lp-1);
		buffer = math_engine(buffer);
		entry.erase(fp,lp-fp+1);
		if (buffer.length()>0){
			if (buffer.find("error",0)!=-1 or buffer.find("Error",0)!=-1)
				return buffer;
			entry.insert(fp," "+buffer+" ");
		}
	}
	if (entry.find("(",0)!=-1 or entry.find(")",0)!=-1){
		return "Syntax error: Bad parenthesis";
	}
	if (entry.find(";",0)!=-1){
		return "Syntax error: Bad semicolon";
	}
	/* clear */
	items.clear();
	
	/* Get elements */
	get_elements(entry);
	
	/* Return if there's nothing to process */
	if (items.size()==0)
		return "";
	/* Remove 'as', 'by', 'of' and 'the' */
	for (uint16_t i=0; i<items.size(); i++){
		std::string element = items[i].element;
		if (!(element=="as" or  element=="by" or element=="of" or element=="the")){
			continue;
		}
		item_remove(i);
		i=0;
	}
	
	/* Change 'define', 'def' and 'is' to '=' */
	for (uint16_t i=0; i<items.size(); i++){
		if (items[i].element=="is"){
			items[i].element="=";
		}
		if (items[i].element=="define" or items[i].element=="def"){
			items[i].element="=";
			item_swap(i,i+1);
		}
	}
	/* 'Assignment operation (Define variables) */
	if (items.size()>2 and item_get(1)=="="){
		bool by_reference=0;
		/* By Reference */
		if (items.size()==4  and  item_get_t(2)==2 and item_get(3)=="#"){
			by_reference=1;
		}
		std::string nelement = item_get(0);
		std::string melement = item_get(2);
		if (item_get_t(0)!=-1 and !is_variable(nelement)){
			return "Error: Cannot use '"+nelement+"' for a variable name";
		}
		if (!by_reference){
			melement="";
			for (uint16_t c=2; c<items.size(); c++){
				/* Cannot define a variable that contains an '=' */
				if (item_get(c)=="="){
					return "Error: Cannot define a variable that contains"+
					std::string(" an assignment operator '='");
				}
				melement+=item_get(c);
				if (c<items.size()-1){
					melement+=" ";
				}
			}
			melement = math_engine(melement);
			if (melement.find("Error: ",0)!=-1 or melement.find("error: ",0)!=-1)
				return melement;
		}else{
			if (nelement==nelement and item_get_t(2)==-1){
				return "Error: Cannot define a variable with an undefined value";
			}
		}
		variable_add(nelement,melement);
		return "";
	}
	
	/* Translater Natural Language operators */
	NL_translate();

	
	/* Swap variables for their values */
	swap_variables();
	
	/* solve signs */
	for (uint16_t i=0; i<items.size(); i++){
		std::string current = item_get(i);
		if ( !((current=="-") and i<items.size()-1) ){
			continue;
		}
		bool before=0;
		if (i!=0)
			before = ( item_get_t(i-1)==0 ? 1 : 0 );
		if (item_get_t(i+1)==1 and ( i==0 or before==1 ) ){
			item_set(i+1,"-"+item_get(i+1),item_get_t(i+1));
			item_remove(i);
		}	
	}
	
	/* Undefinitions or unexisting items or bad assignment operator */
	for (uint16_t i=0; i<items.size(); i++){
		if (item_get_t(i)==-1){
			return "Error: '"+item_get(i)+"' is undefined";
		}
		if (item_get(i)=="="){
			return "Error: Bad assignment operator";
		}
	}
	/*** Do the math! ****/
	
	std::string result_holder=item_get(0);
	/*** Unary operators ***/
	for (uint16_t i=0; i<items.size(); i++){
		swap_variables();
		if (is_uoperator(item_get(i))){
			double result;
			if (item_get_t(i+1)!=1 and item_get(i)!="!" and item_get(i)!="++"
				and item_get(i)!="--")
				return "Syntax error '"+item_get(i)+"'";
			/* Factorial */
			if (item_get(i)=="!"){
				if (i==0 or item_get_t(i-1)!=1)
					return "Syntax error '"+item_get(i)+"'";
				result = factorial(string_to_int(item_get(i-1)));
				item_set(i-1,int_to_string(result),1);
			}
			/* Increment */
			if (item_get(i)=="++"){
				if (items.size()==1)
					return "1";
				if (i==0 or item_get_t(i-1)!=1){
					result = string_to_float(item_get(i+1))+1;
					item_set(i+1,float_to_string(result),1);
				}else{
					result = string_to_float(item_get(i-1))+1;
					item_set(i-1,int_to_string(result),1);			
				}
			}
			/* Decrement */
			if (item_get(i)=="--"){
				if (items.size()==1)
					return "-1";
				if (i==0 or item_get_t(i-1)!=1){
					result = string_to_float(item_get(i+1))-1;
					item_set(i+1,float_to_string(result),1);
				}else{
					result = string_to_float(item_get(i-1))-1;
					item_set(i-1,int_to_string(result),1);			
				}
			}
			/* Round */
			if (item_get(i)=="%ran"){
				result = round_int(string_to_float(item_get(i+1)));
				item_set(i+1,int_to_string(result),1);		
			}
			/* Random */
			if (item_get(i)=="%rnd"){
				result = random_int(string_to_float(item_get(i+1)),0);
				item_set(i+1,int_to_string(result),1);		
			}
			/* Sinus */
			if (item_get(i)=="%sin"){
				result = sin(string_to_float(item_get(i+1))*PI/180);
				item_set(i+1,float_to_string(result),1);
			}
			/* Cosinus */
			if (item_get(i)=="%cos"){
				result = cos(string_to_float(item_get(i+1))*PI/180);
				item_set(i+1,float_to_string(result),1);
			}
			/* Tangente */
			if (item_get(i)=="%tan"){
				result = tan(string_to_float(item_get(i+1))*PI/180);
				item_set(i+1,float_to_string(result),1);
			}
			/* Ceil */
			if (item_get(i)=="%cl"){
				result = ceil_int(string_to_float(item_get(i+1)));
				item_set(i+1,float_to_string(result),1);
			}
			/* Floor and truncate */
			if (item_get(i)=="%fl" or item_get(i)=="%trn"){
				result = floor_int(string_to_float(item_get(i+1)));
				item_set(i+1,float_to_string(result),1);
			}
			/* Logarithm Natural */
			if (item_get(i)=="%ln"){
				result = log(string_to_float(item_get(i+1)));
				item_set(i+1,float_to_string(result),1);
			}
			/* Logarithm */
			if (item_get(i)=="%log"){
				result = log10(string_to_float(item_get(i+1)));
				item_set(i+1,float_to_string(result),1);
			}
			/* Square root */
			if (item_get(i)=="%sqrt"){
				result = sqrt(string_to_float(item_get(i+1)));
				item_set(i+1,float_to_string(result),1);
			}
			/* Logical not */
			if (item_get(i)=="!n"){
				std::string rresult;
				rresult = r_booltype(!(string_to_int(item_get(i+1))%2));
				item_set(i+1,rresult,2);
				result_holder = rresult;
				item_remove(i);
			}else{
				item_remove(i);
				result_holder = float_to_string(result);
			}
		}
	}
	/*** Binary operators ***/
	
	/* PEMDAS */
	
	/*
		0 -> Potentiation
		1 -> Division, multiplication and modulus
		2 -> Addition and subtration
		3 -> Less, less than, more, and more than
		4 -> Equal, not equal
		5 -> XOR
		6 -> Logical and
		7 -> Logical or
		
		8 in total
		
	*/
	for (unsigned int o=0; o<8; o++){
		for (uint16_t i=0; i<items.size(); i++){
			swap_variables();
			if (items[i].type==0 and (i==items.size()-1 or i==0)){
				std::string error = "Syntax error at '";
				if (i>0)
					error+=items[i-1].element;
				error+=items[i].element;
				if (i<items.size()-1)
					error+=items[i+1].element;
				error+="'";
				return error;
			}
			if (!(item_get_t(i)==0 and i>0 and i<items.size()-1)){
				continue;
			}
			std::string current = item_get(i);
			element_type &Arg1 = items[i-1];
			element_type &Arg2 = items[i+1];
			std::string result="";
			bool x = 1;
			/* Potentiation */
			if (current=="^" and o==0){
				result = potentiation(&Arg1,&Arg2);
			}else
			/* Modulus */
			if (current=="%%" and o==1){
				result = mod_op(&Arg1,&Arg2);
			}else
			/* Multiplication */
			if (current=="*" and o==1){
				result = multiplication(&Arg1,&Arg2);
			}else
			/* Division */
			if (current=="/" and o==1){
				result = division(&Arg1,&Arg2);
			}else
			/* Addition */
			if (current=="+" and o==2){
				result = addition(&Arg1,&Arg2);
			}else
			/* Subtraction */
			if (current=="-" and o==2){
				result = subtraction(&Arg1,&Arg2);
			}else
			/* Logical higher than */
			if (current==">" and o==3){
				result = higher_op(&Arg1,&Arg2);
			}else
			/* Logical lower than or equal */
			if (current=="<=" and o==3){
				result = lowereq_op(&Arg1,&Arg2);
			}else
			/* Logical higher than or equal */
			if (current==">=" and o==3){
				result = highereq_op(&Arg1,&Arg2);
			}else
			/* Logical lower thna */
			if (current=="<" and o==3){
				result = lower_op(&Arg1,&Arg2);
			}else
			/* Logical equal */
			if (current=="==" and o==4){
				result = eq_op(&Arg1,&Arg2);
			}else
			/* Logical not equal */
			if (current=="!=" and o==4){
				result = neq_op(&Arg1,&Arg2);
			}else
			/* Logical xor */
			if (current=="%" and o==5){
				result = xor_op(&Arg1,&Arg2);
			}else
			/* Logical and */
			if (current=="&" and o==6){
				result = and_op(&Arg1,&Arg2);
			}else
			/* Logical or */
			if (current=="||" and o==7){
				result = or_op(&Arg1,&Arg2);
			}else{
				x=0;
			}
			/* Return error if there's any */
			if (is_error(result))
				return "Error: "+get_error(result);
			if (!x){
				continue;
			}
			Arg2.element = result;
			std::string n = result;
			Arg2.type = get_item_type(result);
			items.erase(items.begin()+i-1,items.begin()+i+1);
			for (uint16_t h=0; h<items.size(); h++){
				if (n==items[h].element){
					i--;
					break;
				}
			}
			result_holder = result;
		}	
	}
	return result_holder;
}
