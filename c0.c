#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "c0.h"
#include "journal.h"

c0_node *Insert(c0_node *T, char *key, char *value, int flag) {
  if (T == NULL) {
    // printf("null Insert\n" );
    T = (c0_node *) malloc(sizeof(c0_node));
    T->key = (char *) malloc(strlen(key) + 1);
    T->value = (char *) malloc(strlen(value) + 1);
    strcpy(T->key, key);
    strcpy(T->value, value);
    T->flag = flag;
    T->left = NULL;
    T->right = NULL;
    // printf("inserted %p\n", T);
  } else if (strcmp(key, T->key) > 0)        // insert in right subtree
  {
    //   printf("Higher\n" );
    T->right = Insert(T->right, key, value, flag);
    if (BF(T) == -2)
      if (strcmp(key, T->right->key) > 0)
        T = RR(T);
      else
        T = RL(T);
  } else if (strcmp(key, T->key) <= 0) {
    //   printf("Lower\n" );
    T->left = Insert(T->left, key, value, flag);
    if (BF(T) == 2)
      if (strcmp(key, T->left->key) <= 0)
        T = LL(T);
      else
        T = LR(T);
  }

  T->ht = height(T);

  return T;
}

c0_node *Delete(c0_node *T, char *key) {
  c0_node *p;

  if (T == NULL) {
    return NULL;
  } else if (strcmp(key, T->key) > 0) {
    T->right = Delete(T->right, key);
    if (BF(T) == 2)
      if (BF(T->left) >= 0)
        T = LL(T);
      else
        T = LR(T);
  } else if (strcmp(key, T->key) < 0) {
    T->left = Delete(T->left, key);
    if (BF(T) == -2)    //Rebalance during windup
      if (BF(T->right) <= 0)
        T = RR(T);
      else
        T = RL(T);
  } else {
    //data to be deleted is found
    if (T->right != NULL) {    //delete its inorder succesor
      p = T->right;

      while (p->left != NULL)
        p = p->left;

      T->key = p->key;
      T->value = p->value;
      T->right = Delete(T->right, p->key);

      if (BF(T) == 2)//Rebalance during windup
        if (BF(T->left) >= 0)
          T = LL(T);
        else
          T = LR(T);
        \

    } else
      return ( T->left );
  }
  T->ht = height(T);
  return ( T );
}

c0_node *Get(c0_node *T, char *key) {

  if (T == NULL) {
    return NULL;
  } else if (strcmp(key, T->key) == 0) {
    return T;
  } else if (strcmp(key, T->key) < 0) {
    Get(T->left, key);
  } else if (strcmp(key, T->key) > 0) {
    Get(T->right, key);
  }
  return NULL;
}

c0_node *Update(c0_node *T, char *key, char *value, int flag) {
  if (T == NULL) {
    return NULL;
  } else if (strcmp(key, T->key) == 0) {
    T->value = value;
    T->flag = flag;
    return T;
  } else if (strcmp(key, T->key) < 0) {
    Update(T->left, key, value, flag);
  } else if (strcmp(key, T->key) > 0) {
    Update(T->right, key, value, flag);
  }
  return NULL;
}

int height(c0_node *T) {
  int lh, rh;
  if (T == NULL)
    return ( 0 );

  if (T->left == NULL)
    lh = 0;
  else
    lh = 1 + T->left->ht;

  if (T->right == NULL)
    rh = 0;
  else
    rh = 1 + T->right->ht;

  if (lh > rh)
    return ( lh );

  return ( rh );
}

c0_node *rotateright(c0_node *x) {
  c0_node *y;
  y = x->left;
  x->left = y->right;
  y->right = x;
  x->ht = height(x);
  y->ht = height(y);
  return ( y );
}

c0_node *rotateleft(c0_node *x) {
  c0_node *y;
  y = x->right;
  x->right = y->left;
  y->left = x;
  x->ht = height(x);
  y->ht = height(y);

  return ( y );
}

c0_node *RR(c0_node *T) {
  T = rotateleft(T);
  return ( T );
}

c0_node *LL(c0_node *T) {
  T = rotateright(T);
  return ( T );
}

c0_node *LR(c0_node *T) {
  T->left = rotateleft(T->left);
  T = rotateright(T);

  return ( T );
}

c0_node *RL(c0_node *T) {
  T->right = rotateright(T->right);
  T = rotateleft(T);
  return ( T );
}

int BF(c0_node *T) {
  int lh, rh;
  if (T == NULL)
    return ( 0 );

  if (T->left == NULL)
    lh = 0;
  else
    lh = 1 + T->left->ht;

  if (T->right == NULL)
    rh = 0;
  else
    rh = 1 + T->right->ht;

  return ( lh - rh );
}

void preorder(c0_node *T) {
  if (T != NULL) {
    printf("(key:%s, value:%s)[Bf=%d]", T->key, T->value, BF(T));
    preorder(T->left);
    preorder(T->right);
  }
}

void inorder(c0_node *T) {
  if (T != NULL) {
    inorder(T->left);
    printf("(key:%s, value:%s)[Bf=%d]", T->key, T->value, BF(T));
    inorder(T->right);
  }
}

void c0_dump(c0_node *T) {
  int _size = c0_size(T);

  c0_node *nodes[_size];
  dumpToArray(T, nodes, 0);
  // for(int i=0; i<_size; i++)printf("Index:%d, Node: %p\n",i,nodes[i] );
  if (!c1_batch_insert(nodes, _size)) {
    flush_log();
  }
  return;
}

int dumpToArray(c0_node * T, c0_node *nodes[], int i){
    if(T == NULL) return i;
    //   nodes[i] = T;
    //   i++;
    //   if(T->left != NULL)
    i = dumpToArray(T->left, nodes, i);

    nodes[i] = T;
    // printf("node inserted at %d: %s\n",i, nodes[i]->key);//printf("node inserted: %s\n", nodes[i]->key);
    i++;

    //   if(T->right != NULL)
    i = dumpToArray(T->right, nodes, i);

    // return i++;
}
// void dumpToArray(c0_node *T, c0_node *nodes[], int i) {
//   if (T == NULL) return;
//   //   nodes[i] = T;
//   //   i++;
//   //   if(T->left != NULL)
//   dumpToArray(T->left, nodes, i);
//
//   nodes[i] = T;
//   printf("node inserted at %d: %s\n",i, nodes[i]->key);
//   i++;
//
//   //   if(T->right != NULL)
//   dumpToArray(T->right, nodes, i);
// }

void dumpToFile(c0_node *T, FILE *f) {

  if (T != NULL) {
    dumpToFile(T->left, f);
    char *str = (char *) malloc(strlen(T->key) + strlen(T->value) + 3);
    strcpy(str, T->key);
    // printf("KEY:%s, VALUE: %s\n", T->key, T->value);
    strcat(str, " ");
    strcat(str, T->value);
    strcat(str, "\n");
    printf("ATATATAT:%s\n", str);
    fwrite(str, sizeof(char), strlen(str),
           f);//;printf("(key:%s, value:%s)[Bf=%d]",T->key, T->value,BF(T));
    free(str);
    dumpToFile(T->right, f);
  }
}

int c0_size(c0_node *T) {
  // printf("Cacling Size: %p\n",T );
  if (T == NULL)
    return 0;
  else
    return ( c0_size(T->left) + 1 + c0_size(T->right));
}
