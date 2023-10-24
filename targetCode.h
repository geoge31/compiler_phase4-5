//DEFINES 
#define EXPAND_SIZE_INSTRUCTION 1024
#define CURR_SIZE_INSTRUCTION   (totalInstr*sizeof(instruction))
#define CURR_SIZE_STRING        (totalStringConsts*sizeof(instruction))
#define CURR_SIZE_NUMCONST      (totalNumConsts*sizeof(double*))
#define CURR_SIZE_LIBFUNCS      (totalNamedLibfuncs*sizeof(char*))
#define CURR_SIZE_USERFUNCS     (totalUserFuncs*sizeof(userfunc*))
#define NEW_SIZE_INSTRUCTION    (EXPAND_SIZE_INSTRUCTION*sizeof(instruction)+CURR_SIZE_INSTRUCTION)
#define NEW_SIZE_STRING         (EXPAND_SIZE_INSTRUCTION*sizeof(char*)+CURR_SIZE_STRING)
#define NEW_SIZE_NUMCONST       (totalNumConsts*sizeof(double*)+CURR_SIZE_NUMCONST)
#define NEW_SIZE_LIBFUNCS       (totalNamedLibfuncs*sizeof(char*)+CURR_SIZE_LIBFUNCS)
#define NEW_SIZE_USERFUNCS      (totalUserFuncs*sizeof(userfunc*)+CURR_SIZE_USERFUNCS)



//STRUCTS,ENUMS,GLOBAL DECLARATIONS



/*VM Opcode type struct*/
typedef enum vmopcode {
    assign_v,       add_v,          sub_v,
    mul_v,          div_v,          mod_v,
    uminus_v,       and_v,          or_v,
    not_v,          jeq_v,          jne_v,
    jle_v,          jge_v,          jlt_v,
    jgt_v,          call_v,         pusharg_v,
    funcenter_v,    funcexit_v,     newtable_v,
    tablegetelem_v, tablesetelem_v, nop_v,
}vmopcode;


/*VM argument type struct*/
typedef enum vmarg_t {
    label_a     =0,
    global_a    =1,
    formal_a    =2,
    local_a     =3,
    number_a    =4,
    string_a    =5,
    bool_a      =6,
    nil_a       =7,
    userfunc_a  =8,
    libfunc_a   =9,
    retval_a    =10
}vmarg_t;


/*VM Argument Struct*/
typedef struct vmarg{
    vmarg_t         type;
    unsigned int    val;
}vmarg;


/*Instruction Struct*/
typedef struct instruction{
    vmopcode        opcode;
    vmarg           result;
    vmarg           arg1;
    vmarg           arg2;
    unsigned int    srcLine;
}instruction;


/*User's functions Struct*/
typedef struct userfunc{
    unsigned    address;
    unsigned    localSize;
    char*       id;
}userfunc;


/*Incomplete Jumps Struct*/
typedef struct incomplete_jump{
    unsigned int        instrNo;
    unsigned int        instrAddress;
    struct incomplete_jump*    next;
}injump;


/*converter*/
typedef void (*generator_func_t)(quad*);


/*List of num consts*/
double*         numConsts;
unsigned int    totalNumConsts=0;
unsigned int    currNumConsts=0;


/*List of string consts*/
char**          stringConsts;
unsigned int    totalStringConsts=0;
unsigned int    currStringConsts=0;


/*List of Library's functions*/
char**      namedLibfuncs;
unsigned    totalNamedLibfuncs=0;
unsigned    currNamedLibfuncs=0;


/*List of User's functions*/
userfunc*       uf;
unsigned  int   totalUserFuncs=0;
unsigned  int   currUserFuncs=0;

/*Global Pointer to incomplete_jumps struct*/
injump *ijHead=(injump*)0;
/*set total incomplete jumps to 0 at first*/
unsigned int ijTotal=0;

/*global pointer to instructions struct*/
instruction *Hi=(instruction*)0;
/*counter for total instructions*/
unsigned int totalInstr=0;
/*counter for current isntrutions*/
unsigned int currInstr=0;



//FUNCTIONS IMPLEMENTATION



/*Return curren instruction label*/
unsigned int nextInstructionLabel(){
    return currInstr;
}


/*add incomplete jump*/
void add_incomplete_jump(unsigned int instrNo,unsigned int instrAddress){
     injump* ij = (injump*)malloc(sizeof(injump));
    ij->instrNo = instrNo;
    ij->instrAddress = instrAddress;
    ij->next = ijHead;
    ijHead = ij;
    ijTotal++;
}


/*expand strings*/
void expand_strings(void){

    assert(totalStringConsts==currStringConsts);

    char** i=(char**)malloc(NEW_SIZE_STRING);

    if(stringConsts){
        memcpy(i,stringConsts,CURR_SIZE_INSTRUCTION);
        free(stringConsts);
    }
    stringConsts=i;
    totalStringConsts+=EXPAND_SIZE_INSTRUCTION;
}


/*Set new string*/
unsigned int consts_newstring(char* s){

    if(totalStringConsts==currStringConsts){
        expand_strings();
    }

    char** st=stringConsts+currStringConsts++;
    *st=s;

    return currStringConsts-1;

}


/*expand numbers*/
void expand_numbers(void){

    assert(totalNumConsts==currNumConsts);

    double* i=(double*)malloc(NEW_SIZE_NUMCONST);

    if(numConsts){
        memcpy(i,numConsts,CURR_SIZE_NUMCONST);
        free(numConsts);
    }

    numConsts=i;
    totalNumConsts+=EXPAND_SIZE_INSTRUCTION;

}


/*Set new const num*/
unsigned int consts_newnumber(double n){

    if(totalNumConsts==currNumConsts){
        expand_numbers();
    }

    double *nt=numConsts+currNumConsts++;
    *nt=n;

    return currNumConsts-1;
}


/*expand lib's function*/
void expand_libfuncs(void){

    assert(totalNamedLibfuncs==currNamedLibfuncs);

    char** l=(char**)malloc(sizeof(NEW_SIZE_LIBFUNCS));

    if(namedLibfuncs){
        memcpy(l,namedLibfuncs,CURR_SIZE_LIBFUNCS);
        free(namedLibfuncs);
    }

    namedLibfuncs=*l;
    totalNamedLibfuncs+=EXPAND_SIZE_INSTRUCTION;
}


/*set lib's functions*/
unsigned int libfuncs_newused(const char* s){

    if(totalNamedLibfuncs=currNamedLibfuncs){
        expand_libfuncs();
    } 

    char** st=namedLibfuncs+currNamedLibfuncs++;
    *st=s;

    return currNamedLibfuncs-1; 
}


/*expand user's fucntions*/
void expand_userfuncs(void){

    assert(totalUserFuncs==currUserFuncs);

    userfunc *i=(userfunc*)malloc(NEW_SIZE_USERFUNCS);

    if(uf){
        memcpy(i,uf,CURR_SIZE_USERFUNCS);
        free(uf);
    }

    uf=i;
    totalUserFuncs+=EXPAND_SIZE_INSTRUCTION;
}


/*set user's function*/
unsigned int userfuncs_newused(symtable* uf){

    assert(totalUserFuncs==currUserFuncs);

    userfunc *i=(userfunc*)malloc(NEW_SIZE_USERFUNCS);

    if(uf){
        memcpy(i,uf,CURR_SIZE_USERFUNCS);
        free(uf);
    }

    uf=i;
    totalUserFuncs+=EXPAND_SIZE_INSTRUCTION;
}


/*operand function*/
void make_operand (expr* e, vmarg* arg) {
    
    if(e){
        switch (e->type) {

            case var_e:     {

                arg->val=e->sym->offset;

                switch (e->sym->space) {
                    case programvar:    arg->type=global_a;   break;
                    case functionlocal: arg->type=local_a;    break;
                    case formalarg:     arg->type=formal_a;   break;
                    default: assert(0);
                }
            }
            case tableitem_e:   {

                arg->val = e->sym->offset;

                switch (e->sym->space) {
                    case programvar:    arg->type = global_a;   break;
                    case functionlocal: arg->type = local_a;    break;
                    case formalarg:     arg->type = formal_a;   break;
                    default: assert(0);
                }
            }
            case arithexpr_e:   {

                arg->val=e->sym->offset;

                switch (e->sym->space) {
                    case programvar:    arg->type=global_a;   break;
                    case functionlocal: arg->type=local_a;    break;
                    case formalarg:     arg->type=formal_a;   break;
                    default: assert(0);
                }
            }
            case boolexpr_e:    {

                arg->val=e->sym->offset;

                switch (e->sym->space) {
                    case programvar:    arg->type=global_a;   break;
                    case functionlocal: arg->type=local_a;    break;
                    case formalarg:     arg->type=formal_a;   break;
                    default: assert(0);
                }
            }
            case newtable_e:  {

                arg->val = e->sym->offset;

                switch (e->sym->space) {
                    case programvar:    arg->type = global_a;   break;
                    case functionlocal: arg->type = local_a;    break;
                    case formalarg:     arg->type = formal_a;   break;
                    default: assert(0);
                }
            }
        
            /* Constants */
            case constbool_e:   {
                arg->val=e->boolConst;
                arg->type=bool_a;         
                break;
            }
            case conststring_e: {
                arg->val = consts_newstring(e->strConst);
                arg->type = string_a;       break;
            }
            case constnum_e:    {
                arg->val=consts_newnumber(e->numConst);
                arg->type=number_a;       
                break;
            }
            case nil_e: arg->type=nil_a;  break;

            /* Functions */
            case programfunc_e: {
                arg->type = userfunc_a;
                arg->val = e->sym;
                break;
            }
            case libraryfunc_e: {
                arg->type = libfunc_a;
                arg->val = libfuncs_newused(e->sym->name);
                break;
            }
            default: break;
        }
   } else{
        arg = NULL;
    }
}


/*Const number operand*/
void *make_numberoperand(vmarg* arg, double val){

   arg->val = consts_newnumber(val);
   arg->type = number_a;
}


/*Boolean operand*/
void make_booloperand(vmarg* arg, unsigned val){
	
    arg->val = val;
	arg->type = bool_a;
}


/*Return value operand*/
void make_retvaloperand(vmarg* arg){
    
    arg->type = retval_a;
}


/*expand instruction */
void expand_instr(void){

    assert(totalInstr==currInstr);

    instruction* i=(instruction*)malloc(NEW_SIZE_INSTRUCTION);

    if(Hi){
        memcpy(i,Hi,CURR_SIZE_INSTRUCTION);
        free(Hi);
    }
    Hi=i;
    totalInstr+=EXPAND_SIZE_INSTRUCTION;

}

/*Emit instruction*/
void emit_t(instruction* tnew){

    if(currInstr==totalInstr) {
        expand_instr();
    }

    instruction *t=Hi+currInstr++;

    t->opcode=tnew->opcode;
    t->arg1=tnew->arg1;
    t->arg2=tnew->arg2;
    t->result=tnew->result;
    t->srcLine=tnew->srcLine; 

    // if(t->result.type==global_a){
    //     printf("op:%u\tresult:%u\targ1:%u\targ2:%u\n", t->opcode, t->result.type,t->arg1,t->arg2);
    // }
    // printf("op:%u\tresult:%u\targ1:%u\targ2:%u\n", t->opcode, t->result.val,t->arg1.val,t->arg2.val);

    // FILE *q=fopen("BinaryFile.txt","w");
    // fprintf(q, "op:%u\tresult:%u\targ1:%u\targ2:%u\n", t->opcode, t->result,t->arg1,t->arg2);
    // fclose(q); 
}


/*generate isntruction Opcode Quad*/
void generateOQ(vmopcode op,quad* q){
    
    instruction *t=(instruction*)malloc(sizeof(instruction));

    t->opcode=op;    
    t->srcLine=q->line;

    make_operand(q->arg1,&(t->arg1));
    make_operand(q->arg2,&(t->arg2));
    make_operand(q->result,&(t->result));

    q->taddress=nextInstructionLabel();
    
    emit_t(t);
}


/*generate assign*/
void generate_ASSIGN(quad* q){
    generateOQ(assign_v,q);
}


/*generate add*/
void generate_ADD(quad* q){
    generateOQ(add_v,q);
}


/*generate sub*/
void generate_SUB(quad* q) {
    generateOQ(sub_v,q);
}


/*Generate multiply*/
void generate_MUL(quad* q){
    generateOQ(mul_v,q);
}


/*Generate divide*/
void generate_DIV(quad* q){
    generateOQ(div_v,q);
}


/*Generate mod*/
void generate_MOD(quad* q){
    generateOQ(mod_v,q);
}


/*Generate uminus*/
void generate_UMINUS(quad* q) {
    generate(mul_v,q);
}


void generate_NOP(quad* q){
    generate(nop_v,q);
}

void generate_CALL(quad* q){
    libfunc_print();
}

/*begine generating based on opcode*/
generator_func_t generators[] = {
    
    generate_ASSIGN,
    generate_ADD,
    generate_SUB,
    generate_MUL,
    generate_DIV,
    generate_MOD,
    generate_UMINUS,
    generate_CALL,
    /*generate_AND,
    generate_OR,
    generate_NOT,
    generate_IF_EQ,
    generate_IF_NOTEQ,
    generate_IF_LESSEQ,
    generate_IF_GREATEREQ,
    generate_IF_LESS,
    generate_IF_GREATER,
    ,
    generate_PARAM,
    generate_RETURN,
    generate_GETRETVAL,
    generate_FUNCSTART,
    generate_FUNCEND,
    generate_NEWTABLE,
    generate_JUMP,
    generate_TABLEGETELEM,
    generate_TABLESETELEM,
    */
    generate_NOP
    
};


/*generate the parsed quads*/
void generate(void){
            
    for(unsigned int i=0; i<nextQuadLabel(); ++i){
        /*pointer to functions generators*/
        //print(quads[i].op)
        (*generators[quads[i].op])(quads+i);
    }
}


/*Create the Bianry File*/
void writeTo_Binary(){

    unsigned int magic_number,s_Size,i;
    magic_number=340200501;

    FILE *toBin=fopen("BinaryFile.abc","wb"); 

    fwrite(&magic_number,sizeof(unsigned int),1,toBin);
    fwrite(&currNumConsts,sizeof(unsigned),1,toBin);
    //fwrite(&currNamedLibfuncs,sizeof(unsigned),1,toBin);
    
    //fwrite(&,sizeof(unsigned),1,toBin);

    /*
    for(i=0;i<currStringConsts;i++){
        s_Size=strlen(stringConsts[i]);
        fwrite(&(s_Size),sizeof(unsigned),1,toBin);
        fwrite(&(stringConsts[i]),sizeof(char*),1,toBin);
    }
    */
    fwrite(&currInstr,sizeof(unsigned),1,toBin);
    for(i=0;i<currNumConsts;i++){
        fwrite(&(numConsts[i]),sizeof(double),1,toBin);
    }

    /** 
    for(i=0;i<currNamedLibfuncs;i++){
        s_Size=strlen(namedLibfuncs[i]);
        fwrite(&(s_Size),sizeof(unsigned),1,toBin);
        fwrite(&(namedLibfuncs[i]),sizeof(char*),1,toBin);
    }
    */

    
    for(i=0;i<currInstr;i++){
        //printf("%d\n",(Hi[i].opcode));
        fwrite(&(Hi[i].opcode),sizeof(unsigned),1,toBin);
        fwrite(&(Hi[i].result.type),sizeof(int),1,toBin);
        fwrite(&(Hi[i].result.val),sizeof(unsigned),1,toBin);
        fwrite(&(Hi[i].arg1.type),sizeof(int),1,toBin);
        fwrite(&(Hi[i].arg1.val),sizeof(unsigned),1,toBin);
        fwrite(&(Hi[i].arg2.type),sizeof(int),1,toBin);
        fwrite(&(Hi[i].arg2.val),sizeof(unsigned),1,toBin);
        fwrite(&(Hi[i].srcLine),sizeof(unsigned),1,toBin);
    }

    fclose(toBin);

}


/*Readable Binary*/
void read_IntermediateCode(FILE *f) {

    unsigned int magic_number=340200501;
    unsigned int total_instr=currInstr;
    int i=0;
    instruction* ptr;

    fprintf(f,"\nMagic Number = %u\n\n",magic_number);

    fprintf(f, "\nTotal Instructions = %u\n", currInstr);
    ptr=Hi;


    for(i=0;i<currInstr;i++){

        switch((ptr+i)->opcode){

            case assign_v:
                                fprintf(f, "op:\033[1;31m%u\033[1;0m   [\033[1;33massign\033[1;0m] \tresult:%u\targ1:%u\targ2:%u\n", (ptr+i)->opcode,(ptr+i)->result,(ptr+i)->arg1,(ptr+i)->arg2);
                                break;
            case add_v:         
                                fprintf(f, "op:\033[1;31m%u\033[1;0m   [\033[1;33madd\033[1;0m]\t\tresult:%u\targ1:%u\targ2:%u\n", (ptr+i)->opcode,(ptr+i)->result,(ptr+i)->arg1,(ptr+i)->arg2);
                                break;
            case sub_v: 
                                fprintf(f, "op:\033[1;31m%u\033[1;0m   [\033[1;33msub\033[1;0m]\t\tresult:%u\targ1:%u\targ2:%u\n", (ptr+i)->opcode,(ptr+i)->result,(ptr+i)->arg1,(ptr+i)->arg2);
                                break;
            case mul_v:
                                fprintf(f, "op:\033[1;31m%u\033[1;0m   [\033[1;33mmul\033[1;0m]\t\tresult:%u\targ1:%u\targ2:%u\n", (ptr+i)->opcode,(ptr+i)->result,(ptr+i)->arg1,(ptr+i)->arg2);
                                break;
            case div_v:
                                fprintf(f, "op:\033[1;31m%u\033[1;0m   [\033[1;33mdiv\033[1;0m]\t\tresult:%u\targ1:%u\targ2:%u\n", (ptr+i)->opcode,(ptr+i)->result,(ptr+i)->arg1,(ptr+i)->arg2);
                                break;
            case mod_v: 
                                fprintf(f, "op:\033[1;31m%u\033[1;0m   [\033[1;33mmod\033[1;0m]\t\tresult:%u\targ1:%u\targ2:%u\n", (ptr+i)->opcode,(ptr+i)->result,(ptr+i)->arg1,(ptr+i)->arg2);
                                break;
            default:
                                break;
        }
        
    }   
 
}