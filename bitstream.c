#include "bitstream.h"
#include <errno.h>

int read_bitstream(int zip_fd, bitstream_t *bitstream)
{
    int read_len = 0;

    if( bitstream->bit_flag_temp != 0 )
    {
        printf("[read_bitstream 1]->[%x][%d] [%x][%d]\n", bitstream->bits, bitstream->bit_flag, bitstream->bits_temp, bitstream->bit_flag_temp);
        if( 32 > (bitstream->bit_flag + bitstream->bit_flag_temp) )
        {
            bitstream->bits = bitstream->bits | (bitstream->bits_temp << bitstream->bit_flag);
            bitstream->bit_flag += bitstream->bit_flag_temp;
            bitstream->bits_temp = 0;
            bitstream->bit_flag_temp = 0;
            printf("[read_bitstream 2]->[%x][%d] [%x][%d]\n", bitstream->bits, bitstream->bit_flag, bitstream->bits_temp, bitstream->bit_flag_temp);
        }
        else
        {
            bitstream->bits = bitstream->bits | (bitstream->bits_temp << bitstream->bit_flag);
            if( 0 == bitstream->bit_flag )
                bitstream->bits_temp = 0;
            else
                bitstream->bits_temp = bitstream->bits_temp >> (32 - bitstream->bit_flag);
            bitstream->bit_flag_temp = bitstream->bit_flag_temp - (32 - bitstream->bit_flag);
            bitstream->bit_flag = 32;
            printf("[read_bitstream 3]->[%x][%d] [%x][%d]\n", bitstream->bits, bitstream->bit_flag, bitstream->bits_temp, bitstream->bit_flag_temp);
            return 0;
        }
    }
    read_len = read(zip_fd, &(bitstream->bits_temp), 4);
    if( read_len != 4 )
    {
        fprintf(stderr, "readfile error, read_len=[%ld] != header_local_file size[%ld] errno=[%d]\n", read_len, 4, errno);
        return -1;
    }
    printf("[read_bitstream 4]->[%x][%d] [%x][%d]\n", bitstream->bits, bitstream->bit_flag, bitstream->bits_temp, bitstream->bit_flag_temp);

    bitstream->bits = bitstream->bits | (bitstream->bits_temp << bitstream->bit_flag);
    if( 0 == bitstream->bit_flag )
        bitstream->bits_temp = 0;
    else
        bitstream->bits_temp = bitstream->bits_temp >> (32 - bitstream->bit_flag);
    bitstream->bit_flag_temp = bitstream->bit_flag;
    bitstream->bit_flag = 32;
    printf("[read_bitstream 5]->[%x][%d] [%x][%d]\n", bitstream->bits, bitstream->bit_flag, bitstream->bits_temp, bitstream->bit_flag_temp);

    return 0;
}
