//
//  Futoshiki.hpp
//  Futoshiki
//
//  Created by Eduardo Pacheco on 10/16/16.
//  Copyright © 2016 Eduardo Pacheco. All rights reserved.
//

#ifndef Futoshiki_hpp
#define Futoshiki_hpp

#include <stack>

#include "Cell.hpp"
#include "bitflags.hpp"

#define ATTRIBUTIONS_LIMIT 1E06

class Futoshiki
{
	std::stack<uint8_t> *history;
	uint8_t dimension;
	uint8_t count_bfs;
	uint32_t attributions;

	//matriz de numeros, contem elementos da IA
	Cell **cells;

	bitflags::wbitflags **remaining; // bits indicando quais celulas de linha/coluna aceitam tal numero
									// e.g. remaining [dimension + row][num] = 010001
									// significa: celulas 1 e 5 da linha row aceitam num

	bitflags::bbitflags **priority_index; // indice relacionando numero de possibilidades remanescentes
										  // aas celulas. e.g. priority_index [0] = ...100010
										  // significa: a celulas 1 e 6 possuem apenas 1 valor remanescente

	// seleciona um numero para uma celula e aplica a verificacao adiante
	// retorna falso se alguem ficou sem possibilidades - nao ha solucao
	bool set_number (uint8_t col, uint8_t row, uint8_t num);
	// sinaliza que tal num eh o unico candidato para tal celula
	// retorna falso se alguem ficou sem possibilidades - nao ha solucao
	bool single_candidate (uint8_t col, uint8_t row, uint8_t num);
	// remove o num dentre as possibilidades (tem que ser valido)
	// nao propaga os efeitos da mudanca, somente atualiza os contadores de candidatos
	// retorna 0 se alguem ficou sem possibilidades - nao ha solucao
	// retorna 1 se nao removeu das extremiadades - nao precisa apply_restrictions
	// retorna 2 se presica do apply
	// esta eh a unica funcao, alem da set_number, que altera os dados de uma celula
	// a saber, as possibilidades. nisto se resume o estado do jogo.
	// portanto, todas as alteracoes aqui feitas sao salvas na pilha history
	bool remove_possibility (uint8_t x, uint8_t y, uint8_t num);
	// aplica as alteracoes consequentes aa restricao de "menor que" pertencentes
	// a tal celula. seu efeito se propaga recorrentemente aas celulas afetadas.
	// retorna falso se alguma celula ficou sem possibilidades - nao ha solucao
	bool apply_greater(uint8_t x, uint8_t y);
	// aplica as alteracoes consequentes aa restricao de "maior que" pertencentes
	// a tal celula. seu efeito se propaga recorrentemente aas celulas afetadas.
	// retorna falso se alguma celula ficou sem possibilidades - nao ha solucao
	bool apply_lesser(uint8_t x, uint8_t y);

	// de fato aplica uma unica instancia de restricao entre duas celulas
	uint8_t _cutlow (restriction *r);
	uint8_t _cuthigh (restriction *r);

	// verifica, sem verificacao adiante, quais os valores validos para tal celula
	// se houver, chama recursivamente a proxima celula
	// se uma chamada recursiva retorna falso, tenta o proximo valor
	// quando os valores acabarem retorna falso: nao ha solucao
	// retorna verdadeiro se este ramo encontrou uma solucao
	// param
	//	gpos = posicao global (coluna + linha*dimensao) da celula
	bool _solve_uncut (uint8_t gpos);
	bitflags::wbitflags _trivial_lesser(Cell *cell, bitflags::wbitflags remaining);
	bitflags::wbitflags _trivial_greater(Cell *cell, bitflags::wbitflags remaining);

	// utiliza a IA do sistema, que limita as opcoes de cada celula conforme os dados inseridos
	// nao prioriza as celulas com menos valores, mas permuta entre as opcoes remanescentes
	// de uma celula percorrendo a matriz sequencialmente
	// retorna falso, se uma celula ficar sem opcoes
	// retorna verdadeiro, se a recorrencia chegou ao fim e o ramo eh valido
	bool _solve_lookahead (uint8_t gpos);

	// retorna o estado anterior de cada celula, atualizando os indices, ate alcancar
	// pframe - o ponto anterior do frame.
	void history_rollback (size_t pframe);
	//oficializa o push e pop de history para nao ter confusao com a ordem dos bytes
	void history_push (uint8_t gpos, bitflags::wbitflags possibilities);
	void history_pop (uint8_t* gpos, bitflags::wbitflags* possibilities);

	void print_index ();

public:
	// le a entrada conforme protocolo estabelecido
	// a insercao dos dados (numeros e restricoes) desencadeiam a verificacao adiante
	// r = numero de restricoes
	void read_input(short r);

	// encontra uma solucao para o tabuleiro usando backtracking
	// parametros
	//	lookahead = aplicar verificacao adiante
	bool solve (bool lookahead);

	// encontra uma solucao para o tabuleiro usando backtracking
	// tanto a verificacao adiante quanto a politica de "menos valores remanescentes"
	// sao aplicados na otimizacao do backtracking.
	bool solve ();

	// imprime o tabuleiro
	// nao ha representacao grafica para as restricoes
	void print_board ();
	// imprime a lista das restricoes e se elas foram preenchidas ou nao
	// o comportamento dessa funcao eh incorreto para tabuleiros nao preenchidos
	void print_restrictions();
	// imprime o tabuleiro informando as possibilidades remanescentes para cada celula
	// nao funciona para execucao sem lookahead pois os indices nao sao atualizados
	void print_possibilities ();

	// imprime tabuleiro conforme o formato exigido pelo trabalho
	void print_aaaformat ();

	uint32_t attributions_count() { return attributions; }


	Futoshiki(uint8_t d);
	Futoshiki(Futoshiki *f);
	~Futoshiki();
};

#endif /* Futoshiki_hpp */
