#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "PL0.h"


// used to safely halt program at any time
void haltRightThere() {
    deleteList(tokenList);
    freeTable(symbol_table);
    exit(0);
}

// return TRUE if genuine closed comment, FALSE if never closes
int commentCheck(Token * t) {
    while(!feof(inFile)) {
        char b = fgetc(inFile);
        if(b == '*') {
            b = fgetc(inFile);
            if(b == '/') {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// DFA for symbols storing in global LinkedList tokenList
void getSymbol(char b) {
    int comment = FALSE;
    Token * newToken = initToken();

    // symbols are 2 characters max, so we can use switch instead of loop
    switch(b) {
        case '+':
            newToken->tokenType = plussym;
            newToken->lexeme = strdup("+");
            break;
        case '-':
            newToken->tokenType = minussym;
            newToken->lexeme = strdup("-");
            break;
        case '*':
            newToken->tokenType = multsym;
            newToken->lexeme = strdup("*");
            break;
        case '/':
            // could be a comment so check
            fpos_t temp;
            fgetpos(inFile, &temp); // if not closed comment, return fp position to here
            b = fgetc(inFile);
            if(b == '*') {
                comment = commentCheck(newToken);
            }

            if(comment == TRUE) { // reject token if comment
                free(newToken);
                return;
            }
            else {
                ungetc(b, inFile);
                newToken->tokenType = slashsym;
                newToken->lexeme = strdup("/");
                fsetpos(inFile, &temp);
            }
            break;
        case '=':
            newToken->tokenType = eqsym;
            newToken->lexeme = strdup("=");
            break;
        case '<':
            b = fgetc(inFile);
            if(b == '>') {
                newToken->tokenType = neqsym;
                newToken->lexeme = strdup("<>");
            }
            else if(b == '=') {
                newToken->tokenType = leqsym;
                newToken->lexeme = strdup("<=");
            }
            else {
                ungetc(b, inFile);
                newToken->tokenType = lessym;
                newToken->lexeme = strdup("<");
            }
            break;
        case '>':
            b = fgetc(inFile);
            if(b == '=') {
                newToken->tokenType = geqsym;
                newToken->lexeme = strdup(">=");
            }
            else {
                ungetc(b, inFile);
                newToken->tokenType = gtrsym;
                newToken->lexeme = strdup(">");
            }
            break;
        case '(':
            newToken->tokenType = lparentsym;
            newToken->lexeme = strdup("(");
            break;
        case ')':
            newToken->tokenType = rparentsym;
            newToken->lexeme = strdup(")");
            break;
        case ',':
            newToken->tokenType = commasym;
            newToken->lexeme = strdup(",");
            break;
        case ';':
            newToken->tokenType = semicolonsym;
            newToken->lexeme = strdup(";");
            break;
        case '.':
            newToken->tokenType = periodsym;
            newToken->lexeme = strdup(".");
            break;
        case ':':
            b = fgetc(inFile);
            if(b == '=') {
                newToken->tokenType = becomessym;
                newToken->lexeme = strdup(":=");
            }
            else { // there is no strict ":" symbol
                error(NULL, InvalidSym);
                printf("Error: invalid symbol.\n");
                free(tokenList);
                fclose(inFile);
                exit(1);
            }
            break;
        default: // unknown symbol
            error(NULL, InvalidSym);
            printf("Error: invalid symbol.\n");
            free(tokenList);
            fclose(inFile);
            exit(1);
            break;
    }

    addToken(tokenList, newToken); // accept token
    return;
}

// DFA for numbers
void getNum(char b) {
    Token * newToken = initToken();
    char buffer[100];
    int i = 1;
    buffer[0] = b;
    while(!feof(inFile)) {
        b = getc(inFile);

        if(isdigit(b) != 0) {
            buffer[i] = b;
            i++;
            if(i >= NUM_MAX_LEN) {
                error(NULL, LargeNumber);
                newToken->tokenType = longnum;
                free(newToken);
                printf("Error: number too long.\n");
                free(tokenList);
                fclose(inFile);
                exit(1);
            }
        }
        else break;
    }
    if(!feof(inFile)) ungetc(b, inFile);
    buffer[i] = '\0';


    newToken->tokenType = (newToken->tokenType == longnum) ? longnum : numbersym;
    newToken->lexeme = strdup(buffer);
    addToken(tokenList, newToken); // accept token
}

// DFA for reserved words AND identifiers
void getWordOrID(char b) {
    char buffer[50];
    int i = 1;
    buffer[0] = b;
    tokentype = identsym;
    while(!feof(inFile)) {
        b = getc(inFile);

        if(isalpha(b) != 0 || isdigit(b) != 0) {
            buffer[i] = b;
            i++;
            if(i >= ID_MAX_LEN) {
                error(NULL, LongIdent);
            }
        }
        else {
            buffer[i] = '\0';
            ungetc(b, inFile);
            break;
        }
    }

    Token * newToken = initToken();
    newToken->lexeme = strdup(buffer);

    if(tokentype == longident) {
        free(newToken);
        fclose(inFile);
        printf("Error: idenfifier name %s too long.\n", buffer);
        exit(1);
        return;
    }

    int sym = -1;
    for(int i = 0; i < SYM_NUM; i++) {
        if(strcmp(buffer, SYM[i]) == 0) {
            sym = i;
            break;
        }
    }

    if(strcmp(buffer, "odd") == 0) {
        newToken->tokenType = oddsym;
    }
    else if(sym == -1) {
        newToken->tokenType = tokentype;
    }
    else {
        newToken->tokenType = sym + WORD_OFFSET;
    }

    // no else in HW4 grammar
    if(newToken->tokenType == elsesym) newToken->tokenType = identsym;

    addToken(tokenList, newToken); // accept token
    return;
}

// Print an error to the screen and halt
void error(Token * t, int errorcode) {
    status = ERROR;
    switch(errorcode) {
        case MissingPeriod:
            printf("#%d Error: program must end with period.\n", errorcode);
            break;
        case IncompleteDeclaration:
            printf("#%d Error: const and var keywords must be followed by identifier.\n", errorcode);
            break;
        case NoEqualsConst:
            printf("#%d Error: constants must be assigned with =.\n", errorcode);
            break;
        case TakenSymbolName:
            printf("#%d Error: %s has already been declared.\n", errorcode, t->lexeme);
            break;
        case NoIntConst:
            printf("#%d Error: constants must be assigned an integer value.\n", errorcode);
            break;
        case MissingSemicolonComma:
            printf("#%d Error: semicolon or comma missing\n", errorcode);
            break;
        case MissingSemicolonAfterDec:
            printf("#%d Error: constant and variable declarations must be followed by a semicolon.\n", errorcode);
            break;
        case UndeclaredIdent:
            printf("#%d Error: undeclared identifier %s.\n", errorcode, t->lexeme);
            break;
        case MissingIdentAfterCall:
            printf("#%d Error: call must be followed by identifier\n", errorcode);
            break;
        case ModifyNonVar:
            printf("#%d Error: only variable values may be altered.\n", errorcode);
            break;
        case MissingBecomes:
            printf("#%d Error: assignment statements must use :=.\n", errorcode);
            break;
        case MissingEnd:
            printf("#%d Error: begin must be followed by end.\n", errorcode);
            break;
        case MissingThen:
            printf("#%d Error: if must be followed by then.\n", errorcode);
            break;
        case MissingFi:
            printf("#%d Error: then must be followed by fi\n", errorcode);
            break;
        case MissingDo:
            printf("#%d Error: while must be followed by do.\n", errorcode);
            break;
        case ProcInExpression:
            printf("#%d Error: expression must not contain a procedure identifier\n", errorcode);
            break;
        case MissingRelationalOp:
            printf("#%d Error: relational operator expected\n", errorcode);
            break;
        case MissingComparisonOperator:
            printf("#%d Error: condition must contain comparison operator.\n", errorcode);
            break;
        case MissingSemiOrEnd:
            printf("#%d Error: statement must be followed by semicolon or end\n", errorcode);
            break;
        case AjarParenthesis:
            printf("#%d Error: right parenthesis must follow left parenthesis.\n", errorcode);
            break;
        case InvalidFactorStart:
            printf("#%d Error: the preceding factor cannot begin with symbol \"%s\"\n", errorcode, t->lexeme);
            break;
        case InvalidTermStart:
            printf("#%d Error: the preceding term cannot begin with symbol \"%s\"\n", errorcode, t->lexeme);
            break;
        case InvalidExpressionStart:
            printf("#%d Error: An expression cannot begin with symbol \"%s\"\n", errorcode, t->lexeme);
            break;
        case InvalidExpressionSym:
            printf("#%d Error: expression cannot contain the symbol \"%s\"\n", errorcode, t->lexeme);
            break;
        case PrematureEOF:
            printf("#%d Error: reached end of file unexpectedly.\n", errorcode);
            break;
        case InvalidSym:
            printf("#%d Error: invalid symbol\n", errorcode);
            break;
        case LargeNumber:
            printf("#%d Error: Number too large, max number length is 5 digits\n", errorcode);
            break;
        case LongIdent:
            printf("#%d Error: identifier name too long (max length is 11 characters)\n", errorcode);
            break;
        case NegativeL:
            printf("#%d Error: tried to create instruction with negative L\n", errorcode);
            break;
        case NeedBecomes:
            printf("#%d Error: use \":=\" instead of \"=\"\n", errorcode);
            break;
        case WrongSymbolAfterProcedure:
            printf("#%d Error: procedure must be followed by an identifier\n", errorcode);
            break;
        case WrongSymbolAfterStatement:
            printf("#%d Error: incorrect symbol following statement\n", errorcode);
            break;
        case CallNonProc:
            printf("#%d Error: can only make calls to procedure type identifers, %s is of type %d\n", errorcode, t->lexeme, t->tokenType);
            break;
        case ELFTooLong:
            printf("#%d Error: generated instructions too long, will cause stack overflow for stack size %d\n", errorcode, MAX_ELF_LENGTH);
            break;
        case TooManySymbols:
            printf("#%d Error: symbol table size exceeded, too many symbols for symbol table size %d\n", errorcode, MAX_SYMBOL_TABLE_SIZE);
            break;
        default:
            printf("Unknown Error: Something unexpected went wrong or error code not defined (code: %d)\n", errorcode);
            break;
    }

    haltRightThere();
}

// write generated code array to a file to be used in tandem with HW1
void genELF(char * name) {
    outFile = fopen("elf.txt", "wb");

    // write our code in our array to our outfile
    for(int i = 10; i < curCodeInd; i += 3) {
        fprintf(outFile, "%d %d %d\n", elf[i], elf[i+1], elf[i+2]);
    }

    fclose(outFile);
}

// push an instruction to the generated code array
void emit(int opr, int L, int M) {
    if(curCodeInd >= MAX_ELF_LENGTH) {
	    error(NULL, ELFTooLong);
    }


    elf[curCodeInd] = opr;
    elf[curCodeInd + 1] = L;
    elf[curCodeInd + 2] = M;

    curCodeInd += 3;
}

// mark everything in scope of caller
void markLevel(int level) {
    int temp = tp;
    while(temp > 0) {
        if(symbol_table[temp]->level == level) {
            symbol_table[temp]->mark = 1;
        }
        temp--;
    }
}

// if provided symbol name is in symbol_table, return index at which it's located, otherwise return -1
// TODO : seems to not work for nested functions
int exists(char * name, int level) {
    for(int i = tp; i > 0; i--) {
        if(strcmp(name, symbol_table[i]->name) == 0 && symbol_table[i]->mark == 0 && symbol_table[i]->level <= level) {
            return i;
        }
    }
    return -1;
}

// add symbol to symbol table
void addSymbol(int kind, char * name, int val, int level, int addr, int mark) {
    ++tp;
    if(tp > MAX_SYMBOL_TABLE_SIZE) {
        error(NULL, TooManySymbols);
    }

    Symbol * new = (Symbol *)malloc(sizeof(Symbol));
    
    new->name = strdup(name);
    new->kind = kind;
    new->val = val;
    new->level = level;
    new->addr = addr;
    new->mark = mark;

    symbol_table[tp] = new;
}

// prints the tokens inside tokenList
void printTokens() {
    Token * temp = tokenList->head->next;
    while(temp != NULL) {
        printf("%d ", temp->tokenType);
        temp = temp->next;
    }
    printf("\n");
}

// print contents of symbol_table
void printSymbolTable() {
    printf("Symbol Table:\n\n");
    printf("Kind | Name        | Value | Level | Address | Mark\n");
    printf("------------------------------------------------------\n");
    for(int i = 1; i <= tp; i++) {
        printf("%4d | %11s | %5d | %5d | %7d | %4d\n", symbol_table[i]->kind, symbol_table[i]->name, 
            symbol_table[i]->val, symbol_table[i]->level, symbol_table[i]->addr, symbol_table[i]->mark);
    }
}

// print generated instructions
void printInstructions() {
    printf("Assembly Code:\n");
    printf("Line\tOP\tL\tM\n");
    int line = 0;
    for(int i = 10; i < curCodeInd; i+=3) {
        printf("%d\t", line);
        line++;
        switch(elf[i]) {
            case JMP:
                printf("JMP\t");
                break;
            case OPR:
                switch(elf[i+2]) {
                    case RTN:
                        printf("RTN\t");
                        break;
                    case ADD:
                        printf("ADD\t");
                        break;
                    case SUB:
                        printf("SUB\t");
                        break;
                    case MUL:
                        printf("MUL\t");
                        break;
                    case DIV:
                        printf("DIV\t");
                        break;
                    case EQL:
                        printf("EQL\t");
                        break;
                    case NEQ:
                        printf("NEQ\t");
                        break;
                    case LSS:
                        printf("LSS\t");
                        break;
                    case LEQ:
                        printf("LEQ\t");
                        break;
                    case GEQ:
                        printf("GEQ\t");
                        break;
                    case GTR:
                        printf("GTR\t");
                        break;
                    case ODD:
                        printf("ODD\t");
                        break;
                    default:
                        printf("SOMETHING TERRIBLE HAS HAPPENED IN THE CODE DEAR GOD\n");
                        haltRightThere();
                        break;
                }
                break;
            case INC:
                printf("INC\t");
                break;
            case SYS:
                printf("SYS\t");
                break;
            case LIT:
                printf("LIT\t");
                break;
            case LOD:
                printf("LOD\t");
                break;
            case STO:
                printf("STO\t");
                break;
            case CAL:
                printf("CAL\t");
                break;
            case JPC:
                printf("JPC\t");
                break;
            default:
                printf("AAAAAAAAAAAAAAAA\n");
                break;
        }
        printf("%d\t%d\n", elf[i+1], elf[i+2]);
    }
    printf("\n");
}

// generate code for factors
// factor ::= ident | number | "(" expression ")“.
Token * getFactor(Token * t, int level) {
    int L;
    switch(t->tokenType) {
        case identsym:
            int ind = exists(t->lexeme, level);
            if(ind == -1) {
                error(t, UndeclaredIdent);
                break;
            }
            if(symbol_table[ind]->kind == constant) {
                emit(LIT, 0, symbol_table[ind]->val);
            }
            else if(symbol_table[ind]->kind == var) {
                L = level - symbol_table[ind]->level;
                emit(LOD, L, symbol_table[ind]->addr);
            }
            else if(symbol_table[ind]->kind == proc) {
                error(t, ProcInExpression);
            }
            t = t->next;
            break;
        case numbersym:
            emit(LIT, 0, atoi(t->lexeme));
            t = t->next;
            break;
        case lparentsym:
            t = t->next;
            t = genExpression(t, level);
            if(t->tokenType != rparentsym) {
                error(t, AjarParenthesis);
            }
            t = t->next;
            break;
        default:
            error(t, InvalidFactorStart);
            break;
    }
    return t;
}

// generate code for terms
// term ::= factor {("*"|"/")factor}.
Token * getTerm(Token * t, int level) {
    t = getFactor(t, level);
    while(t->tokenType == multsym || t->tokenType == slashsym) { 
        switch(t->tokenType) {
            case multsym:
                t = t->next;
                t = getFactor(t, level);
                emit(OPR, level, MUL);
                break;
            case slashsym:
                t = t->next;
                t = getFactor(t, level);
                emit(OPR, level, DIV);
                break;
            default: 
                error(t, InvalidTermStart);
                break;
        }
    }
    return t;
}

// modified from HW3 document to not use signed numbers
Token * genExpression(Token * t, int level) { 
    if(t->tokenType != numbersym && t->tokenType != identsym && t->tokenType != lparentsym) {
        error(t, InvalidExpressionStart);
        return t;
    }
    t = getTerm(t, level);

    while(t->tokenType == minussym || t->tokenType == plussym) {
        if(t->tokenType == minussym) {
            t = t->next;
            t = getTerm(t, level);
            emit(OPR, 0, SUB);
        }
        else if(t->tokenType == plussym) {
            t = t->next;
            t = getTerm(t, level);
            emit(OPR, 0, ADD);
        }
        else if(t->tokenType == procsym) {
            error(t, ProcInExpression);
        }
        else {
            error(t, InvalidExpressionSym);
            break;
        }
    }

    return t;
}

// generates code for statements
/*
statement ::= [ ident ":=" expression
    | "begin" statement { ";" statement } "end"
    | "if" condition "then" statement "fi"
    | "while" condition "do" statement
    | "read" ident
    | "write" expression
    | empty ] .
*/
Token * genStatement(Token * t, int level) {    
    int jpcInd, ind, L;
    switch(t->tokenType) {
        case identsym:
            ind = exists(t->lexeme, level);
            if(ind == -1) {
                error(t, UndeclaredIdent);
                break;
            }
            if(symbol_table[ind]->kind != 2) {
                error(t, ModifyNonVar);
                break;
            }
            t = t->next;
            if(t->tokenType != becomessym) {
                if(t->tokenType == eqsym) {
                    error(t, NeedBecomes);
                    break;
                }
                error(t, MissingBecomes);
                break;
            }
            t = t->next;
            t = genExpression(t, level);
            L = level - symbol_table[ind]->level;
            emit(STO, L, symbol_table[ind]->addr);
            break;
        case beginsym:
            do {
                t = t->next;
                t = genStatement(t, level);
            } while(t->tokenType == semicolonsym);
            if(t->tokenType != endsym) {
                error(t, MissingEnd);
            }
            t = t->next;
            break;
        case whilesym:
            t = t->next;
            int loopInd = curCodeInd;
            t = genCondition(t, level);
            if(t->tokenType != dosym) {
                error(t, MissingDo);
                break;
            }
            t = t->next;
            jpcInd = curCodeInd;
            emit(JPC, 0, jpcInd);
            t = genStatement(t, level);
            emit(JMP, 0, loopInd);
            elf[jpcInd + 2] = curCodeInd;
            break;
        case callsym:
            t = t->next;
            if(t->tokenType != identsym) error(t, MissingIdentAfterCall);
            ind = exists(t->lexeme, level);
            if(ind == -1) error(t,UndeclaredIdent);
            if(symbol_table[ind]->kind != proc) error(t, CallNonProc);
            emit(CAL, level - symbol_table[ind]->level, symbol_table[ind]->addr);
            t = t->next;
            break;
        case ifsym:
            t = t->next;
            t = genCondition(t, level);
            jpcInd = curCodeInd;
            emit(JPC, 0, jpcInd);
            if(t->tokenType != thensym) {
                error(t, MissingThen);
                break;
            }
            t = t->next;
            t = genStatement(t, level);
            if(t->tokenType != fisym) {
                error(t, MissingFi);
                break;
            }
            t = t->next;
            elf[jpcInd + 2] = curCodeInd;
            break;
        case readsym:
            t = t->next;
            if(t->tokenType != identsym) {
                error(t, ModifyNonVar);
                break;
            }
            ind = exists(t->lexeme, level);
            if(ind == -1) {
                error(t, UndeclaredIdent);
                break;
            }
            if(symbol_table[ind]->kind != 2) {
                error(t, ModifyNonVar);
                break;
            }
            t = t->next;
            emit(SYS, 0, READ);
            L = level - symbol_table[ind]->level; // error on this line
            emit(STO, L, symbol_table[ind]->addr);
            break;
        case writesym:
            t = t->next;
            t = genExpression(t, level);
            emit(SYS, level, WRITE);
            break;
        default:
            break;
    }
    return t;
}

// generates code for conditions and returns the pointer to the token it finished on
// condition ::= "odd" expression | expression rel-op expression.
Token * genCondition(Token * t, int level) {
    if(t->tokenType == oddsym) {
        t = t->next;
        t = genExpression(t, level);
        emit(OPR, 0, ODD);
    }
    else {
        t = genExpression(t, level);
        switch(t->tokenType) {
            case eqsym:
                t = t->next;
                t = genExpression(t, level);
                emit(OPR, 0, EQL);
                break;
            case neqsym:
                t = t->next;
                t = genExpression(t, level);
                emit(OPR, 0, NEQ);
                break;
            case lessym:
                t = t->next;
                t = genExpression(t, level);
                emit(OPR, 0, LSS);
                break;
            case leqsym:
                t = t->next;
                t = genExpression(t, level);
                emit(OPR, 0, LEQ);
                break;
            case gtrsym:
                t = t->next;
                t = genExpression(t, level);
                emit(OPR, 0, GTR);
                break;
            case geqsym:
                t = t->next;
                t = genExpression(t, level);
                emit(OPR, 0, GEQ);
                break;
            default:
                error(t, MissingComparisonOperator);
                break;
        }
    }
    return t;
}

// generates code for declaring constants and puts them in symbol table
// constdeclaration ::= [ “const” ident "=" number {"," ident "=" number} “;"].
Token * constDec(Token * t) {
    if(t->tokenType != constsym) {
        return t;
    }
    do {
        t = t->next;
        if(t->tokenType != identsym) {
            error(t, IncompleteDeclaration);
            break;
        }
        if(exists(t->lexeme, 0) != -1) {
            error(t, TakenSymbolName);
            break;
        }
        t = t->next;
        if(t == NULL || t->tokenType != eqsym) {
            error(t, NoEqualsConst);
            break;
        }
        t = t->next;
        if(t == NULL || t->tokenType != numbersym) {
            error(t, NoIntConst);
            break;
        }
        addSymbol(constant, t->prev->prev->lexeme, atoi(t->lexeme), -1, 0, 0); // constants are global so level is set to always be ignored by marklevel()
        t = t->next;
    } while(t != NULL && t->tokenType == commasym);
    if(t->tokenType != semicolonsym) {
        error(t, MissingSemicolonAfterDec);
    }
    return t->next;
}

// generates code for variable declaration and puts them in symbol_table, increments stack accordingly
// var-declaration ::= [ "var" ident {"," ident} “;"].
Token * varDec(Token * t, int level) {
    if(t->tokenType != varsym) return t;
    int tx;
    numVars = 0;
    do {
        numVars++;
        t = t->next;
        if(t->tokenType != identsym) {
            error(t, IncompleteDeclaration);
            break;
        }
        tx = exists(t->lexeme, level);
        if(tx != -1) {
            if(symbol_table[tx]->level == level) {
                error(t, TakenSymbolName);
                break;
            }
            else {
                addSymbol(var, t->lexeme, 0, level, numVars + 2, 0);
            }
        }
        else addSymbol(var, t->lexeme, 0, level, numVars + 2, 0); // set var addr as the # of already declared vars plus 2 (as per HW3 doc)

        t = t->next;

    } while(t->tokenType == commasym);
    if(t == NULL) {
        error(t, PrematureEOF);
        return t;
    }
    if(t->tokenType != semicolonsym) {
        error(t, MissingSemicolonAfterDec);
        return t;
    }
    // emit(INC, 0, numVars + 3); // increment stack according to number of variables declared

    return t->next;
}

// generates code for a block
// block ::= const-declaration var-declaration statement.
Token * genBlock(Token * t, int level) {
    int tempInd = curCodeInd;
    emit(JMP, 0, 0);

    t = constDec(t);
    t = varDec(t, level);
    int nv = numVars;

    while(t->tokenType == procsym) {
        t = t->next;
        if(t->tokenType != identsym) error(t, WrongSymbolAfterProcedure);
        addSymbol(proc, t->lexeme, 0, level, curCodeInd, 0); 
        t = t->next;
        if(t->tokenType != semicolonsym) error(t, MissingSemicolonAfterDec);
        t = t->next;
        t = genBlock(t, level+1);
        if(t->tokenType != semicolonsym) error(t, MissingSemiOrEnd);
        t = t->next;
    }
    elf[tempInd+2] = curCodeInd;

    emit(INC, 0, nv + 3);
    t = genStatement(t, level);
    markLevel(level); // PROBABLY IN THE WRONG LOCATION
    if(level > 0) emit(OPR, 0, 0); // if not main, emit RTN
    // emit(OPR, 0, 0); // RTN should not be returned by block, instead by procedure

    return t;
}

int main(int argc, char ** argv) {
    if(argc == 2) {
        inFile = fopen(argv[1], "r");
    }
    else {
        printf("Invalid Format, please use ./[executable] [input filename]\n");
        exit(1);
    }

    if(inFile == NULL) {
        printf("Invalid filename or nonexistent file\n");
        exit(1);
    }

    // storing tokens dynamically in tokenList
    tokenList = newLinkedList();

    //prints source program using cat, ONLY WORKS ON LINUX
    printf("Source Program:\n");
    char * tempString = (char *)malloc(sizeof(char) * (4 + strlen(argv[1])));
    sprintf(tempString, "cat %s", argv[1]);
    system(tempString);
    printf("\n\n");


    // loop will look at next char, determine what it could be a part of, then call function to add token to tokenList
    // inFile and tokenList are global so no need for arguments
    while(TRUE) {
        char b = fgetc(inFile);
        if(feof(inFile)) break;

        if(isalpha(b) != 0) {
            getWordOrID(b);
        }
        else if(isdigit(b) != 0) {
            getNum(b);
        }
        else if(isspace(b) != 0) { // invisible characters are ignored and not tokenized
            continue;
        }
        else {
            getSymbol(b);
        }

    }

    fclose(inFile);

    // printTokens();

    Token * temp = tokenList->head->next; // points to first token of input file
    tp = 0; // global variable tp as table pointer in symbol_table
    int level = 0; // starting level
    status = RUNNING; // used to track whether an error was thrown or not (errors should halt program altogether but this is another safety net for a save exit)
    curCodeInd = 10; // starting program counter for code generator
    symbol_table = (Symbol **)malloc(sizeof(int) * MAX_SYMBOL_TABLE_SIZE); // initialize symbol table using pointers for easier access across functions

    temp = genBlock(temp, level);
    if(temp == NULL || temp->tokenType != periodsym) { // block must end with a period
        error(temp, MissingPeriod);
    }
    else {
        status = HealthyHalt;
        printf("No errors, program is syntactically correct\n\n");
        markLevel(0);
    }
    emit(SYS, 0, HALT);

    if(status == HealthyHalt) { // if halted normally, generate file, otherwise stop
        genELF(argv[1]);
        printInstructions();
        // printSymbolTable(); // don't need to print symbol table for HW4
    }
    else printf("Missing Halt\n");

    haltRightThere();

    return 0;
}
