#pragma once

#include <iostream>
#include "tokens.h"
#include <set>
#include <sstream>

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

	AS_OP_ASSIGN__EQ,
	AS_OP_NOT__NEQ,
	AS_OP_ADD__ADD_ASSIGN__INC,
	AS_OP_SUB__SUB_ASSIGN__ARROW__DEC,
	AS_OP_MUL__MUL__ASSIGN,
	AS_OP_DIV__DIV_ASSIGN,
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
	LexemeAnalyzeError(int line_, int column_, char* what) : line(line_), column(column_), error(what) {};

	friend ostream& operator<<(ostream& is, const LexemeAnalyzeError& e) {
		if (e.line >= 0)
			is << e.line << '\t' << e.column << '\t';
		is << e.error << endl;
		return is;
	}
};

class BadNewLine : public LexemeAnalyzeError {
public:
	using LexemeAnalyzeError::LexemeAnalyzeError;
	BadNewLine(int line_, int column_) : LexemeAnalyzeError(line_, column_, "Unexpected new line") {}
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

class SyntaxError {
protected:
	stringstream err;
	token_ptr_t token;
	virtual void make_str() {};
public:
	SyntaxError(string what) : err(what) {};
	SyntaxError(string what, token_ptr_t token) : err(what), token(token) {};
	SyntaxError() : err("Unexpected error") {};

	friend ostream& operator<<(ostream& os, SyntaxError& e) {
		e.make_str();
		if (e.token)
			e.token->print_pos(os);
		os << e.err.str();
		return os;
	};

};

class UnexpectedToken : public SyntaxError {
public:
	token_ptr_t op;
	token_ptr_t actually;
	set<TOKEN> expected_tokens;
	TOKEN expected_token;

	UnexpectedToken() : SyntaxError("Unexpected token") {};
	UnexpectedToken(token_ptr_t actually, TOKEN expected) : actually(actually), expected_token(expected) {};
	UnexpectedToken(token_ptr_t op, token_ptr_t actually, TOKEN expected) : op(op), actually(actually), expected_token(expected) {};
	UnexpectedToken(token_ptr_t actually) : actually(actually), expected_token(T_EMPTY) {};
	UnexpectedToken(token_ptr_t actually, const set<TOKEN>& expected) : actually(actually), expected_tokens(expected), expected_token(T_EMPTY) {};
	UnexpectedToken(token_ptr_t op, token_ptr_t actually, const set<TOKEN>& expected) : op(op), actually(actually), expected_tokens(expected), expected_token(T_EMPTY) {};

	void make_str() override {
		if (op) {
			op->print_pos(err);
			err << "Operator \"" << op->get_name() << "\" ";
		} 
		if (expected_token != T_EMPTY || expected_tokens.size() != 0)
			err << "Instead of the \"" << actually->get_name() << '\"' << endl;
		else {
			if (op)
				err << "was not expecting \"" << actually->get_name() << '\"' << endl;
			else
				err << "Token \"" << actually->get_name() << "\" is not expected";
			return;
		}
		err << "was expecting ";
		if (expected_token != T_EMPTY)
			err << '\"' << token_t::get_name_by_id(expected_token) << '\"' << endl;
		else {
			err << "one of these tokens: " << endl;
			for (auto i = expected_tokens.begin(); i != expected_tokens.end(); ++i)
				err << " \"" << token_t::get_name_by_id(*i) << " \"\n" << endl;
		}
	}

	/*UnexpectedToken(const set<TOKEN>& expected) {
		err << "One of these tokens is expected: \n";
		for (auto i = expected.begin(); i != expected.end(); ++i)
			err << " \"" + token_t::get_name_by_id(*i) << " \"\n";
	};*/
};

class UnexpectedEOF : public SyntaxError {
public:
	UnexpectedEOF() : SyntaxError("Unexpected end of file"){};
};

class ExpressionIsExpected : public SyntaxError {
public:
	ExpressionIsExpected() : SyntaxError("Expression is expected") {};
	ExpressionIsExpected(token_ptr_t token) {
		token->print_pos(err);
		err << "Operator: \"" << token->get_name() << "\" expecting expression" << endl;
	};
};

/*class TokenIsExpected : public SyntaxError {
public:
	TokenIsExpected(TOKEN expected) {
		err = "Token \"" + token_t::get_name_by_id(expected) + "\" is expected";
	};
	TokenIsExpected(const set<TOKEN>& expected) {
		err = "Token \"" + token_t::get_name_by_id(expected) + "\" is expected";
	};
};*/

class CloseBracketExpected : public SyntaxError {
public:
	CloseBracketExpected() : SyntaxError("Close bracket is expected") {};
};

class InvalidCombinationOfSpecifiers : public SyntaxError {
public:
	InvalidCombinationOfSpecifiers(token_ptr_t token) : SyntaxError("Invalid combination of specifiers", token) {};
	InvalidCombinationOfSpecifiers() : SyntaxError("Invalid combination of specifiers") {};
};

class InvalidIncompleteType : public SyntaxError {
public:
	InvalidIncompleteType(token_ptr_t token) : SyntaxError("Invalid incomplete type", token) {};
	InvalidIncompleteType() : SyntaxError("Invalid incomplete type") {};
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
	token_ptr_t prev_token;
	token_ptr_t curr_token;
	bool eof_reached = false;

	AUTOMATON_STATE state;

	void throw_exception(AUTOMATON_STATE);
	void add_char();
	void next_char();
	void skip_spaces();
public:
	lexeme_analyzer_t(istream& is_);
	token_ptr_t next();
	token_ptr_t get();
	token_ptr_t require(TOKEN first, ...);
	token_ptr_t require(token_ptr_t op, TOKEN first, ...);
	token_ptr_t require(set<TOKEN>&);
	bool eof();
};