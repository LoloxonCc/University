/*
    Author: Karol Synowiec
    The program performs addition and multiplication of polynomials, the resulting polynomial must be of degree less then 11 and coefficients must be in range of int type. 
*/
#include <stdio.h>

#define maxDeg 10 //maximum degree of a polynomial in program 


//function to convert character '+' to integer 1 and character '-' to integer -1, in order to save coefficients of polynomial as postive or negative  
int sign(int c){
    int result;

    if(c=='+') result=1;
    else if(c=='-') result=-1;
    else result=0;

    return result; 
}

//function to read a polynomial from console, argument is the polynomial to be read
void read(int P[]){
    for(int i=0; i<=maxDeg; i++) P[i]=0; 
    int coefOrPow=1, s=1,  coef=0, pow=0, temp; /*coefOrpow tells whether we are reading coefficient or power of monomila, s tells whether coefficient is negative or positive,
                                                            coef holds current coefficient of monomial, pow holds current power of monomial, temp is the current character from input*/
    temp=getchar();

    while(temp!='\n'){ //read until we hit the new line 
        if(temp!=' '){ 
            if(temp=='+' || temp=='-'){ //in this case we are starting to read a new monomial
                P[pow]=coef; //update previously read monomial
                coef=0; //reset coefficient
                pow=0; //reset power 
                coefOrPow=1; //we are reading coefficient next 
                s=sign(temp); //the coefficient will be of this sign 
            }
            else if(temp=='x'){ //in this case we have read coefficient
                if(coef==0) coef=s; //if there was no coefficient update then it was 1 or -1 which is held by s
                else coef*=s; //in other case we have updated coefficient and just need to adjust its sign 
                coefOrPow=0; //we are starting to read power
                pow=1; //if there will be no power then it will me a monomial of degree 1 
            }
            else if(temp=='^'){ 
                pow=0; //the degree of monomial will be greater then 1 and later calculated
            }
            else{ //in this case we are reading a digit
                if(coefOrPow==1){ //in this case we are reading digits of a coefficient
                    coef*=10; //we are shifting the number so we can add next digit on its end 
                    coef+=temp-48; //conversion from char to digit
                }
                else{ //in this case we are reading power
                    pow*=10; //we are shifting the number so we can add next digit on its end 
                    pow+=temp-48; //conversion from char to digit
                }
            }
        }
        temp=getchar();  
    } 
    if(coef!=0 && pow>0) //on the end there will be no '+' or '-' so we need to update coefficient if something additional has been read
        P[pow]=coef; 
    else  //in this case we will update the coefficient in monomial of degree 0 and we did not have any x so we also need to update its sign
        P[pow]=s*coef;
  
}

//function to write a polynomial on console, argument is the polynomial to be written
void write(int P[]){
    int first=1; //tells us whether it is the first monomial on the output
    for(int i=maxDeg; i>=0; i--){
        if(!first){ /*if it is not first we need to format the output with spaces between '+' or '-' and the coefficients, 
                        formating below keeps in mind the situation when the coefficient is either 1 or -1 and does not need to be printed, 
                        when the monomial is of degree 1 and we do not need to print '^1'*/
            if(P[i]>1){ 
                if(i>1)
                    printf("%s%d%c%c%d", " + ", P[i], 'x', '^', i);
                else if(i==1)
                    printf("%s%d%c", " + ", P[i], 'x');
                else
                    printf("%s%d", " + ", P[i]);
            }
            else if(P[i]<-1){
                if(i>1)
                    printf("%s%d%c%c%d", " - ", -P[i], 'x', '^', i);
                else if(i==1)
                    printf("%s%d%c", " - ", -P[i], 'x');
                else 
                    printf("%s%d", " - ", -P[i]);
            }
            else if(P[i]==1){
                if(i>1)
                    printf("%s%c%c%d", " + ", 'x', '^', i);
                else if(i==1)
                    printf("%s%c", " + ", 'x');
                else
                    printf("%s%d", " + ", P[i]);
            }
            else if(P[i]==-1){
                if(i>1)
                    printf("%s%c%c%d", " - ", 'x', '^', i);
                else if(i==1)
                    printf("%s%c", " - ", 'x');
                else
                    printf("%s%d", " - ", -P[i]);
            }
        }
        else{ /*in this case we print first monomial so it has no space between '+' or '-' and coefficient 
                    formating below keeps in mind the situation when the coefficient is either 1 or -1 and does not need to be printed, 
                    when the monomial is of degree 1 and we do not need to print '^1'*/
            if(P[i]>1){
                if(i>1)
                    printf("%d%c%c%d", P[i], 'x', '^', i);
                else if(i==1)
                    printf("%d%c",  P[i], 'x');
                else
                    printf("%d", P[i]);
                first=0;
            }
            else if(P[i]<-1){
                if(i>1)
                    printf("%c%d%c%c%d", '-', -P[i], 'x', '^', i);
                else if(i==1)
                    printf("%c%d%c", '-',-P[i] , 'x');
                else
                    printf("%c%d", '-', -P[i]);
                first=0;
            }
            else if(P[i]==1){
                if(i>1)
                    printf("%c%c%d", 'x', '^', i);
                else if(i==1)
                    printf("%c",  'x');
                else
                    printf("%d",  P[i]);
                first=0;
            }
            else if(P[i]==-1){
                if(i>1)
                    printf("%c%c%c%d", '-', 'x', '^', i);
                else if(i==1)
                    printf("%c%c", '-', 'x');
                else
                    printf("%c%d", '-', -P[i]);
                first=0;
            }
            else if(P[i]==0 && i==0){
                printf("%d", P[i]);
                first=0;
            }
        }
    }
    printf("\n");
}

//function to perfom addition of two polynomials, argument is the 'accumulating' polynomial
void add(int P[]){
    int Q[maxDeg+1];
    read(Q);

    for(int i=0; i<=maxDeg; i++) P[i]+=Q[i];
}

//function to perfom multiplication of two polynomial, argument is the 'accumulating' polynomial
void multiply(int P[]){
    int Q[maxDeg+1], W[maxDeg+1]; //Q is the currently read polynomial, W is polynomial to keep the results of multiplication 
    read(Q);
    for(int i=0; i<=maxDeg; i++) W[i]=0;

    for(int i=0; i<=maxDeg; i++){
        for(int j=0; j<=maxDeg; j++){
            if(i+j<=maxDeg)
                W[i+j]+=P[i]*Q[j];
        }
    }
    
    for(int i=0; i<=maxDeg; i++) P[i]=W[i]; //P is our accumulator so we need to update it as a result 
}

//main function of the program which reads one polynomial at a time, perfoms given operation with the 'accumalated' polynomial and prints it 
int main(){
    int P[maxDeg+1], op; //polynomial which is accumulator, op stores current operation to be performed
    for(int i=0; i<=maxDeg; i++) P[i]=0; 
    op=getchar();

    while(op!='.') //we read the input until we hit '.'
    {
        if(op=='+'){ 
            add(P);
        }
        else{
            multiply(P);
        }
        write(P);
        op=getchar();
    } 

    return 0;
}
