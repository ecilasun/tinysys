#include "rombase.h"
#include "sdcard.h"
#include "usbserial.h"
#include "mini-printf.h"
#include "usbserialhandler.h"
#include "usbhidhandler.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static FATFS Fs;

static char k_tmpstr[64];

void ReportError(const uint32_t _width, const char *_error, uint32_t _cause, uint32_t _value, uint32_t _PC)
{
	USBSerialWrite("\033[0m\n\n\033[31m\033[40m");

	USBSerialWrite("┌");
	for (uint32_t w=0;w<_width-2;++w)
		USBSerialWrite("─");
	USBSerialWrite("┐\n│");

	// Message
	int W = strlen(_error);
	W = (_width-2) - W;
	USBSerialWrite(_error);
	for (int w=0;w<W;++w)
		USBSerialWrite(" ");
	USBSerialWrite("│\n|");

	// Cause
	USBSerialWrite("cause:");
	USBSerialWriteHex(_cause);
	for (uint32_t w=0;w<_width-16;++w)
		USBSerialWrite(" ");

	// Value
	USBSerialWrite("│\n|value:");
	USBSerialWriteHex(_value);
	for (uint32_t w=0;w<_width-16;++w)
		USBSerialWrite(" ");

	// PC
	USBSerialWrite("│\n|PC:");
	USBSerialWriteHex(_PC);
	for (uint32_t w=0;w<_width-13;++w)
		USBSerialWrite(" ");

	USBSerialWrite("│\n└");
	for (uint32_t w=0;w<_width-2;++w)
		USBSerialWrite("─");
	USBSerialWrite("┘\n\033[0m\n");
}

uint32_t MountDrive()
{
	// Delayed mount the volume
	FRESULT mountattempt = f_mount(&Fs, "sd:", 0);
	if (mountattempt == FR_OK)
	{
		FRESULT cdattempt = f_chdrive("sd:");
		if (cdattempt != FR_OK)
			return 0;
	}

	USBSerialWrite("Device sd: mounted\n");
	return 1;
}

void UnmountDrive()
{
	/*FRESULT unmountattempt =*/ f_mount(NULL, "sd:", 1);
	/*if (unmountattempt != FR_OK)
		ReportError(32, "File system error (unmount)", unmountattempt, 0, 0);
	else*/
		USBSerialWrite("Device sd: unmounted\n");
}

void ListFiles(const char *path)
{
	DIR dir;
	FRESULT re = f_opendir(&dir, path);
	static const char blankspace[32] = "                                ";
	if (re == FR_OK)
	{
		FILINFO finf;
		do{
			re = f_readdir(&dir, &finf);
			if (re != FR_OK || finf.fname[0] == 0) // Done scanning dir, or error encountered
				break;

			char *isexe = strstr(finf.fname, ".elf");
			int isdir = finf.fattrib&AM_DIR;
			if (isdir)
				USBSerialWrite("\033[33m"); // Yellow
			if (isexe!=NULL)
				USBSerialWrite("\033[32m"); // Green

			// Make sure we're always aligned to max 32 characters
			int count = 0;
			while(finf.fname[count]!=0) { count++; }
			count = count>32 ? 32 : count;
			USBSerialWriteN(finf.fname, count);
			if (count<32)
				USBSerialWriteN(blankspace, 32-count);

			if (isdir)
				USBSerialWrite(" <dir>");
			else
			{
				USBSerialWrite(" ");

				uint32_t inkbytes = (uint32_t)finf.fsize/1024;
				uint32_t inmbytes = inkbytes/1024;

				if (inmbytes!=0)
				{
					USBSerialWriteDecimal(inmbytes);
					USBSerialWrite("Mb");
				}
				else if (inkbytes!=0)
				{
					USBSerialWriteDecimal(inkbytes);
					USBSerialWrite("Kb");
				}
				else
				{
					USBSerialWriteDecimal((uint32_t)finf.fsize);
					USBSerialWrite("b");
				}
			}
			USBSerialWrite("\033[0m\n");
		} while(1);

		f_closedir(&dir);
	}
	else
		ReportError(32, "File system error (unmount)", re, 0, 0);
}

uint32_t ParseELFHeaderAndLoadSections(FIL *fp, struct SElfFileHeader32 *fheader, uint32_t* jumptarget)
{
	uint32_t heap_start = 0;
	if (fheader->m_Magic != 0x464C457F)
	{
		ReportError(32, "ELF header error", fheader->m_Magic, 0, 0);
		return heap_start;
	}

	*jumptarget = fheader->m_Entry;
	UINT bytesread = 0;

	// Read program headers
	for (uint32_t i=0; i<fheader->m_PHNum; ++i)
	{
		struct SElfProgramHeader32 pheader;
		f_lseek(fp, fheader->m_PHOff + fheader->m_PHEntSize*i);
		f_read(fp, &pheader, sizeof(struct SElfProgramHeader32), &bytesread);

		// Something here
		if (pheader.m_MemSz != 0)
		{
			uint8_t *memaddr = (uint8_t *)pheader.m_PAddr;
			// Check illegal range
			if ((uint32_t)memaddr>=HEAP_END_CONSOLEMEM_START || ((uint32_t)memaddr)+pheader.m_MemSz>=HEAP_END_CONSOLEMEM_START)
			{
				ReportError(32, "ELF section in illegal memory", (uint32_t)memaddr, pheader.m_MemSz, 0);
				return 0;
			}
			else
			{
				// Initialize the memory range at target physical address
				// This can be larger than the loaded size
				memset(memaddr, 0x0, pheader.m_MemSz);
				// Load the binary
				f_lseek(fp, pheader.m_Offset);
				f_read(fp, memaddr, pheader.m_FileSz, &bytesread);
				uint32_t blockEnd = (uint32_t)memaddr + pheader.m_MemSz;
				heap_start = heap_start < blockEnd ? blockEnd : heap_start;
			}
		}
	}

	return E32AlignUp(heap_start, 1024);
}

uint32_t LoadExecutable(const char *filename, const bool reportError)
{
	FIL fp;
	FRESULT fr = f_open(&fp, filename, FA_READ);

	if (fr == FR_OK)
	{
		// Something was there, load and parse it
		struct SElfFileHeader32 fheader;
		UINT readsize;
		f_read(&fp, &fheader, sizeof(fheader), &readsize);
		uint32_t branchaddress;
		uint32_t heap_start = ParseELFHeaderAndLoadSections(&fp, &fheader, &branchaddress);
		f_close(&fp);

		// Success?
		if (heap_start != 0)
		{
			asm volatile(
				".word 0xFC000073;"	// Invalidate & Write Back D$ (CFLUSH.D.L1)
				"fence.i;"			// Invalidate I$
			);

			// Set brk() to end of executable's BSS
			// TODO: MMU should handle address space mapping and we should not have to do this manually
			set_elf_heap(heap_start);

			return branchaddress;
		}
		else
			return 0;
	}
	else
	{
		if (reportError)
			ReportError(32, "Executable not found", 0, 0, 0);
	}

	return 0;
}

#define MAX_HANDLES 32
#define MAXFILENAMELEN 32

// Handle allocation mask, positions 0,1 and 2 are reserved
//0	Standard input	STDIN_FILENO	stdin
//1	Standard output	STDOUT_FILENO	stdout
//2	Standard error	STDERR_FILENO	stderr
static uint32_t s_handleAllocMask = 0x00000007;
static FIL s_filehandles[MAX_HANDLES];
static char s_fileNames[MAX_HANDLES][MAXFILENAMELEN+1] = {
	{"stdin"},
	{"stdout"},
	{"stderr"},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "},
	{"                                "}};

static struct STaskContext g_taskctx;
static UINT tmpresult = 0;

struct STaskContext *CreateTaskContext()
{
	// Initialize task context memory
	TaskInitSystem(&g_taskctx);

	return &g_taskctx;
}

struct STaskContext *GetTaskContext()
{
	return &g_taskctx;
}

void HandleSDCardDetect()
{
	uint32_t cardState = *IO_CARDDETECT;
	/*USBSerialWriteHex(cardState);
	USBSerialWrite("\n");*/

	if (cardState == 0x0)	// Removed
		UnmountDrive();
	else					// Inserted
		MountDrive();
}

uint32_t FindFreeFileHandle(const uint32_t _input)
{
	uint32_t tmp = _input;
	for (uint32_t i=0; i<32; ++i)
	{
		if ((tmp & 0x00000001) == 0)
			return (i+1);
		tmp = tmp >> 1;
	}

	return 0;
}

void AllocateFileHandle(const uint32_t _bitIndex, uint32_t * _input)
{
	uint32_t mask = 1 << (_bitIndex-1);
	*_input = (*_input) | mask;
}

void ReleaseFileHandle(const uint32_t _bitIndex, uint32_t * _input)
{
	uint32_t mask = 1 << (_bitIndex-1);
	*_input = (*_input) & (~mask);
}

uint32_t IsFileHandleAllocated(const uint32_t _bitIndex, const uint32_t  _input)
{
	uint32_t mask = 1 << (_bitIndex-1);
	return (_input & mask) ? 1 : 0;
}

void HandleSoftReset(const uint32_t _PC)
{
	ReportError(34, "Soft reset handler not installed", 0, 0, _PC);
}

//void __attribute__((aligned(16))) __attribute__((interrupt("machine"))) interrupt_service_routine() // Auto-saves registers
void __attribute__((aligned(16))) __attribute__((naked)) interrupt_service_routine() // Manual register save
{
	// Use extra space in CSR file to store a copy of the current register set before we overwrite anything
	// NOTE: Stores MEPC as current PC (which is saved before we get here)
	asm volatile(" \
		csrw 0x8A1, ra; \
		csrw 0x8A2, sp; \
		csrw 0x8A3, gp; \
		csrw 0x8A4, tp; \
		csrw 0x8A5, t0; \
		csrw 0x8A6, t1; \
		csrw 0x8A7, t2; \
		csrw 0x8A8, s0; \
		csrw 0x8A9, s1; \
		csrw 0x8AA, a0; \
		csrw 0x8AB, a1; \
		csrw 0x8AC, a2; \
		csrw 0x8AD, a3; \
		csrw 0x8AE, a4; \
		csrw 0x8AF, a5; \
		csrw 0x8B0, a6; \
		csrw 0x8B1, a7; \
		csrw 0x8B2, s2; \
		csrw 0x8B3, s3; \
		csrw 0x8B4, s4; \
		csrw 0x8B5, s5; \
		csrw 0x8B6, s6; \
		csrw 0x8B7, s7; \
		csrw 0x8B8, s8; \
		csrw 0x8B9, s9; \
		csrw 0x8BA, s10; \
		csrw 0x8BB, s11; \
		csrw 0x8BC, t3; \
		csrw 0x8BD, t4; \
		csrw 0x8BE, t5; \
		csrw 0x8BF, t6; \
		csrr a5, mepc; \
		csrw 0x8A0, a5; \
	");

	// CSR[0x011] now contains A7 (SYSCALL number)
	uint32_t value = read_csr(0x8B1);	// Instruction word or hardware bit - A7
	uint32_t cause = read_csr(mcause);	// Exception cause on bits [18:16] (https://riscv.org/wp-content/uploads/2017/05/riscv-privileged-v1.10.pdf)
	uint32_t PC = read_csr(mepc);		// Return address == crash PC
	uint32_t code = cause & 0x7FFFFFFF;

	if (cause & 0x80000000) // Hardware interrupts
	{
		switch (code)
		{
			case IRQ_M_TIMER:
			{
				// Machine Timer Interrupt (timer)
				// Task scheduler runs here

				// Switch between running tasks
				// TODO: Return time slice request of current task
				uint32_t runLength = TaskSwitchToNext(&g_taskctx);

				// Task scheduler will re-visit after we've filled run length of this task
				uint64_t now = E32ReadTime();
				// TODO: Use time slice request returned from TaskSwitchToNext()
				uint64_t future = now + runLength;
				E32SetTimeCompare(future);
				break;
			}

			case IRQ_M_EXT:
			{
				// Bit mask of devices causing the current interrupt
				uint32_t hwid = read_csr(0xFFF);

				if (hwid&1) HandleSDCardDetect();
				else if (hwid&2) HandleUSBSerial();
				else if (hwid&4) HandleUSBHID();
				else if (hwid&8) HandleSoftReset(PC);
				else // No familiar bit set, unknown device
				{
					ReportError(32, "Unknown hardware device, core halted", code, hwid, PC);
					// Put core to endless sleep
					while(1) { asm volatile("wfi;"); }
					break;
				}

				break;
			}

			default:
			{
				ReportError(32, "Unknown interrupt type, core halted", code, 0, PC);

				// Put core to endless sleep
				while(1) { asm volatile("wfi;"); }
				break;
			}
		}
	}
	else
	{
		switch(code)
		{
			case CAUSE_ILLEGAL_INSTRUCTION:
			{
				uint32_t instruction = *((uint32_t*)PC);
				ReportError(32, "Illegal instruction", code, instruction, PC);

				// Terminate task on first chance and remove from list of running tasks
				TaskExitCurrentTask(&g_taskctx);
				// Force switch to next task
				TaskSwitchToNext(&g_taskctx);

				// TODO: Drop into debugger if one's loaded

				break;
			}

			case CAUSE_BREAKPOINT:
			{
				// TODO: call debugger breakpoint handler if one's loaded and stop the task

				// Where there's no debugger loaded, simply exit since we're not supposed to run past ebreak commands
				ReportError(32, "No debugger present, ignoring breakpoint", code, 0x0, PC);

				// Exit task in non-debug mode
				TaskExitCurrentTask(&g_taskctx);
				// Force switch to next task
				TaskSwitchToNext(&g_taskctx);

				// TODO: Drop into debugger if one's loaded

				break;
			}

			case CAUSE_MACHINE_ECALL:
			{
				// See: https://jborza.com/post/2021-05-11-riscv-linux-syscalls/
				// Builtin
				// 0			io_setup	long io_setup(unsigned int nr_events, aio_context_t *ctx_idp);
				// 17			getcwd		char *getcwd(char *buf, size_t size);
				// 50			chdir		chdir(const char *path);
				// 57			close		int sys_close(unsigned int fd);
				// 62			lseek		off_t sys_lseek(int fd, off_t offset, int whence);
				// 63			read		ssize_t read(int fd, void *buf, size_t count);
				// 64			write		ssize_t write(int fd, const void *buf, size_t count);
				// 80			newfstat	long sys_newfstat(unsigned int fd, struct stat __user *statbuf);
				// 93			exit		noreturn void _exit(int status);
				// 129			kill		int kill(pid_t pid, int sig);
				// 214			brk			int brk(void *addr); / void *sbrk(intptr_t increment);
				// 1024			open		long sys_open(const char __user * filename, int flags, umode_t mode); open/create file
				// 1025			rename		int rename(const char *oldpath, const char *newpath);
				// 1026			remove		remove(const char *fname);
				// Custom
				// 0xFFFFFFFF	setdebugger	void *setdebugger(unsigned int flags); // flags: connect/disconnect

				if (value==0) // io_setup()
				{
					//sys_io_setup(unsigned nr_reqs, aio_context_t __user *ctx);
					ReportError(32, "unimpl: io_setup()", code, value, PC);
					write_csr(0x8AA, 0xFFFFFFFF);
				}
				else if (value==17) // getcwd()
				{
					//char *getcwd(char *buf, size_t size);
					char* targetbuffer = (char*)read_csr(0x8AA); // A0
					uint32_t targetsize = read_csr(0x8AB); // A1
					if (targetsize >= MAXFILENAMELEN)
						write_csr(0x8AA, 0x0); // nullptr
					else
					{
						FRESULT cwdattempt = f_getcwd(targetbuffer, targetsize);
						if (cwdattempt == FR_OK)
							write_csr(0x8AA, targetbuffer);
						else
							write_csr(0x8AA, 0x0); // nullptr
					}
				}
				else if (value==50) // chdir
				{
					//chdir(const char *path);
					char *path = (char*)read_csr(0x8AA); // A0
					if (path)
					{
						f_chdir(path);
						write_csr(0x8AA, 0x0);
					}
					else
						write_csr(0x8AA, 0xFFFFFFFF);
				}
				else if (value==57) // close()
				{
					uint32_t file = read_csr(0x8AA); // A0

					if (file > STDERR_FILENO) // Won't let stderr, stdout and stdin be closed
					{
						ReleaseFileHandle(file, &s_handleAllocMask);
						f_close(&s_filehandles[file]);
					}
					write_csr(0x8AA, 0);
				}
				else if (value==62) // lseek()
				{
					// NOTE: We do not support 'holes' in files
					uint32_t file = read_csr(0x8AA); // A0
					uint32_t offset = read_csr(0x8AB); // A1
					uint32_t whence = read_csr(0x8AC); // A2

					// Grab current cursor
					FSIZE_t currptr = s_filehandles[file].fptr;

					if (whence == 2 ) // SEEK_END
					{
						// Offset from end of file
						currptr = offset + s_filehandles[file].obj.objsize;
					}
					else if (whence == 1) // SEEK_CUR
					{
						// Offset from current position
						currptr = offset + currptr;
					}
					else// if (whence == 0) // SEEK_SET
					{
						// Direct offset
						currptr = offset;
					}

					FRESULT seekattempt = f_lseek(&s_filehandles[file], currptr);
					if (seekattempt == FR_OK)
						write_csr(0x8AA, currptr);
					else
					{
						errno = EIO;
						write_csr(0x8AA, 0xFFFFFFFF);
					}
				}
				else if (value==63) // read()
				{
					uint32_t file = read_csr(0x8AA); // A0
					uint32_t ptr = read_csr(0x8AB); // A1
					uint32_t len = read_csr(0x8AC); // A2

					if (file == STDIN_FILENO)
					{
						// TODO: Maybe read one character from UART here?
						errno = EIO;
						write_csr(0x8AA, 0xFFFFFFFF);
					}
					else if (file == STDOUT_FILENO)
					{
						// Can't read from stdout
						errno = EIO;
						write_csr(0x8AA, 0xFFFFFFFF);
					}
					else if (file == STDERR_FILENO)
					{
						// Can't read from stderr
						errno = EIO;
						write_csr(0x8AA, 0xFFFFFFFF);
					}
					else // Any other ordinary file
					{
						FRESULT readattempt = f_read(&s_filehandles[file], (void*)ptr, len, &tmpresult);
						if (IsFileHandleAllocated(file, s_handleAllocMask))
						{
							if (readattempt == FR_OK)
								write_csr(0x8AA, tmpresult);
							else
							{
								errno = EIO;
								write_csr(0x8AA, 0xFFFFFFFF);
							}
						}
						else
						{
							errno = EBADF;
							write_csr(0x8AA, 0xFFFFFFFF);
						}
					}
				}
				else if (value==64) // write()
				{
					uint32_t file = read_csr(0x8AA); // A0
					uint32_t ptr = read_csr(0x8AB); // A1
					uint32_t count = read_csr(0x8AC); // A2

					if (file == STDOUT_FILENO || file == STDERR_FILENO)
					{
						int outcount = USBSerialWriteN((const char*)ptr, count);
						write_csr(0x8AA, outcount);
					}
					else
					{
						if (IsFileHandleAllocated(file, s_handleAllocMask))
						{
							FRESULT writeattempt = f_write(&s_filehandles[file], (const void*)ptr, count, &tmpresult);
							if (writeattempt == FR_OK)
								write_csr(0x8AA, tmpresult);
							else
							{
								errno = EIO;
								write_csr(0x8AA, 0xFFFFFFFF);
							}
						}
						else
						{
							errno = EACCES;
							write_csr(0x8AA, 0xFFFFFFFF);
						}
					}
				}
				else if (value==80) // newfstat()
				{
					uint32_t fd = read_csr(0x8AA); // A0
					uint32_t ptr = read_csr(0x8AB); // A1
					struct stat *buf = (struct stat *)ptr;

					if (fd < 0)
					{
						errno = EBADF;
						write_csr(0x8AA, 0xFFFFFFFF);
					}
					else
					{
						if (fd <= STDERR_FILENO)
						{
							buf->st_dev = 0;
							buf->st_ino = 0;
							buf->st_mode = S_IFCHR; // character device
							buf->st_nlink = 0;
							buf->st_uid = 0;
							buf->st_gid = 0;
							buf->st_rdev = 1;
							buf->st_size = 0;
							buf->st_blksize = 0;
							buf->st_blocks = 0;
							buf->st_atim.tv_sec = 0;
							buf->st_atim.tv_nsec = 0;
							buf->st_mtim.tv_sec = 0;
							buf->st_mtim.tv_nsec = 0;
							buf->st_ctim.tv_sec = 0;
							buf->st_ctim.tv_nsec = 0;
							write_csr(0x8AA, 0x0);
						}
						else // Ordinary files
						{
							FILINFO finf;
							FRESULT fr = f_stat(s_fileNames[fd], &finf);

							if (fr != FR_OK)
							{
								errno = ENOENT;
								write_csr(0x8AA, 0xFFFFFFFF);
							}
							else
							{
								buf->st_dev = 1;
								buf->st_ino = 0;
								buf->st_mode = S_IFREG; // regular file
								buf->st_nlink = 0;
								buf->st_uid = 0;
								buf->st_gid = 0;
								buf->st_rdev = 0;
								buf->st_size = finf.fsize;
								buf->st_blksize = 512;
								buf->st_blocks = (finf.fsize+511)/512;
								buf->st_atim.tv_sec = finf.ftime;
								buf->st_atim.tv_nsec = 0;
								buf->st_mtim.tv_sec = 0;
								buf->st_mtim.tv_nsec = 0;
								buf->st_ctim.tv_sec = 0;
								buf->st_ctim.tv_nsec = 0;
								write_csr(0x8AA, 0x0);
							}
						}
					}
				}
				else if (value==93) // exit()
				{
					// Terminate and remove from list of running tasks
					TaskExitCurrentTask(&g_taskctx);
					write_csr(0x8AA, 0x0);
				}
				else if (value==95) // wait()
				{
					// Wait for child process status change - unused
					// pid_t wait(int *wstatus);
					ReportError(32, "unimpl: wait()", code, value, PC);
					errno = ECHILD;
					write_csr(0x8AA, 0xFFFFFFFF);
				}
				else if (value==129) // kill(pid_t pid, int sig)
				{
					// Signal process to terminate
					uint32_t pid = read_csr(0x8AA); // A0
					uint32_t sig = read_csr(0x8AB); // A1
					TaskExitTaskWithID(&g_taskctx, pid, sig);
					mini_snprintf(k_tmpstr, 64, "\nSIG:0x%x PID:0x%x\n", sig, pid);
					USBSerialWrite(k_tmpstr);
					write_csr(0x8AA, sig);
				}
				else if (value==214) // brk()
				{
					uint32_t addrs = read_csr(0x8AA); // A0
					uint32_t retval = core_brk(addrs);
					write_csr(0x8AA, retval);
				}
				else if (value==1024) // open()
				{
					uint32_t nptr = read_csr(0x8AA); // A0
					uint32_t oflags = read_csr(0x8AB); // A1
					//uint32_t pmode = read_csr(0x8AC); // A2 - permission mode unused for now

					BYTE ff_flags = FA_READ;
					{
						uint32_t fcls = (oflags & 3);
						if(fcls == 00)
							ff_flags = FA_READ; // O_RDONLY
						else if(fcls == 01)
							ff_flags = FA_WRITE; // O_WRONLY
						else if(fcls == 02)
							ff_flags = FA_READ|FA_WRITE; // O_RDWR
						else
							ff_flags = FA_READ;
					}
					ff_flags |= (oflags&100) ? FA_CREATE_ALWAYS : 0; // O_CREAT
					ff_flags |= (oflags&2000) ? FA_OPEN_APPEND : 0; // O_APPEND

					// Grab lowest zero bit's index
					int currenthandle = FindFreeFileHandle(s_handleAllocMask);

					if (currenthandle == 0)
					{
						// No free file handles
						errno = ENFILE;
						write_csr(0x8AA, 0xFFFFFFFF);
					}
					else if (currenthandle > STDERR_FILENO)
					{
						FRESULT openattempt = f_open(&s_filehandles[currenthandle], (const TCHAR*)nptr, ff_flags);
						if (openattempt == FR_OK)
						{
							AllocateFileHandle(currenthandle, &s_handleAllocMask);
							write_csr(0x8AA, currenthandle);

							char *trg = s_fileNames[currenthandle];
							char *src = (char*)nptr;
							uint32_t cntr = 0;
							while(*src!=0 && cntr<MAXFILENAMELEN)
							{
								*trg++ = *src++;
								++cntr;
							}
							*trg = 0;
						}
						else
						{
							errno = ENOENT;
							write_csr(0x8AA, 0xFFFFFFFF);
						}
					}
					else
					{
						// STDIN/STDOUT/STDERR
						write_csr(0x8AA, 0x0);
					}
				}
				else if (value==1025) // rename()
				{
					char* oldname = (char*)read_csr(0x8AA); // A0
					char* newname = (char*)read_csr(0x8AB); // A1
					if (oldname && newname)
					{
						FRESULT fr = f_rename(oldname, newname);
						if (fr != FR_OK)
						{
							errno = ENOENT;
							write_csr(0x8AA, 0xFFFFFFFF);
						}
						else
							write_csr(0x8AA, 0x0);
					}
					else
						write_csr(0x8AA, 0xFFFFFFFF);
				}
				else if (value==1026) // remove() (unlink)
				{
					uint32_t nptr = read_csr(0x8AA); // A0
					FRESULT fr = f_unlink((char*)nptr);
					if (fr == FR_OK)
						write_csr(0x8AA, 0x0);
					else
					{
						errno = ENOENT;
						write_csr(0x8AA, 0xFFFFFFFF);
					}
				}
				else if (value==0xFFFFFFFF) // setdebugger()
				{
					ReportError(32, "Unimplemented debugger interface", code, value, PC);
					write_csr(0x8AA, 0xFFFFFFFF);
				}
				else // Unimplemented syscalls drop here
				{
					ReportError(32, "unimplemented ECALL", code, value, PC);
					errno = EIO;
					write_csr(0x8AA, 0xFFFFFFFF);
				}
				break;
			}

			/*case CAUSE_MISALIGNED_FETCH:
			case CAUSE_FETCH_ACCESS:
			case CAUSE_MISALIGNED_LOAD:
			case CAUSE_LOAD_ACCESS:
			case CAUSE_MISALIGNED_STORE:
			case CAUSE_STORE_ACCESS:
			case CAUSE_USER_ECALL:
			case CAUSE_SUPERVISOR_ECALL:
			case CAUSE_HYPERVISOR_ECALL:
			case CAUSE_FETCH_PAGE_FAULT:
			case CAUSE_LOAD_PAGE_FAULT:
			case CAUSE_STORE_PAGE_FAULT:*/
			default:
			{
				// Code contains the CAUSE_ code
				ReportError(32, "Guru meditation, core halted", code, value, PC);

				// Put core to endless sleep
				while(1) { asm volatile("wfi;"); }
				break;
			}
		}
	}

	// Restore registers to next task's register set
	// NOTE: Restores PC from saved MEPC so that MRET can branch to the task
	asm volatile(" \
		csrr a5, 0x8A0; \
		csrw mepc, a5; \
		csrr ra,  0x8A1; \
		csrr sp,  0x8A2; \
		csrr gp,  0x8A3; \
		csrr tp,  0x8A4; \
		csrr t0,  0x8A5; \
		csrr t1,  0x8A6; \
		csrr t2,  0x8A7; \
		csrr s0,  0x8A8; \
		csrr s1,  0x8A9; \
		csrr a0,  0x8AA; \
		csrr a1,  0x8AB; \
		csrr a2,  0x8AC; \
		csrr a3,  0x8AD; \
		csrr a4,  0x8AE; \
		csrr a5,  0x8AF; \
		csrr a6,  0x8B0; \
		csrr a7,  0x8B1; \
		csrr s2,  0x8B2; \
		csrr s3,  0x8B3; \
		csrr s4,  0x8B4; \
		csrr s5,  0x8B5; \
		csrr s6,  0x8B6; \
		csrr s7,  0x8B7; \
		csrr s8,  0x8B8; \
		csrr s9,  0x8B9; \
		csrr s10, 0x8BA; \
		csrr s11, 0x8BB; \
		csrr t3,  0x8BC; \
		csrr t4,  0x8BD; \
		csrr t5,  0x8BE; \
		csrr t6,  0x8BF; \
		mret; \
	");
}

void InstallISR()
{
	// Set machine trap vector
	write_csr(mtvec, interrupt_service_routine);

	// Set up timer interrupt one second into the future
	uint64_t now = E32ReadTime();
	uint64_t future = now + TWO_HUNDRED_FIFTY_MILLISECONDS_IN_TICKS;
	E32SetTimeCompare(future);

	// Enable machine software interrupts (breakpoint/illegal instruction)
	// Enable machine hardware interrupts
	// Enable machine timer interrupts
	write_csr(mie, MIP_MSIP | MIP_MEIP | MIP_MTIP);

	// Allow all machine interrupts to trigger (thus also enabling task system)
	write_csr(mstatus, MSTATUS_MIE);
}
