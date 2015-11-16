#pragma once

#include <iostream>
#include "tokens.h"
#include <set>
#include "exceptions.h"

using namespace std;

#define register_token(incode_name, printed_name, func_name, statement, ...) AS_END_##incode_name,
#define AUTOMATON_STATE_DECLARATION
#define TOKEN_LIST

void lexeme_analyzer_init();

enum AUTOMATON_STATE {
	AS_START,

	AS_SPACE,

	AS_NEW_LINE,

	AS_NUMBER,
	AS_DOUBLE_FRACT_START,
	AS_DOUBLE_FRACT,
	AS_DOUBLE_EXP_START,
	AS_DOUBLE_EXP,
	AS_DOUBLE_EXP_AFTER_SIGN,

	AS_IDENTIFIER,

	AS_STRING1,
	AS_STRING2,

	AS_CHAR1,
	AS_CHAR2,
	AS_CHAR3,
	AS_CHAR4,

	AS_COMMENT,
	AS_MUL_COMMENT,
	AS_MUL_COMMENT_END,

	AS_OP_ASSIGN__EQ,
	AS_OP_NOT__NEQ,
	AS_OP_ADD__ADD_ASSIGN__INC,
	AS_OP_SUB__SUB_ASSIGN__ARROW__DEC,
	AS_OP_MUL__MUL__ASSIGN,
	AS_OP_DIV__DIV_ASSIGN__COMMENT,
	AS_OP_XOR__XOR_ASSIGN,
	AS_OP_L__LE__LEFT,
	AS_OP_LEFT__LEFT_ASSIGN,
	AS_OP_G__GE__RIGHT,
	AS_OP_RIGHT__RIGHT_ASSIGN,
	AS_OP_MOD__MOD_ASSIGN,
	AS_OP_BIT_OR__OR__BIT_OR_ASSIGN,
	AS_OP_BIT_AND__AND__BIT_AND_ASSIGN,

	AS_END_REACHED,

#include "token_register.h"

	AS_ERR_BAD_NL,
	AS_ERR_BAD_EOF,
	AS_ERR_BAD_CHAR,
	AS_ERR_NO_EXP,
	AS_ERR_NO_FRACT,
	AS_ERR_NO_HEX,
	AS_ERR_NO_CC,
	AS_ERR_BAD_CC,

	AS_ERR_CHAR_TS, // המכזום בûעü סטלגמכ
	AS_ERR_CHAR_TL  // במכüרו מהםמדמ סטלגמכא
};

#undef TOKEN_LIST
#undef AUTOMATON_STATE_DECLARATION
#undef register_token

enum AUTOMATON_CARRET_COMMAND {
	ACC_SKIP,
	ACC_PREV,
	ACC_NEXT,
	ACC_STOP,
	ACC_NEW_LINE,
	ACC_REMEMBER,
	ACC_RETURN_TO_REM,
	ACC_SKIP_AND_ERASE
};

struct automaton_commands_t {
	AUTOMATON_STATE state;
	AUTOMATON_CARRET_COMMAND carret_command;

	automaton_commands_t(AUTOMATON_STATE s, AUTOMATON_CARRET_COMMAND cc) : state(s), carret_command(cc) {};
	automaton_commands_t(AUTOMATON_STATE s) : state(s), carret_command(ACC_NEXT) {};
	automaton_commands_t() : state(AS_ERR_BAD_CHAR), carret_command(ACC_STOP) {};
};

class lexeme_analyzer_t {
protected:
	istream* is;
	unsigned char cc = 0;
	pos_t curr_pos;
	pos_t rem_pos;
	int rem_carr_pos = -1;
	string curr_str;
	token_ptr prev_token;
	token_ptr curr_token;
	bool eof_reached = false;

	AUTOMATON_STATE state;

	void throw_exception(AUTOMATON_STATE);
	void add_char();
	void next_char();
	void skip_spaces();
public:
	lexeme_analyzer_t(istream& is_);
	token_ptr next();
	token_ptr get();
	token_ptr require(TOKEN first, ...);
	token_ptr require(token_ptr op, TOKEN first, ...);
	token_ptr require(set<TOKEN>&);
	bool eof();
};