//INCLUDES,DEFINES
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>


#define SIZE            10
#define EXPAND_SIZE     1024
#define CURR_SIZE       (total*sizeof(quad))
#define NEW_SIZE        (EXPAND_SIZE*sizeof(quad) + CURR_SIZE)    



//ENUMERATIONS,STRUCTS,GLOBAL VARIABLES



/*Enum Types*/
typedef enum SymbolType {
    GLOBALV,
    LOCALV,
    FORMAL,
    USERFUNC,
    LIBFUNC
}SymbolType;

/*Enum scopespace type*/
typedef enum scopespace_t {
    programvar,
    functionlocal,
    formalarg
}scopespace_t;

/*Enum symbol_t type*/
typedef enum symbol_t{
    var_s,
    programfunc_s,
    libraryfunc_s
}symbol_t;

/*Enum instructions opcode*/
typedef enum iopcode{
    assign,         add,            sub,
    mul,            divd,           mod,
    uminus,         and,            or,
    not,            if_eq,          if_noteq,
    if_lesseq,      if_greatereq,   if_less,
    if_greater,     call,           param,
    ret,            getretval,      funcstart,
    funcend,        jump,           tablecreate,    
    tablegetelem,   tablesetelem   
}iopcode;

/*Enum expr_t types*/
typedef enum expr_t{
    var_e,
    tableitem_e,

    programfunc_e,
    libraryfunc_e,

    arithexpr_e,
    boolexpr_e,
    assignexpr_e,
    newtable_e,

    constnum_e,
    constbool_e,
    conststring_e,

    nil_e
}expr_t;

/*Stmt_t Struct*/
typedef struct stmt_t{
    int breakList;
    int contList;
}stmt_t;

/*truelist,falselist implementation*/
typedef struct lists{
    int             q_quad;
    struct lists*   next;

}lists;

/*Expression Struct*/
typedef struct expr{
    expr_t                          type;
    stmt_t                          stm_t; 
    int                             numConst;
    char*                           strConst;
    unsigned char                   boolConst;
    unsigned int                    id;           
    lists*                          truelist;
    lists*                          falselist;
    struct SymbolTableEntry*        sym;  
    struct expr*                    index;
    struct expr*                    next;
    struct expr*                    truenext;
    struct expr*                    falsenext;
}expr;

/*Quad Struct*/
typedef struct quad{
    iopcode         op;
    expr*           result;
    expr*           arg1;
    expr*           arg2;
    int             label;
    unsigned int    line;
    unsigned int    taddress;
}quad;

/*Call Struct*/
typedef struct call_function{
    expr*               elist;
    unsigned char       method;
    char*               name;
}call_function;

/*For Loop Struct*/
typedef struct for_loop{
    unsigned int test;
    unsigned int enter;
}for_loop;

/*Symbol Table Struct*/
typedef struct SymbolTableEntry{
    unsigned int                scope;                          /*Scope of symbol */
    unsigned int                line;                           /*Line of symbol (yylineno)*/
    unsigned int                offset;                         /*Offset in scope space */
    unsigned int                address;                        /*address of function*/
    unsigned int                totallocals;                    /*Total Locals of function*/
    char*                       name;                          /*Name of symbol */
    SymbolType                  type;                           /*Type of symbol */
    scopespace_t                space;                          /*Originating scope space */
    int                         isActive;                       /*If the symbol is not hide */                                   
    struct SymbolTableEntry*    next;                          /*Pointer to store the next symbol */
    struct SymbolTableEntry*    nextScope;                     /*Pointer to store the next symbol with the same scope */
}symtable;

/*Hash Table Struct*/
typedef struct HashTable{
    symtable**  entries;                                     /*An array of pointers to symbol table entries*/
    int         max_scope;
}hashtable;

/*Stack Stract*/
typedef struct stack_s{
    int             top;
    unsigned int    capacity;
    int*            array;
}stack_s;

/*scope offset stack*/
stack_s* scopeOffsetStack;

/*Global Pointer to Hash Table*/
hashtable *Header=NULL;

/*A Global Pointer to store all scopes in a list*/
symtable *scopeList=NULL;

/*Array to store quads*/
quad* quads=(quad*)0;


unsigned int    programVarOffset=0;
unsigned int    functionLocalOffset=0;
unsigned int    formalArgOffset=0;
unsigned int    scopeSpaceCounter=1;
unsigned int    total=0;
unsigned int    tempcounter=0;
unsigned int    gscope=0;
unsigned int    currQuad=0;
unsigned int    tmpName=0;



//FUNCTIONS IMPLEMENTATION



/*new node for truelist and falselist*/
lists* makeList(int q){
    lists* node=(lists*)malloc(sizeof(lists));
    node->q_quad=q;
    node->next=NULL;
    
    return node;
}


/*new Truelist or Falselist*/
lists* newList(int q){
    return makeList(q);
}


/*Merge the lists*/
lists* mergeLists(lists* l1,lists* l2){

    if(l1==NULL){
        return l2;
    }
    if(l2=NULL){
        return l1;
    }

    lists* mrg=l1;

    while(mrg->next!=NULL){
        mrg=mrg->next;
    }
    mrg->next=l2;

    return l1;
}


/*BackPatch*/
void backpatch(lists* list, int tlabel) {
    
    /*code*/
    lists* temp=list;
    while(temp){
        quads[temp->q_quad].label=tlabel;
        temp=temp->next;
    }
}


/*Stack Implementation*/
//------------------ //
/*Create a new stack*/
stack_s* create_Stack(unsigned int capacity){

    /*declarations*/
    stack_s* st=(stack_s*)malloc(sizeof(stack_s));

    assert(st);

    /*code*/
    st->capacity=capacity;
    st->top=-1;
    st->array=(int*)malloc(capacity * sizeof(int));

    return st;
}       
 

/*Check if the Stack is full*/
int isFull_Stack(stack_s* st){

    return st->top==st->capacity-1;
}


/*Check if the stack is empty*/
int isEmpty_Stack(stack_s* st){
    
    if(st->top==-1)
        return 1;
    else 
        return -1;
}


/*Return the top element of stack*/
int isTop_Stack(stack_s* st){

    /*code*/
    if(!isEmpty_Stack(st)){
        return st->array[st->top];
    }else{
        printf("Error in Stack!\n");
        exit(0);
    }
}


/*Push to stack*/
void push_Stack(stack_s* st,int toPush){

    /*code*/
    if(!isFull_Stack(st)){
        st->array[++st->top]=toPush;
    }else{
        printf("Memory Overflow!\n");
        exit(0);
    }
}

/*Pop from stack*/
void pop_Stack(struct stack_s* st){

    /*code*/
    if(!isEmpty_Stack(st)){
        st->array[st->top--];
    }else{
        printf("Stack is Empty!\n");
        exit(0);
    }
}

/*Return the size of the Stack*/
int sizeof_Stack(struct stack_s* st){
    
    return st->top+1;
}

/*Expand dynamically array of quads*/
void expand (void) {

    /*code*/
    assert (total == currQuad);

    quad* p = (quad*) malloc(NEW_SIZE);

    if (quads) {
        memcpy(p, quads, CURR_SIZE);
        free(quads);
    }
    quads =p;
    total += EXPAND_SIZE;
}

/*Reset formal arguments offset*/
void resetFormalArgsOffset() {
    
    /*code*/
    formalArgOffset=0;
}

/*Reset function's local offset*/
void resetFuncLocalsOffset(){
    
    /*code*/
    functionLocalOffset=0;
}

/*scopespace_t current Scopespace*/
scopespace_t currScopeSpace(){

    /*code*/
    if(scopeSpaceCounter==1){
        return programvar;
    }else if(scopeSpaceCounter%2==0){
        return formalarg;
    }else{
        return functionlocal;
    }
}

/*Restore current scope offset*/
void restoreCurrScopeOffset(unsigned int n){
    
    /*code*/
    switch(currScopeSpace()){
        case    programvar      :   programVarOffset=n;
                                    break;
        case    functionlocal   :   functionLocalOffset=n;
                                    break;
        case    formalarg       :   formalArgOffset=n;
                                    break;
        default                 :   assert(0);
    }
}

/*Current scope offset function*/
unsigned int currScopeOffset(){

    /*code*/
    switch(currScopeSpace()){
        case    programvar      :    return programVarOffset;
        case    functionlocal   :    return functionLocalOffset;
        case    formalarg       :    return formalArgOffset;
        default                 :   assert(0);
    }
}

/*In current scope offset function*/
void inCurrScopeOffset(){
    
    /*code*/
    switch(currScopeSpace()){
        case    programvar      :   ++programVarOffset;
                                    break;
        case    functionlocal   :   ++functionLocalOffset;
                                    break;
        case    formalarg       :   ++formalArgOffset;
                                    break;
        default                 :   assert(0);
    }
}

/*Enter scpoe space function*/
void enterScopeSpace(){
    
    /*code*/
    ++scopeSpaceCounter;
}

/*Exit scope space function*/
void exitScopeSpace(){
   
    /*code*/
    assert(scopeSpaceCounter>1);
    --scopeSpaceCounter;
}

/*Convert a non-boolean opperand to boolean*/
void convert_NonBooleanToBoolean(expr* toCnvr){

    /*code*/
    switch(toCnvr->type)
    {
    case    constnum_e:
                if(toCnvr->numConst==0){
                    toCnvr->boolConst='0';
                }else{
                    toCnvr->boolConst='1';
                }
                break;

    case    conststring_e:
                if(strcmp(toCnvr->strConst,"")){
                    toCnvr->boolConst='0';
                }else{
                    toCnvr->boolConst='1';
                }
                break;

    case    nil_e:
                toCnvr->boolConst='0';
                break;

    case    programfunc_e:
                toCnvr->boolConst='1';
                break;
            
    case    libraryfunc_e:
                toCnvr->boolConst='1';
                break;

    case    newtable_e:
                toCnvr->boolConst='1';
                break;

    case    tableitem_e:
                toCnvr->boolConst='1';
                break;

    default:
            break;
    }

}

/*Patch Label*/
void patchLabel(unsigned int quadNo, unsigned int label){

    /*code*/
    assert(quadNo<currQuad);

    quads[quadNo].label = label;
}

/*Patch List*/
void patchList(int list,int label){

    /*code*/
    while(list){
        int next=quads[list].label;
        quads[list].label=label;
        list=next;
    }
}

/*Merge List function*/
int mergeList(int l1,int l2){

    /*code*/
    if(!l1){
        return l2;
    }else{
        if(!l2){
            return l1;
        }else{
            int i=l1;
            while(quads[i].label){
                i=quads[i].label;
            }
            quads[i].label=l2;
            return l1;
        }
    }
}

/*Check correct use of arithmetics*/
void check_arith(expr* e,const char* context){

    /*code*/
    if( e->type==constbool_e    ||
        e->type==conststring_e  ||
        e->type==nil_e          ||
        e->type==newtable_e     ||
        e->type==programfunc_e  ||
        e->type==libraryfunc_e  ||
        e->type==boolexpr_e)
            printf("Illegal expr used in %s!\n",context);    
}

/*Make statement function*/
void make_stmt(stmt_t *s){
    
    /*code*/
    s->breakList=s->contList=0;
}

/*Emitate Quad*/
void emit(iopcode op,expr* arg1,expr* arg2,expr* result,unsigned int label,unsigned int line){

    /*code*/
    if(currQuad==total){
        expand();
    }
        
    quad* p=quads+currQuad++;
    p->op=op;
    p->arg1=arg1;
    p->arg2=arg2;
    p->result=result;
    p->label=label;
    p->line=line;

}

/*Resets tempcounter to 0*/
void resetTemp(){
    /*code*/
    tempcounter=0;
}

/*newTempName function*/
char* newTempName(){
    
    /*declarations*/
    char *s=(char*)malloc(sizeof(char*));
    sprintf(s,"_t%d",tmpName);


    s=strdup(s);
    tmpName++;

    return s;
}

/*Insert a new entry*/
void insert_HT(symtable *s,int index){

    /*code*/                                        
    if(Header->entries[index]==NULL){                        /*if pointer points to NULL add it there */
        Header->entries[index]=malloc(sizeof(symtable));     /*allocate memory for a symtable entry */
        Header->entries[index]=s;                            /*set the specific pointer (bucket of hash table) to point to the new entry */
        Header->entries[index]->next=NULL;
       
    }
    else{                                                    /*else if not NULL */
        s->next=Header->entries[index];                      /*add the new entry at the begginng of the hash table list */
        Header->entries[index]=s;
    }    
}

/*Set attributes of inserting element*/
symtable* set_Entry(unsigned int nscope,unsigned int nline,char *nname,SymbolType ntype,int toHash){

    /*declarations*/
    int index;
    symtable *se;                                           /*pointer se(set_entry) which stores the new element*/
    
    /*memory alloc*/
    se=(symtable*)malloc(sizeof(symtable));

    /*assignments*/                                       /*set attributes */
    se->scope=nscope;
    se->line=nline;
    se->name=nname;
    se->type=ntype;
    se->isActive=1;
    se->next=NULL;
    se->nextScope=NULL;

    /*code*/                                              /*if we are here we have to insert the element in bot list and table */
   // insert_List(scopeList,se,nscope);                       /*insert the element at the list */

    index=hash_Function(toHash);                            /*call the hash_Function to access a position */
    insert_HT(se,index);                                   /*insert the new elemente into the hash table */

    return se;
}

/*Returns a hidden variable in the current scope or an available hidden variable with the name newtempname*/
symtable* newTemp(){

    /*declarations*/
    symtable *tmp;

    /*code*/
    char *name=newTempName();
    tmp=set_Entry(gscope,0,name,LOCALV,10);
    tempcounter++;
    
    return tmp;
}

/*New expression insert*/
expr* newExpr(expr_t  t){

    /*declarations*/
    expr* e=(expr*)malloc(sizeof(expr));

    /*code*/
    memset(e,0,sizeof(expr));
    e->type=t;
    e->truelist=NULL;
    e->falselist=NULL;
    return e;
}

/*boolean emit*/
expr *boolEmit(iopcode op,expr* arg1,expr* arg2,expr* result,unsigned int line){
    
    //declarations
    expr *res;
    res = newExpr(boolexpr_e);
    res-> sym = newTemp();
    emit(op,arg1,arg2,res,-1,line); //not correct

    return res;
}

/*next quad label*/
unsigned int nextQuadLabel(){
    /*code*/
    return currQuad;
}

/*Temp name*/
unsigned int isTempName(char* s){
    
    /*code*/
    printf("%s\n",s);
    return *s=='_';
}

/**/
unsigned int isTempExpr(expr* e){
    
    /*code*/
    return e->sym && isTempName(e->sym->name);
}

/*Hashing Function*/
int hash_Function(int key){
    
    /*code*/
    return (key%SIZE);
}

/*Check if the given name is a functiong*/
int checkForFunction(unsigned int bscope,char *bname){

    /*declarations*/
    symtable *cf;

    /*assignments*/
    cf=Header->entries[1];

    /*code*/
    while(cf){
        if(strcmp(cf->name,bname)==0 && cf->isActive==1){                      /*if the given name (bname) is equal to a node in the lsit */
            if(cf->type==4 || cf->type==3){                 /*if the type of the node is function */
                return 1;                                   /*then return that it is a function  */
            }   
        }
        cf=cf->next;
    }

 
    return 0; 
}

/*Look up in inner scope*/
symtable* lookUp_inscope(unsigned int nscope,char *cname){
    
    /*declarations*/
    symtable *lp;
    int i;

    /*assignments */
    i=0;

    /*code*/
    while(i<SIZE){                              /*trace the whole hash table */
        lp=Header->entries[i];
        if(lp){
            while(lp){
                if(lp->scope==nscope && lp->isActive==1){
                    if(strcmp(lp->name,cname)==0 && lp->type!=4){
                        //ok found
                        //printf("OK, %s found and refers to: #%d,\"%s\"(%d) \n",cname,lp->line,lp->name,lp->scope);
                        return lp;
                    }
                }
                lp=lp->next;
            }
        }
        i++;
    }

    
    
    /*else if not found return NULL*/
    return NULL;
}

/*Look up for function*/
symtable* lookUp_inFunction(unsigned int lscope,char *lname){

    /*declarations*/
    symtable *lf;
    int i;

    /*code*/
    i=0;

    while(i<SIZE){
        lf=Header->entries[i];
        if(lf){
            while(lf){
                if(lf->scope==lscope && lf->isActive==1){
                    if(strcmp(lf->name,lname)==0){
                        return lf; // function's name found 
                    }
                }
                lf=lf->next;
            }
        }
        i++;
    }
    return NULL;
}

/*Look up for namespace*/
int lookUp_namespace(unsigned int nscope,char *sname){

    /*declarations*/
    symtable *np;
    int cnt,sz,i;

    /*assignments*/
    sz=0;
    i=0;

    /*code*/
   while(sz<SIZE){
    np=Header->entries[i];
        if(np){
            while(np){
                    if(strcmp(np->name,sname)==0 && np->scope==nscope){
                        //ok found
                        //printf("OK, %s found and refers to: #%d,\"%s\"(%d) \n",sname,np->line,np->name,np->scope);
                        return 1;
                    }
                
                np=np->next;
            }
        }
    sz++;
   }

    return 0;
}

/*Check collisions*/
int check_collisions(char *cname){
    
    /*declarations*/
    symtable *cc;

    /*assignments */
    cc=Header->entries[1];

    /*code*/
    while(cc){
        if(strcmp(cc->name,cname)==0 && cc->isActive==1){
            if(cc->type==4){
                return 1;
            }
        }
        cc=cc->next;
    }


    return 0;  
}

/*Expression lvalue*/
expr* lvalue_expr(symtable* sym){
    
    /*assertions*/
    assert(sym);

    /*memory allocation*/
    expr* e=(expr*)malloc(sizeof(expr));

    /*code*/
    memset(e,0,sizeof(expr));

    e->next=(expr*)0;
    e->sym=sym;

    switch(sym->type){
        case    var_s           :       e->type=var_e;
                                        break;
        case    programfunc_s   :       e->type=programfunc_e;
                                        break;
        case    libraryfunc_s   :       e->type=libraryfunc_e;
                                        break;
    }

    return e;

}

/*Emit if table function*/
expr* emit_ifTableItem(expr* e){

    /*code*/
    if(e->type!=tableitem_e){
        return e;
    }else{
        expr* result=newExpr(var_e);
        result->sym=newTemp();
        emit(tablegetelem,e,e->index,result,NULL,NULL);

        return result;
    }
}

/*Expression make call*/
expr* make_call(expr* lv,expr* reserved_elist, unsigned int yyline){

    /*declarations*/
    expr* func=emit_ifTableItem(lv);

    /*code*/
    while(reserved_elist){
        emit(param,reserved_elist,NULL,NULL,nextQuadLabel(),yyline);
        reserved_elist=reserved_elist->next;
    }
    emit(call,func,NULL,NULL,nextQuadLabel(),yyline);
    expr* result= newExpr(var_e);
    result->sym=newTemp();
    emit(getretval,result,NULL,NULL,nextQuadLabel(),yyline);
    
    return result;
}

/*Function that returns an expression with a const bool*/
expr* newExpr_constBool(unsigned int b){

    /*declarations*/
    expr* e=newExpr(constbool_e);

    /*code*/
    e->boolConst=!!b;

    return e;
}

/*Function that returns an expression with a const str*/
expr* newExpr_constString(char* s){

    /*declarations*/
    expr* e=newExpr(conststring_e);

    /*code*/
    e->strConst=strdup(s);

    return e;
}

/*Function that return an expression with a const num*/
expr* newExpr_constNum(double i){
    
    /*code*/
    expr* e=newExpr(constnum_e);
    e->numConst=i;

    return e;
}

/*Expression member item*/
expr* member_item(expr* lv,char* name){

    lv=emit_ifTableItem(lv);            // emit code if r-value use of table item
    expr* ti=newExpr(tableitem_e);      // make a new expression
    ti->sym=lv->sym;
    ti->index=newExpr_constString(name);  // const string index
    return ti;
}

/*Create a new hash table*/
hashtable* hash_Table(){

        /*declarations*/
        int i;
        hashtable *nt;

        /*memory alloc*/                             
        nt=(hashtable*)malloc(sizeof(hashtable));
        nt->entries=(hashtable*)malloc(sizeof(symtable)*SIZE);
        nt->max_scope=-1;

        /*code*/
        for(i=0;i<SIZE;i++){
            nt->entries[i]=NULL;                            /*initialise all buckets to point NULL*/
        }
        return nt;
}

/*Lookup inGlobal*/
symtable* lookUp_inglobal(char *cname){
    
    /*declarations*/
    int scope;
    symtable *p;
   

    /*code*/
    while(scope>=0){   
        p=Header->entries[0];                                                                                           /*while the scope is not equal trace the first bucket of hash table which contains all id's */
        if(p){
            while(p){
                if(strcmp(p->name,cname)==0 && p->isActive==1){                                                         /*if the given name is equal to an entry name it means that you have found globally this symbol */
                        //printf("OK,%s found and refers to: #%d,\"%s\"(%d) \n",cname,p->line,p->name,p->scope);
                        return p;                                                                                       /*return that you found it and print in which symbol refers */
                }
                p=p->next;
            }
        }
        scope--;
    }


    return NULL;

}

/*Insert default libraries at scope 0*/
void default_Libs(){
    
    /*code*/
    set_Entry(0,0,"print",LIBFUNC,11);
    set_Entry(0,0,"input",LIBFUNC,11);
    set_Entry(0,0,"objectmemberkeys",LIBFUNC,11);
    set_Entry(0,0,"objjecttotalmembers",LIBFUNC,11);
    set_Entry(0,0,"objectcopy",LIBFUNC,11);
    set_Entry(0,0,"totalarguments",LIBFUNC,11);
    set_Entry(0,0,"argument",LIBFUNC,11);
    set_Entry(0,0,"typeof",LIBFUNC,11);
    set_Entry(0,0,"strtonum",LIBFUNC,11);
    set_Entry(0,0,"sqrt",LIBFUNC,11);
    set_Entry(0,0,"cos",LIBFUNC,11);
    set_Entry(0,0,"sin",LIBFUNC,11);
}

/*hide symbols*/
void hide(unsigned int hscope){

    /*declarations*/
    symtable *hd;
    int i=0;

    /*code*/
    while(i<SIZE){
        hd=Header->entries[i];
        if(hd){
            while(hd){
                if(hd->scope==hscope){
                    hd->isActive=0;
                }
                hd=hd->next;
            }
        }
        i++;
    }
}

/*Print Quads*/
void print_Quads(FILE* f){	

    int i;
    i=0;

    fprintf(f,"\n\t\t\t----- Intermediate Code ----\t\t\t\t\n\n\n");
    fprintf(f,"quad#\t\topcode\t\tresult\t\targ1\t\targ2\t\tlabel\n");
    fprintf(f,"-------------------------------------------------------------------------------------\n");

        while(i<nextQuadLabel()){

            switch((quads+i)->op){

                case assign:            
                                          
                                        if(((quads+i)->arg1->type)==constnum_e){
                                            fprintf(f,"%d:\t\tassign\t\t%s\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst);
                                        }else if(((quads+i)->arg1->type)==conststring_e){
                                           fprintf(f,"%d:\t\tassign\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->strConst);
                                        }else if(((quads+i)->arg1->type)==constbool_e){
                                            if(((quads+i)->arg1->boolConst)==1){
                                                fprintf(f,"%d:\t\tassign\t\t%s\t\t'true'",i+1,(quads+i)->result->sym->name);
                                            }else{
                                                fprintf(f,"%d:\t\tassign\t\t%s\t\t'false'",i+1,(quads+i)->result->sym->name);
                                            }                                            
                                        }else if(((quads+i)->arg1->type)==nil_e){
                                            fprintf(f,"%d:\t\tassign\t\t%s\t\tNULL",i+1,(quads+i)->result->sym->name);
                                        }else if(((quads+i)->arg1->type)==var_e){
                                            fprintf(f,"%d:\t\tassign\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name);
                                        }else{
                                            fprintf(f,"%d:\t\tassign\t\t%s\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->boolConst);
                                        }
                                        
                                        break;

            case add:                   
                                        
                                        if(((quads+i)->arg1->type)==constnum_e && ((quads+i)->arg2->type)==constnum_e){
                                           fprintf(f,"%d:\t\tadd\t\t%s\t\t%d\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst);
                                        }else if(((quads+i)->arg1->type)==constnum_e){
                                           fprintf(f,"%d:\t\tadd\t\t%s\t\t%d\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->arg2->sym->name);
                                        }else if(((quads+i)->arg2->type)==constnum_e){
                                           fprintf(f,"%d:\t\tadd\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst);
                                        }else{
                                           fprintf(f,"%d:\t\tadd\t\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                        }
                                        
                                        break;

            case sub:                   
                                        if(((quads+i)->arg1->type)==constnum_e && ((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tsub\t\t%s\t\t\t%d\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst);
                                        }else if(((quads+i)->arg1->type)==constnum_e){
                                            fprintf(f,"%d:\t\tsub\t\t%s\t\t%d\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->arg2->strConst);
                                        }else if(((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tsub\t\t%s\t\t\t%s\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->strConst,(quads+i)->arg2->numConst);
                                        }else{
                                            fprintf(f,"%d:\t\tsub\t\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                        }
                                        break;

            case mul:                   
                                        if(((quads+i)->arg1->type)==constnum_e && ((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tmul\t\t%s\t\t%d\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst);
                                        }else if(((quads+i)->arg1->type)==constnum_e){
                                            fprintf(f,"%d:\t\tmul\t\t%s\t\t%d\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->arg2->sym->name);
                                        }else if(((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tmul\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst);
                                        }else{
                                            fprintf(f,"%d:\t\tmul\t\t%s\t\t%s\t\t%s",(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                        }
                                        break;

            case divd:                  
                                        if(((quads+i)->arg1->type)==constnum_e && ((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tdiv\t\t%s\t\t%d\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst);
                                        }else if(((quads+i)->arg1->type)==constnum_e){
                                            fprintf(f,"%d:\t\tdiv\t\t%s\t\t%d\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->arg2->sym->name);
                                        }else if(((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tdiv\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst);
                                        }else{
                                            fprintf(f,"%d:\t\tdiv\t\t%s\t\t%s\t\t%s",(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                        }
                                        break;

            case mod:                   
                                        if(((quads+i)->arg1->type)==constnum_e && ((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tmod\t\t%s\t\t%d\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst);
                                        }else if(((quads+i)->arg1)==constnum_e){
                                            fprintf(f,"%d:\t\tmod\t\t%s\t\t%d\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->arg2->sym->name);
                                        }else if(((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tmod\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst);
                                        }else{
                                            fprintf(f,"%d:\t\tmod\t\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                        }
                                        break;

            case uminus:                
                                        fprintf(f,"%d:\t\tuminus\t\t%s\t\t%s",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg1->sym->name);
                                        break;

            case and:                   
                                        if(((quads+i)->arg1->type)==arithexpr_e){
                                            fprintf(f,"%d:\t\tand\t\t%s\t\t%d\t\t\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->label);
                                        }
                                        break;

            case or:                    
                                        if(((quads+i)->arg1->type)==constnum_e){
                                            fprintf(f,"%d:\t\tor\t\t%s\t\t%d\t\t\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->numConst,(quads+i)->label);
                                        }
                                        break;

            case not:                   
                                        break;

            case if_eq:                 
                                        switch((quads+i)->arg1->type){
                                            case constnum_e:
                                                                    if(((quads+i)->arg2->type)==constnum_e){
                                                                        fprintf(f,"%d:\t\tif_eq\t\t\t\t%d\t\t%d\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst,(quads+i)->label);
                                                                    }else if(((quads+i)->arg2->type)==constbool_e){
                                                                        if(((quads+i)->arg2->boolConst==1)){
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t%d\t\t'true'\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->label);
                                                                        }else{
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t%d\t\t'false'\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->label);
                                                                        }
                                                                    }else if(((quads+i)->arg2->type)==conststring_e){
                                                                        fprintf(f,"%d:\t\tif_eq\t\t\t\t%d\t\t%s\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->strConst,(quads+i)->label);
                                                                    }else if(((quads+i)->arg2->type)==var_e){
                                                                        if(((quads+1)->arg1->numConst)==0){
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t'true'\t\t%s\t\t%d",i+1,(quads+i)->arg2->sym->name,(quads+i)->label);                                                                        
                                                                        }else{
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t'false'\t\t%s\t\t%d",i+1,(quads+i)->arg2->sym->name,(quads+i)->label);
                                                                        }
                                                                        
                                                                    }else {
                                                                        break;
                                                                    }
                                                                    break;
                                            case constbool_e:   
                                                                    if(((quads+i)->arg2->type)==constnum_e){
                                                                        if(((quads+i)->arg1->boolConst)==1){
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t'true'\t\t%d\t\t%d",i+1,(quads+i)->arg2->numConst,(quads+i)->label);
                                                                        }else{
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t'false'\t\t%d\t\t%d",i+1,(quads+i)->arg2->numConst,(quads+i)->label);
                                                                        }                                                      
                                                                    }else if(((quads+i)->arg2->type)==constbool_e){
                                                                        if(((quads+i)->arg1->boolConst)==1){
                                                                            if(((quads+i)->arg2->boolConst==1)){
                                                                                fprintf(f,"%d:\t\tif_eq\t\t\t\t'true'\t\t'true'\t\t%d",i+1,(quads+i)->label);
                                                                            }else{
                                                                                fprintf(f,"%d:\t\tif_eq\t\t\t\t'true'\t\t'false'\t\t%d",i+1,(quads+i)->label);
                                                                            }                                                                           
                                                                        }else{
                                                                            if(((quads+i)->arg2->boolConst==1)){
                                                                                fprintf(f,"%d:\t\tif_eq\t\t\t\t'false'\t\t'true'\t\t%d",i+1,(quads+i)->label);
                                                                            }else{
                                                                                fprintf(f,"%d:\t\tif_eq\t\t\t\t'false'\t\t'false'\t\t%d",i+1,(quads+i)->label);
                                                                            }
                                                                        }
                                                                    }else if(((quads+i)->arg2->type)==conststring_e){
                                                                        if(((quads+i)->arg1->boolConst)==1){
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t%'true'\t\t%s\t\t%d",i+1,(quads+i)->arg2->strConst,(quads+i)->label);
                                                                        }else{
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t%'false'\t\t%s\t\t%d",i+1,(quads+i)->arg2->strConst,(quads+i)->label);
                                                                        }                                                                       
                                                                    }else if(((quads+i)->arg2->type)==var_e){
                                                                        if(((quads+i)->arg1->boolConst)==1){
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t'true'\t\t%s\t\t%d",i+1,(quads+i)->arg2->sym->name,(quads+i)->label);
                                                                        }else{
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t'false'\t\t%s\t\t%d",i+1,(quads+i)->arg2->sym->name,(quads+i)->label);
                                                                        }                                                                       
                                                                    }else {
                                                                        break;
                                                                    }
                                                                    break;
                                            case conststring_e: 
                                                                    if(((quads+i)->arg2->type)==constnum_e){
                                                                        fprintf(f,"%d:\t\tif_eq\t\t\t\t%s\t\t%d\t\t%d",i+1,(quads+i)->arg1->strConst,(quads+i)->arg2->numConst,(quads+i)->label);
                                                                    }else if(((quads+i)->arg2->type)==constbool_e){
                                                                        if(((quads+i)->arg2->boolConst)==1){
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t%s\t\t'true'\t\t%d",i+1,(quads+i)->arg1->strConst,(quads+i)->label);
                                                                        }else{
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t%s\t\t'false'\t\t%d",i+1,(quads+i)->arg1->strConst,(quads+i)->label);
                                                                        }
                                                                    }else if(((quads+i)->arg2->type)==conststring_e){
                                                                        fprintf(f,"%d:\t\tif_eq\t\t\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->arg1->strConst,(quads+i)->arg2->strConst,(quads+i)->label);
                                                                    }else if(((quads+i)->arg2->type)==var_e){
                                                                        if(((quads+i)->arg2->boolConst)==1){
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t%s\t\t'true'\t\t%d",i+1,(quads+i)->arg1->strConst,(quads+i)->label);
                                                                        }else{
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t%s\t\t'false'\t\t%d",i+1,(quads+i)->arg1->strConst,(quads+i)->label);
                                                                        }
                                                                    }else {
                                                                        break;
                                                                    }
                                                                    break;
                                            case var_e:
                                                                    if(((quads+i)->arg2->type)==constnum_e){
                                                                        fprintf(f,"%d:\t\tif_eq\t\t\t\t%d\t\t%d\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst,(quads+i)->label);
                                                                    }else if(((quads+i)->arg2->type)==constbool_e){
                                                                        if(((quads+i)->arg2->boolConst)==1){
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t%s\t\t'true'\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->label);
                                                                        }else{
                                                                            fprintf(f,"%d:\t\tif_eq\t\t\t\t%s\t\t'false'\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->label);
                                                                        }                                                                  
                                                                    }else if(((quads+i)->arg2->type)==conststring_e){
                                                                        fprintf(f,"%d:\t\tif_eq\t\t\t\t%d\t\t%s\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->strConst,(quads+i)->label);
                                                                    }else if(((quads+i)->arg2->type)==var_e){
                                                                        fprintf(f,"%d:\t\tif_eq\t\t\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name,(quads+i)->label);
                                                                    }else {
                                                                        break;
                                                                    }
                                                                    break;
                                            default:
                                                                    if((quads+i)->arg2->sym){
                                                                        fprintf(f,"%d\t\tif-eq\t\t\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name,(quads+i)->label);
                                                                    }else{
                                                                        if(!(quads+i)){
                                                                            fprintf(f,"%d\t\tif-eq\t\t\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name,(quads+i)->label);
                                                                        }else{
                                                                            fprintf(f,"error in memory/should printf if eq\n");
                                                                            break;
                                                                        }
                                                                    }
                                                                    break;
                                        }
                                        break;

            case if_noteq:              
                                        if(((quads+i)->arg1->type)==constnum_e && ((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tif_noteq\t\t\t\t%d\t\t%d\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst,(quads+i)->label);
                                        }else if(((quads+i)->arg1)==constnum_e){
                                            fprintf(f,"%d:\t\tif_noteq\t\t\t\t%d\t\t%s\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->sym->name,(quads+i)->label);
                                        }else if(((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tif_noteq\t\t\t\t%s\t\t%\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst,(quads+i)->label);
                                        }else{
                                            fprintf(f,"%d:\t\tif_noteq\t\t\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name,(quads+i)->label);
                                        }
                                        break;

            case if_lesseq:             
                                        if(((quads+i)->arg1->type)==constnum_e && ((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tif_lesseq\t\t\t\t%d\t\t%d\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst,(quads+i)->label);
                                        }else if(((quads+i)->arg1)==constnum_e){
                                            fprintf(f,"%d:\t\tif_lesseq\t\t\t\t%d\t\t%s\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->sym->name,(quads+i)->label);
                                        }else if(((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tif_lesseq\t\t\t\t%s\t\t\t%\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst,(quads+i)->label);
                                        }else{
                                            fprintf(f,"%d:\t\tif_lesseq\t\t\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name,(quads+i)->label);
                                        }        
                                        break;

            case if_greatereq:          
                                        if(((quads+i)->arg1->type)==constnum_e && ((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tif_greatereq\t\t\t\t%d\t\t%d\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst,(quads+i)->label);
                                        }else if((quads+i)->arg1==constnum_e){
                                            fprintf(f,"%d:\t\tif_greatereq\t\t\t\t%d\t\t%s\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->sym->name,(quads+i)->label);
                                        }else if(((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tif_greatereq\t\t\t\t%s\t\t\t%\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst,(quads+i)->label);
                                        }else{
                                            fprintf(f,"%d:\t\tif_greatereq\t\t\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name,(quads+i)->label);
                                        }                                              
                                        break;

            case if_greater:            
                                        if((quads+i)->arg1->type==constnum_e && ((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tif_greater\t\t\t\t%d\t\t%d\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst,(quads+i)->label);
                                        }else if((quads+i)->arg1==constnum_e){
                                            fprintf(f,"%d:\t\tif_greater\t\t\t\t%d\t\t%s\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->sym->name,(quads+i)->label);
                                        }else if(((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tif_greater\t\t\t\t%s\t\t%\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst,(quads+i)->label);
                                        }else{
                                            fprintf(f,"%d:\t\tif_greater\t\t\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name,(quads+i)->label);
                                        }
                                        break;

            case if_less:               
                                        if(((quads+i)->arg1->type)==constnum_e && ((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tif_less\t\t\t\t%d\t\t%d\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->numConst,(quads+i)->label);
                                        }else if(((quads+i)->arg1)==constnum_e){
                                            fprintf(f,"%d:\t\tif_less\t\t\t\t%d\t\t%s\t\t%d",i+1,(quads+i)->arg1->numConst,(quads+i)->arg2->sym->name,(quads+i)->label);
                                        }else if(((quads+i)->arg2->type)==constnum_e){
                                            fprintf(f,"%d:\t\tif_less\t\t\t\t%s\t\t%\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst,(quads+i)->label);
                                        }else{
                                            fprintf(f,"%d:\t\tif_less\t\t\t\t%s\t\t%s\t\t%d",i+1,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name,(quads+i)->label);
                                        }
                                        break;

            case call:                  
                                        fprintf(f,"%d:\t\tcall\t\t\t\t%s",i+1,(quads+i)->arg1->sym->name);
                                        break;

            case param:                 
                                        if(((quads+i)->arg1->type)==constnum_e){
                                            fprintf(f,"%d\t\tparam\t\t\t\t%d",i+1,(quads+i)->arg1->numConst);
                                        }else if(((quads+i)->arg1->type)==conststring_e){
                                            fprintf(f,"%d\t\tparam\t\t\t\t%s",i+1,(quads+i)->arg1->strConst);
                                        }else if(((quads+i)->arg1->type)==constbool_e){
                                            if(((quads+i)->arg1->boolConst)==1){
                                                fprintf(f,"%d\t\tparam\t\t\t\t'true'",i+1);
                                            }else{
                                                fprintf(f,"%d\t\tparam\t\t\t\t'false'",i+1);
                                            }                                            
                                        }else if(((quads+i)->arg1->type)==nil_e){
                                            fprintf(f,"%d\t\tparam\t\t\t\tNULL",i);
                                        }else{
                                            fprintf(f,"%d\t\tparam\t\t\t\t%s",i+1,(quads+i)->arg1->sym->name);
                                        }
                                        break;

            case ret:                   
                                        if(!(quads+i)){
                                            fprintf(f,"%d\t\tret",i);
                                        }else{
                                            if(((quads+i)->arg1->type)==constnum_e){
                                                fprintf(f,"%d\t\tret\t\t\t\t%d",i+1,(quads+i)->arg1->numConst);
                                            }else if(((quads+i)->arg1->type)==conststring_e){
                                                fprintf(f,"%d\t\tret\t\t\t\t%s",i+1,(quads+i)->arg1->strConst);
                                            }else if(((quads+i)->arg1->type)==constbool_e){
                                                if(((quads+i)->arg1->boolConst)==1){
                                                    fprintf(f,"%d\t\tret\t\t\t\t'true'",i+1);
                                                }else{
                                                    fprintf(f,"%d\t\tret\t\t\t\t'false'",i+1);
                                                }
                                            }else if(((quads+i)->arg1->type)==nil_e){
                                                fprintf(f,"%d\t\tret\t\t\t\tNULL",i);
                                            }else{
                                                fprintf(f,"%d\t\tret\t\t\t\t%s",i+1,(quads+i)->arg1->sym->name);
                                            }                          
                                        }
                                        break;

            case getretval:             
                                        fprintf(f,"%d:\t\tgetretval\t\t\t\t%s",i+1,(quads+i)->arg1->sym->name);
                                        break;

            case funcstart:             
                                        if((quads+i)==NULL){
                                            expand();
                                            fprintf(f,"%d:\t\tfuncstart\t%s",i+1,(quads+i)->result->sym->name);
                                        }else{
                                
                                            fprintf(f,"%d:\t\tfuncstart\tfuname(segfault_in_print)",i+1);
                                        }
                                        break;

            case funcend:               
                                        fprintf(f,"%d:\t\tfuncend\t\t\t\t%s",i+1,(quads+i)->arg1->sym->name);
                                        break;

            case jump:                  
                                        fprintf(f,"%d:\t\tjump\t\t\t\t\t\t\t\t%d",i+1,(quads+i)->label);
                                        break;

            case tablecreate:           
                                        fprintf(f,"%d:\t\ttablecreate\t\t\t\t%s",i+1,(quads+i)->arg1->sym->name);
                                        break;

            case tablegetelem:          
                                        if(((quads+i)->arg2->type)==var_e){
                                            fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                        }
                                        break;

            case tablesetelem:          
                                        switch((quads+i)->result->type){
                                            case var_e:
                                                                    if(((quads+i)->arg2->type)==constnum_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst);
                                                                    }else if(((quads+i)->arg2->type)==conststring_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->strConst);
                                                                    }else if(((quads+i)->arg2->type)==constbool_e){
                                                                        if(((quads+i)->arg2->boolConst)==1){
                                                                            fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t'true'",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name);
                                                                        }else{
                                                                            fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t'false'",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name);
                                                                        }                                                
                                                                    }else if(((quads+i)->arg2->type)==var_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                                                    }else{
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                                                    }
                                                                    break;
                                            case constnum_e:
                                                                    if(((quads+i)->arg2->type)==constnum_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst);
                                                                    }else if(((quads+i)->arg2->type)==conststring_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->strConst);
                                                                    }else if(((quads+i)->arg2->type)==constbool_e){
                                                                        if(((quads+i)->arg2->boolConst)==1){
                                                                            fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t'true'",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name);
                                                                        }else{
                                                                            fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t'false'",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name);
                                                                        }                                                                      
                                                                    }else if(((quads+i)->arg2->type)==var_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                                                    }else{
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                                                    }
                                                                    break;
                                            case conststring_e:
                                                                    if(((quads+i)->arg2->type)==constnum_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst);
                                                                    }else if(((quads+i)->arg2->type)==conststring_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->strConst);
                                                                    }else if(((quads+i)->arg2->type)==constbool_e){
                                                                        if(((quads+i)->arg2->boolConst)==1){
                                                                            fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t'true'",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name);
                                                                        }else{
                                                                           fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t'false'",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name); 
                                                                        }                                                                       
                                                                    }else if(((quads+i)->arg2->type)==var_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                                                    }else{
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                                                    }                         
                                                                    break;
                                            case constbool_e:
                                                                    if(((quads+i)->arg2->type)==constnum_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%d",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->numConst);
                                                                    }else if(((quads+i)->arg2->type)==conststring_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->strConst);
                                                                    }else if(((quads+i)->arg2->type)==constbool_e){
                                                                        if(((quads+i)->arg2->boolConst)==1){
                                                                            fprintf(f,"%d:\t\ttablesetelem\t\t%s\t\t%s\t\t'true'",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name);
                                                                        }else{
                                                                            fprintf(f,"%d:\t\ttablesetelem\t\t%s\t\t%s\t\t'false'",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name);
                                                                        }                                                                        
                                                                    }else if(((quads+i)->arg2->type)==var_e){
                                                                        fprintf(f,"%d:\t\ttablesetelem\t\t%s\t\t%s\t\t%s",(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                                                    }else{
                                                                        fprintf(f,"%d:\t\ttablesetelem\t%s\t\t%s\t\t%s",i+1,(quads+i)->result->sym->name,(quads+i)->arg1->sym->name,(quads+i)->arg2->sym->name);
                                                                    }
                                                                    break;
                                            default:
                                                                    break;
                                        }
                                        break;

            default:                    
                                        break;


        }



        fprintf(f,"\n");
        i++;      
    }
}

/*Free list*/
void free_L(symtable *fr){
    
    free(fr);
}

/*Free table*/
void free_HT(hashtable *fr){

    free(fr);
}
