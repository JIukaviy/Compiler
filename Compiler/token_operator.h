#ifdef TOKEN_FUNC
token_ptr token_without_value(string str, AUTOMATON_STATE state, int line, int column) {
	return token_ptr(new token_t(line, column, state_to_token[state]));
}
#endif

#ifdef TOKEN_LIST
register_token(OP_ASSIGN, "=", token_without_value, STMT_NONE, 16) // Must be the first assign operator
register_token(OP_MOD_ASSIGN, "%=", token_without_value, STMT_NONE, 16)
register_token(OP_MUL_ASSIGN, "*=", token_without_value, STMT_NONE, 16)
register_token(OP_DIV_ASSIGN, "/=", token_without_value, STMT_NONE, 16)
register_token(OP_XOR_ASSIGN, "^=", token_without_value, STMT_NONE, 16)
register_token(OP_ADD_ASSIGN, "+=", token_without_value, STMT_NONE, 16)
register_token(OP_SUB_ASSIGN, "-=", token_without_value, STMT_NONE, 16)
register_token(OP_BIT_AND_ASSIGN, "&=", token_without_value, STMT_NONE, 16)
register_token(OP_BIT_OR_ASSIGN, "|=", token_without_value, STMT_NONE, 16)
register_token(OP_LEFT_ASSIGN, "<<=", token_without_value, STMT_NONE, 16)
register_token(OP_RIGHT_ASSIGN, ">>=", token_without_value, STMT_NONE, 16)	// Must be the last assign operator

register_token(QUESTION_MARK, "?", token_without_value, STMT_NONE, 15)
register_token(COLON, ":", token_without_value, STMT_NONE, 15)

register_token(OP_OR, "||", token_without_value, STMT_NONE, 14)

register_token(OP_AND, "&&", token_without_value, STMT_NONE, 13)

register_token(OP_BIT_OR, "|", token_without_value, STMT_NONE, 12)

register_token(OP_XOR, "^", token_without_value, STMT_NONE, 11)

register_token(OP_BIT_AND, "&", token_without_value, STMT_EXPR, 10, 3)

register_token(OP_EQ, "==", token_without_value, STMT_NONE, 9)
register_token(OP_NEQ, "!=", token_without_value, STMT_NONE, 9)

register_token(OP_L, "<", token_without_value, STMT_NONE, 8)
register_token(OP_G, ">", token_without_value, STMT_NONE, 8)
register_token(OP_LE, "<=", token_without_value, STMT_NONE, 8)
register_token(OP_GE, ">=", token_without_value, STMT_NONE, 8)

register_token(OP_LEFT, "<<", token_without_value, STMT_NONE, 7)
register_token(OP_RIGHT, ">>", token_without_value, STMT_NONE, 7)

register_token(OP_SUB, "-", token_without_value, STMT_EXPR, 6, 3)
register_token(OP_ADD, "+", token_without_value, STMT_EXPR, 6, 3)

register_token(OP_MUL, "*", token_without_value, STMT_EXPR, 5, 3)
register_token(OP_DIV, "/", token_without_value, STMT_NONE, 5)
register_token(OP_MOD, "%", token_without_value, STMT_NONE, 5)

register_token(OP_NOT, "!", token_without_value, STMT_EXPR, 3)
register_token(OP_BIT_NOT, "~", token_without_value, STMT_EXPR, 3)

register_token(OP_DOT, ".", token_without_value, STMT_NONE, 2)
register_token(OP_ARROW, "->", token_without_value, STMT_NONE, 2)

register_token(OP_INC, "++", token_without_value, STMT_EXPR, 2, 3)
register_token(OP_DEC, "--", token_without_value, STMT_EXPR, 2, 3)

#ifndef PRIORITY_SET
register_token(COMMA, ",", token_without_value, STMT_NONE)
register_token(SEMICOLON, ";", token_without_value, STMT_EMPTY)
register_token(BRACE_OPEN, "{", token_without_value, STMT_BLOCK)
register_token(BRACE_CLOSE, "}", token_without_value, STMT_NONE)
register_token(BRACKET_OPEN, "(", token_without_value, STMT_EXPR)
register_token(BRACKET_CLOSE, ")", token_without_value, STMT_NONE)
register_token(SQR_BRACKET_OPEN, "[", token_without_value, STMT_NONE)
register_token(SQR_BRACKET_CLOSE, "]", token_without_value, STMT_NONE)
#endif
#endif

/*T_OP_ASSIGN,
	T_OP_DOT,
	T_OP_PTR,
	T_OP_NOT, 
	T_OP_INC,
	T_OP_DEC,
	T_OP_LEFT,
	T_OP_LEFT_ASSIGN,
	T_OP_RIGHT,
	T_OP_RIGHT_ASSIGN,
	T_OP_LE,
	T_OP_GE,
	T_OP_EQ,
	T_OP_NE,
	T_OP_AND,
	T_OP_OR,
	T_OP_MUL,
	T_OP_MUL_ASSIGN,
	T_OP_DIV,
	T_OP_DIV_ASSIGN,
	T_OP_ADD,
	T_OP_ADD_ASSIGN,
	T_OP_SUB,
	T_OP_SUB_ASSIGN,
	T_OP_MOD,
	T_OP_MOD_ASSIGN,
	T_OP_XOR,
	T_OP_XOR_ASSIGN,
	T_OP_BIT_AND,
	T_OP_BIT_AND_ASSIGN,
	T_OP_BIT_OR,
	T_OP_BIT_OR_ASSIGN,
	T_OP_BIT_NOT,
	T_OP_BIT_NOT_ASSIGN,*/