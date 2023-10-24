%{

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"parser.h"

//#define YY_DECL int alpha_yylex (void* ylval)

int tokeno=0;

struct alpha_token_t {

  unsigned int              numLine;
  unsigned int              finishLine;
  unsigned int              numToken;
  char*                     content;
  char*                     type;
  char*                     caps;
  char*                     des;
  struct alpha_token_t*     next;
};

struct alpha_token_t* H=NULL;

void insertToken(struct alpha_token_t **Tok,unsigned int new_numLine,unsigned int new_finishLine, unsigned int new_numToken, char *new_Content,char *new_Type,char *new_Caps, char *new_Des){
    
    struct alpha_token_t* new_Token;
    struct alpha_token_t* last=*Tok;                        /*trace pointer to find the last node of the list*/ 

    new_Token=malloc(sizeof(struct alpha_token_t));         /*allocate memory for the new token*/

    if(new_Token==NULL) {
        printf("Error Memory Allocation\n");
        exit(0);
    }

    new_Token->numLine=new_numLine;
    new_Token->finishLine=new_finishLine;
    new_Token->numToken=new_numToken;
    new_Token->content=new_Content;
    new_Token->type=new_Type;
    new_Token->caps=new_Caps;
    new_Token->des=new_Des; 
    new_Token->next=NULL;

    if(*Tok==NULL){                                         /* if global pointers points to NULL add it as first node*/
        *Tok=new_Token;
    }else{
        while(last->next!=NULL){                            /* Find the Last node of the List*/ 
            last=last->next;
        }
    last->next=new_Token;                                   /*Add new node at the end of the List*/ 
    }
}

void print(struct alpha_token_t *Tok){
    
    /*decalrations*/
    struct alpha_token_t*   n;
    /*assignments*/
    n=Tok;

    /*code*/
    while(n!=NULL){
        if(n->finishLine!=-1){
            printf("%d:     #%d     \"%d-%d\"      %s   %s  %s",n->numLine,n->numToken,n->numLine,n->finishLine,n->type,n->caps,n->des);
        }else{
            printf("%d:     #%d     \"%s\"    %s   %s   %s",n->numLine,n->numToken,n->content,n->type,n->caps,n->des);
        } 
                
        n=n->next;
        printf("\n");     
    }
    free(n);
}

%}

%option noyywrap
%option yylineno



IF                      "if"
ELSE                    "else"
WHILE                   "while"
FOR                     "for"
FUNCTION                "function"
RETURN                  "return"
BREAK                   "break"
CONTINUE                "continue"
AND                     "and"
NOT                     "not"
OR                      "or"
LOCAL                   "local"
TRUE                    "true"
FALSE                   "false"
NIL                     "nil"
ASSIGN                  "="
PLUS                    "+"
MINUS                   "-"
MUL                     "*"
DIVIDE                  "/"
MOD                     "%"
EQUAL                   "=="
NOT_EQUAL               "!="
PLUS_PLUS               "++"
MINUS_MINUS             "--"
BIGGER                  ">"
LESS                    "<"
BIGGER_EQUAL            ">="
LESS_EQUAL              "<="
LEFT_BRACE              "{"
RIGHT_BRACE             "}"
LEFT_BRACKET            "["
RIGHT_BRACKET           "]"
LEFT_PARENTHESIS        "("
RIGHT_PARENTHESIS       ")"
SEMICOLON               ";"
COMMA                   ","
COLON                   ":"
DOUBLE_COLON            "::"
DOT                     "."
DOUBLE_DOT              ".."
ID                      [a-zA-Z][a-zA-Z_0-9]*
INTEGER                 (0[xX][0-9A-Fa-f]+)|([0-9]+)
REAL_CONSTANT           [0-9]*\.[0-9](e|E)?[0-9]*
SPACE                   [\r \t\v]
NEWLINE                 "\n"
STRING                  \"

%%

{IF}                        {tokeno++; insertToken(&H,yylineno,-1,tokeno,"if","KEYWORD","IF","<-enumerated"); return IF;}
{ELSE}                      {tokeno++; insertToken(&H,yylineno,-1,tokeno,"else","KEYWORD","ELSE","<-enumerated"); return ELSE;}
{WHILE}                     {tokeno++; insertToken(&H,yylineno,-1,tokeno,"while","KEYWORD","WHILE","<-enumerated"); return WHILE;}
{FOR}                       {tokeno++; insertToken(&H,yylineno,-1,tokeno,"for","KEYWORD","FOR","<-enumerated"); return FOR;}
{FUNCTION}                  {tokeno++; insertToken(&H,yylineno,-1,tokeno,"function","KEYWORD","FUNCTION","<-enumerated"); return FUNCTION;}
{RETURN}                    {tokeno++; insertToken(&H,yylineno,-1,tokeno,"return","KEYWORD","RETURN","<-enumerated"); return RETURN;}
{BREAK}                     {tokeno++; insertToken(&H,yylineno,-1,tokeno,"break","KEYWORD","BREAK","<-enumerated"); return BREAK;}
{CONTINUE}                  {tokeno++; insertToken(&H,yylineno,-1,tokeno,"continue","KEYWORD","CONTINUE","<-enumerated"); return CONTINUE;}
{AND}                       {tokeno++; insertToken(&H,yylineno,-1,tokeno,"and","KEYWORD","AND","<-enumerated"); return AND;}
{NOT}                       {tokeno++; insertToken(&H,yylineno,-1,tokeno,"not","KEYWORD","NOT","<-enumerated"); return NOT;}
{OR}                        {tokeno++; insertToken(&H,yylineno,-1,tokeno,"or","KEYWORD","OR","<-enumerated"); return OR;}
{LOCAL}                     {tokeno++; insertToken(&H,yylineno,-1,tokeno,"local","KEYWORD","LOCAL","<-enumerated"); return LOCAL;}
{TRUE}                      {tokeno++; insertToken(&H,yylineno,-1,tokeno,"true","KEYWORD","TRUE","<-enumerated"); return TRUE;}
{FALSE}                     {tokeno++; insertToken(&H,yylineno,-1,tokeno,"false","KEYWORD","FALSE","<-enumerated"); return FALSE;}
{NIL}                       {tokeno++; insertToken(&H,yylineno,-1,tokeno,"nil","KEYWORD","NIL","<-enumerated"); return NIL;}
"#".*                       {tokeno++; insertToken(&H,yylineno,-1,tokeno," ","COMMENT","LINE_COMMENT","<-enumerated");}
"//".*                      {tokeno++; insertToken(&H,yylineno,-1,tokeno," ","COMMENT","LINE_COMMENT","<-enumerated");}
"/*"                        {   
                                int in;
                                int start_com=1,finish_com=0,nest_com=0;
                                int yyline1=0,yyline2=0,yyline3=0;

                                yyline1=yylineno;
                                
                                while(in=input()) {
                                
                                    if(in == '/') {
                                        if((in=input()) == '*'){
                                            yyline2=yylineno;
                                            tokeno++;
                                            start_com++;
                                            nest_com++;
                                        }else unput(in);
                                    }
                                    if(in == '*'){
                                        if((in=input()) =='/'){
                                            finish_com++;
                                            nest_com++;
                                            tokeno++;
                                            yyline3=yylineno;
                                                if(start_com!=finish_com) {
                                                    tokeno++;
                                                    insertToken(&H,yyline2,yyline3,tokeno," ","COMMENT","NESTED_COMMENT","<-enumerated");
                                                }
                                        }else unput(in);  
                                    }
                                    if(start_com==finish_com) break;
                                }

                                if(start_com!=finish_com) {
                                    tokeno++; printf("%d:     #%d     ERROR: UNTERMINATED COMMENT\n",yylineno,tokeno);
                                    exit(0);
                                }

                                yyline3=yylineno;
                                tokeno++;
                                insertToken(&H,yyline1,yyline3,tokeno,"","COMMENT","BLOCK_COMMENT","<-enumerated");
                        
                            }

                
{ASSIGN}                    {tokeno++; insertToken(&H,yylineno,-1,tokeno,"=","OPERATOR","ASSIGN","<-enumerated"); return ASSIGN;}
{PLUS}                      {tokeno++; insertToken(&H,yylineno,-1,tokeno,"+","OPERATOR","PLUS","<-enumerated"); return PLUS;}
{MINUS}                     {tokeno++; insertToken(&H,yylineno,-1,tokeno,"-","OPERATOR","MINUS","<-enumerated"); return MINUS;} 
{MUL}                       {tokeno++; insertToken(&H,yylineno,-1,tokeno,"*","OPERATOR",",ULTIPLY","<-enumerated"); return MUL;}
{DIVIDE}                    {tokeno++; insertToken(&H,yylineno,-1,tokeno,"/","OPERATOR","DIVIDE","<-enumerated"); return DIVIDE;}
{MOD}                       {tokeno++; insertToken(&H,yylineno,-1,tokeno,"%%","OPERATOR","MOD","<-enumerated"); return MOD;}
{EQUAL}                     {tokeno++; insertToken(&H,yylineno,-1,tokeno,"==","OPERATOR","EQUAL","<-enumerated"); return EQUAL;}
{NOT_EQUAL}                 {tokeno++; insertToken(&H,yylineno,-1,tokeno,"!=","OPERATOR","NOT_EQUAL","<-enumerated"); return NOT_EQUAL;}
{PLUS_PLUS}                 {tokeno++; insertToken(&H,yylineno,-1,tokeno,"++","OPERATOR","PLUS_PLUS","<-enumerated"); return  PLUS_PLUS; }
{MINUS_MINUS}               {tokeno++; insertToken(&H,yylineno,-1,tokeno,"--","OPERATOR","MINUS_MINUS","<-enumerated"); return MINUS_MINUS;}
{BIGGER}                    {tokeno++; insertToken(&H,yylineno,-1,tokeno,">","OPERATOR","GREATER","<-enumerated"); return BIGGER;}
{LESS}                      {tokeno++; insertToken(&H,yylineno,-1,tokeno,"<","OPERATOR","LESS","<-enumerated"); return  LESS; }
{BIGGER_EQUAL}              {tokeno++; insertToken(&H,yylineno,-1,tokeno,">=","OPERATOR","GREATER_EQUAL","<-enumerated"); return BIGGER_EQUAL;}
{LESS_EQUAL}                {tokeno++; insertToken(&H,yylineno,-1,tokeno,"<=","OPERATOR","LESS_EQUAL","<-enumerated"); return LESS_EQUAL;}
{LEFT_BRACE}                {tokeno++; insertToken(&H,yylineno,-1,tokeno,"{","PUNCTUATION","LEFT BRACE","<-enumerated"); return LEFT_BRACE;}
{RIGHT_BRACE}               {tokeno++; insertToken(&H,yylineno,-1,tokeno,"}","PUNCTUATION","RIGHT BRACE","<-enumerated"); return RIGHT_BRACE;}
{LEFT_BRACKET}              {tokeno++; insertToken(&H,yylineno,-1,tokeno,"[","PUNCTUATION","LEFT BRACKET","<-enumerated"); return LEFT_BRACKET;}
{RIGHT_BRACKET}             {tokeno++; insertToken(&H,yylineno,-1,tokeno,"]","PUNCTUATION","RIGHT BRACKET","<-enumerated"); return RIGHT_BRACKET;}
{LEFT_PARENTHESIS}          {tokeno++; insertToken(&H,yylineno,-1,tokeno,"(","PUNCTUATION","LEFT PARENTHESIS","<-enumerated"); return LEFT_PARENTHESIS;}
{RIGHT_PARENTHESIS}         {tokeno++; insertToken(&H,yylineno,-1,tokeno,")","PUNCTUATION","RIGHT PARENTHESIS","<-enumerated"); return RIGHT_PARENTHESIS;}
{SEMICOLON}                 {tokeno++; insertToken(&H,yylineno,-1,tokeno,";","PUNCTUATION","SEMICOLON","<-enumerated"); return SEMICOLON;}
{COMMA}                     {tokeno++; insertToken(&H,yylineno,-1,tokeno,",","PUNCTUATION","COMMA","<-enumerated"); return COMMA;}
{COLON}                     {tokeno++; insertToken(&H,yylineno,-1,tokeno,":","PUNCTUATION","COLON","<-enumerated"); return COLON;}
{DOUBLE_COLON}              {tokeno++; insertToken(&H,-1,yylineno,tokeno,"::","PUNCTUATION","DOUBLE_COLON","<-enumerated"); return DOUBLE_COLON;}
{DOT}                       {tokeno++; insertToken(&H,yylineno,-1,tokeno,".","PUNCTUATION","DOT","<-enumerated"); return DOT;}
{DOUBLE_DOT}                {tokeno++; insertToken(&H,yylineno,-1,tokeno,"..","PUNCTUATION","DOUBLE_DOT","<-enumerated"); return DOUBLE_DOT;}
{ID}                        {tokeno++; insertToken(&H,yylineno,-1,tokeno,yytext,"ID",yytext,"<-char"); yylval.stringVal=strdup(yytext); return ID;}
{INTEGER}                   {tokeno++; insertToken(&H,yylineno,-1,tokeno,yytext,"CONST_INT",yytext,"<-int"); yylval.intVal=atoi(yytext); return INTEGER;}
{REAL_CONSTANT}             {tokeno++; insertToken(&H,yylineno,-1,tokeno,yytext,"CONST_REAL",yytext,"<-real*"); yylval.realVal=atof(yytext); return REAL_CONSTANT;}
{SPACE}                     { }
{NEWLINE}                   { }
{STRING}                    {
                                int in;
                                int i=0,isClosed=0;
                                char *str=(char *)malloc(5000*sizeof(char));
                                int newyyline=yylineno;

                                while(in=input()){
                                    if(in=='\\'){
                                        if((in=input())=='"'){
                                            str=(char *)realloc(str,5000*sizeof(char));
                                            str[i++]='\"';
                                        }
                                        unput(in);

                                        if((in=input())=='t'){
                                            str=(char *)realloc(str,5000*sizeof(char));
                                            str[i++]='\t';
                                        }
                                        unput(in);

                                        if((in=input())=='n'){
                                            str=(char *)realloc(str,5000*sizeof(char));
                                        str[i++]='\n';                  
                                        }
                                        unput(in);

                                        if((in=input())=='\\'){
                                            str=(char *)realloc(str,5000*sizeof(char));
                                        str[i++]='\\';
                                        }
                                        unput(in);

                                        if((in=input())=='b'){
                                            str=(char *)realloc(str,5000*sizeof(char));
                                            str[i++]='\b';
                                        }
                                    }

                                    else if(in=='\"'){
                                        i++;
                                        isClosed=1;
                                        break;
                                    } 
                                    
                                    else {
                                        str=(char *)realloc(str,5000*sizeof(char));
                                        str[i++]=in;
                                    }
                                    
                                }
                                printf("\n");

                                str=(char *)realloc(str,5000*sizeof(char));
                                str[i++]=0;
                                tokeno++;
                                insertToken(&H,newyyline,-1,tokeno,str,"STRING",str,"<-char*");

                                if(isClosed==0) {
                                    printf("ERROR! Unterminated string\n");
                                    exit(0);
                                }

                                i=0;

                                yylval.stringVal=strdup(str); 
                                return STRING;
                            }


%%

