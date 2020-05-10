#ifndef _OPS_H
#define _OPS_H

#ifndef __ASSEMBLER__

#ifndef NULL
#define NULL                ((void *)0)
#endif

typedef int                 bool;
#define false 0
#define true 1

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({      \
    const typeof(((type *)0)->member) *__mptr = (ptr);  \
    ((type *)((char *)__mptr - __builtin_offsetof(type,member))); })

#define BITS_PER_BYTE		8
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define BITS_PER_TYPE(type)	(sizeof(type) * BITS_PER_BYTE)
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_TYPE(long))

#define BIT_MASK_LONG(nr)            ((unsigned long)(1) << ((nr) % BITS_PER_TYPE(long)))
#define BIT_WORD_LONG(nr)            ((nr) / BITS_PER_TYPE(long))

#define DECLARE_BITMAP(name,bits) \
	unsigned long name[BITS_TO_LONGS(bits)]



/* b의 가장 앞의 1bit 위치를 리턴 
 * __builtin_ctzl함수는 long type의 첫 번째 1bit를 반환하는 builtin함수임 
 */
static inline int sched_find_first_bit(const unsigned long *b)
{
	if (b[0])
		return __builtin_ctzl(b[0]);
	return __builtin_ctzl(b[1]) + 64;
}

/* addr의 nr번째 bit를 1로 setting */
static inline void __set_bit(int nr, volatile unsigned long *addr)
{
        unsigned long mask = BIT_MASK_LONG(nr);
        unsigned long *p = ((unsigned long *)addr) + BIT_WORD_LONG(nr);

        *p  |= mask;
}

/* addr의 nr번째 bit를 return */
static inline int test_bit(int nr, const volatile unsigned long *addr)
{
	return 1UL & (addr[BIT_WORD_LONG(nr)] >> (nr & (BITS_PER_TYPE(long)-1)));
}

#endif
#endif