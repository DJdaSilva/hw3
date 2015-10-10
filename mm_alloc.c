/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines. Remove this comment and provide
 * a summary of your allocator's design here.
 */

#include "mm_alloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

/* Your final implementation should comment out this macro. */
//#define MM_USE_STUBS

s_block_ptr start = NULL;
s_block_ptr end = NULL;


s_block_ptr extend_heap(s_block_ptr last, size_t s){
	s_block_ptr new = (s_block_ptr)sbrk(BLOCK_SIZE + s);
	if (new == NULL) return NULL;
	
	if (start == NULL) {
		s_block_ptr dummy = (s_block_ptr)sbrk(BLOCK_SIZE);
		dummy->size = (size_t)0;
		dummy->next = NULL;
		dummy->prev = NULL;
		dummy->free = 0;
		dummy->ptr = NULL;
		start = dummy;
		end = start;
	}
	
	end->next = new;
	new->prev = end;
	new->next = NULL;
	new->size = s;
	new->ptr = new->data;
	new->free = 1;
	end = new;
	return new;
}

s_block_ptr fusion(s_block_ptr b){
	//Fuse block b with next and previous if they exist and are free
	s_block_ptr curr = b;
	int count;
	int wentLeft = 0;
	int wentRight = 0;
	//Forward
	count = 0;
	while ( (curr->next != NULL) && (curr->next->free) ){
		wentRight = 1;
		count += (curr->next->size + BLOCK_SIZE);
		curr = curr->next;
	}
	
	if (wentRight){
		b->size += count;
		b->next = curr->next;
		curr->next->prev = b;
	}

	//Backward
	count = 0;
	while ( (curr->prev != NULL) && (curr->prev->free) ){
		wentLeft = 1;
		count += (curr->prev->size + BLOCK_SIZE);
		curr = curr->prev;
	}
	
	if (wentLeft){
		curr->prev->size += count;
		curr->prev->next = b->next;
		b->next->prev = curr->prev;
		return curr->prev;
	}
	
	return b;
}

void split_block(s_block_ptr b, size_t s){
	s_block_ptr new = (s_block_ptr)(b->next-(BLOCK_SIZE + s));
	new->size = s;
	new->next = b->next;
	new->prev = b;
	new->free = 1;
	new->ptr = new->data;
	b->size -= (s+BLOCK_SIZE);
	b->next = new;
	
	if (new->next != NULL) new->next->prev = new;
}

s_block_ptr get_block(void *p){
	return p -= BLOCK_SIZE;
}

void* mm_malloc(size_t size)
{	
	s_block_ptr curr = start;
	while ( (curr != NULL)&&((curr->free == 0)||(curr->size < size)) ){
		curr = curr->next;
	}
	
	if (curr == NULL){
		s_block_ptr new = extend_heap(end, size);
		new->free = 0;
		return new->ptr;
	}
	else return (curr->ptr);
}

void* mm_realloc(void* ptr, size_t size)
{
	void* result = mm_malloc(size);
	memcpy(result, ptr, size);
	return result;
}

void mm_free(void* ptr)
{
	s_block_ptr curr = get_block(ptr);
	curr->free = 1;
	fusion(curr);
}
