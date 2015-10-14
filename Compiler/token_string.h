#ifdef TOKEN_FUNC
token_container_t token_string(string str, AUTOMATON_STATE state, int line, int column) {
	string t_str = str;
	t_str.erase(t_str.begin());
	t_str.pop_back();
	return token_container_t(new token_with_value_t<string>(line, column, T_STRING, t_str));
}
#endif

#ifdef TOKEN_LIST
register_token(STRING, "string", token_string)
#endif