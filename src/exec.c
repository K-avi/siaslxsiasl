#include "exec.h"
#include "ast.h"
#include "cells.h"
#include "stack.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int default_mult_div=2;

int exec_prgm( instruction* program, CELLMATRIX* environment, S_STACK* stack) {

  if(! (program && environment && stack)) return -1;
  instruction* curr = program;

  unsigned short stack_ptr = 0;
  
 // int length= environment->size;
  int idx= environment->curindex; 
 
  
  char safe_getchar[256]; //used in read function to pass stuff safely
 
  uint32_t matsize= environment->size* environment->size;
  while (curr) {
    
    unsigned short instruction = curr->symbol.token1 | ((curr->symbol.token2)<<4);
  //printf("%u %u %b\n",curr->symbol.token1 , curr->symbol.token2 ,instruction );
    switch (instruction) {

      /* default operations; the one that already are in SIASL */
        case (INT_NEUT<<4) | INT_NEUT:  break; //neutral op 

        case (INT_LEFT<<4)| INT_NEUT: idx=OP_LEFT(idx, matsize);  break;

        case (INT_RIGHT<<4)| INT_NEUT: idx=OP_RIGHT(idx , matsize); break;

        case (INT_UP<<4)| INT_NEUT: idx=OP_UP(idx, matsize); break;

        case (INT_DOWN<<4) | INT_NEUT : idx= OP_DOWN(idx, matsize); break;

        case (INT_PLUS<<4) | INT_NEUT: environment->mat[idx]++; break;

        case(INT_MINUS <<4) | INT_NEUT: environment->mat[idx]--; break;

        case (INT_MULT <<4) | INT_NEUT :environment->mat[idx]*=default_mult_div; break;

        case (INT_DIV <<4) | INT_NEUT: environment->mat[idx]/=default_mult_div; break;

        case (INT_READ<<4 )| INT_NEUT : 
            fflush(stdin);
            if(fgets(safe_getchar, 255, stdin)){
              
            environment->mat[idx]= safe_getchar[0];
            fflush(stdin);

            memset(safe_getchar, 0, 256*sizeof(char));
            }
            break;

        case (INT_PRINT<<4) | INT_NEUT: printf("%c", environment->mat[idx]); break;

        case (INT_LBRACKET <<4 ) | INT_NEUT :
            if (environment->mat[idx] == 0) {
              curr = curr->other;
            } else {
              if (stack_ptr >= STACK_SIZE) {
                  return -1; //stack overflow
              }
              stack->stack[stack_ptr++] = curr;
            }
            break;
        case (INT_RBRACKET <<4) | INT_NEUT :
            curr = stack->stack[--stack_ptr];
            continue;
      
      /*new predefined operations */

      /* movement operations */

      case (INT_DOWN<<4)| INT_DOWN: idx=0;  break; //goes to first cell 
      case (INT_UP<<4)| INT_UP : idx=(environment->size*environment->size)-1;  break; //goes to last cell 

      case (INT_LEFT<<4)| INT_LEFT :  break; //goes to first cell in line
      case (INT_RIGHT<<4)| INT_RIGHT :  break; //goes to last cell in line 

      /* arithmetic primites ; stores in the current cell the result of :
      curvalue= curvalue (airthmetic op) (movement op)->value

      for exemple: 

      +< stores in the current cell the value of mat[idx]+ mat[OP_RIGHT(idx, matsize)]
      v* stores in the current cell the value of mat[idx] * mat[OP_DOWN(idx, matsize)]
      */
      case INT_RIGHT | (INT_PLUS<<4) : environment->mat[idx]+=environment->mat[OP_RIGHT(idx, matsize)]; break;
      case INT_LEFT | (INT_PLUS<<4) : environment->mat[idx]+=environment->mat[OP_LEFT(idx, matsize)]; break;
      case INT_UP | (INT_PLUS<<4) : environment->mat[idx]+=environment->mat[OP_UP(idx, matsize)]; break;
      case INT_DOWN | (INT_PLUS<<4) : environment->mat[idx]+=environment->mat[OP_DOWN(idx, matsize)]; break;


      case INT_RIGHT | (INT_MINUS<<4) : environment->mat[idx]-=environment->mat[OP_RIGHT(idx, matsize)]; break;
      case INT_LEFT | (INT_MINUS<<4) : environment->mat[idx]-=environment->mat[OP_LEFT(idx, matsize)]; break;
      case INT_UP | (INT_MINUS<<4) : environment->mat[idx]-=environment->mat[OP_UP(idx, matsize)]; break;
      case INT_DOWN | (INT_MINUS<<4) : environment->mat[idx]-=environment->mat[OP_DOWN(idx, matsize)]; break;

      case INT_RIGHT | (INT_MULT<<4) : environment->mat[idx]*=environment->mat[OP_RIGHT(idx, matsize)]; break;
      case INT_LEFT | (INT_MULT<<4) : environment->mat[idx]*=environment->mat[OP_LEFT(idx, matsize)]; break;
      case INT_UP | (INT_MULT<<4) : environment->mat[idx]*=environment->mat[OP_UP(idx, matsize)]; break;
      case INT_DOWN | (INT_MULT<<4) : environment->mat[idx]*=environment->mat[OP_DOWN(idx, matsize)]; break;

      case INT_RIGHT | (INT_DIV<<4) : if(environment->mat[OP_RIGHT(idx,matsize)] ) environment->mat[idx]/=environment->mat[OP_RIGHT(idx, matsize)]; break;
      case INT_LEFT | (INT_DIV<<4) : if(environment->mat[OP_LEFT(idx,matsize)] ) environment->mat[idx]/=environment->mat[OP_LEFT(idx, matsize)]; break;
      case INT_UP | (INT_DIV<<4) : if(environment->mat[OP_UP(idx,matsize)] ) environment->mat[idx]/=environment->mat[OP_UP(idx, matsize)]; break;
      case INT_DOWN | (INT_DIV<<4) : if(environment->mat[OP_DOWN(idx,matsize)] ) environment->mat[idx]/=environment->mat[OP_DOWN(idx, matsize)]; break;

      /* arithmetic stuff */

      case (INT_MULT <<4) | INT_MULT :environment->mat[idx]*=environment->mat[idx]; break; //square operation 
      case (INT_DIV <<4) | INT_DIV : environment->mat[idx]= 1; break; //square operation 
      case (INT_PLUS <<4) | INT_PLUS : environment->mat[idx]*=2; break; //square operation 
      case (INT_MINUS <<4) | INT_MINUS: environment->mat[idx]=0; break; //square operation 


      /* print formats */

      case (INT_PRINT<<4) | INT_PLUS: printf("%d", environment->mat[idx]); break;
      case (INT_PRINT<<4) | INT_MINUS: printf("%x", environment->mat[idx]); break;
      case (INT_PRINT<<4) | INT_MULT: printf("%o", environment->mat[idx]); break;
      case (INT_PRINT<<4) | INT_DIV: printf("%u", (unsigned) environment->mat[idx]); break;
      case INT_PRINT | (INT_WILDCARD <<4)  : printf("%.2f", (float) environment->mat[idx]); break;

      /* loop variations */

      /*arithmetic modes */
      case (INT_LBRACKET <<4 ) | INT_PLUS : //increments curcell each time going on loop 
            if (environment->mat[idx] == 0) {
              curr = curr->other;
            } else {
              environment->mat[idx]++;
              if (stack_ptr >= STACK_SIZE) {
                  return -1; //stack overflow
              }
              stack->stack[stack_ptr++] = curr;
            }
            
            break;
        case (INT_RBRACKET <<4) | INT_PLUS : //increments curcell each time ending loop 
            curr = stack->stack[--stack_ptr];
            environment->mat[idx]++;
            continue;
        

        case (INT_LBRACKET <<4 ) | INT_MINUS : //decrement curcell each time going in loop 
            if (environment->mat[idx] == 0) {
              curr = curr->other;
            } else {
              environment->mat[idx]--;
              if (stack_ptr >= STACK_SIZE) {
                  return -1; //stack overflow
              }
              stack->stack[stack_ptr++] = curr;
            }
            
            break;
        case (INT_RBRACKET <<4) | INT_MINUS : //decrements curcell each time finishing loop 
            curr = stack->stack[--stack_ptr];
            environment->mat[idx]--;

            continue;
        /*movement modes */
        case (INT_LBRACKET <<4 ) | INT_LEFT : //goes to the left of curcell each time going in loop 
            if (environment->mat[idx] == 0) {
              curr = curr->other;
              idx=OP_LEFT(idx, matsize);
            } else {
          
              if (stack_ptr >= STACK_SIZE) {
                  return -1; //stack overflow
              }
              stack->stack[stack_ptr++] = curr;
              idx=OP_LEFT(idx, matsize);
            }
            
            break;
        case (INT_RBRACKET <<4) | INT_LEFT : //goes to the left of curcell each time ending loop
            curr = stack->stack[--stack_ptr];
          
            idx=OP_LEFT(idx, matsize);
            continue;
        
        case (INT_LBRACKET <<4 ) | INT_RIGHT: //goes to the right of curcell each time going on loop 
            if (environment->mat[idx] == 0) {
              curr = curr->other;
              idx=OP_RIGHT(idx, matsize);
            } else {
             
              if (stack_ptr >= STACK_SIZE) {
                  return -1; //stack overflow
              }
              stack->stack[stack_ptr++] = curr;
              idx=OP_RIGHT(idx, matsize);
            }
            
            break;
        case (INT_RBRACKET <<4) | INT_RIGHT ://goes to the right of curcell each time ending loop
            curr = stack->stack[--stack_ptr];
         
            idx=OP_RIGHT(idx, matsize);
            continue;
        
        case (INT_LBRACKET <<4 ) | INT_UP : //goes to up of curcell each time going on loop 
            if (environment->mat[idx] == 0) {
              curr = curr->other;
              idx=OP_UP(idx, matsize);
            } else {
         
              if (stack_ptr >= STACK_SIZE) {
                  return -1; //stack overflow
              }
              stack->stack[stack_ptr++] = curr;
              idx=OP_UP(idx, matsize);
            }
            
            break;
        case (INT_RBRACKET <<4) | INT_UP : //goes to up of curcell each time ending loop
            curr = stack->stack[--stack_ptr];
          
            idx=OP_UP(idx, matsize);
            continue;
        
        case (INT_LBRACKET <<4 ) | INT_DOWN : //goes down of curcell each time going on loop 
            if (environment->mat[idx] == 0) {
              curr = curr->other;
              idx=OP_DOWN(idx, matsize);
            } else {
             
              if (stack_ptr >= STACK_SIZE) {
                  return -1; //stack overflow
              }
              stack->stack[stack_ptr++] = curr;
              idx=OP_DOWN(idx, matsize);
            }
            
            break;
        case (INT_RBRACKET <<4) | INT_DOWN : //goes down of curcell each time ending loop
            curr = stack->stack[--stack_ptr];
    
            idx=OP_DOWN(idx, matsize);
            continue;
            
      /* silly wildcard stuff */

      case (INT_WILDCARD<<4) | INT_READ : 

            fflush(stdin);
            if(fgets(safe_getchar, 255, stdin)){
              
            default_mult_div= safe_getchar[0];
            fflush(stdin);

            memset(safe_getchar, 0, 256*sizeof(char));
            }
          
          break;

      /* not done yet */

        default: 
         break;
    }
    curr = curr->next;
  }
  environment->curindex=idx;
  return 0;
}
