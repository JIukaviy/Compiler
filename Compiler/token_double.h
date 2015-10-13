#ifdef TOKEN_FUNC
token_t token_double(string str, AUTOMATON_STATE state, int line, int column) {
	value_t value;
	value.d = atof(str.c_str());
	return token_t(line, column, T_DOUBLE, value);
}
#endif

#ifdef TOKEN_LIST
register_token(DOUBLE, "double", token_double)
#endif