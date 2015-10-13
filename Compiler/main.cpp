#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include "lexeme_analyzer.h"
#include "parser.h"

//#define TEST

using namespace std;

int main(int argc, char** argv) {
#ifdef TEST
	if (argc != 2) {
		cout << "Enter file name" << endl;
		return 1;
	}
	ifstream fin(argv[1]);

	if (!fin)
		cout << "File \"" << argv[1] << "\" not found" << endl;

	ofstream fout("output.txt");
#else
	ifstream fin("data.txt");
#endif
	lexeme_analyzer_t la(fin);
	parser_t parser(&la);
	try {
		parser.parse();
		parser.print(cout);
	} catch (EOFReached) {

	} catch (LexemeAnalyzeError& e) {
#ifdef TEST
		fout
#else
		cout
#endif
		<< e;
	}

#ifndef TEST
	system("pause");
#endif

	return 0;
}