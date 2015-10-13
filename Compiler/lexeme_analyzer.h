#pragma once

#include <iostream>
#include "tokens.h"

using namespace std;

#define register_token(incode_name, printed_name, func_name) AS_END_##incode_name,
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

	AS_OP_ADD_OR_ASSIGN_ADD,
	AS_OP_SUB_OR_ASSIGN_SUB,
	AS_OP_MUL_OR_ASSIGN_MUL,
	AS_OP_DIV_OR_ASSIGN_DIV,
	AS_OP_XOR_OR_ASSIGN_XOR,
	AS_OP_L_OR_LE,
	AS_OP_G_OR_GE,

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
	ACC_RETURN_TO_REM
};

struct automaton_commands_t {
	AUTOMATON_STATE state;
	AUTOMATON_CARRET_COMMAND carret_command;

	automaton_commands_t(AUTOMATON_STATE s, AUTOMATON_CARRET_COMMAND cc) : state(s), carret_command(cc) {};
	automaton_commands_t(AUTOMATON_STATE s) : state(s), carret_command(ACC_NEXT) {};
	automaton_commands_t() : state(AS_ERR_BAD_CHAR), carret_command(ACC_STOP) {};
};

class LexemeAnalyzeError {
protected:
	virtual char* error_str() const {
		return "Error";
	}
	string error;
public:
	int line = -1;
	int column = -1;

	LexemeAnalyzeError() : line(-1), column(-1) { error = error_str(); };
	LexemeAnalyzeError(int line_, int column_) : line(line_), column(column_) { error = error_str(); }

	friend ostream& operator<<(ostream& is, const LexemeAnalyzeError& e) {
		if (e.line >= 0)
			is << e.line << '\t' << e.column << '\t';
		is << e.error << endl;
		return is;
	}
};

class BadNewLine : public LexemeAnalyzeError {
protected:
	char* error_str() const override {
		return "BadNL";
	}
public:
	BadNewLine(int line_, int column_) : LexemeAnalyzeError(line_, column_) { error = error_str(); }
	BadNewLine() : LexemeAnalyzeError() { error = error_str(); }
};

class BadEOF : public LexemeAnalyzeError {
protected:
	char* error_str() const override {
		return "BadEOF";
	}
public:
	BadEOF(int line_, int column_) : LexemeAnalyzeError(line_, column_) { error = error_str(); }
	BadEOF() : LexemeAnalyzeError() { error = error_str(); }
};

class BadChar : public LexemeAnalyzeError {
protected:
	char* error_str() const override {
		return "BadChar";
	}
public:
	BadChar(int line_, int column_) : LexemeAnalyzeError(line_, column_) { error = error_str(); }
	BadChar() : LexemeAnalyzeError() { error = error_str(); }
};

class NoExp : public LexemeAnalyzeError {
protected:
	char* error_str() const override {
		return "NoExp";
	}
public:
	NoExp(int line_, int column_) : LexemeAnalyzeError(line_, column_) { error = error_str(); }
	NoExp() : LexemeAnalyzeError() { error = error_str(); }
};

class NoFract : public LexemeAnalyzeError {
protected:
	char* error_str() const override {
		return "NoFract";
	}
public:
	NoFract(int line_, int column_) : LexemeAnalyzeError(line_, column_) { error = error_str(); }
	NoFract() : LexemeAnalyzeError() { error = error_str(); }
};

class NoHex : public LexemeAnalyzeError {
protected:
	char* error_str() const override {
		return "NoHex";
	}
public:
	NoHex(int line_, int column_) : LexemeAnalyzeError(line_, column_) { error = error_str(); }
	NoHex() : LexemeAnalyzeError() { error = error_str(); }
};

class NoCC : public LexemeAnalyzeError {
protected:
	char* error_str() const override {
		return "NoCC";
	}
public:
	NoCC(int line_, int column_) : LexemeAnalyzeError(line_, column_) { error = error_str(); }
	NoCC() : LexemeAnalyzeError() { error = error_str(); }
};

class BadCC : public LexemeAnalyzeError {
protected:
	char* error_str() const override {
		return "BadCC";
	}
public:
	BadCC(int line_, int column_) : LexemeAnalyzeError(line_, column_) { error = error_str(); }
	BadCC() : LexemeAnalyzeError() { error = error_str(); }
};

class EOFReached : public LexemeAnalyzeError {
protected:
	char* error_str() const override {
		return "End of file reached";
	}
public:
	EOFReached(int line_, int column_) : LexemeAnalyzeError(line_, column_) { error = error_str(); }
	EOFReached() : LexemeAnalyzeError() { error = error_str(); }
};

class lexeme_analyzer_t {
protected:
	istream* is;
	unsigned char cc = 0;
	int line = 1;
	int column = 0;
	int rem_carr_pos = -1;
	int rem_line = -1;
	int rem_col = -1;
	string curr_str;
	token_t prev_token;
	token_t curr_token;
	bool eof_reached = false;

	AUTOMATON_STATE state;

	void throw_exception(AUTOMATON_STATE);
	void add_char();
	void next_char();
	void skip_spaces();
public:
	lexeme_analyzer_t(istream& is_);
	token_t next();
	token_t get();
	bool eof();
};