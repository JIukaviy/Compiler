#include <list>
#include <vector>
#include <algorithm>
#include "lexeme_analyzer.h"

using namespace std;

#define is_char(a) ((a) >= 'a' && (a) <= 'z' || (a) >= 'A' && (a) <= 'Z')
#define is_digit(a) ((a) >= '0' && (a) <= '9')
#define is_new_line(a) ((a) == 10 || (a) == 13)
#define is_space(a) ((a) == ' ' || (a) == '	')
#define is_EOF(a) ((a) == EOF)
#define is_delimeter(a) ((a) == '.')

#define string_quote '\"'
#define char_quote  '\''


token_container_t token_finder_t::get_token(int line, int column) {
	return token_container_t(new token_printed_t(line, column, *str));
}

result token_int_find_t::is_mine(char a) {
	switch (state) {
		case 0:
			if (is_digit(a)) {
				next_state();
				return OK_OR_CONTINUE;
			} else
				return NOT_MINE;
		case 1: 
			if (is_digit(a))
				return OK_OR_CONTINUE;
			else
				return NOT_MINE;
	}
	return NOT_MINE;
}

token_container_t token_int_find_t::get_token(int line, int column) {
	return token_container_t(new token_integer_t(line, column, *str, atol(str->c_str())));
}

result token_real_find_t::is_mine(char a) {
	switch (state) {
		case 0:
			if (is_digit(a)) {
				next_state();
				return OK_OR_CONTINUE;
			} else
				return NOT_MINE;
		case 1:
			if (is_digit(a))
				return OK_OR_CONTINUE;
			else if (is_delimeter(a)) {
				next_state();
				return CONTINUE;
			} else
				return NOT_MINE;
		case 2:
			if (is_digit(a)) {
				next_state();
				return OK_OR_CONTINUE;
			} else
				throw NoFract();
		case 3:
			if (is_digit(a))
				return OK_OR_CONTINUE;
			else if (a == 'e' || a == 'E') {
				next_state();
				return CONTINUE;
			} else
				return NOT_MINE;
		case 4:
			if (is_digit(a)) {
				state = 6;
				return OK_OR_CONTINUE;
			} else if (a == '+' || a == '-') {
				next_state();
				return CONTINUE;
			} else
				throw NoExp();
		case 5:
			if (is_digit(a)) {
				next_state();
				return OK_OR_CONTINUE;
			} else
				throw NoExp();
		case 6:
			if (is_digit(a))
				return OK_OR_CONTINUE;
			else
				return NOT_MINE;
	}
	return NOT_MINE;
}

token_container_t token_real_find_t::get_token(int line, int column) {
	return token_container_t(new token_float_t(line, column, *str, atof(str->c_str())));
}

result token_ident_find_t::is_mine(char a) {
	switch (state) {
		case 0:
			if (is_char(a)) {
				next_state();
				return OK_OR_CONTINUE;
			} else
				return NOT_MINE;
		case 1:
			if (is_char(a) || is_digit(a))
				return OK_OR_CONTINUE;
			else
				return NOT_MINE;
	}
	return NOT_MINE;
}

token_container_t token_ident_find_t::get_token(int line, int column) {
	return token_container_t(new token_identifier_t(line, column, *str));
}

result token_char_find_t::is_mine(char a) {
	switch (state) {
		case 0: 
			if (a == char_quote) {
				next_state();
				return CONTINUE;
			} else
				return NOT_MINE;
		case 1: 
			if (is_new_line(a))
				throw BadNewLine();
			else if (is_EOF(a))
				throw BadEOF();
			else {
				next_state();
				return CONTINUE;
			}
		case 2: 
			if (is_new_line(a))
				throw BadNewLine();
			else if (is_EOF(a))
				throw BadEOF();
			else if (a == char_quote)
				return OK;
			else
				return NOT_MINE;
	}
	return NOT_MINE;
}

token_container_t token_char_find_t::get_token(int line, int column) {
	return token_container_t(new token_char_t(line, column, *str, str->c_str()[1]));
}

result token_string_find_t::is_mine(char a) {
	switch (state) {
		case 0: 
			if (a == string_quote) {
				next_state(); 
				return CONTINUE;
			} else
				return NOT_MINE;
		case 1: 
			if (is_new_line(a))
				throw BadNewLine();
			else if (is_EOF(a))
				throw BadEOF();
			else if (a == string_quote)
				return OK;
			else
				return CONTINUE;
	}
	return NOT_MINE;
}

token_container_t token_string_find_t::get_token(int line, int column) {
	string t_str = *str;
	t_str.erase(t_str.begin());
	t_str.pop_back();
	return token_container_t(new token_string_t(line, column, *str, t_str));
}

result token_space_find_t::is_mine(char a) {
	if (is_space(a))
		return OK_OR_CONTINUE;
	else
		return NOT_MINE;
}

result token_new_line_find_t::is_mine(char a) {
	if (is_new_line(a))
		return OK;
	else
		return NOT_MINE;
}

result token_comment_find_t::is_mine(char a) {
	switch (state) {
		case 0:
			if (a == '/') {
				next_state();
				return CONTINUE;
			} else
				return NOT_MINE;
		case 1:
			if (a == '/') {
				next_state();
				return CONTINUE;
			} else
				return NOT_MINE;
		case 2:
			if (is_EOF(a) || is_new_line(a)) {
				return OK;
			} else
				return CONTINUE;
	}
	return NOT_MINE;
}

result token_multiline_comment_find_t::is_mine(char a) {
	switch (state) {
		case 0:
			if (a == '/') {
				next_state();
				return CONTINUE;
			} else
				return NOT_MINE;
		case 1:
			if (a == '*') {
				next_state();
				return CONTINUE;
			} else
				return NOT_MINE;
		case 2:
			if (a == '*') {
				next_state();
				return CONTINUE;
			} if (is_EOF(a)) {
				throw BadEOF();
			} else
				return CONTINUE;
		case 3:
			if (a == '/') {
				return OK;
			} if (is_EOF(a)) {
				throw BadEOF();
			} else {
				state = 2;
				return CONTINUE;
			}
	}
	return NOT_MINE;
}

void token_keyword_find_t::clear_state() {
	sort(keywords.begin(), keywords.end());
	first = 0;
	curr_pos = 0;
	finded = false;
	last = keywords.size() - 1;
}

result token_keyword_find_t::is_mine(char a) {
	int i =  (last + first) / 2;

	int d = i;
	int j = first;
	int k = last;

	if (curr_pos == keywords[first].size())
		first++;

	while (k - j > 1) {
		d = (k + j) / 2;
		if (keywords[d][curr_pos] < a)
			j = d;
		else 
			k = d;
	}
	first = keywords[j][curr_pos] == a ? j : k;
	j = first;
	k = last;
	while (k - j > 1) {
		d = (k + j) / 2;
		if (keywords[d][curr_pos] <= a)
			j = d;
		else
			k = d;
	}
	last = keywords[k][curr_pos] == a ? k : j;

	if (first != last && keywords[first].size() == curr_pos + 1 && keywords[first][curr_pos] == a)
		return OK_OR_CONTINUE;

	if (first == last)
		if (keywords[first][curr_pos] == a) {
			if (keywords[first].size() == curr_pos + 1)
				return OK;
		} else
			return NOT_MINE;

	curr_pos++;

	return CONTINUE;
}

token_container_t token_keyword_find_t::get_token(int line, int column) {
	return token_container_t(new token_keyword_t(line, column, *str));
}

token_container_t token_operator_find_t::get_token(int line, int column) {
	return token_container_t(new token_operator_t(line, column, *str));
}

token_container_t token_separator_find_t::get_token(int line, int column) {
	return token_container_t(new token_separator_t(line, column, *str));
}


lexeme_analyzer_t::lexeme_analyzer_t(istream& os_) {
	os = &os_;

	tokens.push_back(new token_space_find_t(&curr_str));
	tokens.push_back(new token_new_line_find_t(&curr_str));
	tokens.push_back(new token_int_find_t(&curr_str));
	tokens.push_back(new token_real_find_t(&curr_str));
	tokens.push_back(new token_char_find_t(&curr_str));
	tokens.push_back(new token_string_find_t(&curr_str));
	tokens.push_back(new token_ident_find_t(&curr_str));
	tokens.push_back(new token_comment_find_t(&curr_str));
	tokens.push_back(new token_multiline_comment_find_t(&curr_str));

	vector<string> keywords;
	keywords.push_back(string("begin"));
	keywords.push_back(string("forward"));
	keywords.push_back(string("do"));
	keywords.push_back(string("else"));
	keywords.push_back(string("end"));
	keywords.push_back(string("for"));
	keywords.push_back(string("function"));
	keywords.push_back(string("if"));
	keywords.push_back(string("array"));
	keywords.push_back(string("of"));
	keywords.push_back(string("procedure"));
	keywords.push_back(string("program"));
	keywords.push_back(string("record"));
	keywords.push_back(string("then"));
	keywords.push_back(string("to"));
	keywords.push_back(string("type"));
	keywords.push_back(string("var"));
	keywords.push_back(string("while"));
	keywords.push_back(string("break"));
	keywords.push_back(string("continue"));
	keywords.push_back(string("downto"));
	keywords.push_back(string("exit"));
	keywords.push_back(string("repeat"));
	keywords.push_back(string("until"));
	tokens.push_back(new token_keyword_find_t(&curr_str, keywords));

	vector<string> operators;
	operators.push_back(string("and"));
	operators.push_back(string("div"));
	operators.push_back(string("mod"));
	operators.push_back(string("not"));
	operators.push_back(string("or"));
	operators.push_back(string("xor"));
	operators.push_back(string("+"));
	operators.push_back(string("-"));
	operators.push_back(string("*"));
	operators.push_back(string("/"));
	operators.push_back(string("^"));
	operators.push_back(string("+="));
	operators.push_back(string("-="));
	operators.push_back(string("*="));
	operators.push_back(string("/="));
	operators.push_back(string("<"));
	operators.push_back(string(">"));
	operators.push_back(string("<="));
	operators.push_back(string(">="));
	operators.push_back(string("="));
	operators.push_back(string("<>"));
	operators.push_back(string("@"));
	//operators.push_back(string("."));
	tokens.push_back(new token_operator_find_t(&curr_str, operators));

	vector<string> separators;
	separators.push_back(string("("));
	separators.push_back(string(")"));
	separators.push_back(string("["));
	separators.push_back(string("]"));
	separators.push_back(string(";"));
	separators.push_back(string(":"));
	separators.push_back(string(".."));
	separators.push_back(string(","));
	tokens.push_back(new token_separator_find_t(&curr_str, separators));

	sort(tokens.begin(), tokens.end(), [](token_finder_t* a, token_finder_t* b) { return *a < *b; });

	next_char();

	try {
		next_token = get_next();
	} catch (EOFReached) {
		eof_reached = true;
	} catch (LexemeAnalyzeError e) {
		fatal_error_appeared = true;
		last_error = e;
	}
}

void lexeme_analyzer_t::next_char() {
	if (cc == '\t') {
		column = column == 0 ? 1 : column;
		int n = ceil(column / 4.0);
		column = 4 * n;
	}
	cc = os->get();
	column++;
		
	if (cc != EOF)
		curr_str += cc;
}

bool lexeme_analyzer_t::eof() {
	return eof_reached;
}

struct candidate_t {
	token_finder_t* token;
	result last_result;
	bool error_appeared = false;
	LexemeAnalyzeError error;
	int last_position;
	int last_col;
	int last_line;
	candidate_t(token_finder_t* t, result lr, int c, int ll, int lc) : token(t), last_result(lr), last_position(c), last_line(ll), last_col(lc) {};
};

token_container_t lexeme_analyzer_t::get_next() {
	bool skip = false;
	do {
		if (os->eof())
			throw EOFReached();
		skip = false;
		list<candidate_t> candidates;
		candidate_t last_ok_candidate(0, NONE, -1, 0, 0);

		for (auto i = tokens.begin(); i != tokens.end(); i++) {
			(*i)->clear_state();
			candidates.push_back(candidate_t(*i, NONE, os->tellg(), 0, 0));
		}

		int start_line = line;
		int start_column = column;

		while (true) {
			for (auto i = candidates.begin(); i != candidates.end(); ) {
				bool del = false;

				try {
					i->last_result = i->token->is_mine(cc);
					bool passed = i->last_result == OK || i->last_result == OK_OR_CONTINUE;

					if (last_ok_candidate.last_position < os->tellg() && passed)
						last_ok_candidate = candidate_t(i->token, i->last_result, os->tellg(), line, column);

					if (i->last_result == NOT_MINE || i->last_result == OK)
						del = true;
				}
				catch (LexemeAnalyzeError& e) {
					del = true;
					if (candidates.size() == 1) {
						e.line = line;
						e.column = column;
						last_ok_candidate.error = e;
						last_ok_candidate.error_appeared = true;
					}
				}

				if (del)
					candidates.erase(i++);
				else
					++i;
			}

			if (candidates.size() > 0)
				next_char();
			else {			
				last_token_error_appeared = curr_token_error_appeared;
				if (last_ok_candidate.error_appeared) {
					curr_token_error_appeared = true;
					last_error = last_ok_candidate.error;
				}

				if (last_ok_candidate.last_position == -1)
					if (curr_token_error_appeared)
						throw last_error;
					else
						throw BadChar(line, column);

				if (last_ok_candidate.last_position != os->tellg() && cc != EOF) {
					int p = (int)os->tellg() - last_ok_candidate.last_position;
					if (cc == '\n')		// фееричный костыль
						p--;
					curr_str.erase(curr_str.end() - p, curr_str.end());
					os->seekg(last_ok_candidate.last_position - 1);

					cc = os->get();

					line = last_ok_candidate.last_line;
					column = last_ok_candidate.last_col;
				}

				if (last_ok_candidate.token->get_priority() == 0) {
					skip = true;
					if (typeid(*last_ok_candidate.token) == typeid(token_new_line_find_t)) {
						line++;
						column = 0;
					}
					curr_str.clear();
					next_char();
					break;
				}

				token_container_t token = last_ok_candidate.token->get_token(start_line, start_column);
				curr_str.clear();
				next_char();
				return token;
			}
		}
	} while (skip);

}

token_container_t lexeme_analyzer_t::next() {
	if (fatal_error_appeared)
		throw last_error;

	last_token = next_token;

	try {
		next_token = get_next();
	} catch (EOFReached) {
		eof_reached = true;
	} catch (LexemeAnalyzeError e) {
		if (last_token_error_appeared)
			throw last_error;
		else {
			fatal_error_appeared = true;
			last_error = e;
		}
	} 

	return last_token;
}