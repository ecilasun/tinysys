#include "rombase.h"
#include "sdcard.h"
#include "uart.h"
#include "mini-printf.h"
#include "serialinringbuffer.h"
#include "gdbstub.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static FATFS Fs;
static char s_workdir[PATH_MAX+4] = "sd:/";

int kprintfn(const int count, const char *fmt, ...)
{
	va_list va;
	int l;

	// Stay away from other uses by devices etc
	char *k_tmpstr = (char*)(KERNEL_TEMP_MEMORY + 1024);

	va_start(va, fmt);
	l = mini_vsnprintf(k_tmpstr, 1023, fmt, va);
	va_end(va);
	l = count < l ? count : l;

	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	k_tmpstr[count] = 0;
	VPUConsolePrint(kernelgfx, k_tmpstr, count);

	return l;
}

int kprintf(const char *fmt, ...)
{
	va_list va;
	int l;

	// Stay away from other uses by devices etc
	char *k_tmpstr = (char*)(KERNEL_TEMP_MEMORY + 1024);

	va_start(va, fmt);
	l = mini_vsnprintf(k_tmpstr, 1023, fmt, va);
	va_end(va);
	l = 1023 < l ? 1023 : l;

	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	k_tmpstr[l] = 0;
	VPUConsolePrint(kernelgfx, k_tmpstr, l);

	return l;
}

int kdebugprintf(const char *fmt, ...)
{
	va_list va;
	int l;

	// Stay away from other uses by devices etc
	char *k_tmpstr = (char*)(KERNEL_TEMP_MEMORY + 1024);

	va_start(va, fmt);
	l = mini_vsnprintf(k_tmpstr, 1023, fmt, va);
	va_end(va);
	l = 1023 < l ? 1023 : l;

	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	k_tmpstr[l] = 0;
	VPUConsolePrint(kernelgfx, k_tmpstr, l);
	UARTSendBlock((uint8_t*)k_tmpstr, l);

	return l;
}

int kfillline(const char c)
{
	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	int count = VPUConsoleFillLine(kernelgfx, c);
	return count;
}

void ksetcolor(int8_t fg, int8_t bg)
{
	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	VPUConsoleSetColors(kernelgfx, fg, bg);
}

void kgetcursor(int *_x, int *_y)
{
	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	*_x = kernelgfx->m_cursorX;
	*_y = kernelgfx->m_cursorY;
}

void ksetcaret(const int _x, const int _y, const int _blink)
{
	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	VPUConsoleSetCaret(kernelgfx, _x, _y, _blink);
}

void ksetcursor(const int _x, const int _y)
{
	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	VPUConsoleSetCursor(kernelgfx, _x, _y);
}

uint32_t _task_abort_to(struct STaskContext *_ctx, const uint32_t _abortTo)
{
	_ctx->currentTask = _abortTo;

	uint32_t *regs = _ctx->tasks[_abortTo].regs;

	// Load next tasks's registers into CSR file
	write_csr(0x8A0, regs[0]);	// PC of next task
	write_csr(0x8A1, regs[1]);	// ra
	write_csr(0x8A2, regs[2]);	// sp
	write_csr(0x8A3, regs[3]);	// gp
	write_csr(0x8A4, regs[4]);	// tp
	write_csr(0x8A5, regs[5]);	// t0
	write_csr(0x8A6, regs[6]);	// t1
	write_csr(0x8A7, regs[7]);	// t2
	write_csr(0x8A8, regs[8]);	// s0
	write_csr(0x8A9, regs[9]);	// s1
	write_csr(0x8AA, regs[10]);	// a0
	write_csr(0x8AB, regs[11]);	// a1
	write_csr(0x8AC, regs[12]);	// a2
	write_csr(0x8AD, regs[13]);	// a3
	write_csr(0x8AE, regs[14]);	// a4
	write_csr(0x8AF, regs[15]);	// a5
	write_csr(0x8B0, regs[16]);	// a6
	write_csr(0x8B1, regs[17]);	// a7
	write_csr(0x8B2, regs[18]);	// s2
	write_csr(0x8B3, regs[19]);	// s3
	write_csr(0x8B4, regs[20]);	// s4
	write_csr(0x8B5, regs[21]);	// s5
	write_csr(0x8B6, regs[22]);	// s6
	write_csr(0x8B7, regs[23]);	// s7
	write_csr(0x8B8, regs[24]);	// s8
	write_csr(0x8B9, regs[25]);	// s9
	write_csr(0x8BA, regs[26]);	// s10
	write_csr(0x8BB, regs[27]);	// s11
	write_csr(0x8BC, regs[28]);	// t3
	write_csr(0x8BD, regs[29]);	// t4
	write_csr(0x8BE, regs[30]);	// t5
	write_csr(0x8BF, regs[31]);	// t6

	return _ctx->tasks[_abortTo].runLength;
}

uint32_t _task_switch_to_next(struct STaskContext *_ctx)
{
	// Load current process ID from TP register
	int32_t currentTask = _ctx->currentTask;

	// Get current task's register stash
	uint32_t *regs = _ctx->tasks[currentTask].regs;

	// Store register back-ups in current task's structure
	regs[0] = read_csr(0x8A0);	// PC of halted task
	regs[1] = read_csr(0x8A1);	// ra
	regs[2] = read_csr(0x8A2);	// sp
	regs[3] = read_csr(0x8A3);	// gp
	regs[4] = read_csr(0x8A4);	// tp
	regs[5] = read_csr(0x8A5);	// t0
	regs[6] = read_csr(0x8A6);	// t1
	regs[7] = read_csr(0x8A7);	// t2
	regs[8] = read_csr(0x8A8);	// s0
	regs[9] = read_csr(0x8A9);	// s1
	regs[10] = read_csr(0x8AA);	// a0
	regs[11] = read_csr(0x8AB);	// a1
	regs[12] = read_csr(0x8AC);	// a2
	regs[13] = read_csr(0x8AD);	// a3
	regs[14] = read_csr(0x8AE);	// a4
	regs[15] = read_csr(0x8AF);	// a5
	regs[16] = read_csr(0x8B0);	// a6
	regs[17] = read_csr(0x8B1);	// a7
	regs[18] = read_csr(0x8B2);	// s2
	regs[19] = read_csr(0x8B3);	// s3
	regs[20] = read_csr(0x8B4);	// s4
	regs[21] = read_csr(0x8B5);	// s5
	regs[22] = read_csr(0x8B6);	// s6
	regs[23] = read_csr(0x8B7);	// s7
	regs[24] = read_csr(0x8B8);	// s8
	regs[25] = read_csr(0x8B9);	// s9
	regs[26] = read_csr(0x8BA);	// s10
	regs[27] = read_csr(0x8BB);	// s11
	regs[28] = read_csr(0x8BC);	// t3
	regs[29] = read_csr(0x8BD);	// t4
	regs[30] = read_csr(0x8BE);	// t5
	regs[31] = read_csr(0x8BF);	// t6

	// Spin until we find a task that is not paused or terminated
	do {
		// Next task in queue
		currentTask = (_ctx->numTasks <= 1) ? 0 : ((currentTask+1) % _ctx->numTasks);

		// Check if task is terminated
		if (_ctx->tasks[currentTask].state == TS_TERMINATING)
		{
			_ctx->kernelError = _ctx->kernelError | 0x80000000; // Task terminated (not a real error, also take care to not overwrite any real errors)
			_ctx->kernelErrorData[0] = currentTask;
			_ctx->kernelErrorData[1] = _ctx->tasks[currentTask].exitCode;
			if (currentTask != _ctx->numTasks-1)
				__builtin_memcpy(&_ctx->tasks[currentTask], &_ctx->tasks[_ctx->numTasks-1], sizeof(struct STask));
			else
				_ctx->tasks[currentTask].state = TS_TERMINATED;
			// One less task to run
			--_ctx->numTasks;
			// Wind back to OS Idle task
			currentTask = 0;
		}
		else if (_ctx->tasks[currentTask].state == TS_RUNNING || currentTask == 0) // Found a task to run or dropped to IDLE task
			break;
	} while (1);

	_ctx->currentTask = currentTask;

	regs = _ctx->tasks[currentTask].regs;

	// Load next tasks's registers into CSR file
	write_csr(0x8A0, regs[0]);	// PC of next task
	write_csr(0x8A1, regs[1]);	// ra
	write_csr(0x8A2, regs[2]);	// sp
	write_csr(0x8A3, regs[3]);	// gp
	write_csr(0x8A4, regs[4]);	// tp
	write_csr(0x8A5, regs[5]);	// t0
	write_csr(0x8A6, regs[6]);	// t1
	write_csr(0x8A7, regs[7]);	// t2
	write_csr(0x8A8, regs[8]);	// s0
	write_csr(0x8A9, regs[9]);	// s1
	write_csr(0x8AA, regs[10]);	// a0
	write_csr(0x8AB, regs[11]);	// a1
	write_csr(0x8AC, regs[12]);	// a2
	write_csr(0x8AD, regs[13]);	// a3
	write_csr(0x8AE, regs[14]);	// a4
	write_csr(0x8AF, regs[15]);	// a5
	write_csr(0x8B0, regs[16]);	// a6
	write_csr(0x8B1, regs[17]);	// a7
	write_csr(0x8B2, regs[18]);	// s2
	write_csr(0x8B3, regs[19]);	// s3
	write_csr(0x8B4, regs[20]);	// s4
	write_csr(0x8B5, regs[21]);	// s5
	write_csr(0x8B6, regs[22]);	// s6
	write_csr(0x8B7, regs[23]);	// s7
	write_csr(0x8B8, regs[24]);	// s8
	write_csr(0x8B9, regs[25]);	// s9
	write_csr(0x8BA, regs[26]);	// s10
	write_csr(0x8BB, regs[27]);	// s11
	write_csr(0x8BC, regs[28]);	// t3
	write_csr(0x8BD, regs[29]);	// t4
	write_csr(0x8BE, regs[30]);	// t5
	write_csr(0x8BF, regs[31]);	// t6

	return _ctx->tasks[currentTask].runLength;
}

int _task_add(struct STaskContext *_ctx, const char *_name, taskfunc _task, enum ETaskState _initialState, const uint32_t _runLength, const uint32_t _hartid, const uint32_t _gp, const uint32_t _parentStackPointer)
{
	int32_t prevcount = _ctx->numTasks;
	if (prevcount >= TASK_MAX)
		return 0;

	// Insert the task before we increment task count
	struct STask *task = &(_ctx->tasks[prevcount]);
	task->HART = _hartid;
	task->regs[0] = (uint32_t)_task;		// Initial PC
	task->regs[2] = _parentStackPointer;	// Stack pointer
	task->regs[3] = _gp;					// Global pointer
	task->regs[8] = _parentStackPointer;	// Frame pointer
	task->runLength = _runLength;			// Time slice dedicated to this task
	task->name = (uint32_t)_name;			// Pointer to task name (in external memory)

	// We assume running state as soon as we start
	task->state = _initialState;

	++_ctx->numTasks;

	return prevcount;
}

void _task_exit_task_with_id(struct STaskContext *_ctx, uint32_t _taskid, uint32_t _signal)
{
	if (_taskid == 0) // Can not exit taskid 0
		return;

	struct STask *task = &_ctx->tasks[_taskid];
	task->state = TS_TERMINATING;
	task->exitCode = _signal;
}

void _task_exit_current_task(struct STaskContext *_ctx)
{
	uint32_t taskid = _ctx->currentTask;
	if (taskid == 0) // Can not exit taskid 0
		return;

	struct STask *task = &_ctx->tasks[taskid];
	task->state = TS_TERMINATING;
	task->exitCode = 0; // Default exit code

	// NOTE: We need some means to call the atexit() handlers before we actually exit
}

uint64_t _task_yield()
{
	// Set up the next task switch interrupt to almost-now
	// so we can yield as soon as possible.
	uint64_t now = E32ReadTime();
	E32SetTimeCompare(now);
	return now;
}

struct STaskContext *_task_get_context(uint32_t _hartid)
{
	// Each task starts at 1Kbyte boundary
	// We have >80Kbytes in the task space so this should support plenty of cores
	struct STaskContext *contextpool = (struct STaskContext *)DEVICE_MAIL;
	return &contextpool[_hartid];
}

void *_task_get_shared_memory()
{
	// One entry away from the last task context
	return (void*)_task_get_context(MAX_HARTS);
}

void _task_init_context(uint32_t _hartid)
{
	// Initialize task context memory
	struct STaskContext *ctx = _task_get_context(_hartid);
	memset(ctx, 0x0, sizeof(struct STaskContext));
	ctx->hartID = _hartid;
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
		// Go to whatever the work directory was set to before
		f_chdir(s_workdir);
		return 1;
	}
	return 0;
}

// Based on https://opensource.apple.com/source/Libc/Libc-391.2.3/stdlib/FreeBSD/realpath.c.auto.html
char *krealpath(const char *path, char resolved[PATH_MAX])
{
	FILINFO finf;
	char *p, *q, *s;
	size_t left_len, resolved_len;
	int serrno;
	char left[PATH_MAX], next_token[PATH_MAX];

	serrno = errno;
	if (path[0] == '/') {
		resolved[0] = 's';
		resolved[1] = 'd';
		resolved[2] = ':';
		resolved[3] = '/';
		resolved[4] = '\0';
		if (path[1] == '\0')
			return (resolved);
		resolved_len = 4;
		left_len = strlcpy(left, path + 1, sizeof(left));
	} else {
		if (f_getcwd(resolved, PATH_MAX) != FR_OK) {
			strlcpy(resolved, ".", PATH_MAX);
			return (NULL);
		}
		resolved_len = strlen(resolved);
		left_len = strlcpy(left, path, sizeof(left));
	}
	if (left_len >= sizeof(left) || resolved_len >= PATH_MAX) {
		errno = ENAMETOOLONG;
		return (NULL);
	}
	/*
	 * Iterate over path components in `left'.
	 */
	while (left_len != 0) {
		/*
		 * Extract the next path component and adjust `left'
		 * and its length.
		 */
		p = strchr(left, '/');
		s = p ? p : left + left_len;
		if (s - left >= sizeof(next_token)) {
			errno = ENAMETOOLONG;
			return (NULL);
		}
		memcpy(next_token, left, s - left);
		next_token[s - left] = '\0';
		left_len -= s - left;
		if (p != NULL)
			memmove(left, s + 1, left_len + 1);
		if (resolved[resolved_len - 1] != '/') {
			if (resolved_len + 1 >= PATH_MAX) {
				errno = ENAMETOOLONG;
				return (NULL);
			}
			resolved[resolved_len++] = '/';
			resolved[resolved_len] = '\0';
		}
		if (next_token[0] == '\0')
			continue;
		else if (strcmp(next_token, ".") == 0)
			continue;
		else if (strcmp(next_token, "..") == 0) {
			/*
			 * Strip the last path component except when we have
			 * single "/"
			 */
			if (resolved_len > 1) {
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}
			continue;
		}

		/*
		 * Append the next path component and lstat() it. If
		 * lstat() fails we still can return successfully if
		 * there are no more path components left.
		 */
		resolved_len = strlcat(resolved, next_token, PATH_MAX);
		if (resolved_len >= PATH_MAX) {
			errno = ENAMETOOLONG;
			return (NULL);
		}
		if (f_stat(resolved, &finf) != FR_OK) {
			if (errno == ENOENT && p == NULL) {
				errno = serrno;
				return (resolved);
			}
			return (NULL);
		}
	}

	// NOTE: We must never remove trailing /
	/*
	 * Remove trailing slash except when the resolved pathname
	 * is a single "/".
	 */
	/*if (resolved_len > 1 && resolved[resolved_len - 1] == '/')
		resolved[resolved_len - 1] = '\0';*/
	return (resolved);
}

void UnmountDrive()
{
	f_mount(NULL, "sd:", 1);
}

void ListFiles(const char *path)
{
	DIR dir;
	FRESULT re = f_opendir(&dir, path);
	static const char blankspace[33] = "                                ";

	if (re == FR_OK)
	{
		FILINFO finf;
		do{
			re = f_readdir(&dir, &finf);
			if (re != FR_OK || finf.fname[0] == 0) // Done scanning dir, or error encountered
				break;

			int isdir = finf.fattrib&AM_DIR;
			char *isexe = strstr(finf.fname, ".elf");
			if (isdir)
				ksetcolor(CONSOLECYAN, CONSOLEDEFAULTBG);
			else if (isexe!=NULL)
				ksetcolor(CONSOLEYELLOW, CONSOLEDEFAULTBG);
			else
				ksetcolor(CONSOLEDIMWHITE, CONSOLEDEFAULTBG);

			// Make sure we're always aligned to max 32 characters
			int count = 0;
			while(finf.fname[count]!=0) { count++; }
			count = count>32 ? 32 : count;
			kprintfn(count, finf.fname);
			if (count<32)
				kprintfn(32-count, blankspace);

			if (isdir)
				kprintf(" <dir>");
			else
			{
				kprintf(" ");

				uint32_t inkbytes = (uint32_t)finf.fsize/1024;
				uint32_t inmbytes = inkbytes/1024;

				if (inmbytes!=0)
					kprintf("%d Mb", inmbytes);
				else if (inkbytes!=0)
					kprintf("%d Kb", inkbytes);
				else
					kprintf("%d b", (uint32_t)finf.fsize);
			}
			kprintf("\n");
		} while(1);

		f_closedir(&dir);
	}
	else
	{
		ksetcolor(CONSOLEDIMRED, CONSOLEDEFAULTBG);
		kprintf("File system error (listfiles)\n");
	}

	ksetcolor(CONSOLEDEFAULTFG, CONSOLEDEFAULTBG);
}

uint32_t ParseELFHeaderAndLoadSections(FIL *fp, struct SElfFileHeader32 *fheader, uint32_t* jumptarget, int _relocOffset)
{
	uint32_t heap_start = 0;
	if (fheader->m_Magic != 0x464C457F)
	{
		kprintf("ELF header error\n");
		return heap_start;
	}

	*jumptarget = fheader->m_Entry + _relocOffset;
	UINT bytesread = 0;

	// Read program headers
	for (uint32_t i=0; i<fheader->m_PHNum; ++i)
	{
		uint8_t tmp[512];
		struct SElfProgramHeader32 *pheader = (struct SElfProgramHeader32 *)tmp;

		f_lseek(fp, fheader->m_PHOff + fheader->m_PHEntSize*i);
		f_read(fp, pheader, sizeof(struct SElfProgramHeader32), &bytesread);

		// Something here
		if (pheader.m_MemSz != 0)
		{
			uint8_t *memaddr = (uint8_t *)(pheader.m_PAddr + _relocOffset);
			// Check illegal range
			if ((uint32_t)memaddr>=HEAP_END || ((uint32_t)memaddr)+pheader.m_MemSz>=HEAP_END)
			{
				kprintf("ELF section in illegal memory region\n");
				return 0;
			}
			else
			{
				// Initialize the memory range at target physical address
				// This can be larger than the loaded size
				memset(memaddr, 0x0, pheader.m_MemSz);

				// Load the binary section depending on 'load' flag
				if (pheader.m_Type == PT_LOAD)
				{
					f_lseek(fp, pheader.m_Offset);
					f_read(fp, memaddr, pheader.m_FileSz, &bytesread);
				}

				uint32_t blockEnd = (uint32_t)memaddr + pheader.m_MemSz;
				heap_start = heap_start < blockEnd ? blockEnd : heap_start;
			}
		}
	}

	return E32AlignUp(heap_start, 1024);
}

uint32_t LoadExecutable(const char *filename, int _relocOffset, const bool reportError)
{
	FIL fp;
	FRESULT fr = f_open(&fp, filename, FA_READ);

	if (fr == FR_OK)
	{
		// Something was there, load and parse it
		uint8_t tmp[512];
		struct SElfFileHeader32 *fheader = (struct SElfFileHeader32 *)tmp;
		UINT readsize;
		f_read(&fp, fheader, sizeof(struct SElfFileHeader32), &readsize);
		uint32_t branchaddress;
		uint32_t heap_start = ParseELFHeaderAndLoadSections(&fp, &fheader, &branchaddress, _relocOffset);
		f_close(&fp);

		// Success?
		if (heap_start != 0)
		{
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
			kprintf("Executable not found\n");
	}

	return 0;
}

#define MAX_HANDLES 16
#define MAXFILENAMELEN 48

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
	{"                                                "},
	{"                                                "},
	{"                                                "},
	{"                                                "},
	{"                                                "},
	{"                                                "},
	{"                                                "},
	{"                                                "},
	{"                                                "},
	{"                                                "},
	{"                                                "},
	{"                                                "},
	{"                                                "},};

static UINT tmpresult = 0;

void HandleSDCardDetect()
{
	uint32_t cardState = *IO_CARDDETECT;

	if (cardState == 0x0)	// Removed
		UnmountDrive();
	else					// Inserted
		MountDrive();
}

void SetWorkDir(const char *_workdir)
{
	// Convert to real path
	int L = strlen(_workdir);
	strncpy(s_workdir, _workdir, L);
	s_workdir[L] = 0;
}

const char* GetWorkDir()
{
	return s_workdir;
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

void HandleUART()
{
	uint32_t currLED = LEDGetState();
	LEDSetState(currLED | 0x8);
	while (UARTGetStatus() & UARTSTA_RXFIFO_VALID)
	{
		uint8_t rcvData = (uint8_t)(UARTReceiveData() & 0x000000FF);
		SerialInRingBufferWrite(&rcvData, 1);
	}
	LEDSetState(currLED);
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
	// NOTE: 0x8A0 now contains PC of the task we halted to get here

	// Set up ISR stack frame (always at the same location) so that we don't risk overflowing the current task's stack
	uint32_t stackTop = (uint32_t)ISR_STACK_TOP;
	asm (
		"csrr a3, mhartid;"
		"slli a3, a3, 10;"	// hartid*1024
		"mv sp, %0;"
		"sub sp, sp, a3;"	// offset stack
		"mv s0, sp;"
		: // Return values (empty)
		: // Input parameters
		"r" (stackTop)
		: // Clobber list
		"a3" );

	// CSR[0x011] now contains A7 (SYSCALL number)
	uint32_t value = read_csr(0x8B1);	// Instruction word or hardware bit - A7
	uint32_t cause = read_csr(mcause);	// Exception cause on bits [18:16] (https://riscv.org/wp-content/uploads/2017/05/riscv-privileged-v1.10.pdf)
	uint32_t PC = read_csr(mepc);		// Return address == IRQ PC
	uint32_t code = cause & 0x7FFFFFFF;

	// Grab the task context that belongs to this HART
	uint32_t hartid = read_csr(mhartid);
	struct STaskContext *taskctx = _task_get_context(hartid);

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
				uint32_t runLength = _task_switch_to_next(taskctx);

				// Update task breakpoints if we're in debug mode
				//if (GDBIsDebugging())
				//	GDBUpdateBreakpoints(hartid, taskctx->currentTask);

				// Task scheduler will re-visit after we've filled run length of this task
				uint64_t now = E32ReadTime();
				// Use the run length of next task to set the next timer interrupt
				uint64_t future = now + runLength;
				E32SetTimeCompare(future);
			}
			break;

			case IRQ_M_EXT:
			{
				// Bit mask of devices causing the current interrupt
				uint32_t hwid = read_csr(0xFFF);

				// See axi4csrfile.v for the device bit assignments
				// NOTE: We will handle _ALL_ device interrupts in this order
				if (hwid&1)
				{
					/// Reserved for future use
				}
				else if (hwid&2)
				{
					HandleSDCardDetect();
				}
				else if (hwid&4)
				{
					HandleUART();
				}
				else
				{
					// No familiar bit set, unknown device
					taskctx->kernelError = 1;
					taskctx->kernelErrorData[0] = hwid;	// The unknown hardwareid received
				}
			}
			break;

			default:
			{
				taskctx->kernelError = 2;
				taskctx->kernelErrorData[0] = code;	// The unknown IRQ code received
			}
			break;
		}
	}
	else
	{
		switch(code)
		{
			case CAUSE_ILLEGAL_INSTRUCTION:
			{
				//if (!GDBIsDebugging())
				{
					// Capture error
					taskctx->kernelError = 4;
					taskctx->kernelErrorData[0] = taskctx->currentTask;	// Task that crashed
					taskctx->kernelErrorData[1] = *((uint32_t*)PC);		// Instruction
					taskctx->kernelErrorData[2] = PC;					// Program counter

					// Terminate task on first chance and remove from list of running tasks
					_task_exit_current_task(taskctx);
					// Force switch to next available task
					_task_switch_to_next(taskctx);
				}
				/*else
				{
					GDBSignalBreakpoint(hartid, taskctx->currentTask, PC, GDB_SIGNAL_ILL);
				}*/
			}
			break;

			case CAUSE_BREAKPOINT:
			{
				//if (!GDBIsDebugging())
				{
					taskctx->kernelError = 5;
					taskctx->kernelErrorData[0] = taskctx->currentTask;	// Task that invoked ebreak
					taskctx->kernelErrorData[1] = *((uint32_t*)PC);		// Instruction
					taskctx->kernelErrorData[2] = PC;					// Program counter
					// Exit task in non-debug mode
					_task_exit_current_task(taskctx);
					// Force switch to next available task
					_task_switch_to_next(taskctx);
				}
				/*else
				{
					GDBSignalBreakpoint(hartid, taskctx->currentTask, PC, GDB_SIGNAL_TRAP);
				}*/
			}
			break;

			case CAUSE_MACHINE_ECALL:
			{
				// See: https://jborza.com/post/2021-05-11-riscv-linux-syscalls/
				// Builtin
				// 0			io_setup		long io_setup(unsigned int nr_events, aio_context_t *ctx_idp);
				// 17			getcwd			char *getcwd(char *buf, size_t size);
				// 29			ioctl			ioctl(unsigned int fd, unsigned int cmd, unsigned long arg);
				// 50			chdir			chdir(const char *path);
				// 57			close			int sys_close(unsigned int fd);
				// 62			lseek			off_t sys_lseek(int fd, off_t offset, int whence);
				// 63			read			ssize_t read(int fd, void *buf, size_t count);
				// 64			write			ssize_t write(int fd, const void *buf, size_t count);
				// 80			newfstat		long sys_newfstat(unsigned int fd, struct stat __user *statbuf);
				// 93			exit			noreturn void _exit(int status);
				// 129			kill			int kill(pid_t pid, int sig);
				// 214			brk				int brk(void *addr); / void *sbrk(intptr_t increment);
				// 403			gettimeofday	int gettimeofday(struct timeval *tv, struct timezone *tz);
				// 1024			open			long sys_open(const char __user * filename, int flags, umode_t mode); open/create file
				// 1025			rename			int rename(const char *oldpath, const char *newpath);
				// 1026			remove			remove(const char *fname);
				// 1038 		_stat			int stat(const char *path, struct stat *buf);

				if (value==0) // io_setup()
				{
					//sys_io_setup(unsigned nr_reqs, aio_context_t __user *ctx);
					errno = EINVAL;
					write_csr(0x8AA, 0xFFFFFFFF);
				}
				else if (value==17) // getcwd()
				{
					//char *getcwd(char *buf, size_t size);
					char* targetbuffer = (char*)read_csr(0x8AA); // A0
					uint32_t targetsize = read_csr(0x8AB); // A1
					targetsize = targetsize > (MAXFILENAMELEN-1) ? (MAXFILENAMELEN-1) : targetsize;
					FRESULT cwdattempt = f_getcwd(targetbuffer, targetsize);
					if (cwdattempt == FR_OK)
						write_csr(0x8AA, targetbuffer);
					else
						write_csr(0x8AA, 0x0); // nullptr
				}
				else if (value==29) // ioctl
				{
					// TODO: device io control commands
					errno = EINVAL;
					write_csr(0x8AA, 0xFFFFFFFF);
				}
				else if (value==50) // chdir
				{
					//chdir(const char *path);
					char *path = (char*)read_csr(0x8AA); // A0
					if (path)
					{
						FRESULT chdirattempt = f_chdir(path);
						if (chdirattempt == FR_OK)
							write_csr(0x8AA, 0x0);
						else
							write_csr(0x8AA, 0xFFFFFFFF);
					}
					else
						write_csr(0x8AA, 0xFFFFFFFF);
				}
				else if (value==57) // close()
				{
					// TODO: route to /dev/ if one is open

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
					// TODO: route to /dev/ if one is open

					uint32_t file = read_csr(0x8AA); // A0
					uint32_t ptr = read_csr(0x8AB); // A1
					uint32_t len = read_csr(0x8AC); // A2

					if (file == STDIN_FILENO)
					{
						// TODO: Read one character from console here, but we need to figure out how to do that
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
					// TODO: route to /dev/ if one is open

					uint32_t file = read_csr(0x8AA); // A0
					uint32_t ptr = read_csr(0x8AB); // A1
					uint32_t count = read_csr(0x8AC); // A2

					if (file == STDOUT_FILENO || file == STDERR_FILENO)
					{
						int outcount = kprintfn(count, (const char*)ptr);
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
					_task_exit_current_task(taskctx);
					// Task return value is store in AO
					uint32_t retval = read_csr(0x8AA); // A0
					write_csr(0x8AA, retval);
					_task_abort_to(taskctx, 0);
				}
				else if (value==95) // wait()
				{
					// Wait for child process status change - unused
					// pid_t wait(int *wstatus);
					errno = ECHILD;
					write_csr(0x8AA, 0xFFFFFFFF);
				}
				else if (value==129) // kill(pid_t pid, int sig)
				{
					// Signal process to terminate
					uint32_t pid = read_csr(0x8AA); // A0
					uint32_t sig = read_csr(0x8AB); // A1
					_task_exit_task_with_id(taskctx, pid, sig);
					kdebugprintf("\nSIG:0x%x PID:0x%x\n", sig, pid);
					write_csr(0x8AA, sig);
					_task_abort_to(taskctx, 0);
				}
				else if (value==214) // brk()
				{
					uint32_t addrs = read_csr(0x8AA); // A0
					uint32_t retval = core_brk(addrs);
					write_csr(0x8AA, retval);
					//UARTPrintf("%d:brk(%0X)->%08x\n", hartid, addrs, retval);
				}
				else if (value==403) // gettimeofday()
				{
					uint32_t ptr = read_csr(0x8AA); // A0
					struct timeval *tv = (struct timeval *)ptr;
					uint64_t now = E32ReadTime();
					uint64_t usec = now % ONE_MILLISECOND_IN_TICKS;
					tv->tv_sec = ClockToMs(now) / 1000;
					tv->tv_usec = usec;
					write_csr(0x8AA, 0x0);
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
				else if (value==1038) // _stat()
				{
					uint32_t nptr = read_csr(0x8AA); // A0
					uint32_t ptr = read_csr(0x8AB); // A1
					struct stat *buf = (struct stat *)ptr;

					FILINFO finf;
					FRESULT fr = f_stat((char*)nptr, &finf);

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
				else if (value==16384) // task_add
				{
					struct STaskContext *context = (struct STaskContext *)read_csr(0x8AA); // A0
					const char * name = (const char *)read_csr(0x8AB); // A1
					taskfunc task = (taskfunc)read_csr(0x8AC); // A2
					enum ETaskState initialState = read_csr(0x8AD); // A3
					const uint32_t runLength = read_csr(0x8AE); // A4
					const uint32_t stackPointer = read_csr(0x8AF); // A5
					const uint32_t gp = read_csr(0x8A3); // GP
					uint32_t parentSP = (stackPointer != 0x0) ? stackPointer : read_csr(0x8A2); // SP
					// Using parent's stack pointer as the new task's stack pointer
					int retVal = _task_add(context, name, task, initialState, runLength, hartid, gp, parentSP);
					write_csr(0x8AA, retVal);
				}
				else if (value==16385) // task_switch_to_next
				{
					// TaskSwitchToNext()
					struct STaskContext *context = (struct STaskContext *)read_csr(0x8AA); // A0
					int retVal = _task_switch_to_next(context);
					write_csr(0x8AA, retVal);
				}
				else if (value==16386) // _task_exit_task_with_id
				{
					struct STaskContext *context = (struct STaskContext *)read_csr(0x8AA); // A0
					uint32_t taskid = read_csr(0x8AB); // A1
					int signal = read_csr(0x8AC); // A2
					_task_exit_task_with_id(context, taskid, signal);
					write_csr(0x8AA, 0);
					_task_abort_to(taskctx, 0);
				}
				else if (value==16387) // _task_exit_current_task
				{
					struct STaskContext *context = (struct STaskContext *)read_csr(0x8AA); // A0
					_task_exit_current_task(context);
					write_csr(0x8AA, 0);
					_task_abort_to(taskctx, 0);
				}
				else if (value==16388) // _task_yield
				{
					//TaskYield()
					// Ignore return value, only OS has access to it
					_task_yield();
					write_csr(0x8AA, 0);
				}
				else if (value==16389) // _task_get_context
				{
					uint32_t taskhartid = read_csr(0x8AA); // A0
					struct STaskContext *context = _task_get_context(taskhartid);
					write_csr(0x8AA, (uint32_t)context);
				}
				else if (value==16390) // _task_get_shared_memory
				{
					void* sharedmem = _task_get_shared_memory();
					write_csr(0x8AA, (uint32_t)sharedmem);
				}
				else if (value==16391) // void *sbrk(uint32_t incr)
				{
					uint32_t incr = read_csr(0x8AA); // A0
					uint32_t retval = core_sbrk(incr);
					write_csr(0x8AA, retval);
					//UARTPrintf("%d:sbrk(%d)->%08x\n", hartid, incr, retval);
				}
				else // Unimplemented syscalls drop here
				{
					kdebugprintf("unimplemented ECALL: %d\b", value);
					errno = EIO;
					write_csr(0x8AA, 0xFFFFFFFF);
				}
			}
			break;

			case CAUSE_FETCH_ACCESS:
			case CAUSE_LOAD_ACCESS:
			case CAUSE_STORE_ACCESS:
			{
				kdebugprintf("Memory access fault: %d\b", value);
				errno = EACCES;
				write_csr(0x8AA, 0xFFFFFFFF);
			}
			break;

			case CAUSE_FETCH_PAGE_FAULT:
			case CAUSE_LOAD_PAGE_FAULT:
			case CAUSE_STORE_PAGE_FAULT:
			{
				kdebugprintf("Memory page fault: %d\b", value);
				errno = EFAULT;
				write_csr(0x8AA, 0xFFFFFFFF);
			}

			/*case CAUSE_MISALIGNED_FETCH:
			case CAUSE_MISALIGNED_LOAD:
			case CAUSE_MISALIGNED_STORE:
			case CAUSE_USER_ECALL:
			case CAUSE_SUPERVISOR_ECALL:
			case CAUSE_HYPERVISOR_ECALL:*/
			default:
			{
				taskctx->kernelError = 3;
				taskctx->kernelErrorData[0] = code;	// The unknown cause code
			}
			break;
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

void InstallISR(uint32_t _hartid, bool _allowMachineHwInt, bool _allowMachineSwInt)
{
	// Set machine trap vector
	write_csr(mtvec, interrupt_service_routine);

	// Set up timer interrupt one second into the future
	uint64_t now = E32ReadTime();
	uint64_t future = now + TWO_HUNDRED_FIFTY_MILLISECONDS_IN_TICKS;
	E32SetTimeCompare(future);

	// Enable machine software interrupts (breakpoint/illegal instruction) if this core handles them
	// Enable machine hardware interrupts if this core should handle them
	// Enable machine timer interrupts for all cores
	write_csr(mie, (_allowMachineHwInt ? MIP_MEIP : 0) | (_allowMachineSwInt ? MIP_MSIP : 0) | MIP_MTIP);

	// Allow all machine interrupts to trigger (thus also enabling task system)
	write_csr(mstatus, MSTATUS_MIE);
}
