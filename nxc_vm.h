/**
 * this file delcare the basic type info ...
 * @date:2010-8-27
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 2011 songyuan, aboutin.me@gmail.com
 *
 * 
 * NXCC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NXCC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with NXCC.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __nxc_vm_h__
#define __nxc_vm_h__

/**===========================================================================*/
/**=============================== portion  ==================================*/
/**===========================================================================*/

///just wrapper va_list , REMEMBER to FIX this while running under ALPHA !!!
#if 0
    #define nxc_va_list     char *
    #define va_start(ap,v)  ( ap = (char*)&v + _INTSIZEOF(v) )
    #define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
    #define va_end(ap)
#else
    #include <stdarg.h>
    #define nxc_va_list     va_list
    #define nxc_va_start    va_start
    #define nxc_va_arg      va_arg
    #define nxc_va_end      va_end
#endif

///-----------------------------------------------------------------------------
/**
 * detect compiler env ...
 */
#ifndef __is_windows_env__
    #if defined(_WIN32) ||defined(_WIN64)
        #define __is_windows_env__
    #endif
#endif

#ifndef __is_64bit_env__
    #if defined(_WIN64) || defined (__x86_64__) || defined (_LP64)
        #define __is_64bit_env__
    #endif
#endif

#ifdef __is_windows_env__
    typedef unsigned __int64       __uint64;
    #ifndef inline
        #define inline             __inline
    #endif
#else
    typedef long long              __int64;
    typedef unsigned long long     __uint64;
#endif
///rename to _fast_
#define ___fast                    static inline

#define nxc_char_shift     0
#define nxc_short_shift    1
#define nxc_int_shift      2
#ifdef __is_64bit_env__
    #define nxc_long_shift     3
#else
    #define nxc_long_shift     2
#endif

#define nxc_align(size, align)     ((size + align - 1) & (~(align - 1)))
#define nxc_align_down(size,align) ((size) & (~(align - 1))))
#define nxc_isdigit(c)             (c >= '0' && c <= '9')
#define nxc_isoctdigit(c)          (c >= '0' && c <= '7')
#define nxc_ishexdigit(c)          \
        (nxc_isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
#define nxc_isupper(c)             (c >= 'A' && c <= 'Z')
#define nxc_toupper(c)             (c & ~0x20)
#define nxc_isalpha(c)             \
        ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#define nxc_isspace(c)             \
        (c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\12')
#define nxc_isletter(c)            \
        ((c >= 'a' && c <= 'z') || (c == '_') || (c >= 'A' && c <= 'Z'))
#define nxc_is_letter_or_digit(c)  \
        (nxc_isletter(c) || nxc_isdigit(c))

#define nxc_high_4bit(v)           ((v) >> (8 * sizeof(int) - 4) & 0x0f)
#define nxc_high_3bit(v)           ((v) >> (8 * sizeof(int) - 3) & 0x07)
#define nxc_high_1bit(v)           ((v) >> (8 * sizeof(int) - 1) & 0x01)

#define nxc_add_flag(v,f)          (*(long*)&(v) |= (f))
#define nxc_del_flag(v,f)          (*(long*)&(v) &= (~(f)))
#define nxc_remove_flag(v,f)       (*(long*)&(v) &= (~(f)))
#define nxc_is_flag_set(v,f)       ((long)(v) & (f))
#define nxc_test_flag(v,f)         ((long)(v) & (f))

/**
 * memory wrapper ...
 */
___fast void nxc_memcpy(void*_dst,void*_src,register int len)
{
    register char* dst = (char *)_dst;
    register char* src = (char *)_src;
    while(len--) *dst++=*src++;
}
___fast void nxc_memset(void*_dst,register int c,register int len)
{
    register char *dst = (char *)_dst;
    while(len--) *dst++=c;
}

___fast int nxc_memcmp(void*_dst,void*_src,register int len)
{
    register char* dst = (char *)_dst;
    register char* src = (char *)_src;
    while(len--)
    {
        if(*dst != *src) return *dst - *src;
        dst++;
        src++;
    }
    return 0;
}
___fast int nxc_strlen(register char*str)
{
    register char* ptr = str;
    while(*ptr) ptr++;
    return ptr - str;
}
___fast int nxc_strcmp(register char*str1,register char*str2)
{
    while(*str1 == *str2)
    {
        if(!*str1) return 0;
        str1++;
        str2++;
    }
    return *str1-*str2;
}

///-----------------------------------------------------------------------------










/**===========================================================================*/
/**==============================list library=================================*/
/**===========================================================================*/
#define nxc_offsetof(TYPE,MEMBER) \
        ((long)&((TYPE *)0)->MEMBER)

#define nxc_container_of(ptr,TYPE,MEMBER) \
        ((TYPE *)((char*)(ptr)-nxc_offsetof(TYPE,MEMBER)))

#ifdef __is_windows_env__

    #define likely(x)           (x)
    #define unlikely(x)         (x)

    #ifndef prefetch
        #define prefetch(x)     1
    #endif

#else ///of __is_windows_env__

    #ifdef __i386
        #ifndef likely
            #define likely(x)       __builtin_expect(!!(x), 1)
        #endif

        #ifndef unlikely
            #define unlikely(x)     __builtin_expect(!!(x), 0)
        #endif

        #ifndef prefetch
            #define prefetch(x)     __builtin_prefetch(x)
        #endif
    #else   ///SPARC ...
        #define likely(x)           (x)
        #define unlikely(x)         (x)

        #ifndef prefetch
            #define prefetch(x)     1
        #endif
    #endif

#endif ///of __is_windows_env__

/**
 * ===========================double linked list===============================
 */
/**
 * this is the basis of double-linked list
 */
typedef struct nxc_dlist_head
{
    struct nxc_dlist_head *next,*prev;
}
nxc_dlist_head_t,
nxc_dlist_node_t;

/**
 * init a dlist-head node, point to it self ...
 */
___fast void nxc_init_dlist_head(nxc_dlist_head_t *list)
{
    list->next = list;
    list->prev = list;
}


/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
___fast void __nxc_dlist_add(nxc_dlist_node_t *_new,
                             nxc_dlist_node_t *prev,
                             nxc_dlist_node_t *next)
{
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}


/**
 * add a new entry
 * @_new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
___fast void nxc_dlist_add(nxc_dlist_node_t *_new,nxc_dlist_head_t *head)
{
    __nxc_dlist_add(_new, head, head->next);
}


/**
 * add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
___fast void nxc_dlist_add_tail(nxc_dlist_node_t *_new,nxc_dlist_head_t *head)
{
    __nxc_dlist_add(_new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
___fast void __nxc_dlist_del(nxc_dlist_node_t * prev,nxc_dlist_node_t * next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
___fast void nxc_dlist_del(nxc_dlist_node_t *entry)
{
    __nxc_dlist_del(entry->prev, entry->next);
    entry->next = 0;
    entry->prev = 0;
}

/**
 * tests whether a list is empty
 * @head: the list to test.
 */
___fast int nxc_dlist_empty(const nxc_dlist_head_t *head)
{
    return head->next == head;
}

/**
 * get the struct for this entry
 * @ptr:    the &struct dlist_head pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 */
#define nxc_dlist_entry(ptr, type, member) \
        nxc_container_of(ptr, type, member)

/**
 * get the first element from a list
 * head_ptr:the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define nxc_dlist_first_entry(head_ptr, type, member) \
        nxc_dlist_entry((head_ptr)->next, type , member)

#define nxc_dlist_tail_entry(head_ptr , type , member) \
        nxc_container_of((head_ptr)->prev, type, member)


/**
 * iterate over a list
 * @pos:    the &struct dlist_head to use as a loop cursor.
 * @head:   the head for your list.
 */
#define nxc_dlist_for_each(pos, head) \
        for (pos = (head)->next; \
             prefetch(pos->next), pos != (head); \
             pos = pos->next)
/**
 * iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */


#define nxc_dlist_for_each_entry(pos,head,member, TYPE) \
        for (pos = nxc_dlist_entry((head)->next, TYPE, member);  \
             prefetch(pos->member.next), &pos->member != (head);    \
             pos = nxc_dlist_entry(pos->member.next, TYPE, member))


/**
 * iterate backwards over list of given type.
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define nxc_dlist_for_each_entry_reverse(pos , head , member , TYPE) \
        for (pos = nxc_dlist_entry((head)->prev, TYPE, member);  \
             prefetch(pos->member.prev), &pos->member != (head); \
             pos = nxc_dlist_entry(pos->member.prev, TYPE, member))

/**
 * iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define nxc_dlist_for_each_entry_safe(pos, n, head, member, TYPE) \
        for (pos = nxc_dlist_entry((head)->next, TYPE, member), \
             n = nxc_dlist_entry(pos->member.next, TYPE, member); \
             &pos->member != (head); \
             pos = n, n = nxc_dlist_entry(n->member.next, TYPE, member))

/**
 *  reverse traversal ...
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define nxc_dlist_for_each_entry_safe_reverse(pos, n, head, member, TYPE) \
        for (pos = nxc_dlist_entry((head)->prev, TYPE, member), \
             n = nxc_dlist_entry(pos->member.prev, TYPE, member); \
             &pos->member != (head); \
             pos = n, n = nxc_dlist_entry(n->member.prev, TYPE, member))

/**
 * ========================double linked hash list=============================
 */
/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */
typedef struct nxc_hlist_node
{
    struct nxc_hlist_node *next, **pprev;
}
nxc_hlist_node_t;

typedef struct nxc_hlist_head
{
    struct nxc_hlist_node *first;
}
nxc_hlist_head_t;

#define nxc_init_hlist_head(ptr)         ((ptr)->first = 0)

___fast void nxc_init_hlist_node(nxc_hlist_node_t *h)
{
//     h->next = 0;
//     h->pprev = 0;
    h->next = 0;
    /**
    * set prev to myself to enable del out of list ...
    */
    ///@rev:2010-12-9 to enable re-del ...
    h->pprev = &h->next;
}

___fast void __nxc_hlist_del(nxc_hlist_node_t *n)
{
    nxc_hlist_node_t *next = n->next;
    nxc_hlist_node_t **pprev = n->pprev;
    *pprev = next;
    if (next)
        next->pprev = pprev;
}

___fast void nxc_hlist_del(nxc_hlist_node_t *n)
{
    __nxc_hlist_del(n);
    n->next = 0;
    n->pprev = 0;
}

___fast void nxc_hlist_add_head(nxc_hlist_node_t *n,nxc_hlist_head_t *head)
{
    nxc_hlist_node_t *first = head->first;
    n->next = first;
    if (first)
        first->pprev = &n->next;
    head->first = n;
    n->pprev = &head->first;
}

#define nxc_hlist_entry(ptr, type, member) nxc_container_of(ptr,type,member)

/**
 * iterate over list of given type
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the hlist_node within the struct.
 */
#define nxc_hlist_for_each_entry(tpos, pos, head, member, TYPE ) \
        for (pos = (head)->first; \
             pos && (tpos = nxc_hlist_entry(pos, TYPE, member)); \
             pos = pos->next)


/**
 * iterate over list of given type safe against removal of list entry
 * @tpos:   the type * to use as a loop cursor.
 * @pos:    the &struct hlist_node to use as a loop cursor.
 * @n:      another &struct hlist_node to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the hlist_node within the struct.
 */
#define nxc_hlist_for_each_entry_safe(tpos, pos, n, head, member, TYPE) \
        for (pos = (head)->first; \
             pos && (n = pos->next) , \
             pos && (tpos = nxc_hlist_entry(pos, TYPE, member)); \
             pos = n)
///-----------------------------------------------------------------------------















/**===========================================================================*/
/**=============================memory tracer=================================*/
/**===========================================================================*/

struct _nxc_mempool;

/**
 * memory block header (sould be 16Byte aligned)...
 */
typedef struct _nxc_memblk
{
    nxc_dlist_head_t     list_head;///trace-list-node ...
    struct _nxc_mempool *pool;///container of this block ...
    long                 size; ///block size (with header included)...
}nxc_memblk_t;

///memory allocator
typedef void *(*nxc_allocator_t)(void*ctx,int size);
///memory deallocator
typedef void  (*nxc_deallocator_t)(void *ptr,int size);

/**
 * memory pool ...
 */
typedef struct _nxc_mempool
{
    nxc_dlist_head_t  busy_list;       ///trace all busy block ...
    nxc_allocator_t   do_malloc;       ///allocator interface ...
    nxc_deallocator_t do_free;         ///deallocator interface ...
    void             *allocator_data;
    long              busy_size;       ///trace total memory allocated ...
    unsigned long     max_size;        ///limit max size(total-busy size)...
    long              pad0;
}
nxc_mempool_t;

/**===========================================================================*/

/**
 * init mempool from given buffer ...
 * return pool object ...
 */
___fast nxc_mempool_t *nxc_init_mpool(nxc_mempool_t *pool,
                                      nxc_allocator_t do_malloc,
                                      nxc_deallocator_t do_free,
                                      void*allocator_data)
{
    nxc_memset(pool,0,sizeof(*pool));
    nxc_init_dlist_head(&pool->busy_list);
    pool->do_malloc= do_malloc;
    pool->do_free = do_free;
    pool->allocator_data = allocator_data;
    pool->max_size = (unsigned long)-1;
    return pool;
}

/**
 * alloc a blk from pool ...
 */
___fast void* nxc_pmalloc(nxc_mempool_t *pool,int size)
{
    nxc_memblk_t *blk;

    ///[1].limit max size here ...
    if((unsigned long)pool->busy_size >= pool->max_size) return 0;

    ///[2].alloc a new block here ...
    blk=(nxc_memblk_t*)pool->do_malloc(pool->allocator_data,sizeof(*blk)+size);
    if(!blk) return 0;
    ///[3].init info ...
    blk->size = (long) sizeof(*blk) + (long)size;
    blk->pool = pool;
    ///[4].trace it ...
    nxc_dlist_add(&blk->list_head,&pool->busy_list);
    pool->busy_size += (long) sizeof(*blk) + (long)size;

    return &blk[1];
}

/**
 * release blk from pool ...
 */
___fast void nxc_pfree(void *ptr)
{
    nxc_memblk_t *blk;
    nxc_mempool_t *pool;

    //if(!ptr) return;
    blk = (nxc_memblk_t *)((long)ptr - sizeof(*blk));
    pool = blk->pool;
    //if(!blk->pool) return;

    nxc_dlist_del(&blk->list_head);
    pool->busy_size -= blk->size;
    
    pool->do_free(blk,blk->size);
}

/**
 * release all memory block in pool ...
 */
___fast nxc_mempool_t *nxc_clear_mpool(nxc_mempool_t *pool)
{
    nxc_memblk_t *blk,*next;

    nxc_dlist_for_each_entry_safe(blk,next,&pool->busy_list,list_head,nxc_memblk_t)
    {
        nxc_dlist_del(&blk->list_head);
        pool->busy_size -= blk->size;
        pool->do_free(blk,blk->size);
    }

    ///here , total size might be a non-zero number (should not be)...
    ///reset just incase ...
    nxc_init_dlist_head(&pool->busy_list);
    pool->busy_size = 0;
    return pool;
}
///-----------------------------------------------------------------------------









/**===========================================================================*/
/**===========================hash table======================================*/
/**===========================================================================*/

///general hash node ...
typedef struct _nxc_hash_node
{
    nxc_hlist_node_t list_node;///hash list node
    int  hash;                 ///hash of key
    int  klen;                 ///length of key
    void*key;                  ///key pointer
    void*data;                 ///data pointer
}
nxc_hash_node_t;

///compare two hash node , return 0 means equal ...
typedef int(*nxc_hash_cmp_t)(nxc_hash_node_t *n1,nxc_hash_node_t *n2);
///hash func , hash table will call this routine with hash node ...
typedef int(*nxc_hash_func_t)(nxc_hash_node_t *node);
///release hash node , hash table will call this to release a removed hash-node.
typedef void(*nxc_hash_del_t)(nxc_hash_node_t *node,void*context);

///hash table object ...
typedef struct _nxc_hash_table
{
    nxc_hash_func_t   hash_func;    ///hash callback ...
    nxc_hash_cmp_t    cmp_func;     ///compare callback ...
    int               total_count;  ///total node count
    int               bucket_count; ///hash bucket size
    nxc_hlist_head_t *buckets;      ///point to the bucket array ...
    nxc_mempool_t    *mem_pool;     ///general mempool of this hash table ...
}
nxc_hash_table_t;

/**===========================================================================*/

/**
 * find target node by a dummy node ...
 * return target node ptr ...
 */
___fast nxc_hash_node_t*nxc_hash_find(nxc_hash_table_t*ht,nxc_hash_node_t*dummy)
{
    int hash;
    int idx;
    nxc_hlist_node_t *ptr;
    register nxc_hash_node_t *node;

    hash = ht->hash_func(dummy);
    idx = hash & (ht->bucket_count - 1);

    nxc_hlist_for_each_entry(node,ptr,&ht->buckets[idx],list_node,nxc_hash_node_t)
    {
        if( node->klen == dummy->klen && ///compare key length
            (node->hash == hash) &&      ///compare hash
            !ht->cmp_func(node,dummy))   ///compare key_data
        {
            return node;
        }
    }

    return 0;
}
/**
 * calc hash oof target node ,
 * and add hash node into table directly ...
 */
___fast nxc_hash_node_t *nxc_hash_add(nxc_hash_table_t *ht,nxc_hash_node_t *new_node)
{
    int hash;
    int idx;

    hash = ht->hash_func(new_node);
    idx = hash & (ht->bucket_count - 1);

    ///klen is set by caller ...
    new_node->hash = hash;
    nxc_hlist_add_head(&new_node->list_node,&ht->buckets[idx]);
    ht->total_count ++;

    return new_node;
}

/**
 * remove a hash node from hash table directly ...
 */
___fast nxc_hash_node_t *nxc_hash_remove(nxc_hash_table_t *ht,nxc_hash_node_t *node)
{
    nxc_hlist_del(&node->list_node);
    ht->total_count --;

    return node;
}

/**
 * string purpose only ...
 */
___fast unsigned int nxc_elf_hash(char* str, unsigned int len)
{
    register unsigned int hash = 0;
    register unsigned int x    = 0;
    register unsigned int i    = 0;
    
    for(i = 0; i < len; str++, i++)
    {
        hash = (hash << 4) + (*str);
        if((x = hash & 0xF0000000L) != 0)
        {
            hash ^= (x >> 24);
        }
        hash &= ~x;
    }
    
    return hash;
}

/**
 * default hash node hash func ...
 */
static int nxc_default_hash_func(nxc_hash_node_t *node)
{
    return (int)nxc_elf_hash(node->key,node->klen);
}

/**
 * default hash node cmp func ...
 * return 0 means okay ...
 */
static int nxc_default_hash_cmp_func(nxc_hash_node_t *n1,nxc_hash_node_t *n2)
{
    return nxc_memcmp(n1->key,n2->key,n1->klen);
}
/**
 * init a hash node ...
 */
static void nxc_init_hash_node(nxc_hash_node_t*node,void*key,int klen,void*data)
{
    nxc_memset(node,0,sizeof(*node));
    nxc_init_hlist_node(&node->list_node);
    node->key  = key;
    node->klen = klen;
    node->data = data;
}

/**
 * inti a hash table ...
 * if hash_func is zero , elf_hash() will be used by default ...
 * if cmp_func is zero  , memcmp()   will be used by default ...
 * !!!ht->buckets should be set before calling this routine ...
 * return hash_table ptr ...
 */
static nxc_hash_table_t *nxc_init_hash_table(nxc_hash_table_t *ht,
                                             nxc_mempool_t *mem_pool,
                                             int bucket_cnt,
                                             nxc_hash_func_t hash_func,
                                             nxc_hash_cmp_t  cmp_func)
{
    int i;

    if(!ht||!ht->buckets) return 0;
    ht->bucket_count = bucket_cnt;
    if(ht->bucket_count<1) ht->bucket_count = 128;
    ht->cmp_func = cmp_func;
    if(!ht->cmp_func) ht->cmp_func = nxc_default_hash_cmp_func;
    ht->hash_func = hash_func;
    if(!ht->hash_func) ht->hash_func = nxc_default_hash_func;
    ht->total_count = 0;
    ht->mem_pool = mem_pool;
    for(i=0;i<ht->bucket_count;i++)
    {
        nxc_init_hlist_head(&ht->buckets[i]);
    }
    
    return ht;
}

/**
 * layout 
 * +--------+---------+-------+----+
 * | ht     | bucket0 |bucket1|... |
 * +--------+---------+-------+----+
 * return 0 means error ...
 */
static nxc_hash_table_t *nxc_init_builtin_hash_table(nxc_hash_table_t *ht,
                                                     nxc_mempool_t *mem_pool,
                                                     int bucket_cnt,
                                                     nxc_hash_func_t hash_func,
                                                     nxc_hash_cmp_t  cmp_func)
{
    ht->buckets = (nxc_hlist_head_t *)&ht[1];
    if(nxc_init_hash_table(ht,mem_pool,bucket_cnt,hash_func,cmp_func))return ht;
    return 0;
}

/**
 * create a hash table ...
 * if hash_func is zero , elf_hash() will be used by default ...
 * if cmp_func is zero  , memcmp()   will be used by default ...
 * return hash_table ptr ...
 */
static nxc_hash_table_t *nxc_create_hash_table(nxc_mempool_t *mem_pool,
                                               int bucket_cnt,
                                               nxc_hash_func_t hash_func,
                                               nxc_hash_cmp_t  cmp_func)
{
    nxc_hash_table_t *ht;
    if(bucket_cnt<1) bucket_cnt = 128;

    ht = (nxc_hash_table_t *)nxc_pmalloc(mem_pool,
                        sizeof(*ht) + bucket_cnt * sizeof(nxc_hlist_head_t));
    if(!ht) return 0;
    ht->buckets = (nxc_hlist_head_t *)&ht[1];

    if(nxc_init_hash_table(ht,mem_pool,bucket_cnt,hash_func,cmp_func))return ht;
    nxc_pfree(ht);
    return 0;
}

/**
 * release hash table ...
 * del_func will be called on each node in hash table ...
 * ht object won't be released!!!
 * return 0 means okay ...
 */
static nxc_hash_table_t *nxc_release_hash_table(nxc_hash_table_t *ht,
                                                nxc_hash_del_t del_func,
                                                void *context)
{
    nxc_hlist_node_t *ptr,*ptr1;
    nxc_hash_node_t *node;
    int i;
    if(!ht||!del_func) return ht;

    for(i=0;i<ht->bucket_count;i++)
    {
        ///traversal list and do deleting ...
        nxc_hlist_for_each_entry_safe(node,
                                      ptr,
                                      ptr1,
                                      &ht->buckets[i],
                                      list_node,
                                      nxc_hash_node_t)
        {
            nxc_hlist_del(&node->list_node);
            del_func(node,context);
            ht->total_count --;
        }
        ///reinit ...
        nxc_init_hlist_head(&ht->buckets[i]);
    }
    return 0;
}
///-----------------------------------------------------------------------------








/**===========================================================================*/
/**=================================lexer object==============================*/
/**===========================================================================*/

struct _nxc_str_node;
typedef struct _nxc_str_node nxc_str_node_t;

struct _nxc_token;
typedef struct _nxc_token nxc_token_t;

struct _nxc_lexer_state;
typedef struct _nxc_lexer_state nxc_lexer_state_t;

struct _nxc_lexer;
typedef struct _nxc_lexer nxc_lexer_t;

/**
 * string node ...
 */
struct _nxc_str_node
{
    nxc_hash_node_t  hash_node;
    nxc_dlist_head_t trace_node; ///to trace static string ...
#if 0
    ///we reuse the hash-node's {key,data} field ...
    char*name;
    char*ext_data;
#endif
    short   type;
    short   ext_type;
    int     flag;
};
///string data info ...
#define nxc_str_node_get_str(node)            ((char *)((node)->hash_node.key))
#define nxc_str_node_set_str(node,_str)       (node)->hash_node.key=(_str)
///extened data info...
#define nxc_str_node_get_ext_data(node)       ((char *)((node)->hash_node.data))
#define nxc_str_node_set_ext_data(node,_data) (node)->hash_node.data=(_data)
///access trace node
#define nxc_str_node_get_trace_node(node)     (&(node)->trace_node)
#define nxc_str_node_init_trace_node(node)    nxc_init_dlist_head(&(node)->trace_node)
///string length ...
#define nxc_str_node_get_len(node)            ((node)->hash_node.klen)
#define nxc_str_node_set_len(node,_len)       (node)->hash_node.klen=(_len)
///node's major type 
#define nxc_str_node_get_type(node)           ((node)->type)
#define nxc_str_node_set_type(node,_type)     (node)->type=(_type)
///node's minor type
#define nxc_str_node_get_ext_type(node)       ((node)->ext_type)
#define nxc_str_node_set_ext_type(node,_type) (node)->ext_type=(_type)
///node's flag ...
#define nxc_str_node_has_flag(node,_flag)    nxc_is_flag_set((node)->flag,_flag)
#define nxc_str_node_add_flag(node,_flag)    nxc_add_flag((node)->flag,_flag)
#define nxc_str_node_del_flag(node,_flag)    nxc_remove_flag((node)->flag,_flag)

/**
 * Token ID Table ...
 */
enum _nxc_token_id
{
    nxc_token_bad=0,
    nxc_token_sizeof,
    
    nxc_token_var,
    nxc_token_func,
    nxc_token_typedef,
    nxc_token_struct,
    nxc_token_class,
    nxc_token_union,
    nxc_token_const,
    nxc_token_api,
    
    nxc_token_id,           ///identifier:var,function,typename...

    nxc_token_const_long,
    nxc_token_const_float,
    nxc_token_const_string, ///64bit unsigned

    nxc_token_goto,
    nxc_token_break,
    nxc_token_continue,
    
    nxc_token_if,
    nxc_token_else,
    
    nxc_token_for,
    nxc_token_do,
    nxc_token_while,
    
    nxc_token_return,
    ///--------------assignment_operators------------
    nxc_token_assign,        ///"="
    nxc_token_add_assign,    ///"+="
    nxc_token_sub_assign,    ///"-="
    nxc_token_mul_assign,    ///"*="
    nxc_token_div_assign,    ///"/="
    nxc_token_mod_assign,    ///"%="
    nxc_token_bitor_assign,  ///"|="
    nxc_token_bitand_assign, ///"&="
    nxc_token_xor_assign,    ///"^="
    nxc_token_lshift_assign, ///"<<="
    nxc_token_rshift_assign, ///">>="
    ///--------------binary_operators---------------
    nxc_token_or,            ///"||"
    nxc_token_and,           ///"&&"
    nxc_token_bit_or,        ///"|"
    nxc_token_bit_and,       ///"&"
    nxc_token_xor,           ///"^"
    nxc_token_equal,         ///"=="
    nxc_token_unequal,       ///"!="
    nxc_token_great_than,    ///">"
    nxc_token_less_than,     ///"<"
    nxc_token_great_equal,   ///">="
    nxc_token_less_equal,    ///"<="
    nxc_token_left_shift,    ///"<<"
    nxc_token_right_shift,   ///">>"
    nxc_token_add,           ///"+"
    nxc_token_sub,           ///"-"
    nxc_token_mul,           ///"*"
    nxc_token_div,           ///"/"
    nxc_token_mod,           ///"%"
    ///---------------unary_operator(part)-----------
    nxc_token_cast,          ///'CAST' dummy stub ...
    nxc_token_inc,           ///"++"
    nxc_token_dec,           ///"--"
    nxc_token_not,           ///"!"
    nxc_token_compensation,  ///"~"
    nxc_token_at,            ///"@"
    nxc_token_left_paren,    ///"("
    nxc_token_right_paren,   ///")"
    nxc_token_left_bracket,  ///"["
    nxc_token_right_bracket, ///"]"
    nxc_token_left_bracket1, ///"[["
    nxc_token_right_bracket1,///"]]"
    nxc_token_left_bracket2, ///"[[["
    nxc_token_right_bracket2,///"]]]"
    nxc_token_left_brace,    ///"{"
    nxc_token_right_brace,   ///"}"
    nxc_token_dot,           ///"."
    nxc_token_ptr_right,     ///"->"
    nxc_token_ellipsis,      ///"..."
    nxc_token_comma,         ///","
    nxc_token_colon,         ///":"
    nxc_token_semicolon,     ///";"
    ///---
    nxc_token_eof=0x80,      ///"\x80"
    nxc_token_max,
};

/**
 * value object ...
 */
struct _nxc_token
{
    int xtype;
    int xlen;
    union       ///keep 64 bit aligned ...
    {
        int             int_val;
        long            long_val;
        float           float_val;
        char*           str_val;
        nxc_str_node_t* str_node;
    }u;
};
#define nxc_token_get_len(token)             ((token)->xlen)
#define nxc_token_set_len(token,_len)        ((token)->xlen)=(_len)

#define nxc_token_get_type(token)            ((token)->xtype)
#define nxc_token_set_type(token,_type)      ((token)->xtype=(_type))

#define nxc_token_get_int_val(token)         ((token)->u.int_val)
#define nxc_token_set_int_val(token,_val)    ((token)->u.int_val)=(_val)

#define nxc_token_get_long_val(token)        ((token)->u.long_val)
#define nxc_token_set_long_val(token,_val)   ((token)->u.long_val)=(_val)

#define nxc_token_get_float_val(token)       ((token)->u.float_val)
#define nxc_token_set_float_val(token,_val)  ((token)->u.float_val)=(_val)

#define nxc_token_get_str_val(token)         (nxc_str_node_get_str((token)->u.str_node))
#define nxc_token_set_str_val(token,_str)    ((token)->u.str_val)=(_str)
///internal only ...
#define __nxc_token_get_str_val(token)       ((token)->u.str_val)
#define __nxc_token_set_str_val(token,_str)  ((token)->u.str_val)=(_str)

#define nxc_token_get_str_node(token)        ((token)->u.str_node)
#define nxc_token_set_str_node(token,_node)  ((token)->u.str_node)=(_node)
///copy src_token to me ...
#define nxc_token_copy(token,src_tok)        nxc_memcpy((token),(src_tok),sizeof(nxc_token_t))

/**
 * lexer object ...
 */
struct _nxc_lexer
{
    char *input;          ///source string ...
    char *fname;          ///source file ...
    int   input_sz;       ///source string's max size
    int   pad0;
    char *err_msg;
    ///-------------------------------------------------------------------------
    char* parse_buff;     ///parse buffer ...
    int   name_len;       ///token real length ...
    int   name_maxlen;    ///buffer max size...
    ///-------------------------------------------------------------------------
    int   line_number;    ///current line number ...
    int   line_offset;    ///current offset in line ...
    int   position;
    int   curr_char;
    int   pad1;
    int   token_type;     ///current token type
    nxc_token_t token[1]; ///save current token info ...
    char  def_parse_buff[64];
};

/**
 * @return 0 means error ...
 */
___fast int nxc_lex_init(nxc_lexer_t *lexer)
{
    nxc_memset(lexer,0,sizeof(*lexer));
    return 0;
}

///set input string ...
___fast void nxc_lex_set_input(nxc_lexer_t *lexer,char*str)
{
    lexer->input    = str;
    lexer->input_sz = nxc_strlen(str);
}
///set current file name ...just for debug ouput ...
___fast void nxc_lex_set_filename(nxc_lexer_t *lexer,char*filename)
{
    if (!filename || !filename[0]) filename = "N/A";
    lexer->fname = filename;
}

///get a char ..
///@return 0 means EOF reached
___fast int nxc_lex_getc(nxc_lexer_t *lexer)
{
    int c = 0;
    if (lexer->position < lexer->input_sz)
    {
        if (lexer->curr_char == '\n')
        {
            lexer->line_number ++;
        }
        c = lexer->input[lexer->position++];
        lexer->curr_char = c;   
    }
    return c;
}
///rollback a char 
___fast void nxc_lex_ungetc(nxc_lexer_t *lexer)
{
    if(lexer->position>0 && lexer->curr_char/*EOF check*/)
    {
        lexer->position--;
        lexer->curr_char = lexer->input[lexer->position];
        if (lexer->curr_char == '\n')
        {
            lexer->line_number --;
        }
    }
}
___fast void  nxc_lex_set_err_msg(nxc_lexer_t *lexer,char*msg){lexer->err_msg = msg;}
___fast char* nxc_lex_get_err_msg(nxc_lexer_t *lexer){return lexer->err_msg;}
___fast int   nxc_lex_get_namelen(nxc_lexer_t *lex){return lex->name_len;}
___fast void  nxc_lex_set_namelen(nxc_lexer_t *lex,int _len){lex->name_len=_len;}
___fast void  nxc_lex_clear_namebuf(nxc_lexer_t *lex){lex->name_len = 0;}
___fast void  nxc_lex_set_parsebuf(nxc_lexer_t *lex,char*buff,int maxlen)
{
    lex->parse_buff   = buff;
    lex->name_len    = 0;
    lex->name_maxlen = maxlen;
    buff[0] = 0;
    buff[maxlen - 1] = 0; ///always zero terminated ...
}
///push a char to name buffer ...
___fast void nxc_lex_push_char(nxc_lexer_t *lex,int c)
{
    if (lex->parse_buff && lex->name_maxlen)
    {
        if(lex->name_len<lex->name_maxlen)
        {
            lex->parse_buff[lex->name_len] = (char)c;
        }
    }
    ///update counter only ...
    lex->name_len ++;
}

___fast void nxc_lex_set_token_type(nxc_lexer_t *lexer,int _type)
{
    lexer->token_type=_type;
    nxc_token_set_type(lexer->token,_type);
}
#define nxc_lex_get_token_type(lex)       ((lex)->token_type)
///copy tolen value
___fast void nxc_lex_copy_token(nxc_lexer_t *lexer,nxc_token_t *tok)
{
    lexer->token_type = nxc_token_get_type(tok);
    nxc_token_copy(lexer->token,tok);
}

/**===========================================================================*/










/**===========================================================================*/
/**==============================compiler & vm================================*/
/**===========================================================================*/

struct _nxc_type;
typedef struct _nxc_type nxc_type_t;

struct _nxc_const_node;
typedef struct _nxc_const_node nxc_const_node_t;

struct _nxc_symbol;
typedef struct _nxc_symbol nxc_sym_t;

struct _nxc_ast;
typedef struct _nxc_ast nxc_ast_t;

struct _nxc_instr;
typedef struct _nxc_instr nxc_instr_t;

struct _nxc_compiler;
typedef struct _nxc_compiler nxc_compiler_t;

typedef struct _nxc_compiler nxc_vm_t;
struct _nxc_vm_ctx;
typedef struct _nxc_vm_ctx nxc_vm_ctx_t;

struct _nxc_extern_sym;
typedef struct _nxc_extern_sym nxc_extern_sym_t;

/**
 * AST Opcode Table ...
 */
enum _nxc_ast_opcdoe
{
    nxc_op_bad = 0,
    ///---------------------------expression opcode-----------------------------
    nxc_op_comma,        ///","
    ///----------------------------assignment opcode----------------------------    
    nxc_op_assign,       ///"="
    nxc_op_add_assign,   ///"+="
    nxc_op_sub_assign,   ///"-="
    nxc_op_mul_assign,   ///"*="
    nxc_op_div_assign,   ///"/="
    nxc_op_mod_assign,   ///"%="
    nxc_op_bitor_assign, ///"|="
    nxc_op_bitand_assign,///"&="
    nxc_op_xor_assign,   ///"^="
    nxc_op_lshift_assign,///"<<="
    nxc_op_rshift_assign,///">>="
    ///----------------------------binarry opcode-------------------------------
    nxc_op_or,           ///"||"
    nxc_op_and,          ///"&&"
    nxc_op_bit_or,       ///"|"
    nxc_op_bit_and,      ///"&"
    nxc_op_xor,          ///"^"
    nxc_op_equal,        ///"=="
    nxc_op_unequal,      ///"!="
    nxc_op_great,        ///">"
    nxc_op_less,         ///"<"
    nxc_op_great_equal,  ///">="
    nxc_op_less_equal,   ///"<="
    nxc_op_left_shift,   ///"<<"
    nxc_op_right_shift,  ///">>"
    nxc_op_add,          ///"+"
    nxc_op_sub,          ///"-"
    nxc_op_mul,          ///"*"
    nxc_op_div,          ///"/"
    nxc_op_mod,          ///"%"
    ///----------------------------unary opcode---------------------------------
    nxc_op_inc,          ///"++"
    nxc_op_dec,          ///"--"
    nxc_op_positive,     ///"+"
    nxc_op_negative,     ///"-"
    nxc_op_compensation, ///"~"
    nxc_op_not,          ///"!"
    ///-------------------------------------------------------------------------
    nxc_op_address,      ///"&"
    nxc_op_sizeof,       ///"sizeof"
    nxc_op_cast,         ///"cast"
    ///-------------------------------------------------------------------------
    nxc_op_index8,       ///"[["       --->char
    nxc_op_index16,      ///"[[[["     --->int
    nxc_op_index32,      ///"[[["      --->short
    nxc_op_index_l,      ///"["        --->long
    ///-------------------------------------------------------------------------
    nxc_op_call,         ///"call"
    nxc_op_member,       ///"."
    nxc_op_ptr_right,    ///"->"
    ///----------------------------------factor---------------------------------
    nxc_op_id,           ///"$ID"
    nxc_op_const,        ///"const"
    ///----------------------------------statement------------------------------
    nxc_op_expression,
    nxc_op_if,
    nxc_op_else,
    nxc_op_for,
    nxc_op_while,
    nxc_op_do,
    nxc_op_break,
    nxc_op_continue,
    nxc_op_return,
    nxc_op_block,
    nxc_op_max,
};

/**
 * VM Instruction Opcode Table ...
 * !!! marco opcode for virtual machine !!!
 */
enum _nxc_vm_opcode
{
    nxc_vmop_halt = 0,             ///stop
    ///~~~~~~~~~~~~~~~~~~~~~~~LOAD Instruction~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    nxc_vmop_ld8,                  ///load char to reg (address in reg)
    nxc_vmop_ld16,                 ///load short to reg (address in reg)
    nxc_vmop_ld32,                 ///load int to reg (address in reg)
    nxc_vmop_ld_l,                 ///"*"
    ///-------------------------------------------
    nxc_vmop_ldx8,                 ///reg1 = *(char *)(reg1+offset)
    nxc_vmop_ldx16,                ///reg1 = *(short*)(reg1+offset)
    nxc_vmop_ldx32,                ///reg1 = *(int  *)(reg1+offset)
    nxc_vmop_ldx_l,                ///reg1 = *(long *)(reg1+offset)
    ///-------------------------------------------
    nxc_vmop_index8,               ///reg=[reg+index]
    nxc_vmop_index16,              ///reg=[reg+index*2]
    nxc_vmop_index32,              ///reg=[reg+index*4]
    nxc_vmop_index_l,              ///reg=[reg+index*4]
    ///-------------------------------------------
    nxc_vmop_ld_lvar,              ///reg=[ebp+offset]
    nxc_vmop_ld_lvar_addr,         ///reg=ebp+offset
    nxc_vmop_ld_gvar,              ///reg=[offset]
    nxc_vmop_ld_gvar_addr,         ///reg=addr
    nxc_vmop_ldi,                  ///load immediate(func addr,const str,number)
    nxc_vmop_addi,                 ///reg=reg+immediate
    ///-------------------------------------------
    nxc_vmop_st_lvar,              ///"="
    nxc_vmop_add_st_lvar,          ///"+="
    nxc_vmop_sub_st_lvar,          ///"-="
    nxc_vmop_mul_st_lvar,          ///"*="
    nxc_vmop_div_st_lvar,          ///"/="
    nxc_vmop_mod_st_lvar,          ///"%="
    nxc_vmop_bor_st_lvar,          ///"|="
    nxc_vmop_band_st_lvar,         ///"&="
    nxc_vmop_xor_st_lvar,          ///"^="
    nxc_vmop_lshift_st_lvar,       ///"<<="
    nxc_vmop_rshift_st_lvar,       ///">>="
    ///~~~~~~~~~~~~~~~~~~~~~~~STORE Instruction~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    nxc_vmop_st8,                  ///"="
    nxc_vmop_add_st8,              ///"+="
    nxc_vmop_sub_st8,              ///"-="
    nxc_vmop_mul_st8,              ///"*="
    nxc_vmop_div_st8,              ///"/="
    nxc_vmop_mod_st8,              ///"%="
    nxc_vmop_bor_st8,              ///"|="
    nxc_vmop_band_st8,             ///"&="
    nxc_vmop_xor_st8,              ///"^="
    nxc_vmop_lshift_st8,           ///"<<="
    nxc_vmop_rshift_st8,           ///">>="
    ///--------------              
    nxc_vmop_st16,                 ///"="
    nxc_vmop_add_st16,             ///"+="
    nxc_vmop_sub_st16,             ///"-="
    nxc_vmop_mul_st16,             ///"*="
    nxc_vmop_div_st16,             ///"/="
    nxc_vmop_mod_st16,             ///"%="
    nxc_vmop_bor_st16,             ///"|="
    nxc_vmop_band_st16,            ///"&="
    nxc_vmop_xor_st16,             ///"^="
    nxc_vmop_lshift_st16,          ///"<<="
    nxc_vmop_rshift_st16,          ///">>="
    ///--------------              
    nxc_vmop_st32,                 ///"="
    nxc_vmop_add_st32,             ///"+="
    nxc_vmop_sub_st32,             ///"-="
    nxc_vmop_mul_st32,             ///"*="
    nxc_vmop_div_st32,             ///"/="
    nxc_vmop_mod_st32,             ///"%="
    nxc_vmop_bor_st32,             ///"|="
    nxc_vmop_band_st32,            ///"&="
    nxc_vmop_xor_st32,             ///"^="
    nxc_vmop_lshift_st32,          ///"<<="
    nxc_vmop_rshift_st32,          ///">>="
    ///--------------              
    nxc_vmop_st_l,                 ///"="
    nxc_vmop_add_st_l,             ///"+="
    nxc_vmop_sub_st_l,             ///"-="
    nxc_vmop_mul_st_l,             ///"*="
    nxc_vmop_div_st_l,             ///"/="
    nxc_vmop_mod_st_l,             ///"%="
    nxc_vmop_bor_st_l,             ///"|="
    nxc_vmop_band_st_l,            ///"&="
    nxc_vmop_xor_st_l,             ///"^="
    nxc_vmop_lshift_st_l,          ///"<<="
    nxc_vmop_rshift_st_l,          ///">>="
    ///--------------
    nxc_vmop_inc,                  ///"++"
    nxc_vmop_dec,                  ///"--"
    ///------------------------Binary opcode------------------------------------
    nxc_vmop_or,                   ///"||"
    nxc_vmop_and,                  ///"&&"
    nxc_vmop_bor,                  ///"|"
    nxc_vmop_band,                 ///"&"
    nxc_vmop_xor,                  ///"^"
    nxc_vmop_eq,                   ///"=="
    nxc_vmop_uneq,                 ///"!="
    nxc_vmop_gt,                   ///">"
    nxc_vmop_lt,                   ///"<"
    nxc_vmop_ge,                   ///">="
    nxc_vmop_le,                   ///"<="
    nxc_vmop_lshift,               ///"<<"
    nxc_vmop_rshift,               ///">>"
    nxc_vmop_add,                  ///"+"
    nxc_vmop_sub,                  ///"-"
    nxc_vmop_mul,                  ///"*"
    nxc_vmop_div,                  ///"/"
    nxc_vmop_mod,                  ///"%"
    ///-------------------------------------
    nxc_vmop_or_i,                 ///"||"
    nxc_vmop_and_i,                ///"&&"
    nxc_vmop_bor_i,                ///"|"
    nxc_vmop_band_i,               ///"&"
    nxc_vmop_xor_i,                ///"^"
    nxc_vmop_eq_i,                 ///"=="
    nxc_vmop_uneq_i,               ///"!="
    nxc_vmop_gt_i,                 ///">"
    nxc_vmop_lt_i,                 ///"<"
    nxc_vmop_ge_i,                 ///">="
    nxc_vmop_le_i,                 ///"<="
    nxc_vmop_lshift_i,             ///"<<"
    nxc_vmop_rshift_i,             ///">>"
    nxc_vmop_add_i,                ///"+"
    nxc_vmop_sub_i,                ///"-"
    nxc_vmop_mul_i,                ///"*"
    nxc_vmop_div_i,                ///"/"
    nxc_vmop_mod_i,                ///"%"
    ///-------------------------------------
    nxc_vmop_or_i1,                ///"||"
    nxc_vmop_and_i1,               ///"&&"
    nxc_vmop_bor_i1,               ///"|"
    nxc_vmop_band_i1,              ///"&"
    nxc_vmop_xor_i1,               ///"^"
    nxc_vmop_eq_i1,                ///"=="
    nxc_vmop_uneq_i1,              ///"!="
    nxc_vmop_gt_i1,                ///">"
    nxc_vmop_lt_i1,                ///"<"
    nxc_vmop_ge_i1,                ///">="
    nxc_vmop_le_i1,                ///"<="
    nxc_vmop_lshift_i1,            ///"<<"
    nxc_vmop_rshift_i1,            ///">>"
    nxc_vmop_add_i1,               ///"+"
    nxc_vmop_sub_i1,               ///"-"
    nxc_vmop_mul_i1,               ///"*"
    nxc_vmop_div_i1,               ///"/"
    nxc_vmop_mod_i1,               ///"%"
    ///--------------------------Float Opcode-----------------------------------
    nxc_vmop_f_gt,                 ///":>"
    nxc_vmop_f_lt,                 ///":<"
    nxc_vmop_f_ge,                 ///":>="
    nxc_vmop_f_le,                 ///":<="
    nxc_vmop_f_add,                ///":+"
    nxc_vmop_f_sub,                ///":-"
    nxc_vmop_f_mul,                ///":*"
    nxc_vmop_f_div,                ///":/"
    nxc_vmop_f_neg,                ///":-"
    ///-------------------------------------
    nxc_vmop_f_gt_i,               ///":>"
    nxc_vmop_f_lt_i,               ///":<"
    nxc_vmop_f_ge_i,               ///":>="
    nxc_vmop_f_le_i,               ///":<="
    nxc_vmop_f_add_i,              ///":+"
    nxc_vmop_f_sub_i,              ///":-"
    nxc_vmop_f_mul_i,              ///":*"
    nxc_vmop_f_div_i,              ///":/"
    ///-------------------------------------
    nxc_vmop_f_gt_i1,              ///":>"
    nxc_vmop_f_lt_i1,              ///":<"
    nxc_vmop_f_ge_i1,              ///":>="
    nxc_vmop_f_le_i1,              ///":<="
    nxc_vmop_f_add_i1,             ///":+"
    nxc_vmop_f_sub_i1,             ///":-"
    nxc_vmop_f_mul_i1,             ///":*"
    nxc_vmop_f_div_i1,             ///":/"
    ///-----------------------------unary opcode--------------------------------
    nxc_vmop_cast,                 ///"cast"
    nxc_vmop_positive,             ///"+"
    nxc_vmop_negative,             ///"-"
    nxc_vmop_compensation,         ///"~"
    nxc_vmop_not,                  ///"!"
    ///-------------------------------------------------------------------------
    nxc_vmop_addr_array8,          ///"&[[["    left + index
    nxc_vmop_addr_array16,         ///"&[[[["   left + index * sizeof(short)
    nxc_vmop_addr_array32,         ///"&[["     left + index * sizeof(int)
    nxc_vmop_addr_array_l,         ///"&["      left + index * sizeof(long)
    ///------------------------------flow-control-------------------------------
    nxc_vmop_call,           ///"call     $reg"
    nxc_vmop_postcall,       ///do post work after call(mov reg,eax;add esp,xxx)
    nxc_vmop_trap,           ///"trap     $reg"
    nxc_vmop_fast_call,      ///"fasttrap $reg"
    nxc_vmop_jmp,            ///"jmp      $imme_addr"
    nxc_vmop_jz,             ///"jmp      if zero"
    nxc_vmop_jnz,            ///"jmp      if non-zero"
    nxc_vmop_return_val,     ///"return   $expr"
    nxc_vmop_ret,            ///"ret"
    nxc_vmop_push,           ///"push     $value"
    nxc_vmop_pop,            ///"pop
    nxc_vmop_enter,          ///enter func
    nxc_vmop_leave,          ///leave func
    nxc_vmop_nop,            ///do nothing
    nxc_vmop_max,
};
///=============================================================================
///binary opcode offset to float binary offset ...
///return -1 means it can't be translated ...
                               
static int __nxc_foffset_map1[]={
    -1,//||
    -1,//&&
    -1,//|
    -1,//&
    -1,//^
    -1,//==
    -1,//!=
    nxc_vmop_f_gt - nxc_vmop_f_gt,//>
    nxc_vmop_f_lt - nxc_vmop_f_gt,//<
    nxc_vmop_f_ge - nxc_vmop_f_gt,//>=
    nxc_vmop_f_le - nxc_vmop_f_gt,//<=
    -1,//<<
    -1,//>>
    nxc_vmop_f_add - nxc_vmop_f_gt,//+
    nxc_vmop_f_sub - nxc_vmop_f_gt,//-
    nxc_vmop_f_mul - nxc_vmop_f_gt,//*
    nxc_vmop_f_div - nxc_vmop_f_gt,//'/'
    -1,//%
};

///translate binary-opcode-ofsfet to float-opcode-offset
___fast int nxc_bin_offset_to_fbin_offset(int b_offset)
{
    if (b_offset<0 || b_offset >=19 ) return -1;
    return __nxc_foffset_map1[b_offset];
}
/**===========================================================================*/

/**
 * type info ...[Unused!!!]
 */
struct _nxc_type
{
    nxc_hash_node_t   hash_node;
    char             *name;  ///type name 
    struct _nxc_type *father_type; ///point to father type 
    int               xclass;  ///type class
    int               size;    ///type size
    int               flag;    ///extra info ...
    int               __pad0;
};

/**
 * constant node , just a marco , replace while lexing !!!
 * a alias node ...
 */
struct _nxc_const_node
{
    nxc_hash_node_t hash_node;      ///hold name info ...
    nxc_token_t     const_token[1]; ///hold associated token info ...
};
#define nxc_const_node_get_name(node)         ((node)->hash_node.key)
#define nxc_const_node_set_name(node,name)    ((node)->hash_node.key=(name))
#define nxc_const_node_get_namelen(node)      ((node)->hash_node.klen)
#define nxc_const_node_set_namelen(node,len)  ((node)->hash_node.klen=(len))

#define nxc_const_node_get_token(node)        ((node)->const_token)
#define nxc_const_node_copy_token(node,token) nxc_token_copy((node)->const_token,(token))

/**===========================================================================*/

///symbol type ...
enum
{
    nxc_sym_bad=0,
    nxc_sym_global_function,
    nxc_sym_global_var,
    nxc_sym_local_param,
    nxc_sym_local_var,
    nxc_sym_member,
    nxc_sym_max,
};

/**
 * symbol header ...
 * vars functions labels ...
 */
struct _nxc_symbol
{
    nxc_hash_node_t hash_node;       ///key = {sym_name}
#define nxc_symflag_lvalue          1<<0
#define nxc_symflag_var_args        1<<1
#define nxc_symflag_extern          1<<2
#define nxc_symflag_array           1<<3
#define nxc_symflag_fastcall        1<<4
    int             sym_flag;        ///compound flag fields ...
    int             pad0;
    struct
    {
        int         sym_type;        ///symbol type (var&function)...
        int         type_size;       ///variable type size ...
        char       *type_name;       ///symbol type's type name ...
        char       *ftype_name;      ///father type's type name ...
    }
    __type;///type info ...
    nxc_str_node_t *str_node;        ///hold symbol name info ...
    union
    {
        int              offset;     ///var offset(lvar,lparam)...
        void            *vaddr;      ///var address ...
        nxc_instr_t     *xaddr;      ///function's first instruction addr...
    }
    addr;  ///address info ...
    union
    {
        nxc_dlist_head_t gsym_node; ///trace all local G-SYM(for ast_check).
    }
    node;  ///trace_node
    nxc_dlist_head_t   local_sym_table;///local symbol table ...
    ///-------------------------------------------------------------------------
    union
    {
        struct///--------------------------function info-----------------------
        {
            nxc_ast_t *stmt;       ///associated ast node ...
            long       lvars_size; ///local variable total size
            int        lregs_size; ///local register area (expr-stack) size...
            ///we should consider the param stack alignment problem!!!
            ///to keep local var's alignment (in 8B), local param's total size 
            ///must be 8B aligned!!!
            int        param_count;///for function ...
            int        param_size; ///for function (local param toal size)
            int        instr_count;
        }
        func;

        struct///------------------------constant info-------------------------
        {
            nxc_token_t tok_info;
        }
        constant;
    }u;
};

/**===========================================================================*/

/**
 * abstract node ...
 */
struct _nxc_ast
{
    nxc_dlist_head_t   list_head;
    short              opcode;     ///current node's operation code   ...
    short              result_reg; ///register that associated with this ast !!!
    int                line_number;///for location info ...
    char              *fname;      ///for src file info ...
    nxc_ast_t         *father_node;///save father node of current ...
    struct
    {
        char          *type_name;  ///current node's type name ...
        char          *ftype_name; ///current node's type name ...
    }
    __type;///type info ...
    struct 
    {
        nxc_dlist_head_t trace_node; ///misc purpose ...
        nxc_instr_t     *instr;      ///associated 'load instruction'
        nxc_sym_t       *sym;        ///associated symbol info ...
    }
    __trace;///trace-info ...
    union   ///context ...
    {
        ///------------------------expression AST node--------------------------
        struct 
        {
            nxc_token_t      token_info;
        }constant;
        struct
        {
            char            *name; ///id-name ...
        }id;
        struct
        {
            nxc_ast_t       *left;
            nxc_ast_t       *right;
        }expr;
        struct
        {
            nxc_ast_t       *unary;
        }prefix;
        struct
        {
            nxc_ast_t       *base;
            nxc_ast_t       *index;
        }array;
        struct
        {
            nxc_ast_t       *func;        ///function info ...
            nxc_dlist_head_t param_list;  ///trace all param ...
            int              param_count; ///param count ...
        }call;
        struct
        {
            char            *member_name; ///member name ...
            nxc_ast_t       *obj;         ///left object node 
            nxc_ast_t       *member;      ///member node ...
        }member;
        ///--------------------compound statement AST node----------------------
        struct
        {
            ///Since we have Expr-AST Why expr-stmt-AST?
            ///Reason :
            ///Expression-AST will hold a register to hold result !!!
            ///this result register will be transfer to other AST for other use!
            ///Expression-Statement AST will release unused (result)register!!!
            nxc_ast_t      *expr;
        }expr_stmt;
        struct
        {
            nxc_ast_t      *cond;
            nxc_ast_t      *then_stmt;
            nxc_ast_t      *else_stmt;
        }if_stmt;
        struct
        {
            /**
             * we should save last loop node here so that the later 
             * 'break/continue' could find a path
             ***Mark : switch should be treat as a loop node (it's has break)***
             * while(1) {    switch(x) {  case 123: break; continue; } }
             * this will confused the compiler ,  so last when we see 'break'
             * we should using compiler->last_loop_node
             * when we see 'continue' ,  we should find 'loop' node 
             * by traversal compiler->last_loop_node list !!!
             */
            nxc_dlist_head_t break_continue_list;//trac all break/continue point
            nxc_ast_t       *init_stmt;
            nxc_ast_t       *cond;
            nxc_ast_t       *next_stmt;
            nxc_ast_t       *loop_stmt;
        }loop_stmt;
        struct
        {
            ///point to father loop node ...
            nxc_ast_t       *loop_node;         
        }
        break_stmt,
        continue_stmt;
        struct
        {
            nxc_ast_t       *result;
        }return_stmt;
        struct
        {
            ///point to father loop node ...
            nxc_dlist_head_t stmt_list;
        }block_stmt;
    }u;
};

/**===========================================================================*/

/**
 * trap handler!!!
 */
typedef void (*nxc_api_proc_t)(nxc_vm_ctx_t *ctx);

/**
 * the compoun instruction object ...
 * cisc mode ...just do a simple instr ...
 */
struct _nxc_instr
{
    int                    opcode;     ///opcode
    int                    reg_index1; ///>>>operand1
    union
    {
        int                reg_index2; ///for store
        long               var_offset; ///for load
        nxc_instr_t       *func_addr;  ///for load ...
        long               jmp_offset; ///for jxx
        nxc_instr_t       *jmp_addr;   ///for jxxx
        nxc_api_proc_t     trap_func;  ///for trapx ...
        void              *xaddr;      ///misc
        char               operand2_addr[4];///just to get operand2's addr
        ///--------------------------------------------
        char               immed8;
        short              immed16;
        int                immed32;
        long               immed_l;   ///for load
        float              immed_f;
    }u;                               ///>>>operand2
};
///initialize only ...
___fast void nxc_instr_do_init(nxc_instr_t *instr){nxc_memset(instr,0,sizeof(*instr));}
#define nxc_instr_get_next(instr)               (&instr[1])
#define nxc_instr_get_opcode(instr)             ((instr)->opcode)
#define nxc_instr_set_opcode(instr,_opcode)     (instr)->opcode=(_opcode)
#define nxc_instr_get_reg_index1(instr)         ((instr)->reg_index1)
#define nxc_instr_set_reg_index1(instr,_index)  (instr)->reg_index1=(_index)
#define nxc_instr_get_reg_index2(instr)         ((instr)->u.reg_index2)
#define nxc_instr_set_reg_index2(instr,_index)  (instr)->u.reg_index2=(_index)

///get operand2's address
#define nxc_instr_get_operand2_addr(instr)      ((instr)->u.operand2_addr)

#define nxc_instr_get_var_offset(instr)         ((instr)->u.var_offset)
#define nxc_instr_set_var_offset(instr,_offset) (instr)->u.var_offset=(_offset)

#define nxc_instr_get_func_addr(instr)          ((instr)->u.func_addr)
#define nxc_instr_set_func_addr(instr,_addr)    (instr)->u.func_addr=(_addr)

#define nxc_instr_get_xaddr(instr)              ((instr)->u.xaddr)
#define nxc_instr_set_xaddr(instr,_addr)        (instr)->u.xaddr=(_addr)

#define nxc_instr_get_jmp_addr(instr)           ((instr)->u.jmp_addr)
#define nxc_instr_set_jmp_addr(instr,_addr)     (instr)->u.jmp_addr=(_addr)

///calc offset from given addr and store to instruction
#define nxc_instr_set_jmp_offset_by_addr(instr,_addr) (instr)->u.jmp_offset=(long)(_addr)-(long)(instr)

#define nxc_instr_get_jmp_offset(instr)         ((instr)->u.jmp_offset)
#define nxc_instr_set_jmp_offset(instr,_offset) (instr)->u.jmp_offset=(_offset)

#define nxc_instr_get_trap_func(instr)          ((instr)->u.trap_func)
#define nxc_instr_set_trap_func(instr,_pfunc)   (instr)->u.trap_func=(_pfunc)
///immediate number ...
#define nxc_instr_get_immed8(instr)             ((instr)->u.immed8)
#define nxc_instr_set_immed8(instr,_immed8)     (instr)->u.immed8=(_immed8)
#define nxc_instr_get_immed16(instr)            ((instr)->u.immed16)
#define nxc_instr_set_immed16(instr,_immed16)   (instr)->u.immed16=(_immed16)
#define nxc_instr_get_immed32(instr)            ((instr)->u.immed32)
#define nxc_instr_set_immed32(instr,_immed32)   (instr)->u.immed32=(_immed32)
#define nxc_instr_get_immed_l(instr)            ((instr)->u.immed_l)
#define nxc_instr_set_immed_l(instr,_immed_L)   (instr)->u.immed_l=(_immed_L)

#define nxc_instr_get_immed_f(instr)            ((instr)->u.immed_f)
#define nxc_instr_set_immed_f(instr,_immed_f)   (instr)->u.immed_f=(_immed_f)

/**===========================================================================*/
/**
 * api node , to save registered api info ...
 */
struct _nxc_api_node;
typedef struct _nxc_api_node nxc_api_node_t;
struct _nxc_api_node
{
    nxc_hash_node_t hash_node[1];
    nxc_instr_t     trap_instr[1];
    char           *name;
    void           *addr;
};

/**
 * API table object ...
 * hold all registered api symbol name and address ...
 */
struct _nxc_api_table;
typedef struct _nxc_api_table nxc_api_table_t;
struct _nxc_api_table
{
    nxc_mempool_t    mem_pool[1];         ///private memory pool of compiler...
    nxc_hash_table_t table[1];
    nxc_hlist_head_t bucket[256];
};

/**===========================================================================*/
struct _nxc_script_image;
typedef struct _nxc_script_image nxc_script_image_t;
struct _nxc_script_image
{
    nxc_instr_t *text_base;          ///base address of text-segment
    char        *data_base;          ///base addr of data-segment
    ///---
    int          text_seg_size;      ///text-segment size ...
    int          text_size;          ///actual text size
    ///---
    int          data_seg_size;      ///data-segment size
    int          data_size;          ///actual data size ...
    ///---
    int          reloc_table_offset; ///data reloc table offste in data seg
    int          reloc_count;        ///static data relocation ...
    int          import_table_offset;///snsym reloc table offste in data seg
    int          import_count;       ///dynamic symbol relocation ...
    int          export_table_offset;///global symbol(extern excluded) table
    int          export_count;
};

/**===========================================================================*/
///output handler ...
typedef int(*nxc_vprintf_t)(nxc_compiler_t*compiler,char*format,nxc_va_list arg);

/**
 * compiler object ...
 * virtual machine object ...
 */
struct _nxc_compiler
{
    nxc_mempool_t    mem_pool[1];      ///private memory pool of compiler...
    ///---------------------------parser's tmp-data-----------------------------
    nxc_lexer_t      lexer[1];         ///lexer of compiler...
    nxc_hash_table_t str_table[1];     ///builtin hash table
    nxc_hlist_head_t str_bucket[128];
    nxc_hash_table_t const_table[1];   ///builtin hash table
    nxc_hlist_head_t const_bucket[128];
    ///----------------------global-variable manager----------------------------
    nxc_hash_table_t sym_table[1];     ///builtin hash table with 128 bucket.
    nxc_hlist_head_t sym_bucket[128];
    ///-------------------------------------------------------------------------
    nxc_dlist_head_t load_lvar_list;   ///trace all load-id instructions ...
    nxc_sym_t       *curr_function;    ///current function info
    nxc_ast_t       *curr_loop_node;   ///save LastLoopNode for 'break/continue'
    nxc_ast_t       *curr_ast_node;    ///to tell about the father node ...
    ///---
    nxc_instr_t     *curr_pre_creat_instr;///trace the pre-created instruction..
    int              expr_sp;
    int              expr_sp_max;         ///save max register size ...

    char            *curr_struct_name;    ///name of current struct ...
    int              curr_struct_namelen; ///length of current name
    int              curr_struct_size;    ///current struct size ...
    ///-------------------------------------------------------------------------
    int              dseg_size;
    int              xseg_size;
    ///-------------------------------------------------------------------------
    nxc_dlist_head_t global_sym_list;///trace global symbol (func and var)
    nxc_dlist_head_t const_str_list; ///trace all static string ...
    nxc_dlist_head_t load_gsym_list; ///trace all load g_var    instruction ...
    nxc_dlist_head_t load_xsym_list; ///trace all load g_func   instruction ...
    nxc_dlist_head_t load_const_list;///trace all load constant instruction ...
    int              global_sym_count;
    int              const_str_count;
    int              load_gsym_count;
    int              load_xsym_count;
    int              load_const_count;
    int              enable_debug_info;
    ///-------------------------------------------------------------------------
    nxc_vprintf_t    vprintf_proc;    ///output handler ...
    nxc_script_image_t*image;         ///current image  ...
};

/**===========================================================================*/
/**===========================================================================*/
___fast int  nxc_sym_has_flag(nxc_sym_t *sym,int flag){return nxc_is_flag_set(sym->sym_flag,flag);}
___fast void nxc_sym_add_flag(nxc_sym_t*sym,int flag){nxc_add_flag(sym->sym_flag,flag);}
___fast void nxc_sym_del_flag(nxc_sym_t*sym,int flag){nxc_remove_flag(sym->sym_flag,flag);}
___fast int  nxc_sym_is_array(nxc_sym_t *sym){return nxc_is_flag_set(sym->sym_flag,nxc_symflag_array);}

___fast void  nxc_sym_set_name(nxc_sym_t*sym,char*_name){sym->hash_node.key = _name;}
___fast char* nxc_sym_get_name(nxc_sym_t*sym){return sym->hash_node.key;}
___fast void  nxc_sym_set_namelen(nxc_sym_t*sym,int _namelen){sym->hash_node.klen = _namelen;}
___fast int   nxc_sym_get_namelen(nxc_sym_t*sym){return sym->hash_node.klen;}

___fast nxc_str_node_t* nxc_sym_get_str_node(nxc_sym_t *sym){return sym->str_node;}
___fast void  nxc_sym_set_str_node(nxc_sym_t *sym,nxc_str_node_t*_node){sym->str_node=_node;}

___fast void nxc_sym_set_type(nxc_sym_t*sym,int _type){sym->__type.sym_type = _type;}
___fast int  nxc_sym_get_type(nxc_sym_t*sym){return sym->__type.sym_type;}

___fast void nxc_sym_set_typesize(nxc_sym_t*sym,int _size){sym->__type.type_size = _size;}
___fast int  nxc_sym_get_typesize(nxc_sym_t*sym){return sym->__type.type_size;}


___fast void nxc_sym_set_typename(nxc_sym_t *sym,char*type_name){sym->__type.type_name=type_name;}
___fast char*nxc_sym_get_typename(nxc_sym_t *sym){return sym->__type.type_name;}

___fast void nxc_sym_set_f_typename(nxc_sym_t *sym,char*type_name){sym->__type.ftype_name=type_name;}
___fast char*nxc_sym_get_f_typename(nxc_sym_t *sym){return sym->__type.ftype_name;}

___fast void nxc_sym_init_local_sym_table(nxc_sym_t *sym){nxc_init_dlist_head(&sym->local_sym_table);}
___fast nxc_dlist_head_t* nxc_sym_get_local_sym_table(nxc_sym_t *sym){return &sym->local_sym_table;}
///add a new symbol to local symbol ...
___fast void nxc_sym_add_local_sym(nxc_sym_t *sym,nxc_sym_t *new_sym){
    nxc_dlist_add_tail((nxc_dlist_head_t *)&new_sym->hash_node.list_node,nxc_sym_get_local_sym_table(sym));
}
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

___fast nxc_instr_t *nxc_sym_get_func_addr(nxc_sym_t *sym){return sym->addr.xaddr;}
___fast void nxc_sym_set_func_addr(nxc_sym_t*sym,nxc_instr_t*addr){sym->addr.xaddr=addr;}

___fast nxc_ast_t *nxc_sym_get_func_stmt_ast(nxc_sym_t *sym){return sym->u.func.stmt;}
___fast void nxc_sym_set_func_stmt_ast(nxc_sym_t *sym,nxc_ast_t*stmt){sym->u.func.stmt=stmt;}
___fast int  nxc_sym_get_param_cnt(nxc_sym_t *sym){return sym->u.func.param_count;}
___fast void nxc_sym_set_param_cnt(nxc_sym_t*sym,int count){sym->u.func.param_count=count;}
___fast int  nxc_sym_get_param_size(nxc_sym_t *sym){return sym->u.func.param_size;}
___fast void nxc_sym_set_param_size(nxc_sym_t*sym,int size){sym->u.func.param_size=size;}
___fast int  nxc_sym_get_func_lvar_size(nxc_sym_t *sym){return sym->u.func.lvars_size;}
___fast void nxc_sym_set_func_lvar_size(nxc_sym_t*sym,int size){sym->u.func.lvars_size=size;}
___fast int  nxc_sym_get_func_lreg_size(nxc_sym_t *sym){return sym->u.func.lregs_size;}
___fast void nxc_sym_set_func_lreg_size(nxc_sym_t*sym,int size){sym->u.func.lregs_size=size;}

___fast int  nxc_sym_get_func_instr_cnt(nxc_sym_t *sym){return sym->u.func.instr_count;}
___fast void nxc_sym_set_func_instr_cnt(nxc_sym_t*sym,int count){sym->u.func.instr_count=count;}
___fast int  nxc_sym_inc_func_instr_cnt(nxc_sym_t *sym){return ++sym->u.func.instr_count;}

___fast int  nxc_sym_get_var_offset(nxc_sym_t *sym){return sym->addr.offset;}
___fast void nxc_sym_set_var_offset(nxc_sym_t*sym,int offset){sym->addr.offset=offset;}
___fast void*nxc_sym_get_var_addr(nxc_sym_t *sym){return sym->addr.vaddr;}
___fast void nxc_sym_set_var_addr(nxc_sym_t*sym,void* addr){sym->addr.vaddr=addr;}

/**===========================================================================*/
/**===========================================================================*/
/**===========================================================================*/

___fast int   nxc_ast_get_opcode(nxc_ast_t *ast){return ast->opcode;}
___fast void  nxc_ast_set_opcode(nxc_ast_t *ast,int opcode){ast->opcode=opcode;}
___fast int   nxc_ast_get_line_num(nxc_ast_t *ast){return ast->line_number;}
___fast void  nxc_ast_set_line_num(nxc_ast_t *ast,int line_num){ast->line_number=line_num;}
___fast char* nxc_ast_get_fname(nxc_ast_t *ast){return ast->fname;}
___fast void  nxc_ast_set_fname(nxc_ast_t *ast,char* fname){ast->fname=fname;}
___fast int   nxc_ast_get_result_reg(nxc_ast_t *ast){return ast->result_reg;}
___fast void  nxc_ast_set_result_reg(nxc_ast_t *ast,int result_reg){ast->result_reg=result_reg;}
___fast nxc_ast_t* nxc_ast_get_father_node(nxc_ast_t *ast){return ast->father_node;}
___fast void       nxc_ast_set_father_node(nxc_ast_t *ast,nxc_ast_t* father_node){ast->father_node=father_node;}

___fast nxc_dlist_head_t *nxc_ast_get_trace_node(nxc_ast_t *ast){return &ast->__trace.trace_node;}
___fast nxc_instr_t*nxc_ast_get_instr(nxc_ast_t *ast){return ast->__trace.instr;}
___fast void        nxc_ast_set_instr(nxc_ast_t *ast,nxc_instr_t*instr){ast->__trace.instr=instr;}
___fast nxc_sym_t  *nxc_ast_get_sym(nxc_ast_t *ast){return ast->__trace.sym;}
___fast void        nxc_ast_set_sym(nxc_ast_t *ast,nxc_sym_t*sym){ast->__trace.sym=sym;}

___fast char* nxc_ast_get_typename(nxc_ast_t *ast){return ast->__type.type_name;}
___fast void  nxc_ast_set_typename(nxc_ast_t *ast,char* tname){ast->__type.type_name=tname;}
___fast char* nxc_ast_get_f_typename(nxc_ast_t *ast){return ast->__type.ftype_name;}
___fast void  nxc_ast_set_f_typename(nxc_ast_t *ast,char* fname){ast->__type.ftype_name=fname;}
///if ast type is float ...
___fast int   nxc_ast_is_float_type(nxc_ast_t *ast){
    return nxc_ast_get_typename(ast) && 
    !nxc_memcmp(nxc_ast_get_typename(ast),"float",6);
}

/**===========================================================================*/
___fast char *nxc_id_ast_get_name(nxc_ast_t *ast){return ast->u.id.name;}
___fast void  nxc_id_ast_set_name(nxc_ast_t *ast,char*name){ast->u.id.name=name;}
/**===========================================================================*/
___fast long nxc_const_ast_get_long_val(nxc_ast_t *ast){return ast->u.constant.token_info.u.long_val;}
___fast void nxc_const_ast_set_long_val(nxc_ast_t *ast,long _val){ast->u.constant.token_info.u.long_val=_val;}
___fast float nxc_const_ast_get_float_val(nxc_ast_t *ast){return ast->u.constant.token_info.u.float_val;}
___fast void  nxc_const_ast_set_float_val(nxc_ast_t *ast,float _val){ast->u.constant.token_info.u.float_val=_val;}

___fast char*nxc_const_ast_get_str_val(nxc_ast_t *ast){return nxc_str_node_get_str(ast->u.constant.token_info.u.str_node);}
___fast nxc_str_node_t*nxc_const_ast_get_str_node(nxc_ast_t *ast){return ast->u.constant.token_info.u.str_node;}
___fast void nxc_const_ast_set_str_node(nxc_ast_t *ast,nxc_str_node_t* _node){ast->u.constant.token_info.u.str_node=_node;}
___fast int  nxc_const_ast_get_type(nxc_ast_t *ast){return ast->u.constant.token_info.xtype;}
___fast void nxc_const_ast_set_type(nxc_ast_t *ast,int _type){ast->u.constant.token_info.xtype=_type;}
___fast int  nxc_const_ast_get_len(nxc_ast_t *ast){return ast->u.constant.token_info.xlen;}
___fast void nxc_const_ast_set_len(nxc_ast_t *ast,int _len){ast->u.constant.token_info.xlen=_len;}
/**===========================================================================*/
___fast nxc_ast_t *nxc_expr_ast_get_left(nxc_ast_t *ast){return ast->u.expr.left;}
___fast void       nxc_expr_ast_set_left(nxc_ast_t *ast,nxc_ast_t*left){ast->u.expr.left=left;}
___fast nxc_ast_t *nxc_expr_ast_get_right(nxc_ast_t *ast){return ast->u.expr.right;}
___fast void       nxc_expr_ast_set_right(nxc_ast_t *ast,nxc_ast_t*right){ast->u.expr.right=right;}
/**===========================================================================*/
___fast nxc_ast_t *nxc_prefix_ast_get_unary(nxc_ast_t *ast){return ast->u.prefix.unary;}
___fast void       nxc_prefix_ast_set_unary(nxc_ast_t *ast,nxc_ast_t*unary){ast->u.prefix.unary=unary;}
/**===========================================================================*/
___fast nxc_ast_t *nxc_array_ast_get_base(nxc_ast_t *ast){return ast->u.array.base;}
___fast void       nxc_array_ast_set_base(nxc_ast_t *ast,nxc_ast_t*base){ast->u.array.base=base;}
___fast nxc_ast_t *nxc_array_ast_get_index(nxc_ast_t *ast){return ast->u.array.index;}
___fast void       nxc_array_ast_set_index(nxc_ast_t *ast,nxc_ast_t*index){ast->u.array.index=index;}
/**===========================================================================*/
___fast nxc_ast_t *nxc_call_ast_get_func(nxc_ast_t *ast){return ast->u.call.func;}
___fast void       nxc_call_ast_set_func(nxc_ast_t *ast,nxc_ast_t*func){ast->u.call.func=func;}
___fast nxc_dlist_head_t *nxc_call_ast_get_parm_list(nxc_ast_t *ast){return &ast->u.call.param_list;}
___fast void nxc_call_ast_init_parm_list(nxc_ast_t *ast){nxc_init_dlist_head(&ast->u.call.param_list);}
___fast void nxc_call_ast_add_parm(nxc_ast_t *ast,nxc_ast_t *parm){nxc_dlist_add_tail(&parm->list_head,&ast->u.call.param_list);}
___fast int  nxc_call_ast_get_parm_cnt(nxc_ast_t *ast){return ast->u.call.param_count;}
___fast void nxc_call_ast_set_parmcnt(nxc_ast_t *ast,int count){ast->u.call.param_count=count;}
/**===========================================================================*/
___fast nxc_ast_t *nxc_member_ast_get_obj(nxc_ast_t *ast){return ast->u.member.obj;}
___fast void       nxc_member_ast_set_obj(nxc_ast_t *ast,nxc_ast_t*obj){ast->u.member.obj=obj;}
___fast char*      nxc_member_ast_get_member_name(nxc_ast_t *ast){return ast->u.member.member_name;}
___fast void       nxc_member_ast_set_member_name(nxc_ast_t *ast,char*name){ast->u.member.member_name=name;}
/**===========================================================================*/
___fast nxc_ast_t *nxc_expr_stmt_ast_get_expr(nxc_ast_t *ast){return ast->u.expr_stmt.expr;}
___fast void       nxc_expr_stmt_ast_set_expr(nxc_ast_t *ast,nxc_ast_t*expr){ast->u.expr_stmt.expr=expr;}
/**===========================================================================*/
___fast nxc_ast_t *nxc_if_stmt_ast_get_cond(nxc_ast_t *ast){return ast->u.if_stmt.cond;}
___fast void       nxc_if_stmt_ast_set_cond(nxc_ast_t *ast,nxc_ast_t*cond){ast->u.if_stmt.cond=cond;}
___fast nxc_ast_t *nxc_if_stmt_ast_get_then_stmt(nxc_ast_t *ast){return ast->u.if_stmt.then_stmt;}
___fast void       nxc_if_stmt_ast_set_then_stmt(nxc_ast_t *ast,nxc_ast_t*then_stmt){ast->u.if_stmt.then_stmt=then_stmt;}
___fast nxc_ast_t *nxc_if_stmt_ast_get_else_stmt(nxc_ast_t *ast){return ast->u.if_stmt.else_stmt;}
___fast void       nxc_if_stmt_ast_set_else_stmt(nxc_ast_t *ast,nxc_ast_t*else_stmt){ast->u.if_stmt.else_stmt=else_stmt;}
/**===========================================================================*/
___fast nxc_ast_t* nxc_break_stmt_ast_get_loop_node(nxc_ast_t *ast){return ast->u.break_stmt.loop_node;}
___fast void       nxc_break_stmt_ast_set_loop_node(nxc_ast_t *ast,nxc_ast_t*loop_node){ast->u.break_stmt.loop_node=loop_node;}
///~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
___fast nxc_ast_t* nxc_continue_stmt_ast_get_loop_node(nxc_ast_t *ast){return ast->u.continue_stmt.loop_node;}
___fast void       nxc_continue_stmt_ast_set_loop_node(nxc_ast_t *ast,nxc_ast_t*loop_node){ast->u.continue_stmt.loop_node=loop_node;}
/**===========================================================================*/
___fast nxc_dlist_head_t*nxc_loop_stmt_ast_get_break_continue_list(nxc_ast_t *ast){return &ast->u.loop_stmt.break_continue_list;}
___fast void nxc_loop_stmt_ast_init_break_continue_list(nxc_ast_t *ast){nxc_init_dlist_head(&ast->u.loop_stmt.break_continue_list);}
___fast void nxc_loop_stmt_ast_trace_continue(nxc_ast_t *ast,nxc_ast_t *continue_ast){nxc_dlist_add_tail(nxc_ast_get_trace_node(continue_ast),nxc_loop_stmt_ast_get_break_continue_list(ast));}
___fast void nxc_loop_stmt_ast_trace_break(nxc_ast_t *ast,nxc_ast_t *break_ast){nxc_dlist_add_tail(nxc_ast_get_trace_node(break_ast),nxc_loop_stmt_ast_get_break_continue_list(ast));}

___fast nxc_ast_t *nxc_loop_stmt_ast_get_init_stmt(nxc_ast_t *ast){return ast->u.loop_stmt.init_stmt;}
___fast void       nxc_loop_stmt_ast_set_init_stmt(nxc_ast_t *ast,nxc_ast_t*init_stmt){ast->u.loop_stmt.init_stmt=init_stmt;}
___fast nxc_ast_t *nxc_loop_stmt_ast_get_cond(nxc_ast_t *ast){return ast->u.loop_stmt.cond;}
___fast void       nxc_loop_stmt_ast_set_cond(nxc_ast_t *ast,nxc_ast_t*cond){ast->u.loop_stmt.cond=cond;}
___fast nxc_ast_t *nxc_loop_stmt_ast_get_next_stmt(nxc_ast_t *ast){return ast->u.loop_stmt.next_stmt;}
___fast void       nxc_loop_stmt_ast_set_next_stmt(nxc_ast_t *ast,nxc_ast_t*next_stmt){ast->u.loop_stmt.next_stmt=next_stmt;}
___fast nxc_ast_t *nxc_loop_stmt_ast_get_loop_stmt(nxc_ast_t *ast){return ast->u.loop_stmt.loop_stmt;}
___fast void       nxc_loop_stmt_ast_set_loop_stmt(nxc_ast_t *ast,nxc_ast_t*loop_stmt){ast->u.loop_stmt.loop_stmt=loop_stmt;}
/**===========================================================================*/
___fast nxc_ast_t *nxc_return_stmt_ast_get_result(nxc_ast_t *ast){return ast->u.return_stmt.result;}
___fast void       nxc_return_stmt_ast_set_result(nxc_ast_t *ast,nxc_ast_t*result){ast->u.return_stmt.result=result;}
/**===========================================================================*/
___fast nxc_dlist_head_t *nxc_block_stmt_ast_get_stmt_list(nxc_ast_t *ast){return &ast->u.block_stmt.stmt_list;}
___fast int  nxc_block_stmt_ast_is_stmt_list_empty(nxc_ast_t *ast){return nxc_dlist_empty(&ast->u.block_stmt.stmt_list);}
___fast void nxc_block_stmt_ast_init_stmt_list(nxc_ast_t *ast){nxc_init_dlist_head(&ast->u.block_stmt.stmt_list);}
___fast void nxc_block_stmt_ast_add_stmt(nxc_ast_t *ast,nxc_ast_t *stmt){nxc_dlist_add_tail(&stmt->list_head,&ast->u.block_stmt.stmt_list);}

/**===========================================================================*/
#define nxc_compiler_get_dseg_size(c)      ((c)->dseg_size)
#define nxc_compiler_set_dseg_size(c,size) ((c)->dseg_size=(size))
#define nxc_compiler_add_dseg_size(c,size) ((c)->dseg_size+=(size))
#define nxc_compiler_get_xseg_size(c)      ((c)->xseg_size)
#define nxc_compiler_set_xseg_size(c,size) ((c)->xseg_size=(size))
#define nxc_compiler_add_xseg_size(c,size) ((c)->xseg_size+=(size))
___fast int nxc_compiler_alloc_dseg_space(nxc_compiler_t*const compiler,int size)
{
    int ret;
    size ++;  ///zero-terminator
    size += 8;///reserved header space ...
    size = nxc_align(size,sizeof(long)*2);///keep 8/16 Bytes aligned 
    ret = nxc_compiler_get_dseg_size(compiler);
    nxc_compiler_add_dseg_size(compiler,size);
    return ret;
}
___fast int nxc_compiler_alloc_xseg_space(nxc_compiler_t*const compiler,int size)
{
    int ret;
    ret = nxc_compiler_get_xseg_size(compiler);
    nxc_compiler_add_xseg_size(compiler,size);
    return ret;
}
///if we are inside a function's body ...
___fast int nxc_in_local_zone(nxc_compiler_t*const compiler)
{
    return compiler->curr_function != 0;
}
///-----------------------------------------------------------------------------
___fast void nxc_compiler_init_global_sym_list(nxc_compiler_t*const compiler){
    nxc_init_dlist_head(&compiler->global_sym_list);
    compiler->global_sym_count = 0;
}
___fast void nxc_compiler_trace_global_sym(nxc_compiler_t*const compiler,nxc_sym_t *func){
    nxc_dlist_add_tail(&func->node.gsym_node,&compiler->global_sym_list);
    compiler->global_sym_count++;
}
///-----------------------------------------------------------------------------
___fast void nxc_compiler_init_load_lvar_list(nxc_compiler_t*const compiler){
    nxc_init_dlist_head(&compiler->load_lvar_list);
}
___fast void nxc_compiler_trace_load_lvar(nxc_compiler_t*const compiler,nxc_ast_t *id_ast){
    nxc_dlist_add_tail(nxc_ast_get_trace_node(id_ast),&compiler->load_lvar_list);
}
///-----------------------------------------------------------------------------
///trace list : 'load_global_sym'
___fast void nxc_compiler_init_load_gsym_list(nxc_compiler_t*const compiler){
    nxc_init_dlist_head(&compiler->load_gsym_list);
    compiler->load_gsym_count = 0;
}
///>>>trace list : 'load_global_sym'
___fast void nxc_compiler_trace_load_g_sym(nxc_compiler_t*const compiler,nxc_ast_t *id_ast)
{
    nxc_dlist_add_tail(nxc_ast_get_trace_node(id_ast),&compiler->load_gsym_list);
    compiler->load_gsym_count++;
}

///-----------------------------------------------------------------------------
///trace-list :  "add load_extern_sym"
___fast void nxc_compiler_init_load_xsym_list(nxc_compiler_t*const compiler){
    nxc_init_dlist_head(&compiler->load_xsym_list);
    compiler->load_xsym_count = 0;
}
///>>>
___fast void nxc_compiler_trace_load_x_sym(nxc_compiler_t*const compiler,nxc_ast_t *id_ast)
{
    nxc_dlist_add_tail(nxc_ast_get_trace_node(id_ast),&compiler->load_xsym_list);
    compiler->load_xsym_count++;
}
///-----------------------------------------------------------------------------
___fast void nxc_compiler_init_load_const_list(nxc_compiler_t*const compiler){
    nxc_init_dlist_head(&compiler->load_const_list);
    compiler->load_const_count = 0;
}
///>>>
___fast void nxc_compiler_trace_load_const(nxc_compiler_t*const compiler,nxc_ast_t *const_ast)
{
    nxc_dlist_add_tail(nxc_ast_get_trace_node(const_ast),&compiler->load_const_list);
    compiler->load_const_count++;
}
///-----------------------------------------------------------------------------
___fast void nxc_compiler_init_const_str_list(nxc_compiler_t*const compiler)
{
    nxc_init_dlist_head(&compiler->const_str_list);
    compiler->load_const_count = 0;
}
///>>>
___fast void nxc_compiler_trace_const_str(nxc_compiler_t*const compiler,nxc_str_node_t *str_node)
{
    ///skip the traced node !!!
    if (!nxc_dlist_empty(nxc_str_node_get_trace_node(str_node))) return ;

    nxc_dlist_add_tail(nxc_str_node_get_trace_node(str_node),&compiler->const_str_list);
    compiler->const_str_count++;
    ///alloc data-seg space ...
    nxc_compiler_alloc_dseg_space(compiler,nxc_str_node_get_len(str_node));
}

/**===========================================================================*/
/**
 * output gateway ...
 */
___fast int nxc_printf(nxc_compiler_t* const compiler,const char *fmt, ... )
{
    nxc_va_list args;
    int rv = 0;

    if(compiler->vprintf_proc)
    {
        nxc_va_start( args, fmt );
        rv = compiler->vprintf_proc((nxc_compiler_t*)compiler,(char*)fmt,args);
        nxc_va_end( args );
    }
    
    return rv;
}
___fast int nxc_vprintf(nxc_compiler_t* const compiler,const char*fmt,va_list args)
{
    int rv;
    if(compiler->vprintf_proc)
    {
        rv = compiler->vprintf_proc((nxc_compiler_t*)compiler,(char*)fmt,args);
        va_end( args );
    }
    return 0;
}

/**
 * alloc/free memory from compiler's mem pool ...
 */
___fast void *nxc_compiler_malloc(nxc_compiler_t*const compiler,int sz)
{
    nxc_lexer_t *lexer;
    void *ret;

    ret = nxc_pmalloc(compiler->mem_pool,sz);
    if (!ret){
        lexer = compiler->lexer;
        nxc_printf(compiler,"(%s,%d):low MEM\n",lexer->fname?lexer->fname:"N/A",lexer->line_number+1);
    }
    return ret;
}
___fast void nxc_compiler_free(void *ptr)
{
    nxc_pfree(ptr);
}
///raw malloc
___fast void *__nxc_compiler_malloc(nxc_compiler_t*const compiler,int sz)
{
    return compiler->mem_pool->do_malloc(compiler->mem_pool->allocator_data,sz);
}
///raw malloc
___fast void *__nxc_compiler_free(nxc_compiler_t*const compiler,void*ptr,int sz)
{
    compiler->mem_pool->do_free(ptr,sz);
}

/**===========================================================================*/
/**===========================================================================*/
/**===========================================================================*/

___fast char*nxc_api_node_get_name(nxc_api_node_t *n){return (char*)n->hash_node->key;}
___fast void nxc_api_node_set_name(nxc_api_node_t *n,void*name){n->hash_node->key=name;}
___fast int  nxc_api_node_get_namelen(nxc_api_node_t *n){return n->hash_node->klen;}
___fast void nxc_api_node_set_namelen(nxc_api_node_t *n,int len){n->hash_node->klen=len;}
___fast void*nxc_api_node_get_addr(nxc_api_node_t *n){return n->hash_node->data;}
___fast void nxc_api_node_set_addr(nxc_api_node_t *n,void*addr){n->hash_node->data=addr;}

/**
 * init a api-table ...
 * @return 0 means okay ...
 */
___fast int nxc_init_api_table(nxc_api_table_t*  api_table,
                               nxc_allocator_t   malloc_proc,
                               nxc_deallocator_t free_proc,
                               void*             allocator_data)
{
    nxc_memset(api_table,0,sizeof(*api_table));
    nxc_init_mpool(api_table->mem_pool,malloc_proc,free_proc,allocator_data);
    nxc_init_builtin_hash_table(api_table->table,api_table->mem_pool,256,0,0);
    return 0;
}
/**
 * destruct a api-table ...
 */
___fast void nxc_fini_api_table(nxc_api_table_t *api_table)
{
    ///release all mem ...
    nxc_clear_mpool(api_table->mem_pool);
    ///reinit hash table ...
    nxc_init_builtin_hash_table(api_table->table,api_table->mem_pool,256,0,0);
    return ;
}
/**
 * find api-address by name ...
 * @return api-address , 0 means error 
 */
___fast void*nxc_find_api(nxc_api_table_t *api_table,char*name)
{
    nxc_api_node_t *n;
    nxc_hash_node_t dummy;
    
    dummy.key  = name;
    dummy.klen = nxc_strlen(name);
    
    ///trnna fix existed ...
    n = (nxc_api_node_t *)nxc_hash_find(api_table->table,&dummy);
    if (n) return nxc_api_node_get_addr(n);

    return 0;
}
/**
 * register a api by name ...
 * @return 0 means okay ...
 */
static int nxc_register_api(nxc_api_table_t *api_table,char*name,nxc_api_proc_t api_proc)
{
    int total_len;
    int namelen;
    nxc_api_node_t *node;
    void*addr;

    ///uniqueness check ...
    if (nxc_find_api(api_table,name)) return -1;

    namelen = nxc_strlen(name);
    total_len = sizeof(nxc_api_node_t) + namelen + 1;
    ///create a new keyword-node (add extra zero to target string)
    node = (nxc_api_node_t *) nxc_pmalloc(api_table->mem_pool,total_len);
    if(!node) return -2;
    ///reset mem
    nxc_memset(node,0,total_len);
    ///copy name ...
    nxc_memcpy(&node[1],name,namelen);
    ///set name addr...
    nxc_init_hash_node(node->hash_node,&node[1],namelen,0);

    ///func addr is the first instruction address ...
    addr = (void*)node->trap_instr;
    ///set info ...
    nxc_api_node_set_name(node,&node[1]);   ///name 
    nxc_api_node_set_namelen(node,namelen); ///namelen
    nxc_api_node_set_addr(node,addr);       ///symbol addr 
    ///init trap instr
    nxc_instr_set_opcode(node->trap_instr,nxc_vmop_trap);///trap on execute !!!
    nxc_instr_set_trap_func(node->trap_instr,api_proc);  ///trap handler !!!
    
    nxc_hash_add(api_table->table,node->hash_node);
    return 0;
}
///init a image object ...
___fast int nxc_script_image_do_init(nxc_script_image_t *image)
{
    nxc_memset(image,0,sizeof(*image));
    return 0;
}
///destroy a image object ...
static int nxc_script_image_do_destroy(nxc_script_image_t *image,
                                        nxc_deallocator_t free_proc)
{
    if(!image) return 0;
    if( ((char*)image->text_base == (char*)image + sizeof(*image))||
        (image->data_base == (char*)image + sizeof(*image)))
    {
        ///data-seg and text-seg is connected , treat as one block , ignore
        free_proc(image,sizeof(*image));
        return 0;
    }
    if (image->text_base)
    {
        free_proc(image->text_base,image->text_seg_size);
        image->text_base = 0;
    }
    if (image->data_base)
    {
        free_proc(image->data_base,image->data_seg_size);
        image->data_base = 0;
    }
    free_proc(image,sizeof(*image));
    return 0;
}

/**
 * test if pointer in data seg (data pointer)
 */
___fast int nxc_script_image_do_test_dptr(nxc_script_image_t *image,void *_pos)
{
    char *pos = (char *)_pos;
    return (pos >= image->data_base) && (pos < image->data_base+image->data_seg_size);
}
/**
 * reloc image ...
 * @new_data_base : new base-addr of data segment ...
 * @return 0 means okay 
 */
static int nxc_script_image_do_reloc(nxc_script_image_t *image,
                                      long new_data_base,
                                      long new_text_base)
{
    long dseg_offset,xseg_offset;
    long *ptr;
    int i;

    dseg_offset = new_data_base - (long)image->data_base;
    xseg_offset = new_text_base - (long)image->text_base;

    ///fix EXP-table dseg_offset (name dseg_offset)...
    ptr = (long *)(new_data_base + image->export_table_offset);
    for (i=0;i<image->export_count;i++)
    {
        ptr[0] += dseg_offset;  ///fix name pointer
        if (nxc_script_image_do_test_dptr(image,(void*)ptr[1]))
        {
            ptr[1] += dseg_offset;  ///fix address
        }
        else
        {
            ptr[1] += xseg_offset;  ///fix address
        }
        
        ptr+=2;
    }

    ///fix IMP-table dseg_offset (name dseg_offset)...
    ptr = (long *)(new_data_base + image->import_table_offset);
    for (i=0;i<image->import_count;i++)
    {
        ptr[0] += dseg_offset;  ///fix name pointer ...
        ptr[1] += xseg_offset;  ///fix instruction pointer
        //printf("-=-=-=name:::%s\n",ptr[0]);
        ptr += 2;
    }

    ///fix RELOC-table dseg_offset ...
    ptr = (long *)(new_data_base + image->reloc_table_offset);
    for (i=0;i<image->reloc_count;i++)
    {
        ///ptr[0] is address if instruction 'load Reg , $XXX'
        ///so ptr[0] should add text-seg-offset
        ptr[1] += xseg_offset;      ///fix pointer first (just add offset) ...
        if (ptr[0] == 0) ///Data Seg Relocation
        {
            * (long *)ptr[1] += dseg_offset;
        }
        else if(ptr[0] == 1)
        {
            * (long *)ptr[1] += xseg_offset;
        }
        else
        {
            //printf("************Invalid Relocation Table!!!**********\n");
            return -1;
            break;
        }
        
        ptr+=2;
    }
    ///set New Data-BaseAddr and TextBase so that the new comer will find a way 
    ///to fix IMP-Table!!!
    image->text_base = (nxc_instr_t *)new_text_base;
    image->data_base = (char*)new_data_base;

    return 0;
}

/**
 * fix import table , fix api address ...
 * @return 0 means okay , none-zero means unresolved symbol name ...
 */
static char* nxc_script_image_do_fix_api(nxc_script_image_t *image,
                                         nxc_api_table_t *api_table)
{
    long *ptr;
    int i;
    void *addr;

    ptr = (long *)((long)image->data_base + image->import_table_offset);
    for (i=0;i<image->import_count;i++)
    {
        addr = nxc_find_api(api_table,(char*)ptr[0]);
        if (!addr) return (char*)ptr[0];
        *(void **)ptr[1] = addr; ///find target by pointer ...
        ptr+=2;
    }

    return 0;
}

/**
 * find symbol address by name ...
 */
___fast void*nxc_script_image_do_find_sym_addr(nxc_script_image_t*image,
                                               char*name)
{
    long *ptr;
    int i,len;

    len = nxc_strlen(name);
    ptr = (long *)((long)image->data_base + image->export_table_offset);
    for (i=0;i<image->export_count;i++)
    {
        if (!nxc_memcmp((void*)ptr[0],name,len)) return (void*)ptr[1];
        ptr+=2;
    }
    
    return 0;
}

/**===========================================================================*/
/**===========================================================================*/
/**===========================================================================*/

/**
 * the virtual machine's context ...
 */
struct _nxc_vm_ctx 
{
    long           *stack;         ///current "process" 's stack start addr
    int             stack_size;    ///total size of stack allocated ...
    long            last_error;    ///save the last error code...
    ///-----virtual cpu reg-----
    nxc_instr_t    *eip;            ///instruction pointer
    long            ebp;           ///link register (frame base)...
    long           *esp;           ///stack top
    long            eax;           ///function ret val
    long            edx;           ///for 64bit under 32bit env
    long            eflag;         ///flag register ...
};


///#############################################################################
/**
 * init a compiler object ...
 * @return 0 means okay ...
 */
int nxc_init_compiler(nxc_compiler_t *compiler,
                      nxc_allocator_t malloc_proc,
                      nxc_deallocator_t free_proc,
                      void*alloc_data,
                      char*parse_buff,
                      int max_parse_buff);

/**
 * finit a compiler object ...
 */
int nxc_fini_compiler(nxc_compiler_t *compiler);

/**
 * compile the given source string and generate marco-instructions...
 * return 0 means okay ...
 * @return non-zero means error.
 */
nxc_script_image_t* nxc_do_compile(nxc_compiler_t*const compiler,
                                   int count,
                                   char *fname_table[],
                                   char *source_table[]);

/**
 * compile source and generate a script image ...
 * @return script image 
 */
___fast nxc_script_image_t *nxc_compile_ex(char*source,char*fname,
                                           nxc_allocator_t malloc_proc,
                                           nxc_deallocator_t free_proc,
                                           void*alloc_data,
                                           nxc_vprintf_t output_proc)
{
    nxc_script_image_t *image;
    char*fname_table[1];
    char*source_table[1];
    nxc_compiler_t compiler[1];
    char parse_buff[8004];

    ///initialize the compiler ...
    nxc_init_compiler(compiler,malloc_proc,free_proc,alloc_data,parse_buff,8000);
    ///install output-handler ...
    compiler->vprintf_proc = output_proc;
    compiler->enable_debug_info = 1;

    ///compile script ...
    fname_table[0] = fname;
    source_table[0] = source;
    image = nxc_do_compile(compiler,1,fname_table,source_table);

    nxc_fini_compiler(compiler);

    return image;
}

/**
 * compile the given source string and generate marco-instructions...
 * return 0 means error ...
 * @return non-zero means error.
 */
___fast nxc_script_image_t*nxc_compile(nxc_compiler_t*const compiler,
                                       char*fname,
                                       char*str)
{
    char *fname_table[1];
    char *source_table[1];

    fname_table[0] = fname;
    source_table[0] = str;
    return nxc_do_compile(compiler,1,fname_table,source_table);
}

/**
 * call specific function under given environment ...
 * return value stored in _ctx->eax
 */
long nxc_do_call(nxc_vm_ctx_t*_ctx,nxc_instr_t *xaddr,int argc,void*argv[]);

/**
 * init a execute context from specific vm ...
 */
___fast void nxc_init_vm_ctx(nxc_vm_ctx_t *ctx,void* stack,int stack_size)
{
    nxc_memset(ctx,0,sizeof(*ctx));
    ///using private stack ...
    ctx->stack = (long*)stack;
    ctx->stack_size = stack_size;
    ///reset last error ...
    ctx->last_error = 0;

    ctx->esp = (long *)((long)ctx->stack + ctx->stack_size);
}

/**
 * call specific function under specific vm ...
 * @return 0 means okay
 * @return non-zero means error ...
 * return value stored in _ctx->eax
 */
___fast long nxc_call_ex(nxc_instr_t*xaddr,void*stack,int stack_size,int argc,void*argv[])
{
    nxc_vm_ctx_t ctx;

    ///init default ctx for call ...
    nxc_init_vm_ctx(&ctx,stack,stack_size);

    return nxc_do_call(&ctx,xaddr,argc,argv);
}

/**
 * call specific function under specific vm ...
 * @return 0 means okay
 * @return non-zero means error ...
  * return value stored in _ctx->eax
 */
___fast long nxc_call(nxc_instr_t*xaddr,int argc,...)
{
    int i;
    nxc_va_list argp;
    nxc_vm_ctx_t ctx;
    void *argv[16];
    #define nxc_vm_default_stack_size 64*1024
    char stack[nxc_vm_default_stack_size];///stack size 64KB

    ///init default ctx for call ...
    nxc_init_vm_ctx(&ctx,(void*)stack,nxc_vm_default_stack_size);

    if (argc>15) argc = 15;

    nxc_va_start(argp,argc);
    for (i=0;i<argc;i++) 
    {
        argv[i]=nxc_va_arg(argp,void *);
    }
    nxc_va_end(argp);

    return nxc_do_call(&ctx,xaddr,argc,argv);
}

#endif
