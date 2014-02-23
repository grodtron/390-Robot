#ifndef _SWAP_H_
#define _SWAP_H_

#define UINT8_SWAP(a, b) \
if(a != b){ \
   a ^= b; \
   b ^= a; \
   a ^= b; \
}

#define REG_SWAP(a, b) \
if(a != b){ \
   volatile uint8_t * reg_swap_temp = a; \
   a = b; \
   b = reg_swap_temp; \
}

#endif
