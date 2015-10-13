#ifdef TOKEN_FUNC
token_t token_char(string str, AUTOMATON_STATE state, int line, int column) {
	value_t value;
	value.ch = str[1];
	return token_t(line, column, T_CHAR, value);
}
#endif

#ifdef TOKEN_LIST
register_token(CHAR, "char", token_char)
#endif