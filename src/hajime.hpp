#pragma once

#include <stdint.h>
#include <stdio.h>
/*
    NOTE(matthias): The format and style of this source is as follows:
	Tab width is four spaces. Scope-braces start on a new-line. Line-width should
	not excede 80 characters. If fuction declarations, function definitions,
	function calls, if-statments, while-statments, or for-statments go past 80
	characters, each item between the parentheses should be put on a new-line.

	Style-wise, all namespaces and #defines should be all caps. Structs, Classes,
	Enums, user types ,functions and methods should be Upper_snake_cased. Member
	and Global variables should beUpperCamelCased. local variables should be
	camelCased.
*/

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float r32;
typedef double r64;

typedef size_t Index;

typedef u8 b8;

// NOTE(matthias): stolen from https://github.com/gpakosz/Assert
#if defined(_WIN32)
#  define debug_break() __debugbreak()
#elif defined(__ORBIS__)
#  define debug_break() __builtin_trap()
#elif defined(__clang__)
#  define debug_break() __builtin_debugtrap()
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__APPLE__)
#  include <signal.h>
#  define debug_break() raise(SIGTRAP)
#elif defined(__GNUC__)
#  define debug_break() __builtin_trap()
#else
#  define debug_break() ((void)0)
#endif

#define debug(fmt, ...) fprintf(stderr, "%s:%d: " fmt "\n", __func__, __LINE__, __VA_ARGS__)

inline void Assert(bool x, const char* reason)
{
	if (!x)
	{
		fprintf(stderr,"%s", reason);
		debug_break();
	}
}

#define CLAMP(x, min, max) ((x<min)||(x>max))?((x<min)? (min) : (max)):(x)
#define ARRAYCOUNT(a) ((sizeof(a))/(sizeof(a[0])))

// CAUTION(matthias): DO NOT USE WITH ARRAYS OF UNKNOWN SIZE.
// DOCUMENTATION(matthias): Move data from one memory pointer to another of size.
void* Memory_Move(void* destination, const void* source, Index size)
{
    u8* to = (u8*)destination;
    u8* from = (u8*)source;

    for(Index i=0; i<size; i++)
    {
        *to++ = *from++;
    }
    return destination;
}

// CAUTION(matthias): DO NOT USE WITH ARRAYS OF UNKNOWN SIZE.
// DOCUMENTATION(matthias): Set memory to value for size.
void Memory_Set(void* source, u8 value, Index size)
{
    u8* result = (u8*)source;
    for(Index i=0; i<size; i++)
    {
        *result++ = (u8)value;
    }
}



#include <cmath>
#include <cstdlib>

#define HAJIME_DEFAULT_ARRAY_SIZE 1
#define HAJIME_ARRAY_GROWTH 1.5
template<typename T, u16 size>
struct StaticArray
{
    union {
        T* Data;
        u8* _data;
    };
    i32 DataSize;

    StaticArray() {
        DataSize = HAJIME_DEFAULT_ARRAY_SIZE;
        Data = (T*)calloc(size, sizeof(T));
    }

    ~StaticArray() { free(Data); }

    const T& operator[](i32 i) const {
        Assert(i >= 0 && i < size, "Index out of Bounds!");
        return *(Data + i);
    }

    T& operator[](i32 i) {
        Assert(i >= 0 && i < size, "Index out of Bounds!");
        return *(Data + i);
    }

    void Resize(i32 s) {
        Assert(s > DataSize && s < size);
        DataSize = s;
    }

    void Remove(i32 i) {
        Assert(i >= 0 && i < DataSize);
        if (i != (DataSize - 1)) {
            Data[i] = Data[DataSize - 1];
        }
        DataSize--;
    }

    void Remove_Ordered(i32 i) {
        Assert(i >= 0 && i < DataSize);
        if (i != (DataSize - 1)) {
            Memory_Move((Data + i), (Data + i + 1), (DataSize - (i - 1))*sizeof(T));
        }
        DataSize--;
    }

    T& Add() {
        Resize(DataSize+1);
        return Data[DataSize - 1];
    }

    T& Add(T item) {
        Add();
        return Data[DataSize - 1] = item;
    }

    T& Insert(i32 i) {
        Resize(DataSize+1);
        Memory_Move((Data + i + 1), (Data + i), (DataSize - i)*sizeof(T));
        return Data[i];
    }

    T& Insert(i32 i, T item) {
        Insert(i);
        return Data[i] = item;
    }
};

template<typename T>
struct Array
{
    union {
        T* Data;
        u8* _data;
    };
    i32 Size;
    i32 Reserve;

    Array() {
        Reserve = 1;
        Size = 0;
        Data = (T*)calloc(Reserve, sizeof(T));
    }

    ~Array() {}// if(Data) free(Data); }

    const T& operator[](i32 i) const {
        Assert(i >= 0 && i < Size);
        return *(Data + i);
    }

    T& operator[](i32 i) {
        Assert(i >= 0 && i < Size, "Index out of range\n");
        return *(Data + i);
    }

    void Reserve_Memory(i32 s) {
        Assert(s > Reserve, "s is too small\n");
        u32 newSize = (u32)pow(HAJIME_ARRAY_GROWTH, (i32)(log(s) / log(HAJIME_ARRAY_GROWTH)) + 1);

        Data = (T*)realloc(Data, sizeof(T) * newSize);
        Memory_Set((Data + Size), 0, newSize - Size);
        Reserve = newSize;
    }

    void Resize(i32 s) {
        Assert(s > Size, "s is too small\n");

        if (s <= Reserve) {
            Size = s;
        }
        else {
            Reserve_Memory(s);
            Size = s;

        }
    }

    void Remove(i32 i) {
        Assert(i >= 0 && i < Size, "Indexe out of range!\n");
        if (i != (Size - 1)) {
            Data[i] = Data[Size - 1];
        }
        Size--;
    }

    void Remove_Ordered(i32 i) {
        Assert(i >= 0 && i < Size, "Index out of range!\n");
        if (i != (Size - 1) || (Size != 1)) {
            Memory_Move((Data + i), (Data + i + 1), (Size - (i+1))*sizeof(T));
        }
        Size--;
    }

    T& Add() {
        Resize(Size+1);
        return Data[Size - 1];
    }

    T& Add(T item) {
        Add();
        return Data[Size - 1] = item;
    }

    T& Insert(i32 i) {
        Resize(Size+1);
        Memory_Move((Data + i + 1), (Data + i), (Size - i)*sizeof(T));
        return Data[i];
    }

    T& Insert(i32 i, T item) {
        Insert(i);
        return Data[i] = item;
    }

};

template<typename T>
struct Queue : public Array<T>
{
    Queue() : Array<T>() {}

    void Push(T item)
		{
			this->Add(item);
		}

    T Pop()
		{
			if (this->Size == 0) Assert(false, "Queue Empty!");

			T result = this->Data[0];
			this->Remove_Ordered(0);
			return result;
		}
};
