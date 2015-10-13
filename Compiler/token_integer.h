#ifdef TOKEN_FUNC
token_t token_int(string str, AUTOMATON_STATE state, int line, int column) {
	value_t value;
	value.i = atol(str.c_str());
	return token_t(line, column, T_INTEGER, value);
}
#endif

#ifdef TOKEN_LIST
register_token(INTEGER, "integer", token_int)
#endif