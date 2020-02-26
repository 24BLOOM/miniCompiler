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
{int}        {yylval.type_int=atoi(yytext);printf("%d, [int, %s]\n", yylineno, yytext); return INT;}
{float}      {yylval.type_float=atof(yytext);  printf("%d, [float, %s]\n", yylineno, yytext);return FLOAT;}
"'"{char}"'" {yylval.type_char=yytext[1];  printf("%d, [char, %s]\n", yylineno, yytext);return CHAR;}
"int"        {strcpy(yylval.type_id, yytext);printf("%d, [type, %s]\n", yylineno, yytext);return TYPE;}
"float"      {strcpy(yylval.type_id, yytext);printf("%d, [type, %s]\n", yylineno, yytext);return TYPE;}
"char"       {strcpy(yylval.type_id, yytext);printf("%d, [type, %s]\n", yylineno, yytext);return TYPE;}
"void"       {strcpy(yylval.type_id, yytext);printf("%d, [type, %s]\n", yylineno, yytext);return TYPE;}

"return"     {printf("%d, [RETURN, %s]\n", yylineno, yytext);return RETURN;}
"if"         {printf("%d, [IF, %s]\n", yylineno, yytext);return IF;}
"else"       {printf("%d, [ELSE, %s]\n", yylineno, yytext);return ELSE;}
"while"      {printf("%d, [WHILE, %s]\n", yylineno, yytext);return WHILE;}
"for"        {printf("%d, [FOR, %s]\n", yylineno, yytext);return FOR;}
"continue"	  {printf("%d, [CONTINUE, %s]\n", yylineno, yytext);return CONTINUE;}
"break"	  {printf("%d, [BREAK, %s]\n", yylineno, yytext);return BREAK;}
"struct" 	  {printf("%d, [STRUCT, %s]\n", yylineno, yytext);return STRUCT;}

{id}       {strcpy(yylval.type_id,  yytext); printf("%d, [ID, %s]\n", yylineno, yytext);  return ID;/*由于关键字的形式也符合表示符的规则，所以把关键字的处理全部放在标识符的前面，优先识别*/}
";"			{return SEMI;}
","			{return COMMA;}
">"|"<"|">="|"<="|"=="|"!=" {strcpy(yylval.type_id, yytext);printf("[RELOP, %s]\n", yytext); return RELOP;}
"="			{printf("%d, [ASSIGNOP, %s]\n", yylineno, yytext);return ASSIGNOP;}
"+"			{printf("%d, [PLUS, %s]\n", yylineno, yytext);return PLUS;}
"+="        {printf("%d, [PLUSASS, %s]\n", yylineno, yytext);return PLUSASS;}
"++"        {printf("%d, [INCREMENT, %s]\n", yylineno, yytext);return INCREMENT;}
"-"			{return MINUS;}
"-="        {printf("%d, [MINUSASS, %s]\n", yylineno, yytext);return MINUSASS;}
"--"		{printf("%d, [DECREMENT, %s]\n", yylineno, yytext);return DECREMENT; }
"*"			{printf("%d, [STAR, %s]\n", yylineno, yytext);return STAR;}
"/"			{printf("%d, [DIV, %s]\n", yylineno, yytext);return DIV;}
"&&"         	{printf("%d, [AND, %s]\n", yylineno, yytext);return AND;}
"||"         	{printf("%d, [OR, %s]\n", yylineno, yytext);return OR;}
"!"			{printf("%d, [NOT, %s]\n", yylineno, yytext);return NOT;}
"("			{printf("%d, [LP, %s]\n", yylineno, yytext);return LP;}
")"			{printf("%d, [RP, %s]\n", yylineno, yytext);return RP;}
"{"			{printf("%d, [LC, %s]\n", yylineno, yytext);return LC;}
"}"			{printf("%d, [RC, %s]\n", yylineno, yytext);return RC;}
"["			{printf("%d, [LM, %s]\n", yylineno, yytext);return LM;}
"]"			{printf("%d, [RM, %s]\n", yylineno, yytext);return RM;}
"//"[^\n]*\n  ;


[\n]        {yycolumn=1;}   
[ \r\t]     {}   
"."			{printf("%d, [ . , %s]\n", yylineno, yytext);return DOT;}
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