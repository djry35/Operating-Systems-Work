#ifndef _PAGE_SWAP_H_
#define _PAGE_SWAP_H_

#include <stdbool.h>
#include <block_store.h>

#define MAX_PAGE_TABLE_ENTRIES_SIZE 2048
#define MAX_PHYSICAL_MEMORY_SIZE 512

typedef struct {
	unsigned int pageTableIdx;
	unsigned char data[1024];
	unsigned char accessTrackingByte;
	unsigned char accessBit;
}Frame_t;

typedef struct {
	Frame_t entries[MAX_PHYSICAL_MEMORY_SIZE];
	unsigned int size;
}FrameTable_t;


typedef struct {
	unsigned int frameIndex;
	unsigned int placeInList;
	uint8_t blockInMemory;
	bool isValid;	
} Page_t;


typedef struct {
	Page_t entries[MAX_PAGE_TABLE_ENTRIES_SIZE];	
	unsigned int size;
}PageTable_t;


typedef struct {
	unsigned short pageRequested;
	unsigned short frameReplaced;
	unsigned short pageReplaced;
}PageAlgorithmResults;

//uuuuuuuuuh my file couldn't reference the structure definition in the 
//library, so I guess I have to put it here to make it compile.
struct block_store {
	bitmap_t *dbm;
	bitmap_t *fbm;
	uint8_t* data_blocks;
};

typedef struct block_store block_store_t;

bool initialize_back_store (void);

void destroy_back_store(void);

bool initailize_frame_list(void);

void destroy_frame_list(void);

PageAlgorithmResults* least_recently_used(const uint32_t pageNumber);

PageAlgorithmResults* approx_least_recently_used (const uint32_t pageNumber, const size_t timeInterval);

int compareAccessByte(const void* a, const void* b);

bool read_from_back_store (Frame_t* frameToFill, uint8_t blockToRead);

bool write_to_back_store (Frame_t* frameWithData, uint8_t blockToWrite);

dyn_array_t* read_page_requests (const char* const filename);

bool initialize (void); 

#endif
