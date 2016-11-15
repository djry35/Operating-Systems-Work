#include "../include/FonefiveFS.h"
#include <string.h>
#include <limits.h>

// redefine because it doesn't hurt and it's good to have for refrence
typedef enum {NONE = 0x00, FILE_LINKED = 0x01, FILE_BASED = 0x02, DIRTY = 0x04, ALL = 0xFF} BS_FLAGS;

//it never likes it when this definition is not around.
struct block_store {
    int fd; // R/W position never guarenteed, flag indicates link state (attempt to set it to -1 when not in use as well)
    BS_FLAGS flags;
    bitmap_t *dbm;
    bitmap_t *fbm;
    uint8_t *data_blocks;
};



/*
    typedef enum {
        REGULAR = 0x01, DIRECTORY = 0x02
    } ftype_t;

    // It's a directory entry. Won't really be used internally
    typedef {
        ftype_t ftype;
        char fname[FNAME_MAX+1];
    } dir_entry_t;

    // It's a directory record, used to report directory contents to the user
    // Won't really be used internally
    typedef struct dir_rec {
        int total; // total valid entries
        dir_entry_t contents[DIR_REC_MAX];
    } dir_rec_t;
*/


// That's it?
struct F15FS {
    block_store_t *bs;
};

typedef inode_ inode_t;

// directory entry, found inside directory blocks
typedef struct {
    char fname[FNAME_MAX + 1];
    inode_ptr_t inode;
} dir_ent_t;

// Sadly, not the same size as an mdata_t, but it's close! 44 Bytes
// They can really be cast back and forth safely since the end is padding anyway
// But if you have an array of them, it could get messy, but why would you ever have one of those?
typedef struct {
    uint32_t size;
    // Number of VALID entries, ENTRIES ARE NOT CONTIGUOUS
    // Which means all entries will need to be scanned every time
    // Maintaining contiguity can be done, and in a transaction safe way, but it's extra work

    // All metadata is contained in the inode, except for size
    // Which leaves... 40B
    uint8_t padding[40];
} dir_mdata_t;

// A directory file. Directories only have one for simplicity
typedef struct {
    dir_mdata_t mdata;
    dir_ent_t entries[DIR_REC_MAX];
} dir_block_t;


// size of data block
#define BLOCK_SIZE 1024

typedef uint8_t data_block_t[BLOCK_SIZE]; // data's just data, also c is weird

// total inode blocks
#define INODE_BLOCK_TOTAL 32

// Number of inodes in a block
#define INODES_PER_BLOCK (BLOCK_SIZE / sizeof(inode_t))

// Total number of inodes
#define INODE_TOTAL ((INODE_BLOCK_TOTAL * BLOCK_SIZE) / sizeof(inode_t))

// Inode blocks start at 8 and go to 40
#define INODE_BLOCK_OFFSET 8

#define DATA_BLOCK_OFFSET (INODE_BLOCK_OFFSET + INODE_BLOCK_TOTAL)

// 6 direct blocks per inode
#define DIRECT_TOTAL 6
// number of directs in an indirect, same as in an inode
#define INDIRECT_TOTAL (BLOCK_SIZE / sizeof(block_ptr_t))
// number of directs in a full double indirect, same as in an inode
#define DBL_INDIRECT_TOTAL (INDIRECT_TOTAL * INDIRECT_TOTAL)

// I did the math... but it can also be calculated
#define FILE_SIZE_MAX ((DIRECT_TOTAL + INDIRECT_TOTAL + DBL_INDIRECT_TOTAL) * BLOCK_SIZE)

// Total available data blocks since the block_store is smaller than the indexing allows
// Might not be used
#define DATA_BLOCK_MAX 65536

// It should really be less (about 12100)
// But we'll overallocate, preferably not on the stack...
#define FS_PATH_MAX 13000

// Calcs what block an inode is in
#define INODE_TO_BLOCK(inodeT) (((inodeT) >> 3) + INODE_BLOCK_OFFSET)

// Calcs the index an inode is at within a block
#define INODE_INNER_IDX(inodeT) ((inodeT) & 0x07)

// Calcs the offset of an inode because I made block_store too kind
// and I can't undo it. Immagine how much of a pain it'd be if you could only read/write entire blocks
// (...like how actual block devices work)
#define INODE_INNER_OFFSET(inodeT) (INODE_INNER_IDX(inodeT) * sizeof(inode_t))

// Converts a file position to a block index (note: not a block id. index 6 is the 6th block of the file)
#define POSITION_TO_BLOCK_INDEX(position) ((position) >> 10)

// Position within a block
#define POSITION_TO_INNER_OFFSET(position) ((position) & 0x3FF)

// tells you if the given block index makes sense
#define BLOCK_IDX_VALID(block_idx) ((block_idx) >= DATA_BLOCK_OFFSET && (block_idx) < DATA_BLOCK_MAX)

// Checks that an inode is the specified type
// Used because the mdata type won't be the same size as the enum and that'll irritate the compiler probably
#define INODE_IS_TYPE(inode_ptr, file_type) ((inode_ptr)->mdata.type & (file_type))

// Because you can't increment an incomplete type
// And writing out the cast every time gets tiring
#define INCREMENT_VOID_PTR(v_ptr, increment) (((uint8_t *)v_ptr) + (increment))



typedef struct {
	uint8_t inode_idx;
	bool isValid;
	//implement at the end...this might be messy.
	uint8_t Oflag;
	uint8_t mode;
} openFile;

typedef struct {
	//guess all files could be open at the same time, technically?	
	openFile table[INODE_TOTAL];
} openFileTable;

//FOR BONUS
openFileTable openFiles;

// Casual warning, if it hasn't been used yet, it might not work yet.
// Also, my bugs are your bugs if you decide to use this. be careful.

//
// Creates a new F15FS file at the given location
// param fname The file to create (or overwrite)
// return Error code, 0 for success, < 0 for error
//
// This is why gotos for resource unwinding is nice. Look at this mess.
int fs_format(const char *const fname) {
    // Gotta do a create and then a link
    if (fname && fname[0]) { // Well, it works.
        block_store_t *bs = block_store_create();
        if (bs) {
            block_store_link(bs, fname);
            if (block_store_errno() == BS_OK) {
                // Ok, made a block store, linked and made the file, now to fill out the inodes
                //  and by that I mean, request the inode table and a data block
                //  and fill out root and its data block
                bool success = true;
                for (size_t i = INODE_BLOCK_OFFSET; i < (INODE_BLOCK_OFFSET + 32); ++i) {
                    success &= block_store_request(bs, i); // haha! C standard says that true is 1, so this works
                }
                if (success) {
                    size_t root_file_block = block_store_allocate(bs); // don't really care where it goes
                    if (root_file_block) {
                        // Ok, NOW we're good... assuming all the writes work. Ugh.
                        inode_t root;
                        dir_block_t root_dir;
                        memset(&root, 0, sizeof(inode_t));
                        memset(&root_dir, 0, sizeof(dir_block_t));

                        root.fname[0] = '/'; // Technically probably an invalid name, but less invalid than an empty string
                        root.mdata.size = 0; // Uhhh...? Not really going to use it in a directory
                        root.mdata.mode = 0; // it's not being used, who cares.
                        root.mdata.c_time = time(NULL); // Time in seconds since epoch. 32-bit time is a problem for 2038
                        root.mdata.a_time = time(NULL); // I guess technically never?
                        root.mdata.m_time = time(NULL); // technically later, but ehhh.
                        root.mdata.parent = 0; // Us!
                        root.mdata.type = (uint8_t) DIRECTORY;
                        root.data_ptrs[0] = root_file_block; // the rest have init to 0
			root.mdata.inUse = true;
                        root_dir.mdata.size = 0; // Uhhh...?

                        if (block_store_write(bs, INODE_TO_BLOCK(0), &root, sizeof(inode_t), 0) == sizeof(inode_t) &&
                                block_store_write(bs, root_file_block, &root_dir, sizeof(dir_block_t), 0) == sizeof(dir_block_t)) {
                            // technically we could loop till it goes, but the way it's written, if it didn't work the first time,
                            //  something's super broke

                            // Ok... Done? Req'd the table, setup root and its folder, wrote it all, time to sync.
                            block_store_destroy(bs, BS_FLUSH);
                            // flushing sets the block_store_errno() to the flush status, so...
                            if (block_store_errno() == BS_OK) {
                                // FINALLY.
                                return 0;
                            }
                            fprintf(stderr, "Flush died, block_store says: %s\n", block_store_strerror(block_store_errno()));
                            // can't destruct because we already did
                            return -1;
                        }
                        fprintf(stderr, "Something didn't write, block_store says: %s\n", block_store_strerror(block_store_errno()));
                        block_store_destroy(bs, BS_FLUSH); // Flushing might not work, but if it does, it might have nice debug info
                        return -1;
                    }
                    fprintf(stderr, "Couldn't request root file, block_store says: %s\n", block_store_strerror(block_store_errno()));
                    block_store_destroy(bs, BS_FLUSH);
                    return -1;
                }
                fprintf(stderr, "Couldn't request inode table, block_store says: %s\n", block_store_strerror(block_store_errno()));
                block_store_destroy(bs, BS_FLUSH);
                return -1;
            }
            fprintf(stderr, "Couldn't link block_store, block_store says: %s\n", block_store_strerror(block_store_errno()));
            block_store_destroy(bs, BS_NO_FLUSH);
            return -1;
        }
        fprintf(stderr, "Couldn't create block_store, block_store says: %s\n", block_store_strerror(block_store_errno()));
        return -1;
    }
    fprintf(stderr, "Filename \"%s\" invalid\n", fname); // It'll either print "(null)" or ""
    return -1;
}

//
// Mounts the specified file and returns an F15FS object
// param fname the file to load
// return An F15FS object ready to use, NULL on error
//
F15FS_t *fs_mount(const char *const fname) {
    // open the bs object and... that's it?
    F15FS_t *fs = malloc(sizeof(F15FS_t));
    if (fs) {
        // this will catch all fname issues
        fs->bs = block_store_import(fname);
        if (fs->bs) {
            if (block_store_errno() == BS_OK) {
                return fs;
            }
            fprintf(stderr, "Issue with import (link problem?), block_store says: %s", block_store_strerror(block_store_errno()));
            block_store_destroy(fs->bs, BS_NO_FLUSH);
            free(fs);
            return NULL;
        }
        fprintf(stderr, "Couldn't open file \"%s\", block_store says: %s\n", fname, block_store_strerror(block_store_errno()));
        free(fs);
        return NULL;
    }
    fprintf(stderr, "Couldn't malloc fs object");
    return NULL;
}

// Unmounts, closes, and destructs the given object,
//  saving all unwritten contents (if any) to file
// param fs The F15FS file
// return 0 on success, < 0 on error. Object is always destructed, if it exists
//
int fs_unmount(F15FS_t *fs) {
    if (fs) {
        block_store_destroy(fs->bs, BS_FLUSH);
        free(fs);
        if (block_store_errno() == BS_OK) {
            return 0;
        }
        fprintf(stderr, "BS_DESTROY failed to flush? Block_store says: %s", block_store_strerror(block_store_errno()));
    }
    return -1;
}

/*
Takes data, the size of the data, the offset to start writing to the file, and the filename. Writes the data out to that file.
*/
ssize_t fs_write_file(F15FS_t *const fs, const char *const fname, const void *data, size_t nbyte, size_t offset)
{
	//error check
	if(!fs || !fs->bs || !fs->bs->dbm || !fs->bs->fbm || !fname || !fs->bs->data_blocks)
	{
		//eh? better than this?
		return -1;
	}
	//size_t totalWritten = 0;
	
	//find the file, make sure it's okay
	inode_t inode;

	char* child = malloc(sizeof(char)*FNAME_MAX);
	char* parent = malloc(sizeof(char)*FNAME_MAX);
	uint8_t check = extractNames(fname, parent, child);
	if(check < 0)
	{
		return check;
	}

	//the file should exist and not be a directory, I assume you can't write to a directory.		
	if(!doesExist(fs, fname, DIRECTORY, 0))
	{
		return -1;
	}

	uint8_t i;
	for(i = 0; i < INODE_TOTAL; i++)
	{	
		block_store_read(fs->bs, INODE_TO_BLOCK(i), (void*)&inode, sizeof(inode_t), INODE_INNER_OFFSET(i));
		if(strcmp(inode.fname, parent) == 0)
		{
			//sweet, read the correct inode in.
			break;
		}
	}
	
	//should not get in here, for testing purposes
	if(i == INODE_TOTAL)
	{
		return -1;
	}	

	bool atEOF = false;
	uint8_t util = 0;// = data;
	uint8_t whichBlock;
	while(1)
	{
		//initial search for the block we're writing to.
		size_t block = POSITION_TO_BLOCK_INDEX(offset);
		size_t blockOffset = POSITION_TO_INNER_OFFSET(offset);
		block_ptr_t blockRequested = findBlockInFS(fs, block, &whichBlock);

		if(inode.mdata.size < nbyte + blockOffset)
		{
			//We're at EOF, sooooooo....
			atEOF = true;
		}
		
		if(atEOF)
		{
			//make a new block/finish old block
			
			//finish old block
			block_store_write(fs->bs, blockRequested, data + util, BLOCK_SIZE - blockOffset, blockOffset);
			nbyte -= (BLOCK_SIZE - blockOffset);
			

			//start a new block and place it in inode
			size_t newBlock = block_store_allocate(fs->bs);
			//...god why. Is there no other way to know where it goes?
			if(whichBlock < DIRECT_TOTAL - 1)
			{
				inode.data_ptrs[whichBlock+1] = newBlock;
			}
			else if(whichBlock == DIRECT_TOTAL - 1)
			{
				inode.data_ptrs[6] = block_store_allocate(fs->bs);
				block_store_write(fs->bs, inode.data_ptrs[6], (void*)&newBlock, sizeof(size_t), 0);
			}
			else if(whichBlock < INDIRECT_TOTAL - 2 + DIRECT_TOTAL)
			{
				data_block_t buffer;
				block_store_read(fs->bs, inode.data_ptrs[6],(void*)buffer, BLOCK_SIZE, 0);
				buffer[whichBlock + 1] = newBlock;
				block_store_write(fs->bs, inode.data_ptrs[6], (void*)buffer, BLOCK_SIZE, 0); 	
			}
			else if(whichBlock == INDIRECT_TOTAL - 1 + DIRECT_TOTAL)
			{
				//that feeling when your mind is melting
				inode.data_ptrs[7] = block_store_allocate(fs->bs);
				size_t newIndirectNewBlock = block_store_allocate(fs->bs);
				block_store_write(fs->bs, inode.data_ptrs[7], (void*)&newIndirectNewBlock, sizeof(size_t), 0);
				block_store_write(fs->bs, newIndirectNewBlock, (void*)&newBlock, sizeof(size_t), 0);
			}
			else if(whichBlock < DBL_INDIRECT_TOTAL - 2 + DIRECT_TOTAL)
			{
				data_block_t buffer;
				block_store_read(fs->bs, inode.data_ptrs[7], (void*)buffer, BLOCK_SIZE, 0);
				if(whichBlock % (INODES_PER_BLOCK - 1 + INODE_BLOCK_TOTAL) == 0)
				{
					//...................
					size_t newIndirectBlock = block_store_allocate(fs->bs);
					block_store_write(fs->bs, newIndirectBlock, (void*)&newBlock, sizeof(size_t), 0);
					block_store_write(fs->bs, inode.data_ptrs[7], (void*)&newIndirectBlock, sizeof(size_t), whichBlock / (INODES_PER_BLOCK - 1 + INODE_BLOCK_TOTAL));
				}
				else
				{
					size_t placeHolder = buffer[(whichBlock % (INODES_PER_BLOCK - 1 + INODE_BLOCK_TOTAL)) - 1];
					//TODO: might be whichBlock%x, not whichBlock%x - 1.
					block_store_read(fs->bs, buffer[(whichBlock % (INODES_PER_BLOCK - 1 + INODE_BLOCK_TOTAL))  - 1], (void*)buffer, BLOCK_SIZE, 0);
					buffer[whichBlock-1] = newBlock;
					block_store_write(fs->bs, placeHolder, (void*)buffer, BLOCK_SIZE, 0);
				}
				//10 bucks I got lost somewhere and screwed it up.
			}
			else
			{
				//error
				//technically could also be reaching max file size. 
				return -1;
			}
			//set up everything for the following checks, using the block we just added.
 			blockRequested = newBlock;
			blockOffset = 0;
			block += 1; 
		}
		if(blockOffset + nbyte <= BLOCK_SIZE-1)
		{
			//just write to the block if we're not filling it.
			block_store_write(fs->bs, blockRequested, data + util, nbyte, blockOffset);
			break;
		}
		else
		{
			//so this is here in case we're overriding data, so blocks will already be there to write to.
			//Otherwise, that mess has to be gone through.
			block_store_write(fs->bs, blockRequested, data + util, BLOCK_SIZE - blockOffset, blockOffset);
			util += (BLOCK_SIZE - blockOffset);
			nbyte -= (BLOCK_SIZE - blockOffset);
			offset += (BLOCK_SIZE - blockOffset);
			//find next block
		}
	}
	//only achievable from the break, denoting that all writes were completed.
	return 0;
}

//Reads nbyte bytes from the file with absolute path fname into data, starting at offset.
ssize_t fs_read_file(F15FS_t *const fs, const char *const fname, void *data, size_t nbyte, size_t offset)
{
	//error check
	if(!fs || !fs->bs || !fs->bs->dbm || !fs->bs->fbm || !fname || !fs->bs->data_blocks)
	{
		//eh? better than this?
		return -1;
	}
	
	
	//find the file, make sure it's okay
	//same as fs_write_file. Will put in a fxn if I have time.
	inode_t inode;
	char* child = malloc(sizeof(char)*FNAME_MAX);
	char* parent = malloc(sizeof(char)*FNAME_MAX);
	uint8_t check = extractNames(fname, parent, child);
	if(check < 0)
	{
		return check;
	}
		
	if(!doesExist(fs, fname, DIRECTORY, 0))
	{
		return -1;
	}

	uint8_t i;
	for(i = 0; i < INODE_TOTAL; i++)
	{	
		block_store_read(fs->bs, INODE_TO_BLOCK(i), (void*)&inode, sizeof(inode_t), INODE_INNER_OFFSET(i));
		if(strcmp(inode.fname, parent) == 0)
		{
			break;
		}
	}
	
	//should not get in here, for testing purposes
	if(i == INODE_TOTAL)
	{
		return -1;
	}
	
	uint8_t whichBlock;
	bool atEOF = false;
	void* util = data;
	while(1)
	{
		//find the file block and offset, and use it to find the actual block and offset on the fs
		size_t block = POSITION_TO_BLOCK_INDEX(offset);
		size_t blockOffset = POSITION_TO_INNER_OFFSET(offset);
		block_ptr_t blockRequested = findBlockInFS(fs, block, &whichBlock);
		
		if(inode.mdata.size < nbyte + offset)
		{
			//We're at EOF, sooooooo....
			atEOF = true;
		}
		if(atEOF)
		{
			//read everything we can, I guess.
			block_store_read(fs->bs, blockRequested, util, inode.mdata.size - (nbyte+offset), blockOffset);
			//technically an error, I imagine.
			return -1;
		}
		if(blockOffset + nbyte <= BLOCK_SIZE-1)
		{
			//if everything is in one block, just read the block.
			block_store_read(fs->bs, blockRequested, util, nbyte, blockOffset);
			break;
		}
		else
		{
			//so data bleeds to the next block, read everything we can, and wait to come back around with the next block.
			block_store_read(fs->bs, blockRequested, util, BLOCK_SIZE - blockOffset, blockOffset);
			util += (BLOCK_SIZE - blockOffset);
			nbyte -= (BLOCK_SIZE - blockOffset);
			offset += (BLOCK_SIZE - blockOffset);
			//find next block
		}
	}
	//gets here from the break in the loop, indicating reads have been done. 
	return 0;
}

//creates a file of type ftype, at the absolute path fname.
int fs_create_file(F15FS_t *const fs, const char *const fname, const ftype_t ftype)
{
	//error check
	if(!fs || !fs->bs || !fs->bs->dbm || !fs->bs->fbm || !fs->bs->data_blocks)
	{
		//eh? better than this?
		return -1;
	}
	if(ftype < 0 || ftype > 2)
	{
		return -2;
	} 
	if(!fname || !(fname && fname[0]))
	{
		return -3;
	}
	//just the first one I find I guess?
	inode_t inode;
	inode_t parentInode;

	char* child = malloc(sizeof(char)*FNAME_MAX);
	char* parent = malloc(sizeof(char)*FNAME_MAX);
	int8_t check = extractNames(fname, parent, child);
	if(check < 0)
	{
		return check;
	}
	
	if(!doesExist(fs, fname, ftype, 1))
	{
		return -1;
	}

	uint8_t i;
	//idk, in case I didn't find a parent
	inode_ptr_t idxParent = -1;
	//oh, well -1 in a uint8_t is 255. Hmm.
	bool foundParent = false;
	for(i = 0; i < INODE_TOTAL; i++)
	{	
		block_store_read(fs->bs, INODE_TO_BLOCK(i), (void*)&inode, sizeof(inode_t), INODE_INNER_OFFSET(i));
						
		if(strcmp(inode.fname, parent) == 0)
		{
			parentInode = inode;
			idxParent = i;
			foundParent = true;
		}
		if(!inode.mdata.inUse)
		{
			break;
		}
	}
	if(i == INODE_TOTAL || !foundParent || parentInode.mdata.type == REGULAR)
	{
		return -1;
	}
	
	inode.mdata.size = 0;
	inode.mdata.parent = idxParent;
	inode.mdata.type = ftype;
	inode.mdata.inUse = true;
	strcpy(inode.fname, child);
	inode.data_ptrs[0] = block_store_allocate(fs->bs);
	printf("creating file \"%s\" with parent node %d and type %d\n", inode.fname,
		inode.mdata.parent, inode.mdata.type);
	fs_write_file(fs, fname, (void*)&i, sizeof(uint8_t), parentInode.mdata.size);
	block_store_write(fs->bs, INODE_TO_BLOCK(i), (void*)&inode, sizeof(inode_t), INODE_INNER_OFFSET(i));

	return 0;
}

int fs_open_file(F15FS_t *const fs, const char * const fname)
{
	//error check
	if(!fs || !fs->bs || !fs->bs->dbm || !fs->bs->fbm || !fname || !fs->bs->data_blocks)
	{
		//eh? better than this?
		return -1;
	}

	//starting to rush, so for now it won't be in a fxn.	
	inode_t inode;
	char* child = malloc(sizeof(char)*FNAME_MAX);
	char* parent = malloc(sizeof(char)*FNAME_MAX);
	uint8_t check = extractNames(fname, parent, child);
	if(check < 0)
	{
		return check;
	}
		
	if(doesExist(fs, fname, DIRECTORY, 0))
	{
		return -1;
	}

	uint8_t i;
	for(i = 0; i < INODE_TOTAL; i++)
	{	
		block_store_read(fs->bs, INODE_TO_BLOCK(i), (void*)&inode, sizeof(inode_t), INODE_INNER_OFFSET(i));
		if(strcmp(inode.fname, parent) == 0)
		{
			break;
		}
	}
	
	//should not get in here, for testing purposes
	if(i == INODE_TOTAL)
	{
		return -1;
	}
	

	//should not be able to modify directory. 
	if(inode.mdata.type == DIRECTORY)
	{
		//but can you read from it? Will assume not.
		return -1;
	}

	//add the FD to the list of open FDs
	
	//but first, find an unused spot

	//this was the index of the inode, hold onto it.
	uint8_t inodeIdx = i;

	for(i = 0; i < INODE_TOTAL; i++)
	{
		if(!openFiles.table[i].isValid)
		{
			//found an open one. Set it up.
			openFiles.table[i].isValid = true;
			openFiles.table[i].inode_idx = inodeIdx;
			return i;
		}
	}
	//no open spots in the open file table.
	return -1;
}


/*
Takes the file block number and finds the actual block number that contains data to be 
read/written to. It also takes note of that block number and stores it for later use.

I'm pretty sure that whichBlock ends up being a duplicate that's not really needed, but it made sense
to me when I made it, so I'll leave it.

Parameters:
fs: a mounted file system
block: the index of the block in the inode that is wanted
whichBlock: a reference to hold the block number. Only used in fs_write_file
return: an index to a block in the file system that has the next desired block of data, or -1 on error.

*/
block_ptr_t findBlockInFS(F15FS_t* const fs, size_t block, uint8_t* whichBlock)
{
		inode_t inode;
		if(block < DIRECT_TOTAL)
		{
			*whichBlock = block;
			return inode.data_ptrs[block];
		}
		else if(block < INDIRECT_TOTAL)
		{
			*whichBlock = block - DIRECT_TOTAL;
			//read indirect
			data_block_t buffer;
			block_store_read(fs->bs, inode.data_ptrs[DIRECT_TOTAL], buffer, BLOCK_SIZE, 0);
			//then get the block number we want
			block_ptr_t tmp;
			memcpy(&tmp, &buffer[(block - DIRECT_TOTAL)*sizeof(uint32_t)], sizeof(uint32_t));
			return tmp;
		}
		else if(block < DBL_INDIRECT_TOTAL)
		{
			*whichBlock = block - INDIRECT_TOTAL;
			//read double indirect
			data_block_t buffer;
			block_store_read(fs->bs, inode.data_ptrs[DIRECT_TOTAL + 1], buffer, BLOCK_SIZE, 0);
			//get the block with the indirect pointers
			block_ptr_t levelPtr;
			memcpy(&levelPtr, &buffer[((DBL_INDIRECT_TOTAL - INDIRECT_TOTAL) % block)*sizeof(uint32_t)], sizeof(uint32_t));
			block_store_read(fs->bs, levelPtr, buffer, BLOCK_SIZE, 0);
			//then get the block number we want
			block_ptr_t tmp;
			memcpy(&tmp, &buffer[(block - INDIRECT_TOTAL)*sizeof(uint32_t)], sizeof(uint32_t));
			return tmp;
		}
		else
		{
			//out of bounds...I don't think it should get here...?
			return -1;
		}
}

/*
Takes in the full path name, and extracts the filename of the file in scope, as well as the name
of its parent. Also determines if the actual pathnames are invalid.

Parameters:
tmpFname: the absolute path of the file in question
parent: a reference to where the parent directory name will be housed
child: a reference to where the absolute file name will be housed
return: 0 if successfully extracted both the parent and child, or < 0 if error. 
*/
int8_t extractNames(const char* tmpFname, char* parent, char* child)
{
	//cause const char* things. 
	char* fname = malloc(sizeof(char)*FNAME_MAX);
	//this is if the absolute path is too long, which is what I interpreted FNAME_MAX as
	if(strlen(tmpFname) > FNAME_MAX)
	{
		strcpy(fname, "ERROR\n");
		return -1;
	}
	strcpy(fname, tmpFname);
	char* token = strtok(fname, "/");
	//this is if the file path is just "/", which I assume is not allowed, cause root is root
	if(!token || *(token + 1) == '\0')
	{
		strcpy(fname, "ERROR\n");
		return -1;
	}
	//set up initial strings, in case it's just "parent/child"
	strcpy(parent, token);
	strcpy(child, token);
	bool isRoot = true;
	while(1)
	{
		//cycle through, replacing parent and child as new ones are found
		token = strtok(NULL, "/");
		if(!token)
		{
			//eventually this ends, so it always returns 0 at this point.
			if(isRoot) strcpy(parent, "/");
			return 0;
		}
		else
		{
			//if FNAME_MAX was actually for absolute path, then it would be checked
			//here, have a return value accordingly.
			if(isRoot) isRoot = false;
			else strcpy(parent, token);
		}
		strcpy(child, token);
	}
}

int fs_get_dir(const F15FS_t *const fs, const char *const fname, dir_rec_t *const records)
{
	inode_t inode;	
	//so this has to be greater than a uint8_t...interesting...	
	uint16_t i;
	for(i = 0; i < INODE_TOTAL; i++)
	{	
		block_store_read(fs->bs, INODE_TO_BLOCK(i), (void*)&inode, sizeof(inode_t), INODE_INNER_OFFSET(i));
		if(strcmp(inode.fname, fname) == 0)
		{
			//this should trigger at some point, to indicate that we found
			//the inode with the directory.
			break;
		}
	}
	if(i == INODE_TOTAL || inode.mdata.type == REGULAR)
	{
		return -1;
	}

	uint8_t idx;
	inode_t tmp;
	unsigned dirSize = 0;
	uint8_t realIdx = 0;
	for(i = 0; i < DIR_REC_MAX; i++)
	{
		//first, read the block number that has the inode in the directory
		block_store_read(fs->bs, inode.data_ptrs[0], (void*)&idx, sizeof(uint8_t), sizeof(uint8_t)*i);
		//then read the inode itself to extract the name and type
		block_store_read(fs->bs, INODE_TO_BLOCK(idx), (void*)&tmp, sizeof(inode_t), INODE_INNER_OFFSET(idx));
		if(!tmp.mdata.inUse) 
		{
			//I believe nothing special needs to be done...?
		}
		else
		{
			//realIdx so that there are no gaps in the array. If i was used, it might
			//find blank inodes. Doubt it, but just in case. 
			dirSize++;
			records->contents[realIdx].ftype = tmp.mdata.type;
			strcpy(records->contents[realIdx].fname, tmp.fname);
			realIdx++;
			//wait, I could just use dirSize...oh well might be useful to have 2 diff
			//vars. 
		}
	}
	//This is how many we actually found.
	records->total = dirSize;

	return 0;
}

bool doesExist(const F15FS_t* const fs, const char* TMPfname, ftype_t ftype, uint8_t toggle)
{
	//copy the entire path into a new string, because const char* seems to not like me.
	char* fname = malloc(sizeof(char)*FNAME_MAX);
	strcpy(fname, TMPfname);
	char* token = strtok(fname, "/");
	dir_rec_t records; 
	//set up initial parent and child, in case we are looking under the root directory.
	char* child = malloc(sizeof(char)*FNAME_MAX);
	char* parent = malloc(sizeof(char)*FNAME_MAX);
	strcpy(parent, token);
	strcpy(child, fname+1);
	while(1)
	{
		//see if the directory exists. See notes in the fxn
		if(fs_get_dir(fs, parent, &records) == -1)
		{
			if(toggle == 1) return true;
			if(toggle == 0) return false;
		}
		//the parent is the directory, so we see if we are doing something under
		//that directory. 
		token = strtok(NULL, "/");
		if(token)
		{
			//set up the child to see if the child is in that directory.
			strcpy(child, token);
		}
		size_t i;
		for(i = 0; i < records.total; i++)
		{
			if(strcmp(records.contents[i].fname, child) == 0)
			{
				//attempts to find the child in the directory records. 
				//If found, break out.
				break;
			}
		}
		if(i != records.total && (records.contents[i].ftype == ftype))
		{
			//toggle = 1 -> this fxn was called from fs_create_file, and ftype doesn't matter. 
			//fs_create_file expects false, otherwise there's already a file with 
			//that name there, whether it's a directory or reg file.
			
			//toggle = 0 -> this fxn was called from fs_write_file, and ftype is dir.
			//fs_write_file expects true, otherwise either A) it wasn't found, or 
			//B) we are writing to a directory, which is assumed to be not allowed.
 
			//terrible way to control what this fxn does, oh well.
			if(toggle == 1) return true;
			if(toggle == 0) return false;
		}	
		if(!token)
		{
			//normal return for fs_write_file
			if(toggle == 1) return true;
			if(toggle == 0) return false;

			return false;
		}
		//here we know we aren't done parsing, so there is another directory to explore.
		strcpy(parent, token);
	}
}



