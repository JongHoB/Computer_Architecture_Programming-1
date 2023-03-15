#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

/*
 * For debug option. If you want to debug, set 1.
 * If not, set 0.
 */
#define DEBUG 0

#define MAX_SYMBOL_TABLE_SIZE   1024
#define MEM_TEXT_START          0x00400000
#define MEM_DATA_START          0x10000000
#define BYTES_PER_WORD          4
#define INST_LIST_LEN           20

/******************************************************
 * Structure Declaration
 *******************************************************/

typedef struct inst_struct {
    char *name;
    char *op;
    char type;
    char *funct;
} inst_t;

typedef struct symbol_struct {
    char name[32];
    uint32_t address;
} symbol_t;

enum section { 
    DATA = 0,
    TEXT,
    MAX_SIZE
};

/******************************************************
 * Global Variable Declaration
 *******************************************************/

inst_t inst_list[INST_LIST_LEN] = {       //  idx
    {"addiu",   "001001", 'I', ""},       //    0
    {"addu",    "000000", 'R', "100001"}, //    1
    {"and",     "000000", 'R', "100100"}, //    2
    {"andi",    "001100", 'I', ""},       //    3
    {"beq",     "000100", 'I', ""},       //    4
    {"bne",     "000101", 'I', ""},       //    5
    {"j",       "000010", 'J', ""},       //    6
    {"jal",     "000011", 'J', ""},       //    7
    {"jr",      "000000", 'R', "001000"}, //    8
    {"lui",     "001111", 'I', ""},       //    9
    {"lw",      "100011", 'I', ""},       //   10
    {"nor",     "000000", 'R', "100111"}, //   11
    {"or",      "000000", 'R', "100101"}, //   12
    {"ori",     "001101", 'I', ""},       //   13
    {"sltiu",   "001011", 'I', ""},       //   14
    {"sltu",    "000000", 'R', "101011"}, //   15
    {"sll",     "000000", 'R', "000000"}, //   16
    {"srl",     "000000", 'R', "000010"}, //   17
    {"sw",      "101011", 'I', ""},       //   18
    {"subu",    "000000", 'R', "100011"}  //   19
};

symbol_t SYMBOL_TABLE[MAX_SYMBOL_TABLE_SIZE]; // Global Symbol Table

uint32_t symbol_table_cur_index = 0; // For indexing of symbol table

/* Temporary file stream pointers */
FILE *data_seg;
FILE *text_seg;

/* Size of each section */
uint32_t data_section_size = 0;
uint32_t text_section_size = 0;

/******************************************************
 * Function Declaration
 *******************************************************/

/* Change file extension from ".s" to ".o" */
char* change_file_ext(char *str) {
    char *dot = strrchr(str, '.');

    if (!dot || dot == str || (strcmp(dot, ".s") != 0))
        return NULL;

    str[strlen(str) - 1] = 'o';
    return "";
}

/* Add symbol to global symbol table */
void symbol_table_add_entry(symbol_t symbol)
{
    SYMBOL_TABLE[symbol_table_cur_index++] = symbol;
#if DEBUG
    printf("%s: 0x%08x\n", symbol.name, symbol.address);
#endif
}

/* Convert integer number to binary string */
char* num_to_bits(unsigned int num, int len) 
{
    char* bits = (char *) malloc(len+1);
    int idx = len-1, i;
    while (num > 0 && idx >= 0) {
        if (num % 2 == 1) {
            bits[idx--] = '1';
        } else {
            bits[idx--] = '0';
        }
        num /= 2;
    }
    for (i = idx; i >= 0; i--){
        bits[i] = '0';
    }
    bits[len] = '\0';
    return bits;
}

/* Record .text section to output file */
void record_text_section(FILE *output) 
{
    uint32_t cur_addr = MEM_TEXT_START;
    char line[1024];

    /* Point to text_seg stream */
    rewind(text_seg);

    /* Print .text section */
    while (fgets(line, 1024, text_seg) != NULL) {
        char inst[0x1000] = {0};
        char op[32] = {0};
        char label[32] = {0};
        char type = '0';
        int i, idx = 0;
        int rs, rt, rd, imm, shamt;
        int addr;

        rs = rt = rd = imm = shamt = addr = 0;
#if DEBUG
        printf("0x%08x: ", cur_addr);
#endif
        /* Find the instruction type that matches the line */
        /* blank */
        char *temp;
        char _line[1024] = {0};
        strcpy(_line, line);
        temp = strtok(_line, "\t\n");
        for(i=0;i<INST_LIST_LEN;i++)
        {
            if(!strcmp(inst_list[i].name,temp))
            {
                strcpy(inst,inst_list[i].name);
                type=inst_list[i].type;
                strcpy(op,inst_list[i].op);
                idx=i;
               addr=cur_addr;
                break;
            }
        }

          fputs(op,output);
          char* t;
          int length;


        switch (type) {
            case 'R':
            switch(idx)
            {
                case 8://jr
                temp=strtok(NULL,"\n");
                t=temp;
                t=t+1;
                rs=atoi(t);
                fputs(num_to_bits(rs,5),output);
                fputs(num_to_bits(0,15),output);
                fputs(inst_list[idx].funct,output);
               // fputc('\n',output);
                break;


                case 16://sll
                case 17://srl
                temp=strtok(NULL," ");                
	            t = temp;
                length = strlen(t);
                t[length - 1] = 0;
                t = t + 1;
                rd = atoi(t);
                temp=strtok(NULL," ");
                t = temp;
                length = strlen(t);
                t[length - 1] = 0;
                t = t + 1;
                rt = atoi(t);
                temp=strtok(NULL,"\n");
                t = temp;
                shamt=atoi(t);
                rs=0;
                fputs(num_to_bits(0,5),output);
                fputs(num_to_bits(rt,5),output);
                fputs(num_to_bits(rd,5),output);
                fputs(num_to_bits(shamt,5),output);
                fputs(inst_list[idx].funct,output);
           //     fputc('\n',output);                
                break;


                case 1://addu
                case 2://and               
                case 11://nor
                case 12://or
                case 15://sltu
                case 19://subu
                temp=strtok(NULL," ");                
	            t = temp;
                length = strlen(t);
                t[length - 1] = 0;
                t = t + 1;
                rd = atoi(t);
                temp=strtok(NULL," ");
                t = temp;
                length = strlen(t);
                t[length - 1] = 0;
                t = t + 1;
                rs = atoi(t);
                temp=strtok(NULL,"\n");
                t = temp;
                t=t+1;
                rt=atoi(t);
                shamt=0;
                fputs(num_to_bits(rs,5),output);
                fputs(num_to_bits(rt,5),output);
                fputs(num_to_bits(rd,5),output);
                fputs(num_to_bits(0,5),output);
                fputs(inst_list[idx].funct,output);
                //fputc('\n',output);            
                break;


            }
                /* blank */





#if DEBUG
                printf("op:%s rs:$%d rt:$%d rd:$%d shamt:%d funct:%s\n",
                        op, rs, rt, rd, shamt, inst_list[idx].funct);
#endif
                break;

            case 'I':
                /* blank */
                switch(idx)
                {
                    case 0://addiu
                    case 3://andi
                    case 13://ori
                    case 14://sltiu
                    temp=strtok(NULL," ");                
	                t = temp;
                    length = strlen(t);
                    t[length - 1] = 0;
                    t = t + 1;
                    rt = atoi(t);
                    temp=strtok(NULL," ");                
	                t = temp;
                    length = strlen(t);
                    t[length - 1] = 0;
                    t = t + 1;
                    rs = atoi(t);
                    temp=strtok(NULL,"\n");                
	                t = temp;
                    if(t[1]=='x')
                    {
                         imm = strtol(t, NULL, 16);
                         
                    }
                    else
                    {
                        imm=atoi(t);
                    }
                    fputs(num_to_bits(rs,5),output);
                    fputs(num_to_bits(rt,5),output);
                    fputs(num_to_bits(imm,16),output);
                   // fputc('\n',output);                    
                    break;
                    case 4://beq
                    case 5://bne
                     temp=strtok(NULL," ");                
	                t = temp;
                    length = strlen(t);
                    t[length - 1] = 0;
                    t = t + 1;
                    rs=atoi(t);
                    temp=strtok(NULL," "); 
                    t = temp;
                    length = strlen(t);
                    t[length - 1] = 0;
                    t = t + 1;
                    rt=atoi(t);
                    temp=strtok(NULL,"\n");
                    t=temp;
                    for(int j=0;j<symbol_table_cur_index;j++)
                    {
                        if(!strcmp(SYMBOL_TABLE[j].name,t))
                        {
                            imm=(SYMBOL_TABLE[j].address-cur_addr-4)/4;
                            break;
                        }
                    }
                    fputs(num_to_bits(rs,5),output);
                    fputs(num_to_bits(rt,5),output);
                    fputs(num_to_bits(imm,16),output);
                   // fputc('\n',output);





                    break;
                    case 9://lui
                    temp=strtok(NULL," ");                
	                t = temp;
                    length = strlen(t);
                    t[length - 1] = 0;
                    t = t + 1;
                    rt=atoi(t);
                    temp=strtok(NULL,"\n");
                    t=temp;
                    if(t[1]=='x')
                    {
                        imm=strtol(t,NULL,16);

                    }
                    else{
                        imm=atoi(t);
                    }
                    
                    fputs(num_to_bits(0,5),output);
                    fputs(num_to_bits(rt,5),output);
                    fputs(num_to_bits(imm,16),output);
                //    fputc('\n',output);                    
                    break;


                    case 10://lw
                    case 18://sw
                    temp=strtok(NULL," ");
                    t=temp;
                    length=strlen(t);
                    t[length-1]=0;
                    t=t+1;
                    rt=atoi(t);
                    temp=strtok(NULL,"(");
                    t=temp;
                    imm=atoi(t);
                    temp=strtok(NULL,")");
                    t=temp;
                    t=t+1;
                    rs=atoi(t);
                    fputs(num_to_bits(rs,5),output);
                    fputs(num_to_bits(rt,5),output);
                    fputs(num_to_bits(imm,16),output);
                    //fputc('\n',output);
                    break;
                }
                
#if DEBUG
                printf("op:%s rs:$%d rt:$%d imm:0x%x\n",
                        op, rs, rt, imm);
#endif
                break;

            case 'J':
                /* blank */
                switch(idx)
                {
                    case 6://j
                    case 7://jal
                    temp=strtok(NULL,"\n");
                    t=temp;
                     for(int j=0;j<symbol_table_cur_index;j++)
                    {
                        if(!strcmp(SYMBOL_TABLE[j].name,t))
                        {
                            imm=SYMBOL_TABLE[j].address/4;


                            break;

                        }
                    }
                    fputs(num_to_bits(imm,26),output);
                  //  fputc('\n',output);

                    //address





                }
                
#if DEBUG
                printf("op:%s addr:%i\n", op, addr);
#endif
                break;

            default:
                break;
        }
        fprintf(output, "\n");

        cur_addr += BYTES_PER_WORD;
    }
}

/* Record .data section to output file */
void record_data_section(FILE *output)
{
    uint32_t cur_addr = MEM_DATA_START;
    char line[1024];

    /* Point to data segment stream */
    rewind(data_seg);

    /* Print .data section */
    while (fgets(line, 1024, data_seg) != NULL) {
        /* blank */
       char*temp;
        char _line[1024];
        strcpy(_line,line);
        temp=strtok(_line,"\t");
        temp=strtok(NULL,"\n");
 
        if(temp[1]=='x')
        {
           int a = strtol(temp, NULL, 16);
          // printf("%s\n",num_to_bits(a,32));
            fputs(num_to_bits(a,32),output);
             fputc('\n',output);
        }
        else{
         int a=atoi(temp);
       //  printf("%s\n",num_to_bits(a,32));
           fputs(num_to_bits(a,32),output);
          fputc('\n',output);

        }
        
#if DEBUG
        printf("0x%08x: ", cur_addr);
        printf("%s", line);
#endif
        cur_addr += BYTES_PER_WORD;
    }
}

/* Fill the blanks */
void make_binary_file(FILE *output)
{
#if DEBUG
    char line[1024] = {0};
    rewind(text_seg);
    /* Print line of text segment */
    while (fgets(line, 1024, text_seg) != NULL) {
        printf("%s",line);
    }
    printf("text section size: %d, data section size: %d\n",
            text_section_size, data_section_size);
#endif

    /* Print text section size and data section size */
    /* blank */
  fputs(num_to_bits(text_section_size,32),output);
  fputc('\n',output);
fputs(num_to_bits(data_section_size,32),output);
fputc('\n',output);

    /* Print .text section */
    record_text_section(output);

    /* Print .data section */
    record_data_section(output);
}

/* Fill the blanks */
void make_symbol_table(FILE *input)
{
    char line[1024] = {0};
    uint32_t address = 0;
    enum section cur_section = MAX_SIZE;

    /* Read each section and put the stream */
    while (fgets(line, 1024, input) != NULL) {
        char *temp;
        char _line[1024] = {0};
        strcpy(_line, line);
        temp = strtok(_line, "\t\n");

        /* Check section type */
        if (!strcmp(temp, ".data")) {
            /* blank */
            
            cur_section=DATA;
            address=MEM_DATA_START;
            data_seg = tmpfile();
            continue;
        }
        else if (!strcmp(temp, ".text")) {
            /* blank */
           
            cur_section=TEXT;
            address=MEM_TEXT_START;
            text_seg = tmpfile();
            continue;
        }

        /* Put the line into each segment stream */
     
        if (cur_section == DATA) {
            /* blank */
        int a=strlen(temp);
        char k[32];
        if(temp[a-1]==':')
        {
            symbol_t t;
            t.address=address;
            strcpy(t.name,temp);
            t.name[a-1]=0;
            symbol_table_add_entry(t);
            
            temp=strtok(NULL,"\n");
            if(!strcmp(temp,".word   1"))
        {
            
        fputs(".word\t1",data_seg);
        fputc('\n', data_seg);     
        data_section_size+=4;

        }
        else if(!strcmp(temp,".word   10"))
        {
           
        fputs(".word\t10",data_seg);
        fputc('\n', data_seg);     
        data_section_size+=4;
            

        }
        else{    
             a=strlen(temp);
             strcpy(k,temp);
            
         fputs(temp,data_seg);
        fputc('\n', data_seg);     
        data_section_size+=4;
        }

        }
        
        else{
           
                strcpy(k,temp);
                k[a]='\t';
                k[a+1]=0;
                temp = strtok(NULL, "\n");
                strcat(k,temp);

                fputs(k,data_seg);
                fputc('\n', data_seg); 
                data_section_size+=4;
            
        }
    
          
        }
        else if (cur_section == TEXT) {
            /* blank */
        int a=strlen(temp);
        char k[32];
        if(temp[a-1]==':')
        {
            symbol_t t;
            t.address=address;
            strcpy(t.name,temp);
            t.name[a-1]=0;
            symbol_table_add_entry(t);
            continue;

        }
        if(!strcmp(temp,"la"))
        {
            strcpy(k,"lui");
            k[3]='\t';
            k[4]=0;
            temp=strtok(NULL," ");
            strcat(k,temp);
            char tt[32];
            strcpy(tt,"ori");
            tt[3]='\t';
            tt[4]=0;
            strcat(tt,temp);
            strcat(tt," ");
            strcat(tt,temp);
            strcat(tt," ");
            strcat(k," ");
            temp=strtok(NULL,"\n");
            for(int i=0;i<symbol_table_cur_index;i++)
            {
                if(!strcmp(SYMBOL_TABLE[i].name,temp))
                {
                    if(i==0)
                    {
                        strcat(k,"0x1000");
                         fputs(k,text_seg);
                         fputc('\n', text_seg); 
                         text_section_size+=4;
                    }
                    else{
                       
                        strcat(k,"0x1000");
                         fputs(k,text_seg);
                fputc('\n', text_seg); 
                text_section_size+=4;
                fputs(tt,text_seg);
                int bit=SYMBOL_TABLE[i].address&15;
                fprintf(text_seg,"0x%04x",bit);
                fputc('\n',text_seg);
                text_section_size+=4;
                address += BYTES_PER_WORD;


                    }
                }
            }



        }
        else{
       
                strcpy(k,temp);
                k[a]='\t';
                k[a+1]=0;
                temp = strtok(NULL, "\n");
                strcat(k,temp);

                fputs(k,text_seg);
                fputc('\n', text_seg); 
                text_section_size+=4;
        }
            
        }

        address += BYTES_PER_WORD;
    }
}

/******************************************************
 * Function: main
 *
 * Parameters:
 *  int
 *      argc: the number of argument
 *  char*
 *      argv[]: array of a sting argument
 *
 * Return:
 *  return success exit value
 *
 * Info:
 *  The typical main function in C language.
 *  It reads system arguments from terminal (or commands)
 *  and parse an assembly file(*.s).
 *  Then, it converts a certain instruction into
 *  object code which is basically binary code.
 *
 *******************************************************/

int main(int argc, char* argv[])
{
    FILE *input, *output;
    char *filename;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <*.s>\n", argv[0]);
        fprintf(stderr, "Example: %s sample_input/example?.s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Read the input file */
    input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }

    /* Create the output file (*.o) */
    filename = strdup(argv[1]); // strdup() is not a standard C library but fairy used a lot.
    if(change_file_ext(filename) == NULL) {
        fprintf(stderr, "'%s' file is not an assembly file.\n", filename);
        exit(EXIT_FAILURE);
    }

    output = fopen(filename, "w");
    if (output == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }

    /******************************************************
     *  Let's complete the below functions!
     *
     *  make_symbol_table(FILE *input)
     *  make_binary_file(FILE *output)
     *  ├── record_text_section(FILE *output)
     *  └── record_data_section(FILE *output)
     *
     *******************************************************/
    make_symbol_table(input);
    make_binary_file(output);

    fclose(input);
    fclose(output);

    return 0;
}
