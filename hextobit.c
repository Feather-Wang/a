#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

int main(int argc, const char *argv[])
{
    int i = 0;
    int num = 0;
    if( argc == 1 )
    {
        fprintf(stderr, "Usage: ./a.out 4b 2f 3c\n");
        return -1;
    }

    for(i = 1; i < argc; i++)
    {
        switch(argv[i][1])
        {
            case 'a':
                printf("0101");
                break;
            case 'b':
                printf("1101");
                break;
            case 'c':
                printf("0011");
                break;
            case 'd':
                printf("1011");
                break;
            case 'e':
                printf("0111");
                break;
            case 'f':
                printf("1111");
                break;
            default:
                num = argv[i][1] - 48;
                printf("%d%d%d%d", num&0x1, (num>>1)&0x1, (num>>2)&0x1, (num>>3)&0x1);
                break;
        }
        switch(argv[i][0])
        {
            case 'a':
                printf("0101");
                break;
            case 'b':
                printf("1101");
                break;
            case 'c':
                printf("0011");
                break;
            case 'd':
                printf("1011");
                break;
            case 'e':
                printf("0111");
                break;
            case 'f':
                printf("1111");
                break;
            default:
                num = argv[i][0] - 48;
                printf("%d%d%d%d", num&0x1, (num>>1)&0x1, (num>>2)&0x1, (num>>3)&0x1);
                break;
        }
        printf("(%s) ", argv[i]);
    }
    printf("\n");
    return 0;
}
