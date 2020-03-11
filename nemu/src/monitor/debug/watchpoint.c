#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
bool is_init = false;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
	wp_pool[i].val = 0;
	wp_pool[i].is_on = false;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *str, int val) {
  WP *wp;
  if (is_init == false) {
    init_wp_pool();
	is_init = true;
	printf("init ok\n");
  }
  if (free_ == NULL) {
    printf("no free wp/n");
	assert(0);
  }
  else {
	// int i;
	printf("set wp:\n");
	wp = free_;
    free_ = free_->next;
	wp->next = NULL;
	printf("expr: %s\n", str);
	strcpy(wp->expr, str);
	// for (i=0; str[i]!='\0'; i++)
	  // wp->expr[i] = str[i];
	// wp->expr[i] = '\0';
	printf("iexpr: %s", wp->expr);
	wp->val = val;
	printf("val: %d", wp->val);
	wp->is_on = true;
	if (head==NULL) {
	  head = wp;
	  printf("head: %d\n", head->val);
	}
	else {
	  wp->next = head;
	  head = wp;
	  printf("head: %d\n", head->val);
	}
	printf("wp set ok\n");
	return wp;
  }
}

void free_wp(int no) {
  WP *wp;
  WP *curr;
  if (head == NULL) {
    printf("no wp found 0/n");
	assert(0);
  }
  else {
	if (head->NO == no) {
	  wp = head;
	  wp->val = 0;
	  head = head->next;
	  wp->next = free_;
	  free_ = wp;
	  printf("wp delete ok\n");
	}
	else {
      for (curr=head; curr!=NULL; curr=curr->next) {
        if (curr->next!=NULL) {
		  if (curr->next->NO == no) {
		    wp = curr->next;
		    wp->val = 0;
		    curr->next = curr->next->next;
		    wp->next = free_;
		    free_ = wp;
			printf("wp delete ok\n");
			return ;
	      }
		}
		else {
		  printf("error: no wp found\n");
		  assert(0);
		}
	  }
    }
  }
}

void print_wp() {
  WP *wp;
  if (head==NULL) {
    printf("no wp\n");
  }
  else {
    for (wp = head; wp!=NULL; wp=wp->next)
      printf("%d	%s	0x%08x\n", wp->NO, wp->expr, wp->val);
  }
}
