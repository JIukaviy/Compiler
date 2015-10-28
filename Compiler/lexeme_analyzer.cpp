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

automaton_commands_t commands[129][124];

map<string, TOKEN> keywords;
map<AUTOMATON_STATE, TOKEN> state_to_token;

#define TOKEN_FUNC
#include "token_register.h"
#undef TOKEN_FUNC

map<AUTOMATON_STATE, token_ptr_t(*)(string, AUTOMATON_STATE, int, int)> token_getters;

lexeme_analyzer_t::lexeme_analyzer_t(istream& is_) {
	is = &is_;
	state = AS_START;

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
		column = 0;
		line++;
	}
	cc = is->get();
	column++;
	
	if (cc == 255)
		cc = eof_code;
	if (cc < 0 || cc > 128)
		throw BadCC(line, column);
}

bool lexeme_analyzer_t::eof() {
	return is->eof();
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

token_ptr_t lexeme_analyzer_t::next() {
	if (eof())
		return curr_token = token_ptr_t(new token_t);
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
			case ACC_PREV: is->seekg((int)is->tellg() - 1); curr_str.pop_back(); break;
			case ACC_REMEMBER: add_char(); rem_carr_pos = is->tellg(); rem_line = line; rem_col = column; next_char(); break;
			case ACC_RETURN_TO_REM: {
				int p = (int)is->tellg() - rem_carr_pos - 1;
				if (cc == '\n')		// костыль
					p--;
				curr_str.erase(curr_str.end() - p - 1, curr_str.end());
				is->seekg(rem_carr_pos - 1);

				cc = is->get();

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

token_ptr_t lexeme_analyzer_t::get() {
	return curr_token;
}

token_ptr_t lexeme_analyzer_t::require(TOKEN first, ...) {
	if (get() == T_EMPTY)
		throw UnexpectedEOF();
	if (!get()->is(&first))
		throw UnexpectedToken(get(), first);
	token_ptr_t t = get();
	next();
	return t;
}

token_ptr_t lexeme_analyzer_t::require(token_ptr_t op, TOKEN first, ...) {
	if (get() == T_EMPTY)
		throw UnexpectedEOF();
	if (!get()->is(&first))
		throw UnexpectedToken(op, get(), first);
	token_ptr_t t = get();
	next();
	return t;
}

token_ptr_t lexeme_analyzer_t::require(set<TOKEN>& tokens) {
	if (get() == T_EMPTY)
		throw UnexpectedEOF();
	if (tokens.find(get()->get_token_id()) != tokens.end())
		throw UnexpectedToken(get(), tokens);
	token_ptr_t t = get();
	next();
	return t;
}

void lexeme_analyzer_init() {
#define register_token(incode_name, printed_name, func_name) token_getters[AS_END_##incode_name] = func_name; \
										 state_to_token[AS_END_##incode_name] = T_##incode_name;
#define TOKEN_LIST
#define AUTOMATON_STATE_DECLARATION
#include "token_register.h"
#undef AUTOMATON_STATE_DECLARATION
#undef TOKEN_LIST
#undef register_token

#define register_token(incode_name, printed_name, func_name) keywords[string(printed_name)] = T_##incode_name;
#define TOKEN_LIST
#define KEYWORD_REGISTRATION
#include "token_keyword.h"
#undef KEYWORD_REGISTRATION
#undef TOKEN_LIST
#undef register_token

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
	set('.', AS_NUMBER, AS_DOUBLE_FRACT_START);
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
	set('=', AS_START, AS_OP_ASSIGN__EQ);
	fill(true, AS_OP_ASSIGN__EQ, AS_END_OP_ASSIGN, ACC_STOP);
	set('=', AS_OP_ASSIGN__EQ, AS_END_OP_EQ);

	set('!', AS_START, AS_OP_NOT__NEQ);
	fill(true, AS_OP_NOT__NEQ, AS_END_OP_NOT, ACC_STOP);
	set('=', AS_OP_NOT__NEQ, AS_END_OP_NEQ);

	set('+', AS_START, AS_OP_ADD__ADD_ASSIGN__INC);
	fill(true, AS_OP_ADD__ADD_ASSIGN__INC, AS_END_OP_ADD, ACC_STOP);
	set('=', AS_OP_ADD__ADD_ASSIGN__INC, AS_END_OP_ADD_ASSIGN);
	set('+', AS_OP_ADD__ADD_ASSIGN__INC, AS_END_OP_INC);

	set('-', AS_START, AS_OP_SUB__SUB_ASSIGN__ARROW__DEC);
	fill(true, AS_OP_SUB__SUB_ASSIGN__ARROW__DEC, AS_END_OP_SUB, ACC_STOP);
	set('=', AS_OP_SUB__SUB_ASSIGN__ARROW__DEC, AS_END_OP_SUB_ASSIGN);
	set('>', AS_OP_SUB__SUB_ASSIGN__ARROW__DEC, AS_END_OP_ARROW);
	set('-', AS_OP_SUB__SUB_ASSIGN__ARROW__DEC, AS_END_OP_DEC);

	set('*', AS_START, AS_OP_MUL__MUL__ASSIGN);
	fill(true, AS_OP_MUL__MUL__ASSIGN, AS_END_OP_MUL, ACC_STOP);
	set('=', AS_OP_MUL__MUL__ASSIGN, AS_END_OP_MUL_ASSIGN);

	set('/', AS_START, AS_OP_DIV__DIV_ASSIGN);
	fill(true, AS_OP_DIV__DIV_ASSIGN, AS_END_OP_DIV, ACC_STOP);
	set('=', AS_OP_DIV__DIV_ASSIGN, AS_END_OP_DIV_ASSIGN);

	set('^', AS_START, AS_OP_XOR__XOR_ASSIGN);
	fill(true, AS_OP_XOR__XOR_ASSIGN, AS_END_OP_XOR, ACC_STOP);
	set('=', AS_OP_XOR__XOR_ASSIGN, AS_END_OP_XOR_ASSIGN);

	set('%', AS_START, AS_OP_MOD__MOD_ASSIGN);
	fill(true, AS_OP_MOD__MOD_ASSIGN, AS_END_OP_MOD, ACC_STOP);
	set('=', AS_OP_MOD__MOD_ASSIGN, AS_END_OP_MOD_ASSIGN);

	set('|', AS_START, AS_OP_BIT_OR__OR__BIT_OR_ASSIGN);
	fill(true, AS_OP_BIT_OR__OR__BIT_OR_ASSIGN, AS_END_OP_BIT_OR, ACC_STOP);
	set('=', AS_OP_BIT_OR__OR__BIT_OR_ASSIGN, AS_END_OP_BIT_OR_ASSIGN);
	set('|', AS_OP_BIT_OR__OR__BIT_OR_ASSIGN, AS_END_OP_OR);

	set('&', AS_START, AS_OP_BIT_AND__AND__BIT_AND_ASSIGN);
	fill(true, AS_OP_BIT_AND__AND__BIT_AND_ASSIGN, AS_END_OP_BIT_AND, ACC_STOP);
	set('=', AS_OP_BIT_AND__AND__BIT_AND_ASSIGN, AS_END_OP_BIT_AND_ASSIGN);
	set('&', AS_OP_BIT_AND__AND__BIT_AND_ASSIGN, AS_END_OP_AND);

	set('~', AS_START, AS_END_OP_BIT_NOT);

	set('<', AS_START, AS_OP_L__LE__LEFT);
	fill(true, AS_OP_L__LE__LEFT, AS_END_OP_L, ACC_STOP);
	set('=', AS_OP_L__LE__LEFT, AS_END_OP_LE);
	set('<', AS_OP_L__LE__LEFT, AS_OP_LEFT__LEFT_ASSIGN);
	fill(true, AS_OP_LEFT__LEFT_ASSIGN, AS_END_OP_LEFT, ACC_STOP);
	set('=', AS_OP_LEFT__LEFT_ASSIGN, AS_END_OP_LEFT_ASSIGN);

	set('>', AS_START, AS_OP_G__GE__RIGHT);
	fill(true, AS_OP_G__GE__RIGHT, AS_END_OP_G, ACC_STOP);
	set('=', AS_OP_G__GE__RIGHT, AS_END_OP_GE);
	set('>', AS_OP_G__GE__RIGHT, AS_OP_RIGHT__RIGHT_ASSIGN);
	fill(true, AS_OP_RIGHT__RIGHT_ASSIGN, AS_END_OP_RIGHT, ACC_STOP);
	set('=', AS_OP_RIGHT__RIGHT_ASSIGN, AS_END_OP_RIGHT_ASSIGN);

	set('?', AS_START, AS_END_QUESTION_MARK);
	set(':', AS_START, AS_END_COLON);

	set('.', AS_START, AS_END_OP_DOT);

	set('{', AS_START, AS_END_BRACE_OPEN);
	set('}', AS_START, AS_END_BRACE_CLOSE);
	set('(', AS_START, AS_END_BRACKET_OPEN);
	set(')', AS_START, AS_END_BRACKET_CLOSE);
	set('[', AS_START, AS_END_SQR_BRACKET_OPEN);
	set(']', AS_START, AS_END_SQR_BRACKET_CLOSE);
	set(';', AS_START, AS_END_SEMICOLON);
	set(',', AS_START, AS_END_COMMA);
}
