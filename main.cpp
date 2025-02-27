#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Response {
    int id;
    char first_name[64];
    char last_name[64];
    char grade;
    char gender;
    char answers[21][64];
    int grade_ids[10];
    int grade_points[10];
    int all_ids[10];
    int all_points[10];
    Response* next;
};

void add_match(int ids[10], int points[10], int id, int point) {
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
    const char girl[64] = "Girl\x0D";
    char token[64] = "";
    int line_index = 0;
    int token_index = 0;
    int token_count = 0;
    while (line[line_index] != 0) {
        if (line[line_index] != ',' && line[line_index] != '\n') {
            token[token_index] = line[line_index];
            token_index++;
        } else {
            switch (token_count) {
                case 0: case 1: break;
                case 23: memcpy(response->last_name, token, 64); break;
                case 24: memcpy(response->first_name, token, 64); break;
                case 25: response->grade = atoi(token); break;
                case 26: response->gender = (strcmp(token, girl)); break;
                default: memcpy(response->answers[token_count - 2] , token, 64); break;
            }
            token_count++;
            memset(token, 0, 64);
            token_index = 0;
        }
        line_index++;
    }
}

Response* parseFile(FILE* inFile) { // very janky, fix later
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
                for (int k = 0; k < 21; k++) {
                    if (strcmp(i->answers[k], j->answers[k]) == 0) {
                        points++;
                    }
                }
                if (i->grade == j->grade) {
                    add_match(i->grade_ids, i->grade_points, j->id, points);
                }
                add_match(i->all_ids, i->all_points, j->id, points);
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
    const char* columnsStart = "ID,Last Name,First Name,Gender,";
    char intBuf[10] = "";
    fputs(columnsStart, outFile);
    for (int i = 1; i <= 10; i++) {
        fputs("Grade Match ", outFile);
        snprintf(intBuf, 10, "%i", i);
        fputs(intBuf, outFile);
        fputs(" Last Name,", outFile);

        fputs("Grade Match ", outFile);
        fputs(intBuf, outFile);
        fputs(" First Name,", outFile);
    }
    for (int i = 1; i <= 10; i++) {
        fputs("All Match ", outFile);
        snprintf(intBuf, 10, "%i", i);
        fputs(intBuf, outFile);
        fputs(" Last Name,", outFile);

        fputs("All Match ", outFile);
        fputs(intBuf, outFile);
        fputs(" First Name,", outFile);
    }
    fputs("\n", outFile);

    Response* response = responses;
    while (response != nullptr) {
        snprintf(intBuf, 10, "%i", response->id);
        fputs(intBuf, outFile);
        fputs(",", outFile);
        
        fputs(response->last_name, outFile);
        fputs(",", outFile);
        fputs(response->first_name, outFile);
        fputs(",", outFile);
        
        fputs(response->gender == 0 ? "Girl" : "Boy", outFile);
        fputs(",", outFile);
        
        for (int i = 0; i < 10; i++) {
            Response* match;
            match = getResponseFromID(responses, response->grade_ids[i]);
            if (match == nullptr) {
                fputs("NULL,NULL,", outFile);
            } else {
                fputs(match->last_name, outFile);
                fputs(",", outFile);
                fputs(match->first_name, outFile);
                fputs(",", outFile);
            }
        }

        for (int i = 0; i < 10; i++) {
            Response* match;
            match = getResponseFromID(responses, response->all_ids[i]);
            if (match == nullptr) {
                fputs("NULL,NULL,", outFile);
            } else {
                fputs(match->last_name, outFile);
                fputs(",", outFile);
                fputs(match->first_name, outFile);
                fputs(",", outFile);
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