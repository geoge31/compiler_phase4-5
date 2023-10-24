%{
    #include"parser.h"
    #include"symbolTable.h"
    #include"targetCode.h"
    #include"avm.h"
    #include<stdio.h>
    #include<stdlib.h>

    #define YY_DECL int alpha_yylex (void* ylval)

    int yyerror(char* yaccProvidedMessage);
    int yylex(void);
    int funName=1;
    int maxScope=0;
    int ignoreLabel=-1;
    int inLoop=0;
    int inFunction=0;



    extern int yylineno;
    extern char* yytext;
    extern FILE* yyin;
    extern unsigned int gscope; 



%}

%start program

%union{
    int                             intVal;
    char*                           stringVal;
    double                          realVal;
    struct SymbolTableEntry*        sym;
    struct expr*                    expr;
    struct stmt_t*                  stmt;
    struct call_function*           call;
    struct for_loop*                forl;
   
}


%token                  IF
%token                  ELSE 
%token                  WHILE
%token                  FOR
%token                  FUNCTION
%token                  RETURN
%token  <stmt>          BREAK
%token  <stmt>          CONTINUE
%token                  AND
%token                  NOT
%token                  OR
%token                  LOCAL
%token                  TRUE
%token                  FALSE
%token                  NIL
%token                  ASSIGN
%token                  PLUS
%token                  MINUS 
%token                  UMINUS
%token                  MUL
%token                  DIVIDE
%token                  MOD
%token                  EQUAL
%token                  NOT_EQUAL
%token                  PLUS_PLUS
%token                  MINUS_MINUS
%token                  BIGGER
%token                  LESS
%token                  BIGGER_EQUAL
%token                  LESS_EQUAL
%token                  LEFT_BRACE 
%token                  RIGHT_BRACE
%token                  LEFT_BRACKET
%token                  RIGHT_BRACKET
%token                  LEFT_PARENTHESIS
%token                  RIGHT_PARENTHESIS
%token                  SEMICOLON
%token                  COMMA
%token                  COLON
%token                  DOUBLE_COLON
%token                  DOT
%token                  DOUBLE_DOT
%token  <stringVal>     STRING
%token  <stringVal>     ID 
%token  <intVal>        INTEGER
%token  <realVal>       REAL_CONSTANT



%right                  ASSIGN 
%left                   OR 
%left                   AND
%nonassoc               EQUAL NOT_EQUAL
%nonassoc               BIGGER BIGGER_EQUAL LESS LESS_EQUAL
%left                   PLUS MINUS
%left                   MUL DIVIDE MOD
%right                  NOT  PLUS_PLUS MINUS_MINUS  
%nonassoc               UMINUS
%left                   DOT DOUBLE_DOT
%left                   LEFT_BRACE RIGHT_BRACE
%left                   LEFT_BRACKET RIGHT_BRACKET
%left                   LEFT_PARENTHESIS RIGHT_PARENTHESIS



%type   <stmt>          stmts
%type   <stmt>          stmt
%type   <expr>          expr
%type   <expr>          term
%type   <expr>          assignexpr
%type   <expr>          primary
%type   <expr>          lvalue
%type   <expr>          tableitem
%type   <expr>          call
%type   <expr>          elist
%type   <expr>          exprs
%type   <expr>          objectdef
%type   <expr>          indexed
%type   <expr>          indexedelements
%type   <expr>          indexedelem
%type   <expr>          funcprefix
%type   <expr>          funcdef
%type   <expr>          const
%type   <expr>          idlist
%type   <expr>          ifstmt
%type   <call>          callsuffix
%type   <call>          normcall
%type   <call>          methodcall
%type   <intVal>        ifprefix
%type   <intVal>        elseprefix
%type   <intVal>        whilestart
%type   <intVal>        whilecond
%type   <intVal>        M
%type   <intVal>        N
%type   <intVal>        block
%type   <forl>          forprefix




%%

program:            
                    stmts
                    ;


stmts:              
                    stmt stmts {

                        $$=$1;
                    } 
                    |{
                        $$=NULL;  
                    }
                    ;

stmt:               
                    expr SEMICOLON {

                        //emit(assign,newExpr_constBool(1),NULL,$1,-1,yylineno);
                        //emit(jump,NULL,NULL,NULL,nextQuadLabel()+2,yylineno);
                        //emit(assign,newExpr_constBool(0),NULL,$1,-1,yylineno);
                    }
                    |ifstmt 
                    |whilestmt{ 
                        
                        $$=NULL;
                    }
                    |forstmt {

                        $$=NULL;
                    }
                    |returnstmt {
                        
                    }
                    |BREAK SEMICOLON {

                        if(inLoop>0){
                            make_stmt($1);
                            $1->breakList=newList(nextQuadLabel());
                            emit(jump,NULL,NULL,0,ignoreLabel,yylineno);
                        }else{
                            yyerror("Break not in a Loop");
                        }
                        
                    } 
                    |CONTINUE SEMICOLON {

                        if(inLoop>0){
                            make_stmt($1);
                            $1->contList=newList(nextQuadLabel());
                            emit(jump,NULL,NULL,0,ignoreLabel,yylineno);
                        }else{
                            yyerror("Continue not in a Loop");
                        }
                        
                    }
                    |block {

                        $$=NULL;
                    }
                    |funcdef {

                        $$=NULL;
                    }
                    |SEMICOLON {
                        
                        $$=NULL;
                    }
                    ;


expr:               
                    assignexpr  {
                        
                        $$=$1;
                    }
                    |expr PLUS expr {
                        
                        $$=newExpr(arithexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($3->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }

                        emit(add,$1,$3,$$,ignoreLabel,yylineno);
                    }
                    |expr MUL expr {

                        $$=newExpr(arithexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($3->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }

                        emit(mul,$1,$3,$$,ignoreLabel,yylineno);
                    }
                    |expr DIVIDE expr { 

                        $$=newExpr(arithexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($3->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }

                        emit(divd,$1,$3,$$,ignoreLabel,yylineno);
                    }
                    |expr MOD expr {
                        
                        $$=newExpr(arithexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($3->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }

                        emit(mod,$1,$3,$$,ignoreLabel,yylineno);
                    }
                    |expr EQUAL expr {

                        $$=newExpr(boolexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($3->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }

                        backpatch($1->truelist,nextQuadLabel());
                        backpatch($3->truelist,nextQuadLabel());
                        $$->truelist=newList(nextQuadLabel());
                        $$->falselist=newList(nextQuadLabel()+1);

                        emit(if_eq,$1,$3,$$,nextQuadLabel()+3,yylineno);
                        emit(jump,NULL,NULL,NULL,nextQuadLabel()+2,yylineno);
                    }
                    |expr BIGGER expr {

                        $$=newExpr(boolexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($3->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }

                        lists*  tl=newList(nextQuadLabel()+3);
                        lists*  fl=newList(nextQuadLabel()+4);
                        
                        backpatch($1->truelist,tl->q_quad);
                        backpatch($3->truelist,fl->q_quad);

                        emit(if_greater,$1,$3,$$,tl->q_quad,yylineno);
                        emit(jump,NULL,NULL,NULL,fl->q_quad+1,yylineno);
                        emit(assign,newExpr_constBool(1),NULL,$$,ignoreLabel,yylineno);
                        emit(jump,NULL,NULL,NULL,fl->q_quad+2,yylineno);
                        emit(assign,newExpr_constBool(0),NULL,$$,ignoreLabel,yylineno);
                    }
                    |expr BIGGER_EQUAL expr {

                        $$=newExpr(boolexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($3->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }

                        lists*  tl=newList(nextQuadLabel()+3);
                        lists*  fl=newList(nextQuadLabel()+4);
                        
                        backpatch($1->truelist,tl->q_quad);
                        backpatch($3->truelist,fl->q_quad);

                        emit(if_greatereq,$1,$3,$$,tl->q_quad,yylineno);
                        emit(jump,NULL,NULL,NULL,fl->q_quad+1,yylineno);
                        emit(assign,newExpr_constBool(1),NULL,$$,ignoreLabel,yylineno);
                        emit(jump,NULL,NULL,NULL,fl->q_quad+2,yylineno);
                        emit(assign,newExpr_constBool(0),NULL,$$,ignoreLabel,yylineno);
                    }
                    |expr LESS expr {

                        $$=newExpr(boolexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($3->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }

                        lists*  tl=newList(nextQuadLabel()+3);
                        lists*  fl=newList(nextQuadLabel()+4);
                        backpatch($1->truelist,tl->q_quad);
                        backpatch($3->truelist,fl->q_quad);

                        emit(if_less,$1,$3,$$,tl->q_quad,yylineno);
                        emit(jump,NULL,NULL,NULL,tl->q_quad+2,yylineno);
                        emit(assign,newExpr_constBool(1),NULL,$$,ignoreLabel,yylineno);
                        emit(jump,NULL,NULL,NULL,tl->q_quad+3,yylineno);
                        emit(assign,newExpr_constBool(0),NULL,$$,ignoreLabel,yylineno);
                    }
                    |expr LESS_EQUAL expr {

                        $$=newExpr(boolexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($3->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }
                    
                        lists*  tl=newList(nextQuadLabel()+3);
                        lists*  fl=newList(nextQuadLabel()+4);
                        backpatch($1->truelist,tl->q_quad);
                        backpatch($3->truelist,fl->q_quad);

                        emit(if_lesseq,$1,$3,$$,tl->q_quad,yylineno);
                        emit(jump,NULL,NULL,NULL,tl->q_quad+2,yylineno);
                        emit(assign,newExpr_constBool(1),NULL,$$,ignoreLabel,yylineno);
                        emit(jump,NULL,NULL,NULL,nextQuadLabel()+3,yylineno);
                        emit(assign,newExpr_constBool(0),NULL,$$,ignoreLabel,yylineno);
                    }
                    |expr NOT_EQUAL expr {

                        $$=newExpr(boolexpr_e);
                        $$->sym=newTemp();

                        if($1->type=constnum_e){
                            if($3->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }
                        lists*  tl=newList(nextQuadLabel()+3);
                        lists*  fl=newList(nextQuadLabel()+6);
                        
                        backpatch($1->truelist,tl->q_quad);
                        backpatch($3->truelist,fl->q_quad);

                        emit(if_noteq,$1,$3,$$,tl->q_quad,yylineno);
                        emit(jump,NULL,NULL,NULL,tl->q_quad+2,yylineno);
                        emit(assign,newExpr_constBool(1),NULL,$$,ignoreLabel,yylineno);
                        emit(jump,NULL,NULL,NULL,nextQuadLabel()+3,yylineno);
                        emit(assign,newExpr_constBool(0),NULL,$$,ignoreLabel,yylineno);
                    }
                    /*KAI EDW*/
                    |expr OR M expr { 
                        
                        $$=newExpr(boolexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($4->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }
                        
                        if($4!=boolexpr_e){
                            convert_NonBooleanToBoolean($4);
                        }
                        if($1!=boolexpr_e){
                            convert_NonBooleanToBoolean($1);
                        }
                        
                        lists* fl=newList(nextQuadLabel()+2);
                        lists* tl=newList(nextQuadLabel()+5);
                        backpatch($1->truelist,tl->q_quad);
                        backpatch($1->falselist,fl->q_quad);
                        $$->truelist=mergeList($1->truelist, $4->truelist);
                        $$->falselist=$4->falselist;

                        
                        emit(if_eq,$1,newExpr_constBool(1),NULL,tl->q_quad,yylineno);
                        emit(jump,NULL,NULL,NULL,fl->q_quad+1,yylineno);
                        emit(if_eq,$4,newExpr_constBool(1),NULL,nextQuadLabel()+3,yylineno);
                        emit(jump,NULL,NULL,NULL,nextQuadLabel()+4,yylineno);
                        emit(assign,newExpr_constBool(1),NULL,$$,ignoreLabel,yylineno);
                        emit(jump,NULL,NULL,NULL,nextQuadLabel()+3,yylineno);
                        emit(assign,newExpr_constBool(0),NULL,$$,ignoreLabel,yylineno);
                              
        
                    }
                    |expr AND M expr {
                        $$=newExpr(boolexpr_e);
                        $$->sym=newTemp();

                        if($1->type==constnum_e){
                            if($4->type==constnum_e){
                                $$->type=constnum_e;
                            }
                        }
                        
                        if($4!=boolexpr_e){
                            convert_NonBooleanToBoolean($4);
                        }
                        if($1!=boolexpr_e){
                            convert_NonBooleanToBoolean($1);
                        }
                        
                        lists* tl=newList(nextQuadLabel()+3);
                        lists* fl=newList(nextQuadLabel()+6);
                        backpatch($1->truelist,tl->q_quad);
                        backpatch($1->falselist,fl->q_quad);
                        $$->truelist=$4->truelist;
                        $$->falselist=mergeList($1->falselist,$4->falselist);

                        emit(if_eq,$1,newExpr_constBool(1),NULL,tl->q_quad,yylineno);
                        emit(jump,NULL,NULL,NULL,fl->q_quad+1,yylineno);
                        emit(if_eq,$4,newExpr_constBool(1),NULL,nextQuadLabel()+3,yylineno);
                        emit(jump,NULL,NULL,NULL,nextQuadLabel()+4,yylineno);
                        emit(assign,newExpr_constBool(1),NULL,$$,ignoreLabel,yylineno);
                        emit(jump,NULL,NULL,NULL,nextQuadLabel()+3,yylineno);
                        emit(assign,newExpr_constBool(0),NULL,$$,ignoreLabel,yylineno);
 

                    }
                    
                    |term {

                        $$=$1;
                    }
                    ;

M:                  
                    {
                        $$=nextQuadLabel();
                    }                    


N:
                    {
                        $$=nextQuadLabel();
                        emit(jump,NULL,NULL,0,ignoreLabel,yylineno);
                    }
term:               
                    LEFT_PARENTHESIS expr RIGHT_PARENTHESIS {

                        $$=$2;
                    }
                    |UMINUS expr {

                        check_arith($2,yylineno);
                        $$=newExpr(arithexpr_e);
                        $$->sym=isTempExpr($2) ? $$->sym : newTemp();
                        emit(uminus,$2,NULL,$$,nextQuadLabel(),yylineno);
                    }
                    |NOT expr {

                        $$=newExpr(boolexpr_e);
                        $$->sym=newTemp();
                        emit(not,$2,NULL,$$,nextQuadLabel(),yylineno);
                    }
                    |PLUS_PLUS lvalue {

                        int isF=checkForFunction(gscope,$2);
                        if(isF==1){
                            yyerror("This Symbol is a Function and you cannot increment it!");
                        }

                        check_arith($2,yylineno);

                        if($2->type==tableitem_e){
                            $$=emit_ifTableItem($2);
                            emit(add,$$,newExpr_constNum(1),$$,nextQuadLabel(),yylineno);
                            emit(tablesetelem,$2,$2->index,$$,nextQuadLabel(),yylineno);
                        }else{
                            emit(add,$2,newExpr_constNum(1),$2,nextQuadLabel(),yylineno);
                            $$=newExpr(arithexpr_e);
                            $$->sym=newTemp();
                            emit(assign,$2,NULL,$$,nextQuadLabel(),yylineno);
                        }
                    }
                    |lvalue PLUS_PLUS {

                        int isF=checkForFunction(gscope,$1);
                        if(isF==1){
                            yyerror("This Symbol is a Function and you cannot increment it!");
                        }

                        check_arith($1,yylineno);
                        $$=newExpr(var_e);
                        $$->sym=newTemp();

                        if($1->type==tableitem_e){
                            expr* val=emit_ifTableItem($1);
                            emit(assign,val,NULL,$$,ignoreLabel,yylineno);
                            emit(add,val,newExpr_constNum(1),val,nextQuadLabel(),yylineno);
                            emit(tablesetelem,$1,$1->index,val,nextQuadLabel(),yylineno);
                        }else{
                            emit(assign,$1,NULL,$$,ignoreLabel,yylineno);
                            emit(add,$1,newExpr_constNum(1),$1,nextQuadLabel(),yylineno);
                        }
                    }
                    |MINUS_MINUS lvalue {

                        int isF=checkForFunction(gscope,$2);
                        if(isF==1){
                            yyerror("This Symbol is a Function and you cannot increment it!");
                        }

                        check_arith($2,yylineno);

                        if($2->type==tableitem_e){
                            $$=emit_ifTableItem($2);
                            emit(sub,$$,newExpr_constNum(1),$$,nextQuadLabel(),yylineno);
                            emit(tablesetelem,$2,$2->index,$$,nextQuadLabel(),yylineno);
                        }else{
                            emit(sub,$2,newExpr_constNum(1),$2,nextQuadLabel(),yylineno);
                            $$=newExpr(arithexpr_e);
                            $$->sym=newTemp();
                            emit(assign,$2,NULL,$$,nextQuadLabel(),yylineno);
                        }
                    }
                    |lvalue MINUS_MINUS {

                        int isF=checkForFunction(gscope,$1);
                        if(isF==1){
                            yyerror("This Symbol is a Function and you cannot increment it!");
                        }

                        check_arith($1,yylineno);
                        $$=newExpr(var_e);
                        $$->sym=newTemp();

                        if($1->type==tableitem_e){
                            expr* val=emit_ifTableItem($1);
                            emit(assign,val,NULL,$$,ignoreLabel,yylineno);
                            emit(sub,val,newExpr_constNum(1),val,nextQuadLabel(),yylineno);
                            emit(tablesetelem,$1,$1->index,val,nextQuadLabel(),yylineno);
                        }else{
                            emit(assign,$1,NULL,$$,ignoreLabel,yylineno);
                            emit(sub,$1,newExpr_constNum(1),$1,nextQuadLabel(),yylineno);
                        }
                    }
                    |primary {

                        $$=$1;
                    }


assignexpr:         
                    lvalue {

                        int isF=checkForFunction(gscope,$1);
                        if(isF==1){
                            yyerror("This Symbol is a Function and you cannot assign it");
                        }
                    } ASSIGN expr {

                       if($1->type==tableitem_e){
                            emit(tablesetelem,$1,$1->index,$4,nextQuadLabel(),yylineno);

                            $$=emit_ifTableItem($1);
                            $$->type=assignexpr_e;
                       }else{

                            emit(assign,$4,NULL,$1,ignoreLabel,yylineno);

                            $$=newExpr(assignexpr_e);
                            $$->sym=newTemp();

                            emit(assign,$1,NULL,$$,ignoreLabel,yylineno);
                       }
                    }
                    ;


primary:            
                    lvalue {
                        $$=emit_ifTableItem($1);
                    }
                    |call
                    |objectdef
                    |LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS {

                       $$=newExpr(programfunc_e);
                       $$->sym=$2; 
                    }   
                    |const {

                        $$=$1;
                    }
                    ;

lvalue:             
                    ID {

                        int chf=checkForFunction(gscope,$1);
                        if(chf==1){
                            yyerror("Symbol is a function");
                        }else{
                            symtable* lkp=lookUp_inglobal($1);
                            if(!lkp){
                                if(gscope==0){
                                    lkp=set_Entry(gscope,yylineno,$1,GLOBALV,10);
                                }else{
                                    lkp=set_Entry(gscope,yylineno,$1,LOCALV,10);
                                }
                            }
                            
                            $$=lvalue_expr(lkp);   
                        }
                    }
                    |LOCAL ID  {

                        int chc=check_collisions($2);
                        int chf=checkForFunction(gscope,$2);
                        if(chc==1) {
                            yyerror("Collision with library function name");
                        }else if(chf==1){
                            yyerror("Symbol is a function");
                        }else{
                            symtable* lkp=lookUp_inscope(gscope,$2); 
                            if(!lkp) {
                                if(gscope==0){
                                    lkp=set_Entry(gscope,yylineno,$2,GLOBALV,10);
                                    $$=lvalue_expr(lkp);;
                                }
                                else{
                                    lkp=set_Entry(gscope,yylineno,$2,LOCALV,10);
                                    $$=lvalue_expr(lkp);
                                } 
                            }
                        }     
                    } 
                    |DOUBLE_COLON ID {

                        symtable* lkp=lookUp_namespace(0,$2); 
                        if(!lkp){
                            yyerror("Symbol doesn't exist");
                        }else{
                            $$=lvalue_expr(lkp);
                            
                        } 
                    }
                    |tableitem {

                        $$=$1;
                    }
                    ;


tableitem:          
                    lvalue DOT ID {
                        
                        $$=member_item($1,$3);
                    }
                    |lvalue LEFT_BRACKET expr RIGHT_BRACKET {

                        $1=emit_ifTableItem($1);
                        $$=newExpr(tableitem_e);
                        $$->sym=$1->sym;
                        $$->index=$3;
                    }
                    |call DOT ID
                    |call LEFT_BRACKET expr RIGHT_BRACKET
                    ;


call:               
                    call  LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {

                        $$=make_call($1,$3,yylineno);
                    }
                    |lvalue callsuffix {

                        int isF=checkForFunction(gscope,$1);
                        symtable* lkp;
                        if(isF==0){
                            lkp=lookUp_inglobal($1);
                            if(lkp==0){
                                if(gscope==0){
                                    lkp=set_Entry(gscope,yylineno,$1,GLOBALV,10);
                                }else{
                                    lkp=set_Entry(gscope,yylineno,$1,LOCALV,10);
                                }   
                            }
                        }
                       
                        $2=emit_ifTableItem($2);

                        if($2->method==1){
                            expr* t=$1;
                            $1=emit_ifTableItem(member_item(t,$2->name));

                            expr* tmp=t;
                            tmp->next=$2->elist;

                            $2->elist=tmp;
                        }
                        $$=make_call($1,$2->elist,yylineno);
                    }
                    |LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {

                        expr* func=newExpr(programfunc_e);
                        func->sym=$2;
                        $$=make_call(func,$5,yylineno);
                    }
                    ;


callsuffix:         
                    normcall {

                        $$=$1;
                    }
                    |methodcall {

                        $$=$1;
                    }
                    ;


normcall:           
                    LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {

                        $$->elist=$2;
                        $$->method=0;
                        $$->name=NULL;
                    } 
                    ;


methodcall:         
                    DOUBLE_DOT ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {

                        $$->elist=$4;
                        $$->method=1;
                        $$->name=$2;
                    } 
                    ;


elist:              
                    expr exprs {

                        $$=$1;
                        $$->next=$2;
                    } 
                    |{

                        $$=NULL;
                    }
                    ;


exprs:              
                    COMMA expr exprs {

                        $$=$2;
                        $$->next=$3;
                    }
                    | {
                        
                        $$=NULL;
                    }
                    ;


objectdef:          
                    LEFT_BRACKET elist RIGHT_BRACKET{

                        int i;

                        expr* t=newExpr(newtable_e);
                        t->sym=newTemp();
                        
                        emit(tablecreate,t,NULL,NULL,nextQuadLabel(),yylineno);
                        
                        for(i=0;$2;$2=$2->next){
                            emit(tablesetelem,t,newExpr_constNum(i++),$2,nextQuadLabel(),yylineno);
                        }
                        $$=t;
                    }
                    |LEFT_BRACKET indexed RIGHT_BRACKET {

                        expr* t=newExpr(newtable_e);
                        t->sym=newTemp();

                        emit(tablecreate,t,NULL,NULL,nextQuadLabel(),yylineno);

                        while($2){
                            emit(tablesetelem,t,$2->index,$2,nextQuadLabel(),yylineno);
                            $2=$2->next;
                        }
                        $$=t;
                    }
                    ;


indexed:            
                    indexedelem indexedelements {

                        $$=$1;
                        $$->next=$2;
                    }
                    ;
 
indexedelements:
                    COMMA indexedelem indexedelements{

                        $$=$2;
                        $$->next=$3;
                    }
                    | {
                        $$=NULL;
                    }
                    ;
indexedelem:        
                    LEFT_BRACE expr COLON expr RIGHT_BRACE {

                        $$=$4;
                        $$->index=$2;
                    }
                    ;

block:              
                    LEFT_BRACE {

                        gscope++;  
                        if(gscope>=maxScope){
                            maxScope=gscope;
                        }
                        enterScopeSpace();
                        resetFuncLocalsOffset();

                    } stmts RIGHT_BRACE {

                        hide(gscope); 
                        gscope--;

                        $$=currScopeOffset();
                        exitScopeSpace();
                    }
                    ;


funcprefix:         
                    FUNCTION ID{

                        /*check if the function's nam e exists in the symbol table*/
                        symtable* lkp=lookUp_inFunction(gscope,$2);
                        if(lkp){
                            yyerror("Collision with a name of an ID or a FUNCTION");
                        }else{
                            /*insert the function into the symbol table*/
                            lkp=set_Entry(gscope,yylineno,$2,USERFUNC,11);
                            $$=lvalue_expr(lkp);
                            $$->sym->address=nextQuadLabel();

                            emit(jump,newExpr_constNum(0),NULL,NULL,nextQuadLabel(),yylineno);
                            emit(funcstart,$$,NULL,NULL,nextQuadLabel(),yylineno);
                            int curScopOf=currScopeOffset();
                            push_Stack(scopeOffsetStack,curScopOf);
                        }

                        resetFormalArgsOffset();
                            
                    
                    }
                    |FUNCTION {

                        /*create a new function*/
                        char* name=(char*)malloc(sizeof(char*));
                        sprintf(name,"_f%d",funName);
                        name=strndup(name,sizeof(char*));
                        funName++;

                        /*new entry in symbol table*/
                        symtable* lkp;
                        lkp=set_Entry(gscope,yylineno,name,USERFUNC,11);
                        
                        $$=lvalue_expr(lkp);
                        $$->sym->address=nextQuadLabel();

                        emit(funcstart,$$,NULL,NULL,nextQuadLabel(),yylineno);
                        //push_Stack(scopeOffsetStack,currScopeOffset());
                        //resetFormalArgsOffset();
                    } 
                    ;


funcdef:            
                    funcprefix LEFT_PARENTHESIS {

                        gscope++;  
                        if(gscope>=maxScope){
                            maxScope=gscope;
                        }
                        enterScopeSpace();
                    } idlist RIGHT_PARENTHESIS {

                        gscope--;
                        enterScopeSpace();
                        resetFuncLocalsOffset();
                    } 
                    block{

                        inFunction--;
                        exitScopeSpace();

                        int curScopeOf=currScopeOffset();
                        $$->sym->totallocals=curScopeOf;

                        exitScopeSpace();

                        int offset;
                        pop_Stack(scopeOffsetStack);
                        emit(funcend,$1,NULL,NULL,nextQuadLabel(),yylineno);
                        //patch
                    }
                    |funcprefix LEFT_PARENTHESIS {

                        gscope++;  
                        if(gscope>=maxScope){
                            maxScope=gscope;
                        }
                        enterScopeSpace();
                    } idlist RIGHT_PARENTHESIS {

                        gscope--;
                        enterScopeSpace();
                        resetFuncLocalsOffset();
                    } 
                    block {

                        inFunction--;
                        exitScopeSpace();

                        int curScopeOf=currScopeOffset();
                        $$->sym->totallocals=curScopeOf;

                        exitScopeSpace();
                        
                        int offset;
                        pop_Stack(scopeOffsetStack);
                        emit(funcend,$1,NULL,NULL,nextQuadLabel(),yylineno);
                        //patch   
                    }
                    ;


const:              
                    INTEGER {

                        $$=newExpr(constnum_e);
                        $$->numConst=$1;

                    }
                    |STRING {

                        $$=newExpr(conststring_e);
                        $$->strConst=$1;
                    }
                    |NIL {

                        $$=newExpr(nil_e);
                    }
                    |TRUE {

                        $$=newExpr_constBool(1); 
                    }
                    |FALSE{

                        $$=newExpr_constBool(0);  
                    }
                    |REAL_CONSTANT{

                        $$=newExpr(constnum_e);
                        $$->numConst=$1;
                    }
                    ;

idlist:             
                    ID COMMA ID {

                        symtable* id;
                        id=set_Entry(gscope,yylineno,$1,FORMAL,10);
                        id=set_Entry(gscope,yylineno,$3,FORMAL,10);

                    }
                    |ID {

                        symtable* id;
                        id=set_Entry(gscope,yylineno,$1,FORMAL,10);
                    }
                    | {

                    }
                    ;


ifprefix:           
                      IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS {

                        backpatch($3->truelist,nextQuadLabel());

                        emit(if_eq,$3,newExpr_constBool(1),NULL,nextQuadLabel()+2,yylineno);
                        $$=nextQuadLabel();
                        emit(jump,NULL,NULL,NULL,0,yylineno);

                    }
                    ;


elseprefix:         
                    ELSE {

                        $$=nextQuadLabel();
                        emit(jump,NULL,NULL,NULL,0,yylineno);
                    }
                    ;


ifstmt:             
                    ifprefix stmt elseprefix stmt {

                        patchLabel($1,$3+1);
                        patchLabel($3,nextQuadLabel());
                    }
                    |ifprefix stmt {

                        patchLabel($1,nextQuadLabel());
                    }
                    ;


whilestart:         
                    WHILE {

                        inLoop++;
                        $$=nextQuadLabel();
                    }
                    ;

whilecond:          
                    LEFT_PARENTHESIS expr RIGHT_PARENTHESIS {

                        emit(if_eq,$2,newExpr_constBool(1),NULL,nextQuadLabel()+2,yylineno);
                        $$=nextQuadLabel();
                        emit(jump,NULL,NULL,NULL,0,yylineno);
                    }
                    ;

whilestmt:          
                    whilestart whilecond stmt {

                        emit(jump,NULL,NULL,NULL,$1,yylineno);
                        patchLabel($2,nextQuadLabel());
                        patchList($3->breakList,nextQuadLabel());
                        patchList($3->contList,$1);
                        inLoop--;
                    }
                    ;

forprefix:         
                    FOR LEFT_PARENTHESIS N elist SEMICOLON M expr SEMICOLON {
                        
                        $$->test=$6;
                        $$->enter=nextQuadLabel();
                        emit(if_eq,$7,newExpr_constBool(1),NULL,0,yylineno);
                    }

forstmt:            
                    forprefix N elist N RIGHT_PARENTHESIS N stmt N {

                        patchLabel($1->enter,$6+2);
                        patchLabel($2,nextQuadLabel());
                        patchLabel($4,$1->test);
                        patchLabel($8,$2+1);

                        patchList($7->breakList,nextQuadLabel());
                        patchList($7->contList,$2+1);
                    }
                    ;


returnstmt:         
                    RETURN expr SEMICOLON {

                        if(!inFunction){
                            emit(ret,$2,NULL,NULL,nextQuadLabel(),yylineno);
                        }else{
                            yyerror("Return not in a Function");
                        }
                    }
                    RETURN SEMICOLON {

                        if(!inFunction){
                            emit(ret,NULL,NULL,NULL,nextQuadLabel(),yylineno);
                        }else{
                            yyerror("Return not in a Function");
                        }
                    }
                    ;


%%

/*yyerror function*/
int yyerror(char* yaccProvidedMessage){
    fprintf(stderr, "%s at line: %d, before token: %s,  INPUT NOT VALID\n",yaccProvidedMessage,yylineno,yytext);
}

/*main function*/
int main(int argc,char** argv){

    if(argc>1){
        if(!(yyin=fopen(argv[1],"r"))){
            fprintf(stderr,"Cannot read file: %s\n",argv[1]);
          return 1;
        }
    }else {
        yyin=stdin;
    }

    
    Header=hash_Table();                                /*Create a new hash table and store its address to the global pointer Header*/ 

    default_Libs();                                     /*Set at scope 0 all default libraries*/

    FILE *f=fopen("Quads.txt","w");                     /*Create a file named quads.txt to write all the Produced Quads*/
    FILE *q=fopen("BinaryFile.txt","w");
    
    yyparse();                                          /*Begin Parsing*/
    generate();

    /*Intermediate Code*/
    print_Quads(f);                                     /*Print the Quads*/
    printf("\n"); 
    fclose(f);                                          /*Close the file of Quads*/

    /*Binarey Readable*/
    read_IntermediateCode(q);
    printf("\n"); 

    writeTo_Binary();


    FILE *bin=fopen("BinaryFile.abc","rb");

    if(bin==NULL){
        printf("ERROR");
            exit(EXIT_FAILURE);
    }
    
    read_magic_number(bin);
    read_numConst(bin);
    //read_strConst(bin);

    read_targetCode(bin);

    avm_initialize();

    while(!executionFinished){
        execute_cycle();
    }


    fclose(q);       //close readable file
    fclose(bin);    //close bin file                    

    
    /*Free allocated memory*/
    free(scopeList);
    free_HT(Header);                                   

    return 0;
}




