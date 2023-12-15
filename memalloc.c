#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

struct header {
    size_t size;
    unsigned is_free;
    struct header *next;

};

struct header *head = NULL, *tail = NULL;
pthread_mutex_t mutex;

struct header *get_free_block(size_t size)
{
	struct header *curr = head;

	while(curr) {
		if (curr->is_free && curr->size >= size)
			return curr;

		curr = curr->next;
	}

	return NULL;
}

/* Returns memory on the heap, assumes a sensible size value */
void *malloc(size_t size)
{
	struct header *header;
	size_t total_size = sizeof(*header) + size;
	void *block;

	pthread_mutex_lock(&mutex);

	header = get_free_block(size);
	if (header) {
        /* Use block that we previously allocated */
		header->is_free = 0;
		goto out;
	}

	block = sbrk(total_size);
	if (sbrk(total_size) == (void*) -1) {
		pthread_mutex_unlock(&mutex);
		return NULL;
	}

	header = block;
	header->size = size;
	header->is_free = 0;
	header->next = NULL;
	if (!head)
		head = header;
	if (tail)
		tail->next = header;

	tail = header;
out:
	pthread_mutex_unlock(&mutex);

    /* Return the start of allocated memory (skip the header) */
	return (void*)(header + 1);
}

void free(void *block)
{
	struct header *header, *tmp;
	void *program_break;

	pthread_mutex_lock(&mutex);

	header = (struct header*)block - 1;

	program_break = sbrk(0);
	if ((char*)block + header->size == program_break) {
		/* Block is at the top of the heap */
		if (head == tail)
			head = tail = NULL;
		else {
			tmp = head;
			while (tmp) {
				if(tmp->next == tail) {
					tmp->next = NULL;
					tail = tmp;
				}
				tmp = tmp->next;
			}
		}
		sbrk(0 - sizeof(struct header) - header->size);
		goto out;
	}
	header->is_free = 1;
out:
	pthread_mutex_unlock(&mutex);
}

void *calloc(size_t num, size_t nsize)
{
	size_t size;
	void *block;

	size = num * nsize;

	block = malloc(size);
	if (!block)
		return NULL;

	memset(block, 0, size);

	return block;
}

void *realloc(void *block, size_t size)
{
	struct header *header;
	void *ret;

	header = (struct header*)block - 1;
	if (header->size >= size)
		return block;

	ret = malloc(size);
	if (ret) {
		memcpy(ret, block, header->size);
		free(block);
	}

	return ret;
}