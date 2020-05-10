#ifndef _LIST_H
#define _LIST_H

#include "ops.h"

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

struct list_head {
    struct list_head *prev, *next;
};


static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
	list->prev = list;
}

/* new entry를 prev와 next entry 사이에 insert */
static inline void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;    
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);      //new entry를 list의 맨 앞(head 다음)에 insert
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);      //new entry를 list의 맨 뒤(head 전)에 insert
}

/* list entry의 prev와 next를 서로 연결하여  list entry를 lsit에서 delete 함 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void __list_del_entry(struct list_head *entry)
{
    if(!entry)
        return;

    __list_del(entry->prev, entry->next);
}

/* list를 delete하고 head의 가장 앞에 insert */
static inline void list_move(struct list_head *list, struct list_head *head)
{
    __list_del_entry(list);
    list_add(list, head);
}

/* list를 delete하고 head의 가장 뒤에 insert */
static inline void list_move_tail(struct list_head *list, struct list_head *head)
{
    __list_del_entry(list);
    list_add_tail(list, head);
}

#endif
