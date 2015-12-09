#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include "lexeme_analyzer.h"
#include "parser.h"
#include "asm_generator.h"
#include "var.h"

using namespace std;

int main(int argc, char** argv) {
	tokens_init();
	lexeme_analyzer_init();
	parser_init();
	asm_generator_init();
	if (argc == 3) {
		ifstream fin(argv[2]);
		if (!fin) {
			cerr << "Can't open file" << endl;
			return 1;
		}
		ofstream fout("output.txt");
		lexeme_analyzer_t la(fin);
		parser_t parser(&la);
		try {
			if (argv[1][0] == 'l') {
				while (!la.eof())
					fout << la.next() << endl;
			} else if (argv[1][0] == 'e') {
				parser.print_expr(fout);
			} else if (argv[1][0] == 't') {
				parser.print_type(fout);
			} else if (argv[1][0] == 'd') {
				parser.print_statements(fout);
			} else if (argv[1][0] == 's') {
				parser.print_statements(fout);
			} else if (argv[1][0] == 'a') {
				parser.print_asm_code(fout);
			}
		} catch (CompileError& e) {
			fout << e;
		}
		return 0;
	}
	ifstream fin("data.txt");
	lexeme_analyzer_t la(fin);
	parser_t parser(&la);
	try {
		parser.print_asm_code(cout);
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