%{
#include "minic.tab.h"
#include "string.h"
#include "def.h"

char parse(char *str);
void yyerror(char *msg);
int yycolumn=1;
extern string error_record;
extern int error_flag;
#define YY_USER_ACTION yylloc.first_line=yylloc.last_line=yylineno; \
yylloc.first_column=yycolumn;	yylloc.last_column=yycolumn+yyleng-1; yycolumn+=yyleng;

typedef union 
{
	int type_int;
	float type_float;
	char type_char;
	char type_id[32];
	struct node *ptr;
} YYLVAL;
#define YYSTYPE YYLVAL
//void comment();
%}
%option yylineno

id   [A-Za-z][A-Za-z0-9]*  
int    [0-9]+
float  ([0-9]*\.[0-9]+)|([0-9]+\.)
char [A-Z]|[a-z]|"!"|"@"|"#"|"$"|"%"|"^"|"&"|"*"|"("|")"|"_"|"-"|"+"|"="|"\\"|"{"|"}"|"["|"]"|":"|";"|"'"|"<"|">"|","|"."|"?"|"/"|"~"|"`"


%s COMMENT
%s COMMENT2

%%
"//"		{BEGIN COMMENT;}
"/*"		{BEGIN COMMENT2;}
"*/"		{BEGIN INITIAL;}
{int}        {yylval.type_int=atoi(yytext);return INT;}
{float}      {yylval.type_float=atof(yytext); return FLOAT;}
"'"{char}"'" {yylval.type_char=yytext[1];  return CHAR;}
"int"        {strcpy(yylval.type_id, yytext);return TYPE;}
"float"      {strcpy(yylval.type_id, yytext);return TYPE;}
"char"       {strcpy(yylval.type_id, yytext);return TYPE;}
"void"       {strcpy(yylval.type_id, yytext);return TYPE;}

"return"     {return RETURN;}
"if"         {return IF;}
"else"       {return ELSE;}
"while"      {return WHILE;}
"for"        {return FOR;}
"continue"	 {return CONTINUE;}
"break"	  	 {return BREAK;}
"struct" 	 {return STRUCT;}

{id}       {strcpy(yylval.type_id,  yytext);  return ID;/*由于关键字的形式也符合表示符的规则，所以把关键字的处理全部放在标识符的前面，优先识别*/}
";"			{return SEMI;}
","			{return COMMA;}
">"|"<"|">="|"<="|"=="|"!=" {strcpy(yylval.type_id, yytext);return RELOP;}
"="			{return ASSIGNOP;}
"+"			{return PLUS;}
"+="        {return PLUSASS;}
"++"        {return INCREMENT;}
"-"			{return MINUS;}
"-="        {return MINUSASS;}
"--"		{return DECREMENT; }
"*"			{return STAR;}
"/"			{return DIV;}
"&&"         	{return AND;}
"||"         	{return OR;}
"!"			{return NOT;}
"("			{return LP;}
")"			{return RP;}
"{"			{return LC;}
"}"			{return RC;}
"["			{return LM;}
"]"			{return RM;}
"//"[^\n]*\n  ;


[\n]        {yycolumn=1;}   
[ \r\t]     {}   
"."			{return DOT;}
%%
int yywrap()
{
	return 1;
}

// void comment()
// {
//     char c, c1;
// loop:
//     while ((c = input()) != '*' && c != 0)
//         putchar(c);

//     if ((c1 = input()) != '/' && c != 0)
//     {
//         unput(c1);
//         goto loop;
//     }
// }
// void yyerror(char *msg){
// 	error_flag = 1;
// 	error_record += msg;
// 	//strcat(error_record, msg);
// 	// strcat（error_record， msg );
// 	// strcat（error_record，"\n"）;
// }