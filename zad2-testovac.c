#define _CRT_SECURE_NO_WARNINGS



#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// find max from a and b
static int maxi(int a, int b)
{
	return a > b ? a : b;
}

unsigned long djb2(unsigned char *str)
{
	unsigned long hash = 5381;
	int c;

	while (c = *str++) {
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	}

	return hash;
}


// A struct respresenting a page
typedef struct pagenode {
	char *page; // page name
	int count; // user liked this page
	struct usernode *root; // root of avl tree of users
} PAGE;

// A struct representing a user
typedef struct usernode {
	char *user; // user name
	int height; // height in avl tree
	int count; // number of ALL successors in avl tree
	struct usernode *left;
	struct usernode *right;
} USER;

// hash table for pages
PAGE **table;
// size of hash table for pages
int size;

PAGE *newPage(char *page) {
	PAGE *node = (PAGE *)malloc(sizeof(PAGE));
	node->page = (char *)malloc((strlen(page) + 1) * sizeof(char));
	node->page = strcpy(node->page, page);
	node->count = 0;
	node->root = NULL;
	return node;
}

USER *newUser(char *user) {
	USER *node = (USER *)malloc(sizeof(USER));
	node->user = (char *)malloc((strlen(user) + 1) * sizeof(char));
	node->user = strcpy(node->user, user);
	node->height = 0;
	node->count = 0;
	node->left = node->right = NULL;
	return node;
}


// get height of node in avl tree
static int height(USER* n)
{
	if (n == NULL)
		return -1;
	else
		return n->height;
}

// get balance of node in avl tree
int balance(USER *node)
{
	if (node == NULL)
		return 0;
	return height(node->left) - height(node->right);
}

// rotate left
static USER* SRL(USER* k2)
{
	USER* k1 = NULL;

	k1 = k2->left;


	// change number of chidren correspondingly
	if (k2->right == NULL) {
		k1->count += 1;
	}
	else {
		k1->count += 2;
		k1->count += k2->right->count;
	}

	if (k1->left == NULL) {
		k2->count -= 1;
	}
	else {
		k2->count -= 2;
		k2->count -= k1->left->count;
	}


	k2->left = k1->right;
	k1->right = k2;

	// change height correspondingly
	k2->height = maxi(height(k2->left), height(k2->right)) + 1;
	k1->height = maxi(height(k1->left), k2->height) + 1;
	return k1;
}

// rotate right
static USER* SRR(USER* k1)
{
	USER* k2;

	k2 = k1->right;


	// change number of chidren correspondingly
	if (k1->left == NULL) {
		k2->count += 1;
	}
	else {
		k2->count += 2;
		k2->count += k1->left->count;
	}

	if (k2->right == NULL) {
		k1->count -= 1;
	}
	else {
		k1->count -= 2;
		k1->count -= k2->right->count;
	}


	k1->right = k2->left;
	k2->left = k1;

	// change height correspondingly
	k1->height = maxi(height(k1->left), height(k1->right)) + 1;
	k2->height = maxi(height(k2->right), k1->height) + 1;

	return k2;
}

// double rotate left
static USER* DRL(USER* k3)
{
	k3->left = SRR(k3->left);

	return SRL(k3);
}

// double rotate right
static USER* DRR(USER* k1)
{
	k1->right = SRL(k1->right);

	return SRR(k1);
}

// find inorder successor
USER *smallestrightinorder(USER *node)
{
	USER *current = node;

	while (current->left != NULL)
		current = current->left;

	return current;
}

// get hash for page in table range
int code(char *str) {
	return (int)(djb2(str) % size);
}



// insert to avl tree
USER* insert(char *user, USER* nnn)
{
	int comp;

	if (nnn == NULL)
	{
		// reached bottom
		nnn = newUser(user);
	}
	else {
		// increment number of children for all nodes on the way
		(nnn->count)++;

		comp = strcmp(user, nnn->user);

		if (comp < 0)
		{
			// before in alphabet
			nnn->left = insert(user, nnn->left);

			comp = strcmp(user, nnn->left->user);

			// balance avl tree
			if (balance(nnn) == 2) {
				if (comp < 0) {
					nnn = SRL(nnn);
				}
				else {
					nnn = DRL(nnn);
				}
			}
		}
		else if (comp > 0)
		{
			// after in alphabet
			nnn->right = insert(user, nnn->right);

			comp = strcmp(user, nnn->right->user);

			// balance avl tree
			if (balance(nnn) == -2) {
				if (comp > 0) {
					nnn = SRR(nnn);
				}
				else {
					nnn = DRR(nnn);
				}
			}
		}
	}

	// set also height
	nnn->height = maxi(height(nnn->left), height(nnn->right)) + 1;
	return nnn;
}

// delete from avl tree
USER* delete(char *user, USER* nnn)
{
	int comp, bal;
	USER *temp;

	if (nnn == NULL)
	{
		// reached bottom
		return nnn;
	}

	// decrement number of children for all nodes on the way
	(nnn->count)--;

	comp = strcmp(user, nnn->user);

	if (comp < 0)
	{
		// before in alphabet
		nnn->left = delete(user, nnn->left);
	}
	else if (comp > 0)
	{
		// after in alphabet
		nnn->right = delete(user, nnn->right);
	}
	else {
		// delete this
		if ((nnn->left == NULL) || (nnn->right == NULL))
		{
			// a NULL child
			temp = nnn->left ? nnn->left : nnn->right;

			if (temp == NULL)
			{
				// both children NULL
				temp = nnn;
				nnn = NULL;
			}
			else {
				// a single children NULL
				*nnn = *temp;
			}
			free(temp);
		}
		else
		{
			// no NULL children
			temp = smallestrightinorder(nnn->right);

			nnn->user = temp->user;

			nnn->right = delete(temp->user, nnn->right);
		}
	}

	if (nnn == NULL) {
		return nnn;
	}

	// set also height
	nnn->height = maxi(height(nnn->left), height(nnn->right)) + 1;

	// balance avl tree
	bal = balance(nnn);

	if (bal > 1 && balance(nnn->left) >= 0) {
		return SRL(nnn);
	}

	if (bal > 1 && balance(nnn->left) < 0)
	{
		return DRL(nnn);
	}

	if (bal < -1 && balance(nnn->right) <= 0) {
		return SRR(nnn);
	}

	if (bal < -1 && balance(nnn->right) > 0)
	{
		return DRR(nnn);
	}

	return nnn;
}

// find k-th user alphabetically
USER *kth(int k, USER* nnn)
{
	int lcount;

	if (nnn->left == NULL) {
		// no left children
		lcount = 0;
	}
	else {
		// count left chidren
		lcount = nnn->left->count + 1;
	}

	if (k < lcount + 1) {
		// k-th node somewhere on the left
		return kth(k, nnn->left);
	}
	else if (k == lcount + 1) {
		// k-th node somewhere on the right
		return nnn;
	}
	else {
		// this is k-th node
		return kth(k - lcount - 1, nnn->right);
	}
}



// initialize global variables
void init()
{
	int i;

	// choose size
	size = 10007;

	// found hash table
	table = (PAGE **)malloc(size * sizeof(PAGE *));
	for (i = 0; i < size; i++) {
		table[i] = NULL;
	}
}

// user liked page
void like(char *page, char *user)
{
	int i, position;

	i = 1;
	// use hash algorithm
	position = code(page);

	while (table[position] != NULL) {
		// page already created in hash table

		if (strcmp(table[position]->page, page) == 0) {

			table[position]->root = insert(user, table[position]->root);
			// increment number of users
			(table[position]->count)++;
			return;
		}

		// linear probing
		position = (position + i) % size;
		i++;
	}

	// page for the first time
	table[position] = newPage(page);
	table[position]->root = insert(user, table[position]->root);
	// increment number of users
	(table[position]->count)++;
}

// user unliked page
void unlike(char *page, char *user)
{
	int i, position;

	i = 1;
	// use hash algorithm
	position = code(page);

	while (table[position] != NULL) {
		// page must be created in hash table

		if (strcmp(table[position]->page, page) == 0) {

			table[position]->root = delete(user, table[position]->root);
			// decrement number of users
			(table[position]->count)--;
			return;
		}

		// linear probing
		position = (position + i) % size;
		i++;
	}
	// unreachable
}

// get k-th user liked page alphabetically
char *getuser(char *page, int k)
{
	int i, position;
	USER *found;

	i = 1;
	// use hash algorithm
	position = code(page);

	while (table[position] != NULL) {
		// page must be created in hash table

		if (strcmp(table[position]->page, page) == 0) {

			if (k > table[position]->count) {
				// k out of range
				return NULL;
			}
			found = kth(k, table[position]->root);
			if (found == NULL) {
				// k not found
				return NULL;
			}
			return found->user;
		}

		// linear probing
		position = (position + i) % size;
		i++;
	}
	// unreachable
	return NULL;
}


int test_simple() {

	char *user;

	printf("TEST SIMPLE\n");

	init();

	printf("like: facebook, ludgo --> ");
	like("facebook", "ludgo");
	printf("OK\n");
	printf("like: facebook, pista --> ");
	like("facebook", "pista");
	printf("OK\n");
	printf("like: facebook, boris --> ");
	like("facebook", "boris");
	printf("OK\n");
	printf("like: facebook, olo --> ");
	like("facebook", "olo");
	printf("OK\n");

	printf("getuser: facebook, 1 = boris --> ");
	user = getuser("facebook", 1);
	if (user == NULL || strcmp(user, "boris") != 0) {
		printf("BAD = %s\n", user);
		return 1;
	}
	printf("OK\n");
	printf("getuser: facebook, 2 = ludgo --> ");
	user = getuser("facebook", 2);
	if (user == NULL || strcmp(user, "ludgo") != 0) {
		printf("BAD = %s\n", user);
		return 1;
	}
	printf("OK\n");
	printf("getuser: facebook, 3 = olo --> ");
	user = getuser("facebook", 3);
	if (user == NULL || strcmp(user, "olo") != 0) {
		printf("BAD = %s\n", user);
		return 1;
	}
	printf("OK\n");
	printf("getuser: facebook, 4 = pista --> ");
	user = getuser("facebook", 4);
	if (user == NULL || strcmp(user, "pista") != 0) {
		printf("BAD = %s\n", user);
		return 1;
	}
	printf("OK\n");
	printf("getuser: facebook, 5 = NULL --> ");
	user = getuser("facebook", 5);
	if (user != NULL) {
		printf("BAD = %s\n", user);
		return 1;
	}
	printf("OK\n");

	printf("unlike: facebook, pista --> ");
	unlike("facebook", "pista");
	printf("OK\n");
	printf("unlike: facebook, ludgo --> ");
	unlike("facebook", "ludgo");
	printf("OK\n");

	printf("getuser: facebook, 1 = boris --> ");
	user = getuser("facebook", 1);
	if (user == NULL || strcmp(user, "boris") != 0) {
		printf("BAD = %s\n", user);
		return 1;
	}
	printf("OK\n");
	printf("getuser: facebook, 2 = olo --> ");
	user = getuser("facebook", 2);
	if (user == NULL || strcmp(user, "olo") != 0) {
		printf("BAD = %s\n", user);
		return 1;
	}
	printf("OK\n");
	printf("getuser: facebook, 3 = NULL --> ");
	user = getuser("facebook", 3);
	if (user != NULL) {
		printf("BAD = %s\n", user);
		return 1;
	}
	printf("OK\n");
	printf("getuser: facebook, 4 = NULL --> ");
	user = getuser("facebook", 4);
	if (user != NULL) {
		printf("BAD = %s\n", user);
		return 1;
	}
	printf("OK\n");
	printf("getuser: facebook, 5 = NULL --> ");
	user = getuser("facebook", 5);
	if (user != NULL) {
		printf("BAD = %s\n", user);
		return 1;
	}
	printf("OK\n");

	return 0;
}

int main()
{
	if (test_simple()) {
		printf("Test simple: Implementacia je chybna\n");
		return 1;
	}


	printf("Implementacia je OK\n");
	return 0;
}

