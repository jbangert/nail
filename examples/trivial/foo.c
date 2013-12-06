
typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;

typedef long int int64_t;







typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

typedef unsigned int uint32_t;



typedef unsigned long int uint64_t;
typedef signed char int_least8_t;
typedef short int int_least16_t;
typedef int int_least32_t;

typedef long int int_least64_t;






typedef unsigned char uint_least8_t;
typedef unsigned short int uint_least16_t;
typedef unsigned int uint_least32_t;

typedef unsigned long int uint_least64_t;
typedef signed char int_fast8_t;

typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
typedef unsigned char uint_fast8_t;

typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
typedef long int intptr_t;


typedef unsigned long int uintptr_t;
typedef long int intmax_t;
typedef unsigned long int uintmax_t;




typedef long unsigned int size_t;



typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;







typedef long int __quad_t;
typedef unsigned long int __u_quad_t;


typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct {
    int __val[2];
} __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;

typedef int __daddr_t;
typedef int __key_t;


typedef int __clockid_t;


typedef void * __timer_t;


typedef long int __blksize_t;




typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;


typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;


typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;


typedef long int __fsword_t;

typedef long int __ssize_t;


typedef long int __syscall_slong_t;

typedef unsigned long int __syscall_ulong_t;



typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;


typedef long int __intptr_t;


typedef unsigned int __socklen_t;
struct _IO_FILE;



typedef struct _IO_FILE FILE;





typedef struct _IO_FILE __FILE;




typedef struct
{
    int __count;
    union
    {

        unsigned int __wch;



        char __wchb[4];
    } __value;
} __mbstate_t;
typedef struct
{
    __off_t __pos;
    __mbstate_t __state;
} _G_fpos_t;
typedef struct
{
    __off64_t __pos;
    __mbstate_t __state;
} _G_fpos64_t;
struct _IO_jump_t;
struct _IO_FILE;
typedef void _IO_lock_t;





struct _IO_marker {
    struct _IO_marker *_next;
    struct _IO_FILE *_sbuf;



    int _pos;
};


enum __codecvt_result
{
    __codecvt_ok,
    __codecvt_partial,
    __codecvt_error,
    __codecvt_noconv
};
struct _IO_FILE {
    int _flags;




    char* _IO_read_ptr;
    char* _IO_read_end;
    char* _IO_read_base;
    char* _IO_write_base;
    char* _IO_write_ptr;
    char* _IO_write_end;
    char* _IO_buf_base;
    char* _IO_buf_end;

    char *_IO_save_base;
    char *_IO_backup_base;
    char *_IO_save_end;

    struct _IO_marker *_markers;

    struct _IO_FILE *_chain;

    int _fileno;



    int _flags2;

    __off_t _old_offset;



    unsigned short _cur_column;
    signed char _vtable_offset;
    char _shortbuf[1];



    _IO_lock_t *_lock;
    __off64_t _offset;
    void *__pad1;
    void *__pad2;
    void *__pad3;
    void *__pad4;
    size_t __pad5;

    int _mode;

    char _unused2[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (size_t)];

};


typedef struct _IO_FILE _IO_FILE;


struct _IO_FILE_plus;

extern struct _IO_FILE_plus _IO_2_1_stdin_;
extern struct _IO_FILE_plus _IO_2_1_stdout_;
extern struct _IO_FILE_plus _IO_2_1_stderr_;
typedef __ssize_t __io_read_fn (void *__cookie, char *__buf, size_t __nbytes);







typedef __ssize_t __io_write_fn (void *__cookie, const char *__buf,
                                 size_t __n);







typedef int __io_seek_fn (void *__cookie, __off64_t *__pos, int __w);


typedef int __io_close_fn (void *__cookie);
extern int __underflow (_IO_FILE *);
extern int __uflow (_IO_FILE *);
extern int __overflow (_IO_FILE *, int);
extern int _IO_getc (_IO_FILE *__fp);
extern int _IO_putc (int __c, _IO_FILE *__fp);
extern int _IO_feof (_IO_FILE *__fp) __attribute__ ((__nothrow__ , __leaf__));
extern int _IO_ferror (_IO_FILE *__fp) __attribute__ ((__nothrow__ , __leaf__));

extern int _IO_peekc_locked (_IO_FILE *__fp);





extern void _IO_flockfile (_IO_FILE *) __attribute__ ((__nothrow__ , __leaf__));
extern void _IO_funlockfile (_IO_FILE *) __attribute__ ((__nothrow__ , __leaf__));
extern int _IO_ftrylockfile (_IO_FILE *) __attribute__ ((__nothrow__ , __leaf__));
extern int _IO_vfscanf (_IO_FILE * __restrict, const char * __restrict,
                        __gnuc_va_list, int *__restrict);
extern int _IO_vfprintf (_IO_FILE *__restrict, const char *__restrict,
                         __gnuc_va_list);
extern __ssize_t _IO_padn (_IO_FILE *, int, __ssize_t);
extern size_t _IO_sgetn (_IO_FILE *, void *, size_t);

extern __off64_t _IO_seekoff (_IO_FILE *, __off64_t, int, int);
extern __off64_t _IO_seekpos (_IO_FILE *, __off64_t, int);

extern void _IO_free_backup_area (_IO_FILE *) __attribute__ ((__nothrow__ , __leaf__));
typedef __off_t off_t;
typedef __ssize_t ssize_t;







typedef _G_fpos_t fpos_t;







extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stderr;







extern int remove (const char *__filename) __attribute__ ((__nothrow__ , __leaf__));

extern int rename (const char *__old, const char *__new) __attribute__ ((__nothrow__ , __leaf__));




extern int renameat (int __oldfd, const char *__old, int __newfd,
                     const char *__new) __attribute__ ((__nothrow__ , __leaf__));








extern FILE *tmpfile (void) ;
extern char *tmpnam (char *__s) __attribute__ ((__nothrow__ , __leaf__)) ;





extern char *tmpnam_r (char *__s) __attribute__ ((__nothrow__ , __leaf__)) ;
extern char *tempnam (const char *__dir, const char *__pfx)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) ;








extern int fclose (FILE *__stream);




extern int fflush (FILE *__stream);

extern int fflush_unlocked (FILE *__stream);






extern FILE *fopen (const char *__restrict __filename,
                    const char *__restrict __modes) ;




extern FILE *freopen (const char *__restrict __filename,
                      const char *__restrict __modes,
                      FILE *__restrict __stream) ;

extern FILE *fdopen (int __fd, const char *__modes) __attribute__ ((__nothrow__ , __leaf__)) ;
extern FILE *fmemopen (void *__s, size_t __len, const char *__modes)
__attribute__ ((__nothrow__ , __leaf__)) ;




extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) __attribute__ ((__nothrow__ , __leaf__)) ;






extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__ , __leaf__));



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
                    int __modes, size_t __n) __attribute__ ((__nothrow__ , __leaf__));





extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
                       size_t __size) __attribute__ ((__nothrow__ , __leaf__));


extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));








extern int fprintf (FILE *__restrict __stream,
                    const char *__restrict __format, ...);




extern int printf (const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
                    const char *__restrict __format, ...) __attribute__ ((__nothrow__));





extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
                     __gnuc_va_list __arg);




extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, const char *__restrict __format,
                     __gnuc_va_list __arg) __attribute__ ((__nothrow__));





extern int snprintf (char *__restrict __s, size_t __maxlen,
                     const char *__restrict __format, ...)
__attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
                      const char *__restrict __format, __gnuc_va_list __arg)
__attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));

extern int vdprintf (int __fd, const char *__restrict __fmt,
                     __gnuc_va_list __arg)
__attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, const char *__restrict __fmt, ...)
__attribute__ ((__format__ (__printf__, 2, 3)));








extern int fscanf (FILE *__restrict __stream,
                   const char *__restrict __format, ...) ;




extern int scanf (const char *__restrict __format, ...) ;

extern int sscanf (const char *__restrict __s,
                   const char *__restrict __format, ...) __attribute__ ((__nothrow__ , __leaf__));
extern int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...) __asm__ ("" "__isoc99_fscanf")

;
extern int scanf (const char *__restrict __format, ...) __asm__ ("" "__isoc99_scanf")
;
extern int sscanf (const char *__restrict __s, const char *__restrict __format, ...) __asm__ ("" "__isoc99_sscanf") __attribute__ ((__nothrow__ , __leaf__))

;








extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
                    __gnuc_va_list __arg)
__attribute__ ((__format__ (__scanf__, 2, 0))) ;





extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
__attribute__ ((__format__ (__scanf__, 1, 0))) ;


extern int vsscanf (const char *__restrict __s,
                    const char *__restrict __format, __gnuc_va_list __arg)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__format__ (__scanf__, 2, 0)));
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vfscanf")



__attribute__ ((__format__ (__scanf__, 2, 0))) ;
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vscanf")

__attribute__ ((__format__ (__scanf__, 1, 0))) ;
extern int vsscanf (const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vsscanf") __attribute__ ((__nothrow__ , __leaf__))



__attribute__ ((__format__ (__scanf__, 2, 0)));









extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);





extern int getchar (void);

extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
extern int fgetc_unlocked (FILE *__stream);











extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);





extern int putchar (int __c);

extern int fputc_unlocked (int __c, FILE *__stream);







extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream);


extern int putw (int __w, FILE *__stream);








extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
;
extern char *gets (char *__s) __attribute__ ((__deprecated__));


extern __ssize_t __getdelim (char **__restrict __lineptr,
                             size_t *__restrict __n, int __delimiter,
                             FILE *__restrict __stream) ;
extern __ssize_t getdelim (char **__restrict __lineptr,
                           size_t *__restrict __n, int __delimiter,
                           FILE *__restrict __stream) ;







extern __ssize_t getline (char **__restrict __lineptr,
                          size_t *__restrict __n,
                          FILE *__restrict __stream) ;








extern int fputs (const char *__restrict __s, FILE *__restrict __stream);





extern int puts (const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
                     size_t __n, FILE *__restrict __stream) ;




extern size_t fwrite (const void *__restrict __ptr, size_t __size,
                      size_t __n, FILE *__restrict __s);

extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
                              size_t __n, FILE *__restrict __stream) ;
extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
                               size_t __n, FILE *__restrict __stream);








extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream) ;




extern void rewind (FILE *__stream);

extern int fseeko (FILE *__stream, __off_t __off, int __whence);




extern __off_t ftello (FILE *__stream) ;






extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);




extern int fsetpos (FILE *__stream, const fpos_t *__pos);



extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));

extern int feof (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;

extern int ferror (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;




extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;








extern void perror (const char *__s);






extern int sys_nerr;
extern const char *const sys_errlist[];




extern int fileno (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;
extern FILE *popen (const char *__command, const char *__modes) ;





extern int pclose (FILE *__stream);





extern char *ctermid (char *__s) __attribute__ ((__nothrow__ , __leaf__));
extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));







typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;




typedef __loff_t loff_t;



typedef __ino_t ino_t;
typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;
typedef __pid_t pid_t;





typedef __id_t id_t;
typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;


typedef __clock_t clock_t;





typedef __time_t time_t;



typedef __clockid_t clockid_t;
typedef __timer_t timer_t;



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
typedef unsigned int u_int8_t __attribute__ ((__mode__ (__QI__)));
typedef unsigned int u_int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int u_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int u_int64_t __attribute__ ((__mode__ (__DI__)));

typedef int register_t __attribute__ ((__mode__ (__word__)));






static __inline unsigned int
__bswap_32 (unsigned int __bsx)
{
    return __builtin_bswap32 (__bsx);
}
static __inline __uint64_t
__bswap_64 (__uint64_t __bsx)
{
    return __builtin_bswap64 (__bsx);
}




typedef int __sig_atomic_t;




typedef struct
{
    unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
} __sigset_t;



typedef __sigset_t sigset_t;





struct timespec
{
    __time_t tv_sec;
    __syscall_slong_t tv_nsec;
};

struct timeval
{
    __time_t tv_sec;
    __suseconds_t tv_usec;
};


typedef __suseconds_t suseconds_t;





typedef long int __fd_mask;
typedef struct
{






    __fd_mask __fds_bits[1024 / (8 * (int) sizeof (__fd_mask))];


} fd_set;






typedef __fd_mask fd_mask;

extern int select (int __nfds, fd_set *__restrict __readfds,
                   fd_set *__restrict __writefds,
                   fd_set *__restrict __exceptfds,
                   struct timeval *__restrict __timeout);
extern int pselect (int __nfds, fd_set *__restrict __readfds,
                    fd_set *__restrict __writefds,
                    fd_set *__restrict __exceptfds,
                    const struct timespec *__restrict __timeout,
                    const __sigset_t *__restrict __sigmask);





__extension__
extern unsigned int gnu_dev_major (unsigned long long int __dev)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
__extension__
extern unsigned int gnu_dev_minor (unsigned long long int __dev)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
__extension__
extern unsigned long long int gnu_dev_makedev (unsigned int __major,
        unsigned int __minor)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));






typedef __blksize_t blksize_t;






typedef __blkcnt_t blkcnt_t;



typedef __fsblkcnt_t fsblkcnt_t;



typedef __fsfilcnt_t fsfilcnt_t;
typedef unsigned long int pthread_t;


union pthread_attr_t
{
    char __size[56];
    long int __align;
};

typedef union pthread_attr_t pthread_attr_t;





typedef struct __pthread_internal_list
{
    struct __pthread_internal_list *__prev;
    struct __pthread_internal_list *__next;
} __pthread_list_t;
typedef union
{
    struct __pthread_mutex_s
    {
        int __lock;
        unsigned int __count;
        int __owner;

        unsigned int __nusers;



        int __kind;

        short __spins;
        short __elision;
        __pthread_list_t __list;
    } __data;
    char __size[40];
    long int __align;
} pthread_mutex_t;

typedef union
{
    char __size[4];
    int __align;
} pthread_mutexattr_t;




typedef union
{
    struct
    {
        int __lock;
        unsigned int __futex;
        __extension__ unsigned long long int __total_seq;
        __extension__ unsigned long long int __wakeup_seq;
        __extension__ unsigned long long int __woken_seq;
        void *__mutex;
        unsigned int __nwaiters;
        unsigned int __broadcast_seq;
    } __data;
    char __size[48];
    __extension__ long long int __align;
} pthread_cond_t;

typedef union
{
    char __size[4];
    int __align;
} pthread_condattr_t;



typedef unsigned int pthread_key_t;



typedef int pthread_once_t;





typedef union
{

    struct
    {
        int __lock;
        unsigned int __nr_readers;
        unsigned int __readers_wakeup;
        unsigned int __writer_wakeup;
        unsigned int __nr_readers_queued;
        unsigned int __nr_writers_queued;
        int __writer;
        int __shared;
        unsigned long int __pad1;
        unsigned long int __pad2;


        unsigned int __flags;

    } __data;
    char __size[56];
    long int __align;
} pthread_rwlock_t;

typedef union
{
    char __size[8];
    long int __align;
} pthread_rwlockattr_t;





typedef volatile int pthread_spinlock_t;




typedef union
{
    char __size[32];
    long int __align;
} pthread_barrier_t;

typedef union
{
    char __size[4];
    int __align;
} pthread_barrierattr_t;





typedef struct HAllocator_ {
    void* (*alloc)(struct HAllocator_* allocator, size_t size);
    void* (*realloc)(struct HAllocator_* allocator, void* ptr, size_t size);
    void (*free)(struct HAllocator_* allocator, void* ptr);
} HAllocator;

typedef struct HArena_ HArena ;

HArena *h_new_arena(HAllocator* allocator, size_t block_size);

void* h_arena_malloc(HArena *arena, size_t count) __attribute__(( malloc, alloc_size(2) ));



void h_arena_free(HArena *arena, void* ptr);
void h_delete_arena(HArena *arena);

typedef struct {
    size_t used;
    size_t wasted;
} HArenaStats;

void h_allocator_stats(HArena *arena, HArenaStats *stats);
typedef int bool;



typedef struct HParseState_ HParseState;

typedef enum HParserBackend_ {
    PB_MIN = 0,
    PB_PACKRAT = PB_MIN,
    PB_REGULAR,
    PB_LLk,
    PB_LALR,
    PB_GLR,
    PB_MAX = PB_GLR
} HParserBackend;

typedef enum HTokenType_ {

    TT_NONE = 1,
    TT_BYTES = 2,
    TT_SINT = 4,
    TT_UINT = 8,
    TT_SEQUENCE = 16,
    TT_RESERVED_1,
    TT_ERR = 32,
    TT_USER = 64,
    TT_MAX
} HTokenType;

typedef struct HCountedArray_ {
    size_t capacity;
    size_t used;
    HArena * arena;
    struct HParsedToken_ **elements;
} HCountedArray;

typedef struct HBytes_ {
    const uint8_t *token;
    size_t len;
} HBytes;
typedef struct HParsedToken_ {
    HTokenType token_type;

    union {
        HBytes bytes;
        int64_t sint;
        uint64_t uint;
        double dbl;
        float flt;
        HCountedArray *seq;
        void *user;
    };



    size_t index;
    char bit_offset;
} HParsedToken;
typedef struct HParseResult_ {
    const HParsedToken *ast;
    int64_t bit_length;
    HArena * arena;
} HParseResult;





typedef struct HBitWriter_ HBitWriter;
typedef HParsedToken* (*HAction)(const HParseResult *p, void* user_data);






typedef bool (*HPredicate)(HParseResult *p, void* user_data);

typedef struct HCFChoice_ HCFChoice;
typedef struct HRVMProg_ HRVMProg;
typedef struct HParserVtable_ HParserVtable;


typedef struct HParser_ {
    const HParserVtable *vtable;
    HParserBackend backend;
    void* backend_data;
    void *env;
    HCFChoice *desugared;
} HParser;


typedef struct HParserTestcase_ {
    unsigned char* input;
    size_t length;
    char* output_unambiguous;
} HParserTestcase;
typedef struct HCaseResult_ {
    bool success;

    union {
        const char* actual_results;
        size_t parse_time;
    };



} HCaseResult;

typedef struct HBackendResults_ {
    HParserBackend backend;
    bool compile_success;
    size_t n_testcases;
    size_t failed_testcases;
    HCaseResult *cases;
} HBackendResults;

typedef struct HBenchmarkResults_ {
    size_t len;
    HBackendResults *results;
} HBenchmarkResults;
HParseResult* h_parse(const HParser* parser, const uint8_t* input, size_t length);
HParseResult* h_parse__m(HAllocator* mm__, const HParser* parser, const uint8_t* input, size_t length);






HParser* h_token(const uint8_t *str, const size_t len);
HParser* h_token__m(HAllocator* mm__, const uint8_t *str, const size_t len);







HParser* h_ch(const uint8_t c);
HParser* h_ch__m(HAllocator* mm__, const uint8_t c);
HParser* h_ch_range(const uint8_t lower, const uint8_t upper);
HParser* h_ch_range__m(HAllocator* mm__, const uint8_t lower, const uint8_t upper);






HParser* h_int_range(const HParser *p, const int64_t lower, const int64_t upper);
HParser* h_int_range__m(HAllocator* mm__, const HParser *p, const int64_t lower, const int64_t upper);







HParser* h_bits(size_t len, bool sign);
HParser* h_bits__m(HAllocator* mm__, size_t len, bool sign);






HParser* h_int64(void);
HParser* h_int64__m(HAllocator* mm__);






HParser* h_int32(void);
HParser* h_int32__m(HAllocator* mm__);






HParser* h_int16(void);
HParser* h_int16__m(HAllocator* mm__);






HParser* h_int8(void);
HParser* h_int8__m(HAllocator* mm__);






HParser* h_uint64(void);
HParser* h_uint64__m(HAllocator* mm__);






HParser* h_uint32(void);
HParser* h_uint32__m(HAllocator* mm__);






HParser* h_uint16(void);
HParser* h_uint16__m(HAllocator* mm__);






HParser* h_uint8(void);
HParser* h_uint8__m(HAllocator* mm__);







HParser* h_whitespace(const HParser* p);
HParser* h_whitespace__m(HAllocator* mm__, const HParser* p);







HParser* h_left(const HParser* p, const HParser* q);
HParser* h_left__m(HAllocator* mm__, const HParser* p, const HParser* q);







HParser* h_right(const HParser* p, const HParser* q);
HParser* h_right__m(HAllocator* mm__, const HParser* p, const HParser* q);







HParser* h_middle(const HParser* p, const HParser* x, const HParser* q);
HParser* h_middle__m(HAllocator* mm__, const HParser* p, const HParser* x, const HParser* q);







HParser* h_action(const HParser* p, const HAction a, void* user_data);
HParser* h_action__m(HAllocator* mm__, const HParser* p, const HAction a, void* user_data);






HParser* h_in(const uint8_t *charset, size_t length);
HParser* h_in__m(HAllocator* mm__, const uint8_t *charset, size_t length);






HParser* h_not_in(const uint8_t *charset, size_t length);
HParser* h_not_in__m(HAllocator* mm__, const uint8_t *charset, size_t length);







HParser* h_end_p(void);
HParser* h_end_p__m(HAllocator* mm__);






HParser* h_nothing_p(void);
HParser* h_nothing_p__m(HAllocator* mm__);







HParser* h_sequence(HParser* p, ...) __attribute__((sentinel));
HParser* h_sequence__m(HAllocator* mm__, HParser* p, ...) __attribute__((sentinel));
HParser* h_sequence__mv(HAllocator* mm__, HParser* p, va_list ap);
HParser* h_sequence__v(HParser* p, va_list ap);
HParser* h_sequence__a(void *args[]);
HParser* h_sequence__ma(HAllocator *mm__, void *args[]);
HParser* h_choice(HParser* p, ...) __attribute__((sentinel));
HParser* h_choice__m(HAllocator* mm__, HParser* p, ...) __attribute__((sentinel));
HParser* h_choice__mv(HAllocator* mm__, HParser* p, va_list ap);
HParser* h_choice__v(HParser* p, va_list ap);
HParser* h_choice__a(void *args[]);
HParser* h_choice__ma(HAllocator *mm__, void *args[]);
HParser* h_butnot(const HParser* p1, const HParser* p2);
HParser* h_butnot__m(HAllocator* mm__, const HParser* p1, const HParser* p2);
HParser* h_difference(const HParser* p1, const HParser* p2);
HParser* h_difference__m(HAllocator* mm__, const HParser* p1, const HParser* p2);







HParser* h_xor(const HParser* p1, const HParser* p2);
HParser* h_xor__m(HAllocator* mm__, const HParser* p1, const HParser* p2);







HParser* h_many(const HParser* p);
HParser* h_many__m(HAllocator* mm__, const HParser* p);







HParser* h_many1(const HParser* p);
HParser* h_many1__m(HAllocator* mm__, const HParser* p);







HParser* h_repeat_n(const HParser* p, const size_t n);
HParser* h_repeat_n__m(HAllocator* mm__, const HParser* p, const size_t n);







HParser* h_optional(const HParser* p);
HParser* h_optional__m(HAllocator* mm__, const HParser* p);







HParser* h_ignore(const HParser* p);
HParser* h_ignore__m(HAllocator* mm__, const HParser* p);
HParser* h_sepBy(const HParser* p, const HParser* sep);
HParser* h_sepBy__m(HAllocator* mm__, const HParser* p, const HParser* sep);







HParser* h_sepBy1(const HParser* p, const HParser* sep);
HParser* h_sepBy1__m(HAllocator* mm__, const HParser* p, const HParser* sep);






HParser* h_epsilon_p(void);
HParser* h_epsilon_p__m(HAllocator* mm__);
HParser* h_length_value(const HParser* length, const HParser* value);
HParser* h_length_value__m(HAllocator* mm__, const HParser* length, const HParser* value);
HParser* h_attr_bool(const HParser* p, HPredicate pred, void* user_data);
HParser* h_attr_bool__m(HAllocator* mm__, const HParser* p, HPredicate pred, void* user_data);
HParser* h_and(const HParser* p);
HParser* h_and__m(HAllocator* mm__, const HParser* p);
HParser* h_not(const HParser* p);
HParser* h_not__m(HAllocator* mm__, const HParser* p);
HParser* h_indirect(void);
HParser* h_indirect__m(HAllocator* mm__);





void h_bind_indirect(HParser* indirect, const HParser* inner);
void h_bind_indirect__m(HAllocator* mm__, HParser* indirect, const HParser* inner);




void h_parse_result_free(HParseResult *result);
void h_parse_result_free__m(HAllocator* mm__, HParseResult *result);






char* h_write_result_unamb(const HParsedToken* tok);




void h_pprint(FILE* stream, const HParsedToken* tok, int indent, int delta);
int h_compile(HParser* parser, HParserBackend backend, const void* params);
int h_compile__m(HAllocator* mm__, HParser* parser, HParserBackend backend, const void* params);




HBitWriter *h_bit_writer_new(HAllocator* mm__);




void h_bit_writer_put(HBitWriter* w, uint64_t data, size_t nbits);






const uint8_t* h_bit_writer_get_buffer(HBitWriter* w, size_t *len);




void h_bit_writer_free(HBitWriter* w);



HParsedToken *h_act_first(const HParseResult *p, void* userdata);
HParsedToken *h_act_second(const HParseResult *p, void* userdata);
HParsedToken *h_act_last(const HParseResult *p, void* userdata);
HParsedToken *h_act_flatten(const HParseResult *p, void* userdata);
HParsedToken *h_act_ignore(const HParseResult *p, void* userdata);


HBenchmarkResults * h_benchmark(HParser* parser, HParserTestcase* testcases);
HBenchmarkResults * h_benchmark__m(HAllocator* mm__, HParser* parser, HParserTestcase* testcases);
void h_benchmark_report(FILE* stream, HBenchmarkResults* results);





int h_allocate_token_type(const char* name);


int h_get_token_type_number(const char* name);


const char* h_get_token_type_name(int token_type);



extern void __assert_fail (const char *__assertion, const char *__file,
                           unsigned int __line, const char *__function)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));


extern void __assert_perror_fail (int __errnum, const char *__file,
                                  unsigned int __line, const char *__function)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));




extern void __assert (const char *__assertion, const char *__file, int __line)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));



HParsedToken *h_act_index(int i, const HParseResult *p, void* user_data);
HParsedToken *h_act_first(const HParseResult *p, void* user_data);
HParsedToken *h_act_second(const HParseResult *p, void* user_data);
HParsedToken *h_act_last(const HParseResult *p, void* user_data);
HParsedToken *h_act_flatten(const HParseResult *p, void* user_data);
HParsedToken *h_act_ignore(const HParseResult *p, void* user_data);
HParsedToken *h_make(HArena *arena, HTokenType type, void *value);
HParsedToken *h_make_seq(HArena *arena);
HParsedToken *h_make_seqn(HArena *arena, size_t n);
HParsedToken *h_make_bytes(HArena *arena, size_t len);
HParsedToken *h_make_sint(HArena *arena, int64_t val);
HParsedToken *h_make_uint(HArena *arena, uint64_t val);
size_t h_seq_len(const HParsedToken *p);


HParsedToken **h_seq_elements(const HParsedToken *p);


HParsedToken *h_seq_index(const HParsedToken *p, size_t i);


HParsedToken *h_seq_index_path(const HParsedToken *p, size_t i, ...);
HParsedToken *h_seq_index_vpath(const HParsedToken *p, size_t i, va_list va);
HParsedToken *h_carray_index(const HCountedArray *a, size_t i);




void h_seq_snoc(HParsedToken *xs, const HParsedToken *x);
void h_seq_append(HParsedToken *xs, const HParsedToken *ys);




const HParsedToken *h_seq_flatten(HArena *arena, const HParsedToken *p);
typedef long int ptrdiff_t;
typedef int wchar_t;


typedef struct test_object {
    struct {
        char *elem;
        size_t count;
    } i1;
    int i2;
} test_object;




typedef struct foo {
    int name1;
    test_object* object;
} foo;



extern const struct foo *parse_foo(const uint8_t *input, size_t length);
extern const HParser * init_foo();


enum HMacroTokenType_ {
    TT_Macro_unused = TT_USER,


    TT_test_object,




    TT_foo,





};




HParsedToken * act_test_object (const HParseResult *p, void* user_data) {
    int i =0;
    HParsedToken **fields = h_seq_elements(p->ast);
    struct test_object *ret = ((struct test_object *) h_arena_malloc(p->arena, sizeof(struct test_object)));
    ret->i1.count = h_seq_len(fields[i]);
    do {
        int j;
        HParsedToken **seq = h_seq_elements(fields[i]);
        ret->i1.elem = (char *)h_arena_malloc(p->arena, sizeof(char) * ret->i1.count);
        for(j=0; j<ret->i1.count; j++) {
            ret->i1.elem[j] = ((((seq[j]->token_type == (HTokenType)TT_UINT) ? (void) (0) : __assert_fail ("seq[j]->token_type == (HTokenType)TT_UINT", "grammar.h", 5, __PRETTY_FUNCTION__)), seq[j])->uint);
        }
    }
    while(0);
    i++;
    ret->i2 = ((((fields[i]->token_type == (HTokenType)TT_SINT) ? (void) (0) : __assert_fail ("fields[i]->token_type == (HTokenType)TT_SINT", "grammar.h", 5, __PRETTY_FUNCTION__)), fields[i])->sint);
    i++;
    return h_make(p->arena,(HTokenType)TT_test_object, ret);
}




HParsedToken * act_foo (const HParseResult *p, void* user_data) {
    int i =0;
    HParsedToken **fields = h_seq_elements(p->ast);
    struct foo *ret = ((struct foo *) h_arena_malloc(p->arena, sizeof(struct foo)));
    ret->name1 = ((((fields[i]->token_type == (HTokenType)TT_SINT) ? (void) (0) : __assert_fail ("fields[i]->token_type == (HTokenType)TT_SINT", "grammar.h", 11, __PRETTY_FUNCTION__)), fields[i])->sint);
    i++;
    if(fields[i]->token_type == TT_NONE) {
        ret->object = ((void *)0);
        i++;
    }
    else {
        ret->object = ((test_object *) (((fields[i]->token_type == (HTokenType)TT_test_object) ? (void) (0) : __assert_fail ("fields[i]->token_type == (HTokenType)TT_test_object", "grammar.h", 11, __PRETTY_FUNCTION__)), fields[i])->user);
        i++;
    }
    return h_make(p->arena,(HTokenType)TT_foo, ret);
}






const HParser * init_foo () {
    static const HParser *ret = ((void *)0);
    if(ret) return ret;

    HParser *test_object = h_action(h_sequence( h_many1(h_ch_range('a','z')), h_int16(), ((void *)0)),act_test_object,((void *)0));




    HParser *foo = h_action(h_sequence( h_int32(), h_optional(test_object), ((void *)0)),act_foo,((void *)0));



    ret = foo;
    return ret;
}
const struct foo * parse_foo(const uint8_t* input, size_t length) {
    HParseResult * ret = h_parse(init_foo (), input,length);
    if(ret && ret->ast) return (const struct foo *)(ret->ast->user);
    else return ((void *)0);
}
int main()
{

    uint8_t input[102400];
    size_t inputsize;
    const struct foo *result;
    inputsize = fread(input, 1, sizeof(input), stdin);
    fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
    fwrite(input, 1, inputsize, stderr);
    result = parse_foo(input,inputsize);
    if(result) {
        printf("\n%d", result->name1);
        if(result->object) {
            printf("%.*s %d\n", result->object->i1.count,result->object->i1.elem,result->object->i2);
        }
        else
            printf("<none>");
        printf("\n");
        return 0;
    } else {
        return 1;
    }
}
