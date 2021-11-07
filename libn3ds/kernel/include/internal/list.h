#pragma once

// Based on https://github.com/torvalds/linux/blob/master/include/linux/list.h

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>


#define LIST_INIT_VAL(name)  ((ListNode){&(name), &(name)})

#define LIST_ENTRY(ptr, type, member)              \
({                                                 \
	void *__mptr = (void*)(ptr);                   \
	(type*)(__mptr - (size_t)&((type*)0)->member); \
})

#define LIST_FIRST_ENTRY(ptr, type, member) \
	LIST_ENTRY((ptr)->next, type, member)

#define LIST_NEXT_ENTRY(pos, member) \
	LIST_ENTRY((pos)->member.next, typeof(*(pos)), member)

#define LIST_FOR_EACH_ENTRY(pos, start, member)              \
	for(pos = LIST_FIRST_ENTRY(start, typeof(*pos), member); \
	    &pos->member != (start);                             \
	    pos = LIST_NEXT_ENTRY(pos, member))


typedef struct ListNode ListNode;
struct ListNode
{
	ListNode *next;
	ListNode *prev;
};
//static_assert(offsetof(ListNode, next) == 0, "Error: Member next of ListNode is not at offset 0!");



static inline void listInit(ListNode *start)
{
	*start = LIST_INIT_VAL(*start);
}

static inline bool listEmpty(const ListNode *start)
{
	return start->next == start;
}

// Internal function. Don't use unless you know what you are doing!
static inline void _listAdd(ListNode *node, ListNode *next, ListNode *prev)
{
	node->next = next;
	node->prev = prev;
	next->prev = node;
	prev->next = node;
}

static inline void listAddBefore(ListNode *entry, ListNode *node)
{
	_listAdd(node, entry, entry->prev);
}

static inline void listAddAfter(ListNode *entry, ListNode *node)
{
	_listAdd(node, entry->next, entry);
}

// Internal function. Don't use unless you know what you are doing!
static inline void _listDelete(ListNode *next, ListNode *prev)
{
	next->prev = prev;
	prev->next = next;
}

static inline void listDelete(ListNode *entry)
{
	_listDelete(entry->next, entry->prev);
}

static inline ListNode* listRemoveTail(ListNode *start)
{
	ListNode *const node = start->next;

	listDelete(node);

	return node;
}

static inline ListNode* listRemoveHead(ListNode *start)
{
	ListNode *const node = start->prev;

	listDelete(node);

	return node;
}

// Some function aliases for queues.
#define listPush(start, node)      listAddBefore((start), (node))
#define listPop(start)             listRemoveTail((start))
#define listPushTail(start, node)  listAddAfter((start), (node))
#define listPopHead(start)         listRemoveHead((start))
