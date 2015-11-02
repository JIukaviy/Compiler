#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include "lexeme_analyzer.h"
#include "parser.h"

using namespace std;

int main(int argc, char** argv) {
	tokens_init();
	lexeme_analyzer_init();
	parser_init();
	if (argc == 3) {
		ifstream fin(argv[2]);
		if (!fin) {
			cerr << "Can't open file" << endl;
			return 1;
		}
		ofstream fout("output.txt");
		lexeme_analyzer_t la(fin);
		parser_t parser(&la);
		if (argv[1][0] == 'l') {
			try {
				while (!la.eof())
					fout << la.next() << endl;
			} catch (CompileError& e) {
				fout << e;
			}
		} else if (argv[1][0] == 's') {
			try {
				parser.print_expr(fout);
			} catch (CompileError& e) {
				fout << e;
			}
		} else if (argv[1][0] == 't') {
			try {
				parser.print_type(fout);
			} catch (CompileError& e) {
				fout << e;
			} 
		} else if (argv[1][0] == 'd') {
			try {
				parser.print_decl(fout);
			} catch (CompileError& e) {
				fout << e;
			}
		} else if (argv[1][0] == 'o') {
			try {
				parser.print_statement(fout);
			} catch (CompileError& e) {
				fout << e;
			}
		}
		return 0;
	}
	ifstream fin("data.txt");
	lexeme_analyzer_t la(fin);
	parser_t parser(&la);
	try {
		parser.print_statement(cout);
	} catch (LexemeAnalyzeError& e) {
		cerr << "Lexeme analyzer error: " << e << endl;
	} catch (SyntaxError& e) {
		cerr << "Parse error: " << e << endl;
	} catch (SemanticError& e) {
		cerr << "Semantic error: " << e << endl;
	}

	system("pause");
		
	return 0;
}