%error-verbose
%locations
%{
#include "stdio.h"
#include "math.h"
#include "string.h"
#include "def.h"
int yyparse(void);
int yylex(void);
extern int yylineno;
extern char *yytext;
extern FILE *yyin;
extern string error_record;
extern int error_flag;
void yyerror(const char* fmt, ...);
void display(struct node *,int);

struct symboltable symbolTable;

struct symbol_scope_begin symbol_scope_TX;

%}

%union {
    int    type_int;
    float  type_float;
    char   type_char;
    char   type_id[32];
    struct node *ptr;
};

//  %type 定义非终结符的语义值类型
%type  <ptr> program ExtDefList ExtDef  Specifier ExtDecList 
%type  <ptr> FuncDec CompSt VarList VarDec ParamDec Stmt StmList DefList Def DecList Dec Exp Args 
%type  <ptr> StructDec ForCondition StructDefList StructDef StructList List_INT
//% token 定义终结符的语义值类型
%token <type_int> INT              //指定INT的语义值是type_int，有词法分析得到的数值
%token <type_id> ID RELOP TYPE  //指定ID,RELOP 的语义值是type_id，有词法分析得到的标识符字符串
%token <type_float> FLOAT         //指定ID的语义值是type_id，有词法分析得到的标识符字符串
%token <type_char>  CHAR
%token <type_void>  VOID

%token LP RP LC RC SEMI COMMA LM RM//用bison对该文件编译时，带参数-d，生成的exp.tab.h中给这些单词进行编码，可在lex.l中包含parser.tab.h使用这些单词种类码
%token PLUS MINUS PLUSASS MINUSASS  STAR DIV ASSIGNOP AND OR NOT IF ELSE WHILE RETURN

%token INCREMENT DECREMENT STRUCT FOR DOT BREAK CONTINUE

%left ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left MINUSASS PLUSASS
%left STAR DIV
%right UMINUS NOT
%left INCREMENT DECREMENT
%right PREINCREMENT PREDECREMENT

%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE

%%

program: ExtDefList    {  
                          semantic_Analysis0($1);
                        }     /*显示语法树,语义分析*/
         ; 
ExtDefList: {$$=NULL;}
          | ExtDef ExtDefList {$$=mknode(EXT_DEF_LIST,$1,$2,NULL,yylineno);}   //每一个EXTDEFLIST的结点，其第1棵子树对应一个外部变量声明或函数
          ;  
ExtDef:   Specifier ExtDecList SEMI   {$$=mknode(EXT_VAR_DEF,$1,$2,NULL,yylineno);}   //该结点对应一个外部变量声明
         | Specifier FuncDec CompSt    {$$=mknode(FUNC_DEF,$1,$2,$3,yylineno);}         //该结点对应一个函数定义
         | StructDec SEMI {$$=$1;} //结构体定义
         | error SEMI   {$$=NULL; }
         ;

StructDec: STRUCT ID LC StructDefList RC {$$=mknode(STRUCT_DEF,$4,NULL,NULL,yylineno); strcpy($$->type_id,$2);}
         ;
Specifier:  TYPE    {$$=mknode(TYPE,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);$$->type=!strcmp($1,"int")?INT:(!strcmp($1,"void"))?VOID:(!strcmp($1, "char")?CHAR:FLOAT);}   
          | STRUCT ID {$$=mknode(TYPE,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$2);/* 缺少类型说明，在语义分析中应该增加该项*/} // 结构体作为变量类型
        //   | TYPE LM RM {$$=mknode(TYPE,NULL,NULL,NULL,yylineno); char t[33]; strcpy(t, $1);strcpy($$->type_id,strcat(t,"数组"));
        //                 $$->type=!strcmp($1,"int")?INT:(!strcmp($1, "char")?CHAR:FLOAT);}
          ;
ExtDecList:  VarDec      {$$=$1;}       /*每一个EXT_DECLIST的结点，其第一棵子树对应一个变量名(ID类型的结点),第二棵子树对应剩下的外部变量名*/
           | VarDec COMMA ExtDecList {$$=mknode(EXT_DEC_LIST,$1,$3,NULL,yylineno);}
           
           ;  
VarDec:  ID   {$$=mknode(ID,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}   //ID结点，标识符符号串存放结点的type_id
        | VarDec LM List_INT RM        {printf("get into array vardec");$$=mknode(Array,$1,$3,NULL,yylineno);} //数组
        ;
         ;
List_INT: INT {$$=mknode(INT,NULL,NULL,NULL,yylineno);$$->type_int=$1;$$->type=INT;}
        ;

FuncDec: ID LP VarList RP   {$$=mknode(FUNC_DEC,$3,NULL,NULL,yylineno);strcpy($$->type_id,$1);}//函数名存放在$$->type_id
        |ID LP  RP   {$$=mknode(FUNC_DEC,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}//函数名存放在$$->type_id
        ;  
VarList: ParamDec  {$$=mknode(PARAM_LIST,$1,NULL,NULL,yylineno);}
        | ParamDec COMMA  VarList  {$$=mknode(PARAM_LIST,$1,$3,NULL,yylineno);}
        ;
ParamDec: Specifier VarDec         {$$=mknode(PARAM_DEC,$1,$2,NULL,yylineno);}
         ;

CompSt: LC DefList StmList RC    {$$=mknode(COMP_STM,$2,$3,NULL,yylineno);}
       ;
StmList: {$$=NULL; }  
        | Stmt StmList  {$$=mknode(STM_LIST,$1,$2,NULL,yylineno);}
        ;
Stmt:   Exp SEMI    {$$=mknode(EXP_STMT,$1,NULL,NULL,yylineno);}
      | CompSt      {$$=$1;}      //复合语句结点直接最为语句结点，不再生成新的结点
      | RETURN Exp SEMI   {$$=mknode(RETURN,$2,NULL,NULL,yylineno);}
      | IF LP Exp RP Stmt %prec LOWER_THEN_ELSE   {$$=mknode(IF_THEN,$3,$5,NULL,yylineno);}
      | IF LP Exp RP Stmt ELSE Stmt   {$$=mknode(IF_THEN_ELSE,$3,$5,$7,yylineno);}
      | WHILE LP Exp RP Stmt {$$=mknode(WHILE,$3,$5,NULL,yylineno);}
      | FOR LP ForCondition RP Stmt {$$=mknode(FOR,$3,$5,NULL,yylineno);}
      ;
  
DefList: {$$=NULL; }
        | Def DefList {$$=mknode(DEF_LIST,$1,$2,NULL,yylineno);}
        ;
StructDefList: {$$=NULL;}
        | StructDef StructDefList {$$=mknode(DEF_LIST, $1,$2,NULL,yylineno);}
        ;

Def:    Specifier DecList SEMI {$$=mknode(VAR_DEF,$1,$2,NULL,yylineno);}
        ;
StructDef: Specifier StructList SEMI {$$=mknode(VAR_DEF,$1,$2,NULL,yylineno);}

DecList: Dec  {$$=mknode(DEC_LIST,$1,NULL,NULL,yylineno);}
       | Dec COMMA DecList  {$$=mknode(DEC_LIST,$1,$3,NULL,yylineno);}
       ;
StructList: VarDec {$$=mknode(DEC_LIST,$1,NULL,NULL,yylineno);}
        | VarDec COMMA StructList {$$=mknode(DEC_LIST,$1,$3,NULL,yylineno);}
Dec:     VarDec  {$$=$1;}
       | VarDec ASSIGNOP Exp {$$=mknode(ASSIGNOP,$1,$3,NULL,yylineno);strcpy($$->type_id,"ASSIGNOP");}
       ;

Exp:    Exp ASSIGNOP Exp {$$=mknode(ASSIGNOP,$1,$3,NULL,yylineno);strcpy($$->type_id,"ASSIGNOP");}//$$结点type_id空置未用，正好存放运算符
      | Exp AND Exp   {$$=mknode(AND,$1,$3,NULL,yylineno);strcpy($$->type_id,"AND");}
      | Exp OR Exp    {$$=mknode(OR,$1,$3,NULL,yylineno);strcpy($$->type_id,"OR");}
      | Exp RELOP Exp {$$=mknode(RELOP,$1,$3,NULL,yylineno);strcpy($$->type_id,$2);}  //词法分析关系运算符号自身值保存在$2中
      | Exp PLUS Exp  {$$=mknode(PLUS,$1,$3,NULL,yylineno);strcpy($$->type_id,"PLUS");}
      | Exp MINUS Exp {$$=mknode(MINUS,$1,$3,NULL,yylineno);strcpy($$->type_id,"MINUS");}
      | Exp STAR Exp  {$$=mknode(STAR,$1,$3,NULL,yylineno);strcpy($$->type_id,"STAR");}
      | Exp DIV Exp   {$$=mknode(DIV,$1,$3,NULL,yylineno);strcpy($$->type_id,"DIV");}
      | LP Exp RP     {$$=$2;}
      | MINUS Exp %prec UMINUS   {$$=mknode(UMINUS,$2,NULL,NULL,yylineno);strcpy($$->type_id,"UMINUS");}
      | NOT Exp       {$$=mknode(NOT,$2,NULL,NULL,yylineno);strcpy($$->type_id,"NOT");}
      | ID LP Args RP {$$=mknode(FUNC_CALL,$3,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
      | ID LP RP      {$$=mknode(FUNC_CALL,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
      | ID            {$$=mknode(ID,NULL,NULL,NULL,yylineno);strcpy($$->type_id,$1);}
      | INT           {$$=mknode(INT,NULL,NULL,NULL,yylineno);$$->type_int=$1;$$->type=INT;}
      | FLOAT         {$$=mknode(FLOAT,NULL,NULL,NULL,yylineno);$$->type_float=$1;$$->type=FLOAT;}
      | CHAR          {$$=mknode(CHAR,NULL,NULL,NULL,yylineno); $$->type_char=$1;$$->type=CHAR;}
      | Exp DECREMENT  {$$=mknode(DECREMENT,$1,NULL,NULL, yylineno); strcpy($$->type_id,(char*)$1);printf("in minic.y: INCREMENT\n");}
      | Exp INCREMENT  {$$=mknode(INCREMENT,$1,NULL,NULL,yylineno);strcpy($$->type_id,(char*)$1);printf("in minic.y: DECREMENT\n");}
      | Exp MINUSASS Exp {$$=mknode(MINUSASS,$1,$3,NULL,yylineno);strcpy($$->type_id,"MINUSASS");} //复合减
      | Exp PLUSASS Exp  {$$=mknode(PLUSASS,$1,$3,NULL,yylineno);strcpy($$->type_id,"PLUSASS");} //复合加
      | INCREMENT Exp %prec PREINCREMENT {$$=mknode(PREINCREMENT,$2,NULL,NULL,yylineno);strcpy($$->type_id,(char*)$2);printf("in minic.y: PREINCREMENT\n");}
      | DECREMENT Exp %prec PREDECREMENT {$$=mknode(PREDECREMENT,$2,NULL,NULL, yylineno); strcpy($$->type_id,(char*)$2);printf("in minic.y: PREDECREMENT\n");}
      // 结构体访问
      | ID DOT ID     {struct node* t1=mknode(ID,NULL,NULL,NULL,yylineno);strcpy(t1->type_id,$1);
                        struct node* t2=mknode(ID, NULL,NULL,NULL,yylineno);strcpy(t2->type_id,$3);
                        $$=mknode(STRUCT_VAR,t1,t2,NULL,yylineno); char t[33]; strcpy(t, $1);strcat(t, ".");strcat(t, $3);strcpy($$->type_id, t);}
    
      | ID LM  Exp RM  {$$=mknode(Array_Call,$3,NULL,NULL,yylineno);strcpy($$->type_id,$1);}//数组索引调用
      | BREAK   {$$=mknode(BREAK,NULL,NULL,NULL,yylineno);}
      | CONTINUE {$$=mknode(CONTINUE,NULL,NULL,NULL,yylineno);}
      ;
Args:    Exp COMMA Args    {$$=mknode(ARGS,$1,$3,NULL,yylineno);}
       | Exp               {$$=mknode(ARGS,$1,NULL,NULL,yylineno);}
       ;

ForCondition: Exp SEMI Exp SEMI Exp {$$=mknode(FOR_CONDITION,$1,$3,$5,yylineno); strcpy($$->type_id, "ForCondition");}


%%

int main(int argc, char *argv[]){

//   struct symbol emptySymbol;
//   int i =0;
//   while(i < MAXLENGTH){
//                 symbolTable.symbols.push_back(emptySymbol);
//                 i++;
//   }
  yyin=fopen(argv[1],"r");
  if (!yyin) return 1; 
  yylineno=1;
  yyparse();
  //printf("err: \n");
  if(error_flag)
        cout<<error_record<<endl;
  return 0;
}

#include<stdarg.h>
void yyerror(const char* fmt, ...)
{
    error_flag = 1;
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Grammar Error at Line %d Column %d: ", yylloc.first_line,yylloc.first_column);
    error_record +="Grammar Error at Line " + to_string(yylloc.first_line) + " Column "+to_string(yylloc.first_column) + " "+fmt +"\n";

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ".\n");
}   

