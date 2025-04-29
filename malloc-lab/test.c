#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* heapListP;

#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & (~0x7))
#define BSIZE(size) ((ALIGN(size) + DSIZE))

#define PUT(p, val) ((*(unsigned int *)(p)) = (val))
#define PACK(size, alloc) ((BSIZE(size)) | (alloc)) // size는 malloc 함수로부터 입력 받은 파라미터

#define GET(p) (*(unsigned int *)(p))
#define GETSIZE(p) ((GET(p)) & (~0x7))
#define GETALLOC(p) ((GET(p)) & (0x1))

#define HDRP(bp) (((char *)bp) - WSIZE)
#define FTRP(bp) (((char *)bp) + (GETSIZE(HDRP(bp))) - DSIZE)

#define NEXT_BLKP(bp) (((char *)bp) + (GETSIZE(HDRP(bp))))
#define PREV_BLKP(bp) (((char *)bp) - (GETSIZE(((char *)bp) - DSIZE)))

#define TYPE_NAME(x) \
    _Generic((x),   \
        char:       "char",            \
        signed char:"signed char",     \
        unsigned char:"unsigned char", \
        short:      "short",           \
        unsigned short:"unsigned short",\
        int:        "int",             \
        unsigned int:"unsigned int",   \
        long:       "long",            \
        unsigned long:"unsigned long", \
        long long:  "long long",       \
        unsigned long long:"unsigned long long", \
        float:      "float",           \
        double:     "double",          \
        long double:"long double",     \
        default:    "unknown"          \
    )

int main(void) {

    unsigned int *p = malloc(32);    // 임의 메모리 공간
    void *bp = (void *)((char *)p + WSIZE);  // payload 시작 부분 가정
    PUT(p, PACK(32, 1));   // p에 임의의 값 삽입

    // printf로 “타입 이름”을 찍어 보기
    printf("ALIGNMENT: %d is type %s\n",ALIGNMENT, TYPE_NAME(ALIGNMENT));
    printf("ALIGN(32): %d is type %s\n",ALIGN(32), TYPE_NAME(ALIGN(32)));        
    printf("BSIZE(32): %d is type %s\n",BSIZE(32), TYPE_NAME(BSIZE(32)));    
    printf("WSIZE: %d is type %s\n",WSIZE, TYPE_NAME(WSIZE));    
    printf("DSIZE: %d is type %s\n",DSIZE, TYPE_NAME(DSIZE));    

    printf("PACK(32, 1): %d is type %s\n",PACK(32, 1), TYPE_NAME(PACK(32, 1)));        

    printf("GET(p): %u is type %s\n", GET(p), TYPE_NAME(GET(p)));    
    printf("GETSIZE(p): %u is type %s\n", GETSIZE(p), TYPE_NAME(GETSIZE(p)));    
    printf("GETALLOC(p): %u is type %s\n", GETALLOC(p), TYPE_NAME(GETALLOC(p)));

    printf("bp: %p\n", bp);
    printf("HDRP(bp): %p\n", HDRP(bp));
    printf("FTRP(bp): %p\n", FTRP(bp));    
    printf("NEXT_BLKP(bp): %p\n", NEXT_BLKP(bp));    
    printf("PREV_BLKP(bp): %p\n", PREV_BLKP(bp));
    free(p);
    return 0;
}

