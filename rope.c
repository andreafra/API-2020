#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>


int main() {
	Tree *tree = createTree();
	Insert(&tree, 0, "A");
	Insert(&tree, 1, "B");
	Insert(&tree, 2, "C");
	Insert(&tree, 3, "D");
	PRINT_TREE(tree);

	for (List *curr = Report(tree, 3,3); curr; curr = curr->next) {
		printf("%s ", curr->value->value);
	}

	destroyTree(tree);

	return 0;
}
