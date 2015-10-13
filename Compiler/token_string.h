#ifdef TOKEN_FUNC
token_t token_string(string str, AUTOMATON_STATE state, int line, int column) {
	string t_str = str;
	t_str.erase(t_str.begin());
	t_str.pop_back();
	value_t value;
	value.str = (char*)t_str.c_str();	// ךמסעûûûûûכü
	return token_t(line, column, T_STRING, value);
}
#endif

#ifdef TOKEN_LIST
register_token(STRING, "string", token_string)
#endif