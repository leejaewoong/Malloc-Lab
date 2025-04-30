/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "w7t5",
    /* First member's full name */
    "Lee jaewoong",
    /* First member's email address */
    "iamlee103@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define WSIZE 4
#define DSIZE 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & (~0x7))
#define BSIZE(size) (ALIGN(size + DSIZE))
#define CHUNKSIZE (1<<12)

#define PUT(p, val) ((*(unsigned int *)(p)) = (val))
#define PACK(size, alloc) (size | alloc) // size는 malloc 함수로부터 입력 받은 파라미터

#define GET(p) (*(unsigned int *)(p))
#define GETSIZE(p) ((GET(p)) & (~0x7))
#define GETALLOC(p) ((GET(p)) & (0x1))

#define HDRP(bp) (((char *)bp) - WSIZE)
#define FTRP(bp) (((char *)bp) + (GETSIZE(HDRP(bp)) - DSIZE)) 

#define NEXT_BLKP(bp) (((char *)bp) + (GETSIZE(HDRP(bp))))
#define PREV_BLKP(bp) (((char *)bp) - (GETSIZE(((char *)bp) - DSIZE))) 

static void* heapListP;


/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // heapListP를 힙의 첫 시작 주소로 세팅 / 패딩 + 프롤로그(헤더, 푸터) + 에필로그(헤더)만큼의 메모리 확보      
    if ((heapListP = mem_sbrk(4 * WSIZE)) == (void *) - 1) 
        return -1;    
    
    // 패딩(4), 프롤로그(헤더(4), 푸터(4)), 에필로그(헤더(4)) 세팅
    PUT(heapListP, 0);   
    PUT(heapListP + (1 * WSIZE), PACK(DSIZE, 1));   
    PUT(heapListP + (2 * WSIZE), PACK(DSIZE, 1));   
    PUT(heapListP + (3 * WSIZE), PACK(0, 1));       
    
    heapListP += DSIZE; // heapListP는 첫 블록(프롤로그)의 payload를 가리킴

    return 0;
}


// 가용 리스트 중에 입력 받은 크기 이상의 블록 탐색 함수
void *findFit(char *bp, size_t newSize)
{           
    bp += DSIZE; // 프롤로그의 PAYLOAD에 위치한 bp를 첫 블록의 PAYLOAD 시작 주소로 이동

    // 주목 블록이 에필로그일 경우, NULL 반환
    if(GETSIZE(HDRP(bp)) == 0) 
        return NULL;
    
    // 에필로그 블록을 만나기 전까지 순회
    while(GETSIZE(HDRP(bp)) != 0)
    {
        // 가용 블록이며 요청한 사이즈 이상의 블록을 찾으면 bp 반환
        if(GETALLOC(HDRP(bp)) == 0 && GETSIZE(HDRP(bp)) >= newSize)         
            return bp;
        
        bp = NEXT_BLKP(bp); // 못 찾았으면 다음 블록의 PAYLOAD로 bp 이동
    }
    
    return NULL; // 적절한 가용 블록이 없으면 NULL 반환    
}


// 블록 할당 시 여분의 공간을 확인하여 자투리 공간 최대 활용
void *place(char *bp, size_t newSize)
{
    // 찾은 가용 블록이 요청받은 사이즈를 할당하고 남은 사이즈를 restSize에 저장
    size_t restSize = GETSIZE(HDRP(bp)) - newSize;
    
    // 가용 블록의 사이즈가 충분하다면,
    if(restSize >= 16)
    {
        // 할당한만큼 사이즈와 할당 상태를 표시
        PUT(HDRP(bp), PACK(newSize, 1));
        PUT(FTRP(bp), PACK(newSize, 1));

        // 남은 블록에 여분의 사이즈와 미할당 상태를 표시
        PUT(FTRP(bp) + WSIZE, PACK(restSize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(restSize, 0));
    }
    
    // 가용 블록의 사이즈가 충분하지 않으면,
    else
    {
        // 가용 블록 전체 사이즈와 할당 상태를 표시
        PUT(HDRP(bp), PACK(GETSIZE(HDRP(bp)), 1));
        PUT(FTRP(bp), PACK(GETSIZE(HDRP(bp)), 1));
    }

    return;
}


void *mm_extend(size_t newSize)
{
    // 힙 확장 실패 시 에러 출력 및 NULL 반환
    void *bp;

    if ((void *)(bp = mem_sbrk(newSize)) == (void *)-1) // 에필로그 헤더를 위해 DSIZE만큼의 추가분 확보
    {
        printf("더 이상 힙을 확장할 수 없습니다\n");            
        return NULL;
    }

    // 힙 확장 성공 시 
    else 
    {
        PUT((char *)bp - WSIZE, PACK(newSize, 1)); // 새로운 블럭의 헤더에 정보 삽입 (지난 에필로그 블록 덮어쓰기)
        PUT(((char *)bp + newSize - DSIZE), PACK(newSize, 1)); // 새로운 블럭의 푸터에 정보 삽입
        PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // 새로운 에필로그 헤더에 정보 삽입
    }

    return (char *)bp;
}


/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t newSize = BSIZE(size); // 입력받은 size를 기반으로 푸터까지 덮어 쓸 수 있는 newSize를 선언
    
    // bp를 힙의 시작으로 초기화하여 newSize 이상의 블록을 탐색
    char *bp = heapListP; 
    bp = findFit(bp, newSize);

    // 반환된 주소가 NULL이 아니면 그대로 반환
    if(bp != NULL)
    {
        bp = place(bp, newSize);
        return (char *)bp;
    }

    // 반환된 주소가 NULL이면, 힙 확장 함수 호출
    else
    {              
        bp = mm_extend(newSize);
        return (char *)bp;    
    }
}     


void coalesce(void *ptr)
{       
    size_t curSize = GETSIZE(HDRP(ptr)); 
    size_t nextSize = 0;
    size_t prevSize = 0;

    // 다음 블록이 가용 블록이면 다음 블록 사이즈 갱신
    if(GETALLOC(HDRP(NEXT_BLKP(ptr))) == 0)
        nextSize = GETSIZE(HDRP(NEXT_BLKP(ptr)));

    // 이전 블록이 가용 블록이면 이전 블록 사이즈 갱신
    if(GETALLOC(HDRP(PREV_BLKP(ptr))) == 0)
        prevSize = GETSIZE(HDRP(PREV_BLKP(ptr)));

    // 경계 블록이 모두 가용 블록이면 이전 블록의 헤더와 다음 블록의 푸터에 사이즈의 합과, 가용 정보를 삽입
    if(prevSize && nextSize)
    {
        PUT(HDRP(PREV_BLKP(ptr)), PACK((prevSize + curSize + nextSize), 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK((prevSize + curSize + nextSize), 0));        
    }      

    // 이전 블록이 가용 블록이면 이전 블록의 헤더와 현재 블록의 푸터에 사이즈의 합과, 가용 정보를 삽입
    else if(prevSize && !nextSize)
    {
        PUT(HDRP(PREV_BLKP(ptr)), PACK((prevSize + curSize), 0));
        PUT(FTRP(ptr), PACK((prevSize + curSize), 0));        
    }

    // 다음 블록이 가용 블록이면 현재 블록의 헤더와 다음 블록의 푸터에 사이즈의 합과, 가용 정보를 삽입
    else if(nextSize && !prevSize)
    {
        PUT(HDRP(ptr), PACK((curSize + nextSize), 0));
        PUT(FTRP(ptr), PACK((curSize + nextSize), 0));        
    }
    
    return;
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{    
    if (ptr == NULL)
        return;  
        
    PUT(HDRP(ptr), PACK((GETSIZE(HDRP(ptr))), 0));
    PUT(FTRP(ptr), PACK((GETSIZE(HDRP(ptr))), 0));   

    coalesce(ptr);
    
    return;
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *p, size_t size)
{    
    void *bp; // 새로 할당할 메모리를 가리킬 포인트 선언

    // p가 NULL일 경우 malloc 함수를 호출해 새 메모리 할당
    if(p == NULL)
    {
        bp = mm_malloc(size);               
        return bp;
    }

    // 사이즈가 0일 경우, 해당 주소의 메모리를 해제
    if(size == 0)
    {
        mm_free(p); 
        return NULL;
    }
    
    size_t oldSize = GETSIZE(HDRP(p)); // 기존 블록의 사이즈를 저장
    size_t newSize = BSIZE(size); // 요청받은 블록의 사이즈를 저장
    
    // 두 사이즈가 같을 경우 주소를 그대로 반환
    if(oldSize == newSize)
        return p;

    // 기존 사이즈가 더 클 경우, place 함수를 호출하여 필요한만큼 메모리를 할당하고 나머지는 자투리 공간으로 활용
    else if(oldSize > newSize)
    {
        place(p, newSize);
        return p;
    }

    // 요청 사이즈가 더 클 경우 malloc 함수를 호출
    else if(oldSize < newSize)
    {
        bp = mm_malloc(size);

        if(bp == NULL)
            return NULL;      
        
        // 새롭게 할당한 메모리에 기존 블록의 내용 일부를 복사, 기존 메모리는 해제
        memcpy(bp, p, oldSize-DSIZE);
        mm_free(p);
        return bp;
    }    
}






