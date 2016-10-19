
//  Futoshiki.cpp
//  Futoshiki
//
//  Created by Eduardo Pacheco on 10/16/16.
//  Copyright © 2016 Eduardo Pacheco. All rights reserved.
//

#include "Futoshiki.hpp"
#include "bitflags.hpp"

#include <iostream>
#include <list>
#include <math.h>

// oficializando o meio de acesso ao vetor remaining
#define REMAINING_COL(col, num) remaining[col][num]
#define REMAINING_ROW(row, num) remaining[dimension + row][num]

#define MATRIX_POS(x, y, d) (x + (d)*(y))

using namespace std;

Futoshiki::Futoshiki (Futoshiki *f) {
	history = new stack<uint8_t> (*f->history);
	dimension = f->dimension;
	count_bfs = f->count_bfs;
	attributions = f->attributions;

	priority_index = new bitflags::bbitflags* [dimension];
	for (uint8_t i = 0; i < dimension; i++)
	{
		priority_index[i] = new bitflags::bbitflags [count_bfs];
		memcpy(priority_index[i], f->priority_index[i], count_bfs);
	}

	remaining = new bitflags::wbitflags* [dimension*2];
	for (uint8_t i = 0; i < dimension*2; i++)
	{
		remaining[i] = new bitflags::wbitflags [dimension];
		memcpy (remaining[i], f->remaining[i], dimension*sizeof(bitflags::wbitflags));
	}

	cells = new Cell* [dimension*dimension];
	for (uint8_t i = 0; i < dimension*dimension; i++)
		cells[i] = new Cell (f->cells[i]);
}

Futoshiki::Futoshiki(uint8_t d) {
	attributions = 0;
	dimension = d;
	history = new stack<uint8_t>();

	remaining = new bitflags::wbitflags* [d*2];
	for (uint8_t i = 0; i < d*2; ++i)
	{
		remaining[i] = new bitflags::wbitflags [d];

		for (uint8_t j = 0; j < d; j++)
			remaining[i][j] = (1 << dimension)-1; // seta todos os bits dentro de dimension para 1
	}

	count_bfs = ceil (d * d / 8.0f); // quantos bitflags sao necessarios para representar todas as celulas

	priority_index = new bitflags::bbitflags* [dimension]; // indice para as que sobram 1 ate todas, exclui 0
	for (uint8_t i = 0; i < dimension - 1; i++)
	{
		priority_index[i] = new bitflags::bbitflags [count_bfs];
		memset (priority_index[i], 0, count_bfs); // limpa todos
	}

	priority_index[dimension-1] = new bitflags::bbitflags [count_bfs];
	memset (priority_index[dimension - 1], -1, count_bfs); // seta todos

	uint8_t spare = count_bfs * 8 - d * d;
	priority_index[dimension-1][count_bfs-1] = priority_index[dimension-1][count_bfs-1] & (0xFF >> spare);

	cells = new Cell* [d*d];
	for (uint8_t i = 0; i < d*d; ++i)
		cells[i] = new Cell (d);
}

Futoshiki::~Futoshiki () {
	for (uint8_t i = 0; i < dimension * dimension; ++i)
		delete cells[i];
	delete cells;

	for (uint8_t i = 0; i < dimension*2; ++i)
		delete remaining[i];
	delete remaining;
}

//lê os dados fixos, bem como as restricoes, do tabuleiro
//	rest = numero de restricoes
void Futoshiki::read_input(short rest) {
	for (uint8_t row = 0; row < dimension; ++row)
		for (uint8_t col = 0; col < dimension; ++col)
		{
			int num;
			cin >> num;

			if (num != 0)
				set_number (col, row, (uint8_t) num);
		}

	for (; rest > 0; --rest)
	{
		int x1, y1, x2, y2;
		cin >> y1 >> x1 >> y2 >> x2;

		restriction *r = new restriction;
		r->x1 = --x1;
		r->x2 = --x2;
		r->y1 = --y1;
		r->y2 = --y2;

		cells[ MATRIX_POS(r->x1, r->y1, dimension) ]->lesser->push_back (r);
		cells[ MATRIX_POS(r->x2, r->y2, dimension) ]->greater->push_back (r);

		apply_lesser (r->x1, r->y1);
		apply_greater (r->x2, r->y2);
	}

	while (!history->empty()) // nao preciso manter historico da alteracoes deferentes da insercao de dados
		history->pop();
}

bool Futoshiki::solve (bool lookahead) {
	if (!lookahead)
		return _solve_uncut (0);

	return _solve_lookahead (0);
}

// encontra uma solucao para o tabuleiro usando backtracking
// tanto a verificacao adiante quanto a politica de "menos valores remanescentes"
// sao aplicados na otimizacao do backtracking.
bool Futoshiki::solve () {
	if (attributions > ATTRIBUTIONS_LIMIT)
		return false;

	uint8_t i;
	uint8_t gpos = count_bfs; // valor auxiliar provisorio

	// TODO: abaixo, fazer em assembly inline

	// sempre que, ou um numero eh setado, ou ocorre um rollback, o priority_at pode perder a linha correta
	// portanto, o priority_at so consegue garantir a maior prioridade possivel, mas nao que seja a correta
	// o loop abaixo procura a proxima maior prioridade e atualiza o priority_at
	for (i = 0; i < dimension - 1 && gpos == count_bfs; ++i)
		for (gpos = 0; gpos < count_bfs && priority_index[i][gpos] == 0; ++gpos);

	if (gpos == count_bfs) // nao encontrou ninguem, tabuleiro preenchido
		return true;

	bitflags::wbitlist *list = new bitflags::wbitlist (priority_index[--i][gpos], 7, 0);
	gpos = gpos * 8 + list->pop_back(); // cada count_bfs representa um grupo de 8 celulas

	Cell *cell = cells[ gpos ];

	size_t hframe = history->size(); // bons tempos de mov %esp, %ebp

	list = new bitflags::wbitlist (cell->possibilities, dimension-1, 0);

	for (uint8_t opt = list->pop_back(); opt != (uint8_t) EOF; opt = list->pop_back ())
	{
		if (set_number(gpos % dimension, gpos / dimension, opt + 1) && solve())
		{
			while (history->size() != hframe)
				history->pop();

			return true;
		}

		history_rollback(hframe);
	}

	return false;
}

bool Futoshiki::_solve_lookahead (uint8_t gpos) {
	if (attributions > ATTRIBUTIONS_LIMIT)
		return false;

	Cell *cell = cells[ gpos ];

	if (cell->selected != 0) // nao ha logica interna que atribui valores automaticamente
							 // entao, essa condicao so sera verdadeira se esta celula eh um dado de entrada
		return (gpos == dimension*dimension - 1) || _solve_lookahead(gpos + 1);

	 size_t hframe = history->size();

	bitflags::wbitlist list (cell->possibilities, dimension-1, 0);

	for (uint8_t opt = list.pop_back(); opt != (uint8_t) EOF; opt = list.pop_back())
	{
		if (set_number(gpos % dimension, gpos / dimension, opt + 1) &&
			(gpos == dimension*dimension - 1 || _solve_uncut(gpos+1)))
		{
			while (history->size() != hframe)
				history->pop();

			return true;
		}

		history_rollback(hframe);
	}

	return false;
}

void Futoshiki::history_pop (uint8_t* gpos, bitflags::wbitflags* possibilities) {
	*gpos = history->top();							// este por ultimo
	history->pop();

	//bitflags ocupa 16 bits, entao foi armazenado em duas partes
	((uint8_t*) possibilities) [1] = history->top();  // este segundo
	history->pop();

	((uint8_t*) possibilities) [0] = history->top();  // este primeiro
	history->pop();
}

void Futoshiki::history_push (uint8_t gpos, bitflags::wbitflags possibilities) {
	history->push ( ((uint8_t*) &possibilities) [0]);
	history->push ( ((uint8_t*) &possibilities) [1]);
	history->push (gpos);
}

void Futoshiki::history_rollback (size_t pframe) {
	while (history->size() > pframe)
	{
		uint8_t gpos, x, y;
		bitflags::wbitflags possibilities;
		Cell *cell;

		history_pop(&gpos, &possibilities);
		x = gpos % dimension;
		y = gpos / dimension;

		cell = cells [gpos];

		// as alteracoes sempre removem possibilidades, nunca adiocionam
		// portanto, a diferenca entre a atual e a passada sao os valores
		// que foram removidos
		// diferenca = XOR
		possibilities = possibilities ^ cell->possibilities;

		// para setar todo a diferenca basta unir os conjuntos
		// uniao = OR
		cell->possibilities = cell->possibilities | possibilities;

		if (cell->count != 0)
			clear_bitarray (priority_index[cell->count-1], gpos); // volta a referencia correta do priority_index

		bitflags::wbitlist list (possibilities, dimension-1, 0);
		for (uint8_t opt = list.pop_back(); opt != (uint8_t) EOF; opt = list.pop_back()) {
			set_bit (REMAINING_COL(x, opt), y);
			set_bit (REMAINING_ROW(y, opt), x);
			++cell->count;
		}

		set_bitarray (priority_index[cell->count-1], gpos);
		cell->selected = 0;
	}
}

bitflags::wbitflags Futoshiki::_trivial_lesser(Cell *cell, bitflags::wbitflags remaining) {
	if (cell->lesser->size() == 0)
		return remaining;

	bitflags::wbitflags mask;
	uint8_t smallestBig = dimension;
	list<restriction*>::iterator it;

	for (it = cell->lesser->begin(); it != cell->lesser->end(); ++it)
	{
		Cell *big = cells [MATRIX_POS((*it)->x2, (*it)->y2, dimension)];
		if (big->selected != 0 && big->selected < smallestBig)
			smallestBig = big->selected;
	}

	// os valores de cell sao limitados por cima; cell nao pode ser maior
	// que o menor dos "grandes"
	//		xxxxx	possibilidades					smallestBig			5 (base 1)
	//		01111	possibilidades >= ao big		1 << 5-1		10000
	// AND	0xxxx									-()				01111
	mask = (1 << (smallestBig-1))-1;

	return remaining & mask;
}

bitflags::wbitflags Futoshiki::_trivial_greater(Cell *cell, bitflags::wbitflags remaining) {
	if (cell->greater->size() == 0)
		return remaining;

	bitflags::wbitflags mask;
	uint8_t greatestSmall = 1;
	list<restriction*>::iterator it;

	for (it = cell->greater->begin(); it != cell->greater->end(); ++it)
	{
		Cell *small = cells [MATRIX_POS((*it)->x1, (*it)->y1, dimension)];
		if (small->selected != 0 && small->selected > greatestSmall)
			greatestSmall = small->selected;
	}

	//		xxxxx	possibilidades					biggestSmall		3 (base 1)
	//		11000	possibilidades <= ao small		1 << 3			01000
	// AND	xx000									-()				00111
	//												~()				11000
	mask = ~((1 << greatestSmall)-1);

	return remaining & mask;
}

bool Futoshiki::_solve_uncut (uint8_t gpos) {
	if (attributions > ATTRIBUTIONS_LIMIT)
		return false;

	Cell *cell = cells[ gpos ];

	if (cell->selected) // nao ha logica interna que atribui valores automaticamente
		// entao, essa condicao so sera verdadeira se esta celula eh um dado de entrada
		return (gpos == dimension*dimension - 1) || _solve_uncut(gpos + 1); // retorna verdadeiro se eh o fim ou o resultado recursivo

	uint8_t x = gpos % dimension;
	uint8_t y = gpos / dimension;
	bitflags::wbitflags rowopt = 0;
	bitflags::wbitflags colopt = 0;

	for (uint8_t acol = 0; acol < dimension; ++acol)
		rowopt = rowopt | (1 << (cells [MATRIX_POS(acol,y,dimension)]->selected-1));
	for (uint8_t arow = 0; arow < dimension; ++arow)
		colopt = colopt | (1 << (cells [MATRIX_POS(x,arow,dimension)]->selected-1));

	// bit setados nao estao disponiveis
	// OR = uniao dos conjuntos
	// NOT = inverte, setados sao os livres
	// tenho a relacao de bits disponiveis conforme linha e coluna
	// falta a relacao das restricoes
	bitflags::wbitflags remaining = ~(rowopt | colopt);

	remaining = _trivial_greater (cell, remaining);
	remaining = _trivial_lesser (cell, remaining);

	bitflags::wbitlist list (remaining, dimension-1, 0);
	uint8_t opt = list.pop_back();
	while (opt != (uint8_t) EOF) {
		cell->selected = opt+1;
		++attributions;

		if (gpos == dimension*dimension - 1 || _solve_uncut(gpos+1) == true)
			return true;

		opt = list.pop_back ();
	}

	cell->selected = 0;
	return false;
}

void Futoshiki::print_possibilities() {
	for (uint8_t row = 0; row < dimension; ++row)
	{
		for (uint8_t col = 0; col < dimension; ++col)
		{
			Cell *cur = cells[ MATRIX_POS(col, row, dimension) ];

			if (cur->selected)
				for (uint8_t opt = 0; opt < dimension; ++opt)
					cout << '*';
			else
			{
				for (uint8_t opt = 0; opt < dimension; ++opt)
					if (cur->possibilities & (1 << opt))
						cout << (int) opt + 1;
					else
						cout << "_";
			}

			cout << " ";
		}

		cout << "\n";
	}
}

void Futoshiki::print_board () {
	for (uint8_t row = 0; row < dimension; ++row)
	{
		cout << '|';
		for (uint8_t col = 0; col < dimension; ++col)
		{
			Cell *cur = cells[ MATRIX_POS(col, row, dimension) ];
			cout << "  ";

			if (cur->selected)
				cout << (int) cur->selected;
			else
				cout << " ";

			cout << "  ";
		}

		cout << "|\n";
	}
}

// imprime bit a bit do array a ordem de impressao respeita a ordem logica dos elementos assim,
// o bit mais relevante (mais aa esquerda) de todo o conjunto eh o bit 8 (base 1) da impressao,
// que eh o oitavo elemento. Enquanto o bit menos relevante do primeiro byte eh o
// bit 1 da impressao.
static void print_flagsarray (bitflags::bbitflags *arr, uint8_t size) {
	for (uint8_t i = 0; i < size; ++i)
	{
		int byte = arr [i];
		for (uint8_t b = 0; b < 8; ++b)
		{
			if ( (b + i*8) % size == 0 )
				cout << '.';

			cout << byte % 2;
			byte >>= 1;
		}
	}

	cout << endl;
}

void Futoshiki::print_index () {
	for (uint8_t p = 0; p < dimension; ++p)
	{
		print_flagsarray(priority_index[p], dimension);
		cout << "***" << endl;
	}

}

void Futoshiki::print_aaaformat () {
	for (uint8_t row = 0; row < dimension; ++row)
	{
		for (uint8_t col = 0; col < dimension; ++col)
		{
			Cell *cur = cells[ MATRIX_POS(col, row, dimension) ];
			cout << (int) cur->selected << ' ';
		}

		cout << '\n';
	}
}

void Futoshiki::print_restrictions () {
	for (uint8_t i = 0; i < dimension*dimension; ++i)
	{
		Cell *cur = cells[i];
		bool single = true;

		for (uint8_t j = 0; j < dimension; ++j)
			single &= (i % dimension == j) || cells[ MATRIX_POS(j, i / dimension, dimension) ]->selected != cur->selected;
		for (uint8_t j = 0; j < dimension; ++j)
			single &= (i / dimension == j) || cells[ MATRIX_POS(i % dimension, j, dimension) ]->selected != cur->selected;

		if (!single)
			cout << "FAIL";

		if (cur->lesser->size() == 0 && cur->greater->size() == 0)
			continue;

		list<restriction*>::iterator it;

		for (it = cur->lesser->begin(); it != cur->lesser->end(); ++it)
		{
			Cell *trg = cells[MATRIX_POS((*it)->x2, (*it)->y2, dimension)];
			if (trg->selected <= cur->selected)
				cout << '(' << i % dimension << ':' << i / dimension << ')' << " > " << '(' << (int)(*it)->x2 << ':' << (int)(*it)->y2 << ')'
					<< ' ' << "FAIL" << endl;
		}

		for (it = cur->greater->begin(); it != cur->greater->end(); ++it)
		{
			Cell *trg = cells[MATRIX_POS((*it)->x1, (*it)->y1, dimension)];
			if (trg->selected >= cur->selected)
				cout << '(' << i % dimension << ':' << i / dimension << ')' <<  " > " << '(' << (int)(*it)->x1 << ':'
					<< (int)(*it)->y1 << ')' << ' ' << "FAIL" << endl;
		}
	}
}

bool Futoshiki::remove_possibility (uint8_t x, uint8_t y, uint8_t num) {
	Cell *cell = cells[ MATRIX_POS(x, y, dimension) ];
	uint8_t gpos = x + y*dimension;

	if (cell->count == 1)
		return false;	// nao posso remover a unica possibilidade de uma celula

	history_push(gpos, cell->possibilities);

	clear_bit (cell->possibilities, num-1);
	clear_bit (REMAINING_COL(x, num-1), y);
	clear_bit (REMAINING_ROW(y, num-1), x);

	--cell->count;
	clear_bitarray (priority_index[cell->count], gpos);	// atualizo vetor de prioridade novo o nivel caso
	set_bitarray (priority_index[cell->count-1], gpos);

	// nao posso atualizar o priority_index se o count for zero, essa verificacao eh feito logo abaixo
	// atualizarei apos a verificacao, com a garantia de que essa celula nunca sera o topo de history
	// se ela passar no teste. e que o topo de history, nunca passou no teste se houver uma falha.
	// portanto, basta ter a cautela no HISTORY_ROLLBACK de nao alterar o priority_index para o primeiro caso.

	if (REMAINING_COL(x, num-1) == 0 || REMAINING_ROW(y, num-1) == 0)
		return false;	//acabaram as opcoes para esse numero nessa linha/coluna - nao ha solucao

	if ( is_power_2_or_0(REMAINING_COL(x, num-1)) ) // se houver somente uma possivel celula para o numero
	{
		uint8_t row = bitflags::wbitlist(REMAINING_COL(x, num-1), dimension-1, 0).pop_back();

		if (!single_candidate (x, row, num)) // o remaining eh uma potencia de dois (tem um unico bit setado)
			return false;
	}

	if ( is_power_2_or_0(REMAINING_ROW(y, num-1)) ) // se chegamos aqui, com certeza nao eh zero
	{
		uint8_t col = bitflags::wbitlist (REMAINING_ROW(y, num-1), dimension-1, 0).pop_back();

		if (!single_candidate (col, y, num))
			return false;
	}

	return true;
}

bool Futoshiki::single_candidate (uint8_t col, uint8_t row, uint8_t num) {
	Cell *cell = cells [MATRIX_POS(col, row, dimension)];

	if (is_power_2_or_0(cell->possibilities)) // nao pode ser zero, senao nao teria a condicao "single candidate"
		return true;						  // estou verificando se ha opcoes a remover

	bool cutlow; // se a menor possibilidade aumentou, poda baixa
	uint8_t aux = EOF;

	uint8_t other = bitflags::wbitlist (cell->possibilities, dimension-1, 0).pop_back();

	cutlow = other != (num-1); // se o menor valor eh o num, entao a borda inferior nao mudou

	while (other != (uint8_t) EOF)
	{
		if (other != (num-1) && !remove_possibility(col, row, other + 1))
			return false;

		aux = other + 1;
		// nao preciso verificar desde o inicio, mudo o offset para depois de other
		// mas preciso pegar o possivel modificado cell->possibilities, por isso reconstruo
		other = bitflags::wbitlist (cell->possibilities, dimension-1, other + 1).pop_back();
	}

	if (cutlow && apply_lesser(col, row) == 0)
		return false;

	if (aux != num && apply_greater(col, row) == 0) // se a maior possibilidade nao era num, entao abaixou, poda alta
		return false;

	return true;
}

// verifica se o num setado na posicao num e o maior e/ou o menor dentre os setados
// retorna dois bits:
// bit0 borda da direita - menor numero
// bit1 borda da esquerda - maior numero
uint8_t ifedge (bitflags::wbitflags flags, uint8_t num) {
	// right-most				left-most
	//		xx1xx (flags)	 		xx1xx (flags)
	// and 	00011 (mask 1)		and 11000 (mask 2)
	// =	000xx				=	xx000

	bitflags::wbitflags mask;
	uint8_t ret;

	mask = (1 << (num-1)) - 1;				// mask 1
	ret = (flags & mask) == 0 ? 0b01 : 0;	// todos os x's sao zeros, era o menor numero disponivel

	mask = ~((1 << num) - 1);					// mask 2
	ret += ((flags & mask) == 0) ? 0b10 : 0; 	// todos os x's sao zeros, era o maior numero disponivel;

	return ret;
}

//opta por um numero para tal celula
//as opcoes de todas as celulas envolvidas sao atualizadas
bool Futoshiki::set_number (uint8_t col, uint8_t row, uint8_t num) {
	++attributions;

	Cell *cell = cells[ MATRIX_POS(col, row, dimension) ];
	uint8_t applyrest;

	// remove num de toda a coluna, se ainda tem em alguem
	uint8_t arow = bitflags::wbitlist (REMAINING_COL(col, num-1), dimension-1, 0).pop_back();
	while (arow != (uint8_t) EOF)
	{
		if (arow != row) {
			applyrest = ifedge(cells[MATRIX_POS(col, arow, dimension)]->possibilities, num);

			if (!remove_possibility (col, arow, num))
				return false;

			if ((applyrest & 0b01) != 0 && !apply_lesser (col, arow))
				return false;

			if ((applyrest & 0b10) != 0 && !apply_greater (col, arow))
				return false;
		}

		arow = bitflags::wbitlist (REMAINING_COL(col, num-1), dimension-1, arow+1).pop_back();
	}

	uint8_t acol = bitflags::wbitlist (REMAINING_ROW(row, num-1), dimension-1, 0).pop_back();
	while (acol != (uint8_t) EOF)
	{
		if (acol != col) {
			applyrest = ifedge(cells[MATRIX_POS(acol, row, dimension)]->possibilities, num);

			if (!remove_possibility (acol, row, num))
				return false;

			if ((applyrest & 0b01) != 0 && !apply_lesser (acol, row))
				return false;

			if ((applyrest & 0b10) != 0 && !apply_greater (acol, row))
				return false;
		}

		acol = bitflags::wbitlist (REMAINING_ROW(row, num-1), dimension-1, acol+1).pop_back();
	}

	// nao eh necessario remover as outras opcoes da celula
	// pois ela com certeza foi eleito single_candidate pela linha e pela coluna

	cell->count = 0;
	cell->selected = num;
	cell->possibilities = 0;
	REMAINING_COL(col, num-1) = 0;
	REMAINING_ROW(row, num-1) = 0;
	clear_bitarray(priority_index[0], col + row*dimension);
	return true;
}

bool Futoshiki::apply_greater (uint8_t x, uint8_t y) {
	list<restriction*>::iterator it;
	Cell *cell = cells [MATRIX_POS(x, y, dimension)];

	for (it = cell->greater->begin(); it != cell->greater->end(); /*conditional iteration*/ ) {
		switch ( _cuthigh(*it) ) {
			case 0: return false;
			case 1:
				++it;
				break;
			default:
				it = cell->greater->begin();
				break;
		}
	}

	return true;
}

bool Futoshiki::apply_lesser (uint8_t x, uint8_t y) {
	list<restriction*>::iterator it;
	Cell *cell = cells [MATRIX_POS(x, y, dimension)];

	for (it = cell->lesser->begin(); it != cell->lesser->end(); /*conditional iteration*/ ) {
		switch ( _cutlow(*it) ) {
			case 0: return false;
			case 1:
				++it;
				break;
			default:
				it = cell->lesser->begin();
				break;
		}
	}

	return true;
}

uint8_t Futoshiki::_cutlow (restriction *r) {
	uint8_t smallestLittle, smallestBig;
	Cell *big = cells[ MATRIX_POS(r->x2, r->y2, dimension) ];
	Cell *small = cells[ MATRIX_POS(r->x1, r->y1, dimension) ];


	smallestLittle = small->selected != 0 ? small->selected - 1
						: bitflags::wbitlist(small->possibilities, dimension-1, 0).pop_back();

	 // precisa ser maior que o smallestLittle
	smallestBig = bitflags::wbitlist (big->possibilities, dimension-1, 0).pop_back();
	if (smallestBig == (uint8_t) EOF || smallestBig > smallestLittle)
		return 1;

	while (smallestBig <= smallestLittle)
	{
		if (!remove_possibility(r->x2, r->y2, smallestBig + 1))
			return 0;

		// se fosse retornar EOF, ja teria falhado no remove_possibility
		smallestBig = bitflags::wbitlist (big->possibilities, dimension-1, smallestBig+1).pop_back();
	}

	if (apply_lesser (r->x2, r->y2) == 0)
		return 0;

	return 2;
}

uint8_t Futoshiki::_cuthigh (restriction *r) {
	uint8_t highestLittle, highestBig;
	Cell *big = cells[MATRIX_POS(r->x2, r->y2, dimension)];
	Cell *small = cells[MATRIX_POS(r->x1, r->y1, dimension)];

	highestBig = big->selected != 0 ? big->selected - 1
					: bitflags::wbitlist(big->possibilities, dimension-1, 0).pop_front();

	//		xxxxx	possibilidades do small			highestBig			2 (base 0)
	//		00111	possibilidades >= ao big		1 << 2+1		01000
	// AND	00xxx									-()				00111
	//												mask = -(1 << (highestBig + 1));
	// IDEIA FALHOU -> preciso remove-los um de cada vez para atualizar indices

	// precisa ser menor que o highestBig
	highestLittle = bitflags::wbitlist (small->possibilities, dimension-1, 0).pop_front();
	if (highestLittle == (uint8_t) EOF || highestLittle < highestBig)
		return 1;

	while (highestLittle >= highestBig) {
		if (!remove_possibility (r->x1, r->y1, highestLittle + 1))
			return 0;

		// se fosse retornar EOF, teria falhado no remove_possibility
		highestLittle = bitflags::wbitlist (small->possibilities, highestLittle-1, 0).pop_front();
	}
	
	if (apply_greater(r->x1, r->y1) == 0)
		return 0;
	
	return 2;
}
