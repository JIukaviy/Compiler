#ifdef TOKEN_FUNC
token_container_t token_double(string str, AUTOMATON_STATE state, int line, int column) {
	return token_container_t(new token_with_value_t<double>(line, column, T_DOUBLE, atof(str.c_str())));
}
#endif

#ifdef TOKEN_LIST
register_token(DOUBLE, "double", token_double)
#endif