#ifdef TOKEN_FUNC
token_ptr_t token_int(string str, AUTOMATON_STATE state, int line, int column) {
	return token_ptr_t(new token_with_value_t<int>(line, column, T_INTEGER, atol(str.c_str())));
}
#endif

#ifdef TOKEN_LIST
register_token(INTEGER, "Integer", token_int)
#endif