/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / common tools sub-project
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#include <gpac/tools.h>

#if defined(_WIN32_WCE)

#include <winbase.h>
#include <winsock.h>
#include <tlhelp32.h>

#if !defined(__GNUC__)
#pragma comment(lib, "toolhelp")
#endif

#elif defined(WIN32)

#include <time.h>
#include <sys/timeb.h>
#include <io.h>
#include <windows.h>
#include <tlhelp32.h>

#if !defined(__GNUC__)
#pragma comment(lib, "winmm")
#endif

#else

#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/resource.h>

#ifndef __BEOS__
#include <errno.h>
#endif


#define SLEEP_ABS_SELECT		1

static u32 sys_start_time = 0;
#endif


#ifndef _WIN32_WCE
#include <locale.h>
#endif


#ifndef WIN32
u32 gf_sys_clock()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return ( (now.tv_sec)*1000 + (now.tv_usec) / 1000) - sys_start_time;
}
#endif


void gf_sleep(u32 ms)
{
#ifdef WIN32
	Sleep(ms);
#else
	s32 sel_err;
	struct timeval tv;

#ifndef SLEEP_ABS_SELECT
	u32 prev, now, elapsed;
#endif

#ifdef SLEEP_ABS_SELECT
	tv.tv_sec = ms/1000;
	tv.tv_usec = (ms%1000)*1000;
#else
	prev = gf_sys_clock();
#endif

	do {
		errno = 0;

#ifndef SLEEP_ABS_SELECT
		now = gf_sys_clock();
		elapsed = (now - prev);
		if ( elapsed >= ms ) {
			break;
		}
		prev = now;
		ms -= elapsed;
		tv.tv_sec = ms/1000;
		tv.tv_usec = (ms%1000)*1000;
#endif

		sel_err = select(0, NULL, NULL, NULL, &tv);
	} while ( sel_err && (errno == EINTR) );
#endif
}

#ifndef gettimeofday
#ifdef _WIN32_WCE

/*
	Conversion code for WinCE from pthreads WinCE
	(FILETIME in Win32 is from jan 1, 1601)
	time between jan 1, 1601 and jan 1, 1970 in units of 100 nanoseconds 
*/
#define TIMESPEC_TO_FILETIME_OFFSET (((LONGLONG)27111902 << 32) + (LONGLONG)3577643008)

s32 gettimeofday(struct timeval *tp, void *tz)
{
	FILETIME ft;
	SYSTEMTIME st;
	s32 val;

	GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);

	val = (s32) ((*(LONGLONG *) &ft - TIMESPEC_TO_FILETIME_OFFSET) / 10000000);
	tp->tv_sec = (u32) val;
	val = (s32 ) ((*(LONGLONG *) &ft - TIMESPEC_TO_FILETIME_OFFSET - ((LONGLONG) val * (LONGLONG) 10000000)) * 100);
	tp->tv_usec = val;
	return 0;
}

#elif defined(WIN32)

s32 gettimeofday(struct timeval *tp, void *tz)
{
	struct _timeb timebuffer;   

	_ftime( &timebuffer );
	tp->tv_sec  = (long) (timebuffer.time);
	tp->tv_usec = timebuffer.millitm * 1000;
	return 0;
}
#endif

#endif

#ifdef _WIN32_WCE

void CE_Assert(u32 valid, char *file, u32 line)
{
	if (!valid) {
		char szBuf[2048];
		u16 wcBuf[2048];
		sprintf(szBuf, "File %s : line %d", file, line);
		CE_CharToWide(szBuf, wcBuf);
		MessageBox(NULL, wcBuf, _T("GPAC Assertion Failure"), MB_OK);
		exit(EXIT_FAILURE);
	}
}

void CE_WideToChar(unsigned short *w_str, char *str)
{
	WideCharToMultiByte(CP_ACP, 0, w_str, -1, str, GF_MAX_PATH, NULL, NULL);
}

void CE_CharToWide(char *str, unsigned short *w_str)
{
	MultiByteToWideChar(CP_ACP, 0, str, -1, w_str, GF_MAX_PATH);
}


#endif

GF_Err gf_delete_file(const char *fileName)
{
#if defined(_WIN32_WCE)
	TCHAR swzName[MAX_PATH];
	CE_CharToWide(fileName, swzName);
	return (DeleteFile(swzName)==0) ? GF_IO_ERR : GF_OK;
#elif defined(WIN32)
	/* success if != 0 */
	return (DeleteFile(fileName)==0) ? GF_IO_ERR : GF_OK;
#else
	/* success is == 0 */
	return ( remove(fileName) == 0) ? GF_OK : GF_IO_ERR;
#endif
}

void gf_move_file(const char *fileName, const char *newFileName)
{
#if defined(_WIN32_WCE)
	TCHAR swzName[MAX_PATH];
	TCHAR swzNewName[MAX_PATH];
	CE_CharToWide(fileName, swzName);
	CE_CharToWide(newFileName, swzNewName);
	MoveFile(swzName, swzNewName);
#elif defined(WIN32)
	MoveFile(fileName, newFileName);
#else
	rename(fileName, newFileName);
#endif
}

void gf_rand_init(Bool Reset)
{
	if (Reset) {
		srand(1);
	} else {
#if defined(_WIN32_WCE)
		srand( (u32) GetTickCount() );
#else
		srand( (u32) time(NULL) );
#endif
	}
}

u32 gf_rand()
{
	return rand();
}

#ifndef _WIN32_WCE
#include <sys/stat.h>
#endif

u64 gf_file_modification_time(const char *filename)
{
#if defined(_WIN32_WCE) 
	WCHAR _file[GF_MAX_PATH]; 
	WIN32_FIND_DATA FindData;
	HANDLE fh;
	ULARGE_INTEGER uli;
	ULONGLONG time_ms;
	CE_CharToWide((char *) filename, _file);
	fh = FindFirstFile(_file, &FindData);
	if (fh == INVALID_HANDLE_VALUE) return 0;
	uli.LowPart = FindData.ftLastWriteTime.dwLowDateTime;
	uli.HighPart = FindData.ftLastWriteTime.dwHighDateTime;
	FindClose(fh);
	time_ms = uli.QuadPart/10000;
	return time_ms;
#elif defined(WIN32) && !defined(__GNUC__)
	struct _stat64 sb;
	if (_stat64(filename, &sb) != 0) return 0;
	return sb.st_mtime;
#else
	struct stat sb;
	if (stat(filename, &sb) != 0) return 0;
	return sb.st_mtime;
#endif
	return 0;
}

FILE *gf_temp_file_new()
{
#if defined(_WIN32_WCE)
	TCHAR pPath[MAX_PATH+1];
	TCHAR pTemp[MAX_PATH+1];
	if (!GetTempPath(MAX_PATH, pPath)) {
		pPath[0] = '.';
		pPath[1] = '.';
	}
	if (GetTempFileName(pPath, TEXT("git"), 0, pTemp))
		return _wfopen(pTemp, TEXT("w+b"));

	return NULL;
#elif defined(WIN32)
	char tmp[MAX_PATH], t_file[100];
	FILE *res = tmpfile();
	if (res) return res;
	/*tmpfile() may fail under vista ...*/
	if (!GetEnvironmentVariable("TEMP",tmp,MAX_PATH)) return NULL;
	sprintf(t_file, "\\gpac_%08x.tmp", (u32) tmp);
	strcat(tmp, t_file);
	return gf_f64_open(tmp, "w+b");
#else
	return tmpfile(); 
#endif
}


void gf_utc_time_since_1970(u32 *sec, u32 *msec)
{
#if defined (WIN32) && !defined(_WIN32_WCE)
	struct _timeb	tb;
	_ftime( &tb );
	*sec = (u32) tb.time;
	*msec = tb.millitm;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	*sec = tv.tv_sec;
	*msec = tv.tv_usec/1000;
#endif
}

void gf_get_user_name(char *buf, u32 buf_size)
{
	strcpy(buf, "mpeg4-user");

#if 0
	s32 len;
	char *t;
	strcpy(buf, "");
	len = 1024;
	GetUserName(buf, &len);
	if (!len) {
		t = getenv("USER");
		if (t) strcpy(buf, t);
	}
#endif
#if 0
	struct passwd *pw;
	pw = getpwuid(getuid());
	strcpy(buf, "");
	if (pw && pw->pw_name) strcpy(name, pw->pw_name);
#endif
}


/*enumerate directories*/
GF_Err gf_enum_directory(const char *dir, Bool enum_directory, gf_enum_dir_item enum_dir_fct, void *cbck, const char *filter)
{
	char item_path[GF_MAX_PATH];

#if defined(_WIN32_WCE)
	char _path[GF_MAX_PATH];
	unsigned short path[GF_MAX_PATH];
	unsigned short w_filter[GF_MAX_PATH];
	char file[GF_MAX_PATH];
#else
	char path[GF_MAX_PATH], *file;
#endif

#ifdef WIN32
	WIN32_FIND_DATA FindData;
	HANDLE SearchH;
#else	
	DIR *the_dir;
	struct dirent* the_file;
	struct stat st;
#endif

	if (!dir || !enum_dir_fct) return GF_BAD_PARAM;

	if (filter && (!strcmp(filter, "*") || !filter[0])) filter=NULL;

	if (!strcmp(dir, "/")) {
#if defined(WIN32) && !defined(_WIN32_WCE)
		u32 len;
		char *drives, *volume;
		len = GetLogicalDriveStrings(0, NULL);
		drives = gf_malloc(sizeof(char)*(len+1));
		drives[0]=0;
		GetLogicalDriveStrings(len, drives);
		len = strlen(drives);
		volume = drives;
		while (len) {
			enum_dir_fct(cbck, volume, "");
			volume += len+1;
			len = strlen(volume);
		}
		gf_free(drives);
		return GF_OK;
#elif defined(__SYMBIAN32__)
		RFs iFs;
		TDriveList aList;
		iFs.Connect();
		iFs.DriveList(aList);
		for (TInt i=0;i<KMaxDrives;i++) {
			if (aList[i]) {
				char szDrive[10];
				TChar aDrive;
				iFs.DriveToChar(i, aDrive);
				sprintf(szDrive, "%c:", (TUint)aDrive);
				enum_dir_fct(cbck, szDrive, "");
			}
		}
		iFs.Close();
		FlushItemList();
		return GF_OK;
#endif
	}


#if defined (_WIN32_WCE)
	switch (dir[strlen(dir) - 1]) {
	case '/':
	case '\\':
		sprintf(_path, "%s*", dir);
		break;
	default:
		sprintf(_path, "%s%c*", dir, GF_PATH_SEPARATOR);
		break;
	}
	CE_CharToWide(_path, path);
	CE_CharToWide((char *)filter, w_filter);
#elif defined(WIN32)
	switch (dir[strlen(dir) - 1]) {
	case '/':
	case '\\':
		sprintf(path, "%s*", dir);
		break;
	default:
		sprintf(path, "%s%c*", dir, GF_PATH_SEPARATOR);
		break;
	}
#else
	strcpy(path, dir);
	if (path[strlen(path)-1] != '/') strcat(path, "/");
#endif

#ifdef WIN32
	SearchH= FindFirstFile(path, &FindData);
	if (SearchH == INVALID_HANDLE_VALUE) return GF_IO_ERR;

#if defined (_WIN32_WCE)
	_path[strlen(_path)-1] = 0;
#else
	path[strlen(path)-1] = 0;
#endif

	while (SearchH != INVALID_HANDLE_VALUE) {

#else

	the_dir = opendir(path);
	if (the_dir == NULL) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[Core] Cannot open directory %s for enumeration\n", path));
		return GF_IO_ERR;
	}
	the_file = readdir(the_dir);
	while (the_file) {

#endif

#if defined (_WIN32_WCE)
		if (!wcscmp(FindData.cFileName, _T(".") )) goto next;
		if (!wcscmp(FindData.cFileName, _T("..") )) goto next;
#elif defined(WIN32)
		if (!strcmp(FindData.cFileName, ".")) goto next;
		if (!strcmp(FindData.cFileName, "..")) goto next;
#else
		if (!strcmp(the_file->d_name, "..")) goto next;
		if (the_file->d_name[0] == '.') goto next;
#endif

#ifdef WIN32
		if (!enum_directory && (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) goto next;
		if (enum_directory && !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) goto next;
#endif

		if (filter) {
#if defined (_WIN32_WCE)
			short ext[30];
			short *sep = wcsrchr(FindData.cFileName, (wchar_t) '.');
			if (!sep) goto next;
			wcscpy(ext, sep+1);
			wcslwr(ext);
			if (!wcsstr(w_filter, ext)) goto next;
#elif defined(WIN32)
			char ext[30];
			char *sep = strrchr(FindData.cFileName, '.');
			if (!sep) goto next;
			strcpy(ext, sep+1);
			strlwr(ext);
			if (!strstr(filter, ext)) goto next;
#else
			char ext[30];
			char *sep = strrchr(the_file->d_name, '.');
			if (!sep) goto next;
			strcpy(ext, sep+1);
			strlwr(ext);
			if (!strstr(filter, sep+1)) goto next;
#endif
		}

#if defined (_WIN32_WCE)
		CE_WideToChar(FindData.cFileName, file);
		strcpy(item_path, _path);
		strcat(item_path, file);
#elif defined(WIN32)
		strcpy(item_path, path);
		strcat(item_path, FindData.cFileName);
		file = FindData.cFileName;
#else
		strcpy(item_path, path);
		strcat(item_path, the_file->d_name);
	GF_LOG(GF_LOG_DEBUG, GF_LOG_CORE, ("[Core] Checking file %s for enum\n", item_path));
		
		if (stat( item_path, &st ) != 0) goto next;
		if (enum_directory && ( (st.st_mode & S_IFMT) != S_IFDIR)) goto next;
		if (!enum_directory && ((st.st_mode & S_IFMT) == S_IFDIR)) goto next;
		file = the_file->d_name;
#endif
		if (enum_dir_fct(cbck, file, item_path)) {
#ifdef WIN32
			FindClose(SearchH);
#endif
			break;
		}

next:
#ifdef WIN32
		if (!FindNextFile(SearchH, &FindData)) {
			FindClose(SearchH);
			break;
		}
#else
		the_file = readdir(the_dir);
#endif
	}
#ifndef WIN32
	closedir(the_dir);
#endif
	return GF_OK;
}


#ifndef WIN32
char * my_str_upr(char *str)
{
	u32 i;
	for (i=0; i<strlen(str); i++) {
		str[i] = toupper(str[i]);
	}
	return str;
}
char * my_str_lwr(char *str)
{
	u32 i;
	for (i=0; i<strlen(str); i++) {
		str[i] = tolower(str[i]);
	}
	return str;
}
#endif

u64 gf_f64_tell(FILE *fp)
{
#if defined(_WIN32_WCE)
	return (u64) ftell(fp);
#elif defined(GPAC_CONFIG_WIN32)	/* mingw or cygwin */
	return (u64) ftell(fp);
#elif defined(WIN32)
	return (u64) _ftelli64(fp);
#elif defined(GPAC_CONFIG_LINUX) && !defined(GPAC_ANDROID)
	return (u64) ftello64(fp);
#elif (defined(GPAC_CONFIG_FREEBSD) || defined(GPAC_CONFIG_DARWIN))
	return (u64) ftello(fp);
#else
	return (u64) ftell(fp);
#endif
}

u64 gf_f64_seek(FILE *fp, s64 offset, s32 whence)
{
#if defined(_WIN32_WCE)
	return (u64) fseek(fp, (s32) offset, whence);
#elif defined(GPAC_CONFIG_WIN32)	/* mingw or cygwin */
	return (u64) fseek(fp, (s32) offset, whence);
#elif defined(WIN32)
	return (u64) _fseeki64(fp, offset, whence);
#elif defined(GPAC_CONFIG_LINUX) && !defined(GPAC_ANDROID)
	return fseeko64(fp, (off64_t) offset, whence);
#elif (defined(GPAC_CONFIG_FREEBSD) || defined(GPAC_CONFIG_DARWIN))
	return fseeko(fp, (off_t) offset, whence);
#else
	return fseek(fp, (s32) offset, whence);
#endif
}

FILE *gf_f64_open(const char *file_name, const char *mode)
{
#if defined(WIN32)
	return fopen(file_name, mode);
#elif defined(GPAC_CONFIG_LINUX) && !defined(GPAC_ANDROID)
	return fopen64(file_name, mode);
#elif (defined(GPAC_CONFIG_FREEBSD) || defined(GPAC_CONFIG_DARWIN))
	return fopen(file_name, mode);
#else
	return fopen(file_name, mode);
#endif
}



/*seems OK under mingw also*/
#ifdef WIN32
#ifdef _WIN32_WCE

Bool gf_prompt_has_input()
{
	return 0;
}
char gf_prompt_get_char() { return 0; }
void gf_prompt_set_echo_off(Bool echo_off) { return; }

#else

#include <conio.h>
#include <windows.h>

Bool gf_prompt_has_input()
{
	return kbhit();
}
char gf_prompt_get_char()
{
	return getchar();
}
void gf_prompt_set_echo_off(Bool echo_off) 
{
	DWORD flags;
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(hStdin, &flags);
	if (echo_off) flags &= ~ENABLE_ECHO_INPUT;
	else flags |= ENABLE_ECHO_INPUT;
	SetConsoleMode(hStdin, flags);
}
#endif
#else
/*linux kbhit/getchar- borrowed on debian mailing lists, (author Mike Brownlow)*/
#include <termios.h>

static struct termios t_orig, t_new;
static s32 ch_peek = -1;

static void init_keyboard()
{
	tcgetattr(0, &t_orig);
	t_new = t_orig;
	t_new.c_lflag &= ~ICANON;
	t_new.c_lflag &= ~ECHO;
	t_new.c_lflag &= ~ISIG;
	t_new.c_cc[VMIN] = 1;
	t_new.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &t_new);
}
static void close_keyboard(Bool new_line)
{
	tcsetattr(0,TCSANOW, &t_orig);
	if (new_line) fprintf(stdout, "\n");
}

void gf_prompt_set_echo_off(Bool echo_off) 
{ 
	init_keyboard();
	if (echo_off) t_orig.c_lflag &= ~ECHO;
	else t_orig.c_lflag |= ECHO;
	close_keyboard(0);
}

Bool gf_prompt_has_input()
{
	u8 ch;
	s32 nread;

	init_keyboard();
	if (ch_peek != -1) return 1;
	t_new.c_cc[VMIN]=0;
	tcsetattr(0, TCSANOW, &t_new);
	nread = read(0, &ch, 1);
	t_new.c_cc[VMIN]=1;
	tcsetattr(0, TCSANOW, &t_new);
	if(nread == 1) {
		ch_peek = ch;
		return 1;
	}
	close_keyboard(0);
	return 0;
}

char gf_prompt_get_char()
{
	char ch;
	if (ch_peek != -1) {
		ch = ch_peek;
		ch_peek = -1;
		close_keyboard(1);
		return ch;
	}
	if (0==read(0,&ch,1))
		ch = 0;
	close_keyboard(1);
	return ch;
}

#endif




static u32 sys_init = 0;
static u32 last_update_time = 0;
static u64 last_process_k_u_time = 0;
GF_SystemRTInfo the_rti;


#if defined(_WIN32_WCE)
static LARGE_INTEGER frequency , init_counter;
static u64 last_total_k_u_time = 0;
static u32 mem_usage_at_startup = 0;


#ifndef GetCurrentPermissions
DWORD GetCurrentPermissions();
#endif
#ifndef SetProcPermissions
void SetProcPermissions(DWORD );
#endif

#elif defined(WIN32)
static LARGE_INTEGER frequency , init_counter;
static u64 last_proc_idle_time = 0;
static u64 last_proc_k_u_time = 0;

static HINSTANCE psapi_hinst = NULL;
typedef BOOL(WINAPI* NTGetSystemTimes)(VOID *,VOID *,VOID *);
NTGetSystemTimes MyGetSystemTimes = NULL;
typedef BOOL(WINAPI* NTGetProcessMemoryInfo)(HANDLE,VOID *,DWORD);
NTGetProcessMemoryInfo MyGetProcessMemoryInfo = NULL;
typedef int(WINAPI* NTQuerySystemInfo)(ULONG,PVOID,ULONG,PULONG);
NTQuerySystemInfo MyQuerySystemInfo = NULL;

#ifndef PROCESS_MEMORY_COUNTERS
typedef struct _PROCESS_MEMORY_COUNTERS 
{  
	DWORD cb;
	DWORD PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS;
#endif

#ifndef SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION 
{
	LARGE_INTEGER IdleTime;
	LARGE_INTEGER KernelTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER Reserved1[2];
	ULONG Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;
#endif


#else
	
static u64 last_cpu_u_k_time = 0;
static u64 last_cpu_idle_time = 0;
static u64 mem_at_startup = 0;

#endif

#ifdef WIN32
static u32 (*OS_GetSysClock)();

u32 gf_sys_clock()
{
	return OS_GetSysClock();
}
#endif


#ifdef WIN32

static u32 OS_GetSysClockHIGHRES()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	now.QuadPart -= init_counter.QuadPart;
	return (u32) ((now.QuadPart * 1000) / frequency.QuadPart);
}

static u32 OS_GetSysClockNORMAL() 
{ 
#ifdef _WIN32_WCE
	return GetTickCount();
#else
	return timeGetTime();
#endif
}

#endif /* WIN32 */

#if defined(__sh__) 
/* Avoid exception for denormalized floating point values */
static int
sh4_get_fpscr()
{
   int ret;
   asm volatile ("sts fpscr,%0" : "=r" (ret));
   return ret;
}

static void
sh4_put_fpscr(int nv)
{
    asm volatile ("lds %0,fpscr" : : "r" (nv));
}

#define SH4_FPSCR_FR 0x00200000
#define SH4_FPSCR_SZ 0x00100000
#define SH4_FPSCR_PR 0x00080000
#define SH4_FPSCR_DN 0x00040000
#define SH4_FPSCR_RN 0x00000003
#define SH4_FPSCR_RN_N 0
#define SH4_FPSCR_RN_Z 1

extern int __fpscr_values[2];

void
sh4_change_fpscr(int off, int on)
{
   int b = sh4_get_fpscr();
   off = ~off;
   off |=   0x00180000;
   on  &= ~ 0x00180000;
   b &= off;
   b |= on;
   sh4_put_fpscr(b);
   __fpscr_values[0] &= off;
   __fpscr_values[0] |= on;
   __fpscr_values[1] &= off;
   __fpscr_values[1] |= on;
}

#endif 

#ifdef GPAC_MEMORY_TRACKING
void gf_mem_enable_tracker();
#endif

void gf_sys_init(Bool enable_memory_tracker)
{
	if (!sys_init) {
#if defined(_WIN32_WCE)
		MEMORYSTATUS ms;
#endif

		if (enable_memory_tracker) {
#ifdef GPAC_MEMORY_TRACKING
			gf_mem_enable_tracker();
#endif
		}

#if defined(__sh__)
		/* Round all denormalized floatting point number to 0.0 */
		sh4_change_fpscr(0,SH4_FPSCR_DN) ;
#endif

#if defined(WIN32)
		frequency.QuadPart = 0;
		/*clock setup*/
		if (QueryPerformanceFrequency(&frequency)) {
			QueryPerformanceCounter(&init_counter);
			OS_GetSysClock = OS_GetSysClockHIGHRES;
			GF_LOG(GF_LOG_INFO, GF_LOG_CORE, ("[core] using WIN32 performance timer\n"));
		} else {
			OS_GetSysClock = OS_GetSysClockNORMAL;
			GF_LOG(GF_LOG_INFO, GF_LOG_CORE, ("[core] using WIN32 regular timer\n"));
		}

#ifndef _WIN32_WCE
		timeBeginPeriod(1);
#endif

		GF_LOG(GF_LOG_INFO, GF_LOG_CORE, ("[core] checking for run-time info tools"));
#if defined(_WIN32_WCE)
		last_total_k_u_time = last_process_k_u_time = 0;
		last_update_time = 0;
		memset(&the_rti, 0, sizeof(GF_SystemRTInfo));
		the_rti.pid = GetCurrentProcessId();
		GlobalMemoryStatus(&ms);
		mem_usage_at_startup = ms.dwAvailPhys;
#else
		/*cpu usage tools are buried in win32 dlls...*/
		MyGetSystemTimes = (NTGetSystemTimes) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetSystemTimes");
		if (!MyGetSystemTimes) {
			MyQuerySystemInfo = (NTQuerySystemInfo) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtQuerySystemInformation");
			if (MyQuerySystemInfo) {
				GF_LOG(GF_LOG_INFO, GF_LOG_CORE, (" - CPU: QuerySystemInformation"));
			}
		} else {
			GF_LOG(GF_LOG_INFO, GF_LOG_CORE, (" - CPU: GetSystemsTimes"));
		}
		psapi_hinst = LoadLibrary("psapi.dll");
		MyGetProcessMemoryInfo = (NTGetProcessMemoryInfo) GetProcAddress(psapi_hinst, "GetProcessMemoryInfo");
		if (MyGetProcessMemoryInfo) {
			GF_LOG(GF_LOG_INFO, GF_LOG_CORE, (" - memory: GetProcessMemoryInfo"));
		}
		last_process_k_u_time = last_proc_idle_time = last_proc_k_u_time = 0;
		last_update_time = 0;
		memset(&the_rti, 0, sizeof(GF_SystemRTInfo));
		the_rti.pid = GetCurrentProcessId();
#endif
		GF_LOG(GF_LOG_INFO, GF_LOG_CORE, ("\n"));

#else
		/*linux threads...*/
		last_process_k_u_time = 0;
		last_cpu_u_k_time = last_cpu_idle_time = 0;
		last_update_time = 0;
		memset(&the_rti, 0, sizeof(GF_SystemRTInfo));
		the_rti.pid = getpid();
		sys_start_time = gf_sys_clock();
#endif
		GF_LOG(GF_LOG_INFO, GF_LOG_CORE, ("[core] process id %d\n", the_rti.pid));

#ifndef _WIN32_WCE
		setlocale( LC_NUMERIC, "C" ); 
#endif
	}
	sys_init += 1;
}

void gf_sys_close()
{
	if (sys_init > 0) {
		sys_init --;
		if (sys_init) return;
		/*prevent any call*/
		last_update_time = 0xFFFFFFFF;

#if defined(WIN32) && !defined(_WIN32_WCE)
		timeEndPeriod(1);
		
		MyGetSystemTimes = NULL;
		MyGetProcessMemoryInfo = NULL;
		MyQuerySystemInfo = NULL;
		if (psapi_hinst) FreeLibrary(psapi_hinst);
		psapi_hinst = NULL;
#endif
	}
}

#ifdef GPAC_MEMORY_TRACKING
extern size_t gpac_allocated_memory;
extern size_t gpac_nb_alloc_blocs;
#endif

/*CPU and Memory Usage*/

#ifdef WIN32

Bool gf_sys_get_rti(u32 refresh_time_ms, GF_SystemRTInfo *rti, u32 flags)
{
#if defined(_WIN32_WCE)
	THREADENTRY32 tentry;
	u64 total_cpu_time, process_cpu_time;
	DWORD orig_perm;	
#endif
	MEMORYSTATUS ms;
	u64 creation, exit, kernel, user, process_k_u_time, proc_idle_time, proc_k_u_time;
	u32 nb_threads, entry_time;
	HANDLE hSnapShot;

	assert(sys_init);

	if (!rti) return 0;

	proc_idle_time = proc_k_u_time = process_k_u_time = 0;
	nb_threads = 0;

	entry_time = gf_sys_clock();
	if (last_update_time && (entry_time - last_update_time < refresh_time_ms)) {
		memcpy(rti, &the_rti, sizeof(GF_SystemRTInfo));
		return 0;
	}

	if (flags & GF_RTI_SYSTEM_MEMORY_ONLY) {
		memset(rti, 0, sizeof(GF_SystemRTInfo));
		rti->sampling_instant = last_update_time;
		GlobalMemoryStatus(&ms);
		rti->physical_memory = ms.dwTotalPhys;
		rti->physical_memory_avail = ms.dwAvailPhys;
#ifdef GPAC_MEMORY_TRACKING
		rti->gpac_memory = (u64) gpac_allocated_memory;
#endif
		return 1;
	}

#if defined (_WIN32_WCE)

	total_cpu_time = process_cpu_time = 0;

	/*get a snapshot of all running threads*/
	orig_perm = GetCurrentPermissions();
	SetProcPermissions(0xFFFFFFFF);
	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
	if (hSnapShot) {
		tentry.dwSize = sizeof(THREADENTRY32); 
		the_rti.thread_count = 0;
		/*note we always act as if GF_RTI_ALL_PROCESSES_TIMES flag is set, since there is no other way
		to enumerate threads from a process, and GetProcessTimes doesn't exist on CE*/
		if (Thread32First(hSnapShot, &tentry)) {
			do {
				/*get thread times*/
				if (GetThreadTimes( (HANDLE) tentry.th32ThreadID, (FILETIME *) &creation, (FILETIME *) &exit, (FILETIME *) &kernel, (FILETIME *) &user)) {
					total_cpu_time += user + kernel;
					if (tentry.th32OwnerProcessID==the_rti.pid) {
						process_cpu_time += user + kernel;
						the_rti.thread_count ++;
					}
				}
			} while (Thread32Next(hSnapShot, &tentry));
		}
		CloseToolhelp32Snapshot(hSnapShot); 
	}

	if (flags & GF_RTI_PROCESS_MEMORY) {
		HEAPLIST32 hlentry;
		HEAPENTRY32 hentry;
		the_rti.process_memory = 0;
		hlentry.dwSize = sizeof(HEAPLIST32); 
		hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, the_rti.pid); 
		if (hSnapShot && Heap32ListFirst(hSnapShot, &hlentry)) {
			do {
				hentry.dwSize = sizeof(hentry);
				if (Heap32First(hSnapShot, &hentry, hlentry.th32ProcessID, hlentry.th32HeapID)) {
					do {
						the_rti.process_memory += hentry.dwBlockSize;
					} while (Heap32Next(hSnapShot, &hentry));
				}
			} while (Heap32ListNext(hSnapShot, &hlentry));
		}
		CloseToolhelp32Snapshot(hSnapShot); 
	}
	SetProcPermissions(orig_perm);
	total_cpu_time /= 10;
	process_cpu_time /= 10;

#else
	/*XP-SP1 and Win2003 servers only have GetSystemTimes support. This will give a better estimation
	of CPU usage since we can take into account the idle time*/
	if (MyGetSystemTimes) {
		u64 u_time;
		MyGetSystemTimes(&proc_idle_time, &proc_k_u_time, &u_time);
		proc_k_u_time += u_time;
		proc_idle_time /= 10;
		proc_k_u_time /= 10;
	}
	/*same rq for NtQuerySystemInformation*/
	else if (MyQuerySystemInfo) {
		DWORD ret;
		SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION info;
		MyQuerySystemInfo(0x8 /*SystemProcessorPerformanceInformation*/, &info, sizeof(info), &ret); 
		if (ret && (ret<=sizeof(info))) {
			proc_idle_time = info.IdleTime.QuadPart / 10;
			proc_k_u_time = (info.KernelTime.QuadPart + info.UserTime.QuadPart) / 10;
		}
	}
	/*no special API available, ONLY FETCH TIMES if requested (may eat up some time)*/
	else if (flags & GF_RTI_ALL_PROCESSES_TIMES) {
	    PROCESSENTRY32 pentry; 
		/*get a snapshot of all running threads*/
		hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
		if (!hSnapShot) return 0;
		pentry.dwSize = sizeof(PROCESSENTRY32); 
		if (Process32First(hSnapShot, &pentry)) {
			do {
				HANDLE procH = NULL;
				if (pentry.th32ProcessID) procH = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pentry.th32ProcessID);
				if (procH && GetProcessTimes(procH, (FILETIME *) &creation, (FILETIME *) &exit, (FILETIME *) &kernel, (FILETIME *) &user) ) {
					user += kernel;
					proc_k_u_time += user;
					if (pentry.th32ProcessID==the_rti.pid) {
						process_k_u_time = user;
						nb_threads = pentry.cntThreads;
					}
				}
				if (procH) CloseHandle(procH);
			} while (Process32Next(hSnapShot, &pentry));
		}
		CloseHandle(hSnapShot); 
		proc_k_u_time /= 10;
	} 


	if (!process_k_u_time) {
		HANDLE procH = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, the_rti.pid);
		if (procH && GetProcessTimes(procH, (FILETIME *) &creation, (FILETIME *) &exit, (FILETIME *) &kernel, (FILETIME *) &user) ) {
			process_k_u_time = user + kernel;
		}
		if (procH) CloseHandle(procH);
		if (!process_k_u_time) return 0;
	}
	process_k_u_time /= 10;

	/*this won't cost a lot*/
	if (MyGetProcessMemoryInfo) {
		PROCESS_MEMORY_COUNTERS pmc;
		HANDLE procH = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, the_rti.pid);
		MyGetProcessMemoryInfo(procH, &pmc, sizeof (pmc));
		the_rti.process_memory = pmc.WorkingSetSize;
		if (procH) CloseHandle(procH);
	}
	/*THIS IS VERY HEAVY (eats up mem and time) - only perform if requested*/
	else if (flags & GF_RTI_PROCESS_MEMORY) {
		HEAPLIST32 hlentry;
		HEAPENTRY32 hentry;
		the_rti.process_memory = 0;
		hlentry.dwSize = sizeof(HEAPLIST32); 
		hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, the_rti.pid); 
		if (hSnapShot && Heap32ListFirst(hSnapShot, &hlentry)) {
			do {
				hentry.dwSize = sizeof(hentry);
				if (Heap32First(&hentry, hlentry.th32ProcessID, hlentry.th32HeapID)) {
					do {
						the_rti.process_memory += hentry.dwBlockSize;
					} while (Heap32Next(&hentry));
				}
			} while (Heap32ListNext(hSnapShot, &hlentry));
		}
		CloseHandle(hSnapShot); 
	}
#endif

	the_rti.sampling_instant = last_update_time;

	if (last_update_time) {
		the_rti.sampling_period_duration = entry_time - last_update_time;
		the_rti.process_cpu_time_diff = (u32) ((process_k_u_time - last_process_k_u_time)/1000);

#if defined(_WIN32_WCE)
		the_rti.total_cpu_time_diff = (u32) ((total_cpu_time - last_total_k_u_time)/1000);
		/*we're not that accurate....*/
		if (the_rti.total_cpu_time_diff > the_rti.sampling_period_duration) 
			the_rti.sampling_period_duration = the_rti.total_cpu_time_diff;
	
		/*rough values*/
		the_rti.cpu_idle_time = the_rti.sampling_period_duration - the_rti.total_cpu_time_diff;
		the_rti.total_cpu_usage = (u32) (100 * the_rti.total_cpu_time_diff / the_rti.sampling_period_duration);
		the_rti.process_cpu_usage = (u32) (100*the_rti.process_cpu_time_diff / (the_rti.total_cpu_time_diff + the_rti.cpu_idle_time) );

#else
		/*oops, we have no choice but to assume 100% cpu usage during this period*/
		if (!proc_k_u_time) {
			the_rti.total_cpu_time_diff = the_rti.sampling_period_duration;
			proc_k_u_time = last_proc_k_u_time + the_rti.sampling_period_duration;
			the_rti.cpu_idle_time = 0;
			the_rti.total_cpu_usage = 100;
			if (the_rti.sampling_period_duration)
				the_rti.process_cpu_usage = (u32) (100*the_rti.process_cpu_time_diff / the_rti.sampling_period_duration);
		} else {
			u64 samp_sys_time, idle;
			the_rti.total_cpu_time_diff = (u32) ((proc_k_u_time - last_proc_k_u_time)/1000);

			/*we're not that accurate....*/
			if (the_rti.total_cpu_time_diff > the_rti.sampling_period_duration) {
				the_rti.sampling_period_duration = the_rti.total_cpu_time_diff;
			}
			
			if (!proc_idle_time) 
				proc_idle_time = last_proc_idle_time + (the_rti.sampling_period_duration - the_rti.total_cpu_time_diff); 

			samp_sys_time = proc_k_u_time - last_proc_k_u_time;
			idle = proc_idle_time - last_proc_idle_time;
			the_rti.cpu_idle_time = (u32) (idle/1000);
			if (samp_sys_time) {
				the_rti.total_cpu_usage = (u32) ( (samp_sys_time - idle) / (samp_sys_time / 100) );
				the_rti.process_cpu_usage = (u32) (100*the_rti.process_cpu_time_diff / (samp_sys_time/1000));
			}
		}
#endif
	}
	last_update_time = entry_time;
	last_process_k_u_time = process_k_u_time;

	GlobalMemoryStatus(&ms);
	the_rti.physical_memory = ms.dwTotalPhys;
#ifdef GPAC_MEMORY_TRACKING
	the_rti.gpac_memory = (u64) gpac_allocated_memory;
#endif
	the_rti.physical_memory_avail = ms.dwAvailPhys;

#if defined(_WIN32_WCE)	
	last_total_k_u_time = total_cpu_time;
	if (!the_rti.process_memory) the_rti.process_memory = mem_usage_at_startup - ms.dwAvailPhys;
#else
	last_proc_idle_time = proc_idle_time;
	last_proc_k_u_time = proc_k_u_time;
#endif

	if (!the_rti.gpac_memory) the_rti.gpac_memory = the_rti.process_memory;

	memcpy(rti, &the_rti, sizeof(GF_SystemRTInfo));
	return 1;
}

	
#elif defined(GPAC_CONFIG_DARWIN) && !defined(GPAC_IPHONE)

#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <mach/mach_port.h>
#include <mach/mach_traps.h>
#include <mach/task_info.h>
#include <mach/thread_info.h>
#include <mach/thread_act.h>
#include <mach/vm_region.h>
#include <mach/vm_map.h>
#include <mach/task.h>
#include <mach/shared_memory_server.h>
	
static u64 total_physical_memory = 0;
	
Bool gf_sys_get_rti(u32 refresh_time_ms, GF_SystemRTInfo *rti, u32 flags)
{
	size_t length;
	u32 entry_time, i, percent;
    int mib[6];
    int result;
    int pagesize;
	u64 process_u_k_time;
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;	
    vm_statistics_data_t vmstat;
    task_t task;
    kern_return_t error;
    thread_array_t thread_table;
    thread_basic_info_t thi;
    thread_basic_info_data_t thi_data;
    unsigned table_size;
    struct task_basic_info ti;
	double utime, stime;
	
	entry_time = gf_sys_clock();
	if (last_update_time && (entry_time - last_update_time < refresh_time_ms)) {
		memcpy(rti, &the_rti, sizeof(GF_SystemRTInfo));
		return 0;
	}
	
	mib[0] = CTL_HW;
    mib[1] = HW_PAGESIZE;
    length = sizeof(pagesize);
    if (sysctl(mib, 2, &pagesize, &length, NULL, 0) < 0) {
		return 0;
    }

	if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) != KERN_SUCCESS) {
		return 0;
    }

	the_rti.physical_memory = (vmstat.wire_count + vmstat.active_count + vmstat.inactive_count + vmstat.free_count)* pagesize;
	the_rti.physical_memory_avail = vmstat.free_count * pagesize;

	if (!total_physical_memory) {
		mib[0] = CTL_HW;
		mib[1] = HW_PHYSMEM;
		length = sizeof(result);
		if (sysctl(mib, 2, &result, &length, NULL, 0) >= 0) {
			total_physical_memory = result;
		}
	}
	
	
    error = task_for_pid(mach_task_self(), the_rti.pid, &task);
 	if (error) {
		return 0;
	}
	
	percent = 0;
	utime = ti.user_time.seconds + ti.user_time.microseconds * 1e-6;
	stime = ti.system_time.seconds + ti.system_time.microseconds * 1e-6;
	error = task_threads(task, &thread_table, &table_size);
	assert(error == KERN_SUCCESS);
	thi = &thi_data;
	for (i = 0; i != table_size; ++i) {
		count = THREAD_BASIC_INFO_COUNT;
		error = thread_info(thread_table[i], THREAD_BASIC_INFO, (thread_info_t)thi, &count);
		assert(error == KERN_SUCCESS);
		if ((thi->flags & TH_FLAGS_IDLE) == 0) {
			utime += thi->user_time.seconds + thi->user_time.microseconds * 1e-6;
			stime += thi->system_time.seconds + thi->system_time.microseconds * 1e-6;
			percent +=  (u32) (100 * (double)thi->cpu_usage / TH_USAGE_SCALE);
		}
	}
	error = vm_deallocate(mach_task_self(), (vm_offset_t)thread_table, table_size * sizeof(thread_array_t));
    mach_port_deallocate(mach_task_self(), task);
	
	process_u_k_time = utime + stime;
	
	the_rti.sampling_instant = last_update_time;
	
	if (last_update_time) {
		the_rti.sampling_period_duration = (entry_time - last_update_time);
		the_rti.process_cpu_time_diff = (process_u_k_time - last_process_k_u_time) * 10;
		
		the_rti.total_cpu_time_diff = the_rti.sampling_period_duration;
		/*TODO*/
		the_rti.cpu_idle_time = 0;
		the_rti.total_cpu_usage = 0;
		if (!the_rti.process_cpu_time_diff) the_rti.process_cpu_time_diff = the_rti.total_cpu_time_diff;

		the_rti.process_cpu_usage = percent;
	} else {
		mem_at_startup = the_rti.physical_memory_avail;
	}
	the_rti.process_memory = mem_at_startup - the_rti.physical_memory_avail;

#ifdef GPAC_MEMORY_TRACKING
	the_rti.gpac_memory = gpac_allocated_memory;
#endif
	
	last_process_k_u_time = process_u_k_time;
	last_cpu_idle_time = 0;
	last_update_time = entry_time;
	memcpy(rti, &the_rti, sizeof(GF_SystemRTInfo));
	return 1;
}

//linux
#else

Bool gf_sys_get_rti(u32 refresh_time_ms, GF_SystemRTInfo *rti, u32 flags)
{
  u32 entry_time;
  u64 process_u_k_time;
  u32 u_k_time, idle_time;
#if 0
  char szProc[100];
#endif
  char line[2048];
  FILE *f;

  assert(sys_init);

  entry_time = gf_sys_clock();
  if (last_update_time && (entry_time - last_update_time < refresh_time_ms)) {
    memcpy(rti, &the_rti, sizeof(GF_SystemRTInfo));
    return 0;
  }

  u_k_time = idle_time = 0;
  f = gf_f64_open("/proc/stat", "r");
  if (f) {
    u32 k_time, nice_time, u_time;
    if (fgets(line, 128, f) != NULL) {
      if (sscanf(line, "cpu  %u %u %u %u\n", &u_time, &k_time, &nice_time, &idle_time) == 4) {
	u_k_time = u_time + k_time + nice_time;
      }
    }
    fclose(f);
  }

  process_u_k_time = 0;
  the_rti.process_memory = 0;

  /*FIXME? under LinuxThreads this will only fetch stats for the calling thread, we would have to enumerate /proc to get
   the complete CPU usage of all therads of the process...*/
#if 0
  sprintf(szProc, "/proc/%d/stat", the_rti.pid);
  f = gf_f64_open(szProc, "r");
  if (f) {
    fflush(f);
    if (fgets(line, 2048, f) != NULL) {
      char state;
      char *start;
      long cutime, cstime, priority, nice, itrealvalue, rss;
      int exit_signal, processor;
      unsigned long flags, minflt, cminflt, majflt, cmajflt, utime, stime,starttime, vsize, rlim, startcode, endcode, startstack, kstkesp, kstkeip, signal, blocked, sigignore, sigcatch, wchan, nswap, cnswap, rem;
      int ppid, pgrp ,session, tty_nr, tty_pgrp, res;
      start = strchr(line, ')');
      if (start) start += 2;
      else {
	start = strchr(line, ' ');
	start++;
      }
      res = sscanf(start,"%c %d %d %d %d %d %lu %lu %lu %lu \
%lu %lu %lu %ld %ld %ld %ld %ld %ld %lu \
%lu %ld %lu %lu %lu %lu %lu %lu %lu %lu \
%lu %lu %lu %lu %lu %d %d",
		   &state, &ppid, &pgrp, &session, &tty_nr, &tty_pgrp, &flags, &minflt, &cminflt, &majflt,
		   &cmajflt, &utime, &stime, &cutime, &cstime, &priority, &nice, &itrealvalue, &rem, &starttime,
		   &vsize, &rss, &rlim, &startcode, &endcode, &startstack, &kstkesp, &kstkeip, &signal, &blocked,
		   &sigignore, &sigcatch, &wchan, &nswap, &cnswap, &exit_signal, &processor);
 
      if (res) process_u_k_time = (u64) (cutime + cstime);
      else {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[RTI] PROC %s parse error\n", szProc));
	  }
    } else {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[RTI] error reading pid/stat\n\n", szProc));
    }
    fclose(f);
  } else {
	GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[RTI] cannot open %s\n", szProc));
  }
  sprintf(szProc, "/proc/%d/status", the_rti.pid);
  f = gf_f64_open(szProc, "r");
  if (f) {
    while (fgets(line, 1024, f) != NULL) {
      if (!strnicmp(line, "VmSize:", 7)) {
	sscanf(line, "VmSize: %"LLD" kB",  &the_rti.process_memory);
	the_rti.process_memory *= 1024;
      }
    }
    fclose(f);
  } else {
	GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[RTI] cannot open %s\n", szProc));
  }
#endif


  the_rti.physical_memory = the_rti.physical_memory_avail = 0;
  f = gf_f64_open("/proc/meminfo", "r");
  if (f) {
    while (fgets(line, 1024, f) != NULL) {
      if (!strnicmp(line, "MemTotal:", 9)) {
	sscanf(line, "MemTotal: "LLU" kB",  &the_rti.physical_memory);
	the_rti.physical_memory *= 1024;
      }else if (!strnicmp(line, "MemFree:", 8)) {
	sscanf(line, "MemFree: "LLU" kB",  &the_rti.physical_memory_avail);
	the_rti.physical_memory_avail *= 1024;
	break;
      }
    }
    fclose(f);
  } else {
	GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[RTI] cannot open /proc/meminfo\n"));
  }


  the_rti.sampling_instant = last_update_time;
  
  if (last_update_time) {
    the_rti.sampling_period_duration = (entry_time - last_update_time);
    the_rti.process_cpu_time_diff = (process_u_k_time - last_process_k_u_time) * 10;

    /*oops, we have no choice but to assume 100% cpu usage during this period*/
    if (!u_k_time) {
      the_rti.total_cpu_time_diff = the_rti.sampling_period_duration;
      u_k_time = last_cpu_u_k_time + the_rti.sampling_period_duration;
      the_rti.cpu_idle_time = 0;
      the_rti.total_cpu_usage = 100;
      if (!the_rti.process_cpu_time_diff) the_rti.process_cpu_time_diff = the_rti.total_cpu_time_diff;
      the_rti.process_cpu_usage = (u32) ( 100 *  the_rti.process_cpu_time_diff / the_rti.sampling_period_duration);
    } else {
      u64 samp_sys_time;
      /*move to ms (/proc/stat gives times in 100 ms unit*/
      the_rti.total_cpu_time_diff = (u_k_time - last_cpu_u_k_time)*10;

      /*we're not that accurate....*/
      if (the_rti.total_cpu_time_diff > the_rti.sampling_period_duration)
      	the_rti.sampling_period_duration = the_rti.total_cpu_time_diff;

   
      if (!idle_time) idle_time = (the_rti.sampling_period_duration - the_rti.total_cpu_time_diff)/10;
      samp_sys_time = u_k_time - last_cpu_u_k_time;
      the_rti.cpu_idle_time = idle_time - last_cpu_idle_time;
      the_rti.total_cpu_usage = (u32) ( 100 * samp_sys_time / (the_rti.cpu_idle_time + samp_sys_time ) );
      /*move to ms (/proc/stat gives times in 100 ms unit*/
      the_rti.cpu_idle_time *= 10;
      if (!the_rti.process_cpu_time_diff) the_rti.process_cpu_time_diff = the_rti.total_cpu_time_diff;
      the_rti.process_cpu_usage = (u32) ( 100 *  the_rti.process_cpu_time_diff / (the_rti.cpu_idle_time + 10*samp_sys_time ) );
    }
  } else {
    mem_at_startup = the_rti.physical_memory_avail;
  }
  the_rti.process_memory = mem_at_startup - the_rti.physical_memory_avail;
#ifdef GPAC_MEMORY_TRACKING
  the_rti.gpac_memory = gpac_allocated_memory;
#endif

  last_process_k_u_time = process_u_k_time;
  last_cpu_idle_time = idle_time;
  last_cpu_u_k_time = u_k_time;
  last_update_time = entry_time;
  memcpy(rti, &the_rti, sizeof(GF_SystemRTInfo));
  return 1;
}

#endif

char * gf_get_default_cache_directory(){  
#ifdef _WIN32_WCE
  return gf_strdup( "\\windows\\temp" );
#elif defined(WIN32)
  char szPath[512];
  GetWindowsDirectory(szPath, 507);
  if (szPath[strlen(szPath)-1] != '\\')
    strcat((char*)szPath, "\\");
  strcat((char *)szPath, "Temp");
  return gf_strdup( szPath );
#else
  return gf_strdup("/tmp");
#endif
}




Bool gf_sys_get_battery_state(Bool *onBattery, u32 *onCharge, u32*level, u32 *batteryLifeTime, u32 *batteryFullLifeTime) 
{
#if defined(_WIN32_WCE)
	SYSTEM_POWER_STATUS_EX sps;
	GetSystemPowerStatusEx(&sps, 0);
	if (onBattery) *onBattery = sps.ACLineStatus ? 0 : 1;
	if (onCharge) *onCharge = (sps.BatteryFlag & BATTERY_FLAG_CHARGING) ? 1 : 0;
	if (level) *level = sps.BatteryLifePercent;
	if (batteryLifeTime) *batteryLifeTime = sps.BatteryLifeTime;
	if (batteryFullLifeTime) *batteryFullLifeTime = sps.BatteryFullLifeTime;
#elif defined(WIN32)
	SYSTEM_POWER_STATUS sps;
	GetSystemPowerStatus(&sps);
	if (onBattery) *onBattery = sps.ACLineStatus ? 0 : 1;
	if (onCharge) *onCharge = (sps.BatteryFlag & BATTERY_FLAG_CHARGING) ? 1 : 0;
	if (level) *level = sps.BatteryLifePercent;
	if (batteryLifeTime) *batteryLifeTime = sps.BatteryLifeTime;
	if (batteryFullLifeTime) *batteryFullLifeTime = sps.BatteryFullLifeTime;
#endif
	return 1;
}


struct GF_GlobalLock {
    const char * resourceName;
};


#ifndef WIN32
#define CPF_CLOEXEC 1

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

struct _GF_GlobalLock_opaque {
    char * resourceName;
    char * pidFile;
    int fd;
};

GF_GlobalLock * gf_create_PID_file( const char * resourceName )
{
  const char * prefix = "/gpac_lock_";
  const char * dir = gf_get_default_cache_directory();
  char * pidfile;
  int flags;
  int status;
  pidfile = gf_malloc(strlen(dir)+strlen(prefix)+strlen(resourceName)+1);
  strcpy(pidfile, dir);
  strcat(pidfile, prefix);
  /* Use only valid names for file */
  {
    const char *res;
    char * pid = &(pidfile[strlen(pidfile)]);
    for (res = resourceName; *res ; res++){
      if (*res >= 'A' && *res <= 'z')
	*pid = * res;
      else
	*pid = '_';
      pid++;
    }
    *pid = '\0';
  }
  int fd = open(pidfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1)
    goto exit;
  /* Get the flags */
  flags = fcntl(fd, F_GETFD);
  if (flags == -1){
      goto exit;
  }
   /* Set FD_CLOEXEC, so exclusive lock will be removed on exit, so even if GPAC crashes,
    * lock will be allowed for next instance */
  flags |= FD_CLOEXEC;
  /* Now, update the flags */
  if (fcntl(fd, F_SETFD, flags) == -1){
    goto exit;
  }
  
  /* Now, we try to lock the file */
  {
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = fl.l_len = 0;
    status = fcntl(fd, F_SETLK, &fl);
  }

  if (status == -1) {
    goto exit;
  }

    if (ftruncate(fd, 0) == -1){
      goto exit;
    }
  /* Write the PID */
  {
    int sz = 100;
    char * buf = gf_malloc( sz );
    sz = snprintf(buf, sz, "%ld\n", (long) getpid());
    if (write(fd, buf, sz) != sz){
        gf_free(buf);
	goto exit;
    }
  }
  sync();
  {
      GF_GlobalLock * lock = gf_malloc( sizeof(GF_GlobalLock));
      lock->resourceName = gf_strdup(resourceName);
      lock->pidFile = pidfile;
      lock->fd = fd;
      return lock;
  }
exit:
  if (fd >= 0)
    close(fd);
  return NULL;
}
#else /* WIN32 */
struct _GF_GlobalLock_opaque {
    char * resourceName;
	HANDLE hMutex; /*a named mutex is a system-mode object on windows*/
};
#endif


GF_GlobalLock * gf_global_resource_lock(const char * resourceName){
#ifdef WIN32
	GF_GlobalLock *lock = gf_malloc(sizeof(GF_GlobalLock));
	lock->resourceName = gf_strdup(resourceName);

	/*first ensure mutex is created*/
	lock->hMutex = CreateMutex(NULL, TRUE, resourceName);
	if (!lock->hMutex) {
		DWORD lastErr = GetLastError();
		if (lastErr != ERROR_ALREADY_EXISTS)
			return NULL;
	}

	/*then lock it*/
	switch (WaitForSingleObject(lock->hMutex, INFINITE)) {
	case WAIT_ABANDONED:
	case WAIT_TIMEOUT:
		assert(0); /*serious error: someone has modified the object elsewhere*/
		gf_global_resource_unlock(lock);
		return NULL;
	}

	return lock;
#else /* WIN32 */
  return gf_create_PID_file(resourceName);
#endif /* WIN32 */
}

/*!
 * Unlock a previouly locked resource
 * \param lock The resource to unlock
 * \return GF_OK if evertything went fine
 */
GF_Err gf_global_resource_unlock(GF_GlobalLock * lock){
    if (!lock)
      return GF_BAD_PARAM;
#ifndef WIN32
    assert( lock->pidFile);
    close(lock->fd);
    if (unlink(lock->pidFile))
      perror("Failed to unlink lock file");
    gf_free(lock->pidFile);
    lock->pidFile = NULL;
    lock->fd = -1;
#else /* WIN32 */
	{
		/*MSDN: "The mutex object is destroyed when its last handle has been closed."*/
		BOOL ret = ReleaseMutex(lock->hMutex);
		assert(ret);
		ret = CloseHandle(lock->hMutex);
		assert(ret);
	}
#endif
	if (lock->resourceName)
		gf_free(lock->resourceName);
    lock->resourceName = NULL;
    gf_free(lock);
    return GF_OK;
}
