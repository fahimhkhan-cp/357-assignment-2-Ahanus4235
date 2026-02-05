#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>

#define NAME_LEN 32

void print_inode_file(const char *inode_filename);
char *uint32_to_str(uint32_t i);
void *checked_malloc(int len);
typedef struct {
    uint32_t index;
    char type;
}Inode;

Inode inodes[1024];
uint32_t cwd = 0;

int main(int argc, char* argv[]){
    if (argc<2){
        printf("No emulated fs specified.\n");
        return 1;
    }
    

    //Using the directory in argv[1] to open the inodes_list file
    if (chdir(argv[1])!=0){ //Should be in the fs directory now
        printf("Specified directory doesn't exist\n");
    }
    
    FILE *inodes_list = fopen("inodes_list","rb");

    //We are first going to check that inode 0 is a directory
    uint32_t inode_num;
    char type;
    fread(&inode_num, sizeof(uint32_t), 1, inodes_list);
    fread(&type, sizeof(char), 1, inodes_list);
    if (inode_num!=0 || type!='d'){
        printf("Inode 0 either doesn't exist or is not a directory.\n");
        return 1;
    }

    //Now we can make our array of inode structs to work with
    inodes[0].index=inode_num;
    inodes[0].type=type;
    uint32_t inodes_count=1;

    while( fread(&inode_num, sizeof(uint32_t), 1, inodes_list) &&
    fread(&type, sizeof(char), 1, inodes_list)){
        inodes[inodes_count].index=inode_num;
        inodes[inodes_count].type=type;
        inodes_count+=1;
    }
    fclose(inodes_list);

    //Now we need to start in the directory there?
    //Or we just need to track inode 0 as the cwd..how?
   
    
    char user_input[128];
    //Make for a max of 10 command inputs
    char tokens[10][32];
    int index;
    while (fgets(user_input, sizeof(user_input),stdin)!=NULL){
        index=0;
        //clearing the inputs form last use
        for (int i = 0; i < 10; i++) {
        tokens[i][0] = '\0'; // Mark as empty string
        }

        char *word = strtok(user_input," \t\r\n\v\f");

        while(word !=NULL){
            strcpy(tokens[index], word);
            index=index+1;
            word = strtok(NULL," \t\r\n\v\f");
        }
        //After this point, tokens should contain each command the user input
        //The command in tokens[0] should be among:
        //exit, ls, cd, touch, or mkdir

        //exit function
        if (strcmp(tokens[0],"exit")==0){
            printf("Exiting and saving.\n");

            FILE *updateInodesList = fopen("inodes_list","w");
            for (int c = 0; c<inodes_count;c++){
                fwrite(&inodes[c].index, sizeof(uint32_t), 1, updateInodesList);
                fwrite(&inodes[c].type, sizeof(char), 1, updateInodesList);
            }
            fclose(updateInodesList);
            exit(0);
        }

        //ls function
        else if (strcmp(tokens[0],"ls")==0){
            char* cwdstr=uint32_to_str(cwd);
            print_inode_file(cwdstr);
            free(cwdstr);
        }

        //cd function
        else if (strcmp(tokens[0],"cd")==0){

            //Checking cwd for the desired dir name
            char* fileName=uint32_to_str(cwd);
            FILE *f = fopen(fileName, "rb");
            uint32_t inode;
            char name[NAME_LEN];
            uint32_t precwd=cwd;
            while (fread(&inode, sizeof(uint32_t), 1, f) == 1 &&
           fread(name, 1, NAME_LEN, f) == NAME_LEN) {

                //If we find an inode with the desired name, 
                //check if it is of type 'd'
                if (strcmp(name,tokens[1])==0){
                    if (inodes[inode].type=='d'){
                        cwd=inode;
                    }                
                }    
            }
            fclose(f);
            free(fileName);
            //If the cwd did not change, give an error message
            if (precwd==cwd){
                printf("Specified name was not a  directory.\n");
            }
        }

        //touch
        else if (strcmp(tokens[0],"touch")==0){
            if (strlen(tokens[1])==0){
                printf("No file name specified\n");
            }
            else{

            if (strlen(tokens[1])>32){
                tokens[1][NAME_LEN]='\0';
            }
                char* fileName=uint32_to_str(cwd);
            FILE *f = fopen(fileName, "rb");
            uint32_t inode;
            char name[NAME_LEN];
            int nameFound=0;
            while (fread(&inode, sizeof(uint32_t), 1, f) == 1 &&
           fread(name, 1, NAME_LEN, f) == NAME_LEN) {

                //We need to check the cwd for an inode with the same name
                if (strcmp(name,tokens[1])==0){
                    nameFound=1;
                    break;             
                }    
           }
            if(nameFound==1){
                printf("A file or directory with that name already exists.\n");
            }
            else if (inodes_count==1023){
                    printf("maximum inode capacity reached.\n");
                }
            else{
                char* newInodeNum = uint32_to_str(inodes_count);
                FILE *newInode = fopen(newInodeNum,"w");
                
                char* cwdstr= uint32_to_str(cwd);
                FILE *updatecwd=fopen(cwdstr,"a");
                //Updating the cwd with the directory we are creating
                fwrite(&inodes_count, sizeof(uint32_t), 1, updatecwd);
                fwrite(tokens[1], 1, NAME_LEN, updatecwd);

                //Adding the filename to the newly created file
                fwrite(tokens[1], 1, NAME_LEN, newInode);

                //Adding it to our inodes list of structs
                //This will help in updating the inodes_list file on program exit
                inodes[inodes_count].index=inode_num;
                inodes[inodes_count].type='f';
                
                inodes_count+=1;

                free(cwdstr);
                fclose(updatecwd);
                free(newInodeNum);
                fclose(newInode);
            }
            free(fileName);
            fclose(f);
            }
        }
        //mkdir
        else if (strcmp(tokens[0],"mkdir")==0){
            if (strlen(tokens[1])==0){
                printf("No directory name specified\n");
            }
            else{
                char* fileName=uint32_to_str(cwd);
            FILE *f = fopen(fileName, "rb");
            uint32_t inode;
            char name[NAME_LEN];
            int nameFound=0;
            while (fread(&inode, sizeof(uint32_t), 1, f) == 1 &&
           fread(name, 1, NAME_LEN, f) == NAME_LEN) {

                //We need to check the cwd for an inode with the same name
                if (strcmp(name,tokens[1])==0){
                    nameFound=1;
                    break;             
                }    
           }
            if(nameFound==1){
                printf("A file or directory with that name already exists.\n");
            }
            else if (inodes_count==1023){
                    printf("maximum inode capacity reached.\n");
                }
            else{
                char* newInodeNum = uint32_to_str(inodes_count);
                FILE *newInode = fopen(newInodeNum,"w");
                
                char* cwdstr= uint32_to_str(cwd);
                FILE *updatecwd=fopen(cwdstr,"a");
                //Updating the cwd with the directory we are creating
                fwrite(&inodes_count, sizeof(uint32_t), 1, updatecwd);
                fwrite(tokens[1], 1, NAME_LEN, updatecwd);

                //Writing the two default entries in the directory
                fwrite(&inodes_count, sizeof(uint32_t), 1, newInode);
                fwrite(".", 1, NAME_LEN, newInode);
                fwrite(&cwd, sizeof(uint32_t), 1, newInode);
                fwrite("..", 1, NAME_LEN, newInode);

                //Adding it to our inodes list of structs
                //This will help in updating the inodes_list file on program exit
                inodes[inodes_count].index=inode_num;
                inodes[inodes_count].type='d';
                
                inodes_count+=1;

                free(cwdstr);
                fclose(updatecwd);
                free(newInodeNum);
                fclose(newInode);
            }
            free(fileName);
            fclose(f);
            }
        }
        else{
            printf("Unkown command.\n");
        }
    }
}

void print_inode_file(const char *inode_filename) {
    FILE *f = fopen(inode_filename, "rb");
    if (!f) {
        perror("Error opening file");
        return;
    }

    uint32_t inode;
    unsigned char name[NAME_LEN];

    while (fread(&inode, sizeof(uint32_t), 1, f) == 1 &&
           fread(name, 1, NAME_LEN, f) == NAME_LEN) {

        printf("%u %s\n", inode, name);
    }

    fclose(f);

    printf("\n"); //This line is just for cleaner output
}

void *checked_malloc(int len)
{
    void *p = malloc(len);
    if (!p)
    {
        fprintf(stderr,"\nRan out of memory!\n");
        exit(1);
    }
    return p;
}

char *uint32_to_str(uint32_t i)
{
   int length = snprintf(NULL, 0, "%lu", (unsigned long)i);       // pretend to print to a string to get length
   char* str = checked_malloc(length + 1);                        // allocate space for the actual string
   snprintf(str, length + 1, "%lu", (unsigned long)i);            // print to string

   return str;
}

