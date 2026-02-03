#define _GSU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
    if (argc<2){
        printf("No emulated fs specified.\n");
        return 1;
    }
    //use argv[1] here to open something like fs_copy

    char user_input[128];
    char tokens[10][32];
    int index;
    while (fgets(user_input, sizeof(user_input),stdin)!=NULL){
        index=0;

        char *word = strtok(user_input," \t\r\n\v\f");

        while(word !=NULL){
            strcpy(tokens[index], word);
            index=index+1;
            word = strtok(NULL," \t\r\n\v\f");
        }
        //After this point, tokens should contain each command the user input
        //The command in tokens[0] should be among:
        //exit, ls, cd, touch, or mkdir
        if (strcmp(tokens[0],"exit")==0){
            printf("Exiting simulation\n");
            return 0;
        }
        else if (strcmp(tokens[0],"ls")==0){
            printf("Valid command\n");
        }
        else if (strcmp(tokens[0],"cd")==0){
            printf("Valid command\n");
        }
        else if (strcmp(tokens[0],"touch")==0){
            printf("Valid command\n");
        }
        else if (strcmp(tokens[0],"mkdir")==0){
            printf("Valid command\n");
        }
        else{
            printf("Unkown command.\n");
        }
    }
}