//
//  Cell.cpp
//  Futoshiki
//
//  Created by Eduardo Pacheco on 10/16/16.
//  Copyright © 2016 Eduardo Pacheco. All rights reserved.
//

#include "Cell.hpp"

#include <list>
#include <stdlib.h>

using namespace std;

bool Cell::isequal_possibilities (Cell *cell) {

	return this->possibilities == cell->possibilities;
}

Cell::Cell (Cell *c) {
	possibilities = c->possibilities;
	lesser = new list<restriction*> (*c->lesser);
	greater = new list<restriction*> (*c->greater);
	selected = c->selected;
	count = c->count;
}

Cell::Cell (uint8_t width) {
	selected = 0;
	count = width;

	//1 << (w+1) = 0010000000
	//menos 1 	 = 0001111111
	possibilities = (1 << width) - 1; // seta todos os bits dentro da width
	lesser = new list<restriction*>;
	greater = new list<restriction*>;
}

Cell::~Cell() {
	list<restriction*>::iterator it;
}
