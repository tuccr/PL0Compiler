#define ID_MAX_LEN 11
#define NUM_MAX_LEN 5
#define TRUE 1
#define FALSE 0
#define SYM_NUM 31
#define WORD_OFFSET 3
#define MAX_SYMBOL_TABLE_SIZE 500
#define MAX_ELF_LENGTH 500
#define ERROR -1

// all symbols and reserved words possible with the exception of comments
const char SYM[SYM_NUM][18] = {"odd", "+", "-", "*", "/", "fi", "=", "<>", "<", "<=", ">", ">=", "(", ")", ",", ";", ".", ":=", "begin", "end", "if", "then", "while", "do", "call", 
                                "const", "var", "procedure", "write", "read", "else"};


typedef struct node {
	struct node * next;
	struct node * prev;
	int tokenType;
	char * lexeme;
} Token;

typedef struct linkedlist {
	struct node * head;
	struct node * tail;
	int size;
} LinkedList;

typedef struct symbol {
    int kind; // const = 1, var = 2, proc = 3
    char * name; // name up to 11 chars
    int val; // number (ASCII value)
    int level; // L level
    int addr; // M address
    int mark; // to indicate unavailable or deleted
} Symbol;

typedef enum {
    constant = 1, var, proc
} KINDS;

typedef enum {
    LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, SYS
} ISA;

typedef enum {
    WRITE = 1, READ, HALT
} SYSCODES;

typedef enum {
    RTN, ADD, SUB, MUL, DIV, EQL, NEQ, LSS, LEQ, GTR, GEQ, ODD
} OPRCODES;

// HW2 doc plus a few error types
typedef enum { // get rid of skipsym
	oddsym = 1, identsym, numbersym, plussym, minussym,
	multsym,  slashsym, fisym, eqsym, neqsym, lessym, leqsym,
	gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
	periodsym, becomessym, beginsym, endsym, ifsym, thensym, 
	whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
	readsym, elsesym, invalidsym = -999, longident = -998, longnum = -997
} Token_Type;

// // HW3 errors
// typedef enum {
//     RUNNING, MissingPeriod, IncompleteDeclaration, TakenSymbolName, IncompleteConst, 
//     NoIntConst, MissingSemicolon, UndeclaredIdent, ModifyNonVar, 
//     InvalidAssignment, MissingEnd, MissingThen, MissingDo, MissingComparisonOperator, 
//     AjarParenthesis, InvalidArthEqn, PrematureEOF, HealthyHalt, DearGodHelpMe = -999
// } Error;

// HW4 Errors
typedef enum {
    RUNNING, NeedBecomes, TakenSymbolName, NoIntConst, NoEqualsConst, IncompleteDeclaration, 
    MissingSemicolonAfterDec, MissingSemicolonComma, WrongSymbolAfterProcedure, MissingStatement, 
    WrongSymAfterStatementInBlock, MissingPeriod, MissingSemicolonBetweenStatements, UndeclaredIdent, ModifyNonVar, MissingBecomes,
    MissingIdentAfterCall, CallNonProc, MissingThen, MissingSemiOrEnd, MissingDo, WrongSymbolAfterStatement,
    MissingRationalOp, ProcInExpression, MissingRelationalOp, AjarParenthesis, InvalidFactorStart, InvalidExpressionStart, 
    LargeNumber, LongIdent, InvalidSym, MissingEnd, MissingComparisonOperator, InvalidTermStart, InvalidExpressionSym,
    PrematureEOF, NegativeL, ELFTooLong, TooManySymbols, MissingFi, HealthyHalt, DearGodHelpMe = -999
} Error;

void freeTable(Symbol ** table) {
    for(int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++) {
        if(table[i] == NULL) break;
        free(table[i]->name);
        free(table[i]);
    }
    free(table);
}

FILE * inFile;
FILE * outFile;
LinkedList * tokenList;
Token_Type tokentype;
Symbol ** symbol_table;
int tp, status, curCodeInd, numVars;
int elf[MAX_ELF_LENGTH];


Token * getFactor(Token * t, int level);
Token * getTerm(Token * t, int level);
Token * genExpression(Token * t, int level);
Token * genStatement(Token * t, int level);
Token * genCondition(Token * t, int level);
Token * constDec(Token * t);
Token * varDec(Token * t, int level);
void error(Token * t, int errorcode);
void toIdent(Token * t);

// delete an entire LinkedList
void deleteList(LinkedList * list) {
    Token * temp = list->head;
    while(temp != list->tail) {
        temp = temp->next;
        free(temp->prev->lexeme);
        free(temp->prev);
    }
    free(temp->lexeme);
    free(temp);
    free(list);
}

// delete an existing Token from a LinkedList
void deleteToken(LinkedList * list, Token * del) {
    if(del == list->tail) {
        list->tail = list->tail->prev;
        list->tail->next = NULL;
        list->size--;
        free(list->tail->lexeme);
        free(del);
        return;
    }
    else if(del == list->head) {
        list->head = list->head->next;
        list->size--;
        free(del);
        return;
    }

    Token * temp = list->head;
    while(temp != del) {
        temp = temp->next;
    }
    temp->prev->next = temp->next;
    temp->next->prev = temp->prev;
    list->size -= 1;
    free(temp->lexeme);
    free(del);
}

// add an existing Token to a LinkedList
void addToken(LinkedList * list, Token * node) {
	node->prev = list->tail;
	list->tail->next = node;
    list->tail = node;
	list->size += 1;
}

// allocate memory for a new Token
Token * initToken() {
	return (Token *)malloc(sizeof(Token));
}

// initialize a new LinkedList
LinkedList * newLinkedList() {
	LinkedList * newList = (LinkedList *)malloc(sizeof(LinkedList));
	newList->head = initToken();
	newList->tail = newList->head;
	newList->size = 0;

	return newList;
}
