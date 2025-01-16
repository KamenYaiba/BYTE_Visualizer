#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

bool bytesToString2(char *buffer, int *bufferPos, uint8_t *bytes, int nBytes);
bool bytesToString16(char *buffer, int *bufferPos, uint8_t *bytes, int nBytes);
int visualizeBytes(char *fileName, char mode);
char* changeExtension(char *fileName, char *ex);
bool stringToBytes2(uint8_t *buffer, int *bufferPos, char *str, int len);
bool stringToBytes16(uint8_t *buffer, int *bufferPos, char *str, int len);
int restoreFile(char *fileName, char *extension);
int getHexDigit(char c);


int main()
{
    //visualizeBytes("login.res", 'H');
    //restoreFile("login.b16", "od");
    char buf[128];
    getcwd(buf, sizeof(buf));
    printf("%s", buf);
}


int visualizeBytes(char *fileName, char mode)
{
    bool (*bytesToStrFunc)(char*, int*, uint8_t*, int);
    char *extension;
    switch(mode)
    {
        case 'h':
        case 'H':
            bytesToStrFunc = &bytesToString16;
            extension = "b16";
            break;
        default:
            bytesToStrFunc = &bytesToString2;
            extension = "b2";
    }

    FILE *src = fopen(fileName, "rb");
    if(src == NULL) return -1;
    FILE *des = fopen(changeExtension(fileName, extension), "w");
    if(des == NULL)
    {
        fclose(src);
        return -2;
    }

    uint8_t *bytes = malloc(2048);
    char *buffer = malloc(18432);
    if(bytes == NULL || buffer == NULL) return -3;

    int bufferPos = 0;
    int bytesRead;
    while((bytesRead = fread(bytes, sizeof(uint8_t), 2048, src)) != 0)
    {
        bytesToStrFunc(buffer, &bufferPos, bytes, bytesRead);
        fwrite(buffer, sizeof(char), bufferPos, des);
        bufferPos = 0;
    }
    fclose(src);
    fclose(des);
    free(bytes);
    free(buffer);
    return 0;
}


bool bytesToString2(char *buffer, int *bufferPos, uint8_t *bytes, int nBytes)
{
    if(buffer == NULL || bufferPos == NULL || bytes == NULL || nBytes > 2048) return false;

    char *halfBytes[] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
                         "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"};
    uint8_t byte;
    static uint8_t lineLen = 0;
    for(int i = 0; i < nBytes; i++)
    {
        byte = bytes[i];
        for(int j = 0; j < 4; j++)  
            buffer[(*bufferPos)++] = halfBytes[byte >> 4][j];
        for(int j = 0; j < 4; j++)
        {
            byte = (byte << 4);
            byte = (byte >> 4);
            buffer[(*bufferPos)++] = halfBytes[byte][j];
        }
            
        lineLen++;
        if(lineLen == 12)
        {
            lineLen = 0;
            buffer[(*bufferPos)++] = '\n';
        }
        else
            buffer[(*bufferPos)++] = ' ';
    }
}


bool bytesToString16(char *buffer, int *bufferPos, uint8_t *bytes, int nBytes)
{
    if(buffer == NULL || bufferPos == NULL || bytes == NULL || nBytes > 2048) return false;

    char halfBytes[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    uint8_t byte;
    static uint8_t lineLen = 0;
    for(int i = 0; i < nBytes; i++)
    {
        byte = bytes[i];
        buffer[(*bufferPos)++] = halfBytes[byte >> 4];

        byte = (byte << 4);
        byte = (byte >> 4);
        buffer[(*bufferPos)++] = halfBytes[byte];
            
        lineLen++;
        if(lineLen == 32)
        {
            lineLen = 0;
            buffer[(*bufferPos)++] = '\n';
        }
        else
            buffer[(*bufferPos)++] = ' ';  
    }
}


int restoreFile(char *fileName, char* extension)
{
    bool (*strToBytesFunc)(uint8_t *buffer, int *bufferPos, char *str, int len);
    char type = fileName[strlen(fileName)-1];
    switch(type)
    {
        case '2':
            strToBytesFunc = &stringToBytes2;
            break;
        case '6':
            strToBytesFunc = &stringToBytes16;
    }
    FILE *src = fopen(fileName, "r");
    if(src == NULL) return -1;
    FILE *des = fopen(changeExtension(fileName, extension), "wb");
    if(des == NULL)
    {
        fclose(src);
        return -2;
    } 

    char *str = malloc(18432);
    uint8_t *buffer = malloc(2048);
    
    int bufferPos = 0;
    int charsRead = 0;
    while((charsRead = fread(str, sizeof(char), 18432, src)) != 0)
    {
        stringToBytes16(buffer, &bufferPos, str, charsRead);
        fwrite(buffer, sizeof(char), bufferPos, des);
        bufferPos = 0;
    }
    fclose(src);
    fclose(des);
    free(str);
    free(buffer);
    return 0;
}


bool stringToBytes2(uint8_t *buffer, int *bufferPos, char *str, int len)
{
    if(buffer == NULL || bufferPos == NULL || str == NULL || len > 18432) return false;
    uint8_t byte;
    uint8_t b;
    char c;
    for(int i = 0; i < len; i++)
    {
        byte = 0;
        b = 128;
        for(int j = 0; j < 8; j++)
        {
            c = str[i++];
            if(c == '1')
                byte = byte | b;
            b = b >> 1;
        }
        buffer[(*bufferPos)++] = byte;
    }
    return true;
}


bool stringToBytes16(uint8_t *buffer, int *bufferPos, char *str, int len)
{
    if(buffer == NULL || bufferPos == NULL || str == NULL || len > 18432) return false;
    uint8_t byte;
    uint8_t b;
    char c;
    uint8_t halfBytes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    for(int i = 0; i < len; i++)
    {
        byte = 0;

        c = str[i++];
        byte = byte | halfBytes[getHexDigit(c)];
        byte = byte << 4;
        
        c = str[i++];
        byte = byte | halfBytes[getHexDigit(c)];

        buffer[(*bufferPos)++] = byte;
    }
    return true;
}


char* changeExtension(char *fileName, char *ex)
{
    if(fileName == NULL || ex == NULL) return "DEFAULT.defualt";

    char c;
    int len;
    for(len = 0; ((c = fileName[len]) != '.' && c!='\0'); len++);
    
    int exLen = strlen(ex);
    char *newFileName = malloc((len + exLen + 1));

    int idx = 0;
    for(int i = 0; i < len; i++)
        newFileName[i] = fileName[idx++];

    newFileName[idx++] = '.';

    for(int i = 0; i < exLen; i++)
    {
        newFileName[idx++] = ex[i];
    }
    newFileName[idx] = '\0';

    return newFileName;
}


int getHexDigit(char c)
{
    int d = c - '0';
    if(d < 0) return -1;
    if(d > 9) d -= 7;
    return d;
}