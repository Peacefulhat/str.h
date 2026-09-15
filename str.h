#ifndef SV_H
#define SV_H

//TODO: Error Handling and logging

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

#define FILE_READING 0

typedef int8_t    int8;
typedef uint8_t   uint8;
typedef int16_t   int16;
typedef uint16_t  uint16;
typedef int32_t   int32;
typedef int64_t   int64;
typedef uint32_t  uint32;
typedef uint64_t  uint64;
#define internal  static

#define TODO(message) do { fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); abort(); } while(0)
#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

// Dynamic array inital capacity
#ifndef DA_INIT_CAP
#define DA_INIT_CAP 256
#endif



//String Builder
typedef struct {
    char* Data;
    size_t Count;
    size_t Capacity;
} string_builder;

// String View
typedef struct {
    const char* Data;
    size_t Count;
} string_view;

// String Builder functions
bool ReadEntireFileAsSb(const char *FilePath, string_builder *Sb);
void SbAppendItem(string_builder* Sb, char Item);
void SbAppendManySB(string_builder* Dest, string_builder* Src);
void SbAppendManyCstr(string_builder* Sb, const char* Items);
void SbAppendBuffer(string_builder* Sb, const char* Buffer, size_t BufferSize);
void SbAppendSv(string_builder* Sb, string_view* Sv);
void SbAppendNull(string_builder* Sb);
void SbDataResize(string_builder* Sb, size_t NewSize);
char SbPopItem(string_builder* Sb);
char SbFirstItem(string_builder* Sb);
char SbLastItem(string_builder* Sb);
void SbFree(string_builder* Sb);;
void FreeSbData(string_builder* Sb);
const char* SbToCstr(string_builder* Sb);
void SbToSv(string_builder* Sb, string_view* Sv);

// String Views functions
string_view CstrAsSv(const char* Cstr);
string_view SvTrimLeft(string_view Sv);
string_view SvTrimRight(string_view Sv);
string_view SvTrim(string_view Sv);
string_view SvChopByDelim(string_view* Sv, char Delim);
string_view SvChopBySv(string_view* A, string_view* B);
bool SvChopPrefix(string_view *Sv, string_view Prefix);
bool SvChopSuffix(string_view *Sv, string_view Suffix);
string_view SvChopLeft(string_view* Sv, size_t N);
string_view SvChopRight(string_view* Sv, size_t N);
int SvEq(string_view A, string_view B);
int SvEqn(string_view A, string_view B);


#if FILE_READING
// File Reading
char* ReadEntireFile(char* FilePath);
#endif

#endif //

#ifdef STR_IMPLEMENTATION

// String Builder
#define SWAP(type, A, B) do { type T = A; A = B; B = T; } while (0)
internal int64 GetFileSize(FILE* File)
{
    if(!File) {
        return (1);
    }
    if(fseek(File, 0, SEEK_END) != 0) {
        return (1);
    }
    int64 Size = 0;
    if((Size = ftell(File)) == -1) {
        return (1);
    }
    fseek(File, 0, SEEK_SET);
    return (Size);
}

bool ReadEntireFileAsSb(const char *FilePath, string_builder *Sb)
{
    bool Result = false;
    size_t NewCount = 0;
    long long FileSize = 0;
    FILE *Fp = fopen(FilePath, "rb");
    FileSize = GetFileSize(Fp);
    if(FileSize == 1) {
        fprintf(stderr, "Unable to get the %s size\n", FilePath);
        return (Result);
    }
    NewCount = FileSize + Sb->Count;
    if(NewCount > Sb->Capacity) {
        Sb->Data = (char*)realloc(Sb->Data, NewCount);
        assert(Sb->Data != NULL && "Buy More RAM");
        Sb->Capacity = NewCount;
    }
    fread(Sb->Data + Sb->Count, FileSize, 1, Fp);
    if(ferror(Fp)) {
        fprintf(stderr, "Couldn't read the file %s", FilePath);
        return(Result);
    }
    Sb->Count = NewCount;
    return (!Result);
}

// Reserve memory for Data pointer in string_builder
internal void SbReserveMem(string_builder* Sb, uint32 ExpectedCapacity)
{
    if(Sb == NULL) {
        return;
    }
    if(Sb->Capacity == 0) {
        Sb->Capacity = DA_INIT_CAP;
    }
    while(ExpectedCapacity > Sb->Capacity) {
        Sb->Capacity *= 2;
    }
    Sb->Data = realloc(Sb->Data, Sb->Capacity * sizeof(*(Sb->Data)));
    assert(Sb->Data != NULL && "Buy more RAM");
}

void SbFree(string_builder* Sb)
{
    if(Sb == NULL) {
        return;
    }
    free(Sb);
}

void FreeSbData(string_builder* Sb)
{
    if(Sb == NULL) {
        return;
    }
    if(Sb->Data == NULL) {
        return;
    }
    free(Sb->Data);
    Sb->Count = 0;
    Sb->Capacity = 0;
}

// adds single element
void SbAppendItem(string_builder* Sb, char Item)
{
    if(Sb == NULL) {
        return;
    }
    SbReserveMem(Sb, Sb->Count + 1);
    Sb->Data[Sb->Count++] = Item;
}

void SbAppendManySB(string_builder* Dest, string_builder* Src)
{
    if(Dest == NULL || Src == NULL) {
        return;
    }
    SbReserveMem(Dest, Dest->Count + Src->Count);
    memcpy(Dest->Data + Dest->Count, Src->Data, Src->Count * sizeof(*(Dest->Data)));
    Dest->Count += Src->Count;
}

// Append a NULL-terminated string to a string builder
void SbAppendManyCstr(string_builder* Sb, const char* Items)
{
    if(Sb == NULL) {
        return;
    }
    size_t ItemsCount = strlen(Items);
    SbReserveMem(Sb, Sb->Count + (ItemsCount));
    memcpy(Sb->Data + Sb->Count, Items, ItemsCount * sizeof(*(Sb->Data)));
    Sb->Count += ItemsCount;
}


// Append a sized buffer to a string builder
void SbAppendBuffer(string_builder* Sb, const char* Buffer, size_t BufferSize)
{
    if(Sb == NULL) {
        return;
    }
    if(Buffer == NULL) {
        return;
    }
    SbReserveMem(Sb, Sb->Count + BufferSize);
    memcpy(Sb->Data + Sb->Count, Buffer, BufferSize * sizeof(*(Sb->Data)));
    Sb->Count += BufferSize;
}

// Append a string view to a string builder
void SbAppendSv(string_builder* Sb, string_view* Sv)
{
    if(Sb == NULL || Sv == NULL) {
        return;
    }
    SbAppendBuffer(Sb, Sv->Data, Sv->Count);
}

// Append Null at the end of sb Buffer
void SbAppendNull(string_builder* Sb)
{
    if(Sb == NULL) {
        return;
    }
    SbAppendItem(Sb, '\0');
}

void SbDataResize(string_builder* Sb, size_t NewSize)
{
    if(Sb == NULL) {
        return;
    }
    SbReserveMem(Sb, NewSize);
    Sb->Count = (NewSize);
}

char SbPopItem(string_builder* Sb)
{
    if(Sb == NULL) {
        return (0);
    }
    assert(Sb->Count > 0);
    return (Sb->Data[Sb->Count--]);
}

char SbFirstItem(string_builder* Sb)
{
    if(Sb == NULL) {
        return (0);
    }
    assert(Sb->Count > 0);
    return (Sb->Data[0]);
}


char SbLastItem(string_builder* Sb)
{
    if(Sb == NULL) {
        return (0);
    }
    assert(Sb->Count > 0);
    return (Sb->Data[Sb->Count - 1]);
}

const char* SbToCstr(string_builder* Sb)
{
    if(Sb == NULL) {
        // logging
        return NULL;
    }
    char* Cstr = (char*)malloc((Sb->Count + 1) * sizeof(char));
    assert(Cstr !=NULL && "Buy More Ram\n");
    memcpy(Cstr, Sb->Data, Sb->Count);
    Cstr[Sb->Count] = '\0';
    return(Cstr);
}

void SbToSv(string_builder* Sb, string_view* Sv)
{
    if(Sb == NULL || Sv == NULL) {
        return;
    }
    Sv->Count = Sb->Count;
    Sv->Data = Sb->Data;
}

#define PRINT_SV_OR_SBP(Str) printf("{\n%.*s\n}\n", (int32_t)(Str)->Count, (Str)->Data)
#define PRINT_SV_OR_SB(Str)  printf("{\n%.*s\n}\n", (int32_t)(Str).Count, (Str).Data)
#define PRINT_SV(Str)  printf("%.*s\n", (int32_t)(Str).Count, (Str).Data)

string_view CstrAsSv(const char* Cstr)
{
    return (string_view) {
        .Data = Cstr,
        .Count = strlen(Cstr)
    };
}

string_view SvTrimLeft(string_view Sv)
{
    int32 Index = 0;
    while(Index < Sv.Count && Sv.Data[Index] == ' ') {
        ++Index;

    }
    return (string_view) {
        .Data = Sv.Data + Index,
        .Count = Sv.Count - Index
    };
}

string_view SvTrimRight(string_view Sv)
{
    int32 Index = 0;
    while(Index < Sv.Count && isspace(Sv.Data[Sv.Count - 1 - Index])) {
        ++Index;
    }
    return (string_view) {
        .Data = Sv.Data,
        .Count = Sv.Count - Index
    };
}

string_view SvTrim(string_view Sv)
{
    return (SvTrimRight(SvTrimLeft(Sv)));
}

// chop By string_view or something
// not the best implementation but for now it good enough before i write optimized verison.
string_view SvChopBySv(string_view* A, string_view* B)
{
    string_view Result = {};
    if(A == NULL || B == NULL) {
        // logging
        // return empty string_view
    }
    uint32 Len = 0;

    uint32 Index = 0;
    //TODO: use strncmp()
    while(Index < A->Count) {
        if(A->Data[Index] == B->Data[0]) {
            Len++;
            for(uint32 Idx = 1; Idx < B->Count; ++Idx) {
                if(A->Data[Index + Idx] == B->Data[Idx]) {
                    Len++;
                }
            }
            if(Len == B->Count) {

                break;
            }
        }
        Len = 0;
        Index++;
    }
    Result = (string_view) {
        .Data = A->Data,
        .Count = Index
    };
    if(Len == B->Count) {
        A->Data += Index + B->Count ;
        A->Count -= Index + B->Count;
    } else {
        A->Data += Index;
        A->Count -= Index;
    }
    return (Result);
}

string_view SvChopByDelim(string_view* Sv, char Delim)
{
    if(Sv == NULL) {
        return (string_view) {
            .Data = NULL,
            .Count = 0
        };
    }
    int32 Index = 0;
    while(Index < Sv->Count && (Sv->Data[Index] != Delim )) {
        Index++;
    }
    string_view Result = {Sv->Data, Index};
    if(Index < Sv->Count) {
        Sv->Data+=Index + 1;
        Sv->Count-= Index + 1;
    } else {
        // return what ever there is left
        Sv->Count -= Index ;
        Sv->Data += Index ;
    }
    return (Result);
}


// remove any prefix and return true if it was choped,
//else return false and string_view remains unchanged.
bool SvChopPrefix(string_view *Sv, string_view Prefix)
{
    if(Sv == NULL) {
        return (false);
    }
    bool PrefixRem = false;
    if (SvEqn(*Sv, Prefix)) {
        SvChopLeft(Sv, Prefix.Count);
        PrefixRem = true;
    }
    return(PrefixRem);
}

// remove any suffix and return true if it was choped,
//else return false and string_view remains unchanged.

bool SvChopSuffix(string_view *Sv, string_view Suffix)
{
     if(Sv == NULL) {
        return (false);
    }
    bool SuffixRem = false;
    string_view Temp = *Sv;
    Temp.Data += Temp.Count - Suffix.Count;
    if (SvEqn(Temp, Suffix)) {
        SvChopRight(Sv, Suffix.Count);
        SuffixRem = true;
    }
    return(SuffixRem);
}

string_view SvChopLeft(string_view* Sv, size_t N)
{
    string_view Result= {};
    if(Sv == NULL) {
        // logging
        // Return empty string_view
    }
    if(N > Sv->Count) {
        N = Sv->Count;
    }
    Sv->Data += N;
    Sv->Count -= N;
    Result = *Sv;
    return (Result);
}

string_view SvChopRight(string_view *Sv, size_t N)
{
    string_view Result = {};
    if (N > Sv->Count) {
        N = Sv->Count;
    }
    Result.Data =  Sv->Data + Sv->Count - N ;
    Result.Count =  N;
    Sv->Count -= N;
    return (Result);
}

int SvEq(string_view A, string_view B)
{
    if(A.Count != B.Count) {
        // if length are not equal.
        return (-1);
    }
    return (strncmp(A.Data, B.Data, A.Count) == 0);
}

// checks first n characters of B in A
int SvEqn(string_view A, string_view B)
{
    if(A.Count <= B.Count) {
        return (strncmp(A.Data, B.Data, B.Count) == 0);
    }
    return (-1);
}


#if FILE_READING
// File Reading
char* ReadEntireFile(char* FilePath)
{
    FILE* File = fopen(FilePath, "r");
    if(!File) {
        return (NULL);
    }

    int64 Size = GetFileSize(File);
    char* FContents = (char*)malloc(Size + 1);

    if(fread(FContents, 1, Size, File) != Size) {
        return (NULL);
    }
    FContents[Size] = '\0';
    return (FContents);
}
#endif

#endif //
