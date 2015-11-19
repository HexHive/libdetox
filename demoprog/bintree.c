/*
 * test the finalization of TUs, when the instruction limit for a TU is reached
 */

#include <stdio.h>
#include <stdlib.h>
#include <libfastbt.h>

#define PTRSIZE 10

struct bintree {
  struct bintree *left;
  struct bintree *right;
  int val;
};

struct bintree foobar[10];
struct bintree *root=NULL;

int insert(int val, struct bintree *ptr) {
  if (root==NULL) {
    root = malloc(sizeof(struct bintree));
    //root = &foobar[0];
    root->left=0;
    root->right=0;
    root->val=val;
    return 0;
  }
  if (val<ptr->val) {
    if (ptr->left!=NULL) {
      insert(val,ptr->left);
    } else {
      ptr->left = malloc(sizeof(struct bintree));
      //ptr->left = &foobar[1];
      ptr->left->right = NULL;
      ptr->left->left = NULL;
      ptr->left->val=val;
      return 0;
    }
  }
  if (val>ptr->val) {
    if (ptr->right!=NULL) {
      insert(val,ptr->right);
    } else {
      ptr->right = malloc(sizeof(struct bintree));
      //ptr->right = &foobar[1];
      ptr->right->right = NULL;
      ptr->right->left = NULL;
      ptr->right->val=val;
      return 0;
    }
  }
  return 1;
}

int search(int val, struct bintree *ptr) {
  if (ptr->val==val) return 1;
  if (val<ptr->val && ptr->left!=NULL) return search(val, ptr->left);
  if (val>ptr->val && ptr->right!=NULL) return search(val, ptr->right);
  return 0;
}


int main(int argc, char **argv)
{
	fbt_init(NULL);
	insert(15, root);
	fbt_start_transaction(&fbt_commit_transaction);
	insert(10, root);
	insert(20, root);
	insert(12, root);
	insert(8, root);
	fbt_commit_transaction();
	printf("Search 8: %d\n", search(8, root));
	printf("Search 10: %d\n", search(10, root));
	printf("Search 11: %d\n", search(11, root));
	printf("\n");
	return 0;
}
