//
//  statistics_run.cpp
//  Futoshiki
//
//  Created by Eduardo Pacheco on 10/16/16.
//  Copyright © 2016 Eduardo Pacheco. All rights reserved.
//

#include <iostream>
#include "Futoshiki.hpp"

#include <time.h>

using namespace std;

void watch_game (Futoshiki *board, clock_t *tdone, int *cdone, clock_t *tlimit, int *climit, bool lookahead, bool mvr);

int main() {
	clock_t full_done = 0, full_limit = 0;
	clock_t half_done = 0, half_limit = 0;
	clock_t no_done = 0, no_limit = 0;

	int cfull_done = 0, cfull_limit = 0;
	int chalf_done = 0, chalf_limit = 0;
	int cno_done = 0, cno_limit = 0;

	int k, d, rest;

	cin >> k;
	for (int i = 1; i <= k; ++i)
	{
		cin >> d >> rest;

		Futoshiki *board = new Futoshiki ((uint8_t) d);
		board->read_input((uint8_t) rest);

		Futoshiki *cpy1 = new Futoshiki (board);
		Futoshiki *cpy2 = new Futoshiki (board);

		watch_game (board, &full_done, &cfull_done, &full_limit, &cfull_limit, true, true);
		watch_game (cpy1, &half_done, &chalf_done, &half_limit, &chalf_limit, true, false);
		watch_game (cpy2, &no_done, &cno_done, &no_limit, &cno_limit, false, false);
	}

	clock_t total;

	cout << "----- Dados de execucao:" << endl << endl;
	cout << "Lookahead + MVR:" << endl;
	total = full_limit + full_done;
	cout << "Total de clocks: " << total << ", equivalentes a " << total / CLOCKS_PER_SEC << " segundos" << endl;
	cout << cfull_done << " tabuleiros resolvidos e ocupam " << full_done << " clocks." << endl;
	cout << cfull_limit << " tabuleiros fora do limite de atribuicoes e ocupam " << full_limit << " clocks." << endl;
	cout << endl;

	cout << "Lookahead somente:" << endl;
	total = half_limit + half_done;
	cout << "Total de clocks: " << total << ", equivalentes a " << total / CLOCKS_PER_SEC << " segundos" << endl;
	cout << chalf_done << " tabuleiros resolvidos e ocupam " << half_done << " clocks." << endl;
	cout << chalf_limit << " tabuleiros fora do limite de atribuicoes e ocupam " << half_limit << " clocks." << endl;
	cout << endl;

	cout << "Nenhuma poda:" << endl;
	total = no_limit + no_done;
	cout << "Total de clocks: " << total << ", equivalentes a " << total / CLOCKS_PER_SEC << " segundos" << endl;
	cout << cno_done << " tabuleiros resolvidos e ocupam " << no_done << " clocks." << endl;
	cout << cno_limit << " tabuleiros fora do limite de atribuicoes e ocupam " << no_limit << " clocks." << endl;
	cout << endl;
}

void watch_game (Futoshiki *board, clock_t *tdone, int *cdone, clock_t *tlimit, int *climit, bool lookahead, bool mvr) {
	clock_t beg, end;
	bool ret;

	beg = clock();
	ret = mvr ? board->solve () :  board->solve(lookahead);
	end = clock();

	if (ret)
	{
		*tdone += end - beg;
		++(*cdone);
	}
	else
		if (board->attributions_count() >= ATTRIBUTIONS_LIMIT)
		{
			*tlimit += end - beg;
			++(*climit);
		}
}
