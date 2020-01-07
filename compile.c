#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *getAssemblyName(char *side);

void writeToSwitch(char *side, char *side1, char *anOperator, int pointers, FILE *pFile);

int main() {
    FILE *readfile = fopen("switch.c", "r");
    FILE *writefile = fopen("switch.s", "w");

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if (readfile == NULL) {
        exit(EXIT_FAILURE);
    }

    long maxCase = 0, minCase = 0, firstCasePassed = 0;
    // goal - find the maxCase and minCase
    while ((read = getline(&line, &len, readfile)) != -1) {
        if (strstr(line, "case") != NULL) { // if line contains the word "case"
            int i = 4, startNumber = 0, demical = 1, number = 0;
            while (i < read) {
                if (line[i] == ' ') {
                    startNumber = 1;
                } else if (line[i] == ':') {
                    break; // we don't need to check anymore
                } else if (startNumber > 0) {
                    number = number * demical;
                    number += (line[i] - '0');
                    demical = demical * 10;
                }
                i++;
            }
            if (!firstCasePassed) {
                maxCase = number;
                minCase = number;
                firstCasePassed = 1;
            } else {
                if (number > maxCase) {
                    maxCase = number;
                } else if (number < minCase) {
                    minCase = number;
                }
            }

        }
    }

    // now we know who is the max and who is the min
    printf("max: %ld, min: %ld\n", maxCase, minCase);
    int casesArrSize = maxCase - minCase + 2;
    char cases[casesArrSize];
    int i = 0;
    for (i = 0; i < casesArrSize - 1; i++) {
        cases[i] = '0';
    }
    cases[casesArrSize - 1] = '\0';

    rewind(readfile); // put pointer back to start
    while ((read = getline(&line, &len, readfile)) != -1) {
        if (strstr(line, "case") != NULL) { // if line contains the word "case"
            int i = 4, startNumber = 0, demical = 1, number = 0;
            while (i < read) {
                if (line[i] == ' ') {
                    startNumber = 1;
                } else if (line[i] == ':') {
                    break; // we don't need to check anymore
                } else if (startNumber > 0) {
                    number = number * demical;
                    number += (line[i] - '0');
                    demical = demical * 10;
                }
                i++;
            }
            int index = number - minCase;
            cases[index] = '1';
        }
    }

    printf("%s\n", cases);
    // now we filled up the cases array

    rewind(readfile); // put pointer back to start

    long diffrent = maxCase - minCase;

    // start writing switch.s
    fputs(".section .text\n", writefile);
    fputs(".globl switch2\n", writefile);
    fputs("switch2:\n", writefile);
    fputs("\tmovq\t$0,%rax\n", writefile); //result = 0

    // keep in mind - result = %rax, *p1 = %rdi, *p2 = %rsi,
    // action (current case) = %rdx, helper register = %rcx

    // next both lines are toghether
    fprintf(writefile, "\tsubq\t$%ld", minCase); // ld- long int
    fputs(",%rdx\n", writefile); // action = action- minCase

    // next both lines are toghether
    fprintf(writefile, "\tcmpq\t$%ld", diffrent); // action - diffetent
    fputs(",%rdx\n", writefile);

    char *defaultCase = ".DefaultCase:";
    fprintf(writefile, "ja  \t%s\n", defaultCase); // if !(SF ^ ZF) - go to default

    // next both lines are toghether
    fprintf(writefile, "jmp \t*.CaseTable"); // else go to the right place
    fputs("(,%rdx,8)\n", writefile);

    char *afterBreak = ".FinishSwitch";

    int foundCase = 0;

    while ((read = getline(&line, &len, readfile)) != -1 && !(strstr(line, "}") != NULL)) {
        // get to first time of "case"
        if (!foundCase) {
            while ((read = getline(&line, &len, readfile)) != -1) {
                if (strstr(line, "case") != NULL) { // if line contains the word "case"
                    foundCase = 1;
                    break;
                }
            }
        }
        foundCase = 1;
        if (strstr(line, "case") != NULL) { // if line contains the word "case" so we need to write the case
            int i = 0, startNumber = 0, demical = 1, number = 0;
            while (i < read) {
                if (line[i] == ' ') {
                    startNumber = 1;
                } else if (line[i] == ':') {
                    break; // we don't need to check anymore
                } else if (startNumber > 0) {
                    number = number * demical;
                    number += (line[i] - '0');
                    demical = demical * 10;
                }
                i++;
            }
            number = number - minCase;
            fprintf(writefile, ".L%d:\n", number);
        } else if (strstr(line, "break;") != NULL) { // jump to break case
            fprintf(writefile, "\tjmp %s\n", afterBreak);
        } else if (strstr(line, "default:") != NULL) {
            fprintf(writefile, "%s\n", defaultCase); // default case
        } else {
            // got 2 elements with operator here
            i = 0;
            char *rightSide, *leftSide, *operator;
            int numOfWord = 1;
            while ((line[i] == ' ' || i == 0) && i < read) {
                i++;
                int stringSize = 1;
                char *str = (char *) malloc(stringSize);
                while (line[i] != ' ' && line[i] != ';' && i < read) {
                    str = (char *) realloc(str, stringSize);
                    str[stringSize - 1] = line[i];
                    stringSize++;
                    i++;
                }
                if (numOfWord == 1) {
                    leftSide = strdup(str);
                } else if (numOfWord == 2) {
                    operator = strdup(str);
                } else if (numOfWord == 3) {
                    rightSide = strdup(str);
                }
                numOfWord++;
            }
            int numOfPointers = 0;
            if (strstr(leftSide, "*")) {
                numOfPointers++;
            }
            if (strstr(rightSide, "*")) {
                numOfPointers++;
            }
            leftSide = getAssemblyName(leftSide);
            rightSide = getAssemblyName(rightSide);

            writeToSwitch(leftSide, rightSide, operator, numOfPointers, writefile);

        }
    }

    fprintf(writefile, "%s:\n", afterBreak); // add another one to finish the switch and go to next commands

    fputs("\tret\n", writefile);
    fputs(".section .rodata\n", writefile);
    fputs(".align 8\n", writefile);
    fputs(".CaseTable:\n", writefile);

    // now we need to write the cases from minCase to maxCase
    // if the number doesn't exist in the cases, put default case instead

    for (i = 0; i < diffrent + 1; i++) {
        fputs("\t.quad ", writefile);
        if (cases[i] == '1') {
            fprintf(writefile, ".L%d", i);
        } else {
            fprintf(writefile, "%s", defaultCase);
        }
        fputs("\n", writefile);
    }


    fclose(readfile);
    fclose(writefile);


    return 0;
}

void writeToSwitch(char *leftSide, char *rightSide, char *operator,
                   int numOfPointers, FILE *writefile) {

    char *function = "";
    int shiftFlag = 0;
    if (strcmp(operator, "+=") == 0) { function = "addq"; }
    else if (strcmp(operator, "=") == 0) { function = "movq"; }
    else if (strcmp(operator, "-=") == 0) { function = "subq"; }
    else if (strcmp(operator, "*=") == 0) { function = "imulq"; }
    else if (strcmp(operator, ">>=") == 0) { function = "sarq"; shiftFlag = 1; }
    else if (strcmp(operator, "<<=") == 0) { function = "salq"; shiftFlag = 1; }


    if (numOfPointers == 2) {
        fprintf(writefile, "\tmovq\t%s,", rightSide);
        fputs("%rcx\n", writefile);

        fprintf(writefile, "\t%s\t", function);
        fputs("%rcx,", writefile);
        fprintf(writefile, "%s\n", leftSide);
    } else {
        char *dollarsign = "";
        if (rightSide[0] >= '0' && rightSide[0] <= '9') { // it's a number
            dollarsign = "$";
            shiftFlag = 0; // we don't need it anyway if we shift just number
        }

        if(shiftFlag){
            fprintf(writefile, "\tmovq\t%s,", rightSide);
            fputs("%rcx\n", writefile);
            rightSide = "%cl";
        }


        fprintf(writefile, "\t%s\t%s%s,%s\n", function, dollarsign, rightSide, leftSide);

    }

}

char *getAssemblyName(char *str) {
    if (strstr(str, "*")) {
        if (strcmp(str, "*p1") == 0) {
            str = "(%rdi)";
        } else {
            str = "(%rsi)";
        }
    } else if (strcmp(str, "result") == 0) {
        str = "%rax";
    }
    return str;
}
