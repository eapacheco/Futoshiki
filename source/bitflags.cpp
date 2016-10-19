//
//  bflags.cpp
//  Futoshiki
//
//  Created by Eduardo Pacheco on 10/16/16.
//  Copyright © 2016 Eduardo Pacheco. All rights reserved.
//

#include <stdlib.h>

#include "bitflags.hpp"
#include "Futoshiki.hpp"

using namespace bitflags;

uint8_t wbitlist::pop_back () {
	wbitflags mask = 1 << tail; // mascara para bit0

	// verifica bit a bit, ate encontrar um setado
	for (; tail < head + 1; ++tail) {;
		if ((flags & mask) != 0) // achei
			break;

		mask <<= 1; // proxima mascara
	}

	if (tail == head+1)
		return EOF;

	clear_bit (flags, tail);
	return tail++;
}

uint8_t wbitlist::pop_front () {
	wbitflags mask = 1 << head; // mascara do ultimo bit

	// verifica bit a bit, ate encontrar um setado
	for (; head != (uint8_t) -1; --head) {
		if ((flags & mask) != 0) // achei
			break;

		mask >>= 1; // proxima mascara
	}

	if (head == (uint8_t) -1)
		return EOF;

	clear_bit (flags, head);
	return head--;
}
