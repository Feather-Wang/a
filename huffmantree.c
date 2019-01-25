#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "huffmantree.h"

int huffmantree_create(huffmantree_t **header)
{
    *header = malloc(sizeof(huffmantree_t));
    if( NULL == *header )
        return -1;
    (*header)->value = -1;
    (*header)->left = NULL;
    (*header)->right = NULL;

    return 0;
}

int huffmantree_insert(huffmantree_t **node, int plies_node, int value, int plies_value)
{
    huffmantree_t *node_temp = NULL;
    int ret = -1;

    if( NULL == (*node) )
    {
        node_temp = NULL;
        ret = huffmantree_create(&node_temp); 
        if( -1 == ret )
        {
            return -1;
        }
        *node = node_temp;
    }

    if( -1 != (*node)->value )
    {
        /*
        printf("[judmp] plies_node=[%d], there is a value=[%d]\n", plies_node, (*node)->value);
        */
        return 1;
    }

    if( plies_node < plies_value )
    {
        /*
        printf("[judmp] plies_node=[%d], left\n", plies_node);
        */
        ret = huffmantree_insert(&((*node)->left), plies_node+1, value, plies_value);
        if( 0 > ret )
            return -2;
        else if( 0 == ret )
            return 0;

        /*
        printf("[judmp] plies_node=[%d], right\n", plies_node);
        */
        ret = huffmantree_insert(&((*node)->right), plies_node+1, value, plies_value);
        if( 0 > ret )
            return -2;
        else if( 0 == ret )
            return 0;
    }
    else if( plies_node == plies_value )
    {
        if( -1 == (*node)->value )
        {
            /*
            printf("plies_node = [%d], insert value = [%d]\n", plies_node, value);
            */
            (*node)->value = value;
            return 0;
        }
    }

    return 1;
}

int huffmantree_search(huffmantree_t *header, int jump)
{
    static huffmantree_t *node = NULL;
    int value;

    if( NULL == node )
        node = header; 
    if( 0 == jump )
        node = node->left;
    else if( 1 == jump )
        node = node->right;
    if( NULL == node )
        return -2;

    value = node->value;
    if( -1 != node->value )
        node = NULL;

    return value;
}

void huffmantree_show(huffmantree_t *header, int plies)
{
    if( NULL != header->left )
        huffmantree_show(header->left, plies+1);
    printf("[huffmantree_show] plies=[%d], value=[%d]\n", plies, header->value);
    if( NULL != header->right )
        huffmantree_show(header->right, plies+1);
}

/*
int main(int argc, const char *argv[])
{
    //int huffmantree[] = {3, 5, 5, 5, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 3};
    int huffmantree[] = {3, 4, 4, 4, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 3};
    int i = 0, j = 0;
    huffmantree_t *header = NULL;
    int ret = -1;

    //构造huffman树
    //i表示层数，j表示数组下标
    for(i = 1; i < 19; i++)
    {
        for(j = 0; j < 19; j++)
        {
            if( i == huffmantree[j] && i != 0 )
            {
                printf("plies=[%d] value=[%d]\n", i, j);
                huffmantree_insert(&header, 0, j, huffmantree[j]);
            }
        }
    }

    printf("-----------show-------------------\n");
    huffmantree_show(header, 0);
    printf("-----------show end-------------------\n");

    //搜索某个huffman
    int huffmanvalue[] = {1, 0, 0};
    int value = -1;
    for(i = 0; i < (argc-1); i++)
    {
        value = huffmantree_search(header, atoi(argv[i+1])); 
    }
    printf("value=[%d]\n", value);
    return 0;
}
*/
