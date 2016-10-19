//
//  bflags.hpp
//  Futoshiki
//
//  Created by Eduardo Pacheco on 10/16/16.
//  Copyright © 2016 Eduardo Pacheco. All rights reserved.
//

#ifndef bflags_hpp
#define bflags_hpp

namespace bitflags {
	#define clear_bit(f, n) (f -= 1 << (n)) // se o bit nao estava setado, vai dar problema
											// mas eh mais facil de ver
	//#define clear_bit(f, n) (f = (f) ^ 1 << (n))
	#define set_bit(f, n) (f += 1 << (n))

	#define ifbitset(f, n) ((f & 1 << (n)) != 0) 

	#define clear_bitarray(arr, nbit) (arr[(nbit) / 8] -= 1 << (nbit) % 8)
	#define set_bitarray(arr, nbit) (arr[(nbit) / 8] += 1 << (nbit) % 8)

	// todo numero pode ser escrito da seguinte forma
	// v = xxxx1[0...0]; eg: 101, 110, 10111
	// quando se subtrai 1 desse numero, o primeiro bit setado volta para zero (sobe um)
	// e todos os outros anteriories sao setados; eg: 7 = 111, 6 = 110; 10 = 1010, 9 = 1001
	// assim sendo, v AND v-1 retorna assim:
	//		xxx1[0..0]
	// AND	xxx0[1..1]
	//	=	xxx0[0..0]
	// se o numero era potencial de dois ele so tinha um 1, portanto os x's sao todos 0
	// se, e somente se, o numero for uma potencia de dois ou nulo, o retorno do AND eh zero
	#define is_power_2_or_0(v) (((v) & ((v) - 1)) == 0)

	//end-of-flags
	#ifndef EOF
	#define EOF ((uint8_t)-1)
	#endif

	typedef uint16_t wbitflags; // 16 bits
	typedef uint8_t bbitflags; // 8 bits

	class wbitlist {
		uint8_t tail;
		uint8_t head;
		wbitflags flags;

	public:
		wbitlist(wbitflags _flags, uint8_t _head, uint8_t _tail) : flags(_flags), tail(_tail),
			head(_head) {}

		// retorna a posicao da proxima flag aa direita
		// retorna -1 se acabou
		uint8_t pop_back ();

		// a primeira chamada a pop deve informar o bitflags
		// as chamadas subsequentes devem passar NULL como parametro
		// retorna a posicao da proxima flag aa esquerda
		// retorna -1 se acabou
		uint8_t pop_front ();
	};
}


#endif /* bflags_hpp */
