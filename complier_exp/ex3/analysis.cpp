#include "def.h"
// #include<string>
// #include<iostream>
// using namespace std;
extern struct symboltable symbolTable;
extern struct symbol_scope_begin symbol_scope_TX;
int hasRet = 0;
int inFunc = 0;
int inLoop = 0;
struct codenode *crementCode = NULL;
struct node *crementNode = NULL;
string error_record ="";
int error_flag = 0;

/* 实现myitoa函数的源码 */ 
char *myitoa(int num,char *str,int radix) 
{  
    /* 索引表 */ 
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 
    unsigned unum; /* 中间变量 */ 
    int i=0,j,k; 
    /* 确定unum的值 */ 
    if(radix==10&&num<0) /* 十进制负数 */ 
    { 
        unum=(unsigned)-num; 
        str[i++]='-'; 
    } 
    else unum=(unsigned)num; /* 其它情况 */ 
    /* 逆序 */ 
    do  
    { 
        str[i++]=index[unum%(unsigned)radix]; 
        unum/=radix; 
    }while(unum); 
    str[i]='\0'; 
    /* 转换 */ 
    if(str[0]=='-') k=1; /* 十进制负数 */ 
    else k=0; 
    /* 将原来的“/2”改为“/2.0”，保证当num在16~255之间，radix等于16时，也能得到正确结果 */ 
    char temp; 
    for(j=k;j<=(i-k-1)/2.0;j++) 
    { 
        temp=str[j]; 
        str[j]=str[i-j-1]; 
        str[i-j-1]=temp; 
    } 
    return str; 
}

char *strcat0(char *s1,char *s2){
    static char result[10];
    strcpy(result,s1);
    strcat(result,s2);
    return result;
}

char *newAlias() {
    static int no=1;
    char s[10];
    myitoa(no++,s,10);
    return strcat0("v",s);
}

char *newLabel() {
    static int no=1;
    char s[10];
    myitoa(no++,s,10);
    return strcat0("label",s);
}

char *newTemp(){
    static int no=1;
    char s[10];
    myitoa(no++,s,10);
    return strcat0("temp",s);
}

//生成一条TAC代码的结点组成的双向循环链表，返回头指针
struct codenode *genIR(int op,struct opn opn1,struct opn opn2,struct opn result){
    struct codenode *h=(struct codenode *)malloc(sizeof(struct codenode));
    h->op=op;
    h->opn1=opn1;
    h->opn2=opn2;
    h->result=result;
    h->next=h->prior=h;
    return h;
}

//生成一条标号语句，返回头指针
struct codenode *genLabel(char *label){
    struct codenode *h=(struct codenode *)malloc(sizeof(struct codenode));
    h->op=LABEL;
    strcpy(h->result.id,label);
    h->next=h->prior=h;
    return h;
}

//生成GOTO语句，返回头指针
struct codenode *genGoto(char *label){
    struct codenode *h=(struct codenode *)malloc(sizeof(struct codenode));
    h->op=GOTO;
    strcpy(h->result.id,label);
    h->next=h->prior=h;
    return h;
}

//合并多个中间代码的双向循环链表，首尾相连
struct codenode *merge(int num,...){
    struct codenode *h1,*h2,*p,*t1,*t2;
    va_list ap;
    va_start(ap,num);
    h1=va_arg(ap,struct codenode *);
    while (--num>0) {
        h2=va_arg(ap,struct codenode *);
        if (h1==NULL) h1=h2;
        else if (h2){
            t1=h1->prior;
            t2=h2->prior;
            t1->next=h2;
            t2->next=h1;
            h1->prior=t2;
            h2->prior=t1;
            }
        }
    va_end(ap);
    return h1;
}

//输出中间代码
void prnIR(struct codenode *head){
    char opnstr1[32],opnstr2[32],resultstr[32];
    struct codenode *h=head;
    do {
        if (h->opn1.kind==INT)
             sprintf(opnstr1,"#%d",h->opn1.const_int);
        if (h->opn1.kind==FLOAT)
             sprintf(opnstr1,"#%f",h->opn1.const_float);
        if (h->opn1.kind==CHAR)
            sprintf(opnstr1,"\'%c\'",h->opn1.const_char);
        if (h->opn1.kind==ID)
             sprintf(opnstr1,"%s",h->opn1.id);
        if (h->opn2.kind==INT)
             sprintf(opnstr2,"#%d",h->opn2.const_int);
        if (h->opn2.kind==FLOAT)
             sprintf(opnstr2,"#%f",h->opn2.const_float);
        if (h->opn2.kind==ID)
             sprintf(opnstr2,"%s",h->opn2.id);
        sprintf(resultstr,"%s",h->result.id);
        switch (h->op) {
            case ASSIGNOP:  printf("  %s := %s\n",resultstr,opnstr1);
                            break;
            case PLUS:
            case MINUS:
            case STAR:
            case DIV: printf("  %s := %s %c %s\n",resultstr,opnstr1, \
                      h->op==PLUS?'+':h->op==MINUS?'-':h->op==STAR?'*':'\\',opnstr2);
                      break;
            case UMINUS:
            case NOT:
                        printf("  %s := %c%s\n",resultstr, \
                      h->op==UMINUS?'-':'!',opnstr1);
                      break;
            case FUNCTION: printf("\nFUNCTION %s :\n",h->result.id);
                           break;
            case PARAM:    printf("  PARAM %s\n",h->result.id);
                           break;
            case LABEL:    printf("LABEL %s :\n",h->result.id);
                           break;
            case GOTO:     printf("  GOTO %s\n",h->result.id);
                           break;
            case JLE:      printf("  IF %s <= %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case JLT:      printf("  IF %s < %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case JGE:      printf("  IF %s >= %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case JGT:      printf("  IF %s > %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case EQ:       printf("  IF %s == %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case NEQ:      printf("  IF %s != %s GOTO %s\n",opnstr1,opnstr2,resultstr);
                           break;
            case ARG:      printf("  ARG %s\n",h->result.id);
                           break;
            case CALL:     printf("  %s := CALL %s\n",resultstr, opnstr1);
                           break;
            case RETURN:   if (h->result.kind)
                                printf("  RETURN %s\n",resultstr);
                           else
                                printf("  RETURN\n");
                           break;

        }
    h=h->next;
    } while (h!=head);
}
void semantic_error(int line,char *msg1,char *msg2){
    //这里可以只收集错误信息，最后一次显示
    error_flag = 1;
    error_record +="在" + to_string(line) + "行," + msg1 + " "+ msg2 +"\n";
    //printf("在%d行,%s %s\n",line,msg1,msg2);
}
void prn_symbol(){ //显示符号表
    int i=0;
    printf("%7s        %6s  %6s  %6s %4s %6s\n","变量名","别 名","层 号","类  型","标记","偏移量");
    for(i=0;i<symbolTable.index;i++) {
        char type[10];
        sprintf(type, "%s",symbolTable.symbols[i].type==INT?"int": \
                symbolTable.symbols[i].type==FLOAT?"float": \
                symbolTable.symbols[i].type==CHAR?"char":"struct");
        if (symbolTable.symbols[i].isArray) strcat(type, "[]");
        printf("%12s %6s %6d  %6s %4c %6d\n",symbolTable.symbols[i].name,\
                symbolTable.symbols[i].alias,symbolTable.symbols[i].level, type, \
                symbolTable.symbols[i].flag,symbolTable.symbols[i].offset);
    }
}

int searchSymbolTable(char *name) {
    int i;
    for(i=symbolTable.index-1;i>=0;i--)
        if (!strcmp(symbolTable.symbols[i].name, name))  return i;
    return -1;
}

int fillSymbolTable(char *name,char *alias,int level,int type,char flag,int offset, char isArray) {
    //首先根据name查符号表，不能重复定义 重复定义返回-1
    int i;
    /*符号查重，考虑外部变量声明前有函数定义，
    其形参名还在符号表中，这时的外部变量与前函数的形参重名是允许的*/
    for(i=symbolTable.index-1; i>=0 && (symbolTable.symbols[i].level==level||level==0); i--) {
        if (level==0 && symbolTable.symbols[i].level==1) continue;  //外部变量和形参不必比较重名
        if (!strcmp(symbolTable.symbols[i].name, name))  return -1;
    }
    //填写符号表内容
    strcpy(symbolTable.symbols[symbolTable.index].name,name);
    strcpy(symbolTable.symbols[symbolTable.index].alias,alias);
    symbolTable.symbols[symbolTable.index].level=level;
    symbolTable.symbols[symbolTable.index].type=type;
    symbolTable.symbols[symbolTable.index].flag=flag;
    symbolTable.symbols[symbolTable.index].offset=offset;
    symbolTable.symbols[symbolTable.index].isArray=isArray;
    return symbolTable.index++; //返回的是符号在符号表中的位置序号，中间代码生成时可用序号取到符号别名
}

//填写临时变量到符号表，返回临时变量在符号表中的位置
int fill_Temp(char *name,int level,int type,char flag,int offset) {
    strcpy(symbolTable.symbols[symbolTable.index].name,"");
    strcpy(symbolTable.symbols[symbolTable.index].alias,name);
    symbolTable.symbols[symbolTable.index].level=level;
    symbolTable.symbols[symbolTable.index].type=type;
    symbolTable.symbols[symbolTable.index].flag=flag;
    symbolTable.symbols[symbolTable.index].offset=offset;
    return symbolTable.index++; //返回的是临时变量在符号表中的位置序号
}



int LEV=0;      //层号
int func_size;  //1个函数的活动记录大小



void ext_var_list(struct node *T){  //处理变量列表
    int rtn,num=1;
    int plusWidth = 1;
    int count = 0;
    int width =0;
    struct node *tmp;
    switch (T->kind){
        case EXT_DEC_LIST:
            T->ptr[0]->type=T->type;              //将类型属性向下传递变量结点
            T->ptr[0]->offset=T->offset;          //外部变量的偏移量向下传递
            T->ptr[0]->isArray=T->isArray;
            T->ptr[1]->type=T->type;              //将类型属性向下传递变量结点
            T->ptr[1]->offset=T->offset+T->width; //外部变量的偏移量向下传递
            T->ptr[1]->width=T->width;
            T->ptr[1]->isArray=T->isArray;
            ext_var_list(T->ptr[0]);
            ext_var_list(T->ptr[1]);
            T->num=T->ptr[1]->num+1;
        break;
        case Array:
            //printf("array dec");
            if (T->ptr[1]->type_int <= 0)
            {
                semantic_error(T->pos, "", "数组长度不能小于等于0");
            }
            rtn=fillSymbolTable(T->ptr[0]->type_id,newAlias(),LEV,T->type,'V',T->offset+T->width,T->isArray);
            //printf("%s\n", symbolTable.symbols[rtn].alias);
            if (rtn==-1)
                semantic_error(T->pos,T->type_id, "变量重复定义");
            else T->place=rtn;
            tmp = T;
            plusWidth = 1;
            count = 0;
            width = T->width;
            //多维数组的处理
            while(tmp->ptr[0] != NULL)
            {
                plusWidth *= tmp->ptr[1]->type_int;
                symbolTable.symbols[rtn].listLength[count] = tmp->ptr[1]->type_int;
                tmp = tmp->ptr[0];
            }
            strcpy(symbolTable.symbols[rtn].name,tmp->type_id);
            T->width+=width*plusWidth;
            symbolTable.symbols[rtn].isArray = 1; 
        break;       
        case ID:
            rtn=fillSymbolTable(T->type_id,newAlias(),LEV,T->type,'V',T->offset,T->isArray);  //最后一个变量名
            if (rtn==-1)
                semantic_error(T->pos,T->type_id, "变量重复定义");
            else T->place=rtn;
            T->num=1;
        break;
    }
}

int  match_param(int i,struct node *T,int pos){
    //printf("get into match param");
    int j,num=symbolTable.symbols[i].paramnum;
    int type1,type2;
    //cout<<"num :"<<num<<endl;
    if(T==NULL && num>0){
        //printf("too less\n");
        semantic_error(pos,"", "函数调用参数太少");
        //printf("match finish");
        return 0;
    }
    for (j=1;j<=num;j++) {
        if (!T){
            if (num==0 && T==NULL) 
                return 1;
            semantic_error(pos,"", "函数调用参数太少");
            return 0;
        }
            type1=symbolTable.symbols[i+j].type;  //形参类型
            type2=T->ptr[0]->type;
        if (type1!=type2){
            semantic_error(pos,"", "参数类型不匹配");
            return 0;
        }
        T=T->ptr[1];
    }
    if (T){ //num个参数已经匹配完，还有实参表达式
        //printf("too much para");
        semantic_error(pos,"", "函数调用参数太多");
        return 0;
        }
    return 1;
    }

void boolExp(struct node *T){  //布尔表达式，参考文献[2]p84的思想
  struct opn opn1,opn2,result;
  int op;
  int rtn;
  if (T)
	{
	switch (T->kind) {
        case INT:  if (T->type_int!=0)
                        T->code=genGoto(T->Etrue);
                   else T->code=genGoto(T->Efalse);
                   T->width=0;
                   break;
        case FLOAT:  if (T->type_float!=0.0)
                        T->code=genGoto(T->Etrue);
                   else T->code=genGoto(T->Efalse);
                   T->width=0;
                   break;
        case ID:    //查符号表，获得符号表中的位置，类型送type
                   rtn=searchSymbolTable(T->type_id);
                   if (rtn==-1)
                        semantic_error(T->pos,T->type_id, "变量未定义");
                   if (symbolTable.symbols[rtn].flag=='F')
                        semantic_error(T->pos,T->type_id, "是函数名，类型不匹配");
                   else {
                        opn1.kind=ID; strcpy(opn1.id,symbolTable.symbols[rtn].alias);
                        opn1.offset=symbolTable.symbols[rtn].offset;
                        opn2.kind=INT; opn2.const_int=0;
                        result.kind=ID; strcpy(result.id,T->Etrue);
                        T->code=genIR(NEQ,opn1,opn2,result);
                        T->code=merge(2,T->code,genGoto(T->Efalse));
                        }
                   T->width=0;
                   break;
        case RELOP: //处理关系运算表达式,2个操作数都按基本表达式处理
                    T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                    Exp(T->ptr[0]);
                    T->width=T->ptr[0]->width;
                    Exp(T->ptr[1]);
                    if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                    opn1.kind=ID; strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                    opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                    opn2.kind=ID; strcpy(opn2.id,symbolTable.symbols[T->ptr[1]->place].alias);
                    opn2.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                    result.kind=ID; strcpy(result.id,T->Etrue);
                    if (strcmp(T->type_id,"<")==0)
                            op=JLT;
                    else if (strcmp(T->type_id,"<=")==0)
                            op=JLE;
                    else if (strcmp(T->type_id,">")==0)
                            op=JGT;
                    else if (strcmp(T->type_id,">=")==0)
                            op=JGE;
                    else if (strcmp(T->type_id,"==")==0)
                            op=EQ;
                    else if (strcmp(T->type_id,"!=")==0)
                            op=NEQ;
                    T->code=genIR(op,opn1,opn2,result);
                    T->code=merge(4,T->ptr[0]->code,T->ptr[1]->code,T->code,genGoto(T->Efalse));
                    break;
        case AND:
        case OR:
                    if (T->kind==AND) {
                        strcpy(T->ptr[0]->Etrue,newLabel());
                        strcpy(T->ptr[0]->Efalse,T->Efalse);
                        }
                    else {
                        strcpy(T->ptr[0]->Etrue,T->Etrue);
                        strcpy(T->ptr[0]->Efalse,newLabel());
                        }
                    strcpy(T->ptr[1]->Etrue,T->Etrue);
                    strcpy(T->ptr[1]->Efalse,T->Efalse);
                    T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                    boolExp(T->ptr[0]);
                    T->width=T->ptr[0]->width;
                    boolExp(T->ptr[1]);
                    if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                    if (T->kind==AND)
                        T->code=merge(3,T->ptr[0]->code,genLabel(T->ptr[0]->Etrue),T->ptr[1]->code);
                    else
                        T->code=merge(3,T->ptr[0]->code,genLabel(T->ptr[0]->Efalse),T->ptr[1]->code);
                    break;
        case NOT:   strcpy(T->ptr[0]->Etrue,T->Efalse);
                    strcpy(T->ptr[0]->Efalse,T->Etrue);
                    boolExp(T->ptr[0]);
                    T->code=T->ptr[0]->code;
                    break;
        default: 
                semantic_error(T->pos,"","缺少bool表达式");
                break;
        }
	}
}


void Exp(struct node *T)
{//处理基本表达式，参考文献[2]p82的思想
  int rtn,num,width;
  struct node *T0;
  struct opn opn1,opn2,result;
  if (T)
	{
    // if (T->kind==INCREMENT) printf("T->kind == INCREMENT!\n");
	switch (T->kind) {
	case ID:    //查符号表，获得符号表中的位置，类型送type
                rtn=searchSymbolTable(T->type_id);
                if (rtn==-1)
                    semantic_error(T->pos,T->type_id, "变量未定义");
                if (symbolTable.symbols[rtn].flag=='F')
                    semantic_error(T->pos,T->type_id, "是函数名，类型不匹配");
                else {
                    if (T->isArray) {
                        // 分析第一个孩子结点
                        Exp(T->ptr[0]);
                        if (T->ptr[0]->type!=INT) {
                            semantic_error(T->pos,T->type_id, "数组索引类型不为int");
                        } else if (T->ptr[0]->kind==UMINUS) { // 如果是int类型，而且是uminus类型
                            struct node *num = T->ptr[0]->ptr[0];
                            if (num->kind==INT) {
                                semantic_error(T->pos,T->type_id, "数组索引值不能为负数");
                            }
                        }
                    }
                    T->place=rtn;       //结点保存变量在符号表中的位置
                    T->code=NULL;       //标识符不需要生成TAC
                    T->type=symbolTable.symbols[rtn].type;
                    T->offset=symbolTable.symbols[rtn].offset;
                    T->width=0;   //未再使用新单元
                    }
                break;
    case INT:   T->place=fill_Temp(newTemp(),LEV,T->type,'T',T->offset); //为整常量生成一个临时变量
                T->type=INT;
                opn1.kind=INT;opn1.const_int=T->type_int;
                result.kind=ID; strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.offset=symbolTable.symbols[T->place].offset;
                T->code=genIR(ASSIGNOP,opn1,opn2,result);
                T->width=4;
                break;
    case FLOAT: T->place=fill_Temp(newTemp(),LEV,T->type,'T',T->offset);   //为浮点常量生成一个临时变量
                T->type=FLOAT;
                opn1.kind=FLOAT; opn1.const_float=T->type_float;
                result.kind=ID; strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.offset=symbolTable.symbols[T->place].offset;
                T->code=genIR(ASSIGNOP,opn1,opn2,result);
                T->width=4;
                break;
    case CHAR:  T->place=fill_Temp(newTemp(),LEV,T->type,'T',T->offset);
                T->type=CHAR;
                opn1.kind=CHAR; opn1.const_char=T->type_char;
                result.kind=ID; strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.offset=symbolTable.symbols[T->place].offset;
                T->code=genIR(ASSIGNOP,opn1,opn2,result);
                T->width=4;
                break;
	case ASSIGNOP:
                if (T->ptr[0]->kind!=ID && T->ptr[0]->kind != Array_Call){
                    semantic_error(T->pos,"", "赋值语句需要左值");
                    }
                else {
                    Exp(T->ptr[0]);   //处理左值，例中仅为变量
                    T->ptr[1]->offset=T->offset;
                    Exp(T->ptr[1]);

                    
                    
                    if(T->ptr[0]->kind == Array_Call)
                    {
                        rtn = searchSymbolTable(T->ptr[0]->type_id);
                        if(!symbolTable.symbols[rtn].isArray){
                            semantic_error(T->pos,symbolTable.symbols[rtn].name,"不是一个数组");
                        }
                        else if (symbolTable.symbols[rtn].type != T->ptr[1]->type)
                        {
                                semantic_error(T->pos,"","赋值语句数组类型不一致");
                        }
                    }
                    else if (T->ptr[0]->type != T->ptr[1]->type) 
                    {
                        semantic_error(T->pos,"", "赋值时变量类型不匹配");
                    }
                    T->type=T->ptr[0]->type;
                    T->width=T->ptr[1]->width;
                    T->code=merge(2,T->ptr[0]->code,T->ptr[1]->code);
                    opn1.kind=ID;   strcpy(opn1.id,symbolTable.symbols[T->ptr[1]->place].alias);//右值一定是个变量或临时变量
                    opn1.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                    result.kind=ID; strcpy(result.id,symbolTable.symbols[T->ptr[0]->place].alias);
                    result.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                    T->code=merge(2,T->code,genIR(ASSIGNOP,opn1,opn2,result));
                    }
                break;
    
                // if (T->ptr[0]->kind!=ID && T->ptr[0]->kind != Array_Call){
                //     semantic_error(T->pos,"", "赋值语句需要左值");
                //     }
                // else {
                //     Exp(T->ptr[0]);   //处理左值，例中仅为变量
                //     T->ptr[1]->offset=T->offset;
                //     Exp(T->ptr[1]);

                //      if(T->ptr[0]->kind == Array_Call)
                //     {
                //         rtn = searchSymbolTable(T->ptr[0]->type_id);
                //         if (symbolTable.symbols[rtn].type != T->ptr[1]->type)
                //         {
                //                 semantic_error(T->pos,"","赋值语句数组类型不一致");
                //         }
                //     }
                //     else if(T->ptr[1]->kind == FUNC_CALL)
                //     {
                //         int rtn_func = searchSymbolTable(T->ptr[1]->type_id);
                //         rtn = searchSymbolTable(T->ptr[0]->type_id);
                //         if (symbolTable.symbols[rtn].type != symbolTable.symbols[rtn_func].type )
                //         {
                //             semantic_error(T->pos,"","赋值语句左右类型不一致");
                //         }
                //     }
                //     else if (T->ptr[0]->type != T->ptr[1]->type) 
                //     {
                //         semantic_error(T->pos,"", "赋值时变量类型不匹配");
                //     }
                //     T->ptr[1]->offset=T->offset;
                //     Exp(T->ptr[1]);
                //     T->type=T->ptr[0]->type;
                //     T->width=T->ptr[1]->width;
                //     T->code=merge(2,T->ptr[0]->code,T->ptr[1]->code);
                //     opn1.kind=ID;

                //     strcpy(opn1.id,symbolTable.symbols[T->ptr[1]->place].alias);//右值一定是个变量或临时变量
                //     opn1.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                //     if(T->ptr[0]->kind == Array_Call)
                //     {
                //         result.kind=Array_Call;
                //         Exp(T->ptr[0]->ptr[0]);
                //         opn2.kind = ID;
                //         strcpy(opn2.id,symbolTable.symbols[T->ptr[0]->ptr[0]->place].alias);
                //         opn2.offset=symbolTable.symbols[T->ptr[0]->ptr[0]->place].offset;
                //     }
                //     else
                //     {
                //         result.kind=ID;
                //     }
                //     strcpy(result.id,symbolTable.symbols[T->ptr[0]->place].alias);
                //     result.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                //     T->code=merge(2,genIR(ASSIGNOP,opn1,opn2,result), T->code);
                   
                //     }


                    // T->type=T->ptr[0]->type;
                    // T->width=T->ptr[1]->width;
                    // T->code=merge(2,T->ptr[0]->code,T->ptr[1]->code);
                    // opn1.kind=ID;   
                    // strcpy(opn1.id,symbolTable.symbols[T->ptr[1]->place].alias);//右值一定是个变量或临时变量
                    // opn1.offset=symbolTable.symbols[T->ptr[1]->place].offset;
                    // result.kind=ID; strcpy(result.id,symbolTable.symbols[T->ptr[0]->place].alias);
                    // result.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                    // T->code=merge(2,T->code,genIR(ASSIGNOP,opn1,opn2,result));
                    
                break;
	case AND:   //按算术表达式方式计算布尔值，未写完
	case OR:    //按算术表达式方式计算布尔值，未写完
	case RELOP: //按算术表达式方式计算布尔值，未写完
                T->type=INT;
                T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                Exp(T->ptr[0]);
                Exp(T->ptr[1]);
                break;
	case PLUS:
	case MINUS:
	case STAR:
	case DIV:   T->ptr[0]->offset=T->offset;
                Exp(T->ptr[0]);
                T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                Exp(T->ptr[1]);
                //判断T->ptr[0]，T->ptr[1]类型是否正确，可能根据运算符生成不同形式的代码，给T的type赋值
                //下面的类型属性计算，没有考虑错误处理情况
                if (T->ptr[0]->type==CHAR || T->ptr[1]->type==CHAR) {
                    semantic_error(T->pos,"", "不能计算的表达式");
                        break;
                }
                if (T->kind==DIV) { // 除零错误
                    if ((T->ptr[1]->kind==INT && T->ptr[1]->type_int==0) || 
                        (T->ptr[1]->kind==FLOAT && T->ptr[1]->type_float==0.0)) {
                        semantic_error(T->pos,"", "除零错误");
                    }
                }
                if (T->ptr[0]->type==FLOAT || T->ptr[1]->type==FLOAT)
                     T->type=FLOAT,T->width=T->ptr[0]->width+T->ptr[1]->width+4;
                else T->type=INT,T->width=T->ptr[0]->width+T->ptr[1]->width+2;
                T->place=fill_Temp(newTemp(),LEV,T->type,'T',T->offset+T->ptr[0]->width+T->ptr[1]->width);
                opn1.kind=ID; 
                strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                opn1.type=T->ptr[0]->type;
                opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;

                opn2.kind=ID; 
                strcpy(opn2.id,symbolTable.symbols[T->ptr[1]->place].alias);
                opn2.type=T->ptr[1]->type;
                opn2.offset=symbolTable.symbols[T->ptr[1]->place].offset;

                result.kind=ID; 
                strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.type=T->type;result.offset=symbolTable.symbols[T->place].offset;
                T->code=merge(3,T->ptr[0]->code,T->ptr[1]->code,genIR(T->kind,opn1,opn2,result));
                T->width=T->ptr[0]->width+T->ptr[1]->width+(T->type==INT?4:8);
                break;
	case NOT:
	case UMINUS:
                T->ptr[0]->offset=T->offset;
                Exp(T->ptr[0]);
                if (T->kind== NOT && T->ptr[0]->type!=INT)
                    semantic_error(T->pos,"","取反类型不为int");
                // 如果为char类型，那么就会出错
                else if (T->ptr[0]->type==CHAR) {
                    semantic_error(T->pos,"","符号变反的类型不能为char");
                }
                T->type=T->ptr[0]->type; // 把子结点的类型赋给父节点

                T->width=T->ptr[0]->width+T->ptr[0]->type==INT?2:4;
                // 中间代码生成
                //---------------------------------
                T->place=fill_Temp(newTemp(),LEV,T->type,'T',T->offset+T->ptr[0]->width);
                opn1.kind=ID; strcpy(opn1.id,symbolTable.symbols[T->ptr[0]->place].alias);
                opn1.type=T->ptr[0]->type;opn1.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                result.kind=ID; strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.type=T->type; result.offset=symbolTable.symbols[T->place].offset;
                T->code=merge(2,T->ptr[0]->code,genIR(T->kind,opn1,opn2,result));
                T->width=T->ptr[0]->width+(T->type==INT)?4:(T->type==CHAR?2:8); // maybe have a bug
                // printf("for debug: symbolTable.symbols[T->place].alias=%s\n", symbolTable.symbols[T->place].alias);
                //---------------------------------
                break;
    case Array_Call:
                //cout<<"array call"<<endl;
                rtn = searchSymbolTable(T->type_id);
                if (rtn == -1)
                {
                    semantic_error(T->pos, "", "数组变量未定义");
                }
                //只能检查一维
                //Exp(T->ptr[0]);
                //EXP为整形的时候检查数组访问是否越界
                // cout<<"(T->ptr[0]->kind "<<T->ptr[0]->kind<<endl;
                // cout<<" ID" <<INT <<endl;
                // cout<<"T->ptr[0]->type_int"<<T->ptr[0]->type_int<<endl;
                // cout<<"symbolTable.symbols[rtn].listLength[0]"<<symbolTable.symbols[rtn].listLength[0]<<endl;
                if(symbolTable.symbols[rtn].isArray)
                {
                    if (T->ptr[0]->kind == INT && T->ptr[0]->type_int >= symbolTable.symbols[rtn].listLength[0])
                    {
                        semantic_error(T->pos, "", "数组访问越界");
                    }
                    if (T->ptr[0]->type == CHAR || T->ptr[0]->type == FLOAT) {
                                semantic_error(T->pos,T->type_id, "数组索引类型不为int");
                    } 
                    else if (T->ptr[0]->kind==UMINUS) { // 如果是int类型，而且是uminus类型
                                struct node *num = T->ptr[0]->ptr[0];
                                if (num->kind==INT) {
                                    semantic_error(T->pos,T->type_id, "数组索引值不能为负数");
                                }
                    }


                    T->place = rtn;
                    T->code=NULL;       //标识符不需要生成TAC
                    result.kind=Array_Call; 
                    strcpy(result.id,symbolTable.symbols[T->place].alias);
                    result.offset=symbolTable.symbols[T->place].offset;
                    T->type=symbolTable.symbols[rtn].type;
                    T->offset=symbolTable.symbols[rtn].offset;
                    T->width=0;
                }   //未再使用新单元
                break;
    case FUNC_CALL: //根据T->type_id查出函数的定义，如果语言中增加了实验教材的read，write需要单独处理一下
                rtn=searchSymbolTable(T->type_id);
                if (rtn==-1){
                    semantic_error(T->pos,T->type_id, "函数未定义");
                    break;
                    }
                if (symbolTable.symbols[rtn].flag!='F'){
                    semantic_error(T->pos,T->type_id, "不是一个函数");
                     break;
                    }
                T->type=symbolTable.symbols[rtn].type;
                width=T->type==INT?4:8;   //存放函数返回值的单数字节数
                if (T->ptr[0]){
                    T->ptr[0]->offset=T->offset;
                    Exp(T->ptr[0]);       //处理所有实参表达式求值，及类型
                    T->width=T->ptr[0]->width+width; //累加上计算实参使用临时变量的单元数
                    T->code=T->ptr[0]->code;
                    }
                else {T->width=width; T->code=NULL;}
                match_param(rtn,T->ptr[0],T->pos);   //处理所有参数的匹配
                
                    //处理参数列表的中间代码
                T0=T->ptr[0];
                while (T0) {
                    result.kind=ID;  
                    strcpy(result.id,symbolTable.symbols[T0->ptr[0]->place].alias);
                    result.offset=symbolTable.symbols[T0->ptr[0]->place].offset;
                    T->code=merge(2,T->code,genIR(ARG,opn1,opn2,result));
                    T0=T0->ptr[1];
                    }
                T->place=fill_Temp(newTemp(),LEV,T->type,'T',T->offset+T->width-width);
                opn1.kind=ID;     
                strcpy(opn1.id,T->type_id);  //保存函数名
                opn1.offset=rtn;  //这里offset用以保存函数定义入口,在目标代码生成时，能获取相应信息
                result.kind=ID;   
                strcpy(result.id,symbolTable.symbols[T->place].alias);
                result.offset=symbolTable.symbols[T->place].offset;
                T->code=merge(2,T->code,genIR(CALL,opn1,opn2,result)); //生成函数调用中间代码
                //printf("match finish\n");
                break;
    case ARGS:      //此处仅处理各实参表达式的求值的代码序列，不生成ARG的实参系列
                T->ptr[0]->offset=T->offset;
                Exp(T->ptr[0]);
                T->width=T->ptr[0]->width;
                T->code=T->ptr[0]->code;
                if (T->ptr[1]) {
                    T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                    Exp(T->ptr[1]);
                    T->width+=T->ptr[1]->width;
                    T->code=merge(2,T->code,T->ptr[1]->code);
                    }
                break; 
    case BREAK: 
                if (!inLoop)
                    semantic_error(T->pos,"","break不在循环中");
                // printf("For debug: case BREAK, T->Snext=%s; T->Etrue=%s; T->Efalse=%s\n", T->Snext, T->Etrue, T->Efalse);
                T->code=merge(2,T->code,genGoto(T->Efalse));
                break;
    case CONTINUE:
                if (!inLoop)
                    semantic_error(T->pos,"","continue不在循环中");
                // printf("For debug: case CONTINUE, T->Snext=%s; T->Etrue=%s; T->Efalse=%s\n", T->Snext, T->Etrue, T->Efalse);
                T->code=merge(2,T->code,genGoto(T->Etrue));
                break;
    case INCREMENT: 
    case DECREMENT:
                // printf("for debug: crement!!\n");
                // 生成临时变量
                T->ptr[1] = mknode(INT, NULL,NULL,NULL,T->pos);
                T->ptr[1]->type_int = 1;

                crementNode = mknode(T->kind==INCREMENT?PLUS:MINUS,T->ptr[0], T->ptr[1],NULL,T->pos);
                Exp(crementNode);
                // 把生成的临时变量赋值给T
                if (T->ptr[0]->kind!=ID) {
                    semantic_error(T->pos,"","不是可赋值的左值");
                    break;
                } else {
                    rtn=searchSymbolTable(T->ptr[0]->type_id);

                    //cout<<"T->ptr[0]->kind "<<symbolTable.symbols[rtn].type<<"STRUCT "<<STRUCT<<endl;
                if(symbolTable.symbols[rtn].type != INT)

                     
                        {semantic_error(T->pos,"","不是可赋值的左值");
                        break;}
                }
                

                result.kind=ID; strcpy(result.id, symbolTable.symbols[crementNode->ptr[0]->place].alias);
                result.offset = symbolTable.symbols[crementNode->ptr[0]->place].offset;
                opn1.kind=ID; strcpy(opn1.id, symbolTable.symbols[crementNode->place].alias);
                opn1.offset = symbolTable.symbols[crementNode->place].offset;
                if (crementCode)
                    crementCode=merge(3,crementCode,crementNode->code,genIR(ASSIGNOP,opn1,opn2,result));
                else 
                    crementCode=merge(2,crementNode->code,genIR(ASSIGNOP,opn1,opn2,result));
                T->type = T->ptr[0]->type;
                T->width = T->ptr[0]->width;
                T->place = T->ptr[0]->place;
                break;
                //  printf("for debug: crement!!\n");
                // // 生成临时变量
                // T->ptr[1] = mknode(INT, NULL,NULL,NULL,T->pos);
                // T->ptr[1]->type_int = 1;

                // crementNode = mknode(T->kind==INCREMENT?PLUS:MINUS,T->ptr[0], T->ptr[1],NULL,T->pos);
                // Exp(crementNode);
                // // 把生成的临时变量赋值给T
                // if (T->ptr[0]->kind!=ID ) {
                //     semantic_error(T->pos,"","不是可赋值的左值");
                //     break;
                // }
                // else {
                //     rtn=searchSymbolTable(T->ptr[0]->type_id);

                //     cout<<"T->ptr[0]->kind "<<symbolTable.symbols[rtn].type<<"STRUCT "<<STRUCT<<endl;
                //     if(symbolTable.symbols[rtn].type != INT)

                     
                //         semantic_error(T->pos,"","不是可赋值的左值");
                //     break;
                // }
                

                // result.kind=ID; 
                // strcpy(result.id, symbolTable.symbols[crementNode->ptr[0]->place].alias);
                // result.offset = symbolTable.symbols[crementNode->ptr[0]->place].offset;
                // opn1.kind=ID; 
                // strcpy(opn1.id, symbolTable.symbols[crementNode->place].alias);
                // opn1.offset = symbolTable.symbols[crementNode->place].offset;
                // if (crementCode)
                //     crementCode=merge(3,crementCode,crementNode->code,genIR(ASSIGNOP,opn1,opn2,result));
                // else 
                //     crementCode=merge(2,crementNode->code,genIR(ASSIGNOP,opn1,opn2,result));
                // T->type = T->ptr[0]->type;
                // T->width = T->ptr[0]->width;
                // T->place = T->ptr[0]->place;
                // break;
    case PREINCREMENT:
    case PREDECREMENT:
                // printf("for debug: Precrement!!\n");
                // 生成临时变量
                T->ptr[1] = mknode(INT, NULL,NULL,NULL,T->pos);
                T->ptr[1]->type_int = 1;
                if (T->kind==PREINCREMENT) {
                    T->kind =(enum node_kind) PLUS;
                    Exp(T);
                } else {
                    T->kind =(enum node_kind) MINUS;
                    Exp(T);
                }
                // 把生成的临时变量赋值给T
                if (T->ptr[0]->kind!=ID) {
                    semantic_error(T->pos,"","不是可赋值的左值");
                    break;
                }
                result.kind=ID; strcpy(result.id, symbolTable.symbols[T->ptr[0]->place].alias);
                result.offset = symbolTable.symbols[T->ptr[0]->place].offset;
                opn1.kind=ID; strcpy(opn1.id, symbolTable.symbols[T->place].alias);
                opn1.offset = symbolTable.symbols[T->place].offset;
                T->type = T->ptr[0]->type;
                T->width = T->ptr[0]->width;
                T->place = T->ptr[0]->place;
                T->code=merge(2,T->code,genIR(ASSIGNOP,opn1,opn2,result));
                //---------------------------------
                break;
         }
      }
}



void semantic_Analysis(struct node *T)
{//对抽象语法树的先根遍历,按display的控制结构修改完成符号表管理和语义检查和TAC生成（语句部分）
  int rtn,num,width;
  struct node *T0;
  struct opn opn1,opn2,result;
  if (T)
	{
	switch (T->kind) {
	case EXT_DEF_LIST:
            if (!T->ptr[0]) break;
            T->ptr[0]->offset=T->offset;
            semantic_Analysis(T->ptr[0]);    //访问外部定义列表中的第一个
            T->code=T->ptr[0]->code;
            if (T->ptr[1]){
                T->ptr[1]->offset=T->ptr[0]->offset+T->ptr[0]->width;
                T->ptr[1]->isArray=T->ptr[0]->isArray;T->ptr[1]->isArray=T->ptr[0]->isArray;
                semantic_Analysis(T->ptr[1]); //访问该外部定义列表中的其它外部定义
                T->code=merge(2,T->code,T->ptr[1]->code);
                }
            break;
	case EXT_VAR_DEF:   //处理外部说明,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
            T->ptr[1]->type=!strcmp(T->ptr[0]->type_id,"int")?INT: 
                                !strcmp(T->ptr[0]->type_id,"float")?FLOAT:
                                !strcmp(T->ptr[0]->type_id,"char")?CHAR:
                                strstr(T->ptr[0]->type_id,"数组")?T->ptr[0]->type:STRUCT;
            if (strstr(T->ptr[0]->type_id,"数组")) T->ptr[1]->isArray = 1;
                else T->ptr[1]->isArray = 0;
            T->ptr[1]->offset=T->offset;        //这个外部变量的偏移量向下传递
            T->ptr[1]->width=T->type==INT?4:T->ptr[1]->type==INT?8:1;;  //将一个变量的宽度向下传递
            ext_var_list(T->ptr[1]);            //处理外部变量说明中的标识符序列
            T->width=(T->type==INT?4:T->ptr[1]->type==INT?8:1)* T->ptr[1]->num; //计算这个外部变量说明的宽度
            T->code=NULL;             //这里假定外部变量不支持初始化
            break;
	case FUNC_DEF:      //填写函数定义信息到符号表
            T->ptr[1]->type=!strcmp(T->ptr[0]->type_id,"int")?INT: //获取函数返回类型送到含函数名、参数的结点
                                !strcmp(T->ptr[0]->type_id,"float")?FLOAT:
                                !strcmp(T->ptr[0]->type_id,"char")?CHAR:
                                !strcmp(T->ptr[0]->type_id,"void")?VOID:
                                strstr(T->ptr[0]->type_id,"数组")?T->ptr[0]->type:STRUCT;
            if (strstr(T->ptr[0]->type_id,"数组")) T->ptr[1]->isArray = 1;
            else T->ptr[1]->isArray = 0;
            inFunc = 1; // add by walker
            T->width=0;     //函数的宽度设置为0，不会对外部变量的地址分配产生影响
            T->offset=DX;   //设置局部变量在活动记录中的偏移量初值
            semantic_Analysis(T->ptr[1]); //处理函数名和参数结点部分，这里不考虑用寄存器传递参数
            T->offset+=T->ptr[1]->width;   //用形参单元宽度修改函数局部变量的起始偏移量
            T->ptr[2]->offset=T->offset;
            strcpy(T->ptr[2]->Snext,newLabel());  //函数体语句执行结束后的位置属性
            semantic_Analysis(T->ptr[2]);         //处理函数体结点
            if (!hasRet)
                semantic_error(T->pos,"","缺少返回值");
            inFunc = 0;
            //计算活动记录大小,这里offset属性存放的是活动记录大小，不是偏移
            symbolTable.symbols[T->ptr[1]->place].offset=T->offset+T->ptr[2]->width;
            T->code=merge(3,T->ptr[1]->code,T->ptr[2]->code,genLabel(T->ptr[2]->Snext));          //函数体的代码作为函数的代码
            break;
	case FUNC_DEC:      //根据返回类型，函数名填写符号表
            rtn=fillSymbolTable(T->type_id,newAlias(),LEV,T->type,'F',0,T->isArray);//函数不在数据区中分配单元，偏移量为0
            if (rtn==-1){
                semantic_error(T->pos,T->type_id, "函数重复定义");
                break;
            }
            else T->place=rtn;
            result.kind=ID;   strcpy(result.id,T->type_id);
            result.offset=rtn;
            T->code=genIR(FUNCTION,opn1,opn2,result);     //生成中间代码：FUNCTION 函数名
            T->offset=DX;   //设置形式参数在活动记录中的偏移量初值
            if (T->ptr[0]) { //判断是否有参数
                T->ptr[0]->offset=T->offset;
                semantic_Analysis(T->ptr[0]);  //处理函数参数列表
                T->width=T->ptr[0]->width;
                symbolTable.symbols[rtn].paramnum=T->ptr[0]->num;
                T->code=merge(2,T->code,T->ptr[0]->code);  //连接函数名和参数代码序列
                }
            else symbolTable.symbols[rtn].paramnum=0,T->width=0;
            break;
	case PARAM_LIST:    //处理函数形式参数列表
            T->ptr[0]->offset=T->offset;
            semantic_Analysis(T->ptr[0]);
            if (T->ptr[1]){
                T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                semantic_Analysis(T->ptr[1]);
                T->num=T->ptr[0]->num+T->ptr[1]->num;        //统计参数个数
                T->width=T->ptr[0]->width+T->ptr[1]->width;  //累加参数单元宽度
                T->code=merge(2,T->ptr[0]->code,T->ptr[1]->code);  //连接参数代码
                }
            else {
                T->num=T->ptr[0]->num;
                T->width=T->ptr[0]->width;
                T->code=T->ptr[0]->code;
                }
            break;
	case  PARAM_DEC:
            rtn=fillSymbolTable(T->ptr[1]->type_id,newAlias(),1,T->ptr[0]->type,'P',T->offset,T->isArray);
            if (rtn==-1)
                semantic_error(T->ptr[1]->pos,T->ptr[1]->type_id, "参数名重复定义");
            else T->ptr[1]->place=rtn;
            T->num=1;       //参数个数计算的初始值
            T->width=T->ptr[0]->type==INT?4:8;  //参数宽度
            result.kind=ID;   strcpy(result.id, symbolTable.symbols[rtn].alias);
            result.offset=T->offset;
            T->code=genIR(PARAM,opn1,opn2,result);     //生成：FUNCTION 函数名
            break;
	case COMP_STM:
            LEV++;
            hasRet = 0;
            //设置层号加1，并且保存该层局部变量在符号表中的起始位置在symbol_scope_TX
            symbol_scope_TX.TX[symbol_scope_TX.top++]=symbolTable.index;
            T->width=0;
            T->code=NULL;
            if (T->ptr[0]) {
                T->ptr[0]->offset=T->offset;
                semantic_Analysis(T->ptr[0]);  //处理该层的局部变量DEF_LIST
                T->width+=T->ptr[0]->width;
                T->code=T->ptr[0]->code;
                }
            if (T->ptr[1]){
                strcpy(T->ptr[1]->Etrue, T->Etrue); // 向下传递
                strcpy(T->ptr[1]->Efalse, T->Efalse); // 向下传递
                // printf("for debug: COMP_STM: T->ptr[1]->Etrue=%s; T->ptr[1]->Efalse=%s;\n",T->ptr[1]->Etrue,T->ptr[1]->Efalse);
                // printf("for debug: COMP_STM: T->ptr[1]->kind=%d\n", T->ptr[1]->kind);
                T->ptr[1]->offset=T->offset+T->width;
                strcpy(T->ptr[1]->Snext,T->Snext);  //S.next属性向下传递
                semantic_Analysis(T->ptr[1]);       //处理复合语句的语句序列
                T->width+=T->ptr[1]->width;
                T->code=merge(2,T->code,T->ptr[1]->code);
                }
            
             prn_symbol();       //c在退出一个符合语句前显示的符号表
             LEV--;    //出复合语句，层号减1
             symbolTable.index=symbol_scope_TX.TX[--symbol_scope_TX.top]; //删除该作用域中的符号
             break;
    case DEF_LIST:
            T->code=NULL;
            if (T->ptr[0]){
                T->ptr[0]->offset=T->offset;
                semantic_Analysis(T->ptr[0]);   //处理一个局部变量定义
                T->code=T->ptr[0]->code;
                T->width=T->ptr[0]->width;
                }
            if (T->ptr[1]) {
                T->ptr[1]->offset=T->offset+T->ptr[0]->width;
                semantic_Analysis(T->ptr[1]);   //处理剩下的局部变量定义
                T->code=merge(2,T->code,T->ptr[1]->code);
                T->width+=T->ptr[1]->width;
                }
                break;
    case VAR_DEF://处理一个局部变量定义,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
                 //类似于上面的外部变量EXT_VAR_DEF，换了一种处理方法
                T->code=NULL;
                T->ptr[1]->type=!strcmp(T->ptr[0]->type_id,"int")?INT: 
                                !strcmp(T->ptr[0]->type_id,"float")?FLOAT:
                                !strcmp(T->ptr[0]->type_id,"char")?CHAR:
                                strstr(T->ptr[0]->type_id,"数组")?T->ptr[0]->type:STRUCT;  //确定变量序列各变量类型
                T0=T->ptr[1];
                if (T0->ptr[0]->kind==Array)
                    T->ptr[1]->isArray = 1;
                 
                    T->ptr[1]->isArray = 0;
                 //T0为变量名列表子树根指针，对ID、ASSIGNOP类结点在登记到符号表，作为局部变量
                num=0;
                T0->offset=T->offset;
                T->width=0;
                //width=T->ptr[1]->type==INT?4:8;  //一个变量宽度
                width=T->ptr[1]->type==INT?4:T->ptr[1]->type==INT?8:1;  //一个变量宽度
                //cout<<"Array"<<Array;
                while (T0) {  //处理所以DEC_LIST结点
                    num++;
                    T0->ptr[0]->type=T0->type;  //类型属性向下传递
                    if (T0->ptr[1])
                        T0->ptr[1]->type=T0->type;
                    T0->ptr[0]->offset=T0->offset;  //类型属性向下传递
                    T0->ptr[0]->isArray=T0->isArray; 
                    //cout<<"T0->ptr[0]->kind :"<<T0->ptr[0]->kind<<endl;
                    if (T0->ptr[1]) 
                    {
                        T0->ptr[1]->offset=T0->offset+width; 
                        T0->ptr[1]->isArray=T0->isArray; 
                    }
                    if (T0->ptr[0]->kind==ID){
                        rtn=fillSymbolTable(T0->ptr[0]->type_id,newAlias(),LEV,T0->ptr[0]->type,'V',T->offset+T->width,T0->isArray);//此处偏移量未计算，暂时为0
                        if (rtn==-1)
                            semantic_error(T0->ptr[0]->pos,T0->ptr[0]->type_id, "变量重复定义");
                        else  
                            T0->ptr[0]->place=rtn;
                        T->width+=width;
                        symbolTable.symbols[rtn].isArray = 0;
                    }
                    else if (T0->ptr[0]->kind==Array)
                    {
                        //printf("array dec");
                        if (T0->ptr[0]->ptr[1]->type_int <= 0)
                        {
                            semantic_error(T0->ptr[0]->pos, "", "数组长度不能小于等于0");
                        }
                        rtn=fillSymbolTable(T0->ptr[0]->ptr[0]->type_id,newAlias(),LEV,T0->ptr[0]->type,'V',T->offset+T->width,T0->isArray);
                        //printf("%s\n", symbolTable.symbols[rtn].alias);
                        if (rtn==-1)
                            semantic_error(T0->ptr[0]->pos,T0->ptr[0]->type_id, "变量重复定义");
                        else T0->ptr[0]->place=rtn;
                        struct node *tmp = T0->ptr[0];
                        int plusWidth = 1;
                        int count = 0;
                        //多维数组的处理
                        while(tmp->ptr[0] != NULL)
                        {
                            plusWidth *= tmp->ptr[1]->type_int;
                            symbolTable.symbols[rtn].listLength[count] = tmp->ptr[1]->type_int;
                            tmp = tmp->ptr[0];
                        }
                        strcpy(symbolTable.symbols[rtn].name,tmp->type_id);
                        T->width+=width*plusWidth;
                        symbolTable.symbols[rtn].isArray = 1;
                    }
                    else if (T0->ptr[0]->kind==ASSIGNOP){
                            rtn=fillSymbolTable(T0->ptr[0]->ptr[0]->type_id,newAlias(),LEV,T0->ptr[0]->type,'V',T->offset+T->width,T0->isArray);//此处偏移量未计算，暂时为0
                            if (rtn==-1)
                                semantic_error(T0->ptr[0]->ptr[0]->pos,T0->ptr[0]->ptr[0]->type_id, "变量重复定义");
                            else {
                                T0->ptr[0]->place=rtn;
                                T0->ptr[0]->ptr[1]->offset=T->offset+T->width+width;
                                Exp(T0->ptr[0]->ptr[1]);
                                opn1.kind=ID; 
                                strcpy(opn1.id,symbolTable.symbols[T0->ptr[0]->ptr[1]->place].alias);
                                result.kind=ID; 
                                strcpy(result.id,symbolTable.symbols[T0->ptr[0]->place].alias);
                                T->code=merge(3,T->code,T0->ptr[0]->ptr[1]->code,genIR(ASSIGNOP,opn1,opn2,result));
                                }
                            T->width+=width+T0->ptr[0]->ptr[1]->width;
                        }
                    T0=T0->ptr[1];
                    }
                break;
	case STM_LIST:
                if (!T->ptr[0]) { T->code=NULL; T->width=0; break;}   //空语句序列
                if (T->ptr[1]) //2条以上语句连接，生成新标号作为第一条语句结束后到达的位置
                    strcpy(T->ptr[0]->Snext,newLabel());
                 else     //语句序列仅有一条语句，S.next属性向下传递
                    strcpy(T->ptr[0]->Snext,T->Snext);

                strcpy(T->ptr[0]->Etrue, T->Etrue); // 向下传递
                strcpy(T->ptr[0]->Efalse, T->Efalse); // 向下传递
                // printf("for debug: STM_LIST: T->ptr[0]->Etrue=%s; T->ptr[0]->Efalse=%s;\n",T->ptr[0]->Etrue,T->ptr[0]->Efalse);
                // printf("for debug: STM_LIST: T->ptr[0]->kind=%d\n", T->ptr[0]->kind);
                T->ptr[0]->offset=T->offset;
                semantic_Analysis(T->ptr[0]);
                T->code=T->ptr[0]->code;
                T->width=T->ptr[0]->width;
                if (T->ptr[1]){     //2条以上语句连接,S.next属性向下传递
                    strcpy(T->ptr[1]->Snext,T->Snext);
                    strcpy(T->ptr[1]->Etrue, T->Etrue); // 向下传递
                    strcpy(T->ptr[1]->Efalse, T->Efalse); // 向下传递
                    T->ptr[1]->offset=T->offset;  //顺序结构共享单元方式
//                  T->ptr[1]->offset=T->offset+T->ptr[0]->width; //顺序结构顺序分配单元方式
                    semantic_Analysis(T->ptr[1]);
                    //序列中第1条为表达式语句，返回语句，复合语句时，第2条前不需要标号
                    if (T->ptr[0]->kind==RETURN ||T->ptr[0]->kind==EXP_STMT ||T->ptr[0]->kind==COMP_STM)
                        T->code=merge(2,T->code,T->ptr[1]->code);
                    else
                        T->code=merge(3,T->code,genLabel(T->ptr[0]->Snext),T->ptr[1]->code);
                    if (T->ptr[1]->width>T->width) T->width=T->ptr[1]->width; //顺序结构共享单元方式
//                        T->width+=T->ptr[1]->width;//顺序结构顺序分配单元方式
                    }
                break;
	case IF_THEN:
                strcpy(T->ptr[0]->Etrue,newLabel());  //设置条件语句真假转移位置
                strcpy(T->ptr[0]->Efalse,T->Snext);
                T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                boolExp(T->ptr[0]);
                T->width=T->ptr[0]->width;
                strcpy(T->ptr[1]->Snext,T->Snext);
                strcpy(T->ptr[1]->Etrue, T->Etrue); // 向下传递
                strcpy(T->ptr[1]->Efalse,T->Efalse); // 向下传递
                semantic_Analysis(T->ptr[1]);      //if子句
                if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                T->code=merge(3,T->ptr[0]->code, genLabel(T->ptr[0]->Etrue),T->ptr[1]->code);
                break;  //控制语句都还没有处理offset和width属性
	case IF_THEN_ELSE:
                strcpy(T->ptr[0]->Etrue,newLabel());  //设置条件语句真假转移位置
                strcpy(T->ptr[0]->Efalse,newLabel());
                T->ptr[0]->offset=T->ptr[1]->offset=T->ptr[2]->offset=T->offset;
                boolExp(T->ptr[0]);      //条件，要单独按短路代码处理
                T->width=T->ptr[0]->width;
                strcpy(T->ptr[1]->Snext,T->Snext);
                strcpy(T->ptr[1]->Etrue, T->Etrue); // 向下传递
                strcpy(T->ptr[1]->Efalse,T->Efalse); // 向下传递
                semantic_Analysis(T->ptr[1]);      //if子句
                if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                strcpy(T->ptr[2]->Snext,T->Snext);
                strcpy(T->ptr[2]->Etrue, T->Etrue); // 向下传递
                strcpy(T->ptr[2]->Efalse,T->Efalse); // 向下传递
                semantic_Analysis(T->ptr[2]);      //else子句
                if (T->width<T->ptr[2]->width) T->width=T->ptr[2]->width;
                T->code=merge(6,T->ptr[0]->code,genLabel(T->ptr[0]->Etrue),T->ptr[1]->code,\
                              genGoto(T->Snext),genLabel(T->ptr[0]->Efalse),T->ptr[2]->code);
                break;
	case WHILE: strcpy(T->ptr[0]->Etrue,newLabel());  //子结点继承属性的计算
                strcpy(T->ptr[0]->Efalse,T->Snext);
                T->ptr[0]->offset=T->ptr[1]->offset=T->offset;
                boolExp(T->ptr[0]);      //循环条件，要单独按短路代码处理
                T->width=T->ptr[0]->width;
                strcpy(T->ptr[1]->Snext,newLabel());
                strcpy(T->ptr[1]->Etrue, T->ptr[1]->Snext);
                strcpy(T->ptr[1]->Efalse, T->ptr[0]->Efalse);
                // printf("for debug: WHILE: T->ptr[1]->Etrue=%s; T->ptr[1]->Efalse=%s;\n",T->ptr[1]->Etrue,T->ptr[1]->Efalse);
                // printf("for debug: WHILE: T->ptr[1]->kind=%d\n", T->ptr[1]->kind);
                inLoop = 1;
                semantic_Analysis(T->ptr[1]);      //循环体
                inLoop = 0;
                if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                T->code=merge(5,genLabel(T->ptr[1]->Snext),T->ptr[0]->code, \
                genLabel(T->ptr[0]->Etrue),T->ptr[1]->code,genGoto(T->ptr[1]->Snext));
                break;
    case FOR:  // 
                // 分析初始化的条件
                T->ptr[0]->offset=T->offset;
                T->ptr[0]->ptr[0]->offset=T->ptr[0]->offset;
                Exp(T->ptr[0]->ptr[0]); 
                T->ptr[0]->offset += T->ptr[0]->ptr[0]->width;
                T->offset = T->ptr[1]->offset = T->ptr[0]->offset;

                // 分析循环条件
                strcpy(T->ptr[0]->ptr[1]->Etrue,newLabel());
                strcpy(T->ptr[0]->ptr[1]->Efalse,T->Snext);
                T->ptr[0]->ptr[1]->offset=T->ptr[0]->offset;
                boolExp(T->ptr[0]->ptr[1]);
                T->width=T->ptr[0]->width;
                strcpy(T->ptr[1]->Snext,newLabel());
                inLoop = 1; //
                strcpy(T->ptr[0]->ptr[2]->Snext, newLabel());
                T->ptr[0]->ptr[2]->code=genLabel(T->ptr[0]->ptr[2]->Snext);
                strcpy(T->ptr[1]->Etrue, T->ptr[0]->ptr[2]->Snext); // 向下传递
                strcpy(T->ptr[1]->Efalse, T->ptr[0]->ptr[1]->Efalse); // 向下传递
                // printf("for debug: FOR: T->ptr[1]->Etrue=%s; T->ptr[1]->Efalse=%s;\n",T->ptr[1]->Etrue,T->ptr[1]->Efalse);
                // printf("for debug: FOR: T->ptr[1]->kind=%d\n", T->ptr[1]->kind);
                semantic_Analysis(T->ptr[1]);
                T->ptr[0]->ptr[2]->offset = T->ptr[0]->offset+T->ptr[1]->width;
                semantic_Analysis(T->ptr[0]->ptr[2]); // 分析最后一个表达式
                if (T->width<T->ptr[1]->width) T->width=T->ptr[1]->width;
                inLoop = 0; //
                T->code=merge(7,T->ptr[0]->ptr[0]->code,genLabel(T->ptr[1]->Snext),T->ptr[0]->ptr[1]->code, \
                genLabel(T->ptr[0]->ptr[1]->Etrue),T->ptr[1]->code,T->ptr[0]->ptr[2]->code,genGoto(T->ptr[1]->Snext));
                break;
    case EXP_STMT:
                T->ptr[0]->offset=T->offset;
                strcpy(T->ptr[0]->Etrue, T->Etrue);
                strcpy(T->ptr[0]->Efalse, T->Efalse);
                // printf("for debug: EXP_STMT: T->ptr[0]->kind=%d\n", T->ptr[0]->kind);
                semantic_Analysis(T->ptr[0]);
                T->code=T->ptr[0]->code;
                T->width=T->ptr[0]->width;
                break;
	case RETURN:   if (T->ptr[0]){
                    T->ptr[0]->offset=T->offset;
                    Exp(T->ptr[0]);
                    num=symbolTable.index;
                    do num--; while (symbolTable.symbols[num].flag!='F');
                    hasRet = 1;
                    if (T->ptr[0]->type!=symbolTable.symbols[num].type) {
                        semantic_error(T->pos, "返回值类型错误","");
                        T->width=0;T->code=NULL;
                        break;
                    }
                    
                    T->width=T->ptr[0]->width;
                    result.kind=ID; strcpy(result.id,symbolTable.symbols[T->ptr[0]->place].alias);
                    result.offset=symbolTable.symbols[T->ptr[0]->place].offset;
                    T->code=merge(2,T->ptr[0]->code,genIR(RETURN,opn1,opn2,result));
                    }
                else{
                    T->width=0;
                    result.kind=0;
                    T->code=genIR(RETURN,opn1,opn2,result);
                    }
                break;
	case ID:
    case INT:
    case FLOAT:
    case CHAR:
	case ASSIGNOP:
	case AND:
	case OR:
	case RELOP:
	case PLUS:
	case MINUS:
	case STAR:
	case DIV:
	case NOT:
	case UMINUS:
    case FUNC_CALL:
    case BREAK:
    case CONTINUE:
    case INCREMENT:
    case DECREMENT:
    case PREDECREMENT:
    case PREINCREMENT:
                    
                    crementCode = NULL;
                    Exp(T);          //处理基本表达式
                    // 处理完表达式，如果有带链接的自增或者自减运算，那么就需要进行连接
                    if (crementCode) {
                        T->code=merge(2, T->code, crementCode);
                    }
                    if (crementNode)
                        free(crementNode);
                    break;
    }
    }
}
void objectCode(struct codenode *head)
{
    char opnstr1[32],opnstr2[32],resultstr[32];
    struct codenode *h=head,*p;
    int i;
    FILE *fp;
    fp=fopen("object.s","w");
    fprintf(fp,".data\n");
    fprintf(fp,"_Prompt: .asciiz \"Enter an integer:  \"\n");
    fprintf(fp,"_ret: .asciiz \"\\n\"\n");
    fprintf(fp,".globl main\n");
    fprintf(fp,".text\n");
    fprintf(fp,"addi $sp,$zero,0x00002ffc\n");
    fprintf(fp,"add $t1, $zero, 0x0003310\n");
    fprintf(fp,"j main\n");
    fprintf(fp,"read:\n");
    fprintf(fp,"  li $v0,4\n");
    fprintf(fp,"  la $a0,_Prompt\n");
    fprintf(fp,"  syscall\n");
    fprintf(fp,"  li $v0,5\n");
    fprintf(fp,"  syscall\n");
    fprintf(fp,"  jr $ra\n");
    fprintf(fp,"write:\n");
    // fprintf(fp,"  addi $v0,$0,34\n");
    // fprintf(fp,"  syscall\n");
    fprintf(fp,"  li $v0,1\n");
    fprintf(fp,"  syscall\n");
    fprintf(fp,"  li $v0,4\n");
    fprintf(fp,"  la $a0,_ret\n");
    fprintf(fp,"  syscall\n");
    fprintf(fp,"  move $v0,$0\n");
    fprintf(fp,"  jr $ra\n");
    do {
        switch (h->op) {
            case ASSIGNOP:
                        if (h->opn1.kind==INT)
                            // fprintf(fp, "  addi $t3,$0,%d\n", h->opn1.const_int);
                            fprintf(fp, "  li $t3,%d\n", h->opn1.const_int);
                        else {
                            fprintf(fp, "  lw $t1, %d($sp)\n", h->opn1.offset);
                            fprintf(fp, "  move $t3, $t1\n");
                            }
                        fprintf(fp, "  sw $t3, %d($sp)\n", h->result.offset);
                        break;
            case PLUS:
            case MINUS:
            case STAR:
            case DIV:
                       fprintf(fp, "  lw $t1, %d($sp)\n", h->opn1.offset);
                       fprintf(fp, "  lw $t2, %d($sp)\n", h->opn2.offset);
                       if (h->op==PLUS)       fprintf(fp, "  add $t3,$t1,$t2\n");
                       else if (h->op==MINUS) fprintf(fp, "  sub $t3,$t1,$t2\n");
                            else if (h->op==STAR)  fprintf(fp, "  mul $t3,$t1,$t2\n");
                                 else  {fprintf(fp, "  div $t1, $t2\n");
                                        fprintf(fp, "  mflo $t3\n");
                                        }
                        fprintf(fp, "  sw $t3, %d($sp)\n", h->result.offset);
                        break;
            case FUNCTION:
                        fprintf(fp, "\n%s:\n", h->result.id);
                        if (!strcmp(h->result.id,"main")) 
                            fprintf(fp, "  addi $sp, $sp, -%d\n",symbolTable.symbols[h->result.offset].offset);
                        break;
            case PARAM: 
                        break;
            case LABEL: fprintf(fp, "%s:\n", h->result.id);
                        break;
            case GOTO:  fprintf(fp, "  j %s\n", h->result.id);
                        break;
            case JLE:
            case JLT:
            case JGE:
            case JGT:
            case EQ:
            case NEQ:
                        fprintf(fp, "  lw $t1, %d($sp)\n", h->opn1.offset);
                        fprintf(fp, "  lw $t2, %d($sp)\n", h->opn2.offset);
                        if (h->op==JLE) fprintf(fp, "  ble $t1,$t2,%s\n", h->result.id);
                        else if (h->op==JLT) fprintf(fp, "  blt $t1,$t2,%s\n", h->result.id);
                        else if (h->op==JGE) fprintf(fp, "  bge $t1,$t2,%s\n", h->result.id);
                        else if (h->op==JGT) fprintf(fp, "  bgt $t1,$t2,%s\n", h->result.id);
                        else if (h->op==EQ)  fprintf(fp, "  beq $t1,$t2,%s\n", h->result.id);
                        else                 fprintf(fp, "  bne $t1,$t2,%s\n", h->result.id);
                        break;
            case ARG:   
                        break;
            case CALL:  if (!strcmp(h->opn1.id,"read")){ 
                            fprintf(fp, "  addi $sp, $sp, -4\n");
                            fprintf(fp, "  sw $ra,0($sp)\n"); 
                            fprintf(fp, "  jal read\n"); 
                            fprintf(fp, "  lw $ra,0($sp)\n"); 
                            fprintf(fp, "  addi $sp, $sp, 4\n");
                            fprintf(fp, "  sw $v0, %d($sp)\n",h->result.offset);
                            break;
                            }
                        if (!strcmp(h->opn1.id,"write")){ 
                            fprintf(fp, "  lw $a0, %d($sp)\n",h->prior->result.offset);
                            fprintf(fp, "  addi $sp, $sp, -4\n");
                            fprintf(fp, "  sw $ra,0($sp)\n");
                            fprintf(fp, "  jal write\n");
                            fprintf(fp, "  lw $ra,0($sp)\n");
                            fprintf(fp, "  addi $sp, $sp, 4\n");
                            break;
                            }

                        for(p=h,i=0;i<symbolTable.symbols[h->opn1.offset].paramnum;i++)  
                                p=p->prior;
                        
                        fprintf(fp, "  move $t0,$sp\n"); 
                        fprintf(fp, "  addi $sp, $sp, -%d\n", symbolTable.symbols[h->opn1.offset].offset);
                        fprintf(fp, "  sw $ra,0($sp)\n"); 
                        i=h->opn1.offset+1;  
                        while (symbolTable.symbols[i].flag=='P')
                            {
                            fprintf(fp, "  lw $t1, %d($t0)\n", p->result.offset); 
                            fprintf(fp, "  move $t3,$t1\n");
                            fprintf(fp, "  sw $t3,%d($sp)\n",  symbolTable.symbols[i].offset); 
                            p=p->next; i++;
                            }
                        fprintf(fp, "  jal %s\n",h->opn1.id); 
                        fprintf(fp, "  lw $ra,0($sp)\n"); 
                        fprintf(fp, "  addi $sp,$sp,%d\n",symbolTable.symbols[h->opn1.offset].offset); 
                        fprintf(fp, "  sw $v0,%d($sp)\n", h->result.offset); 
                        break;
            case RETURN:fprintf(fp, "  lw $v0,%d($sp)\n",h->result.offset); 
                        fprintf(fp, "  jr $ra\n");
                        break;

        }
    h=h->next;
    } while (h!=head);
fclose(fp);
}
void semantic_Analysis0(struct node *T) {
    symbolTable.index=0;
    fillSymbolTable("read","",0,INT,'F',4, 0);
    symbolTable.symbols[0].paramnum=0;//read的形参个数
    fillSymbolTable("write","",0,INT,'F',4, 0);
    symbolTable.symbols[1].paramnum=1;
    fillSymbolTable("x","",1,INT,'P',12, 0);
    symbol_scope_TX.TX[0]=0;  //外部变量在符号表中的起始序号为0
    symbol_scope_TX.top=1;
    T->offset=0;              //外部变量在数据区的偏移量
    semantic_Analysis(T);
    int rtn = searchSymbolTable("main");
    if (rtn == -1)
        semantic_error(T->pos,"", "缺少main函数");
    prnIR(T->code);
    objectCode(T->code);
    //printf("ef %d\n",error_flag);
    // if(error_flag)
    //     cout<<error_record;
    
 }
