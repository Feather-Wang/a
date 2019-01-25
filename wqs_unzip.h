#ifndef WQS_UNZIP_H__

#define WQS_UNZIP_H__

/*设置对齐字节数*/
#pragma pack(1)

#define SIGNATURE_HEADER_LOCAL_FILE 0x04034B50
#define SIGNATURE_HEADER_CENTRAL_DIRECTORY_FILE 0x02014B50
#define SIGNATURE_END_OF_CENTRAL_DIRECTORY_RECORD 0x06054B50

#define ZIP_WINDOW 32768    /*32KB*/
#define ZIP_WINDOW_SLIDE(index, num)    ((index + num) % ZIP_WINDOW)

typedef unsigned char unit_8_t;
typedef unsigned short unit_16_t;
typedef unsigned int unit_32_t;

static unit_8_t ccl_reverse[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
static unit_16_t array_length[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static unit_16_t array_length_bits[] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
static unit_16_t array_distance[] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
static unit_16_t array_distance_bits[] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

#define GET_BITS(bits,recv,num) { recv = bits & (0xffffffff>>(32-num)); bits = bits >> num; }
#define GET_ARRAY_INDEX(array,size,i,value,index) for(i=0;i<size;i++){if(array[i]==value) {index=i;break;}}


struct header_local_file_s
{
    unit_32_t       signature;
    unit_16_t       extract_version;
    unit_16_t       general_flag;
    unit_16_t       compress_method;
    unit_16_t       file_last_modification_time;
    unit_16_t       file_last_modification_date;
    unit_32_t       crc32_mothed;
    unit_32_t       filesize_after_compress;
    unit_32_t       filesize_before_compress;
    unit_16_t       filename_len;
    unit_16_t       extra_field_len;
    unit_8_t       *filename;
    unit_8_t       *extra_field;
};
typedef struct header_local_file_s header_local_file_t;

struct header_central_directory_file_s
{
    unit_32_t       signature;
    unit_16_t       pkware_version;
    unit_16_t       extract_version;
    unit_16_t       general_flag;
    unit_16_t       compress_method;
    unit_16_t       file_last_modification_time;
    unit_16_t       file_last_modification_date;
    unit_32_t       crc32_mothed;
    unit_32_t       filesize_after_compress;
    unit_32_t       filesize_before_compress;
    unit_16_t       filename_len;
    unit_16_t       extra_field_len;
    unit_16_t       file_comment_len;
    unit_16_t       disk_num;
    unit_16_t       internal_file_attributes;
    unit_32_t       external_file_attributes;
    unit_32_t       relative_offset_of_local_file_header;
    unit_8_t       *filename;
    unit_8_t       *extra_field;
    unit_8_t       *file_comment;
};
typedef struct header_central_directory_file_s header_central_directory_file_t;

struct bits_header_s
{
    unit_8_t header;
    unit_8_t hufman_type;
    unit_8_t hlit;
    unit_8_t hdist;
    unit_8_t hclen;
};
typedef struct bits_header_s bits_header_t;

typedef struct {
    unit_16_t value;
    int plies;
} array_unit_t;

#endif /* end of include guard: WQS_UNZIP_H__ */
