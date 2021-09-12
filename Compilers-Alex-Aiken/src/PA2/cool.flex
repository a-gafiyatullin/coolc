/* The scanner definition for COOL. */

%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

// The compiler assumes these identifiers.
#define yylval cool_yylval

extern "C" int yylex(void);
extern int cool_yylex(void) {
    return yylex();
}

// Max size of string constants
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler. */
#undef YY_INPUT
#define YY_INPUT(buf, result, max_size) \
    if ((result = fread((char*)buf, sizeof(char), max_size, fin)) < 0) \
	    YY_FATAL_ERROR("read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

static int current_str_len  = 0;
static int error_flag       = 0;
static int escape           = 0;
static int comments_lvl     = 0;

void append_if_enough(char ch)
{
    string_buf[current_str_len++] = ch;
    
    if(MAX_STR_CONST - current_str_len - 1 < 0) {
        error_flag = 1;
    }
}

%}

CONTROL_SYMBOLS_TOKEN [;\{\}:\(\)\.@~*\/\+\-\<=,]
CLASS_TOKEN           [cC][lL][aA][sS][sS]
ELSE_TOKEN            [eE][lL][sS][eE]
FI_TOKEN              [fF][iI]
IF_TOKEN              [iI][fF]
IN_TOKEN              [iI][nN]
INHERITS_TOKEN        [iI][nN][hH][eE][rR][iI][tT][sS]
LET_TOKEN             [lL][eE][tT]
LOOP_TOKEN            [lL][oO][oO][pP]
POOL_TOKEN            [pP][oO][oO][lL]
THEN_TOKEN            [tT][hH][eE][nN]
WHILE_TOKEN           [wW][hH][iI][lL][eE]
CASE_TOKEN            [cC][aA][sS][eE]
ESAC_TOKEN            [eE][sS][aA][cC]
OF_TOKEN              [oO][fF]
NOT_TOKEN             [nN][oO][tT]
DARROW_TOKEN          =\>
NEW_TOKEN             [nN][eE][wW]
ISVOID_TOKEN          [iI][sS][vV][oO][iI][dD]
STR_BOUND_TOKEN       \"
STR_INTERNALS_TOKEN   [^"\n\0\\]*
NULL_TERM_TOKEN       \0
ESCAPE_TOKEN          \\
INT_CONST_TOKEN       [0-9]+
BOOL_CONST_TOKEN      t[rR][uU][eE]|f[aA][lL][sS][eE]
TYPEID_TOKEN          [A-Z][a-zA-Z0-9_]*
OBJECTID_TOKEN        [a-z][a-zA-Z0-9_]*
ASSIGN_TOKEN          "<-"
LE_TOKEN              "<="
COMM_OPEN_TOKEN       "(*"
DASH_COMM_OPEN_TOKEN  "--"
COMM_CLOSE_TOKEN      "*)"
WS_TOKEN              [\f\r\t\v ]+
NEXT_LINE_TOKEN       \n
IGNORE_TOKEN          [^*\n\(]*
COMM_STAR_TOKEN       \*
COMM_PAR_TOKEN        \(
DASH_IGNORE_TOKEN     [^\n]*
ERROR_TOKEN           .

%START COMMENT_OPEN DASH_COMMENT_OPEN IN_STRING

%%
<INITIAL,COMMENT_OPEN>{COMM_OPEN_TOKEN} { comments_lvl++; BEGIN COMMENT_OPEN;       }
<INITIAL>{DASH_COMM_OPEN_TOKEN}         { BEGIN DASH_COMMENT_OPEN;                  }
<COMMENT_OPEN>{COMM_CLOSE_TOKEN}        {
                                            if(comments_lvl == 1) {
                                                BEGIN 0;
                                            }
                                            comments_lvl--;
                                        }
<COMMENT_OPEN>{IGNORE_TOKEN}            ;
<COMMENT_OPEN>{COMM_STAR_TOKEN}         ;
<COMMENT_OPEN>{COMM_PAR_TOKEN}          ;
<DASH_COMMENT_OPEN>{DASH_IGNORE_TOKEN}  ;
<DASH_COMMENT_OPEN>{NEXT_LINE_TOKEN}    { curr_lineno++; BEGIN 0;    }
<INITIAL>{CONTROL_SYMBOLS_TOKEN}        { return yytext[0];          }
<INITIAL>{CLASS_TOKEN}      { return CLASS;   }
<INITIAL>{ELSE_TOKEN}       { return ELSE;    }
<INITIAL>{FI_TOKEN}         { return FI;      }
<INITIAL>{IF_TOKEN}         { return IF;      }
<INITIAL>{IN_TOKEN}         { return IN;      }
<INITIAL>{INHERITS_TOKEN}   { return INHERITS;}
<INITIAL>{LET_TOKEN}        { return LET;     }
<INITIAL>{LOOP_TOKEN}       { return LOOP;    }
<INITIAL>{POOL_TOKEN}       { return POOL;    }
<INITIAL>{THEN_TOKEN}       { return THEN;    }
<INITIAL>{WHILE_TOKEN}      { return WHILE;   }
<INITIAL>{CASE_TOKEN}       { return CASE;    }
<INITIAL>{ESAC_TOKEN}       { return ESAC;    }
<INITIAL>{OF_TOKEN}         { return OF;      }
<INITIAL>{DARROW_TOKEN}     { return DARROW;  }
<INITIAL>{NEW_TOKEN}        { return NEW;     }
<INITIAL>{ISVOID_TOKEN}     { return ISVOID;  }
<INITIAL>{NOT_TOKEN}        { return NOT;     }
<INITIAL>{STR_BOUND_TOKEN}  { BEGIN IN_STRING;}
<IN_STRING>{STR_BOUND_TOKEN} {
                                if(escape) {
                                    append_if_enough(yytext[0]);
                                    escape = 0;
                                } else {
                                    if(error_flag) {
                                        current_str_len = 0;
                                        error_flag = 0;
                                        escape = 0;

                                        BEGIN 0;
                                        return ERROR;
                                    } else {
                                        string_buf[current_str_len] = '\0';
                                        cool_yylval.symbol = stringtable.add_string(string_buf);

                                        current_str_len = 0;

                                        BEGIN 0;
                                        return STR_CONST;
                                    }
                                }
                             }
<IN_STRING>{NEXT_LINE_TOKEN} {
                                curr_lineno++;

                                if(escape) {
                                    append_if_enough('\n');
                                    escape = 0;
                                } else {
                                    if(!error_flag) {
                                        cool_yylval.error_msg = "Unterminated string constant";
                                    }

                                    current_str_len = 0;
                                    error_flag = 0;
                                    escape = 0;

                                    BEGIN 0;
                                    return ERROR;
                                }
                            }
<IN_STRING>{NULL_TERM_TOKEN} {
                                if(!error_flag) {
                                    error_flag = 1;
                                    cool_yylval.error_msg = "String contains null character";
                                }
                            }
<IN_STRING>{ESCAPE_TOKEN}   {
                                if(!error_flag) {
                                    if(escape) {
                                        append_if_enough('\\');
                                        escape = 0;
                                    } else {
                                        escape = 1;
                                    }
                                }
                            }
<IN_STRING>{STR_INTERNALS_TOKEN} {
                                if(!error_flag) {
                                    int i = 0;

                                    if(escape) {
                                        char ch = yytext[0];
                                        switch(yytext[i]) {
                                            case 'n':
                                                ch = '\n';
                                                break;
                                            case 'b':
                                                ch = '\b';
                                                break;
                                            case 't':
                                                ch = '\t';
                                                break;
                                            case 'f':
                                                ch = '\f';
                                                break;
                                        }
                                        append_if_enough(ch);
                                        i++;
                                    }
                                    for(; i < yyleng; i++) {
                                        append_if_enough(yytext[i]);
                                    
                                        if(error_flag) {
                                            cool_yylval.error_msg = "String constant too long";
                                            break;
                                        }
                                    }

                                    escape = 0;
                                }
                            }
<INITIAL>{INT_CONST_TOKEN}  {
                                cool_yylval.symbol = inttable.add_string(yytext, yyleng);
                                
                                return INT_CONST;
                            }
<INITIAL>{BOOL_CONST_TOKEN} {
                                cool_yylval.boolean = (yytext[0] == 't');

                                return BOOL_CONST;
                            }
<INITIAL>{TYPEID_TOKEN}     {
                                cool_yylval.symbol = idtable.add_string(yytext, yyleng);

                                return TYPEID;
                            }
<INITIAL>{OBJECTID_TOKEN}   {
                                cool_yylval.symbol = idtable.add_string(yytext, yyleng);

                                return OBJECTID;
                            }
<INITIAL>{ASSIGN_TOKEN}     { return ASSIGN;        }
<INITIAL>{LE_TOKEN}         { return LE;            }
<INITIAL>{COMM_CLOSE_TOKEN} {
                                cool_yylval.error_msg = "Unmatched *)";

                                return ERROR;
                            }
<INITIAL>{WS_TOKEN}         ;
{NEXT_LINE_TOKEN}           { curr_lineno++;        }
<INITIAL>{ERROR_TOKEN}      {
                                cool_yylval.error_msg = new char;
                                cool_yylval.error_msg[0] = yytext[0];

                                return ERROR;
                            }
<IN_STRING><<EOF>>          {
                                if(!error_flag) {
                                    cool_yylval.error_msg = "EOF in string constant";
                                }

                                current_str_len = 0;
                                error_flag = 0;
                                escape = 0;

                                BEGIN 0;
                                return ERROR;
                            }
<COMMENT_OPEN><<EOF>>       {
                                cool_yylval.error_msg = "EOF in comment";

                                BEGIN 0;
                                return ERROR;
                            }
%%