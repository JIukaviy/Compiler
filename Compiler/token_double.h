#ifdef TOKEN_FUNC
token_ptr token_double(string str, AUTOMATON_STATE state, int line, int column) {
	return token_ptr(new token_with_value_t<double>(line, column, T_DOUBLE, atof(str.c_str())));
}
#endif

#ifdef TOKEN_LIST
register_token(DOUBLE, "Double", token_double, STMT_EXPR)
#endif