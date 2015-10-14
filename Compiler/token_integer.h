#ifdef TOKEN_FUNC
token_container_t token_int(string str, AUTOMATON_STATE state, int line, int column) {
	return token_container_t(new token_with_value_t<int>(line, column, T_INTEGER, atol(str.c_str())));
}
#endif

#ifdef TOKEN_LIST
register_token(INTEGER, "integer", token_int)
#endif