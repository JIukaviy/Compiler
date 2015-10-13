#include "lexeme_analyzer.h"
#include <map>
#include <set>

#define is_char(a) ((a) >= 'a' && (a) <= 'z' || (a) >= 'A' && (a) <= 'Z')
#define is_digit(a) ((a) >= '0' && (a) <= '9')
#define is_hex_digit(a) (is_digit(i) || (a) >= 'a' && (a) <= 'f' || (a) >= 'A' && (a) <= 'F')
#define is_new_line(a) ((a) == 10 || (a) == 13)
#define is_space(a) ((a) == ' ' || (a) == '\t')
#define eof_code 128
#define is_EOF(a) ((a) == EOF)
#define is_delimeter(a) ((a) == '.')

#define fill(condition, q, ...) \
			for (int i = 0; i < 129; i++) \
				if (condition)			\
					commands[i][q] = automaton_commands_t(__VA_ARGS__);

#define set(ch, q, ...) commands[ch][q] = automaton_commands_t(__VA_ARGS__);

#define register_token(token_name, func) token_getters[AS_END_##token_name] = func; \
										 state_to_token[AS_END_##token_name] = T_##token_name;

automaton_commands_t commands[129][124];

map<string, TOKEN> keywords;
map<AUTOMATON_STATE, TOKEN> state_to_token;

token_t token_ident(string str, AUTOMATON_STATE state, int line, int column) {
	if (keywords.find(str) != keywords.end())
		return token_t(line, column, keywords[str]);
	else {
		value_t val;
		val.str = (char*)str.c_str();
		return token_t(line, column, T_IDENTIFIER, val);
	}
}

token_t token_int(string str, AUTOMATON_STATE state, int line, int column) {
	value_t value;
	value.i = atol(str.c_str());
	return token_t(line, column, T_INTEGER, value);
}

token_t token_double(string str, AUTOMATON_STATE state, int line, int column) {
	value_t value;
	value.d = atof(str.c_str());
	return token_t(line, column, T_DOUBLE, value);
}

token_t token_without_value(string str, AUTOMATON_STATE state, int line, int column) {
	return token_t(line, column, state_to_token[state]);
}

token_t token_string(string str, AUTOMATON_STATE state, int line, int column) {
	string t_str = str;
	t_str.erase(t_str.begin());
	t_str.pop_back();
	value_t value;
	value.str = (char*)t_str.c_str();	// костыыыыыль
	return token_t(line, column, T_STRING, value);
}

token_t token_char(string str, AUTOMATON_STATE state, int line, int column) {
	value_t value;
	value.ch = str[1];
	return token_t(line, column, T_CHAR, value);
}

map<AUTOMATON_STATE, token_t(*)(string, AUTOMATON_STATE, int, int)> token_getters;

lexeme_analyzer_t::lexeme_analyzer_t(istream& os_) {
	os = &os_;
	state = AS_START;

	register_token(IDENTIFIER, token_ident);
	register_token(INTEGER, token_int);
	register_token(DOUBLE, token_double);
	register_token(STRING, token_string);
	register_token(CHAR, token_char);

	register_token(SEMICOLON, token_without_value);
	register_token(BRACE_OPEN, token_without_value);
	register_token(BRACE_CLOSE, token_without_value);
	register_token(COLON, token_without_value);
	register_token(COMMA, token_without_value);
	register_token(QUESTION_MARK, token_without_value);

	register_token(OP_BRACKET_OPEN, token_without_value);
	register_token(OP_BRACKET_CLOSE, token_without_value);
	register_token(OP_SQR_BRACKET_OPEN, token_without_value);
	register_token(OP_SQR_BRACKET_OPEN, token_without_value);
	register_token(OP_ASSIGN, token_without_value);
	register_token(OP_DOT, token_without_value);
	register_token(OP_PTR, token_without_value);
	register_token(OP_NOT, token_without_value);
	register_token(OP_INC, token_without_value);
	register_token(OP_DEC, token_without_value);
	register_token(OP_LEFT, token_without_value);
	register_token(OP_LEFT_ASSIGN, token_without_value);
	register_token(OP_RIGHT, token_without_value);
	register_token(OP_RIGHT_ASSIGN, token_without_value);
	register_token(OP_LE, token_without_value);
	register_token(OP_GE, token_without_value);
	register_token(OP_EQ, token_without_value);
	register_token(OP_NE, token_without_value);
	register_token(OP_AND, token_without_value);
	register_token(OP_OR, token_without_value);
	register_token(OP_MUL, token_without_value);
	register_token(OP_MUL_ASSIGN, token_without_value);
	register_token(OP_DIV, token_without_value);
	register_token(OP_DIV_ASSIGN, token_without_value);
	register_token(OP_ADD, token_without_value);
	register_token(OP_ADD_ASSIGN, token_without_value);
	register_token(OP_SUB, token_without_value);
	register_token(OP_SUB_ASSIGN, token_without_value);
	register_token(OP_MOD, token_without_value);
	register_token(OP_MOD_ASSIGN, token_without_value);
	register_token(OP_XOR, token_without_value);
	register_token(OP_XOR_ASSIGN, token_without_value);
	register_token(OP_BIT_AND, token_without_value);
	register_token(OP_BIT_AND_ASSIGN, token_without_value);
	register_token(OP_BIT_OR, token_without_value);
	register_token(OP_BIT_OR_ASSIGN, token_without_value);
	register_token(OP_BIT_NOT, token_without_value);
	register_token(OP_BIT_NOT_ASSIGN, token_without_value);

	keywords[string("char")] = T_KWRD_CHAR;
	keywords[string("const")] = T_KWRD_CONST;
	keywords[string("continue")] = T_KWRD_CONTINUE;
	keywords[string("double")] = T_KWRD_DOUBLE;
	keywords[string("else")] = T_KWRD_ELSE;
	keywords[string("float")] = T_KWRD_FLOAT;
	keywords[string("for")] = T_KWRD_FOR;
	keywords[string("if")] = T_KWRD_IF;
	keywords[string("int")] = T_KWRD_INT;
	keywords[string("long")] = T_KWRD_LONG;
	keywords[string("return")] = T_KWRD_RETURN;
	keywords[string("short")] = T_KWRD_SHORT;
	keywords[string("signed")] = T_KWRD_SIGNED;
	keywords[string("sizeof")] = T_KWRD_SIZEOF;
	keywords[string("unsigned")] = T_KWRD_UNSIGNED;
	keywords[string("void")] = T_KWRD_VOID;
	keywords[string("while")] = T_KWRD_WHILE;
	
	set(eof_code, AS_START, AS_END_REACHED, ACC_STOP);
	//-----------SPACE/NEW_LINE----------------
	fill(is_space(i), AS_START, AS_SPACE, ACC_SKIP);
	fill(is_space(i), AS_SPACE, AS_SPACE, ACC_SKIP);
	fill(!is_space(i), AS_SPACE, AS_START, ACC_STOP);
	set('\n', AS_START, AS_SPACE, ACC_SKIP);
	set('\n', AS_SPACE, AS_SPACE, ACC_SKIP);
	set(eof_code, AS_SPACE, AS_END_REACHED);

	//-----------IDENTIFIER----------------
	fill(is_char(i), AS_START, AS_IDENTIFIER);
	fill(is_char(i) || is_digit(i), AS_IDENTIFIER, AS_IDENTIFIER);
	fill(!is_char(i) && !is_digit(i), AS_IDENTIFIER, AS_END_IDENTIFIER, ACC_STOP);

	//--------------CHAR----------------
	set('\'', AS_START, AS_CHAR1);
	fill(true, AS_CHAR1, AS_CHAR2);
	set('\'', AS_CHAR1, AS_ERR_CHAR_TS, ACC_STOP);
	fill(true, AS_CHAR2, AS_ERR_CHAR_TL, ACC_STOP);
	set('\'', AS_CHAR2, AS_END_CHAR);
	set(eof_code, AS_CHAR1, AS_ERR_BAD_EOF, ACC_STOP);
	set(eof_code, AS_CHAR2, AS_ERR_BAD_EOF, ACC_STOP);
	fill(is_new_line(i), AS_CHAR1, AS_ERR_BAD_NL, ACC_STOP);
	fill(is_new_line(i), AS_CHAR2, AS_ERR_BAD_NL, ACC_STOP);

	//--------------STRING----------------
	set('\"', AS_START, AS_STRING1);
	fill(true, AS_STRING1, AS_STRING1);
	set(eof_code, AS_STRING1, AS_ERR_BAD_EOF, ACC_STOP);
	fill(is_new_line(i), AS_STRING1, AS_ERR_BAD_NL, ACC_STOP);
	set('\"', AS_STRING1, AS_END_STRING, ACC_NEXT);

	//-----------NUMBER(INTEGER, REAL)----------------
	fill(is_digit(i), AS_START, AS_NUMBER);
	fill(is_digit(i), AS_NUMBER, AS_NUMBER);
	fill(!is_digit(i), AS_NUMBER, AS_END_INTEGER, ACC_STOP);
	set('.', AS_NUMBER, AS_DOUBLE_FRACT);
	//-------------------REAL-----------------------
	fill(is_digit(i), AS_DOUBLE_FRACT_START, AS_DOUBLE_FRACT);
	fill(!is_digit(i), AS_DOUBLE_FRACT_START, AS_ERR_NO_FRACT, ACC_STOP);
	fill(is_digit(i), AS_DOUBLE_FRACT, AS_DOUBLE_FRACT);
	fill(!is_digit(i), AS_DOUBLE_FRACT, AS_END_DOUBLE, ACC_STOP);
	set('E', AS_DOUBLE_FRACT, AS_DOUBLE_EXP_START);
	set('e', AS_DOUBLE_FRACT, AS_DOUBLE_EXP_START);
	fill(is_digit(i), AS_DOUBLE_EXP, AS_DOUBLE_EXP);
	fill(!is_digit(i), AS_DOUBLE_EXP, AS_END_DOUBLE, ACC_STOP);
	fill(is_digit(i), AS_DOUBLE_EXP_START, AS_DOUBLE_EXP);
	fill(!is_digit(i), AS_DOUBLE_EXP_START, AS_ERR_NO_EXP, ACC_STOP);
	set('+', AS_DOUBLE_EXP_START, AS_DOUBLE_EXP_AFTER_SIGN);
	set('-', AS_DOUBLE_EXP_START, AS_DOUBLE_EXP_AFTER_SIGN);
	fill(is_digit(i), AS_DOUBLE_EXP_AFTER_SIGN, AS_DOUBLE_EXP);
	fill(!is_digit(i), AS_DOUBLE_EXP_AFTER_SIGN, AS_ERR_NO_EXP, ACC_STOP);

	//------------------OPERATORS/SEPARATORS---------------
	set('=', AS_START, AS_END_OP_ASSIGN);

	set('+', AS_START, AS_OP_ADD_OR_ASSIGN_ADD);
	fill(i != '+', AS_OP_ADD_OR_ASSIGN_ADD, AS_END_OP_ADD, ACC_STOP);
	set('=', AS_OP_ADD_OR_ASSIGN_ADD, AS_END_OP_ADD_ASSIGN);

	set('-', AS_START, AS_OP_SUB_OR_ASSIGN_SUB);
	fill(i != '-', AS_OP_SUB_OR_ASSIGN_SUB, AS_END_OP_SUB, ACC_STOP);
	set('=', AS_OP_SUB_OR_ASSIGN_SUB, AS_END_OP_SUB_ASSIGN);

	set('*', AS_START, AS_OP_MUL_OR_ASSIGN_MUL);
	fill(i != '*', AS_OP_MUL_OR_ASSIGN_MUL, AS_END_OP_MUL, ACC_STOP);
	set('=', AS_OP_MUL_OR_ASSIGN_MUL, AS_END_OP_MUL_ASSIGN);

	set('/', AS_START, AS_OP_DIV_OR_ASSIGN_DIV);
	fill(i != '/', AS_OP_DIV_OR_ASSIGN_DIV, AS_END_OP_DIV, ACC_STOP);
	set('=', AS_OP_DIV_OR_ASSIGN_DIV, AS_END_OP_DIV_ASSIGN);

	set('^', AS_START, AS_OP_XOR_OR_ASSIGN_XOR);
	fill(i != '^', AS_OP_XOR_OR_ASSIGN_XOR, AS_END_OP_XOR, ACC_STOP);
	set('=', AS_OP_XOR_OR_ASSIGN_XOR, AS_END_OP_XOR_ASSIGN);

	set('<', AS_START, AS_OP_L_OR_LE);
	fill(i != '<', AS_OP_L_OR_LE, AS_END_OP_L, ACC_STOP);
	set('=', AS_OP_L_OR_LE, AS_END_OP_LE);

	set('>', AS_START, AS_OP_G_OR_GE);
	fill(i != '>', AS_OP_G_OR_GE, AS_END_OP_G, ACC_STOP);
	set('=', AS_OP_G_OR_GE, AS_END_OP_GE);

	set(':', AS_START, AS_END_COLON);

	set('.', AS_START, AS_END_OP_DOT);

	set('(', AS_START, AS_END_OP_BRACKET_OPEN);
	set(')', AS_START, AS_END_OP_BRACKET_CLOSE);
	set('[', AS_START, AS_END_OP_SQR_BRACKET_OPEN);
	set(']', AS_START, AS_END_OP_SQR_BRACKET_CLOSE);
	set(';', AS_START, AS_END_SEMICOLON);
	set(',', AS_START, AS_END_COMMA);

	next_char();
	skip_spaces();
}

void lexeme_analyzer_t::add_char() {
	curr_str += cc;
}

void lexeme_analyzer_t::next_char() {
	if (cc == '\t') {
		column = column == 0 ? 1 : column;
		int n = ceil(column / 4.0);
		column = 4 * n;
	}
	if (cc == '\n') {
		column = 1;
		line++;
	}
	cc = os->get();
	column++;
	
	if (cc == 255)
		cc = eof_code;
	if (cc < 0 || cc > 128)
		throw BadCC(line, column);
}

bool lexeme_analyzer_t::eof() {
	return os->eof();
}

void lexeme_analyzer_t::throw_exception(AUTOMATON_STATE state) {
	switch (state) {
		case AS_END_REACHED: throw EOFReached(line, column); break;
		case AS_ERR_BAD_CC: throw BadCC(line, column); break;
		case AS_ERR_BAD_CHAR: throw BadChar(line, column); break;
		case AS_ERR_BAD_EOF: throw BadEOF(line, column); break;
		case AS_ERR_BAD_NL: throw BadNewLine(line, column); break;
		case AS_ERR_CHAR_TL: throw BadChar(line, column); break;
		case AS_ERR_CHAR_TS: throw BadChar(line, column); break;
		case AS_ERR_NO_CC: throw NoCC(line, column); break;
		case AS_ERR_NO_EXP: throw NoExp(line, column); break;
		case AS_ERR_NO_FRACT: throw NoFract(line, column); break;
		case AS_ERR_NO_HEX: throw NoHex(line, column); break;
	}
}

void lexeme_analyzer_t::skip_spaces() {
	state = AS_START;
	while ((state = commands[cc][state].state) == AS_SPACE) {
		 next_char();
	}
}

token_t lexeme_analyzer_t::next() {
	state = AS_START;
	curr_str.clear();
	int start_line;
	int start_column;
	while (state < AS_END_REACHED) {
		if (state == AS_START) {
			start_line = line;
			start_column = column;
		}
		AUTOMATON_CARRET_COMMAND carret_command = commands[cc][state].carret_command;
		state = commands[cc][state].state;
		switch (carret_command) {
			case ACC_NEXT: add_char(); next_char(); break;
			case ACC_PREV: os->seekg((int)os->tellg() - 1); curr_str.pop_back(); break;
			case ACC_REMEMBER: add_char(); rem_carr_pos = os->tellg(); rem_line = line; rem_col = column; next_char(); break;
			case ACC_RETURN_TO_REM: {
				int p = (int)os->tellg() - rem_carr_pos - 1;
				if (cc == '\n')		// костыль
					p--;
				curr_str.erase(curr_str.end() - p - 1, curr_str.end());
				os->seekg(rem_carr_pos - 1);

				cc = os->get();

				line = rem_line;
				column = rem_col;
			} break;
			case ACC_SKIP: next_char(); break;
		}
	}

	throw_exception(state);

	curr_token = token_getters[state](curr_str, state, start_line, start_column);
	skip_spaces();
	return curr_token;
}

token_t lexeme_analyzer_t::get() {
	return curr_token;
}
