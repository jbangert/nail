






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





typedef __off_t off_t;
typedef __pid_t pid_t;





typedef __id_t id_t;




typedef __ssize_t ssize_t;





typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;


typedef __clock_t clock_t;





typedef __time_t time_t;



typedef __clockid_t clockid_t;
typedef __timer_t timer_t;
typedef long unsigned int size_t;



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
typedef int int8_t __attribute__ ((__mode__ (__QI__)));
typedef int int16_t __attribute__ ((__mode__ (__HI__)));
typedef int int32_t __attribute__ ((__mode__ (__SI__)));
typedef int int64_t __attribute__ ((__mode__ (__DI__)));


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







struct iovec
{
    void *iov_base;
    size_t iov_len;
};
extern ssize_t readv (int __fd, const struct iovec *__iovec, int __count)
;
extern ssize_t writev (int __fd, const struct iovec *__iovec, int __count)
;
extern ssize_t preadv (int __fd, const struct iovec *__iovec, int __count,
                       __off_t __offset) ;
extern ssize_t pwritev (int __fd, const struct iovec *__iovec, int __count,
                        __off_t __offset) ;







typedef __socklen_t socklen_t;




enum __socket_type
{
    SOCK_STREAM = 1,


    SOCK_DGRAM = 2,


    SOCK_RAW = 3,

    SOCK_RDM = 4,

    SOCK_SEQPACKET = 5,


    SOCK_DCCP = 6,

    SOCK_PACKET = 10,







    SOCK_CLOEXEC = 02000000,


    SOCK_NONBLOCK = 00004000


};
typedef unsigned short int sa_family_t;


struct sockaddr
{
    sa_family_t sa_family;
    char sa_data[14];
};
struct sockaddr_storage
{
    sa_family_t ss_family;
    unsigned long int __ss_align;
    char __ss_padding[(128 - (2 * sizeof (unsigned long int)))];
};



enum
{
    MSG_OOB = 0x01,

    MSG_PEEK = 0x02,

    MSG_DONTROUTE = 0x04,






    MSG_CTRUNC = 0x08,

    MSG_PROXY = 0x10,

    MSG_TRUNC = 0x20,

    MSG_DONTWAIT = 0x40,

    MSG_EOR = 0x80,

    MSG_WAITALL = 0x100,

    MSG_FIN = 0x200,

    MSG_SYN = 0x400,

    MSG_CONFIRM = 0x800,

    MSG_RST = 0x1000,

    MSG_ERRQUEUE = 0x2000,

    MSG_NOSIGNAL = 0x4000,

    MSG_MORE = 0x8000,

    MSG_WAITFORONE = 0x10000,

    MSG_FASTOPEN = 0x20000000,


    MSG_CMSG_CLOEXEC = 0x40000000



};




struct msghdr
{
    void *msg_name;
    socklen_t msg_namelen;

    struct iovec *msg_iov;
    size_t msg_iovlen;

    void *msg_control;
    size_t msg_controllen;




    int msg_flags;
};


struct cmsghdr
{
    size_t cmsg_len;




    int cmsg_level;
    int cmsg_type;

    __extension__ unsigned char __cmsg_data [];

};
extern struct cmsghdr *__cmsg_nxthdr (struct msghdr *__mhdr,
                                      struct cmsghdr *__cmsg) __attribute__ ((__nothrow__ , __leaf__));
enum
{
    SCM_RIGHTS = 0x01





};



struct linger
{
    int l_onoff;
    int l_linger;
};




struct osockaddr
{
    unsigned short int sa_family;
    unsigned char sa_data[14];
};




enum
{
    SHUT_RD = 0,

    SHUT_WR,

    SHUT_RDWR

};
extern int socket (int __domain, int __type, int __protocol) __attribute__ ((__nothrow__ , __leaf__));





extern int socketpair (int __domain, int __type, int __protocol,
                       int __fds[2]) __attribute__ ((__nothrow__ , __leaf__));


extern int bind (int __fd, const struct sockaddr * __addr, socklen_t __len)
__attribute__ ((__nothrow__ , __leaf__));


extern int getsockname (int __fd, struct sockaddr *__restrict __addr,
                        socklen_t *__restrict __len) __attribute__ ((__nothrow__ , __leaf__));
extern int connect (int __fd, const struct sockaddr * __addr, socklen_t __len);



extern int getpeername (int __fd, struct sockaddr *__restrict __addr,
                        socklen_t *__restrict __len) __attribute__ ((__nothrow__ , __leaf__));






extern ssize_t send (int __fd, const void *__buf, size_t __n, int __flags);






extern ssize_t recv (int __fd, void *__buf, size_t __n, int __flags);






extern ssize_t sendto (int __fd, const void *__buf, size_t __n,
                       int __flags, const struct sockaddr * __addr,
                       socklen_t __addr_len);
extern ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n,
                         int __flags, struct sockaddr *__restrict __addr,
                         socklen_t *__restrict __addr_len);







extern ssize_t sendmsg (int __fd, const struct msghdr *__message,
                        int __flags);
extern ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags);
extern int getsockopt (int __fd, int __level, int __optname,
                       void *__restrict __optval,
                       socklen_t *__restrict __optlen) __attribute__ ((__nothrow__ , __leaf__));




extern int setsockopt (int __fd, int __level, int __optname,
                       const void *__optval, socklen_t __optlen) __attribute__ ((__nothrow__ , __leaf__));





extern int listen (int __fd, int __n) __attribute__ ((__nothrow__ , __leaf__));
extern int accept (int __fd, struct sockaddr *__restrict __addr,
                   socklen_t *__restrict __addr_len);
extern int shutdown (int __fd, int __how) __attribute__ ((__nothrow__ , __leaf__));




extern int sockatmark (int __fd) __attribute__ ((__nothrow__ , __leaf__));







extern int isfdtype (int __fd, int __fdtype) __attribute__ ((__nothrow__ , __leaf__));

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







enum
{
    IPPROTO_IP = 0,

    IPPROTO_HOPOPTS = 0,

    IPPROTO_ICMP = 1,

    IPPROTO_IGMP = 2,

    IPPROTO_IPIP = 4,

    IPPROTO_TCP = 6,

    IPPROTO_EGP = 8,

    IPPROTO_PUP = 12,

    IPPROTO_UDP = 17,

    IPPROTO_IDP = 22,

    IPPROTO_TP = 29,

    IPPROTO_DCCP = 33,

    IPPROTO_IPV6 = 41,

    IPPROTO_ROUTING = 43,

    IPPROTO_FRAGMENT = 44,

    IPPROTO_RSVP = 46,

    IPPROTO_GRE = 47,

    IPPROTO_ESP = 50,

    IPPROTO_AH = 51,

    IPPROTO_ICMPV6 = 58,

    IPPROTO_NONE = 59,

    IPPROTO_DSTOPTS = 60,

    IPPROTO_MTP = 92,

    IPPROTO_ENCAP = 98,

    IPPROTO_PIM = 103,

    IPPROTO_COMP = 108,

    IPPROTO_SCTP = 132,

    IPPROTO_UDPLITE = 136,

    IPPROTO_RAW = 255,

    IPPROTO_MAX
};



typedef uint16_t in_port_t;


enum
{
    IPPORT_ECHO = 7,
    IPPORT_DISCARD = 9,
    IPPORT_SYSTAT = 11,
    IPPORT_DAYTIME = 13,
    IPPORT_NETSTAT = 15,
    IPPORT_FTP = 21,
    IPPORT_TELNET = 23,
    IPPORT_SMTP = 25,
    IPPORT_TIMESERVER = 37,
    IPPORT_NAMESERVER = 42,
    IPPORT_WHOIS = 43,
    IPPORT_MTP = 57,

    IPPORT_TFTP = 69,
    IPPORT_RJE = 77,
    IPPORT_FINGER = 79,
    IPPORT_TTYLINK = 87,
    IPPORT_SUPDUP = 95,


    IPPORT_EXECSERVER = 512,
    IPPORT_LOGINSERVER = 513,
    IPPORT_CMDSERVER = 514,
    IPPORT_EFSSERVER = 520,


    IPPORT_BIFFUDP = 512,
    IPPORT_WHOSERVER = 513,
    IPPORT_ROUTESERVER = 520,


    IPPORT_RESERVED = 1024,


    IPPORT_USERRESERVED = 5000
};



typedef uint32_t in_addr_t;
struct in_addr
{
    in_addr_t s_addr;
};
struct in6_addr
{
    union
    {
        uint8_t __u6_addr8[16];

        uint16_t __u6_addr16[8];
        uint32_t __u6_addr32[4];

    } __in6_u;





};

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;
struct sockaddr_in
{
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;


    unsigned char sin_zero[sizeof (struct sockaddr) -
                           (sizeof (unsigned short int)) -
                           sizeof (in_port_t) -
                           sizeof (struct in_addr)];
};


struct sockaddr_in6
{
    sa_family_t sin6_family;
    in_port_t sin6_port;
    uint32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t sin6_scope_id;
};




struct ip_mreq
{

    struct in_addr imr_multiaddr;


    struct in_addr imr_interface;
};

struct ip_mreq_source
{

    struct in_addr imr_multiaddr;


    struct in_addr imr_interface;


    struct in_addr imr_sourceaddr;
};




struct ipv6_mreq
{

    struct in6_addr ipv6mr_multiaddr;


    unsigned int ipv6mr_interface;
};




struct group_req
{

    uint32_t gr_interface;


    struct sockaddr_storage gr_group;
};

struct group_source_req
{

    uint32_t gsr_interface;


    struct sockaddr_storage gsr_group;


    struct sockaddr_storage gsr_source;
};



struct ip_msfilter
{

    struct in_addr imsf_multiaddr;


    struct in_addr imsf_interface;


    uint32_t imsf_fmode;


    uint32_t imsf_numsrc;

    struct in_addr imsf_slist[1];
};





struct group_filter
{

    uint32_t gf_interface;


    struct sockaddr_storage gf_group;


    uint32_t gf_fmode;


    uint32_t gf_numsrc;

    struct sockaddr_storage gf_slist[1];
};
struct ip_opts
{
    struct in_addr ip_dst;
    char ip_opts[40];
};


struct ip_mreqn
{
    struct in_addr imr_multiaddr;
    struct in_addr imr_address;
    int imr_ifindex;
};


struct in_pktinfo
{
    int ipi_ifindex;
    struct in_addr ipi_spec_dst;
    struct in_addr ipi_addr;
};
extern uint32_t ntohl (uint32_t __netlong) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
extern uint16_t ntohs (uint16_t __netshort)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
extern uint32_t htonl (uint32_t __hostlong)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
extern uint16_t htons (uint16_t __hostshort)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));




extern int bindresvport (int __sockfd, struct sockaddr_in *__sock_in) __attribute__ ((__nothrow__ , __leaf__));


extern int bindresvport6 (int __sockfd, struct sockaddr_in6 *__sock_in)
__attribute__ ((__nothrow__ , __leaf__));

typedef __builtin_va_list __gnuc_va_list;








extern void warn (const char *__format, ...)
__attribute__ ((__format__ (__printf__, 1, 2)));
extern void vwarn (const char *__format, __gnuc_va_list)
__attribute__ ((__format__ (__printf__, 1, 0)));


extern void warnx (const char *__format, ...)
__attribute__ ((__format__ (__printf__, 1, 2)));
extern void vwarnx (const char *__format, __gnuc_va_list)
__attribute__ ((__format__ (__printf__, 1, 0)));


extern void err (int __status, const char *__format, ...)
__attribute__ ((__noreturn__, __format__ (__printf__, 2, 3)));
extern void verr (int __status, const char *__format, __gnuc_va_list)
__attribute__ ((__noreturn__, __format__ (__printf__, 2, 0)));
extern void errx (int __status, const char *__format, ...)
__attribute__ ((__noreturn__, __format__ (__printf__, 2, 3)));
extern void verrx (int __status, const char *, __gnuc_va_list)
__attribute__ ((__noreturn__, __format__ (__printf__, 2, 0)));
















extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
                     size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern void *memmove (void *__dest, const void *__src, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));






extern void *memccpy (void *__restrict __dest, const void *__restrict __src,
                      int __c, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));





extern void *memset (void *__s, int __c, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern int memcmp (const void *__s1, const void *__s2, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern void *memchr (const void *__s, int __c, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern char *strcpy (char *__restrict __dest, const char *__restrict __src)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncpy (char *__restrict __dest,
                      const char *__restrict __src, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *strcat (char *__restrict __dest, const char *__restrict __src)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncat (char *__restrict __dest, const char *__restrict __src,
                      size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcmp (const char *__s1, const char *__s2)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern int strncmp (const char *__s1, const char *__s2, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcoll (const char *__s1, const char *__s2)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern size_t strxfrm (char *__restrict __dest,
                       const char *__restrict __src, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));






typedef struct __locale_struct
{

    struct __locale_data *__locales[13];


    const unsigned short int *__ctype_b;
    const int *__ctype_tolower;
    const int *__ctype_toupper;


    const char *__names[13];
} *__locale_t;


typedef __locale_t locale_t;


extern int strcoll_l (const char *__s1, const char *__s2, __locale_t __l)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));

extern size_t strxfrm_l (char *__dest, const char *__src, size_t __n,
                         __locale_t __l) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 4)));





extern char *strdup (const char *__s)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));






extern char *strndup (const char *__string, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));

extern char *strchr (const char *__s, int __c)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern char *strrchr (const char *__s, int __c)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





extern size_t strcspn (const char *__s, const char *__reject)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern size_t strspn (const char *__s, const char *__accept)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *strpbrk (const char *__s, const char *__accept)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *strstr (const char *__haystack, const char *__needle)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));




extern char *strtok (char *__restrict __s, const char *__restrict __delim)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));




extern char *__strtok_r (char *__restrict __s,
                         const char *__restrict __delim,
                         char **__restrict __save_ptr)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));

extern char *strtok_r (char *__restrict __s, const char *__restrict __delim,
                       char **__restrict __save_ptr)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));


extern size_t strlen (const char *__s)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





extern size_t strnlen (const char *__string, size_t __maxlen)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





extern char *strerror (int __errnum) __attribute__ ((__nothrow__ , __leaf__));

extern int strerror_r (int __errnum, char *__buf, size_t __buflen) __asm__ ("" "__xpg_strerror_r") __attribute__ ((__nothrow__ , __leaf__))

__attribute__ ((__nonnull__ (2)));
extern char *strerror_l (int __errnum, __locale_t __l) __attribute__ ((__nothrow__ , __leaf__));





extern void __bzero (void *__s, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));



extern void bcopy (const void *__src, void *__dest, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));


extern void bzero (void *__s, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));


extern int bcmp (const void *__s1, const void *__s2, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *index (const char *__s, int __c)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern char *rindex (const char *__s, int __c)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern int ffs (int __i) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__const__));
extern int strcasecmp (const char *__s1, const char *__s2)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strncasecmp (const char *__s1, const char *__s2, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *strsep (char **__restrict __stringp,
                     const char *__restrict __delim)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));




extern char *strsignal (int __sig) __attribute__ ((__nothrow__ , __leaf__));


extern char *__stpcpy (char *__restrict __dest, const char *__restrict __src)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpcpy (char *__restrict __dest, const char *__restrict __src)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));



extern char *__stpncpy (char *__restrict __dest,
                        const char *__restrict __src, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpncpy (char *__restrict __dest,
                      const char *__restrict __src, size_t __n)
__attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));

typedef __gnuc_va_list va_list;






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
    const char *name;
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
HParseResult* h_parse_error(const HParser *parser, const uint8_t* input, size_t length, FILE *error_fd);
HParseResult* h_parse_error__m(HAllocator* mm__, const HParser *parser, const uint8_t* input, size_t length, FILE *error_fd);






HParser* h_token(const uint8_t *str, const size_t len);
HParser* h_token__m(HAllocator* mm__, const uint8_t *str, const size_t len);







HParser* h_ch(const uint8_t c);
HParser* h_ch__m(HAllocator* mm__, const uint8_t c);
HParser* h_ch_range(const uint8_t lower, const uint8_t upper);
HParser* h_ch_range__m(HAllocator* mm__, const uint8_t lower, const uint8_t upper);






HParser* h_int_range(const HParser *p, const int64_t lower, const int64_t upper);
HParser* h_int_range__m(HAllocator* mm__, const HParser *p, const int64_t lower, const int64_t upper);




HParser * h_strint(const int signed);
HParser * h_strint__m(HAllocator* mm__, const int signed);






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




HParser * h_name(const char *name, HParser *p);



HParser * h_trace(FILE *file, const char*prefix,HParser *p);
HParser * h_trace__m(HAllocator* mm__, FILE *file, const char*prefix,HParser *p);







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
