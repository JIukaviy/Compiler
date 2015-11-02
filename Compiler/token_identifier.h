#ifdef TOKEN_FUNC
token_ptr_t token_ident(string str, AUTOMATON_STATE state, int line, int column) {
	if (keywords.find(str) != keywords.end())
		return token_ptr_t(new token_t(line, column, keywords[str]));
	else {
		return token_ptr_t(new token_with_value_t<string>(line, column, T_IDENTIFIER, str));
	}
}
#endif

#ifdef TOKEN_LIST
register_token(IDENTIFIER, "Identifier", token_ident)
#endif