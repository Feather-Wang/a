#ifndef _HUFFMANTREE_H_

#define _HUFFMANTREE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wqs_unzip.h"

struct huffmantree_s 
{
    int value;
    struct huffmantree_s *left;
    struct huffmantree_s *right;
};
typedef struct huffmantree_s huffmantree_t;

extern int huffmantree_create(huffmantree_t **header);
extern int huffmantree_insert(huffmantree_t **node, int plies_node, int value, int plies_value);
extern int huffmantree_search(huffmantree_t *header, int jump);
extern void huffmantree_show(huffmantree_t *header, int plies);

#endif /* end of include guard: _HUFFMANTREE_H_ */
