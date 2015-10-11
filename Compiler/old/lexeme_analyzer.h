#pragma once
#include <vector>
#include <iostream>
#include "tokens.h"

using namespace std;

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

enum result { NONE, CONTINUE, OK_OR_CONTINUE, NOT_MINE, OK };

/*---------------------Lexemes--------------------------*/


class token_finder_t {
protected:
	int priority = 1;
	int state = 0;
	void next_state() { state++; }
	string* str;
public:
	token_finder_t(string* str_) : str(str_) {};
	virtual result is_mine(char a) = 0;
	virtual void clear_state() { state = 0; }
	int get_priority() { return priority; }

	virtual token_container_t get_token(int line, int column);

	bool operator<(const token_finder_t& e) const {
		return priority > e.priority;
	}
};

class token_int_find_t : public token_finder_t {
public:
	token_int_find_t(string* str_) : token_finder_t(str_) { priority = 2; }
	result is_mine(char a) override;
	token_container_t get_token(int line, int column) override;
};

class token_real_find_t : public token_finder_t {
protected:
	double value;
public:
	token_real_find_t(string* str_) : token_finder_t(str_) { priority = 1; }
	result is_mine(char a) override;
	token_container_t get_token(int line, int column) override;
};

class token_ident_find_t : public token_finder_t {
public:
	token_ident_find_t(string* str_) : token_finder_t(str_) { priority = 1; }
	result is_mine(char a) override;
	token_container_t get_token(int line, int column) override;
};

class token_keyword_find_t : public token_finder_t {
protected:
	vector<string> keywords;
	int first;
	int last;
	int curr_pos;
	bool finded;
	void clear_state();
public:
	token_keyword_find_t(string* str_, vector<string> keywords_) : token_finder_t(str_), keywords(keywords_) { priority = 2; clear_state(); }
	result is_mine(char a) override;
	token_container_t get_token(int line, int column) override;
};

class token_operator_find_t : public token_keyword_find_t {
public:
	token_operator_find_t(string* str_, vector<string> keywords_) : token_keyword_find_t(str_, keywords_) { priority = 2; clear_state(); }
	token_container_t get_token(int line, int column) override;
};

class token_separator_find_t : public token_keyword_find_t {
public:
	token_separator_find_t(string* str_, vector<string> keywords_) : token_keyword_find_t(str_, keywords_) { priority = 1; clear_state(); }
	token_container_t get_token(int line, int column) override;
};

class token_char_find_t : public token_finder_t {
protected:
	char value;
public:
	token_char_find_t(string* str_) : token_finder_t(str_) { priority = 1; }
	result is_mine(char a) override;
	token_container_t get_token(int line, int column) override;
};

class token_string_find_t : public token_finder_t {
protected:
	string value;
public:
	token_string_find_t(string* str_) : token_finder_t(str_) { priority = 1; }
	result is_mine(char a) override;
	token_container_t get_token(int line, int column) override;
};

class token_space_find_t : public token_finder_t {
public:
	token_space_find_t(string* str_) : token_finder_t(str_) { priority = 0; }
	result is_mine(char a) override;
};

class token_new_line_find_t : public token_finder_t {
public:
	token_new_line_find_t(string* str_) : token_finder_t(str_) { priority = 0; }
	result is_mine(char a) override;
};

class token_comment_find_t : public token_finder_t {
public:
	token_comment_find_t(string* str_) : token_finder_t(str_) {  priority = 1; }
	result is_mine(char a) override;
};

class token_multiline_comment_find_t : public token_finder_t {
public:
	token_multiline_comment_find_t(string* str_) : token_finder_t(str_) { priority = 1; }
	result is_mine(char a) override;
};

/*-----------------Lexeme-Analyzer-----------------*/

class lexeme_analyzer_t {
protected:
	istream* os;
	vector<token_finder_t*> tokens;
	token_container_t last_token;
	token_container_t next_token;
	LexemeAnalyzeError last_error;

	bool curr_token_error_appeared;
	bool last_token_error_appeared;
	
	bool fatal_error_appeared = false;
	char cc = 0;
	int line = 1;
	int column = 0;
	string curr_str;
	bool eof_reached = false;
	void next_char();

	token_container_t get_next();
public:
	lexeme_analyzer_t(istream& os_);
	token_container_t next();
	token_container_t get();
	bool eof();
};