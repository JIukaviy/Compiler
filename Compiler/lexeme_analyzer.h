#pragma once

#include <iostream>
#include "tokens.h"

using namespace std;

enum TOKEN_TYPE {
	TT_IDENTIFIER,
	TT_OPERATOR,
	TT_INTEGER,
	TT_REAL,
	TT_CHAR,
	TT_STRING,
	TT_SEPARATOR
};

enum AUTOMATON_STATES {
	AS_START,

	AS_SPACE,

	AS_NEW_LINE,

	AS_NUMBER,
	AS_REAL1,
	AS_REAL2,
	AS_REAL3,
	AS_REAL4,
	AS_REAL5,

	AS_REAL_OR_OP,

	AS_HEX1,
	AS_HEX2,

	AS_WORD,

	AS_STRING1,
	AS_STRING2,

	AS_CHAR1,
	AS_CHAR2,
	AS_CHAR3,
	AS_CHAR4,

	AS_OPERATOR1,
	AS_OPERATOR2,
	AS_OPERATOR3,
	AS_OPERATOR4,
	AS_OPERATOR5,
	AS_OPERATOR6,
	AS_OPERATOR7,
	AS_OPERATOR8,
	AS_OPERATOR9,
	AS_OPERATOR10,

	AS_END_REACHED,

	AS_END_IDENTIFIER,
	AS_END_OPERATOR,
	AS_END_SEPARATOR,
	AS_END_CHAR,
	AS_END_STRING,
	AS_END_INTEGER,
	AS_END_REAL,
	AS_END_HEX,

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
	AUTOMATON_STATES state;
	AUTOMATON_CARRET_COMMAND carret_command;

	automaton_commands_t(AUTOMATON_STATES s, AUTOMATON_CARRET_COMMAND cc) : state(s), carret_command(cc) {};
	automaton_commands_t(AUTOMATON_STATES s) : state(s), carret_command(ACC_NEXT) {};
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

	friend ostream& operator<<(ostream& os, const LexemeAnalyzeError& e) {
		if (e.line >= 0)
			os << e.line << '\t' << e.column << '\t';
		os << e.error << endl;
		return os;
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
	istream* os;
	unsigned char cc = 0;
	int line = 1;
	int column = 0;
	int rem_carr_pos = -1;
	int rem_line = -1;
	int rem_col = -1;
	string curr_str;
	bool eof_reached = false;

	AUTOMATON_STATES state;

	void throw_exception(AUTOMATON_STATES);
	void add_char();
	void next_char();
public:
	lexeme_analyzer_t(istream& os_);
	token_container_t next();
	token_container_t get();
	bool eof();
};