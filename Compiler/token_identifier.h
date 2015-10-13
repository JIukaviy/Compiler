#ifdef TOKEN_FUNC
token_t token_ident(string str, AUTOMATON_STATE state, int line, int column) {
	if (keywords.find(str) != keywords.end())
		return token_t(line, column, keywords[str]);
	else {
		value_t val;
		val.str = (char*)str.c_str();
		return token_t(line, column, T_IDENTIFIER, val);
	}
}
#endif

#ifdef TOKEN_LIST
register_token(IDENTIFIER, "ident", token_ident)
#endif