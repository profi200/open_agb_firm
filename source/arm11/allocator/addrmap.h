#pragma once

static rbtree_t sAddrMap;

struct addrMapNode
{
	rbtree_node node;
	MemChunk chunk;
};

#define getAddrMapNode(x) rbtree_item((x), addrMapNode, node)

static int addrMapNodeComparator(const rbtree_node_t* _lhs, const rbtree_node_t* _rhs)
{
	auto lhs = getAddrMapNode(_lhs)->chunk.addr;
	auto rhs = getAddrMapNode(_rhs)->chunk.addr;
	if (lhs < rhs)
		return -1;
	if (lhs > rhs)
		return 1;
	return 0;
}

static void addrMapNodeDestructor(rbtree_node_t* a)
{
	free(getAddrMapNode(a));
}

static addrMapNode* getNode(void* addr)
{
	addrMapNode n;
	n.chunk.addr = (u8*)addr;
	auto p = rbtree_find(&sAddrMap, &n.node);
	return p ? getAddrMapNode(p) : nullptr;
}

static addrMapNode* newNode(const MemChunk& chunk)
{
	auto p = (addrMapNode*)malloc(sizeof(addrMapNode));
	if (!p) return nullptr;
	p->chunk = chunk;
	return p;
}

static void delNode(addrMapNode* node)
{
	rbtree_remove(&sAddrMap, &node->node, addrMapNodeDestructor);
}
