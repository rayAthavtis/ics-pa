#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_GE, TK_LE, TK_NEQ, TK_AND, TK_OR, TK_LP, TK_RP, TK_NUM, TK_HEX, TK_REG, TK_NG, TK_DEREF

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
  
  {">=", TK_GE},
  {"<=", TK_LE},
  {"!=", TK_NEQ},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {">", '>'},
  {"<", '<'},
  {"!", '!'},

  {"-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", TK_LP},
  {"\\)", TK_RP},
  {"=", '='},
  {"0x[1-9A-Fa-f][0-9A-Fa-f]*", TK_HEX},
  {"[1-9][0-9]*|0", TK_NUM},
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)", TK_REG}
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
		if (substr_len>32)
		  assert(0);
		if (rules[i].token_type==TK_NOTYPE)
		  break;
		tokens[nr_token].type = rules[i].token_type;
		// printf("type: %d\n", tokens[nr_token].type);
        switch (tokens[nr_token].type) {
		  case TK_NUM:
		    strncpy(tokens[nr_token].str, substr_start, substr_len);
		    *(tokens[nr_token].str+substr_len)='\0';
		    // printf("i: %d, num: %s\n", nr_token, tokens[nr_token].str);
			break;
		  case TK_HEX:
		    strncpy(tokens[nr_token].str, substr_start + 2, substr_len - 2);
		    *(tokens[nr_token].str+substr_len-2)='\0';
		    break;
		  case TK_REG:
		    strncpy(tokens[nr_token].str, substr_start + 1, substr_len -1);
		    *(tokens[nr_token].str+substr_len-1)='\0';
			break;
		  case '-':
			if (nr_token==0)
			  tokens[0].type = TK_NG;
			else if (tokens[nr_token-1].type != TK_NUM && tokens[nr_token-1].type != TK_HEX && tokens[nr_token-1].type != TK_REG && tokens[nr_token-1].type != TK_RP)
			  tokens[nr_token].type = TK_NG;
			break;
		  case '*':
			if (nr_token==0)
			  tokens[0].type = TK_DEREF;
			else if (tokens[nr_token-1].type != TK_NUM && tokens[nr_token-1].type != TK_HEX && tokens[nr_token-1].type != TK_REG && tokens[nr_token-1].type != TK_RP)
			  tokens[nr_token].type = TK_DEREF;
			break;
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
  int i;
  int ct=0;
  if (tokens[lp].type != TK_LP || tokens[rp].type != TK_RP)
	return false;
  for (i=lp; i<rp; i++) {
    if (tokens[i].type == TK_LP)
	  ct++;
	else if(tokens[i].type == TK_RP)
	  ct--;
	if (ct<1)
	  return false;
  }
  if (ct>0) {
	printf("extra '(' in expr\n");
	assert(0);
  }
  else if (ct<0) {
    printf("extra ')' in expr\n");
	assert(0);
  }
  return true;
}

static int search_dmtop(int tk_sta, int tk_end) {
  int i;
  int fl=0;
  int op[8]={-1,-1,-1,-1,-1, -1};
  for (i=tk_sta; i<tk_end; i++) {
    if (tokens[i].type != TK_NUM) {
	  if (tokens[i].type == TK_RP)
	    fl--;
	  else if (tokens[i].type == TK_LP)
	    fl++;
	  if (fl == 0) {
		switch (tokens[i].type) {
		  case '!':
		  case TK_NG:
		  case TK_DEREF:
			op[0]=i;
			break;
		  case '*':
		  case '/':
			op[1]=i;
			break;
		  case '+':
		  case '-':
			op[2]=i;
			break;
		  case '>':
		  case '<':
		  case TK_GE:
		  case TK_LE:
			op[3]=i;
			break;
		  case TK_EQ:
		  case TK_NEQ:
			op[4]=i;
			break;
		  case TK_AND:
			op[5]=i;
			break;
		  case TK_OR:
			op[6]=i;
			break;
		  case '=':
			op[7]=i;
			break;
		}
	  }
	}
  }
  for (i=7; i>=0; i--) {
    if (op[i]!=-1)
	  return op[i];
  }
  printf("op error in search_domtop\n");
  assert(0);
  // printf("search op: %c\n", tokens[op].type);
}

static int make_prase(int tk_sta, int tk_end) {
  int op;
  int val1, val2;
  vaddr_t addr;
  if (tk_sta > tk_end) {
	printf("make_prase error in eval\n");
    assert(0);
	// return 0;
  }
  else if (tk_sta == tk_end) {
	// printf("type: %d, value: %s\n", tokens[tk_sta].type, tokens[tk_sta].str);
  	int i;
	switch (tokens[tk_sta].type) {
	  case TK_NUM:
		sscanf(tokens[tk_sta].str, "%d", &val1);
		break;
	  case TK_HEX:
		sscanf(tokens[tk_sta].str, "%x", &val1);
	    break;
	  case TK_REG:
		for (i=0; i<8; i++) {
		  if (strcmp(tokens[tk_sta].str, regsl[i])==0)
			return reg_l(i);
		  else if (strcmp(tokens[tk_sta].str, regsw[i])==0)
			return reg_w(i);
		  else if (strcmp(tokens[tk_sta].str, regsb[i])==0)
			return reg_b(i);
		}
		if (strcmp(tokens[tk_sta].str, "eip")==0)
		  return cpu.eip;
		else
		  printf("incorrect reg\n");
		assert(0);
	}
	// printf("val: %d\n", val1);
	return val1;
  }
  else if (ck_prt(tk_sta, tk_end) == true) {
    return make_prase(tk_sta+1, tk_end-1);
  }
  else {
	op = search_dmtop(tk_sta, tk_end);
	printf("op_pos: %d\n", op);
	if (tokens[op].type == '!') {
	  val1 = make_prase(tk_sta + 1, tk_end);
	  if (val1==0)
		return 1;
	  else
		return 0;
	}
	else if (tokens[op].type == TK_NG)
	  return -make_prase(tk_sta + 1, tk_end);
	else if (tokens[op].type == TK_DEREF) {
	  addr = make_prase(tk_sta + 1, tk_end);
	  val1 = vaddr_read(addr, 4);
	  printf("addr=%u(0x%x), value=%d(0x%08x)\n", addr, addr, val1, val1);
	  return val1;
	} 
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
	  case '=':
		return val1=val2;
	  case '>':
		return val1>val2;
	  case '<':
		return val1<val2;
	  case TK_GE:
		return val1>=val2;
	  case TK_LE:
		return val1<=val2;
	  case TK_EQ:
		return val1==val2;
	  case TK_NEQ:
		return val1!=val2;
	  case TK_AND:
		return val1&&val2;
	  case TK_OR:
		return val1||val2;
	  default:
	  {
		printf("error yet\n");
		assert(0);
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

  printf("token num: %d\n", nr_token);
  int res = make_prase(0, nr_token-1);
  printf("result: %d\n", res);
  
  return 0;
}
