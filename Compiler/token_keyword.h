		
#ifndef AUTOMATON_STATE_DECLARATION
#if defined(TOKEN_LIST) || defined (TYPE_SPECIFIER_LIST) || defined (DECL_SPECIFIER_LIST)
register_token(KWRD_VOID, "void", token_keyword, STMT_DECL)
register_token(KWRD_DOUBLE, "double", token_keyword, STMT_DECL)
register_token(KWRD_INT, "int", token_keyword, STMT_DECL)
register_token(KWRD_CHAR, "char", token_keyword, STMT_DECL)
register_token(KWRD_STRUCT, "struct", token_keyword, STMT_DECL)
#endif
#if defined (TOKEN_LIST) || defined (DECL_SPECIFIER_LIST)
register_token(KWRD_TYPEDEF, "typedef", token_keyword, STMT_DECL)
register_token(KWRD_CONST, "const", token_keyword, STMT_DECL)
#endif
#ifdef TOKEN_LIST
register_token(KWRD_BREAK, "break", token_keyword, STMT_BREAK)
register_token(KWRD_CONTINUE, "continue", token_keyword, STMT_CONTINUE)
register_token(KWRD_IF, "if", token_keyword, STMT_IF)
register_token(KWRD_ELSE, "else", token_keyword, STMT_NONE)
register_token(KWRD_DO, "do", token_keyword, STMT_DO_WHILE)
register_token(KWRD_WHILE, "while", token_keyword, STMT_WHILE)
register_token(KWRD_FOR, "for", token_keyword, STMT_FOR)
register_token(KWRD_RETURN, "return", token_keyword, STMT_RETURN)

register_token(KWRD_LONG, "long", token_keyword, STMT_NONE)
register_token(KWRD_FLOAT, "float", token_keyword, STMT_NONE)
register_token(KWRD_SHORT, "short", token_keyword, STMT_NONE)
register_token(KWRD_SIGNED, "signed", token_keyword, STMT_NONE)
register_token(KWRD_SIZEOF, "sizeof", token_keyword, STMT_NONE)
register_token(KWRD_UNSIGNED, "unsigned", token_keyword, STMT_NONE)

#define reg_res_func(incode_name, name, asm_name, check_ops_func, res_type, ...) \
register_token(KWRD_##incode_name, #name, token_keyword, STMT_EXPR)
#include "register_reserved_function.h"
#undef reg_res_func

#endif
#endif
 