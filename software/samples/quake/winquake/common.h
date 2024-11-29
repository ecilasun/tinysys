/*
Copyright (C) 1996-1997 Id Software, Inc.
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
// comndef.h  -- general definitions

#if !defined BYTE_DEFINED
typedef unsigned char 		byte;
#define BYTE_DEFINED 1
#endif

#include <stdbool.h>
typedef _Bool qboolean;

//============================================================================

typedef struct sizebuf_s
{
	qboolean	allowoverflow;	// if false, do a Sys_Error
	qboolean	overflowed;		// set to true if the buffer size failed
	byte	*data;
	int		maxsize;
	int		cursize;
} sizebuf_t;

void SZ_Alloc (sizebuf_t *buf, int startsize);
void SZ_Free (sizebuf_t *buf);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, int length);
void SZ_Write (sizebuf_t *buf, void *data, int length);
void SZ_Print (sizebuf_t *buf, char *data);	// strcats onto the sizebuf

//============================================================================

typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;


void ClearLink (link_t *l);
void RemoveLink (link_t *l);
void InsertLinkBefore (link_t *l, link_t *before);
void InsertLinkAfter (link_t *l, link_t *after);

// (type *)STRUCT_FROM_LINK(link_t *link, type, member)
// ent = STRUCT_FROM_LINK(link,entity_t,order)
// FIXME: remove this mess!
#define	STRUCT_FROM_LINK(l,t,m) ((t *)((byte *)l - (uintptr_t)&(((t *)0)->m)))

//============================================================================

#ifndef NULL
#define NULL ((void *)0)
#endif

#define Q_MAXCHAR ((char)0x7f)
#define Q_MAXSHORT ((short)0x7fff)
#define Q_MAXINT	((int)0x7fffffff)
#define Q_MAXLONG ((int)0x7fffffff)
#define Q_MAXFLOAT ((int)0x7fffffff)

#define Q_MINCHAR ((char)0x80)
#define Q_MINSHORT ((short)0x8000)
#define Q_MININT 	((int)0x80000000)
#define Q_MINLONG ((int)0x80000000)
#define Q_MINFLOAT ((int)0x7fffffff)

//============================================================================

/*
 * ========================================================================
 *                          BYTE ORDER FUNCTIONS
 * ========================================================================
 */

#undef MSB_FIRST
#undef LSB_FIRST
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define MSB_FIRST 1
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LSB_FIRST 1
#else
#error "Invalid endianness macros"
#endif

static inline short bswap16(short s) {
	return ((s & 255) << 8) | ((s >> 8) & 255);
}
static inline int bswap32(int l) {
	return
		(((l >>  0) & 255) << 24)
		| (((l >>  8) & 255) << 16)
		| (((l >> 16) & 255) <<  8)
		| (((l >> 24) & 255) <<  0);
}

#ifdef MSB_FIRST
static inline short BigShort(short s) { return s; }
static inline int BigLong(int l) { return l; }
static inline float BigFloat(float f) { return f; }
static inline short LittleShort(short s) { return bswap16(s); }
static inline int LittleLong(int l) { return bswap32(l); }
static inline float LittleFloat(float f) {
	union {
		float f;
		byte b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}
#else
static inline short BigShort(short s) { return bswap16(s); }
static inline int BigLong(int l) { return bswap32(l); }
static inline float BigFloat(float f) {
	union {
		float f;
		byte b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}
static inline short LittleShort(short s) { return s; }
static inline int LittleLong(int l) { return l; }
static inline float LittleFloat(float f) { return f; }
#endif

//============================================================================

void MSG_WriteChar (sizebuf_t *sb, int c);
void MSG_WriteByte (sizebuf_t *sb, int c);
void MSG_WriteShort (sizebuf_t *sb, int c);
void MSG_WriteLong (sizebuf_t *sb, int c);
void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, char *s);
void MSG_WriteCoord (sizebuf_t *sb, float f);
void MSG_WriteAngle (sizebuf_t *sb, float f);

extern	int			msg_readcount;
extern	qboolean	msg_badread;		// set if a read goes beyond end of message

void MSG_BeginReading (void);
int MSG_ReadChar (void);
int MSG_ReadByte (void);
int MSG_ReadShort (void);
int MSG_ReadLong (void);
float MSG_ReadFloat (void);
char *MSG_ReadString (void);

float MSG_ReadCoord (void);
float MSG_ReadAngle (void);

//============================================================================

#if defined(__riscv) || defined(__riscv__) /* older GCC toolchain */
static inline void Q_memset(void *dest, int fill, int count) { memset(dest, fill, count); }
static inline void Q_memcpy(void *dest, void *src, int count) { memcpy(dest, src, count); }
static inline int Q_memcmp(void *m1, void *m2, int count) { return memcmp(m1, m2, count); }
static inline void Q_strcpy(char *dest, char *src) { strcpy(dest, src); }
static inline void Q_strncpy(char *dest, char *src, int count) { strncpy(dest, src, count); }
static inline int Q_strlen(char *str) { return strlen(str); }
static inline char *Q_strrchr(char *s, char c) { return strrchr(s, c); }
static inline void Q_strcat(char *dest, char *src) { strcat(dest, src); }
static inline int Q_strcmp(char *s1, char *s2) { return strcmp(s1, s2); }
static inline int Q_strncmp(char *s1, char *s2, int count) { return strncmp(s1, s2, count); }
static inline int Q_strcasecmp(char *s1, char *s2) { return strcasecmp(s1, s2); }
static inline int Q_strncasecmp(char *s1, char *s2, int n) { return strncasecmp(s1, s2, n); }
static inline int Q_atoi(char *str) { return atoi(str); }
static inline float Q_atof(char *str) { return atof(str); }
#else
void Q_memset (void *dest, int fill, int count);
void Q_memcpy (void *dest, void *src, int count);
int Q_memcmp (void *m1, void *m2, int count);
void Q_strcpy (char *dest, char *src);
void Q_strncpy (char *dest, char *src, int count);
int Q_strlen (char *str);
char *Q_strrchr (char *s, char c);
void Q_strcat (char *dest, char *src);
int Q_strcmp (char *s1, char *s2);
int Q_strncmp (char *s1, char *s2, int count);
int Q_strcasecmp (char *s1, char *s2);
int Q_strncasecmp (char *s1, char *s2, int n);
int	Q_atoi (char *str);
float Q_atof (char *str);
#endif

//============================================================================

extern	char		com_token[1024];
extern	qboolean	com_eof;

char *COM_Parse (char *data);


extern	int		com_argc;
extern	char	**com_argv;

int COM_CheckParm (char *parm);
void COM_Init (char *path);
void COM_InitArgv (int argc, char **argv);

char *COM_SkipPath (char *pathname);
void COM_StripExtension (char *in, char *out);
void COM_FileBase (char *in, char *out, size_t outsize);
void COM_DefaultExtension (char *path, char *extension);

char	*va(char *format, ...);
// does a varargs printf into a temp buffer


//============================================================================

extern int com_filesize;
struct cache_user_s;

extern	char	com_gamedir[MAX_OSPATH];

void COM_WriteFile (char *filename, void *data, int len);
int COM_OpenFile (char *filename, int *hndl);
int COM_FOpenFile (char *filename, FILE **file);
void COM_CloseFile (int h);

byte *COM_LoadStackFile (char *path, void *buffer, int bufsize);
byte *COM_LoadTempFile (char *path);
byte *COM_LoadHunkFile (char *path);
void COM_LoadCacheFile (char *path, struct cache_user_s *cu);


extern	struct cvar_s	registered;

extern qboolean		standard_quake, rogue, hipnotic;
