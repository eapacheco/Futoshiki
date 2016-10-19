//
//  Cell.hpp
//  Futoshiki
//
//  Created by Eduardo Pacheco on 10/16/16.
//  Copyright © 2016 Eduardo Pacheco. All rights reserved.
//

#ifndef Cell_hpp
#define Cell_hpp

#include <list>
#include "bitflags.hpp"

typedef struct {
	uint8_t x1, y1, x2, y2;
} restriction;

class Cell
{
public:
	bitflags::wbitflags possibilities;
	std::list<restriction*> *lesser;
	std::list<restriction*> *greater;
	uint8_t selected;
	uint8_t count;

	Cell (uint8_t width);
	Cell (Cell *c);
	~Cell();

	bool isequal_possibilities (Cell *cell);
};

#endif /* Cell_hpp */
