#include "../include/block_store.h"
#include <stdio.h>
// Overriding these will probably break it since I'm not testing it that much
// it probably won't go crazy so long as the sizes are reasonable and powers of two
// So just don't touch it unless you want to debug it
#define BLOCK_COUNT 65536
#define BLOCK_SIZE 1024
#define FBM_SIZE ((BLOCK_COUNT >> 3) / BLOCK_SIZE)
#if (((FBM_SIZE * BLOCK_SIZE) << 3) != BLOCK_COUNT)
    #error "BLOCK MATH DIDN'T CHECK OUT"
#endif

// Handy macro, does what it says on the tin
#define BLOCKID_VALID(id) ((id > (FBM_SIZE - 1)) && (id < BLOCK_COUNT))

//There is no way it's that simple...
size_t utility_read_file(const int fd, uint8_t *buffer, const size_t count);

//This also can't be that simple...
size_t utility_write_file(const int fd, const uint8_t *buffer, const size_t count);

// The magical glue that holds it all together
struct block_store {
    bitmap_t *dbm;
    // Why not. It'll track if we have unsaved changes.
    // It'll be handly in V2
    bitmap_t *fbm;
    uint8_t *data_blocks;
    // add an fd for V2 for better disk stuff
};

/*
This function attempts to allocate memory that will represent the block storage device. If, at any point, memory
does not get allocated, the entire structure is destroyed and processes will end. 

Inputs: None
Returns: The address to a fully allocated bs device if successful, otherwise NULL
*/
block_store_t *block_store_create() {
    block_store_t *bs = malloc(sizeof(block_store_t));
    if (bs) {
	//creates a bitmap of (BLOCK_COUNT >> 3) bytes
        bs->fbm = bitmap_create(BLOCK_COUNT);
        if (bs->fbm) {
	    //same thing
            bs->dbm = bitmap_create(BLOCK_COUNT);
            if (bs->dbm) {
                // Eh, calloc, why not (technically a security risk if we don't)
                bs->data_blocks = calloc(BLOCK_SIZE, BLOCK_COUNT - FBM_SIZE);
                if (bs->data_blocks) {
                    for (size_t idx = 0; idx < FBM_SIZE; ++idx) {
			//initializes FBM/DBM
			//(to what though? 0s?)
			bitmap_set(bs->fbm, idx);
                        bitmap_set(bs->dbm, idx);
                    }
		    //For when everything checks out
                    block_store_errno = BS_OK;
                    return bs;
                }
                bitmap_destroy(bs->dbm);
            }
            bitmap_destroy(bs->fbm);
        }
        free(bs);
    }
    block_store_errno = BS_MEMORY;
    //Error: Memory allocation error
    return NULL;
}

/*
Destroys the bs device, after the program has successfully been run. 
For eliminating pieces of the bs device, see bitmap_destroy

Input: A bs device 
Return: None

*/
void block_store_destroy(block_store_t *const bs) {
    if (bs) {
	//Destroy the pieces first
	bitmap_destroy(bs->fbm);
        bs->fbm = NULL;
        bitmap_destroy(bs->dbm);
        bs->dbm = NULL;
	//Then destroy the data set
        free(bs->data_blocks);
        bs->data_blocks = NULL;
	//Then the bs device itself
        free(bs);
        block_store_errno = BS_OK;
        return;
    }
    //Error: was a bad bs device	
    block_store_errno = BS_PARAM;
}

/*
Initializes each block in the FBM, I believe. So somewhere, this is done 8 times.

Input: the bs device
Return: The size of the block that was just allocated, or 0 if an error occurred.
*/
size_t block_store_allocate(block_store_t *const bs) {
    if (bs && bs->fbm) {
        //(I don't know)
	//Update: I still don't know what ffz/ffs does
	size_t free_block = bitmap_ffz(bs->fbm);
        if (free_block != SIZE_MAX) {
            bitmap_set(bs->fbm, free_block);
            // not going to mark dbm because there's no change (yet)
            return free_block;
        }
	//Error: the bs device is full; cannot support new blocks
        block_store_errno = BS_FULL;
        return 0;
    }
    //Error: bad bs device or bad FBM in the bs device
    block_store_errno = BS_PARAM;
    return 0;
}

/*
    // V2
    size_t block_store_request(block_store_t *const bs, size_t block_id) {
    if (bs && bs->fbm && BLOCKID_VALID(block_id)) {
        if (!bitmap_test(bs->fbm, block_id)) {
            bitmap_set(bs->fbm, block_id);
            block_store_errno = BS_OK;
            return block_id;
        } else {
            block_store_errno = BS_IN_USE;
            return 0;
        }
    }
    block_store_errno = BS_PARAM;
    return 0;
    }
*/

/*
Takes the ID of a piece of the FBM and frees it. I believe it does this by setting the storage block to all 0s.

Returns the ID of the block if successful, 0 otherwise. 
*/
size_t block_store_release(block_store_t *const bs, const size_t block_id) {
    if (bs && bs->fbm && BLOCKID_VALID(block_id)) {
        // we could clear the dirty bit, since the info is no longer in use but...
        // We'll keep it. Could be useful. Doesn't really hurt anything.
        // Keeps it more true to a standard block device.
        // You could also use this function to format the specified block for security reasons
        bitmap_reset(bs->fbm, block_id);
	//Error code: OK
        block_store_errno = BS_OK;
        return block_id;
    }
    //Error code: bad bs device, FBM, or block in the FBM (bad parameter)
    block_store_errno = BS_PARAM;
    return 0;
}

/*
Takes a buffer of bytes and extracts bytes into it from the FBM at a specific location.

Takes a bs device, the ID of the block being written over, the number of bytes being written to it,
the offset where the starting location of the write is, and the buffer with data being written to the block. 

Returns the number of bytes written to the block if successful, 0 if unsuccessful. 
*/
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer, const size_t nbytes, const size_t offset) {
    //Checks for any NULL parameter
    if (bs && bs->fbm && bs->data_blocks && BLOCKID_VALID(block_id)
            && buffer && nbytes && (nbytes + offset <= BLOCK_SIZE)) {
        // Not going to forbid reading of not-in-use blocks (but we'll log it via errno)
        size_t total_offset = offset + (BLOCK_SIZE * (block_id - FBM_SIZE));
	//This is the actual write. 
        memcpy(buffer, (void *)(bs->data_blocks + total_offset), nbytes);
	//This does the check to see if the block can be written to, I believe. The write still happens, but it is logged. 
	//the error becomes BS_OK if the block is okay to be written to, FBM_REQUEST_MISMATCH otherwise
        block_store_errno = bitmap_test(bs->fbm, block_id) ? BS_OK : BS_FBM_REQUEST_MISMATCH;
        return nbytes;
    }
    // technically we return BS_PARAM even if the internal structure of the BS object is busted
    // Which, in reality, would be more of a BS_INTERNAL or a BS_FATAL... but it'll add another branch to everything
    // And technically the bs is a parameter...
    block_store_errno = BS_PARAM;
    return 0;
}

/*
Writes a buffer of bytes to a block of memory in the bs device.

Takes the bs_device, the ID of the block to be written to, the offset on where to start writing, the number of bytes
needed to be written, and the buffer of bytes to be written. Returns the number of bytes written if successful, 0 if error
*/
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer, const size_t nbytes, const size_t offset) {
    //So I guess all parameters still have to be legitimate just like bs_read
    if(bs && bs->fbm && bs->data_blocks && BLOCKID_VALID(block_id) && bs->dbm
	&& buffer && nbytes && (nbytes + offset <= BLOCK_SIZE))
    {
	//So I definitely wayyyyyy overthought this...
	bitmap_set(bs->dbm, block_id);

	//So the offset still has to be the same. 
    	size_t total_offset = offset + (BLOCK_SIZE*(block_id - FBM_SIZE));
	//I imagine that it's the literal opposite of bs_read.
	memcpy((void*)(bs->data_blocks + total_offset), buffer, nbytes);
	//still gotta check for a valid memory block. Logged the same if a bad block, but still written to.
	block_store_errno = bitmap_test(bs->fbm, block_id) ? BS_OK : BS_FBM_REQUEST_MISMATCH;

    	return nbytes;
    }
    block_store_errno = BS_PARAM;
    return 0;
}

block_store_t *block_store_import(const char *const filename) {
	//There's no way I covered all the error checks. Something should break this I imagine. 	
	if(filename)
	{
		block_store_t *bs;
		if((bs = block_store_create()))
		{
			//Not sure if this was the right approach to the DBM, but made sense to me.
			bitmap_format(bs->dbm, 0x0);
			const int fd = open(filename, O_RDONLY, S_IRUSR | S_IRGRP);
			//Still not convinced that this is the way to check fd, but okay. 
			if(fd > 0)
			{
				//Passes the test, so I imagine I guessed right when I 
				//said the first thing in the file is the FBM data. 
				if(utility_read_file(fd,(uint8_t*)bitmap_export(bs->fbm), FBM_SIZE*BLOCK_SIZE) == (FBM_SIZE*BLOCK_SIZE))
				{
					if(utility_read_file(fd,bs->data_blocks, BLOCK_SIZE*(BLOCK_COUNT - FBM_SIZE)) == (BLOCK_SIZE*(BLOCK_COUNT - FBM_SIZE)))
					{
						block_store_errno = BS_OK;
						close(fd);
						return bs;
					}
				}
			}
			block_store_destroy(bs);	
			block_store_errno = BS_FILE_ACCESS;
			//Surprised this doesn't bomb when a file is invalid. 
			close(fd);
			return NULL;			
		}
		block_store_errno = BS_MEMORY;
		return NULL;
	}
	block_store_errno = BS_PARAM;
	return NULL;

    block_store_errno = BS_FATAL;
    return NULL;
}

/*
Takes a filename and a bs device, and writes the entire bs device to the file

Returns number of bytes written if successful, 0 if error. 
*/
size_t block_store_export(const block_store_t *const bs, const char *const filename) {
    // Thankfully, this is less of a mess than import...
    // we're going to ignore dbm, we'll treat export like it's making a new copy of the drive
    if (filename && bs && bs->fbm && bs->data_blocks) {
	//Does the system call to open the file. Clears the file, creates if no exist, and makes write only. 
	//User is given r/w permissions, and the user's system group is given read permissions.
        const int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);
        if (fd != -1) {
	    //does the write for the FBM
            if (utility_write_file(fd, bitmap_export(bs->fbm), FBM_SIZE * BLOCK_SIZE) == (FBM_SIZE * BLOCK_SIZE)) {
                //does the write for the data in the bs device
		if (utility_write_file(fd, bs->data_blocks, BLOCK_SIZE * (BLOCK_COUNT - FBM_SIZE)) == (BLOCK_SIZE * (BLOCK_COUNT - FBM_SIZE))) {
                    block_store_errno = BS_OK;
                    close(fd);
                    return BLOCK_SIZE * BLOCK_COUNT;
                }
            }
            block_store_errno = BS_FILE_IO;
            close(fd);
            return 0;
        }
        block_store_errno = BS_FILE_ACCESS;
        return 0;
    }
    block_store_errno = BS_PARAM;
    return 0;
}

/*
Defines the various custom errors that are returned by above functions. The error codes are kept in an enum in the header file.

Takes in an error code and returns a description for that error code in string format. 
*/
const char *block_store_strerror(block_store_status bs_err) {
    switch (bs_err) {
        case BS_OK:
            return "Ok";
        case BS_PARAM:
            return "Parameter error";
        case BS_INTERNAL:
            return "Generic internal error";
        case BS_FULL:
            return "Device full";
        case BS_IN_USE:
            return "Block in use";
        case BS_NOT_IN_USE:
            return "Block not in use";
        case BS_FILE_ACCESS:
            return "Could not access file";
        case BS_FATAL:
            return "Generic fatal error";
        case BS_FILE_IO:
            return "Error during disk I/O";
        case BS_MEMORY:
            return "Memory allocation failure";
        case BS_WARN:
            return "Generic warning";
        case BS_FBM_REQUEST_MISMATCH:
            return "Read/write request to a block not marked in use";
        default:
	    //How did I get here?
            return "???";
    }
}


// V2 idea:
//  add an fd field to the struct (and have export(change name?) fill it out if it doesn't exist)
//  and use that for sync. When we sync, we only write the dirtied blocks
//  and once the FULL sync is complete, we format the dbm
//  So, if at any time, the connection to the file dies, we have the dbm saved so we can try again
//   but it's probably totally broken if the sync failed for whatever reason
//   I guess a new export will fix that?


/*
Takes a file descriptor and reads count bytes from it to the buffer. 

Takes the open file, the buffer, and the number of bytes as input. Returns the number of bytes written on success, 0 on failure
*/
size_t utility_read_file(const int fd, uint8_t *buffer, const size_t count) {
	//Everything must be non-NULL/non-zero
  	if(buffer && fd && count)
  	{	
		//This can't be it...
		//Update: I guess it is...
		if(read(fd, (void*)buffer, count) == count && errno != EINTR)
		{
			return count; 
		}
	}

  block_store_errno = BS_PARAM;
  return 0;
}


/*
Takes a file descriptor and writes count bytes from the buffer to it. 

Takes the open file, the buffer, and the number of bytes as input. Returns the number of bytes written on success, 0 on failure
*/
size_t utility_write_file(const int fd, const uint8_t *buffer, const size_t count) {
	//Pretty sure fd > 0 and fd do the same thing, but I won't touch it JIC
	if(buffer && fd > 0 && count)
	{
		//I'm pretty sure this isn't it, but I'll wait to test. 
		//Update: appears to work, but I'm skeptical.
		if(write(fd, (void*)buffer, count) == count && errno != EINTR)
		{
			return count;	
		}
	}

	block_store_errno = BS_PARAM;
    return 0;
}
