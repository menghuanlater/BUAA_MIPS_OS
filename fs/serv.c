/*
 * File system server main loop -
 * serves IPC requests from other environments.
 */

#include "fs.h"
#include "fd.h"
#include "lib.h"
#include <mmu.h>

#define debug 0

struct Open {
	struct File *o_file;	// mapped descriptor for open file
	u_int o_fileid;		// file id
	int o_mode;		// open mode
	struct Filefd *o_ff;	// va of filefd page
};

// Max number of open files in the file system at once
#define MAXOPEN	1024
#define FILEVA 0x60000000

// initialize to force into data section
struct Open opentab[MAXOPEN] = { { 0, 0, 1 } };

// Virtual address at which to receive page mappings containing client requests.
#define REQVA	0x0ffff000

void
serve_init(void)
{
	int i;
	u_int va;

	va = FILEVA;
	for (i=0; i<MAXOPEN; i++) {
		opentab[i].o_fileid = i;
		opentab[i].o_ff = (struct Filefd*)va;
		va += BY2PG;
	}
}

// Allocate an open file.
int
open_alloc(struct Open **o)
{
	int i, r;

	// Find an available open-file table entry
	for (i = 0; i < MAXOPEN; i++) {
		switch (pageref(opentab[i].o_ff)) {
		case 0:
			//writef("^^^^^^^^^^^^^^^^ (u_int)opentab[i].o_ff: %x\n",(u_int)opentab[i].o_ff);
			if ((r = syscall_mem_alloc(0, (u_int)opentab[i].o_ff, PTE_V|PTE_R|PTE_LIBRARY)) < 0)
				return r;
			
		case 1:
			opentab[i].o_fileid += MAXOPEN;
			*o = &opentab[i];
			user_bzero((void*)opentab[i].o_ff, BY2PG);
			return (*o)->o_fileid;
		}
	}
	return -E_MAX_OPEN;
}

// Look up an open file for envid.
int
open_lookup(u_int envid, u_int fileid, struct Open **po)
{
	struct Open *o;

	o = &opentab[fileid%MAXOPEN];
	if (pageref(o->o_ff) == 1 || o->o_fileid != fileid)
		return -E_INVAL;
	*po = o;
	return 0;
}

// Serve requests, sending responses back to envid.
// To send a result back, ipc_send(envid, r, 0, 0).
// To include a page, ipc_send(envid, r, srcva, perm).

void
serve_open(u_int envid, struct Fsreq_open *rq)
{
	writef("serve_open %08x %x 0x%x\n", envid, (int)rq->req_path, rq->req_omode);

	u_char path[MAXPATHLEN];
	struct File *f;
	struct Filefd *ff;
	int fileid;
	int r;
	struct Open *o;

	// Copy in the path, making sure it's null-terminated
	user_bcopy(rq->req_path, path, MAXPATHLEN);
	path[MAXPATHLEN-1] = 0;
//writef("serve_open:enter open %s\n",rq->req_path);
	// Find a file id.
	
	if ((r = open_alloc(&o)) < 0) {
		writef("open_alloc failed: %d", r);
		goto out;
	}
	fileid = r;
	
//writef("serve_open:ending find a file id	o = %x\n",o);
	// Open the file.
	if ((r = file_open(path, &f)) < 0) {
		writef("file_open failed: %e", r);
		goto out;
	}
//writef("serve_open:ending open the file\n");
	// Save the file pointer.
	o->o_file = f;

	// Fill out the Filefd structure
	ff = (struct Filefd*)o->o_ff;
	ff->f_file = *f;
	ff->f_fileid = o->o_fileid;
	o->o_mode = rq->req_omode;
	ff->f_fd.fd_omode = o->o_mode;
	ff->f_fd.fd_dev_id = devfile.dev_id;
//writef("serve_open:will to ipc send\n");
	if (debug) writef("sending success, page %08x\n", (u_int)o->o_ff);
	ipc_send(envid, 0, (u_int)o->o_ff, PTE_V|PTE_R|PTE_LIBRARY);
//writef("serve_open:end of open %s\n",rq->req_path);
	return;
out:user_panic("*********************path:%s",path);
	ipc_send(envid, r, 0, 0);
}

void
serve_map(u_int envid, struct Fsreq_map *rq)
{
	if (debug) writef("serve_map %08x %08x %08x\n", envid, rq->req_fileid, rq->req_offset);
	
	struct Open *pOpen;
	u_int filebno;
	void *blk;
	int r;
	// Your code here
	if((r = open_lookup(envid, rq->req_fileid, &pOpen))<0)
	{
		ipc_send(envid,r,0,0);
		return;
	}
	
	filebno = rq->req_offset/BY2BLK;
	if((r = file_get_block(pOpen->o_file, filebno, &blk))<0)
	{
		ipc_send(envid,r,0,0);
		return;
	}

	ipc_send(envid, 0, blk, PTE_V|PTE_R|PTE_LIBRARY);
	return;
//	user_panic("serve_map not implemented");
}

void
serve_set_size(u_int envid, struct Fsreq_set_size *rq)
{
	if (debug) writef("serve_set_size %08x %08x %08x\n", envid, rq->req_fileid, rq->req_size);

        struct Open *pOpen;
        int r;
        // Your code here
        if((r = open_lookup(envid, rq->req_fileid, &pOpen))<0)
        {
                ipc_send(envid,r,0,0);
                return;
        }
	
	if((r = file_set_size(pOpen->o_file, rq->req_size))<0)
	{
		ipc_send(envid,r,0,0);
		return;
	}

	ipc_send(envid, 0, 0, 0);//PTE_V);
	return;
//	user_panic("serve_set_size not implemented");
}

void
serve_close(u_int envid, struct Fsreq_close *rq)
{
	if (debug) writef("serve_close %08x %08x\n", envid, rq->req_fileid);

        struct Open *pOpen;
        int r;
        if((r = open_lookup(envid, rq->req_fileid, &pOpen))<0)
        {
                ipc_send(envid,r,0,0);
                return;
        }
//writef("serve_close:pOpen = %x\n",pOpen);	
	file_close(pOpen->o_file);
	ipc_send(envid, 0, 0, 0);//PTE_V);
	
//	syscall_mem_unmap(0, (u_int)pOpen);
	return;		
//	user_panic("serve_close not implemented");
}
		
void
serve_remove(u_int envid, struct Fsreq_remove *rq)
{
	if (debug) writef("serve_map %08x %s\n", envid, rq->req_path);

	// Your code here
        int r;
	u_char path[MAXPATHLEN];

        // Copy in the path, making sure it's null-terminated
        user_bcopy(rq->req_path, path, MAXPATHLEN);
        path[MAXPATHLEN-1] = 0;
	
	if((r = file_remove(path))<0)
        {
                ipc_send(envid,r,0,0);
                return;
        }
	
	ipc_send(envid, 0, 0, 0);//PTE_V);
//	user_panic("serve_remove not implemented");
}

void
serve_dirty(u_int envid, struct Fsreq_dirty *rq)
{
	if (debug) writef("serve_dirty %08x %08x %08x\n", envid, rq->req_fileid, rq->req_offset);

	// Your code here
        struct Open *pOpen;
        int r;
        if((r = open_lookup(envid, rq->req_fileid, &pOpen))<0)
        {
                ipc_send(envid,r,0,0);
                return;
        }

	if((r = file_dirty(pOpen->o_file, rq->req_offset))<0)
	{
		ipc_send(envid,r,0,0);
                return;
	}

	ipc_send(envid, 0, 0, 0);
	return;
//	user_panic("serve_dirty not implemented");
}

void
serve_sync(u_int envid)
{
	fs_sync();
	ipc_send(envid, 0, 0, 0);
}

void
serve(void)
{
	u_int req, whom, perm;
	for(;;) {
		perm = 0;

		req = ipc_recv(&whom, REQVA, &perm);
		

		// All requests must contain an argument page
		if (!(perm & PTE_V)) {
			writef("Invalid request from %08x: no argument page\n",
				whom);
			continue; // just leave it hanging...
		}

		switch (req) {
		case FSREQ_OPEN:
			serve_open(whom, (struct Fsreq_open*)REQVA);
			break;
		case FSREQ_MAP:
			serve_map(whom, (struct Fsreq_map*)REQVA);
			break;
		case FSREQ_SET_SIZE:
			serve_set_size(whom, (struct Fsreq_set_size*)REQVA);
			break;
		case FSREQ_CLOSE:
			serve_close(whom, (struct Fsreq_close*)REQVA);
			break;
		case FSREQ_DIRTY:
			serve_dirty(whom, (struct Fsreq_dirty*)REQVA);
			break;
		case FSREQ_REMOVE:
			serve_remove(whom, (struct Fsreq_remove*)REQVA);
			break;
		case FSREQ_SYNC:
			serve_sync(whom);
			break;
		default:
			writef("Invalid request code %d from %08x\n", whom, req);
			break;
		}
		syscall_mem_unmap(0, REQVA);
	}
}

void
umain(void)
{
	user_assert(sizeof(struct File)==256);
	writef("FS is running\n");

	// Check that we are able to do I/O
	//outw(0x8A00, 0x8A00);
	writef("FS can do I/O\n");

	serve_init();
	fs_init();
	fs_test();
	
	serve();
}

