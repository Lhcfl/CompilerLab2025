#ifndef CMM_AVL_H
#define CMM_AVL_H

#include "predefines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// AVL 树节点
typedef struct AVLTree {
    char*           key;
    void*           value;
    struct AVLTree* left;
    struct AVLTree* right;
    int             height;
} AVLTree;

// 创建一个新节点
AVLTree* avl_create_tree(const char* key, void* value) {
    AVLTree* node = (AVLTree*)malloc(sizeof(AVLTree));
    node->key     = cmm_clone_string(key);
    node->value   = value;
    node->left    = NULL;
    node->right   = NULL;
    node->height  = 1;
    return node;
}

// 获取节点高度
int avl_get_height(AVLTree* node) { return node ? node->height : 0; }

// 计算平衡因子
int avl_get_balance(AVLTree* node) {
    return node ? avl_get_height(node->left) - avl_get_height(node->right) : 0;
}

// 右旋
AVLTree* avl_rotate_right(AVLTree* y) {
    AVLTree* x  = y->left;
    AVLTree* T2 = x->right;
    x->right    = y;
    y->left     = T2;
    y->height   = 1 + (avl_get_height(y->left) > avl_get_height(y->right)
                           ? avl_get_height(y->left)
                           : avl_get_height(y->right));
    x->height   = 1 + (avl_get_height(x->left) > avl_get_height(x->right)
                           ? avl_get_height(x->left)
                           : avl_get_height(x->right));
    return x;
}

// 左旋
AVLTree* avl_rotate_left(AVLTree* x) {
    AVLTree* y  = x->right;
    AVLTree* T2 = y->left;
    y->left     = x;
    x->right    = T2;
    x->height   = 1 + (avl_get_height(x->left) > avl_get_height(x->right)
                           ? avl_get_height(x->left)
                           : avl_get_height(x->right));
    y->height   = 1 + (avl_get_height(y->left) > avl_get_height(y->right)
                           ? avl_get_height(y->left)
                           : avl_get_height(y->right));
    return y;
}

// 插入节点
AVLTree* avl_insert_node(AVLTree* node, const char* key, void* value) {
    if (!node) return avl_create_tree(key, value);
    int cmp = strcmp(key, node->key);
    if (cmp < 0)
        node->left = avl_insert_node(node->left, key, value);
    else if (cmp > 0)
        node->right = avl_insert_node(node->right, key, value);
    else {
        node->value = value;
        return node;
    }   // 更新值
    node->height = 1 + (avl_get_height(node->left) > avl_get_height(node->right)
                            ? avl_get_height(node->left)
                            : avl_get_height(node->right));
    int balance  = avl_get_balance(node);
    if (balance > 1 && strcmp(key, node->left->key) < 0) return avl_rotate_right(node);
    if (balance < -1 && strcmp(key, node->right->key) > 0) return avl_rotate_left(node);
    if (balance > 1 && strcmp(key, node->left->key) > 0) {
        node->left = avl_rotate_left(node->left);
        return avl_rotate_right(node);
    }
    if (balance < -1 && strcmp(key, node->right->key) < 0) {
        node->right = avl_rotate_right(node->right);
        return avl_rotate_left(node);
    }
    return node;
}

// 查找节点
void* avl_search_tree(AVLTree* node, const char* key) {
    while (node) {
        int cmp = strcmp(key, node->key);
        if (cmp == 0) return node->value;
        node = (cmp < 0) ? node->left : node->right;
    }
    return NULL;
}

// 释放 AVL 树
void avl_free_tree(AVLTree* node) {
    if (!node) return;
    avl_free_tree(node->left);
    avl_free_tree(node->right);
    free(node->key);
    free(node);
}

void avl_iterate_node(AVLTree* node, void fp(AVLTree* node)) {
    if (!node) return;
    avl_iterate_node(node->left, fp);
    avl_iterate_node(node->right, fp);
    fp(node);
}

// 测试示例
int avl_test_case() {
    AVLTree* root = NULL;
    int      val1 = 42, val2 = 84, val3 = 168;
    root     = avl_insert_node(root, "apple", &val1);
    root     = avl_insert_node(root, "banana", &val2);
    root     = avl_insert_node(root, "cherry", &val3);
    int* res = (int*)avl_search_tree(root, "banana");
    if (res)
        printf("Found: %d\n", *res);
    else
        printf("Not found\n");
    avl_free_tree(root);
    return 0;
}


#endif