#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Indexes of columns in the input 
#define EMAIL 0
#define TIMESTAMP 1
#define FIRST_NAME 24
#define LAST_NAME 23
#define GRADE 25
#define GENDER 26
#define FIRST_QUESTION 2

// Number of questions
#define NUM_OF_QUESTIONS 21

struct Response {
    int id;
    char firstName[64];
    char lastName[64];
    char grade;
    char gender;
    char answers[NUM_OF_QUESTIONS][64];
    int gradeIds[10];
    int gradePoints[10];
    int allIds[10];
    int allPoints[10];
    Response* next;
};

void addMatch(int ids[10], int points[10], int id, int point) {
    for (int i = 0; i < 10; i++) {
        if (points[i] < point) {
            memmove(&ids[i+1], &ids[i], (10 - i - 1) * sizeof(int));
            ids[i] = id;
            memmove(&points[i+1], &points[i], (10 - i - 1) * sizeof(int));
            points[i] = point;
            ids[i] = id;
            points[i] = point;
            return;
        }
    }
}

void parseResponse(Response* response, char line[1024]) {
    const char girl[64] = "Girl";
    char token[64] = "";
    int lineIndex = 0;
    int tokenIndex = 0;
    int tokenCount = 0;
    while (line[lineIndex] != 0) {
        if (line[lineIndex] != ',' && line[lineIndex] != '\n') {
            token[tokenIndex] = line[lineIndex];
            tokenIndex++;
        } else {
            switch (tokenCount) {
                case EMAIL: case TIMESTAMP: break;
                case LAST_NAME: memcpy(response->lastName, token, 64); break;
                case FIRST_NAME: memcpy(response->firstName, token, 64); break;
                case GRADE: response->grade = atoi(token); break;
                case GENDER: response->gender = (strcmp(token, girl)); break;
                default: memcpy(response->answers[tokenCount - FIRST_QUESTION] , token, 64); break;
            }
            tokenCount++;
            memset(token, 0, 64);
            tokenIndex = 0;
        }
        lineIndex++;
    }
}

Response* parseFile(FILE* inFile) {
    char buffer[1024];
    Response* prev = nullptr;
    Response* head;
    int id = 1;
    fgets(buffer, 1024, inFile); // skip first line
    while (fgets(buffer, 1024, inFile)) {
        if (prev == nullptr) {
            head = (Response*)malloc(sizeof(Response));
            parseResponse(head, buffer);
            head->id = id;
            id++;
            prev = head;
            fgets(buffer, 1024, inFile);
        }
        prev->next = (Response*)malloc(sizeof(Response));
        prev->next->id = id;
        id++;
        parseResponse(prev->next, buffer);
        prev = prev->next;
    }
    return head;
}

void generateMatches(Response* responses) {
    Response* i = responses;
    Response* j;
    int points = 0;
    while (i != nullptr) {
        j = responses;
        while (j != nullptr) {
            if (i->id != j->id && i->gender != j->gender) {
                for (int k = 0; k < NUM_OF_QUESTIONS; k++) {
                    if (strcmp(i->answers[k], j->answers[k]) == 0) {
                        points++;
                    }
                }
                if (i->grade == j->grade) {
                    addMatch(i->gradeIds, i->gradePoints, j->id, points);
                }
                addMatch(i->allIds, i->allPoints, j->id, points);
            }
            points = 0;
            j = j->next;
        }
        i = i->next;
    }
}

Response* getResponseFromID(Response* responses, int id) {
    Response* response = responses;
    while (response != nullptr) {
        if (response->id == id) {
            return response;
        }
        response = response->next;
    }
    return nullptr;
}

void outputMatches(Response* responses, FILE* outFile) {
    char intBuf[10] = "";
    const char* row = "OWNER,TEN,NINE,EIGHT,SEVEN,SIX,FIVE,FOUR,THREE,TWO,ONE,OWNER2,TEN2,NINE2,EIGHT2,SEVEN2,SIX2,FIVE2,FOUR2,THREE2,TWO2,ONE2\n";
    fputs(row, outFile);

    Response* response = responses;
    while (response != nullptr) {
        fputs("\"", outFile);
        fputs(response->lastName, outFile);
        fputs(", ", outFile);
        fputs(response->firstName, outFile);
        fputs("   ", outFile);
        snprintf(intBuf, 10, "%i", response->grade);
        fputs(intBuf, outFile);
        fputs("\",", outFile);
        
        for (int i = 0; i < 10; i++) {
            Response* match;
            match = getResponseFromID(responses, response->allIds[i]);
            if (match == nullptr) {
                fputs("NULL,", outFile);
            } else {
                fputs("\"", outFile);
                fputs(match->lastName, outFile);
                fputs(", ", outFile);
                fputs(match->firstName, outFile);
                fputs("   ", outFile);
                snprintf(intBuf, 10, "%i", response->grade);
                fputs(intBuf, outFile);
                fputs("\",", outFile);
            }
        }
        
        for (int i = 0; i < 10; i++) {
            Response* match;
            match = getResponseFromID(responses, response->gradeIds[i]);
            if (match == nullptr) {
                fputs("NULL,", outFile);
            } else {
                fputs("\"", outFile);
                fputs(match->lastName, outFile);
                fputs(", ", outFile);
                fputs(match->firstName, outFile);
                fputs("   ", outFile);
                snprintf(intBuf, 10, "%i", response->grade);
                fputs(intBuf, outFile);
                fputs("\",", outFile);
            }
        }
        fputs("\n", outFile);
        
        response = response->next;
    }
}

void freeResponses(Response* responses) {
    Response* response = responses;
    Response* temp;
    while (response != nullptr) {
        temp = response->next;
        free(response);
        response = temp;
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Error: needs input and output file.\n");
        return 1;
    }

    FILE* inFile = fopen(argv[1], "r");
    FILE* outFile = fopen(argv[2], "w");

    Response* responses = parseFile(inFile);
    generateMatches(responses);
    outputMatches(responses, outFile);

    freeResponses(responses);
    fclose(inFile);
    fclose(outFile);
}