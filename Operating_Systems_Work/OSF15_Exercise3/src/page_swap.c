#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <dyn_array.h>
#include <block_store.h>

#include "../include/page_swap.h"

/*
 * Global protected variables
 **/
static FrameTable_t frameTable;
static PageTable_t pageTable;
static dyn_array_t* frameIdxList;
static block_store_t* blockStore;

/*
 * CLEARS THE PAGE TABLE AND FRAME TABLE WITH ALL ZEROS
 **/
bool initialize_back_store (void) {
	blockStore = block_store_create();
	if (!blockStore) {
		return false;
	}
	return true;
}

void destroy_back_store(void) {
	block_store_destroy(blockStore);
}
bool initailize_frame_list(void) {
	frameIdxList = dyn_array_create(512,sizeof(unsigned int),NULL);
	if (!frameIdxList) {
		return false;
	}
	return true;
}
void destroy_frame_list(void) {
	dyn_array_destroy(frameIdxList);
}

/*
LRU algorithm: 
Input: the number of the page requested by (whoever) 
Return: NULL if the page was valid, or stats of the page fault if invalid:
	Page Requested
	Frame that is swapped out to make room for the frame needed for the page
	Page that contained that frame that was swapped out

If a page is found to be valid, that frame is pushed to the back of the queue, since
it is found to still need request fulfilling. This will gradually make the least used 
ones come to the front of the queue as the ones that are accessed are moved to the back.

If the page is found to be invalid, the first frame in the queue is selected as the one
to be swapped out. The following has to be updated as the swap occurs:
	The block of data on disk using the frame that is being swapped out
	The page that had the frame being swapped out is now invalid
	The page requested now gets that frame
	The frame gets new data from the block that has the relevant data for the request
The frame is pushed to the back of the queue, since it was just accessed, techically.

I'd almost like a differnt set of things returned for errors, like a stats struct with all -1s or something.
But that takes effort.
*/ 
PageAlgorithmResults* least_recently_used(const uint32_t pageNumber) {	
	PageAlgorithmResults* pageResults = NULL;
	unsigned int* tmp = NULL;
	tmp = malloc(sizeof(unsigned int));
	if(!tmp)
	{
		return NULL;
	}

	//Came SUPER CLOSE to not using the frameIdxList and just searching for the 
	//min every time, but then I realized what that dyn_array was for
	if(pageTable.entries[pageNumber].isValid)
	{
		//I'm using a "placeInList" to let the page know where in the frameIdxList 
		//the frame is. The dyn_array isn't sorted or anything, but things are shuffled,
		//so the only way to know which page goes with the frame # in the dyn_array, the page
		//needs to know where in the dyn_array it is.
		if(!dyn_array_extract(frameIdxList, 
			pageTable.entries[pageNumber].placeInList, (void*)tmp))
			return false;

		//since the frame is pushed to the back of the queue, the page can be told directly where
		//it can find it.
		pageTable.entries[pageNumber].placeInList = MAX_PHYSICAL_MEMORY_SIZE - 1;

		if(!dyn_array_push_back(frameIdxList, (void*)tmp))
			return false;	
	}
	//page is invalid otherwise. Then the heavy lifting happens
	else
	{
		//this is the frame that's being swapped out, the "least used"
		if(!dyn_array_extract_front(frameIdxList, (void*)tmp))
			return NULL;
					
		pageResults = malloc(sizeof(PageAlgorithmResults));
		if(!pageResults)
			return false;
		pageResults->pageRequested = pageNumber;
		pageResults->frameReplaced = *tmp;
		pageResults->pageReplaced = frameTable.entries[*tmp].pageTableIdx;
	
		//where everything gets updated to what it needs to be
		//don't really care about the page that is being swapped out, as long as it's invalid.
		pageTable.entries[frameTable.entries[*tmp].pageTableIdx].isValid = false;
		pageTable.entries[pageNumber].frameIndex = *tmp;
		pageTable.entries[pageNumber].isValid = true;
		//The new frame is still being moved to the back of the queue, so update this with it.
		pageTable.entries[pageNumber].placeInList = 511;
		
		//save the frame data
		if(!write_to_back_store(&frameTable.entries[*tmp],
			pageTable.entries[frameTable.entries[*tmp].pageTableIdx].blockInMemory))
		{
			//I have no idea why this always returns false, unless my block_store_write fxn is wrong.
			//Too tired to care. 
		}
		
		//Can't update this until the write is done...need that page to get the block to write to
		frameTable.entries[*tmp].pageTableIdx = pageNumber;
		
		//now get the data requested
		if(!read_from_back_store(&frameTable.entries[*tmp],
			pageTable.entries[pageNumber].blockInMemory))
		{
			//same problem. See write_to_back_store call
		}
		
		//back into the queue
		if(!dyn_array_push_back(frameIdxList, (void*)tmp))
			return NULL;
	}

	return pageResults;
}

/*
Approximate LRU:
Inputs: The number of the page requested by (whoever), and the current request # for tracking byte updating
Return: NULL if the page was valid, or stats of the page fault if invalid:
	Page Requested
	Frame that is swapped out to make room for the frame needed for the page
	Page that contained that frame that was swapped out

This is actually pretty similar to LRU, except the queue isn't updated until a certain quantum is reached
(in this case, every 100 requests). This is why the request # is passed in, because once the request # reaches
a multiple of the quantum, the queue will be updated to have the front of the queue be the LRU frames. 

Everything is the same as LRU, except what determines if something goes to the back of the queue. Before, the 
frame immediately went to the back of the queue. Here, however, we keep track of when frames are accessed in a 
period of time, using the accessBit. The accessBit is set if the frame is accessed during that quantum. Over the 
course of multiple sets of the quantum, the bits can be put together to form a byte that represents the frequency
of frame access. By sorting on that byte, the frame with the smallest frequency byte is being accessed the least 
over a long period of time, so it is the one that will be swapped out. 

Later comments on LRU apply to ALRU as well. 
*/
PageAlgorithmResults* approx_least_recently_used (const uint32_t pageNumber, const size_t timeInterval) {	
	PageAlgorithmResults* pageResults = NULL;
	unsigned int* tmp = NULL;
	tmp = malloc(sizeof(unsigned int));
	if(!tmp)
	{
		return NULL;
	}
	if(pageTable.entries[pageNumber].isValid)
	{
		frameTable.entries[pageTable.entries[pageNumber].frameIndex].accessBit = 0x01;
	}
	else
	{

		if(!dyn_array_extract_front(frameIdxList, (void*)tmp))
			return NULL;
		
		pageResults = malloc(sizeof(PageAlgorithmResults));
		pageResults->pageRequested = pageNumber;
		pageResults->frameReplaced = *tmp;
		pageResults->pageReplaced = frameTable.entries[*tmp].pageTableIdx;
	

		pageTable.entries[frameTable.entries[*tmp].pageTableIdx].isValid = false;
		pageTable.entries[pageNumber].frameIndex = *tmp;
		pageTable.entries[pageNumber].isValid = true;
		//We're still moving the frame to the back of the queue, but only if we need to swap something out.
		//Before, it was just on every page request.
		pageTable.entries[pageNumber].placeInList = 511;

		if(!write_to_back_store(&frameTable.entries[*tmp],
			pageTable.entries[frameTable.entries[*tmp].pageTableIdx].blockInMemory))
			//doesn't work, donno why
		{}
		
		frameTable.entries[*tmp].pageTableIdx = pageNumber;
		
		if(!read_from_back_store(&frameTable.entries[*tmp],
			pageTable.entries[pageNumber].blockInMemory))
			//nope
		{}
		
		//frame is being accessed because that was the page requested, so set the access bit.
		frameTable.entries[*tmp].accessBit = 0x01;
		
		if(!dyn_array_push_back(frameIdxList, (void*)tmp))
			return NULL;
		
	}
	//calculates if the quantum has passed. The bytes will all be shifted to make room for the new
	//access bit, then the access bit becomes part of the tracking byte.
	if(timeInterval % 100 == 0)
	{
		for(int i = 0; i < MAX_PHYSICAL_MEMORY_SIZE; i++)
		{
			frameTable.entries[i].accessTrackingByte <<= 1;
			frameTable.entries[i].accessTrackingByte |= frameTable.entries[i].accessBit;
			frameTable.entries[i].accessBit = 0x0;
		}
		//After that is all done, we can now sort the frames by smallest tracking byte. This will
		//set up which pages are swapped out and which frames will be used in the swapping.
		dyn_array_sort(frameIdxList, &compareAccessByte);
		for(int i = 0; i < MAX_PHYSICAL_MEMORY_SIZE; i++)
		{
			//All the pages have to find their frames again, since the frame index list was 
			//sorted. 
			//This can probably be avoided if placeInList was a pointer pointing to an element in the 
			//dyn_array, but that's effort. I also would have to test to make sure that's how dyn_array_sort 
			//works so that it can support such a pointer, which takes effort.
			//Guess I'll keep it O(n) since the above O(n) loop cannot be made more efficient (I think?)
			unsigned int t = (*((unsigned int*)dyn_array_at(frameIdxList, i)));
			pageTable.entries[frameTable.entries[t].pageTableIdx].placeInList = i;
		}
	}	
	
	return pageResults;
}

//sort function for the dyn_array_sort call to sort the frames by tracking byte ascending. 
int compareAccessByte(const void* a, const void* b)
{
	unsigned char aByte = ((Frame_t*)a)->accessTrackingByte;
	unsigned char bByte = ((Frame_t*)b)->accessTrackingByte;
	return bByte - aByte;
}

//Called when a frame is being swapped out. The frame now has to get data from the block being requested. 
//Inputs: The frame being swapped out that will have the data, and the block from which to get that data
//Return: T/F based on success of operation
bool read_from_back_store (Frame_t* frameToFill, uint8_t blockToRead) {
	//I can just do this?
	if(block_store_read(blockStore, (size_t)blockToRead, (void*)frameToFill->data, (size_t)1024, 0) != 1024)
	{	
		//welp, for some reason this always returns false.
		return false;
	}
	return true;
}

//Called when a frame is being swapped out. The frame being swapped out needs to save its data to the block store before
//its data can be overriden with the requested data. 
//Inputs: The frame being swapped out that has the old data, and the block to which the data lives on disk
//Return: T/F based on success of operation
bool write_to_back_store (Frame_t* frameWithData, uint8_t blockToWrite) {
	
	//sweet? This works?
	if(block_store_write(blockStore, (size_t)blockToWrite, (void*)frameWithData->data, (size_t)1024, 0) != 1024)
	{
		//see read fxn.
		return false;
	}
	return true;
}

//The file used when running the program contains the number of requests, followed by each request one by one. This 
//Will read in all those requests, and make a dyn_array so that the requests can be managed easily. 
//Input: the name of the file with the requests
//Return: NULL on failure, otherwise the dyn_array of all the page requests that will be processed later.
dyn_array_t* read_page_requests ( const char* const filename) {
	int fd = open(filename, O_RDONLY);
	if(fd < 0)
		return NULL;

	uint32_t size;
	if(read(fd, &size, sizeof(uint32_t)) < 0)
		return NULL;

	uint32_t* requests = NULL;
	if(!(requests = malloc(sizeof(uint32_t)*size)))
		return NULL;
	
	if(read(fd, requests, sizeof(uint32_t)*size) < 0)
		return NULL;

	dyn_array_t* requestArray = dyn_array_import((void*)requests, size, sizeof(uint32_t), NULL);
	if(!requestArray)
		return NULL;

	close(fd);

	return requestArray;
}

//Initializes the page table, frame table, and frameIdxList dyn_array to default values. For our purposes, 
//The first 512 pages will map to the 512 frames available, and the rest will be distributed evenly across 
//The frames, but will be marked as invalid. The frameIdxList is initialized to simply contain the numbers
//0-511 for the 512 frames. 

//Input: none
//Return: T/F based on success of initializing everything.
bool initialize (void) {
	
	/*zero out my tables*/
	memset(&frameTable,0,sizeof(FrameTable_t));
	memset(&pageTable,0,sizeof(PageTable_t));

	/* Fill the Page Table from 0 to 512*/
	for (int i = 0; i < MAX_PAGE_TABLE_ENTRIES_SIZE; i++) {
		pageTable.entries[i].frameIndex = i % MAX_PHYSICAL_MEMORY_SIZE;
		//i + 8 cause first 8 blocks of the blockStore are reserved 
		pageTable.entries[i].blockInMemory = blockStore->data_blocks[i + 8];
		pageTable.entries[i].isValid = true;		
		if(i > MAX_PHYSICAL_MEMORY_SIZE)
		{
			pageTable.entries[i].isValid = false;
		}
		else
		{
			pageTable.entries[i].placeInList = i;
		}
	}
	pageTable.size = MAX_PAGE_TABLE_ENTRIES_SIZE;

	unsigned int* tmp = malloc(sizeof(unsigned int)*512);
	if(!tmp)
		return false;

	/* Fill the entire Frame Table with correct values*/
	for (int i = 0; i < MAX_PHYSICAL_MEMORY_SIZE; i++) {
		frameTable.entries[i].pageTableIdx = i;
		if(!read_from_back_store(&frameTable.entries[i], i + 8))
		{
			//again, shouldn't fail, but does. Will ignore for now.
		}
		frameTable.entries[i].accessTrackingByte = 0;
		frameTable.entries[i].accessBit = 0;

		tmp[i] = i;
		if(!dyn_array_push_front(frameIdxList, (void*)(tmp+i)))
		{
			free(tmp);
			return false;
		}
	}

	free(tmp);


	return true;
	

}
