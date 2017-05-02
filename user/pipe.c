#include "lib.h"
#include <mmu.h>
#include <env.h>
#define debug 0

static int pipeclose(struct Fd*);
static int piperead(struct Fd *fd, void *buf, u_int n, u_int offset);
static int pipestat(struct Fd*, struct Stat*);
static int pipewrite(struct Fd *fd, const void *buf, u_int n, u_int offset);

struct Dev devpipe =
{
.dev_id=	'p',
.dev_name=	"pipe",
.dev_read=	piperead,
.dev_write=	pipewrite,
.dev_close=	pipeclose,
.dev_stat=	pipestat,
};

#define BY2PIPE 32		// small to provoke races

struct Pipe {
	u_int p_rpos;		// read position
	u_int p_wpos;		// write position
	u_char p_buf[BY2PIPE];	// data buffer
};

int
pipe(int pfd[2])
{
	int r, va;
	struct Fd *fd0, *fd1;
	// allocate the file descriptor table entries
	if ((r = fd_alloc(&fd0)) < 0
	||  (r = syscall_mem_alloc(0, (u_int)fd0, PTE_V|PTE_R|PTE_LIBRARY)) < 0)
		goto err;

	if ((r = fd_alloc(&fd1)) < 0
	||  (r = syscall_mem_alloc(0, (u_int)fd1, PTE_V|PTE_R|PTE_LIBRARY)) < 0)
		goto err1;
	//writef("%x :%x\n",fd0,fd1);
	// allocate the pipe structure as first data page in both
	va = fd2data(fd0);
	if ((r = syscall_mem_alloc(0, va, PTE_V|PTE_R|PTE_LIBRARY)) < 0)
		goto err2;
	if ((r = syscall_mem_map(0, va, 0, fd2data(fd1), PTE_V|PTE_R|PTE_LIBRARY)) < 0)
		goto err3;

	// set up fd structures
	fd0->fd_dev_id = devpipe.dev_id;
	fd0->fd_omode = O_RDONLY;

	fd1->fd_dev_id = devpipe.dev_id;
	fd1->fd_omode = O_WRONLY;

	writef("[%08x] pipecreate \n", env->env_id, (* vpt)[VPN(va)]);

	pfd[0] = fd2num(fd0);
	pfd[1] = fd2num(fd1);
	return 0;

err3:	syscall_mem_unmap(0, va);
err2:	syscall_mem_unmap(0, (u_int)fd1);
err1:	syscall_mem_unmap(0, (u_int)fd0);
err:	return r;
}

static int
_pipeisclosed(struct Fd *fd, struct Pipe *p)
{
	// Your code here.
	// 
	// Check pageref(fd) and pageref(p),
	// returning 1 if they're the same, 0 otherwise.
	// 
	// The logic here is that pageref(p) is the total
	// number of readers *and* writers, whereas pageref(fd)
	// is the number of file descriptors like fd (readers if fd is
	// a reader, writers if fd is a writer).
	// 
	// If the number of file descriptors like fd is equal
	// to the total number of readers and writers, then
	// everybody left is what fd is.  So the other end of
	// the pipe is closed.
	int pfd,pfp,runs;
	//使用do while循环进行判断进程是否发生切换
	do{
		pfd = pageref(fd);
		runs = env->env_runs;
		pfp = pageref(p);
	}while(runs!=env->env_runs); //如果不等,则表明进程发生了切换
	if(pfd == pfp)
		return 1;
	else
		return 0;

//	panic("_pipeisclosed not implemented");
//	return 0;
}

int
pipeisclosed(int fdnum)
{
	struct Fd *fd;
	struct Pipe *p;
	int r;

	if ((r = fd_lookup(fdnum, &fd)) < 0)
		return r;
	p = (struct Pipe*)fd2data(fd);
	return _pipeisclosed(fd, p);
}

static int
piperead(struct Fd *fd, void *vbuf, u_int n, u_int offset)
{
	// Your code here.  See the lab text for a description of
	// what piperead needs to do.  Write a loop that 
	// transfers one byte at a time.  If you decide you need
	// to yield (because the pipe is empty), only yield if
	// you have not yet copied any bytes.  (If you have copied
	// some bytes, return what you have instead of yielding.)
	// If the pipe is empty and closed and you didn't copy any data out, return 0.
	// Use _pipeisclosed to check whether the pipe is closed.
	int i = 0;
	struct Pipe *p;
	char *rbuf;
	if(n==0) return 0;//读取的字节为0,直接返回
	p = (struct Pipe *)fd2data(fd);
	rbuf = (char *)vbuf;//void*转char*
	//offset 意思是在当前读取的rpos基础上增加offset偏移
	/*如果我们刚执行这函数,什么字节都没读取,我们应该调度其他进程,
	 *但是，需要用循环判断,如果读取了>=1个字节,则就算没读取到n个字节，也要返回
	*/
	writef("address in read:%x\n",&p);
	while(p->p_rpos>=p->p_wpos){
		if(_pipeisclosed(fd,p)){
			writef("no data read,we found the write process exit.\n");
			return 0;
		}else{
			syscall_yield();
		}
	}
	while((i<n) && p->p_rpos<p->p_wpos){
		//user_bcopy(&(p->p_buf[p->p_rpos%BY2PIPE]),rbuf,1);
		rbuf[i] = p->p_buf[p->p_rpos%BY2PIPE];
		p->p_rpos++;
		//rbuf++;
		i++;
	}
	return i;

//	panic("piperead not implemented");
//	return -E_INVAL;
}

static int
pipewrite(struct Fd *fd, const void *vbuf, u_int n, u_int offset)
{
	// Your code here.  See the lab text for a description of what 
	// pipewrite needs to do.  Write a loop that transfers one byte
	// at a time.  Unlike in read, it is not okay to write only some
	// of the data.  If the pipe fills and you've only copied some of
	// the data, wait for the pipe to empty and then keep copying.
	// If the pipe is full and closed, return 0.
	// Use _pipeisclosed to check whether the pipe is closed.
	int i = 0;
	struct Pipe *p = (struct Pipe *)fd2data(fd);
	char *wbuf = (char *)vbuf;
	if(n==0) return 0;
	p->p_wpos += offset;
	writef("address in write:%x\n",&p);
	while((p->p_wpos - p->p_rpos)>=BY2PIPE){
		if(_pipeisclosed(fd,p)){
			writef("no data write,we found the read process is closed.\n");
			return 0;
		}else{
			syscall_yield();
		}
	}
	while(i<n){
		//user_bcopy(wbuf,&(p->p_buf[p->p_wpos%BY2PIPE]),1);
		p->p_buf[p->p_wpos%BY2PIPE] = wbuf[i];
		p->p_wpos++;
		i++;
		//wbuf++;
		//判断缓冲区是否被写满,写满则通知读者去读缓冲区
		while(i<n && (p->p_wpos - p->p_rpos >= BY2PIPE)){
			syscall_yield();
		}
		//writef("in write i:%d\n",i);
	}
	writef("success write length:%d,but the n is:%d,offset is:%d\n",i,n,offset);
	return i;
	//	return n;
//	panic("pipewrite not implemented");
//	return -E_INVAL;
}

static int
pipestat(struct Fd *fd, struct Stat *stat)
{
	struct Pipe *p;
	p = (struct Pipe *)fd2data(fd);
	strcpy(stat->st_name,"<pipe>");
	stat->st_size = p->p_wpos - p->p_rpos;
	stat->st_isdir = 0;
	stat->st_dev = &devpipe;
	return 0;
}

static int
pipeclose(struct Fd *fd)
{
	//syscall_mem_unmap(0,fd);
	syscall_mem_unmap(0, fd2data(fd));
	return 0;
}

