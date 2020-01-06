#include <stdio.h>
#include <string.h>

int main() {
    FILE *readfile = fopen("switch.c", "r");
    FILE *writefile = fopen("switch.s", "r");

    int lineLength = 255;
    char line[lineLength];
    int maxCase = 0, minCase = 0, firstCasePassed = 0;
    // goal - find the maxCase and minCase
    while (fgets(line, lineLength, readfile)) {
        if (strstr(line, "case") != NULL) { // if line contains the word "case"
            int i = 4, startNumber = 0, demical = 1, number = 0;
            while (i < lineLength) {
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

    printf("max: %d, min: %d\n", maxCase, minCase);
    int casesArrSize = maxCase - minCase + 2;
    char cases[casesArrSize];
    int i = 0;
    for (i = 0; i < casesArrSize - 1; i++) {
        cases[i] = '0';
    }
    cases[casesArrSize -1] = '\0';

    rewind(readfile);
    while (fgets(line, lineLength, readfile)) {
        if (strstr(line, "case") != NULL) { // if line contains the word "case"
            int i = 4, startNumber = 0, demical = 1, number = 0;
            while (i < lineLength) {
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

    fclose(readfile);


    return 0;
}
