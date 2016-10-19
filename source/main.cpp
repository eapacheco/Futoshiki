//
//  main.cpp
//  Futoshiki
//
//  Created by Eduardo Pacheco on 10/16/16.
//  Copyright © 2016 Eduardo Pacheco. All rights reserved.
//

#include <iostream>
#include "Futoshiki.hpp"

// fazendo juz aa vibe do trabalho, fica o define em bits
//#define applyinteligence 0b00
//#define applyinteligence 0b01
#define applyinteligence 0b11

#define apply_nothing 0b00
#define apply_lookahead 0b01
#define apply_full 0b11

using namespace std;

int main() {
	int k, d, rest;
	bool ret;

	cin >> k;
	for (int i = 1; i <= k; ++i)
	{
		cin >> d >> rest;

		Futoshiki *board = new Futoshiki ((uint8_t) d);
		board->read_input((uint8_t) rest);

		switch (applyinteligence) {
			case apply_nothing:
				ret = board->solve (false);
				break;
			case apply_lookahead:
				ret = board->solve(true);
				break;
			default:
				ret = board->solve();
				break;
		}

		//board->print_restrictions();

		cout << i << endl;

		if (ret)
		{
			board->print_aaaformat();
			cout << endl;
		}
		else
			if (board->attributions_count() >= ATTRIBUTIONS_LIMIT)
				cout << "limite de " << ATTRIBUTIONS_LIMIT << " atribuicoes foi alcancado" << endl;
			else
				cout << "nao ha solucao para o tabuleiro proposto." << endl;

		delete board;
	}
	
}
