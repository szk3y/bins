#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 0x100

typedef enum {
  VALUE,
  STRING,
  PLUS,
  MINUS,
  MULTI,
  DIVI,
  END,
  SENTINEL,
} Kind;

typedef struct {
  Kind kind;
  const char* str;
  int value;
} Token;

typedef struct {
  Token data[STACK_SIZE];
  int top;
} Stack;

Stack values;
Stack operators;
int top = 0;

void copy_token(Token* dst, const Token* src)
{
  memcpy(dst, src, sizeof(Token));
}

int empty(const Stack* stack)
{
  return stack->top == 0;
}

void pop(Stack* stack, Token* res)
{
  stack->top--;
  if(stack->top < 0) {
    fputs("Stack is empty\n", stderr);
    exit(1);
  }
  copy_token(res, &stack->data[stack->top]);
}

void push(Stack* stack, const Token* token)
{
  copy_token(&stack->data[stack->top], token);
  stack->top++;
}

int is_value(const char* str)
{
  size_t i = 0;
  if(str[i] == '-') {
    i++;
  }
  for(i = 0; i < strlen(str); i++) {
    if(!isdigit(str[i])) {
      return 0;
    }
  }
  return 1;
}

int order(const Token* token)
{
  switch(token->kind) {
    case MULTI:
    case DIVI:
      return 7;
    case PLUS:
    case MINUS:
      return 6;
    case SENTINEL:
      return -1;
    default:
      return 0;
  }
}

void operate(const Token* token)
{
  Token operand1, operand2, result;
  switch(token->kind) {
    case PLUS:
      pop(&values, &operand2);
      pop(&values, &operand1);
      // TODO: check that both operands are truly values
      result.kind = VALUE;
      result.value = operand1.value + operand2.value;
      push(&values, &result);
      return;
    case MINUS:
      pop(&values, &operand2);
      pop(&values, &operand1);
      result.kind = VALUE;
      result.value = operand1.value - operand2.value;
      push(&values, &result);
      return;
    case MULTI:
      pop(&values, &operand2);
      pop(&values, &operand1);
      result.kind = VALUE;
      result.value = operand1.value * operand2.value;
      push(&values, &result);
      return;
    case DIVI:
      pop(&values, &operand2);
      pop(&values, &operand1);
      result.kind = VALUE;
      result.value = operand1.value / operand2.value;
      push(&values, &result);
      return;
    case SENTINEL:
      return;
    default:
      fprintf(stderr, "operate: not implemented: %d\n", token->kind);
      exit(1);
  }
}

void eval(const Token* token)
{
  Token top;
  switch(token->kind) {
    case VALUE:
    case STRING:
      push(&values, token);
      return;
    case PLUS:
    case MINUS:
    case MULTI:
    case DIVI:
      pop(&operators, &top);
      if(order(token) <= order(&top)) {
        operate(&top);
      } else {
        push(&operators, &top);
      }
      push(&operators, token);
      break;
    case END:
      while(!empty(&operators)) {
        pop(&operators, &top);
        operate(&top);
      }
      break;
    default:
      fprintf(stderr, "eval: unknown token found: %d\n", token->kind);
      exit(1);
  }
}

void parse(const char* str)
{
  Token token;
  token.str = str;
  token.value = 0;
  if(str == NULL) {
    token.kind = END;
  } else if(is_value(str)) {
    token.kind = VALUE;
    token.value = atoi(str);
  } else if(strcmp(str, "+") == 0) {
    token.kind = PLUS;
  } else if(strcmp(str, "-") == 0) {
    token.kind = MINUS;
  } else if(strcmp(str, "*") == 0) {
    token.kind = MULTI;
  } else if(strcmp(str, "/") == 0) {
    token.kind = DIVI;
  } else {
    token.kind = STRING;
  }
  eval(&token);
}

void init_operators()
{
  Token sentinel;
  sentinel.kind = SENTINEL;
  push(&operators, &sentinel);
}

int main(int argc, char** argv)
{
  if(argc <= 1) {
    fputs("expr: Missing operand\n", stderr);
    exit(1);
  }
  init_operators();
  for(int i = 1; i <= argc; i++) {
    parse(argv[i]);
  }
  if(values.top != 1) {
    fputs("Syntax error\n", stderr);
    fputs("Note: if you want to use an asterisk, you should escape it.\n",
        stderr);
    exit(1);
  }
  switch(values.data[0].kind) {
    case VALUE:
      printf("%d\n", values.data[0].value);
      break;
    case STRING:
      puts(values.data[0].str);
      break;
    default:
      fprintf(stderr, "Unknown kind of token: %d\n", values.data[0].kind);
      exit(1);
  }
  return 0;
}
