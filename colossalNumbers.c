/*
    Author: Karol Synowiec
    The program implements basic operations on colossal numbers. It has a memory stack.
    Operations included:
    1) '0', '1': Read colossal number as a text and push it onto the memory stack 
    2) '.': pop number from the memory stack and print it
    3) ':': pop number from the memory stack and push it twice onto the memory stack
    4) '^': pop number from the memory stack and push its power of two onto the memory stack
    5) '+': pop two numbers from the memory stack and push their sum onto the memory stack
    6) '*': pop two numbers from the memory stack and push their product onto the memory stack
*/
#include <stdio.h>
#include <stdlib.h>
//Implementation of colossal number structure described in the project
struct colossal{
    struct colossal *firstDigit;
    struct colossal *nextDigit;
};
typedef struct colossal Tcolossal;
//Implementation of stack with elements being colossal numbers 
struct stack{
    Tcolossal *colossal;
    struct stack *next;
};
typedef struct stack Tstack;
//Adds a collosal number on top of a stack
void push(Tstack **S, Tcolossal *c){
    Tstack *pom=malloc(sizeof(Tstack));
    pom->next=*S;
    pom->colossal=c;
    *S=pom;
}
//Removes a collosal number on top of a stack
Tcolossal* pop(Tstack **S){
    if(*S==NULL) return NULL;
    Tstack *pom=*S;
    Tcolossal *c=(*S)->colossal;
    *S=(*S)->next;
    free(pom);
    return c;
}
//converts string representation of a colossal number to a colossal number represented with Tcolossal structure 
Tcolossal* stringToColossal(char **s){
    if(**s=='0'){
        (*s)++;
        return NULL;
    }

    if(**s=='1'){
        (*s)++;
        Tcolossal *newDigit=malloc(sizeof(Tcolossal));
        newDigit->firstDigit=stringToColossal(s);
        newDigit->nextDigit=stringToColossal(s);
        return newDigit;
    }

    return NULL;
}
//prints a colossal number as a text
void printColossal(Tcolossal *c){
    if(c==NULL){
        printf("%c", '0');
        return;
    }
    
    printf("%c", '1');
    printColossal(c->firstDigit);
    printColossal(c->nextDigit);
}
//reads colossal number from input as a string 
char *readColossal(char **s, int temp){
    int i=0;
    while(temp!='\n' && temp!=EOF){
        *s=realloc(*s, sizeof(char)*(long unsigned int)(i+1));
        (*s)[i]=(char)temp;
        i++;
        temp=getchar();
    }

    *s=realloc(*s, sizeof(char)*(long unsigned int)(i+1));
    (*s)[i]='\0';
    return *s;
}
//frees memory allocated for a Tcolossal
void freeColossal(Tcolossal *c){
    if(c==NULL) return;
    
    freeColossal(c->firstDigit);
    freeColossal(c->nextDigit);
    free(c);
}
//compares two normalized colossal numbers, result of comparison: 1: a > b, 0: a == b, -1: a < b
int compareColossal(Tcolossal *a, Tcolossal *b){
    if(a==b) return 0;
    if(a==NULL) return -1;
    if(b==NULL) return 1;

    int res=compareColossal(a->firstDigit, b->firstDigit);
    if(res!=0) return res;

    return compareColossal(a->nextDigit, b->nextDigit);
}
//copies colossal number
Tcolossal* copyColossal(Tcolossal *c){
    if(c==NULL) return NULL;

    Tcolossal *copy=malloc(sizeof(Tcolossal));
    copy->firstDigit=copyColossal(c->firstDigit);
    copy->nextDigit=copyColossal(c->nextDigit);
    return copy;
}
//adds a power of two with colossal exponent to a normalized colossal number
Tcolossal* addPowerOfTwo(Tcolossal *number, Tcolossal *exponent){
    if(number==NULL){
        Tcolossal *newDigit=malloc(sizeof(Tcolossal));
        newDigit->firstDigit=exponent;
        newDigit->nextDigit=NULL;
        return newDigit;
    }

    int temp=compareColossal(number->firstDigit, exponent);
    if(temp==1){ 
        //exponent is smaller than first digit so we need to go further in the list of this colossal
        number->nextDigit=addPowerOfTwo(number->nextDigit, exponent);
        //normalization if after addition there are duplicates of exponents
        if(number->nextDigit!=NULL && compareColossal(number->firstDigit, number->nextDigit->firstDigit)==0){
            Tcolossal *nextNext=number->nextDigit->nextDigit;
            Tcolossal *duplicateExp=number->nextDigit->firstDigit;
            Tcolossal *biggerExp=addPowerOfTwo(copyColossal(duplicateExp), NULL);
            //freeing memory
            freeColossal(number->firstDigit);
            freeColossal(duplicateExp); 
            free(number->nextDigit);    
            free(number);               
            
            return addPowerOfTwo(nextNext, biggerExp);
        }
        return number;
    } 
    else if(temp==-1){ 
        //exponent is bigger than first digit so we add it in front of the number
        Tcolossal *newDigit=malloc(sizeof(Tcolossal));
        newDigit->firstDigit=exponent;
        newDigit->nextDigit=number;
        return newDigit;
    } 
    else{
        //exponents are equal so we add them (2^x+2^x=2^(x+1)) and add this increased power of two to the remaining number
        Tcolossal *tail = number->nextDigit;
        Tcolossal *biggerExp = addPowerOfTwo(copyColossal(exponent), NULL);

        freeColossal(number->firstDigit);
        freeColossal(exponent);
        free(number);
        
        return addPowerOfTwo(tail, biggerExp);
    }
}
//normalizes colossal number
Tcolossal* normalizeColossal(Tcolossal *c){
    if (c==NULL) return NULL;

    Tcolossal *res=NULL;
    Tcolossal *cur=c;
    while (cur!=NULL) {
        Tcolossal *next=cur->nextDigit;
        Tcolossal *normalizedExp=normalizeColossal(cur->firstDigit);
        res=addPowerOfTwo(res, normalizedExp);
        free(cur);
        cur=next;
    }

    return res;
}
//creates a colossal number being a power of 2 of a given colossal number
Tcolossal* powerOfTwo(Tcolossal *c){
    Tcolossal *power=malloc(sizeof(Tcolossal));
    power->firstDigit=c;
    power->nextDigit=NULL;
    return power;
}
//performs addition of 2 colossal numbers
Tcolossal* addColossal(Tcolossal *a, Tcolossal *b){
    if (b==NULL) return a;
    if (a==NULL) return b;

    Tcolossal *currentB=b;
    //both numbers are sum of powers of two, so we can iteratively add powers of two included in b to a 
    while(currentB!=NULL){
        Tcolossal *nextB=currentB->nextDigit;
        Tcolossal *exponentToAdd=currentB->firstDigit;
        a=addPowerOfTwo(a, exponentToAdd);
        free(currentB);
        currentB=nextB;
    }
    return a;
}
//performs multiplication of 2 colossal numbers
Tcolossal* multiplyColossal(Tcolossal *a, Tcolossal *b){
    if (a==NULL || b==NULL) return NULL;
    Tcolossal *res=NULL;

    for (Tcolossal *curA=a; curA!=NULL; curA=curA->nextDigit) {
        for (Tcolossal *curB=b; curB!=NULL; curB=curB->nextDigit) {
            Tcolossal *sumExp=addColossal(copyColossal(curA->firstDigit), copyColossal(curB->firstDigit));
            Tcolossal *term=powerOfTwo(sumExp);
            res = addColossal(res, term);
        }
    }
    return res;
}
//initializes the memory stack, reads input and performs operations
int main(){
    Tstack *memoryStack = NULL;
    int temp;

    while((temp=getchar())!=EOF){
        if(temp=='.'){
                Tcolossal *c=pop(&memoryStack);
                printColossal(c);
                printf("\n");
                freeColossal(c);
        } 
        else if(temp==':'){
                Tcolossal* c=pop(&memoryStack);
                push(&memoryStack, copyColossal(c));
                push(&memoryStack, c);
        } 
        else if(temp=='^'){
                Tcolossal* c=pop(&memoryStack);
                push(&memoryStack, powerOfTwo(c));
        } 
        else if(temp=='+'){
                Tcolossal* b=pop(&memoryStack);
                Tcolossal* a=pop(&memoryStack);
                push(&memoryStack, addColossal(a, b));
        } 
        else if(temp == '*'){
                Tcolossal* a=pop(&memoryStack);
                Tcolossal* b=pop(&memoryStack);
                Tcolossal* res=multiplyColossal(a, b);
                push(&memoryStack, res);
                freeColossal(a);
                freeColossal(b);
        } 
        else if(temp=='0' || temp=='1'){
            char *s=NULL;
            s=readColossal(&s, temp);
            char *pom=s;
            Tcolossal *colossalNumber=stringToColossal(&pom);
            Tcolossal *normalized=normalizeColossal(colossalNumber);
            push(&memoryStack, normalized);
            free(s);
        }
    }
    //memory stack clearing
    while(memoryStack){
        freeColossal(pop(&memoryStack));
    }

    return 0;
}