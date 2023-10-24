//DEFINES
#define AVM_STACKSIZE           4096
#define AVM_TABLE_HASHSIZE      211
#define AVM_WIPEOUT(m)         memset(&(m),0,sizeof(m))
#define AVM_STACKENV_SIZE        4
#define AVM_ENDING_PC           codeSize
#define AVM_MAX_INSTRUCTIONS    24
#define AVM_NUMACTUAL_OFFSET+4
#define LIBFUNC_SIZE            11
#define AVM_NUMACTUALS_OFFSET+4
#define AVM_SAVEDPC_OFFSET+3
#define AVM_SAVEDTOP_OFFSET+2
#define AVM_SAVEDTOPSP_OFFSET+1
#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic
#define execute_jle execute_relational
#define execute_jlt execute_relational
#define execute_jgt execute_relational
#define execute_jge execute_relational

//STRUCTS,ENUMS,GLOBAL DECLARATIONS



unsigned int    executionFinished=0;
unsigned int    pc_counter=0;
unsigned int    currLine=0;
unsigned int    codeSize=0;
unsigned int    top;
unsigned int    topsp;
unsigned int    totalActuals=0;
instruction*    code=(instruction*)0;
instruction*    myInstructions;
userfunc*       ua=(userfunc*)0;
double*         myNums;
char**          myStrings;
char**          myLibFuncs;





/*AVM Memcells Types Struct*/
typedef enum avm_memcell_t {
    number_m    =0,
    string_m    =1,
    bool_m      =2,
    table_m     =3,
    userfunc_m  =4,
    libfunc_m   =5,
    nil_m       =6,
    undef_m     =7
}avm_memcell_t;


/*AVM Table Bucket struct*/
typedef struct avm_table_bucket {
    avm_memcell_t         key;
    avm_memcell_t         value;
    struct avm_table_bucket*   next;
}avm_table_bucket;


/*AVM Table Struct*/
typedef struct avm_table {
    unsigned            refCounter;
    avm_table_bucket*   strIndexed[AVM_TABLE_HASHSIZE];
    avm_table_bucket*   numIndexed[AVM_TABLE_HASHSIZE];
    unsigned            total;
}avm_table;


/*AVM Memcell Struct*/
typedef struct avm_memcell {
    avm_memcell_t type;
    union {
        double          numVal;
        char*           strVal;
        unsigned char   boolVal;
        avm_table*      tableVal;
        unsigned        funcVal;
        char*           libfuncVal;
    }data;
}avm_memcell;

/*AVM's memecell stack*/
avm_memcell stack[AVM_STACKSIZE];
avm_memcell ax,bx,cx,retval;



typedef void            (*execute_func_t)(instruction*);
typedef void            (*library_func_t)(void);
typedef void            (*memclear_funct_t)(avm_memcell*);
typedef char*           (*tostring_func_t)(avm_memcell* m);
typedef double          (*arithmetic_func_t)(double x,double y);
typedef unsigned char   (*tobool_func_t)(avm_memcell*);

// FUNCTIONS IMPLEMENTATION



void avm_error(char* msg,char* id){
    
    printf("ERROR:%s\tof %s\n",msg,id);
}


/*Get Table's element*/
avm_memcell* avm_table_get_elem (avm_memcell* key); 


/*get enviromental value*/
double avm_get_envalue(unsigned int i){

    return (stack[i].data.numVal);
}


/*Total Avm Actuals*/
unsigned int avm_total_actuals(void){
    return avm_get_envalue(topsp+AVM_NUMACTUALS_OFFSET);
}


/*Avm Get Actuals*/
avm_memcell* avm_getactual(unsigned int i){

    assert(i<avm_total_actuals());
    return (&stack[topsp+AVM_STACKENV_SIZE+1+i]);
}


/*get number*/
double consts_getnumber(unsigned int n) {
    
    return myNums[n];
}


/*get string*/
char* consts_getstring(unsigned int n) {
    
    return myStrings[n];
}


/*get lib funcs*/
char* libfuncs_getused(unsigned int n) {
    
    return myLibFuncs[n];   
}


/*This function is called by the functions of Virtual Machine*/
avm_memcell* avm_translate_operand(vmarg* arg,avm_memcell* reg){

    if(arg){

        switch(arg->type){
            case global_a:          return &stack[AVM_STACKSIZE-1-(arg->val)];
            case local_a:           return &stack[topsp-(arg->val)];
            case formal_a:          return &stack[topsp+AVM_STACKENV_SIZE+1+(arg->val)];
            case retval_a:          return &retval;
            case number_a:      {
                                    reg->type=number_m;
                                    reg->data.numVal=consts_getnumber(arg->val);
                                    return reg;
                            }
            case string_a:      {
                                    reg->type=string_m;
                                    reg->data.strVal=strdup(consts_getstring(arg->val));
                                    return reg;
                            }
            case bool_a:        {
                                    reg->type=bool_m;
                                    reg->data.boolVal=arg->val;
                                    return reg;
                            }
            case nil_a:         {
                                    reg->type=nil_m;
                                    return reg;
                            }

            case userfunc_a:    {
                                    reg->type=userfunc_m;
                                    /*if function address is directly stored*/
                                    reg->data.funcVal=arg->val;
                                    /*if function index in func table is stored*/
                                    reg->data.funcVal=(arg->val);
                                    return reg;
                            }  
            case libfunc_a:     {
                                    reg->type=libfunc_m;
                                    reg->data.libfuncVal=libfuncs_getused(arg->val);
                                    return reg;
                            }
            default:                break;
                         

        }
    }
    
}


/*number tostring*/
char* number_tostring(avm_memcell* m){

    char* s=(char*)malloc(sizeof(char));
    sprintf(s,"%f", m->data.numVal);
    return s;
}


/*string tostring*/
char* string_tostring(avm_memcell* m){

    return (m->data.strVal);
}


/*bool tostring*/
char* bool_tostring(avm_memcell* m){

    switch(m->data.boolVal){
        case 1:
                        return "TRUE";
                        break;
        case 0:     
                        return "FALSE";
                        break;
        default:     
                        return "NULL";
                        break;
    }
 
}


/*table tostring*/
char* table_tostring(avm_memcell* m){

    char* s=(char*)malloc(sizeof(char));
    sprintf(s,"%u",ua[m->data.funcVal].address);
    
    return s;
}


/*userfunc tostring*/
char* userfunc_tostring(avm_memcell* m){

}


/*libfunc tostring*/
char* libfunc_tostring(avm_memcell* m){

    return m->data.libfuncVal;
}


/*nil tostring*/
char* nil_tostring(avm_memcell* m){

    return "nil";
}


/*undef tostring*/
char* undef_tostring(avm_memcell* m){

    return "undef";
}


/*dispatch*/
tostring_func_t tostringFuncs[]={

    number_tostring,
    string_tostring,
    bool_tostring,
    table_tostring,
    userfunc_tostring,
    libfunc_tostring,
    nil_tostring,
    undef_tostring
};


/*Avm toString*/
char* avm_tostring(avm_memcell* m){

    assert((m->type >=0) && (m->type <=7));
    return (*tostringFuncs[m->type])(m);
    
}


/*library function print*/
void libfunc_print(void){

    unsigned int n=avm_total_actuals();

    for(unsigned int i=0; i<n; i++){
        char* s=avm_tostring(avm_getactual(i));
        puts(s);
        free(s);
    }
}


/*Get Libraries*/
library_func_t avm_getlibraryfunc(char* id){

    int i=0;

    while(i<LIBFUNC_SIZE){
        if(!strcmp(id,"print")){ 
            printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
            return libfunc_print; 
        }
	    
        /*else if(!strcmp(id,"typeof")) 
            return libfunc_typeof; 
	    else if(!strcmp(id,"sqrt")) 
            return libfunc_sqrt; 
        else if(!strcmp(id,"cos")) 
            return libfunc_cos; 
	    else if(!strcmp(id,"sin")) 
            return libfunc_sin;
      	else if(!strcmp(id,"argument")) 
            return libfunc_argument; 
	    else if(!strcmp(id,"strtonum")) 
            return libfunc_strtonum; 
	    else if(!strcmp(id,"objectmemberkeys")) 
            return libfunc_objectmemberkeys; 
	    else if(!strcmp(id,"objecttotalmembers")) 
            return libfunc_objecttotalmembers; 
	    else if(!strcmp(id,"objectcopy")) 
            return libfunc_objectcopy;
	    else if(!strcmp(id,"totalarguments")) 
            return libfunc_totalarguments;
        else break;
        */
        i++;
    }
}


/*func enter*/
void execute_funcenter(instruction* i){
    /* printf("nothing\n"); */
    
    /* avm_memcell*func = avm_translate_operand(&i->result,&ax);
    assert(func);
    assert(pc_counter = func->data.funcVal); // func address should match pc 

    //callee actions
    totalActuals =0;
    userfunc* funcInfo = avm_getfuncinfo(pc_counter);
    topsp = top;
    top = top = funcInfo->localSize; 
    */
}


/*func exit*/
void execute_funcexit(instruction* i){
    /* printf("nothing\n"); */
    unsigned oldTop = top;
    top = avm_get_envalue(topsp + AVM_SAVEDTOP_OFFSET);
    pc_counter = avm_get_envalue(topsp + AVM_SAVEDPC_OFFSET);
    topsp = avm_get_envalue(topsp + AVM_SAVEDTOPSP_OFFSET);
    while (++oldTop<=top)
        avm_memcell_clear(&stack[oldTop]);
    
}


/*AVM library function*/
void avm_calllibfunc(char* id){

    library_func_t f;
    char*s;
    s=id;
    f=avm_getlibraryfunc(s);

    if(!f){
        avm_error("unsupported lib func '%s' called!",id);
        executionFinished=1;
    }else{
        /* notice that enter function and exit function
        are called manually */
        //avm_call_save_environment();
        topsp=top;
        totalActuals=0;
        (*f)();                                    //call library function
        if(!executionFinished){                     // an error may naturally occur inside
            execute_funcexit((instruction*)0);      // return sequence 
        }
    }
}


/*Initalise Stack*/
void avm_init_stack(void){
   
    for (unsigned int i=0; i<AVM_STACKSIZE; ++i) {
        AVM_WIPEOUT(stack[i]); 
        stack[i].type=undef_m;
    }
}


/*Init Table Buckets*/
void avm_table_buckets_init(avm_table_bucket** p){
    for (unsigned i=0; i<AVM_TABLE_HASHSIZE; ++i)
        p[i] = (avm_table_bucket*) 0;
}


/*Create a new Table*/
avm_table* avm_table_new (void) {
    
    avm_table* t = (avm_table*) malloc(sizeof(avm_table));
    AVM_WIPEOUT(*t);

    t->refCounter = t->total = 0;
    avm_table_buckets_init(t->numIndexed);
    avm_table_buckets_init(t->strIndexed);

    return t;

}


/*Clear mem Table*/
void memclear_table(avm_memcell* m) {

    assert(m->data.tableVal);
    avm_table_dec_refcounter(m->data.tableVal);
}


/*Clear mem string*/
void memclear_string(avm_memcell* m){

    assert(m->data.strVal);
    free(m->data.strVal);
}


/*dispatch*/
memclear_funct_t memclearFuncs[]={
    0,               
    memclear_string, 
    0,               
    memclear_table,  
    0,               
    0,               
    0,               
    0,              
};


/*Clear Memory cells*/
void avm_memcell_clear(avm_memcell* m){

    if(m->type !=undef_m){
        memclear_funct_t f=memclearFuncs[m->type];
        if(f){
            (*f)(m);
            m->type=undef_m;
        }
    }
};


/*Destroy Table's Buckets*/
void avm_table_buckets_destroy(avm_table_bucket**   p){
    for(unsigned int i=0; i<AVM_TABLE_HASHSIZE; ++i,++p){
        
        for(avm_table_bucket* b=*p; b;){
            avm_table_bucket* del=b;
            b=b->next;
            avm_memcell_clear(&(del->key));
            avm_memcell_clear(&(del->value));
            free(del);
        }
        p[i]=(avm_table_bucket*)0;
    }
}


/*Destroy Table*/
void avm_table_destroy(avm_table*   t){
    avm_table_buckets_destroy(t->strIndexed);
    avm_table_buckets_destroy(t->numIndexed);
    free(t);
}


/* Automatic garbage collection for tables when reference counter gets0 */
void avm_table_inc_refcounter(avm_table* t) {
    ++t->refCounter;
}


/* Automatic garbage collection for tables when reference counter gets0 */
void avm_table_dec_refcounter(avm_table* t) {
    assert(t->refCounter >0);
    if (!--t->refCounter)
        avm_table_destroy(t);
}


/*Push Table Argument*/
void avm_push_table_arg(avm_table* t){

    stack[top].type=table_m;

    avm_table_inc_refcounter(stack[top].data.tableVal=t);
    ++totalActuals;
    avm_dec_top();
}


/*Decrease top Stack*/
void avm_dec_top(void){
    
    if(!top){
        /*Stack Overflow*/
        avm_error("stack overflow","stack");
        executionFinished=1;
    }
}


/*avm_assign*/
void avm_assign(avm_memcell* lv,avm_memcell* rv){

    if(lv==rv){
        return;
    }
    if((lv->type)==table_m && (rv->type)==table_m && (lv->data.tableVal)==(rv->data.tableVal)){
        avm_warning("assigning from 'undef' content!");
    }

    avm_memcell_clear(lv);                  //clear old cell contents

    memcpy(lv,rv,sizeof(avm_memcell));


    if((lv->type)==string_m){
        lv->data.strVal=strdup(rv->data.strVal);
    }else{
        if((lv->type)==table_m){
            avm_table_inc_refcounter(lv->data.tableVal);
        }
    }
}


/*warning*/
void avm_warning(char* format){
    printf("%s\n",format);
}


/*Push Environment Value*/
void avm_push_envvalue(unsigned int val){

    stack[top].type=number_m;
    stack[top].data.numVal=val;
    avm_dec_top();
}

/*Save Environment*/
void avm_call_save_environment(void){

    avm_push_envvalue(totalActuals);
    assert(code[pc_counter].opcode==call_v);
    avm_push_envvalue(pc_counter+1);
    avm_push_envvalue(top+totalActuals+2);
    avm_push_envvalue(topsp);

}


/*Call Functor*/
void avm_call_functor(avm_table* t);



/*Execute Call*/
void execute_call(instruction* i){
    
    avm_memcell* func=avm_translate_operand(&(i->result),&ax);
    assert(func);

    switch(func->type){
        case userfunc_m:
                            {
                                avm_call_save_environment();
                                pc_counter=func->data.funcVal;
                                assert(pc_counter<AVM_ENDING_PC);
                                assert((code[pc_counter].opcode)==funcenter_v);
                                break;
                            }  
        case string_m:        
                            {
                                avm_calllibfunc(func->data.strVal);
                                break;

                            }
        case libfunc_m:     
                            {
                                avm_calllibfunc(func->data.libfuncVal);
                                break;
                            }
        case table_m:       
                            {
                                //avm_call_functor(func->data.tableVal);
                                break;
                            }
        default:            
                            {
                                char* s=avm_tostring(func);
                                avm_error("call: cannot bind '%s' to function!",s);
                                free(s);
                                executionFinished=1;
                            }
    }
}


/*Execute Assign*/
void execute_assign(instruction* i){

    avm_memcell* lv=avm_translate_operand(&(i->result),(avm_memcell*)0);
    avm_memcell* rv=avm_translate_operand(&(i->arg1),&ax);

    //assert(lv && ( stack[N-1]>=lv && lv>&stack[top])) || lv==&retval);
    assert(rv);

    avm_assign(lv,rv);
}


/*execute add*/
void execute_add(instruction* i);

/*execute sub*/
void execute_sub(instruction* i);

/*execute mul*/
void execute_mul(instruction* i);

/*execute div*/
void execute_div(instruction* i);

/*execute mod*/
void execute_mod(instruction* i);

/*execute umunius*/
void execute_uminus(instruction* i);


/*Execute Push Argument*/
void execute_pusharg(instruction* i){

    avm_memcell* arg=avm_translate_operand(&(i->arg1),&ax);
    assert(arg);

    avm_assign(&stack[top],arg);
    ++totalActuals;
    avm_dec_top();
}


/*dispatch*/
execute_func_t executeFuncs[] = {
    execute_assign,
    execute_add,
    execute_sub,
    execute_mul,
    execute_div,
    execute_mod,
    //execute_and,
    //execute_or,
    //execute_not,
    //execute_jeq,
    //execute_jne,
    //execute_jle,
    //execute_jge,
    //execute_jlt,
    //execute_jgt,
    execute_call,
    execute_pusharg,
    execute_funcenter,
    execute_funcexit
    //execute_newtable,
    //execute_tablegetelem,
    //execute_tablesetelem,
    //execute_nop,
    //execute_jump
};


/*Execute Cycle*/
void execute_cycle(void){

    if(executionFinished){
        return ;
    }else{
        if(pc_counter==AVM_ENDING_PC){
            executionFinished=1;
            return;
        }else{
            assert(pc_counter<AVM_ENDING_PC);
            instruction* instr=code+pc_counter;

            assert(instr->opcode>=0 && instr->opcode<=AVM_MAX_INSTRUCTIONS);
            if(instr->srcLine){
                currLine=instr->srcLine;
            }
            unsigned int oldPc=pc_counter;
            (*executeFuncs[instr->opcode])(instr);
            if(pc_counter==oldPc){
                ++pc_counter;
            }
        }
    }
    
}


/*Set Table's element*/
void avm_table_set_elem (avm_memcell* key, avm_memcell* value);


/*add implemntations*/
double add_impl(double x,double y){ 
    
    return x + y; 
}


/*sub implementation*/
double sub_impl(double x,double y){ 
    
    return x - y; 
}


/*mul implementation*/
double mul_impl(double x,double y){ 
    
    return x * y; 
}


/*divide implementation*/
double div_impl(double x,double y){
    
    return x / y;  
}


/*mod implementation*/
double mod_impl(double x,double y) {

    return ((unsigned)x)%((unsigned)y);
}


/*dispatch*/
arithmetic_func_t arithmeticFuncs[] = {
    add_impl,
    sub_impl,
    mul_impl,
    div_impl,
    mod_impl
};


/*execute arithmetic */
void execute_arithmetic(instruction* i) {

    avm_memcell* lv=avm_translate_operand(&(i->result),(avm_memcell*)0);
    avm_memcell* rv1=avm_translate_operand(&(i->arg1),&ax);
    avm_memcell* rv2=avm_translate_operand(&(i->arg2),&bx);

    assert(lv && (&stack[AVM_STACKSIZE-1] >= lv &&  &stack[top] < lv || lv== &retval));
    assert(rv1 && rv2);

    if(rv1->type!= number_m || rv2->type!=number_m) {
        avm_error("not a number","arithmetic!"); 
    }else {
        arithmetic_func_t op=arithmeticFuncs[i->opcode-add_v];
        avm_memcell_clear(lv);
        lv->type=number_m;
        lv->data.numVal=(*op)(rv1->data.numVal, rv2->data.numVal);
    }
}


/*number tobool*/
unsigned char number_tobool(avm_memcell* m){

    return m->data.numVal!=0;
}


/*string tobool*/
unsigned char string_tobool(avm_memcell* m){

    return m->data.strVal[0]!=0;
}


/*bool tobool*/
unsigned char bool_tobool(avm_memcell* m){

    return m->data.boolVal;
}


/*table tobool*/
unsigned char table_tobool(avm_memcell* m){
    return 1;
}


/*userfunction tobool*/
unsigned char userfunc_tobool(avm_memcell* m){
   
    return 0;
}


/*library function tobool*/
unsigned char libfunc_tobool(avm_memcell* m){
    
    return 1;
}


/*nil tobool*/
unsigned nil_tobool(avm_memcell* m){

    return 0;
}


/*undefined tobool*/
unsigned undef_tobool(avm_memcell* m){

    return 0;
}


/*discpatch*/
tobool_func_t toboolFuncs[] = {
    number_tobool,
    string_tobool,
    bool_tobool,
    table_tobool,
    userfunc_tobool,
    libfunc_tobool,
    nil_tobool,
    undef_tobool
};


/*AVM tobool*/
unsigned char avm_tobool(avm_memcell* m){

    assert(m->type>=0 && m->type<undef_m);
    return (*toboolFuncs[m->type])(m);
}


/*Library's Functions*/
library_func_t defined_libs[LIBFUNC_SIZE];


/*Hash for Libs*/
unsigned int hash_Libs(char* id){

    int i=0;

    while(i<LIBFUNC_SIZE){
        if(!strcmp(id,"print"))
            return 0; 
	    /*else if(!strcmp(id,"typeof")) 
            return 1; 
	    else if(!strcmp(id,"sqrt")) 
            return 2; 
        else if(!strcmp(id,"cos")) 
            return 3; 
	    else if(!strcmp(id,"sin")) 
            return 3;
      	else if(!strcmp(id,"argument")) 
            return 4; 
	    else if(!strcmp(id,"strtonum")) 
            return 5; 
	    else if(!strcmp(id,"objectmemberkeys")) 
            return 6; 
	    else if(!strcmp(id,"objecttotalmembers")) 
            return 7; 
	    else if(!strcmp(id,"objectcopy")) 
            return 8;
	    else if(!strcmp(id,"totalarguments")) 
            return 9;
        else break;
        */
        i++;
    }
}


/*Register defined Libraries*/
void avm_registerlibfunc(char* id, library_func_t addr) {

    unsigned int toHash=hash_Libs(id);
	defined_libs[toHash]=addr;
}


/*get enviromental value*/
unsigned int avm_get_envvalue(unsigned int n){

    assert((stack[n].type)==number_m);
    unsigned int newVal=(unsigned int)stack[n].data.numVal;
    assert((stack[n].data.numVal)==newVal);
    return newVal;
}


/*total arguments*/
void libfunc_totalarguments(void) {
    
    /*Get topsp of prev activation record
    */
    unsigned int p_topsp=avm_get_envvalue(topsp+AVM_SAVEDTOPSP_OFFSET);
    avm_memcell_clear(&retval);
    
    if(!p_topsp){       /*If 0, no previous activation record. */
        avm_error("'totalarguments' called outside","of a function");
        retval.type=nil_m;
    }else{
            /*Extract the num of actual arguments for the previous
            activation record. */
        retval.type=number_m;
        retval.data.numVal=avm_get_envalue(p_topsp+AVM_NUMACTUALS_OFFSET);
    }
}


/*init AVM*/
void avm_initialize(void){

    int i=0;
    avm_init_stack();

    avm_registerlibfunc("print",libfunc_print);
    /*
    avm_registerlibfunc("typeof",libfunc_typeof);
    avm_registerlibfunc("sqrt",libfunc_sqrt);
    avm_registerlibfunc("cos",libfunc_cos);
    avm_registerlibfunc("sin",libfunc_typeof);
    avm_registerlibfunc("argument",libfunc_argument);
    avm_registerlibfunc("strtonum",libfunc_strtonum);
    avm_registerlibfunc("objectmemberkeys",libfunc_objectmemberkeys);
    avm_registerlibfunc("objecttotalmembers",libfunc_objecttotalmembers);
    avm_registerlibfunc("objectcopy",libfunc_objectcopy);
    avm_registerlibfunc("totalarguments",libfunc_totalarguments);
    */
}


/*read magic number*/
void read_magic_number(FILE* f){

    unsigned int magicnumber;

    fread(&magicnumber,sizeof(unsigned),1,f);
    if(magicnumber!=340200501){
        avm_error("ERROR NUMBER","MAGIC NUMBER");
        exit(EXIT_FAILURE);
    }

}


/*read string consts*/
void read_strConst(FILE *f) {

    unsigned int totalstrings,sizeOfstrings,i;
    char* str;
    char** str2;


    fread(&totalstrings,sizeof(unsigned),1,f);

    myStrings=(char**)malloc(sizeof(char*)*totalstrings);

    for(i=0; i<totalstrings; i++){
        fread(&sizeOfstrings,sizeof(unsigned),1,f);
        str = (char*) malloc(sizeof(char)*(sizeOfstrings + 1)); 
        fread(str,sizeof(char),sizeOfstrings,f);
        str[sizeOfstrings]='\0';
        myStrings[i]=str;
    }
}


/*read num consts*/
void read_numConst(FILE* f){

    unsigned int   totalnums=0;
    unsigned int    i;
    double         num1;
    double*        num2;

    
    fprintf(f,"totalnumber=%u\n",totalnums);
    fread(&totalnums,sizeof(unsigned),1,f);
    myNums=(double*)malloc(sizeof(double)*totalnums);


    for(i=0;i<totalnums;i++){
        fread(&num1,sizeof(double),1,f);
        //printf("num1: %lf\n", num1);
        num2=&myNums[i];
        *num2=num1;
    }

}


/*read target code*/
void read_targetCode(FILE *f){

    unsigned int i;
    instruction* ni;

    fread(&codeSize,sizeof(unsigned),1,f);

    myInstructions=(instruction*)malloc(sizeof(instruction)*codeSize);

    for(i=0;i<codeSize;i++){
        
        ni=myInstructions+i;

        fread(&(ni->opcode),sizeof(unsigned),1,f);
        fread(&(ni->result.type),sizeof(int),1,f);
        fread(&(ni->result.val),sizeof(unsigned),1,f);
        fread(&(ni->arg1.type),sizeof(int),1,f);
        fread(&(ni->arg1.val),sizeof(unsigned),1,f);
        fread(&(ni->arg2.type),sizeof(int),1,f);
        fread(&(ni->arg2.val),sizeof(unsigned),1,f);
        fread(&(ni->srcLine),sizeof(unsigned),1,f);
    }

}


/*read global variables*/
void read_global_variables(FILE* f){

    unsigned int totalgl;

    fread(&totalgl,sizeof(unsigned),1,f);
}

// read librar'ys function 






