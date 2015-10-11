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

automaton_commands_t commands[129][50];

set<string> keywords;

token_container_t token_ident(string str, int line, int column) {
	if (keywords.find(str) == keywords.end())
		return token_container_t(new token_identifier_t(line, column, str));
	else
		return token_container_t(new token_keyword_t(line, column, str));
}

token_container_t token_int(string str, int line, int column) {
	return token_container_t(new token_integer_t(line, column, str, atol(str.c_str())));
}

token_container_t token_real(string str, int line, int column) {
	return token_container_t(new token_float_t(line, column, str, atof(str.c_str())));
}

token_container_t token_operator(string str, int line, int column) {
	return token_container_t(new token_operator_t(line, column, str));
}

token_container_t token_separator(string str, int line, int column) {
	return token_container_t(new token_separator_t(line, column, str));
}

token_container_t token_string(string str, int line, int column) {
	string t_str = str;
	t_str.erase(t_str.begin());
	t_str.pop_back();
	return token_container_t(new token_string_t(line, column, str, t_str));
}

token_container_t token_char(string str, int line, int column) {
	return token_container_t(new token_char_t(line, column, str, str[1]));
}


map<AUTOMATON_STATES, token_container_t(*)(string, int, int)> token_getters;

lexeme_analyzer_t::lexeme_analyzer_t(istream& os_) {
	os = &os_;
	state = AS_START;

	token_getters[AS_END_IDENTIFIER] = token_ident;
	token_getters[AS_END_INTEGER] = token_int;
	token_getters[AS_END_REAL] = token_real;
	token_getters[AS_END_OPERATOR] = token_operator;
	token_getters[AS_END_SEPARATOR] = token_separator;
	token_getters[AS_END_STRING] = token_string;
	token_getters[AS_END_CHAR] = token_char;

	keywords.insert(string("begin"));
	keywords.insert(string("forward"));
	keywords.insert(string("do"));
	keywords.insert(string("else"));
	keywords.insert(string("end"));
	keywords.insert(string("for"));
	keywords.insert(string("function"));
	keywords.insert(string("if"));
	keywords.insert(string("array"));
	keywords.insert(string("of"));
	keywords.insert(string("procedure"));
	keywords.insert(string("program"));
	keywords.insert(string("record"));
	keywords.insert(string("then"));
	keywords.insert(string("to"));
	keywords.insert(string("type"));
	keywords.insert(string("var"));
	keywords.insert(string("while"));
	keywords.insert(string("break"));
	keywords.insert(string("continue"));
	keywords.insert(string("downto"));
	keywords.insert(string("exit"));
	keywords.insert(string("repeat"));
	keywords.insert(string("until"));

	
	set(eof_code, AS_START, AS_END_REACHED, ACC_STOP);
	//-----------SPACE----------------
	fill(is_space(i), AS_START, AS_SPACE, ACC_SKIP);
	fill(is_space(i), AS_SPACE, AS_SPACE, ACC_SKIP);
	fill(!is_space(i), AS_SPACE, AS_START, ACC_STOP);
	set(eof_code, AS_SPACE, AS_END_REACHED);

	//------------NEW_LINE---------------------
	set('\n', AS_START, AS_START, ACC_NEW_LINE);

	//-----------IDENTIFIER----------------
	fill(is_char(i), AS_START, AS_WORD);
	fill(is_char(i) || is_digit(i), AS_WORD, AS_WORD);
	fill(!is_char(i) && !is_digit(i), AS_WORD, AS_END_IDENTIFIER, ACC_STOP);

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
	set('.', AS_NUMBER, AS_REAL_OR_OP, ACC_REMEMBER);
	fill(true, AS_REAL_OR_OP, AS_REAL1, ACC_STOP);
	set('.', AS_REAL_OR_OP, AS_END_INTEGER, ACC_RETURN_TO_REM);
	//-------------------REAL-----------------------
	fill(is_digit(i), AS_REAL1, AS_REAL2);
	fill(!is_digit(i), AS_REAL1, AS_ERR_NO_FRACT, ACC_STOP);
	fill(is_digit(i), AS_REAL2, AS_REAL2);
	fill(!is_digit(i), AS_REAL2, AS_END_REAL, ACC_STOP);
	set('E', AS_REAL2, AS_REAL3);
	set('e', AS_REAL2, AS_REAL3);
	fill(is_digit(i), AS_REAL4, AS_REAL4);
	fill(!is_digit(i), AS_REAL4, AS_END_REAL, ACC_STOP);
	fill(is_digit(i), AS_REAL3, AS_REAL4);
	fill(!is_digit(i), AS_REAL3, AS_ERR_NO_EXP, ACC_STOP);
	set('+', AS_REAL3, AS_REAL5);
	set('-', AS_REAL3, AS_REAL5);
	fill(is_digit(i), AS_REAL5, AS_REAL4);
	fill(!is_digit(i), AS_REAL5, AS_ERR_NO_EXP, ACC_STOP);

	//------------------HEX---------------------
	set('$', AS_START, AS_HEX1);
	fill(is_hex_digit(i), AS_HEX1, AS_HEX2);
	fill(!is_hex_digit(i), AS_HEX1, AS_ERR_NO_HEX, ACC_STOP);
	fill(!is_hex_digit(i), AS_HEX2, AS_END_HEX, ACC_STOP);

	//------------------OPERATORS/SEPARATORS---------------
	set('=', AS_START, AS_END_OPERATOR);

	set('+', AS_START, AS_OPERATOR1);
	fill(i != '+', AS_OPERATOR1, AS_END_OPERATOR, ACC_STOP);
	set('=', AS_OPERATOR1, AS_END_OPERATOR);

	set('-', AS_START, AS_OPERATOR2);
	fill(i != '-', AS_OPERATOR2, AS_END_OPERATOR, ACC_STOP);
	set('=', AS_OPERATOR2, AS_END_OPERATOR);

	set('*', AS_START, AS_OPERATOR3);
	fill(i != '*', AS_OPERATOR3, AS_END_OPERATOR, ACC_STOP);
	set('=', AS_OPERATOR3, AS_END_OPERATOR);

	set('/', AS_START, AS_OPERATOR4);
	fill(i != '/', AS_OPERATOR4, AS_END_OPERATOR, ACC_STOP);
	set('=', AS_OPERATOR4, AS_END_OPERATOR);

	set('^', AS_START, AS_OPERATOR5);
	fill(i != '^', AS_OPERATOR5, AS_END_OPERATOR, ACC_STOP);
	set('=', AS_OPERATOR5, AS_END_OPERATOR);

	set('<', AS_START, AS_OPERATOR6);
	fill(i != '<', AS_OPERATOR6, AS_END_OPERATOR, ACC_STOP);
	set('=', AS_OPERATOR6, AS_END_OPERATOR);
	set('>', AS_OPERATOR6, AS_END_OPERATOR);

	set('>', AS_START, AS_OPERATOR7);
	fill(i != '>', AS_OPERATOR7, AS_END_OPERATOR, ACC_STOP);
	set('=', AS_OPERATOR7, AS_END_OPERATOR);

	set(':', AS_START, AS_OPERATOR8);
	fill(i != ':', AS_OPERATOR8, AS_END_OPERATOR, ACC_STOP);
	set('=', AS_OPERATOR8, AS_END_SEPARATOR);

	set('@', AS_START, AS_END_OPERATOR);

	set('.', AS_START, AS_OPERATOR9);
	fill(i != '.', AS_OPERATOR9, AS_END_OPERATOR, ACC_STOP);
	set('.', AS_OPERATOR9, AS_END_SEPARATOR);

	set('(', AS_START, AS_END_SEPARATOR);
	set(')', AS_START, AS_END_SEPARATOR);
	set('[', AS_START, AS_END_SEPARATOR);
	set(']', AS_START, AS_END_SEPARATOR);
	set(';', AS_START, AS_END_SEPARATOR);
	set(':', AS_START, AS_END_SEPARATOR);
	set(',', AS_START, AS_END_SEPARATOR);

	next_char();
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

void lexeme_analyzer_t::throw_exception(AUTOMATON_STATES state) {
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

token_container_t lexeme_analyzer_t::next() {
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
			case ACC_NEW_LINE: next_char(); line++; column = 1; break;
			case ACC_SKIP: next_char(); break;
		}
	}

	throw_exception(state);

	return token_getters[state](curr_str, start_line, start_column);
}