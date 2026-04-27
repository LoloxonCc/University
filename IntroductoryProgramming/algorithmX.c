/*
    Author: Karol Synowiec
    The program finds all solutions to exact cover problem using Donald Knuth's Algorithm X approach. 
*/
#include <stdio.h>
#include <stdlib.h>

//boundaries on number of rows and columns given in the problem 
#define MAX_ROWS 200 
#define MAX_COLUMNS 300

//reads filter and its size
int readFandColSize(char* F){
    int colSize=0;
    char temp=(char)getchar();
    while(temp!='\n'){
        F[colSize]=temp;
        colSize++;
        temp=(char)getchar();
    }

    return colSize;
}
//turns read filter with MAX_COLUMNS columns to a filter with exact quantity of columns
void actualF(char* F, char* tempF, int colSize){
    for(int i=0; i<colSize; i++)
        F[i]=tempF[i];
}
//reads subsets for exact cover problem and their quantity
int readWandRowSize(char** W, int colSize){
    int rowSize=0;
    char temp=(char)getchar();
    while(temp!=EOF && temp!='\n'){
        for(int i=0; i<colSize; i++){
            W[rowSize][i]=temp;
            temp=(char)getchar();
        } 
        rowSize++;
        temp=(char)getchar();
    } 

    return rowSize;
}
//turns table of read subsets with MAX_ROWS length into one with exact quantity of rows
void actualW(char** W, char** tempW, int rowSize, int colSize){
    for(int i=0; i<rowSize; i++)
        for(int j=0; j<colSize; j++)
            W[i][j]=tempW[i][j];
}
//prints filtered solution to the exact cover problem 
void printFilteredSolution(char* solution, int colSize, char* F){
    for(int i=0; i<colSize; i++)//if the solution is incomplete then we do not print it
        if(solution[i]=='_')
            return;

    for(int i=0; i<colSize; i++)
        if(F[i]=='+')
            printf("%c", solution[i]);

    printf("\n");
}
//adds elements of subsets to solution
void partialSolution(char** W, int colSize, int row, char* solution){
    for(int i=0; i<colSize; i++)
        if(W[row][i]!='_')
            solution[i]=W[row][i];
}
//deletes row of a matrix of subsets by changing its signs into an empty '_' sign
void deleteRow(char** W, int colSize, int row){
    for(int i=0; i<colSize; i++)
        W[row][i]='_';
}
//deletes column of a matrix of subsets by changing its signs into an empty '_' sign
void deleteColumn(char** W, int rowSize, int col){
    for(int i=0; i<rowSize; i++)
        W[i][col]='_';
}
//copies matrix of subsets to a new one
char** copyW(char** W, int rowSize, int colSize){
    char **tempW=malloc(sizeof(char*)*((long unsigned int)rowSize));
    for(int i=0; i<rowSize; i++){
        tempW[i]=malloc(sizeof(char)*((long unsigned int)colSize));
    }
    actualW(tempW, W, rowSize, colSize);

    return tempW;
}
//copies solution to a new one 
char* copySolution(char* solution, int colSize){
    char* tempSolution=malloc(sizeof(char)*((long unsigned int)colSize));
    for(int i=0; i<colSize; i++)
        tempSolution[i]=solution[i];

    return tempSolution;
}
//frees matrix
void freeW(char **W, int rowSize){
    for(int i=0; i<rowSize; i++)
        free(W[i]);
    free(W);
}
//finds column of matrix which is not empty, if there is no such left returns -1
int findColumn(char ** W, int rowSize, int colSize, int col){
    for(int i=col; i<colSize; i++)
        for(int j=0; j<rowSize; j++)
            if(W[j][i]!='_')
                return i;

    return -1;
}
/*implements Donald Knuth's Algorithm X:
    1. If the matrix W has no columns, the current partial solution is a valid solution; terminate successfully.
    2. Otherwise choose a column c (deterministically).
    3. Choose a row r such that Wr, c != '_' (nondeterministically).
    4. Include row r in the partial solution.
    5. For each column j such that Wr, j != '_',
        for each row i such that Wi, j != '_',
            delete row i from matrix W.
        delete column j from matrix W.
    6. Repeat this algorithm recursively on the reduced matrix 
*/
void algorithmX(char** W, int rowSize, int colSize, int col, char* solution, char* F){
    //print solution when there are no nonempty columns left
    if(col==colSize){ 
        printFilteredSolution(solution, colSize, F);
        return;
    }

    col=findColumn(W, rowSize, colSize, col); 
    if(col!=-1){ 
        for(int i=0; i<rowSize; i++){
            if(W[i][col]!='_'){
                char** tempW=copyW(W, rowSize, colSize);
                char* tempSolution=copySolution(solution, colSize);
                partialSolution(tempW, colSize, i, tempSolution);
                for(int j=0; j<colSize; j++){
                    if(tempW[i][j]!='_'){
                        for(int k=0; k<rowSize; k++){
                            if(k!=i && tempW[k][j]!='_'){
                                deleteRow(tempW, colSize, k);
                            }
                        }
                        deleteColumn(tempW, rowSize, j);
                    }
                }
                deleteRow(tempW, colSize, i);
                algorithmX(tempW, rowSize, colSize, col+1, tempSolution, F);
                free(tempSolution);
                freeW(tempW, rowSize);
            }
        }
    }
    else
        algorithmX(W, rowSize, colSize, colSize, solution, F);
}
//finds covers for the exact cover problem
void findCovers(char **W, int rowSize, int colSize, char* F){
    char* solution=malloc(sizeof(char)*((long unsigned int)colSize));
    for(int i=0; i<colSize; i++)
        solution[i]='_';
    algorithmX(W, rowSize, colSize, 0, solution, F);
    free(solution);
}
//initializes filter, subsets for exact cover problem, finds all covers and frees memory
int main(){
    //initialize filter
    char *tempF=malloc(sizeof(char)*MAX_COLUMNS);
    int colSize=readFandColSize(tempF);
    char *F=malloc(sizeof(char)*((long unsigned int)colSize));
    actualF(F, tempF, colSize);
    free(tempF);
    
    //initialize subsets
    char **tempW=malloc(sizeof(char*)*MAX_ROWS);
    for(int i=0; i<MAX_ROWS; i++){
        tempW[i]=malloc(sizeof(char)*((long unsigned int)colSize));
    }
    int rowSize=readWandRowSize(tempW, colSize);
    char **W=copyW(tempW, rowSize, colSize);
    freeW(tempW, MAX_ROWS);

    findCovers(W, rowSize, colSize, F);

    free(F);
    freeW(W, rowSize);
    return 0;
}
