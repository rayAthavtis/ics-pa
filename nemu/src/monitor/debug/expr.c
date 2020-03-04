#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_LP, TK_RP, TK_NUM

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", TK_LP},
  {"\\)", TK_RP},
  {"=", '='},
  {"[1-9][0-9]*|0", TK_NUM}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  // int val1, val2;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
		tokens[i].type = rules[i].token_type;
		printf("type: %d\n", tokens[i].type);
        switch (tokens[i].type) {
		  case TK_NUM:
		  {
			strncpy(tokens[i].str, substr_start, substr_len);
			printf("num: %s\n", tokens[i].str);
		    break;
		  }
		  case '+':
		  {
			printf("+\n");
		    break;
		  }
		  case '-':
		  {
			printf("-\n");
			break;
		  }
		  case '*':
		  {
			printf("*\n");
			break;
		  }
		  case '/':
		  {
			printf("/\n");
			break;
		  }
		  default: 
			printf("token type: %d\n", rules[i].token_type);
        }
		nr_token++;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static bool ck_prt(int lp, int rp) {
  if (tokens[lp].type == TK_LP && tokens[rp].type == TK_RP) {
    printf("a pair of prt\n");
	return true;
  }
  return false;
}

static int search_dmtop(int tk_sta, int tk_end) {
  int i;
  int fl=0;
  int op=0;
  for (i=tk_sta; i<tk_end; i++) {
    if (tokens[i].type != TK_NUM) {
	  if (tokens[i].type == TK_RP)
	    fl = 0;
	  else if (tokens[i].type == TK_LP)
	    fl = 1;
	  if (fl == 1 || ((tokens[op].type == '+' || tokens[op].type == '-')
		 && (tokens[i].type == '*' || tokens[i].type == '/')))
		continue;
	  else op = i;
	}
  }
  printf("search op: %c\n", tokens[op].type);
  return op;
}

static int make_prase(int tk_sta, int tk_end) {
  int op;
  int val1, val2;
  if (tk_sta > tk_end) {
	printf("make_prase wrong in eval\n");
    return 0;
  }
  else if (tk_sta == tk_end) {
	printf("pos: %d\n", tk_sta);
	printf("type: %d, value: %s\n", tokens[tk_sta].type, tokens[tk_sta].str);
  	sscanf(tokens[tk_sta].str, "%d", &val1);
	printf("val: %d\n", val1);
	return val1;
  }
  else if (ck_prt(tk_sta, tk_end) == true) {
    return make_prase(tk_sta+1, tk_end-1);
  }
  else {
	op = search_dmtop(tk_sta, tk_end);
	printf("op_pos: %d\n", op);
	val1 = make_prase(tk_sta, op - 1);
	val2 = make_prase(op + 1, tk_end);
	switch (tokens[op].type) {
	  case '+':
		return val1 + val2;
	  case '-':
		return val1 - val2;
	  case '*':
		return val1 * val2;
	  case '/':
		return val1 / val2;
	  default:
	  {
		printf("defalt end\n");
		return 0;
	  }
	}
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    printf("make_token error\n");
	*success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // TODO();

  // printf("end: %d", nr_token);
  int res = make_prase(0, nr_token-1);
  printf("result: %d\n", res);
  
  return 0;
}
