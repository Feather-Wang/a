#ifndef __BITSTREAM_H__

#define __BITSTREAM_H__

#include "huffmantree.h"

struct bitstream_s
{
    unit_32_t   bits;
    int         bit_flag;
    unit_32_t   bits_temp;
    int         bit_flag_temp;
};
typedef struct bitstream_s bitstream_t;

extern int read_bitstream(int zip_fd, bitstream_t *bitstream);

#endif /* end of include guard: __BITSTREAM_H__ */
