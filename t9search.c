// File: t9search.c
// Subject: IZP
// Project: #1
// Author: Andrii Klymenko, FIT VUT
// Login: xklyme00
// Date: 16.6.2023

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 100
#define MAX_LINE_BUFFER_LENGTH MAX_LINE_LENGTH + 2 // + '\n' + '\0' == +2

// represents the arguments of the program
typedef struct {
    char searched_number[MAX_LINE_BUFFER_LENGTH]; // phone number where we will search for contacts
    char flag[3]; // switch ("-s", "-l")
    int levenshtein_distance; // maximum levenshtein distance between searched number and contact name/number/their representations
} Arguments_t;

// represents contact
typedef struct {
    char name[MAX_LINE_BUFFER_LENGTH]; // contact name
    char number_represent[MAX_LINE_BUFFER_LENGTH]; // number representation of contact name
    char number[MAX_LINE_BUFFER_LENGTH]; // contact number
    char number_with_plus[MAX_LINE_BUFFER_LENGTH]; // == number, but if number[0] == '+', number_with_plus[0] = '0'
} Contact_t;

void printUsage()
{
    printf("Usage:\n./t9search.\n./t9search NUMBER\n./t9search -s NUMBER\n./t9search NUMBER -l LEVENSTEIN_DISTANCE\n./t9search -l LEVENSTEIN_DISTANCE NUMBER\n");
}

void printErrAndUsage(char *err_msg)
{
    fprintf(stderr, "%s\n", err_msg);
    printUsage();
}

// check if the string contains a phone number
bool isNumber(char *str)
{
    for(int i = 0; str[i] != '\0'; i++)
    {
        // the first character of the phone number can be '+'
        if(i == 0 && str[i] == '+')
            continue;

        // all other characters must be digits 0-9
        if(!isdigit(str[i]))
            return false;
    }

    return true;
}

// convert contact name to its number representation
void nameToNumber(char *name, char *number_represent)
{
    int i;
    for(i = 0; name[i] != '\0'; i++)
    {
        // Each digit represents the following characters in addition to itself
        switch(name[i])
        {
            case 'a':
            case 'b':
            case 'c':
                number_represent[i] = '2'; // 2 (abc)
                break;

            case 'd':
            case 'e':
            case 'f':
                number_represent[i] = '3'; //3 (def)
                break;

            case 'g':
            case 'h':
            case 'i':
                number_represent[i] = '4'; // 4 (ghi)
                break;

            case 'j':
            case 'k':
            case 'l':
                number_represent[i] = '5'; // 5 (jkl)
                break;

            case 'm':
            case 'n':
            case 'o':
                number_represent[i] = '6'; // 6 (mno)
                break;

            case 'p':
            case 'q':
            case 'r':
            case 's':
                number_represent[i] = '7'; //  7 (pqrs)
                break;

            case 't':
            case 'u':
            case 'v':
                number_represent[i] = '8'; // 8 (tuv)
                break;

            case 'w':
            case 'x':
            case 'y':
            case 'z':
                number_represent[i] = '9'; // 9 (wxyz)
                break;

            case '+':
                number_represent[i] = '0'; // 0 (+)
                break;

            default: // if character is not lower case letter, just copy it
                number_represent[i] = name[i];
                break;
        }
    }
    number_represent[i] = '\0';
}

// check if searched number is in contact name/number/their representations as discontinuous sequence
// the program will also find contacts where
// between every two found characters there can be any sequence of other characters
bool searchDiscontinuousSequence(char *contact, char *searched_number)
{
    int searched_num_length = strlen(searched_number);
    int same_chars_count = 0;

    for(int i = 0; contact[i] != '\0'; i++)
    {
        // if characters are the same,
        // compare next character of the 'searched_number' in the next iteration
        if(contact[i] == *(searched_number + same_chars_count))
            same_chars_count++;

        if(same_chars_count == searched_num_length)
            return true;
    }

    return false;
}

// convert all possible characters of the string to lower case
void strToLower(char *str)
{
    for(int i = 0; str[i] != '\0'; i++)
        str[i] = tolower(str[i]);
}

// returns the smallest number of the three numbers
int min(int a, int b, int c)
{
    int tmp = a;

    if(b < tmp)
        tmp = b;

    if(c < tmp)
        tmp = c;

    return tmp;
}

// returns levenshtein distance between two strings
int getLevenshteinDistance(char *s1, char *s2)
{
    int s1_len = strlen(s1) + 1;
    int s2_len = strlen(s2) + 1;

    int matrix[s1_len][s2_len];

    for(int i = 0; i < s1_len; i++)
        matrix[i][0] = i;

    for(int j = 1; j < s1_len; j++)
        matrix[0][j] = j;

    for(int i = 1; i < s1_len; i++)
    {
        for(int j = 1; j < s2_len; j++)
        {
            int substitutionCost = 1;

            if(s1[i] == s2[j])
                substitutionCost = 0;

            matrix[i][j] = min(matrix[i - 1][j] + 1, matrix[i][j - 1] + 1, matrix[i - 1][j - 1] + substitutionCost);
        }
    }

    return matrix[s1_len - 1][s2_len - 1];
}

// check if contact name/number length is valid (<= 100 characters)
bool isValidLength(char *str)
{
    // if there is no '\n' character in the string
    // it means that contact name/number has more than 100 characters
    if(strchr(str, '\n') == NULL)
        return false;

    // replace '\n' with '\0' because we don't need it
    str[strcspn(str, "\n")] = '\0';
    return true;
}

int getContactAndCheckIt(Contact_t *contact)
{
    // read contact name
    if((fgets(contact->name, MAX_LINE_BUFFER_LENGTH, stdin)) != NULL)
    {
        // check its length
        if(!isValidLength(contact->name))
        {
            printErrAndUsage("Error! Contact's name is too long: its length exceeds 100 characters");
            return -1;
        }

        // the program is case-insensitive =>
        // for simplicity and convenience work only with lower case letters
        strToLower(contact->name);

        nameToNumber(contact->name, contact->number_represent);
    }
    else  // if EOF is reached return 0
        return 0;

    // read contact number
    if((fgets(contact->number, MAX_LINE_BUFFER_LENGTH, stdin)) != NULL)
    {
        // check its length
        if(!isValidLength(contact->number))
        {
            printErrAndUsage("Error! Contact's phone number is too long: its length exceeds 100 characters");
            return -1;
        }

        if(!isNumber(contact->number))
        {
            printErrAndUsage("Error! Invalid number format: it must contain only "
                             "a sequence of digits 0-9");
            return -1;
        }

        strcpy(contact->number_with_plus, contact->number);

        // '+' represents '0'
        if(contact->number_with_plus[0] == '+')
            contact->number_with_plus[0] = '0';
    }
    else // if EOF is reached return 0
        return 0;

    return 1;
}

// check if contact needs to be printed (it matches the searched number)
bool matchContact(char *flag, char *contact, char *searched_number, int levenshtein_distance)
{
    // if no arguments provided => all contacts with valid name and number are matched
    if(searched_number[0] == '\0' && flag[0] == '\0')
        return true;

    // if no flag was provided as program argument => we search only by phone number
    // check if searched number is substring of contact name/number/their representations
    if(flag[0] == '\0')
        return (strstr(contact, searched_number) != NULL);

    // if flag is "-s", the program will also find contacts
    // where between every two found characters there can be any sequence of other characters
    if(strcmp(flag, "-s") == 0)
        return (searchDiscontinuousSequence(contact, searched_number));

    // if flag is "-l"
    // check if levenshtein distance between searched number and name/number/their representations
    // is less or equal to specified maximum levenshtein distance
    return (getLevenshteinDistance(contact, searched_number) - (int)(strlen(contact) - strlen(searched_number)) <= levenshtein_distance);
}

bool processContacts(Arguments_t *arguments)
{
    if(arguments->searched_number[0] == '+')
        arguments->searched_number[0] = '0';

    Contact_t contact;

    bool isContactPrinted = false; // was at least one contact written out?
    int result; // stores a return value of 'getContactAndCheckIt' function

    // while contact name/number is valid and EOF is not reached
    while((result = getContactAndCheckIt(&contact)) == 1)
    {
        // if the contact matches the searched number, print it and mention that contact was printed
        if(matchContact(arguments->flag, contact.number_with_plus, arguments->searched_number, arguments->levenshtein_distance) ||
            matchContact(arguments->flag, contact.number_represent, arguments->searched_number, arguments->levenshtein_distance))
        {
            printf("%s, %s\n", contact.name, contact.number);
            isContactPrinted = true;
        }
    }

    // if some error has occurred (invalid length, not a number)
    if(result == -1)
        return false;

    // if no contact was printed
    if(!isContactPrinted)
        printf("Not found\n");

    return true;
}

// check if string contains a phone number
// and whether its length is less or equal to the permissible length (100 characters)
// if so, copy it to string 'dst' and return true
bool isValidNumber(char *searched_number, char *dst)
{
    if(strlen(searched_number) > MAX_LINE_LENGTH || !isNumber(searched_number))
    {
        printErrAndUsage("Error! The argument NUMBER should consist only of the sequence of digits 0-9 "
                         "(the first character may be '+') and its length should not exceed 100 characters");
        return false;
    }

    strcpy(dst, searched_number);
    return true;
}

// check if there is an integer in the string
// if so, assign a value to the *Levenshtein_distance
bool isInteger(char *str, int *Levenshtein_distance)
{
    // check if string contains only the digits 0-9 as characters
    if(!isdigit(str[0]) || !isNumber(str))
    {
        printErrAndUsage("Error! The Levenshtein distance must be a non-negative integer\n");
        return false;
    }

    *Levenshtein_distance = atoi(str);

    // if the length of the string is greater than 3 => this means that its integer value is greater than 1000
    // in this case assign *Levenshtein_distance to 101
    // because if we have 2 strings with a maximum length of 100
    // then the maximum Levenshtein distance between them is 100
    if(strlen(str) > 3)
        *Levenshtein_distance = 101;

    return true;
}

// parses program arguments
// returns true if arguments are valid, otherwise returns false
bool parseArguments(int argc, char *argv[], Arguments_t *arguments)
{
    // in the 'worst' case program can be run with these arguments:
    // ./t9search -l L NUMBER
    // no more arguments are needed
    if(argc > 4)
    {
        printErrAndUsage("Error! Too many arguments: in the \"worst\" case, the program needs 3 arguments to run");
        return false;
    }

    // ./t9search
    // no arguments provided => nothing to check (just print all contacts) => return true
    if(argc == 1)
        return true;

    // ./t9search NUMBER
    if(argc == 2)
        return isValidNumber(argv[1], arguments->searched_number);

    // ./t9search -s NUMBER
    if(argc == 3 && isValidNumber(argv[2], arguments->searched_number))
    {
        if(strcmp(argv[1], "-s") != 0)
        {
            printErrAndUsage("Error! Invalid first program's argument: expected switch '-s'");
            return false;
        }

        // copy switch "-s" to arguments->flag
        strcpy(arguments->flag, argv[1]);
        return true;
    }

    if(argc == 4)
    {
        // ./t9search -l L NUMBER
        if(strcmp(argv[1], "-l") == 0)
        {
            if(isInteger(argv[2], &arguments->levenshtein_distance) && isValidNumber(argv[3], arguments->searched_number))
            {
                // copy switch "-l" to arguments->flag
                strcpy(arguments->flag, argv[1]);
                return true;
            }
        }

        // ./t9search NUMBER -l L
        else if(strcmp(argv[2], "-l") == 0)
        {
            if(isInteger(argv[3], &arguments->levenshtein_distance) && isValidNumber(argv[1], arguments->searched_number))
            {
                // copy switch "-l" to arguments->flag
                strcpy(arguments->flag, argv[2]);
                return true;
            }
        }
        else  // if switch "-l" is a third program argument => it hasn't a value
            printErrAndUsage("Error! Invalid program's argument: expected switch '-l'");
    }

    return false;
}

int main(int argc, char *argv[])
{
    Arguments_t arguments = {.searched_number = "", .flag = "", .levenshtein_distance = -1};

    if(!parseArguments(argc, argv, &arguments))
        return -1;

    if(processContacts(&arguments))
        return 0;

    return -1;
}