#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

typedef struct Node_ {
	struct Node_ *left, *right, *parent;
	int key;
} Node;

Node *ROOT = NULL;

bool COMPARE(int a, int b) {
	return a < b;
}

Node *NEW_NODE(int key) {
	Node *tmp = malloc(sizeof(Node));
	assert(tmp != NULL);
	tmp->parent = NULL;
	tmp->left = NULL;
	tmp->right = NULL;
	tmp->key = key;
	return tmp;
}

void FREE_NODE(Node *x) {
	free(x);
}

void left_rotate(Node *x) {
	Node *y = x->right;
	if (y) {
		x->right = y->left;
		if (y->left) y->left->parent = x;
		y->parent = x->parent;
	}

	if (x->parent == NULL) ROOT = y;
	else if (x == x->parent->left) x->parent->left = y;
	else x->parent->right = y;
	if (y) y->left = x;
	x->parent = y;
}

void right_rotate(Node *x) {
	Node *y = x->left;
	if (y) {
		x->left = y->right;
		if (y->right) y->right->parent = x;
		y->parent = x->parent;
	}
	if (x->parent == NULL) ROOT = y;
	else if (x == x->parent->left) x->parent->left = y;
	else x->parent->right = y;
	if (y) y->right = x;
	x->parent = y;
}

void splay(Node *x) {
	while (x->parent) {
		if (x->parent->parent == NULL) {
			if (x->parent->left == x) right_rotate(x->parent);
			else left_rotate(x->parent);
		} else if (x->parent->left == x && x->parent->parent->left == x->parent) {
			right_rotate(x->parent->parent);
			right_rotate(x->parent);
		} else if (x->parent->right == x && x->parent->parent->right == x->parent) {
			left_rotate(x->parent->parent);
			left_rotate(x->parent);
		} else if (x->parent->left == x && x->parent->parent->right == x->parent) {
			right_rotate(x->parent);
			left_rotate(x->parent);
		} else {
			left_rotate(x->parent);
			right_rotate(x->parent);
		}
	}
}

void replace(Node *u, Node *v) {
	if (u->parent == NULL) ROOT = v;
	else if (u == u->parent->left) u->parent->left = v;
	else u->parent->right = v;
	if (v) v->parent = u->parent;
}

Node *subtree_minimum(Node *u) {
	while (u->left) u = u->left;
	return u;
}

Node *subtree_maximum(Node *u) {
	while (u->right) u = u->right;
	return u;
}

void insert(int key) {
	Node *z = ROOT;
	Node *p = NULL;

	while (z) {
		p = z;
		if (COMPARE(z->key, key)) z = z->right;
		else z = z->left;
	}

	z = NEW_NODE(key);
	z->parent = p;

	if (!p) ROOT = z;
	else if (COMPARE(p->key, z->key)) p->right = z;
	else p->left = z;

	splay(z);
}

Node *find(int key) {
	Node *z = ROOT;
	while (z) {
		if (COMPARE(z->key, key)) z = z->right;
		else if (COMPARE(key, z->key)) z = z->left;
		else return z;
	}
	return NULL;
}

void erase(int key) {
	Node *z = find(key);
	if (!z) return;

	splay(z);

	if (!z->left) replace(z, z->right);
	else if (!z->right) replace(z, z->left);
	else {
		Node *y = subtree_minimum(z->right);
		if (y->parent != z) {
			replace(y, y->right);
			y->right = z->right;
			y->right->parent = y;
		}
		replace(z, y);
		y->left = z->left;
		y->left->parent = y;
	}

	FREE_NODE(z);
}

void PRINT_TREE(Node *root) {
	if (root == NULL) return;
	PRINT_TREE(root->left);
	printf("%d ", root->key);
	PRINT_TREE(root->right);
}


int main() {
	insert(7);
	insert(1000);
	insert(14);
	insert(1);
	erase(1000);
	insert(2);
	insert(3);

	PRINT_TREE(ROOT);

	return 0;
}