/*
 
 Azelis Thomas 2013
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define ALPHA 0
#define DIGIT 1
#define PLUS 2
#define MINUS 3
#define MUL 4
#define DIV 5
#define LESS 6
#define GREAT 7
#define EQUAL 8
#define COLON 9
#define SEMICOLON 10
#define COMMA 11
#define LEFTBRACKET 12
#define RIGHTBRACKET 13
#define LEFTSQUAREBRACKET 14
#define RIGHTSQUAREBRACKET 15
#define eof 16
#define OTHER 17
#define SPACE 18

#define STATE0 0
#define STATE1 1
#define STATE2 2
#define STATE3 3
#define STATE4 4
#define STATE5 5
#define STATE6 6
#define STATE7 7
#define STATE8 8

#define IDTK 9
#define NUMBERTK 10
#define PLUSTK 11
#define MINUSTK 12
#define MULTK 13
#define DIVTK 14
#define LESSTK 15
#define LESSEQUALTK 16
#define NONEQUALTK 17
#define GREATTK 18
#define GREATEQUALTK 19
#define EQUALTK 20
#define ASSIGNMENTTK 21
#define SEMICOLONTK 22
#define COMMATK 23
#define LEFTBRACKETTK 24
#define RIGHTBRACKETTK 25
#define LEFTSQUAREBRACKETTK 26
#define RIGHTSQUAREBRACKETTK 27
#define EOF_ERROR 28
#define COLON_ERROR 29
#define OTHER_ERROR 30
#define EOFTK 31

#define FUNCTIONTK 32
#define WHILETK 33
#define CALLTK 34
#define PROCEDURETK 35
#define ANDTK 36
#define RETURNTK 37
#define IFTK 38
#define ORTK 39
#define INPUTTK 40
#define ELSETK 41
#define NOTTK 42
#define PRINTTK 43
#define PROGRAMTK 44
#define BEGINTK 45
#define ENDTK 46
#define CONSTTK 47
#define VARTK 48
#define THENTK 49
#define DOTK 50
#define FORTK 51
#define TOTK 52
#define STEPTK 53


#define SYMBOLS 19
#define STATES 9

#define N 30

#define PARAMETER 54
#define TEMPORARY 55

#define CV 56
#define REF 57

int qnum=1,tnum=1;
int foundLevel;

struct quads{
	int number;
	char *op,*x,*y,*z;
	struct quads *next;
};

struct list {
	int num;
	struct list *next;
};

struct result {
    char place[30];
    struct list *true,*false;
};

struct argument {
    int type;
    struct argument *next;
};

struct entity {
    int type;
    char name[N];
    union {
        struct variable {
            int type,offset;
        } var;
        int offset;

        struct function {
            int type,startQuad,framelength,nestingLevel;
            struct argument *args;
        } func;
        char value[N];
        struct parameter {
            int type,offset;
        } param;
    } u;
    struct entity *next;
};
struct scope {
    int nestingLevel;
    struct entity *entities;
    struct scope *next,*prev;
};

struct scope *head=NULL;
struct quads *first=NULL,*first_final=NULL;

void genquad(char *op,char *x,char *y,char *z);
char *newtemp();
void backpatch(struct list *l,int num);
struct list *mergelist(struct list *l1,struct list *l2);
void myprint(FILE *out);

void addScope();
void addArgument(int type,struct entity *temp);
void addVariable(char *name,int type,int offset);
void addFunction(char *name,int type);
void addConstant(char *name,char *value);
void addParameter(char *name,int type,int offset);
void addTemporary(char *name,int offset);
int newoffset();
void deleteScope();
struct entity *findFunc(char *name);
struct entity *findEntity(char *name);
void printScopes();
int check(char *name);

FILE *infile=NULL,*out=NULL;
int line=1;

int trans[STATES][SYMBOLS];
void init();
int nextchar();
void lex();

void program();
void programblock(int *id,char *word,char *blockName,int type);
void block(int *id,char *word);
void declarations(int *id,char *word);
void subprograms(int *id,char *word);
void sequence(int *id,char *word);
void statement(int *id,char *word);
void constdecl(int *id,char *word);
void vardecl(int *id,char *word);
void assignconst(int *id,char *word);
void procorfunc(int *id,char *word);
void procorfuncbody(int *id,char *word,char *blockName,int type);
void formalpars(int *id,char *word,char *blockName);
void formalparlist(int *id,char *word,char *blockName);
void formalparitem(int *id,char *word,char *blockName);
void assignmentstat(int *id,char *word);
void ifstat(int *id,char *word);
void elsepart(int *id,char *word);
void whilestat(int *id,char *word);
void forstat(int *id,char *word);
void steppart(int *id,char *word,char *assVar);
void callstat(int *id,char *word);
void printstat(int *id,char *word);
void inputstat(int *id,char *word);
struct result* expression(int *id,char *word);
struct result* condition(int *id,char *word);
void blockorstat(int *id,char *word);
void actualpars(int *id,char *word,struct argument *temp);
void actualparlist(int *id,char *word,struct argument *args);
void actualparitem(int *id,char *word,struct argument *args);
struct result* boolfactor(int *id,char *word);
struct result* boolterm(int *id,char *word);
void relationaloper(int *id,char *word);
int muloper(int *id,char *word);
int addoper(int *id,char *word);
void optactualpars(int *id,char *word,struct argument *args);
struct result* factor(int *id,char *word);
struct result* term(int *id,char *word);
void optionalsign(int *id,char *word);



void endToFinal();
void storerv(int r,char *v);
void loadvr(char *v,int r);
void gnlvcode(struct entity *ent);

int main(int argc,char *argv[]){

	FILE *outfile;

	if(argc<4){
		printf("Usage: %s <input_filename> <intermediate_code_filename> <final_code_filename>\n",argv[0]);
		exit(0);
	}
	if((infile=fopen(argv[1],"r"))==NULL){
		printf("Error opening file\n");
		exit(0);
	}
	if((outfile=fopen(argv[2],"a"))==NULL){
		printf("Error opening file\n");
		exit(0);
	}

	if((out=fopen(argv[3],"w"))==NULL){
		printf("Error opening file\n");
		exit(0);
	}
	fprintf(out,"                                    ");
	init();

	program();
	printf("Program %s has no errors\n",argv[1]);
	myprint(outfile);
	printScopes();

    return(0);
}


void init(){

	int i;
	trans[STATE0][ALPHA]=STATE1;
	trans[STATE0][DIGIT]=STATE2;
	trans[STATE0][SPACE]=STATE0;
	trans[STATE0][PLUS]=PLUSTK;
	trans[STATE0][MINUS]=MINUSTK;
	trans[STATE0][MUL]=MULTK;
	trans[STATE0][DIV]=STATE3;
	trans[STATE0][LESS]=STATE6;
	trans[STATE0][EQUAL]=EQUALTK;
	trans[STATE0][GREAT]=STATE7;
	trans[STATE0][COLON]=STATE8;
	trans[STATE0][SEMICOLON]=SEMICOLONTK;
	trans[STATE0][COMMA]=COMMATK;
	trans[STATE0][LEFTBRACKET]=LEFTBRACKETTK;
	trans[STATE0][RIGHTBRACKET]=RIGHTBRACKETTK;
	trans[STATE0][LEFTSQUAREBRACKET]=LEFTSQUAREBRACKETTK;
	trans[STATE0][RIGHTSQUAREBRACKET]=RIGHTSQUAREBRACKETTK;
	trans[STATE0][eof]=EOFTK;
	trans[STATE0][OTHER]=OTHER_ERROR;

	for(i=0;i<SYMBOLS;i++){
		trans[STATE1][i]=IDTK;
	}
	trans[STATE1][ALPHA]=STATE1;
	trans[STATE1][DIGIT]=STATE1;

	for(i=0;i<SYMBOLS;i++){
		trans[STATE2][i]=NUMBERTK;
	}
	trans[STATE2][DIGIT]=STATE2;


	for(i=0;i<SYMBOLS;i++){
		trans[STATE3][i]=DIVTK;
	}
	trans[STATE3][MUL]=STATE4;


	for(i=0;i<SYMBOLS;i++){
		trans[STATE4][i]=STATE4;
	}
	trans[STATE4][MUL]=STATE5;
	trans[STATE4][eof]=EOF_ERROR;

	for(i=0;i<SYMBOLS;i++){
		trans[STATE5][i]=STATE4;
	}
	trans[STATE5][MUL]=STATE5;
	trans[STATE5][eof]=EOF_ERROR;
	trans[STATE5][DIV]=STATE0;

	for(i=0;i<SYMBOLS;i++){
		trans[STATE6][i]=LESSTK;
	}
	trans[STATE6][GREAT]=NONEQUALTK;
	trans[STATE6][EQUAL]=LESSEQUALTK;


	for(i=0;i<SYMBOLS;i++){
		trans[STATE7][i]=GREATTK;
	}
	trans[STATE7][EQUAL]=GREATEQUALTK;


	for(i=0;i<SYMBOLS;i++){
		trans[STATE8][i]=COLON_ERROR;
	}
	trans[STATE8][EQUAL]=ASSIGNMENTTK;

}

int nextchar(char *ch){

	char c=getc(infile);
	*ch=c;
	if(isspace(c)){
		if(c=='\n'){
			line++;
		}
		c=' ';

	}
	else if(isalpha(c)){
		c='a';
	}
	else if(isdigit(c)){
		c='1';
	}

	switch(c){
		case 'a':
			return (ALPHA);
		case '1':
			return (DIGIT);
		case ' ':
			return (SPACE);
		case '+':
			return (PLUS);
		case '-':
			return (MINUS);
		case '*':
			return (MUL);
		case '/':
			return (DIV);
		case '<':
			return (LESS);
		case '>':
			return (GREAT);
		case '=':
			return (EQUAL);
		case ':':
			return (COLON);
		case ';':
			return (SEMICOLON);
		case ',':
			return (COMMA);
		case '(':
			return (LEFTBRACKET);
		case ')':
			return (RIGHTBRACKET);
		case '[':
			return (LEFTSQUAREBRACKET);
		case ']':
			return (RIGHTSQUAREBRACKET);
		case EOF:
			return (eof);
		default:
			return (OTHER);
	}
}

void lex(int *id,char word[N]){

	char keywords[22][20]={"function","while","call","procedure","and","return","if","or","input","else","not","print","program","begin","end","const","var","then","do","for","to","step"};
	int i=0,state=STATE0,c;
	char ch;

	word[0]='\0';
	while(state<=STATE8){
		c=nextchar(&ch);

		if(state!=STATE4 && state!=STATE5 && c!=SPACE && i<30){
			word[i]=ch;
			i++;
		}
		state=trans[state][c];
		if(state==STATE4 || state==STATE5){
			i=0;
			word[i]='\0';
		}

	}
	word[i]='\0';

	if(state==DIVTK || state==LESSTK || state==GREATTK || state==IDTK || state==NUMBERTK){
		if(ch=='\n'){
			line--;
		}
		ungetc(ch,infile);

		if(c!=SPACE){
			word[i-1]='\0';
		}
	}

	if(state==EOF_ERROR){
		printf("Unexpected eof inside comments\n");
		exit(0);
	}
	if(state==COLON_ERROR){
		printf("Expected = after :\n");
		exit(0);
	}

	if(state==OTHER_ERROR){
		printf("Unexpected character %c\n",ch);
		exit(0);
	}
	if(state==IDTK){
		for(i=0;i<22;i++){
			if(strcmp(word,keywords[i])==0){
				state=32+i;
			}
		}

	}
	*id=state;
}

void program(){
	int id;
	char word[N],blockName[N];

	lex(&id,word);

	if(id==PROGRAMTK){
		lex(&id,word);
		if(id==IDTK){
            strcpy(blockName,word);
			lex(&id,word);
			addScope();
			programblock(&id,word,blockName,-1);
		}
		else{
			printf("Line %d:Program name expected\n",line);
			exit(0);
		}
	}
	else{
		printf("Line %d:The keyword program was expected\n",line);
		exit(0);
	}
}
void programblock(int *id,char word[N],char blockName[N],int type){

    struct entity *temp;

	declarations(id,word);
	subprograms(id,word);
	genquad("beginblock",blockName,"_","_");
	if(type!=-1){
        temp=findFunc(blockName);
       	temp->u.func.startQuad=qnum;
	}
	block(id,word);
	if(type==-1){
		genquad("halt","_","_","_");
	}
	genquad("endblock",blockName,"_","_");
	if(type==-1){
		fseek(out,0,SEEK_SET);
		fprintf(out,"movi R[0],600\n");
		fprintf(out,"jmp L%d\n",first_final->next->number);

		fseek(out,0,SEEK_END);
		deleteScope();
	}
}


void block(int *id, char word[N]){

	if(*id==BEGINTK){
		lex(id,word);
		sequence(id,word);
		if(*id==ENDTK){
			lex(id,word);
		}
		else{
			printf("Line %d: The keyword end was expected\n",line);
			exit(0);
		}
	}
	else{
		printf("Line %d: The keyword begin was expected\n",line);
		exit(0);
	}
}

void declarations(int *id,char *word){
	while(*id==CONSTTK){
		constdecl(id,word);
	}
	while(*id==VARTK){
		vardecl(id,word);
	}
}
void subprograms(int *id,char *word){
	while(*id==FUNCTIONTK || *id==PROCEDURETK){
		procorfunc(id,word);
	}
}
void sequence(int *id,char *word){
	statement(id,word);
	while(*id==SEMICOLONTK){
		lex(id,word);
		statement(id,word);
	}
}


void constdecl(int *id,char *word){
	lex(id,word);
	assignconst(id,word);
	while(*id==COMMATK){
		lex(id,word);
		assignconst(id,word);
	}
	if(*id==SEMICOLONTK){
		lex(id,word);
	}
	else{
		printf("Line %d: ; was expected\n",line);
		exit(0);
	}
}
void vardecl(int *id,char *word){

	lex(id,word);
	if(*id==IDTK){
        addVariable(word,1,newoffset());
		lex(id,word);
		while(*id==COMMATK){
			lex(id,word);
			if(*id==IDTK){
                addVariable(word,1,newoffset());
				lex(id,word);
			}
			else{
				printf("Line %d: variable name was expected\n",line);
				exit(0);
			}
		}
	}
	else{
		printf("Line %d: variable name was expected\n",line);
		exit(0);
	}

	if(*id==SEMICOLONTK){
		lex(id,word);
	}
	else{
		printf("Line %d: ; was expected\n",line);
		exit(0);
	}
}

void assignconst(int *id,char *word){
    char assVar[N];
	if(*id==IDTK){
        strcpy(assVar,word);
		lex(id,word);
		if(*id==ASSIGNMENTTK){
			lex(id,word);
			if(*id==NUMBERTK){
			    addConstant(assVar,word);
				lex(id,word);
			}
			else{
				printf("Line %d: constant was expected\n",line);
				exit(0);
			}
		}
		else{
			printf("Line %d: := was expected\n",line);
			exit(0);
		}
	}
	else{
		printf("Line %d: variable name was expected\n",line);
		exit(0);
	}
}

void procorfunc(int *id,char *word){
    char blockName[N];
    int type=*id;
    struct entity *temp;

	lex(id,word);
	if(*id==IDTK){
        addFunction(word,type);
        addScope();
	    strcpy(blockName,word);
		lex(id,word);
		procorfuncbody(id,word,blockName,type);
		temp=findFunc(blockName);
		temp->u.func.framelength=newoffset();
		printScopes();
		deleteScope();

	}
	else{
		printf("Line %d: function or procedure name was expected\n",line);
		exit(0);
	}
}

void procorfuncbody(int *id,char *word,char *blockName,int type){
	formalpars(id,word,blockName);
	programblock(id,word,blockName,type);
}

void formalpars(int *id,char *word,char *blockName){
	if(*id==LEFTBRACKETTK){
		lex(id,word);
		if(*id==IDTK || *id==VARTK){
			formalparlist(id,word,blockName);
		}
		if(*id==RIGHTBRACKETTK){
			lex(id,word);
		}
		else{
			printf("Line %d: ) was expected\n",line);
			exit(0);
		}
	}
	else{
		printf("Line %d: ( was expected\n",line);
		exit(0);
	}
}
void formalparlist(int *id,char *word,char *blockName){
	formalparitem(id,word,blockName);
	while(*id==COMMATK){
		lex(id,word);
		formalparitem(id,word,blockName);
	}
}

void formalparitem(int *id,char *word,char *blockName){

	if(*id==IDTK){
		addParameter(word,CV,newoffset());
		addArgument(CV,findEntity(blockName));
		lex(id,word);
	}
	else if(*id==VARTK){
		lex(id,word);
		if(*id==IDTK){
            addParameter(word,REF,newoffset());
            addArgument(REF,findEntity(blockName));
			lex(id,word);
		}
		else{
			printf("Line %d: variable name was expected\n",line);
			exit(0);
		}
	}
	else{
		printf("Line %d: variable name or keyword var was expected\n",line);
		exit(0);
	}

}


void statement(int *id,char *word){
    struct entity *temp=NULL;;

	if(*id==IDTK){
//		lex(id,word);
        temp=findEntity(word);
        if(temp==NULL){
            printf("Line %d: %s not found",line,word);
            exit(0);
        }
		assignmentstat(id,word);
	}
	else if(*id==IFTK){
		lex(id,word);
		ifstat(id,word);
	}
	else if(*id==FORTK){
		lex(id,word);
		forstat(id,word);
	}
	else if(*id==WHILETK){
		lex(id,word);
		whilestat(id,word);
	}
	else if(*id==CALLTK){
		lex(id,word);
		callstat(id,word);
	}
	else if(*id==PRINTTK){
		lex(id,word);
		printstat(id,word);
	}
	else if(*id==INPUTTK){
		lex(id,word);
		inputstat(id,word);
	}
	else{
		printf("Line %d: Variable name, keyword if, for, while, call, print or input was expected\n",line);
		exit(0);
	}
}

void assignmentstat(int *id,char *word){
    struct result *exp;
    struct entity *temp;

    char assVar[30];

    strcpy(assVar,word);
    lex(id,word);
    temp=findEntity(assVar);
    if(temp->type==NUMBERTK){
        printf("Line %d: %s is a constant\n",line,assVar);
        exit(0);
    }
    else if(temp->type==FUNCTIONTK){
        printf("Line %d: %s is a function\n",line,assVar);
        exit(0);
    }
	if(*id==ASSIGNMENTTK){
		lex(id,word);
		exp=expression(id,word);
		genquad(":=",exp->place,"_",assVar);
	}
	else{
		printf("Line %d: := was expected\n",line);
		exit(0);
	}
}

void ifstat(int *id,char *word){
    struct result *cond;
    struct list *jump;

	cond=condition(id,word);
	if(*id==THENTK){
        backpatch(cond->true,qnum);
		lex(id,word);
		blockorstat(id,word);
		jump=(struct list *)malloc(sizeof(struct list));
		if(jump==NULL){
            printf("Error allocating memory");
            exit(0);
		}
		jump->next=NULL;
		jump->num=qnum;

		genquad("jump","_","_","_");
		backpatch(cond->false,qnum);
		elsepart(id,word);
		backpatch(jump,qnum);
	}
	else{
		printf("Line %d: keyword then expected\n",line);
		exit(0);
	}
}


void elsepart(int *id,char *word){
	if(*id==ELSETK){
		lex(id,word);
		blockorstat(id,word);
	}
}

void whilestat(int *id,char *word){
    struct result *cond;
    char condStart[10];

    sprintf(condStart,"%d",qnum);
	cond=condition(id,word);
	if(*id==DOTK){
        backpatch(cond->true,qnum);
		lex(id,word);
		blockorstat(id,word);
		genquad("jump","_","_",condStart);
		backpatch(cond->false,qnum);
	}
	else{
		printf("Line %d: keyword then expected\n",line);
		exit(0);
	}
}

void forstat(int *id,char *word){
    struct result *exp,*cond;
    char condStart[30],step[30];

    cond=(struct result *)malloc(sizeof(struct result));
    cond->true=(struct list *)malloc(sizeof(struct list));
    cond->false=(struct list *)malloc(sizeof(struct list));

    char assVar[30];
    strcpy(assVar,word);

	assignmentstat(id,word);
	if(*id==TOTK){
		lex(id,word);

		exp=expression(id,word);

		cond->true->next=NULL;
		cond->true->num=qnum;
		sprintf(condStart,"%d",qnum);
		genquad("<=",assVar,exp->place,"_");


		cond->false->next=NULL;
		cond->false->num=qnum;

		genquad("jump","_","_","_");
		sprintf(step,"%d",qnum);

		steppart(id,word,assVar);
		genquad("jump","_","_",condStart);
		backpatch(cond->true,qnum);
		blockorstat(id,word);
		genquad("jump","_","_",step);
		backpatch(cond->false,qnum);
	}
	else{
		printf("Line %d: keyword then expected\n",line);
		exit(0);
	}
}

void steppart(int *id,char *word,char *assVar){
    struct result *exp;
	if(*id==STEPTK){
        lex(id,word);
		exp=expression(id,word);
		genquad("+",assVar,exp->place,assVar);
	}
	else{
        genquad("+",assVar,"1",assVar);
	}
}
void callstat(int *id,char *word){
    char call[30];
    struct entity *temp;

    temp=findEntity(word);
    if(temp==NULL){
        printf("Line %d: %s not found\n",line,word);
        exit(0);
    }
    else if(temp->type!=FUNCTIONTK || temp->u.func.type!=PROCEDURETK){
        printf("Line %d: %s not a procedure\n",line,word);
        exit(0);
    }
    strcpy(call,word);
	if(*id==IDTK){
		lex(id,word);
		actualpars(id,word,temp->u.func.args);
		genquad("call",call,"_","_");
	}
}

void printstat(int *id,char *word){
    struct result *exp;
	if(*id==LEFTBRACKETTK){
		lex(id,word);
		exp=expression(id,word);
		genquad("out",exp->place,"_","_");
		if(*id==RIGHTBRACKETTK){
			lex(id,word);
		}
		else{
			printf("Line %d: ) expected\n",line);
			exit(0);
		}
	}
	else{
		printf("Line %d: ) expected\n",line);
		exit(0);
	}
}

void inputstat(int *id,char *word){
	if(*id==IDTK){
        genquad("inp",word,"_","_");
		lex(id,word);
	}
	else{
		printf("Line %d: variable name was expected\n",line);
		exit(0);
	}
}

struct result* expression(int *id,char *word){

    struct result *term1,*term2;
    char *temp,oper[N];
    int x,flag=0;

    if(*id==PLUSTK || *id==MINUSTK){
        flag=1;
        strcpy(oper,word);
    }
	optionalsign(id,word);
	term1=term(id,word);
	if(flag==1){
        genquad(oper,"0",term1->place,term1->place);
	}
	while((x=addoper(id,word))!=0){
		term2=term(id,word);
		temp=newtemp();
		if(x==PLUSTK){
            genquad("+",term1->place,term2->place,temp);
		}
		else{
            genquad("-",term1->place,term2->place,temp);
		}
        strcpy(term1->place,temp);
	}
	return(term1);
}

struct result* term(int *id,char *word){
    struct result *factor1,*factor2;
    char *temp;
    int x;
	factor1=factor(id,word);
	while((x=muloper(id,word))!=0){
		factor2=factor(id,word);
		temp=newtemp();
		if(x==MULTK){
            genquad("*",factor1->place,factor2->place,temp);
		}
		else{
            genquad("/",factor1->place,factor2->place,temp);
		}
	    strcpy(factor1->place,temp);
	}
	return(factor1);
}

struct result* factor(int *id,char *word){

    struct result *new;
    struct entity *temp;
    int flag=0;

	if(*id==NUMBERTK){
        new=(struct result *)malloc(sizeof(struct result));

        if(new==NULL){
            printf("Error allocating memory\n");
            exit(0);
        }
		strcpy(new->place,word);
		lex(id,word);
	}
	else if(*id==LEFTBRACKETTK){
		lex(id,word);
		new=expression(id,word);
		if(*id==RIGHTBRACKETTK){
			lex(id,word);
		}
		else{
			printf("Line %d: ) was expected\n",line);
			exit(0);
		}
	}
	else if(*id==IDTK){
        new=(struct result *)malloc(sizeof(struct result));

        if(new==NULL){
            printf("Error allocating memory\n");
            exit(0);
        }
        strcpy(new->place,word);
        temp=findEntity(word);
        if(temp==NULL){
            printf("Line %d: %s not found\n",line,word);
            exit(0);
        }
		lex(id,word);
		if(*id==LEFTBRACKETTK){
            flag=1;
		}
		optactualpars(id,word,temp->u.func.args);
		if(flag==1){
            if(temp->type!=FUNCTIONTK || temp->u.func.type!=FUNCTIONTK){
                printf("Line %d: %s is not a function",line,temp->name);
                exit(0);
            }
            genquad("call",new->place,"_","_");
            strcpy(new->place,newtemp());
		}
		else{
            if(temp->type==FUNCTIONTK){
                printf("Line %d: %s is a function",line,temp->name);
                exit(0);
            }
		}
	}
	else{
		printf("Line %d: variable name, keyword not or [ was expected\n",line);
		exit(0);
	}
	return(new);
}

void optactualpars(int *id,char *word,struct argument *args){
	if(*id==LEFTBRACKETTK){
		actualpars(id,word,args);
	}
}

int addoper(int *id,char *word){
    int x=*id;
	if(*id==PLUSTK || *id==MINUSTK){
		lex(id,word);
		return(x);
	}
	return(0);
}
int muloper(int *id,char *word){
    int x=*id;
	if(*id==MULTK || *id==DIVTK){
		lex(id,word);
		return(x);
	}
	return(0);
}

struct result* condition(int *id,char *word){
    struct result *bterm1,*bterm2;

	bterm1=boolterm(id,word);
	while(*id==ORTK){
		lex(id,word);
		backpatch(bterm1->false,qnum);
		bterm2=boolterm(id,word);
		bterm1->true=mergelist(bterm1->true,bterm2->true);
		bterm1=bterm2;
	}
	return(bterm1);
}

struct result* boolterm(int *id,char *word){
    struct result *bfactor1,*bfactor2;

	bfactor1=boolfactor(id,word);
	while(*id==ANDTK){
		lex(id,word);
		backpatch(bfactor1->true,qnum);

		bfactor2=boolfactor(id,word);
		bfactor1->false=mergelist(bfactor1->false,bfactor2->false);
		bfactor1=bfactor2;
	}
	return(bfactor1);
}

struct result* boolfactor(int *id,char *word){
    struct result *new,*exp1,*exp2;
    struct list *temp;

    char relop[10];
	if(*id==NOTTK){
		lex(id,word);
		if(*id==LEFTSQUAREBRACKETTK){
			lex(id,word);
			new=condition(id,word);
			temp=new->true;
			new->true=new->false;
			new->false=temp;

			if(*id==RIGHTSQUAREBRACKETTK){
				lex(id,word);
			}
			else{
				printf("Line %d: ] expected\n",line);
				exit(0);
			}
		}
		else{
			printf("Line %d: [ expected\n",line);
			exit(0);
		}

	}
	else if(*id==LEFTSQUAREBRACKETTK){
		lex(id,word);
		new=condition(id,word);
		if(*id==RIGHTSQUAREBRACKETTK){
			lex(id,word);
		}
		else{
			printf("Line %d: ] expected\n",line);
			exit(0);
		}
	}
	else{
        new=(struct result *)malloc(sizeof(struct result));

        if(new==NULL){
            printf("Error allocating memory\n");
            exit(0);
        }

		exp1=expression(id,word);
		strcpy(relop,word);
		relationaloper(id,word);
		exp2=expression(id,word);
		new->true=(struct list *)malloc(sizeof(struct list));
		if(new->true==NULL){
            printf("Error allocating memory");
            exit(0);
		}
		new->true->num=qnum;
		new->true->next=NULL;

		genquad(relop,exp1->place,exp2->place,"_");

		new->false=(struct list *)malloc(sizeof(struct list));
		if(new->false==NULL){
            printf("Error allocating memory");
            exit(0);
		}
		new->false->num=qnum;
		new->false->next=NULL;

		genquad("jump","_","_","_");
	}
	return(new);
}

void relationaloper(int *id,char *word){
	if(*id==EQUALTK || *id==LESSTK || *id==LESSEQUALTK || *id==NONEQUALTK || *id==GREATTK || *id==GREATTK || *id==GREATEQUALTK){
		lex(id,word);
	}
	else{
		printf("Line %d: =, <, >, <=, >= or <> expected\n",line);
		exit(0);
	}
}

void blockorstat(int *id,char *word){

	if(*id==BEGINTK){
		block(id,word);
	}
	else{
		statement(id,word);
	}
}

void actualpars(int *id,char *word,struct argument *args){
	if(*id==LEFTBRACKETTK){
		lex(id,word);
		if(*id!=RIGHTBRACKETTK){
			actualparlist(id,word,args);
		}
		if(*id==RIGHTBRACKETTK){
			lex(id,word);
		}
		else{
			printf("Line %d: ) expected\n",line);
			exit(0);
		}
	}
	else{
		printf("Line %d: ( expected\n",line);
		exit(0);
	}
}

void actualparlist(int *id,char *word, struct argument *args){
    struct argument *temp=args;

	actualparitem(id,word,temp);
	while(*id==COMMATK){
		lex(id,word);
		temp=temp->next;
		actualparitem(id,word,temp);
	}
}

void actualparitem(int *id,char *word,struct argument *temp){
    struct result *exp;
	if(*id==VARTK){
        if(temp==NULL || temp->type==CV){
            printf("Line %d: problem with arguments\n",line);
            exit(0);
        }
		lex(id,word);
		if(*id==IDTK){
		    genquad("par",word,"ref","_");
			lex(id,word);
		}
		else{
			printf("Line %d: variable name was expected\n",line);
			exit(0);
		}
	}
	else{
        if(temp==NULL || temp->type==REF){
            printf("Line %d: problem with arguments\n",line);
            exit(0);
        }
		exp=expression(id,word);
	    genquad("par",exp->place,"cv","_");
	}
}
void optionalsign(int *id,char *word){
	if(*id==PLUSTK || *id==MINUSTK){
		lex(id,word);
	}
}


void genquad(char *op,char *x,char *y,char *z){
    struct quads *new,*temp;
    new = (struct quads *)malloc(sizeof(struct quads));
    if(new == NULL){
        printf("Error allocating memory\n");
        exit(0);
    }
    new->number=qnum;
    qnum++;
    new->op=(char *)malloc((strlen(op)+1)*sizeof(char));
    if(new->op==NULL){
        printf("Error allocating memory\n");
        exit(0);
    }
    new->x=(char *)malloc((strlen(x)+1)*sizeof(char));
    if(new->x==NULL){
        printf("Error allocating memory\n");
        exit(0);
    }

    new->y=(char *)malloc((strlen(y)+1)*sizeof(char));
    if(new->y==NULL){
        printf("Error allocating memory\n");
        exit(0);
    }
    new->z=(char *)malloc((strlen(z)+1)*sizeof(char));
    if(new->z==NULL){
        printf("Error allocating memory\n");
        exit(0);
    }
    strcpy(new->op,op);
    strcpy(new->x,x);
    strcpy(new->y,y);
    strcpy(new->z,z);
    new->next=NULL;

    if(first==NULL){
        first=new;
    }
    else{
        temp=first;
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=new;
    }
}

char *newtemp(){
    char *c;
    c=(char *)malloc(10*sizeof(char));
    if(c==NULL){
        printf("Error allocating memory...");
        exit(0);
    }
    sprintf(c,"T_%d",tnum);
    tnum++;
    addTemporary(c,newoffset());
    return(c);
}

void backpatch(struct list *temp,int num){
    //struct list *temp=l;
    struct quads *q;
    char z[10];
    sprintf(z,"%d",num);

    while(temp!=NULL){
        q=first;
        while(q!=NULL && q->number!=temp->num){
            q=q->next;
        }
        if(q==NULL) {
            printf("skata");
            exit(0);
        }
        q->z=(char *)malloc((strlen(z)+1)*sizeof(char));
        strcpy(q->z,z);
        temp=temp->next;
    }
}

struct list *mergelist(struct list *l1,struct list *l2){
    struct list *temp=l1;

    while(temp->next!=NULL){
        temp=temp->next;
    }
    temp->next=l2;
    return(l1);
}

void myprint(FILE *out){
    struct quads *temp=first;
    while(temp!=NULL){
        fprintf(out,"%d:%s, %s, %s, %s\n",temp->number,temp->op,temp->x,temp->y,temp->z);
        temp=temp->next;
    }
}

void addScope(){
    struct scope *new;
    new=(struct scope *)malloc(sizeof(struct scope));
    if(new==NULL){
        printf("Error allocating memory");
        exit(0);
    }
    new->next=head;
    new->entities=NULL;
    if(head==NULL){
        head=new;
        new->nestingLevel=0;
    }
    else{
        head=new;
        new->nestingLevel=new->next->nestingLevel+1;
    }
}

void addArgument(int type, struct entity *e){
    struct argument *new,*temp;

    new=(struct argument *)malloc(sizeof(struct argument));
    if(new==NULL){
        printf("Error allocating memory");
        exit(0);
    }
    new->next=NULL;
    new->type=type;

    temp=e->u.func.args;
    if(temp==NULL){
        e->u.func.args=new;
    }
    else{
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=new;
    }
}

void addVariable(char *name,int type,int offset){
    struct entity *new,*temp;

    if(check(name)==0){
        printf("Line %d: %s already declared in this block",line,name);
        exit(0);
    }

    new=(struct entity *)malloc(sizeof(struct entity));
    if(new==NULL){
        printf("Error allocating memory");
        exit(0);
    }
    new->next=NULL;
    new->type=IDTK;
    strcpy(new->name,name);
    new->u.var.offset=offset;
    new->u.var.type=type;

    temp=head->entities;
    if(temp==NULL){
        head->entities=new;
    }
    else{
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=new;
    }
}

void addFunction(char *name,int type){
    struct entity *new,*temp;

    new=(struct entity *)malloc(sizeof(struct entity));
    if(new==NULL){
        printf("Error allocating memory");
        exit(0);
    }
    if(check(name)==0){
        printf("Line %d: %s already declared in this block",line,name);
        exit(0);
    }
    new->next=NULL;
    new->type=FUNCTIONTK;
    strcpy(new->name,name);
    new->u.func.args=NULL;
    new->u.func.type=type;
    new->u.func.startQuad=-1;
    new->u.func.framelength=-1;

    temp=head->entities;
    if(temp==NULL){
        head->entities=new;
    }
    else{
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=new;
    }
}

void addConstant(char *name,char *value){
    struct entity *new,*temp;

    if(check(name)==0){
        printf("Line %d: %s already declared in this block",line,name);
        exit(0);
    }

    new=(struct entity *)malloc(sizeof(struct entity));
    if(new==NULL){
        printf("Error allocating memory");
        exit(0);
    }
    new->next=NULL;
    new->type=NUMBERTK;
    strcpy(new->name,name);
    strcpy(new->u.value,value);

    temp=head->entities;
    if(temp==NULL){
        head->entities=new;
    }
    else{
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=new;
    }
}

void addParameter(char *name,int type,int offset){
    struct entity *new,*temp;

    if(check(name)==0){
        printf("Line %d: %s already declared in this block",line,name);
        exit(0);
    }

    new=(struct entity *)malloc(sizeof(struct entity));
    if(new==NULL){
        printf("Error allocating memory");
        exit(0);
    }
    new->next=NULL;
    new->type=PARAMETER;
    strcpy(new->name,name);
    new->u.param.offset=offset;
    new->u.param.type=type;

    temp=head->entities;
    if(temp==NULL){
        head->entities=new;
    }
    else{
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=new;
    }
}


void addTemporary(char *name,int offset){
    struct entity *new,*temp;

    new=(struct entity *)malloc(sizeof(struct entity));
    if(new==NULL){
        printf("Error allocating memory");
        exit(0);
    }
    new->next=NULL;
    new->type=TEMPORARY;
    strcpy(new->name,name);
    new->u.offset=offset;

    temp=head->entities;
    if(temp==NULL){
        head->entities=new;
    }
    else{
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=new;
    }
}

int newoffset(){
    struct entity *temp=head->entities;

    int counter=0;
    while(temp!=NULL){
        if(temp->type==IDTK || temp->type==TEMPORARY || temp->type==PARAMETER){
            counter++;
        }
        temp=temp->next;
    }
    return(12+4*counter);
}

void printScopes(){
    struct scope *temp;
    struct entity *tmp;
    struct argument *arg;

    temp=head;
    while(temp!=NULL){
        printf("%d: ",temp->nestingLevel);
        tmp=temp->entities;
        while(tmp!=NULL){
            printf("(%s~",tmp->name);
            if(tmp->type==TEMPORARY){
                printf("%d ",tmp->u.offset);
            }
            else if(tmp->type==FUNCTIONTK){
                printf("%d,%d",tmp->u.func.framelength,tmp->u.func.startQuad);
                if(tmp->u.func.type==FUNCTIONTK){
                    printf(",FUNC[");
                }
                else{
                    printf(",PROC[");
                }
                arg=tmp->u.func.args;
                while(arg!=NULL){
                    if(arg->type==CV){
                        printf("cv");
                    }
                    else{
                        printf("ref");
                    }
                    arg=arg->next;
                    if(arg!=NULL){
                        printf(",");
                    }
                }
                printf("] ");
            }
            else if(tmp->type==IDTK){
                printf("%d ",tmp->u.var.offset);
            }
            else if(tmp->type==PARAMETER){
                printf("%d,",tmp->u.param.offset);
                if(tmp->u.param.type==CV){
                    printf("cv ");
                }
                else{
                    printf("ref ");
                }
            }

            printf(")");
            tmp=tmp->next;
        }
        printf("\n");
        temp=temp->next;
    }
    printf("\n");
}

void deleteScope(){
    struct entity *temp;
 	
    endToFinal(first_final);
    head=head->next;
}

struct entity *findFunc(char *name){
    struct entity *temp;

    temp=head->next->entities;
    while(temp!=NULL){
        if(strcmp(temp->name,name)==0){
            return(temp);
        }
        temp=temp->next;
    }
    return(NULL);
}

struct entity *findEntity(char *name){
    struct entity *temp;
    struct scope *sc;


    sc=head;
    foundLevel=head->nestingLevel;
    while(sc!=NULL){
     foundLevel=head->nestingLevel;

        temp=sc->entities;
        while(temp!=NULL){
            if(strcmp(temp->name,name)==0){
                return(temp);
            }
            temp=temp->next;
        }
        sc=sc->next;
        foundLevel--;
    }
    return(NULL);
}

int check(char *name){
    struct entity *temp;

    temp=head->entities;
    while(temp!=NULL){
        if(strcmp(temp->name,name)==0){
            return(0);
        }
        temp=temp->next;
    }
    return(1);
}

void gnlvcode(struct entity *ent){
    int i;
    fprintf(out,"movi R[255],M[4+R[0]]\n");
    for(i=foundLevel;i<head->nestingLevel-1;i++)
        fprintf(out,"movi R[255],M[4+R[255]]\n");
    if(ent->type==IDTK)
        fprintf(out,"movi R[254],%d\n",ent->u.var.offset);
    else
        fprintf(out,"movi R[254],%d\n",ent->u.offset);
    fprintf(out,"addi R[255],R[254],R[255]");
}

void loadvr(char *v,int r){
    struct entity *ent = findEntity(v);
    if(isdigit(v[0])){
        fprintf(out,"movi R[%d] ,%s\n",r,v);
    }
    else {
        if(foundLevel==0 && ent->type==IDTK){
            fprintf(out,"movi R[%d] ,M[600+%d]\n",r,ent->u.var.offset);
        }
        else if(foundLevel==head->nestingLevel && ent->type==IDTK){
            fprintf(out,"movi R[%d] ,M[R[0]+%d]\n",r,ent->u.var.offset);
        }
        else if(foundLevel==head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==CV){
            fprintf(out,"movi R[%d] ,M[R[0]+%d]\n",r,ent->u.param.offset);
        }
        else if(foundLevel==head->nestingLevel && ent->type==TEMPORARY){
            fprintf(out,"movi R[%d] ,M[R[0]+%d]\n",r,ent->u.offset);
        }
        else if(foundLevel==head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==REF){
            fprintf(out,"movi R[255] ,M[R[0]+%d]\n",ent->u.param.offset);
            fprintf(out,"movi R[%d] ,M[R[255]]\n",r);
        }
        else if(foundLevel<head->nestingLevel && ent->type==IDTK){
            gnlvcode(ent);
            fprintf(out,"movi R[%d] ,M[M[R[255]]\n",r);
        }
        else if(foundLevel<head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==CV){
            gnlvcode(ent);
            fprintf(out,"movi R[%d] ,M[M[R[255]]\n",r);
        }
        else if(foundLevel==head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==REF){
            gnlvcode(ent);
            fprintf(out,"movi R[255] ,M[R[255]\n");
            fprintf(out,"movi R[%d] ,M[R[255]]\n",r);
        }
    }
}

void storerv(int r,char *v){
    struct entity *ent = findEntity(v);

        if(foundLevel==0 && ent->type==IDTK){
            fprintf(out,"movi R[%d] ,M[600+%d]\n",ent->u.var.offset,r);
        }
        else if(foundLevel==head->nestingLevel && ent->type==IDTK){
            fprintf(out,"movi M[R[0]+%d],R[%d] \n",ent->u.var.offset,r);
        }
        else if(foundLevel==head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==CV){
            fprintf(out,"movi M[R[0]+%d],R[%d] \n",ent->u.param.offset,r);
        }
        else if(foundLevel==head->nestingLevel && ent->type==TEMPORARY){
            fprintf(out,"movi M[R[0]+%d],R[%d] \n",ent->u.offset,r);
        }
        else if(foundLevel==head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==REF){
            fprintf(out,"movi M[R[0]+%d],R[255]\n",ent->u.param.offset);
            fprintf(out,"movi M[R[255]],R[%d] \n",r);
        }
        else if(foundLevel<head->nestingLevel && ent->type==IDTK){
            gnlvcode(ent);
            fprintf(out,"movi M[R[255]],R[%d] \n",r);
        }
        else if(foundLevel<head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==CV){
            gnlvcode(ent);
            fprintf(out,"movi M[R[255]],R[%d] \n",r);
        }
        else if(foundLevel==head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==REF){
            gnlvcode(ent);
            fprintf(out,"movi R[255] ,M[R[255]\n");
            fprintf(out,"movi M[R[255]],R[%d]\n",r);
        }
}

void endToFinal(){
	struct quads *temp=NULL;
	int counter=-1,d;
	struct entity *ent;
	
	if(first_final==NULL){
		first_final=first;
	}
	else{
		first_final=first_final->next;
	}
	temp=first_final;
	
    while(temp!=NULL){
    	if(!strcmp("par",temp->op)){
    		counter++;
    	}
    	else{
    		counter=0;
    	}
        fprintf(out,"L%d: ",temp->number);
        if(!strcmp("jump",temp->op)){
            fprintf(out,"jmp L%s\n",temp->z);
        }
        if(!strcmp("=",temp->op)){
            loadvr(temp->x,1);
            loadvr(temp->y,2);
            fprintf(out,"cmpi R[1],R[2]\n");
            fprintf(out,"je L%s\n",temp->z);
        }
        if(!strcmp("<>",temp->op)){
            loadvr(temp->x,1);
            loadvr(temp->y,2);
            fprintf(out,"cmpi R[1],R[2]\n");
            fprintf(out,"jne L%s\n",temp->z);
        }
        if(!strcmp(">",temp->op)){
            loadvr(temp->x,1);
            loadvr(temp->y,2);
            fprintf(out,"cmpi R[1],R[2]\n");
            fprintf(out,"jb L%s\n",temp->z);
        }
        if(!strcmp(">=",temp->op)){
            loadvr(temp->x,1);
            loadvr(temp->y,2);
            fprintf(out,"cmpi R[1],R[2]\n");
            fprintf(out,"jbe L%s\n",temp->z);
        }
        if(!strcmp("<",temp->op)){
            loadvr(temp->x,1);
            loadvr(temp->y,2);
            fprintf(out,"cmpi R[1],R[2]\n");
            fprintf(out,"ja L%s\n",temp->z);
        }
        if(!strcmp("<=",temp->op)){
            loadvr(temp->x,1);
            loadvr(temp->y,2);
            fprintf(out,"cmpi R[1],R[2]\n");
            fprintf(out,"jae L%s\n",temp->z);
        }

        if(!strcmp(":=",temp->op)){
            loadvr(temp->x,1);
            storerv(1,temp->z);
        }

        if(!strcmp("+",temp->op)){
            loadvr(temp->x,1);
            loadvr(temp->y,2);
            fprintf(out,"addi R[3],R[1],R[2]\n");
            storerv(3,temp->z);
        }
        if(!strcmp("-",temp->op)){
            loadvr(temp->x,1);
            loadvr(temp->y,2);
            fprintf(out,"subi R[3],R[1],R[2]\n");
            storerv(3,temp->z);
        }

        if(!strcmp("*",temp->op)){
            loadvr(temp->x,1);
            loadvr(temp->y,2);
            fprintf(out,"muli R[3],R[1],R[2]\n");
            storerv(3,temp->z);
        }
        if(!strcmp("/",temp->op)){
            loadvr(temp->x,1);
            loadvr(temp->y,2);
            fprintf(out,"divi R[3],R[1],R[2]\n");
            storerv(3,temp->z);
        }
        if(!strcmp("par",temp->op)){
	        ent = findEntity(temp->x);
        	d=ent->u.func.framelength+12+4*counter;
        	if(!strcmp("cv",temp->y)){
        		loadvr(temp->x,255);
        		fprintf(out,"movi M[%d+R[0]],R[255]\n",d);
        	}
        	else{
        		
				if(foundLevel==head->nestingLevel && ent->type==IDTK){
					fprintf(out,"movi R[255],R[0] \n");
				    fprintf(out,"movi R[254],%d \n",ent->u.var.offset);
				    fprintf(out,"addi R[255],R[254],R[255]\n");
				    fprintf(out,"movi M[%d+R[0]],R[255]\n",d);
				    
				}
				else if(foundLevel==head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==CV){
					fprintf(out,"movi R[255],R[0] \n");
				    fprintf(out,"movi R[254],%d \n",ent->u.param.offset);
				    fprintf(out,"addi R[255],R[254],R[255]\n");
				    fprintf(out,"movi M[%d+R[0]],R[255]\n",d);
				}
 				else if(foundLevel==head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==REF){
					fprintf(out,"movi R[255],R[0] \n");
				    fprintf(out,"movi R[254],%d \n",ent->u.param.offset);
				    fprintf(out,"addi R[255],R[254],R[255]\n");
				    fprintf(out,"movi R[1],M[R[255]]\n");
				    fprintf(out,"movi M[%d+R[0]],R[1]\n",d);
				}
 				else if(foundLevel<head->nestingLevel && ent->type==IDTK){
					gnlvcode(ent);
				    fprintf(out,"movi M[%d+R[0]],R[255]\n",d);
				    
				}
				else if(foundLevel<head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==CV){
					gnlvcode(ent);
				    fprintf(out,"movi M[%d+R[0]],R[255]\n",d);
				}
 				else if(foundLevel<head->nestingLevel && ent->type==PARAMETER && ent->u.param.type==REF){
					gnlvcode(ent);
				    fprintf(out,"movi R[1],M[R[255]]\n");
				    fprintf(out,"movi M[%d+R[0]],R[1]\n",d);
				}
        	}
        }
        if(!strcmp("call",temp->op)){
        	ent = findEntity(temp->x);
        	if(ent->u.func.nestingLevel==head->nestingLevel){
        		d=ent->u.func.framelength+4;
        		fprintf(out,"movi R[255],M[4+R[0]]\n");
				fprintf(out,"movi M[%d+R[0]],R[255]\n",d);
					
        	}
        	else{
        		d=ent->u.func.framelength+4;
        		fprintf(out,"movi R[255],M[4+R[0]]\n");
				fprintf(out,"movi M[%d+R[0]],R[0]\n",d);        		
        	}
        	fprintf(out,"movi R[255],%d\n",ent->u.func.framelength);
        	fprintf(out,"addi R[0],R[255],R[0]\n");
        	fprintf(out,"movi R[255],$\n");
        	fprintf(out,"addi R[254],15\n");
        	fprintf(out,"addi R[255],R[255],R[254]\n");
        	fprintf(out,"movi M[R[0]],R[255]\n");
        	fprintf(out,"jmp L%d\n",ent->u.func.startQuad);
        	fprintf(out,"subi R[0],R[0],R[255]\n");
        }
        if(!strcmp("endblock",temp->op)){
        	fprintf(out,"jmp M[R[0]]\n");	
        }
        if(!strcmp("halt",temp->op)){
        	fprintf(out,"halt\n");	
        }
        first_final=temp;
        temp=temp->next;
        
    }
}
