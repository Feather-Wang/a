#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>

#include "wqs_unzip.h"
#include "huffmantree.h"
#include "bitstream.h"

static new_fd_count = 0;

int huffman_fixed(huffmantree_t **huffmantree_1, huffmantree_t **huffmantree_2)
{
    int index = 0;
    int ret = -1;

    for( index = 256; index <= 279; index++ ) 
    {
        huffmantree_insert(huffmantree_1, 0, index, 7); 
    }

    for( index = 0; index <= 143; index++ ) 
    {
        huffmantree_insert(huffmantree_1, 0, index, 8); 
    }

    for( index = 280; index <= 287; index++ ) 
    {
        huffmantree_insert(huffmantree_1, 0, index, 8); 
    }

    for( index = 144; index <= 255; index++ ) 
    {
        huffmantree_insert(huffmantree_1, 0, index, 9); 
    }

    for( index = 0; index <= 29; index++ ) 
    {
        huffmantree_insert(huffmantree_2, 0, index, 5); 
    }
    return 0;
}

int huffman_dynamic(int *zip_fd, bitstream_t *bitstream, int hclen, int hlit, int hdist, huffmantree_t **huffmantree_1, huffmantree_t **huffmantree_2)
{
    int index_i = 0, index_j = 0;

    unit_8_t ccl_list[19] = {0};
    huffmantree_t *huffmantree_3 = NULL;

    array_unit_t *cl1_list = NULL;
    int cl1_list_num = 0;

    array_unit_t *cl2_list = NULL;
    int cl2_list_num = 0;

    int cl_len_count = 0;
    int plies_max = 0;

    int ret = -1;

    /*6. CCL. (HCLEN+4)bits, 解析CCL，获取huffman码表3*/
    //printf("CCL=\n");
    for(index_i = 0; index_i < (hclen+4); index_i++)
    {
        if(bitstream->bit_flag < 8 )
        {
            ret = read_bitstream(*zip_fd, bitstream);
            if( 0 > ret )
            {
                fprintf(stderr, "read_bitstream error,ret=[%d]\n", ret);
                return -1;
            }
        }

        char ch = 0;
        GET_BITS(bitstream->bits, ch, 3);
        bitstream->bit_flag -= 3;
        ccl_list[ccl_reverse[index_i]] = ch;

        //printf("bits=[%x], bit_flag=[%d] index_i=[%d], ccl_index=[%d], value=[%x]\n", bitstream->bits, bitstream->bit_flag, index_i, ccl_reverse[index_i], ch);

    }
    for(; index_i < 19; index_i++)
    {
        ccl_list[ccl_reverse[index_i]] = 0;
        /*
           printf("[%d][%x]\n", index_i, 0);
           */
    }

    /*
    printf("\n");

    for(index_i = 0; index_i < 19; index_i++)
        printf("%x ", ccl_list[index_i]);
    printf("\n");
    */

    /*构造huffman树码表3*/
    /*i表示层数，j表示数组下标*/
    for(index_i = 1; index_i <= 15; index_i++)
    {
        for(index_j = 0; index_j < 19; index_j++)
        {
            if( index_i == ccl_list[index_j] )
            {
                //printf("before->plies=[%d], value=[%d]\n", index_i, index_j);
                huffmantree_insert(&huffmantree_3, 0, index_j, index_i);
            }
        }
    }

    /*7. CL1. (HLIT+2)个,这个是根据huffman码表3解析后的数字的个数，根据解析后的信息构建iteral/length码表（及huffman码表1）*/
    //printf("CL1--------------------------------------------------------------------\n");
    cl1_list = malloc(sizeof(array_unit_t)*(hlit+257));
    if( NULL == cl1_list )
    {
        fprintf(stderr, "cl1_list malloc error, errno=[%d]\n", errno);
        return -1;
    }
    memset(cl1_list, 0x00, sizeof(array_unit_t)*(hlit+257));
    cl_len_count = 0;
    cl1_list_num = 0;
    plies_max = 0;
    while( 1 )
    {
        /*
           printf("bits=[%x], bit_flag=[%d]\n", bitstream->bits, bitstream->bit_flag);
           */
        if( bitstream->bit_flag < 8 )
        {
            /*
               printf("get read\n");
               */
            if( 0 > read_bitstream(*zip_fd, bitstream) )
            {
                fprintf(stderr, "read_bitstream error\n");
                return -1;
            }
        }

        char ch = 0;
        int value = 0, value_p = 0;
        while( 1 )
        {
            GET_BITS(bitstream->bits, ch, 1);
            bitstream->bit_flag -= 1;
            value_p = huffmantree_search(huffmantree_3, ch);
            //printf("%d", ch);
            if( -2 == value_p )
            {
                fprintf(stderr, "CL1 huffmantree_search error, the huffmantree is wrong\n");
                return -1;
            }
            if( -1 != value_p )
            {
                break; 
            }
        }

        if( 18 == value_p )
        {
            /*当检测到游程编码18时,表示后面出现了多个0，取值范围为[11-138]，所以个数是从11开始的*/
            GET_BITS(bitstream->bits, ch, 7); 
            bitstream->bit_flag -= 7;
            cl_len_count += ch + 11;
            //printf("=[%d] ch = [%d]\n", value_p, ch+11);
        }
        else if( 17 == value_p )
        {
            /*当检测到游程编码17时,表示后面出现了多个0，取值范围为[3-10]，所以个数是从3开始的*/
            GET_BITS(bitstream->bits, ch, 3); 
            bitstream->bit_flag -= 3;
            cl_len_count += ch + 3;
            //printf("=[%d] ch = [%d]\n", value_p, ch+3);
        }
        else if( 16 == value_p )
        {
            /*当检测到游程编码16时，表示后面重复出现了前一个数字，取值范围[3,6]，所以数字是从3开始的*/
            GET_BITS(bitstream->bits, ch, 2); 
            bitstream->bit_flag -= 2;

            for(index_i = 0; index_i < (ch+3); index_i++)
            {
                cl1_list[cl1_list_num].value = cl_len_count + index_i;
                cl1_list[cl1_list_num].plies = value;
                ++cl1_list_num;
            }

            cl_len_count += ch + 3;
            //printf("=[%d] value=[%d], ch = [%d]\n", value_p, value, ch+3);
        }
        else
        {
            value = value_p;

            cl1_list[cl1_list_num].value = cl_len_count;
            cl1_list[cl1_list_num].plies = value;
            ++cl1_list_num;

            if( value > plies_max )
                plies_max = value;

            cl_len_count += 1;
            //printf("=[%d]\n", value);
        }

        if( cl_len_count == (hlit+257) )
        {
            //printf("CL1 resolve over\n");
            break;
        }
    }

    /*构造huffman树码表1*/
    /*i表示层数，j表示数组下标*/
    for(index_i = 1; index_i <= plies_max; index_i++)
    {
        for(index_j = 0; index_j < cl1_list_num; index_j++)
        {
            if( index_i == cl1_list[index_j].plies )
            {
                //printf("before->plies=[%d], value=[%d]\n", index_i, cl1_list[index_j].value);
                huffmantree_insert(huffmantree_1, 0, cl1_list[index_j].value, index_i);
            }
        }
    }
    //printf("huffmantree_show\n");
    //huffmantree_show(*huffmantree_1, 0);

    /*8. CL2. (HDIST+1)个,这个是根据huffman码表3解析后的数字的个数，根据解析后的信息构建distance码表（及huffman码表2）*/
    //printf("CL2--------------------------------------------------------------------\n");
    cl2_list = malloc(sizeof(array_unit_t)*(hdist+1));
    if( NULL == cl2_list )
    {
        fprintf(stderr, "cl2_list malloc error, errno=[%d]\n", errno);
        return -1;
    }
    memset(cl2_list, 0x00, sizeof(array_unit_t)*(hdist+1));
    cl_len_count = 0;
    cl2_list_num = 0;
    plies_max = 0;
    while( 1 )
    {
        /*
           printf("bits=[%x], bit_flag=[%d]\n", bitstream->bits, bitstream->bit_flag);
           */
        if( bitstream->bit_flag < 8 )
        {
            /*
               printf("get read\n");
               */
            if( 0 > read_bitstream(*zip_fd, bitstream) )
            {
                fprintf(stderr, "read_bitstream error\n");
                return -1;
            }
        }

        char ch = 0;
        int value = 0, value_p = 0;
        while( 1 )
        {
            GET_BITS(bitstream->bits, ch, 1);
            bitstream->bit_flag -= 1;
            value_p = huffmantree_search(huffmantree_3, ch);
            /*
               printf("[%d] value_p=[%d]\n", ch, value_p);
               */
            if( -2 == value_p )
            {
                fprintf(stderr, "CL2 huffmantree_search error, the huffmantree is wrong\n");
                return -1;
            }
            if( -1 != value_p )
            {
                break; 
            }
        }

        if( 18 == value_p )
        {
            /*当检测到游程编码18时,表示后面出现了多个0，取值范围为[11-138]，所以个数是从11开始的*/
            GET_BITS(bitstream->bits, ch, 7); 
            bitstream->bit_flag -= 7;
            cl_len_count += ch + 11;
            /*
               printf("[18] ch = [%d]\n", ch+11);
               */
        }
        else if( 17 == value_p )
        {
            /*当检测到游程编码17时,表示后面出现了多个0，取值范围为[3-10]，所以个数是从3开始的*/
            GET_BITS(bitstream->bits, ch, 3); 
            bitstream->bit_flag -= 3;
            cl_len_count += ch + 3;
            /*
               printf("[17] ch = [%d]\n", ch+3);
               */
        }
        else if( 16 == value_p )
        {
            /*当检测到游程编码16时，表示后面重复出现了前一个数字，取值范围[3,6]，所以数字是从3开始的*/
            GET_BITS(bitstream->bits, ch, 2); 
            bitstream->bit_flag -= 2;

            for(index_i = 0; index_i < (ch+3); index_i++)
            {
                cl2_list[cl2_list_num].value = cl_len_count + index_i;
                cl2_list[cl2_list_num].plies = value;
                ++cl2_list_num;
            }

            cl_len_count += ch + 3;
            /*
               printf("[16] value = [%d], ch = [%d]\n", value, ch+3);
               */
        }
        else
        {
            value = value_p;

            cl2_list[cl2_list_num].value = cl_len_count;
            cl2_list[cl2_list_num].plies = value;
            ++cl2_list_num;

            if( value > plies_max )
                plies_max = value;

            cl_len_count += 1;
            /*
               printf("[%d]\n", value);
               */
        }

        if( cl_len_count == (hdist+1) )
        {
            //printf("CL2 resolve over\n");
            break;
        }
    }

    /*构造huffman树码表1*/
    /*i表示层数，j表示数组下标*/
    for(index_i = 1; index_i <= plies_max; index_i++)
    {
        for(index_j = 0; index_j < cl2_list_num; index_j++)
        {
            if( index_i == cl2_list[index_j].plies )
            {
                //printf("before->plies=[%d], value=[%d]\n", index_i, cl2_list[index_j].value);
                huffmantree_insert(huffmantree_2, 0, cl2_list[index_j].value, index_i);
            }
        }
    }
    //printf("huffmantree_show\n");
    //huffmantree_show(*huffmantree_2, 0);

    return 0;
}

int main(int argc, const char *argv[])
{
    int zip_fd = 0;
    int ret = -1;
    ssize_t read_len = -1;
    int temp_len = -1;
    int index_i = 0, index_j = 0;

    int new_fd = 0;

    bitstream_t bitstream;

    header_local_file_t header_local_file;
    bits_header_t bits_header;
    huffmantree_t *huffmantree_1 = NULL;
    huffmantree_t *huffmantree_2 = NULL;

    unit_8_t zip_win[ZIP_WINDOW] = {0};
    unit_32_t index_win = 0;
    
    char filename[64] = {0};

    memset(&bitstream, 0x00, sizeof(bitstream));

    if( argc == 1 )
        sprintf(filename, "a.zip");
    else
        sprintf(filename, "%s", argv[1]);

    /*获取Local File Header*/    
    zip_fd = open(filename, O_RDONLY);
    if( -1 == zip_fd )
    {
        fprintf(stderr, "open(%s) error, errno=[%d]\n", argv[1], errno);
        return -1;
    }

    ret = lseek(zip_fd, 0, SEEK_SET);
    if( -1 == ret )
    {
        fprintf(stderr, "lseek(%s) to SEEK_SET error, errno=[%d]\n", argv[1], errno);
        return -1;
    }

    memset(&header_local_file, 0x00, sizeof(header_local_file_t));
    temp_len = sizeof(header_local_file_t)-sizeof(unit_8_t*)*2;
    read_len = read(zip_fd, &header_local_file, temp_len);
    if( read_len != temp_len )
    {
        fprintf(stderr, "read(%s) error, read_len=[%ld] != header_local_file size[%ld] errno=[%d]\n", argv[1], read_len, sizeof(header_local_file_t), errno);
        return -1;
    }
    /*
       printf("signature=[%x]\n", header_local_file.signature);
       printf("extract_version=[%x]\n", header_local_file.extract_version);
       printf("general_flag=[%x]\n", header_local_file.general_flag);
       printf("compress_method=[%x]\n", header_local_file.compress_method);
       printf("file_last_modification_time=[%x]\n", header_local_file.file_last_modification_time);
       printf("file_last_modification_date=[%x]\n", header_local_file.file_last_modification_date);
       printf("crc32_mothed=[%x]\n", header_local_file.crc32_mothed);
       printf("file size after compress=[%x]\n", header_local_file.filesize_after_compress);
       printf("file size before compress=[%x]\n", header_local_file.filesize_before_compress);
       printf("filename_len=[%x]\n", header_local_file.filename_len);
       printf("extra_field_len=[%x]\n", header_local_file.extra_field_len);
       */

    header_local_file.filename = malloc(sizeof(unit_8_t)*header_local_file.filename_len+1);
    if( NULL == header_local_file.filename )
    {
        printf("malloc error for header local file, errno=[%d]\n", errno);
        return -1;
    }

    temp_len = sizeof(unit_8_t)*header_local_file.filename_len;
    memset(header_local_file.filename, 0x00, temp_len+1);
    read_len = read(zip_fd, header_local_file.filename, temp_len);
    if( read_len != temp_len )
    {
        fprintf(stderr, "read(%s) error, read_len=[%ld] != header_local_file size[%ld] errno=[%d]\n", argv[1], read_len, temp_len, errno);
        return -1;
    }
    printf("filename=[%s]\n", header_local_file.filename);

    header_local_file.extra_field = malloc(sizeof(unit_8_t)*header_local_file.extra_field_len+1);
    if( NULL == header_local_file.extra_field )
    {
        printf("malloc error for header local file, errno=[%d]\n", errno);
        return -1;
    }

    temp_len = sizeof(unit_8_t)*header_local_file.extra_field_len;
    memset(header_local_file.extra_field, 0x00, temp_len+1);
    read_len = read(zip_fd, header_local_file.extra_field, temp_len);
    if( read_len != temp_len )
    {
        fprintf(stderr, "read(%s) error, read_len=[%ld] != header_local_file size[%ld] errno=[%d]\n", argv[1], read_len, temp_len, errno);
        return -1;
    }

    new_fd = open("copy.txt", O_CREAT | O_RDWR, 0666);
    if( -1 == new_fd )
    {
        fprintf(stderr, "open(%s) error, errno=[%d]\n", header_local_file.filename, errno);
        return -1;
    }

    /*************************************************************/
    /*开始解析压缩块*/
    if( bitstream.bit_flag < 8 )
    {
        /*
           printf("get read\n");
           */
        if( 0 > read_bitstream(zip_fd, &bitstream) )
        {
            fprintf(stderr, "read_bitstream error\n");
            return -1;
        }
    }
    //printf("bits=[%x]\n", bitstream.bits);

    memset(&bits_header, 0x00, sizeof(bits_header));

    /*1. header. 获取header, 1bits*/
    GET_BITS(bitstream.bits, bits_header.header, 1);
    bitstream.bit_flag -= 1;
    //printf("header=[%x]\n", bits_header.header);

    /*2. hufman_type. 获取hufman_type, 2bits*/
    GET_BITS(bitstream.bits, bits_header.hufman_type, 2);
    bitstream.bit_flag -= 2;
    //printf("hufman_type=[%x]\n", bits_header.hufman_type);

    if( 1 == bits_header.hufman_type )
    {
        ret = huffman_fixed(&huffmantree_1, &huffmantree_2);
        //printf("huffman_dynamic, ret=[%d]\n", ret);
        /*
        int huffmanvalue[] = {1, 0, 0, 0, 0};
        int value = -1;
        int i = 0;
        for(i = 0; i < (sizeof(huffmanvalue)/4); i++)
        {
            value = huffmantree_search(huffmantree_2, huffmanvalue[i]); 
        }
        printf("value=[%d]\n", value);

        return 1;
        */
    }
    else
    {
        /*3. HLIT. 获取HLIT, 5bits*/
        GET_BITS(bitstream.bits, bits_header.hlit, 5);
        bitstream.bit_flag -= 5;
        //printf("hlit=[%d]\n",bits_header.hlit);

        /*4. HDIST. 获取HDIST, 5bits*/
        GET_BITS(bitstream.bits, bits_header.hdist, 5);
        bitstream.bit_flag -= 5;
        //printf("hdist=[%d]\n",bits_header.hdist);

        /*5. HCLEN. 获取HCLEN, 4bits*/
        GET_BITS(bitstream.bits, bits_header.hclen, 4);
        bitstream.bit_flag -= 4;
        //printf("hclen=[%d]\n",bits_header.hclen);

        /*
        6. 获取CCL
        7. 获取CL1
        8. 获取CL2
        */
        ret = huffman_dynamic(&zip_fd, &bitstream, bits_header.hclen, bits_header.hlit, bits_header.hdist, &huffmantree_1, &huffmantree_2);
        //printf("huffman_dynamic, ret=[%d]\n", ret);
    }

    /*9. uncompress. 根据iteral/length码表和distance码表解析剩下的比特流*/
    //printf("99999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999\n");
    while( 1 )
    {
        //printf("[%s,%d] bits=[%x], bit_flag=[%d]\n", __FILE__, __LINE__, bits, bit_flag);
        if( bitstream.bit_flag < 15 )
        {
            //printf("get read\n");
            if( 0 > read_bitstream(zip_fd, &bitstream) )
            {
                fprintf(stderr, "read_bitstream error\n");
                return -1;
            }
        }

        unit_16_t ch = 0;
        int value = 0;
        while( 1 )
        {
            GET_BITS(bitstream.bits, ch, 1);
            bitstream.bit_flag -= 1;
            value = huffmantree_search(huffmantree_1, ch);

            //printf("%d", ch);

            if( -2 == value )
            {
                fprintf(stderr, "CL1 huffmantree_search error, the huffmantree is wrong\n");
                return -1;
            }
            if( -1 != value )
            {
                break; 
            }
        }

        if( 0 <= value && value <= 255 )
        {
            //printf(" insert=[%c]\n", value);
            zip_win[index_win] = value; 
            ret = write(new_fd, &(zip_win[index_win]), 1);
            if( ret != 1 )
            {
                fprintf(stderr, "[%d] write file errno, errno=[%d]\n", errno);
                return -1;
            }
            new_fd_count++;
            index_win = ZIP_WINDOW_SLIDE(index_win, 1);             
        }
        else if( 256 == value )
        {
            //printf("zip uncompress over\n");
            break;
        }
        else if( 256 < value )
        {
            //printf("=[%d]\n", value);
            //printf("[%s,%d] bits=[%x], bit_flag=[%d]\n", __FILE__, __LINE__, bitstream.bits, bitstream.bit_flag);
            if( bitstream.bit_flag < 15 )
            {
                //printf("get read\n");
                if( 0 > read_bitstream(zip_fd, &bitstream) )
                {
                    fprintf(stderr, "read_bitstream error\n");
                    return -1;
                }
            }

            unit_32_t length = 0;
            unit_32_t distance = 0;
            index_i = value - 257;
            if( array_length_bits[index_i] > 0 )
            {
                GET_BITS(bitstream.bits, ch, array_length_bits[index_i]);
                bitstream.bit_flag -= array_length_bits[index_i];
                length = array_length[index_i] + ch;
                //printf("value=[%d] bit=[%d][%x] length=[%d]\n", value, array_length_bits[index_i], ch, length);
            }
            else
            {
                length = array_length[index_i];
                //printf("value=[%d] bit=[%d] length=[%d]\n", value, array_length_bits[index_i], array_length[index_i]);
            }
            
            //printf("[%s,%d] bits=[%x], bit_flag=[%d]\n", __FILE__, __LINE__, bitstream.bits, bitstream.bit_flag);
            if( bitstream.bit_flag < 15 )
            {
                //printf("get read\n");
                if( 0 > read_bitstream(zip_fd, &bitstream) )
                {
                    fprintf(stderr, "read_bitstream error\n");
                    return -1;
                }
            }

            while( 1 )
            {
                GET_BITS(bitstream.bits, ch, 1);
                bitstream.bit_flag -= 1;
                value = huffmantree_search(huffmantree_2, ch);
                //printf("%d", ch);
                if( -2 == value )
                {
                    fprintf(stderr, "CL1 huffmantree_search error, the huffmantree is wrong\n");
                    return -1;
                }
                if( -1 != value )
                {
                    break; 
                }
            }

            //printf("[%s,%d] bits=[%x], bit_flag=[%d]\n", __FILE__, __LINE__, bitstream.bits, bitstream.bit_flag);
            if( bitstream.bit_flag < 15 )
            {
                //printf("get read\n");
                if( 0 > read_bitstream(zip_fd, &bitstream) )
                {
                    fprintf(stderr, "read_bitstream error\n");
                    return -1;
                }
            }


            if( array_distance_bits[value] > 0 )
            {
                GET_BITS(bitstream.bits, ch, array_distance_bits[value]);
                bitstream.bit_flag -= array_distance_bits[value];
                distance = array_distance[value] + ch;
                //printf("=[%d] bit=[%d][%x] distance=[%d] zip_win=-------------\n%s\n------------------\n", value, array_distance_bits[value], ch, distance, zip_win);
            }
            else
            {
                distance = array_distance[value];
                //printf("=[%d] bit=[%d] distance=[%d] zip_win=-------------\n%s\n------------------\n", value, array_distance_bits[value], distance, zip_win);
            }

            unit_32_t index_win_temp = index_win;
            index_win_temp = ZIP_WINDOW_SLIDE(index_win_temp, -distance);
            for( index_i = 0; index_i < length; index_i++ )
            {
                zip_win[index_win] = zip_win[index_win_temp]; 
                //printf("index=[%d] insert=[%c]\n", index_i, zip_win[index_win]);
                ret = write(new_fd, &(zip_win[index_win]), 1);
                if( ret != 1 )
                {
                    fprintf(stderr, "[%d] write file errno, errno=[%d]\n", errno);
                    return -1;
                }
            new_fd_count++;
                index_win = ZIP_WINDOW_SLIDE(index_win, 1);             
                index_win_temp = ZIP_WINDOW_SLIDE(index_win_temp, 1);
            }

        }

    }
    //printf("zip context=##################################################\n%s\n#######################################################\n", zip_win);

    /*获取Central directory file header*/

    fprintf(stderr, "index_win length=[%d], new_fd_count=[%d]\n", index_win, new_fd_count);
    ret = close(zip_fd);
    fprintf(stderr, "zip_fd close, ret=[%d]\n", ret);
    ret = close(new_fd);
    fprintf(stderr, "new_fd close, ret=[%d]\n", ret);

    /*获取End of central directory record*/
    return 0;
}
