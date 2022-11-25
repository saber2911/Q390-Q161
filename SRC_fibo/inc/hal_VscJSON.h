/*
  Copyright (c) 2009-2017 Dave Gamble and Vs_cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef Vs_cJSON__h
#define Vs_cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 3 define options:

Vs_CJSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
Vs_CJSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
Vs_CJSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the Vs_CJSON_API_VISIBILITY flag to "export" the same symbols the way Vs_CJSON_EXPORT_SYMBOLS does

*/

#define Vs_CJSON_CDECL __cdecl
#define Vs_CJSON_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(Vs_CJSON_HIDE_SYMBOLS) && !defined(Vs_CJSON_IMPORT_SYMBOLS) && !defined(Vs_CJSON_EXPORT_SYMBOLS)
#define Vs_CJSON_EXPORT_SYMBOLS
#endif

#if defined(Vs_CJSON_HIDE_SYMBOLS)
#define Vs_CJSON_PUBLIC(type)   type Vs_CJSON_STDCALL
#elif defined(Vs_CJSON_EXPORT_SYMBOLS)
#define Vs_CJSON_PUBLIC(type)   __declspec(dllexport) type Vs_CJSON_STDCALL
#elif defined(Vs_CJSON_IMPORT_SYMBOLS)
#define Vs_CJSON_PUBLIC(type)   __declspec(dllimport) type Vs_CJSON_STDCALL
#endif
#else /* !__WINDOWS__ */
#define Vs_CJSON_CDECL
#define Vs_CJSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(Vs_CJSON_API_VISIBILITY)
#define Vs_CJSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define Vs_CJSON_PUBLIC(type) type
#endif
#endif

/* project version */
#define Vs_CJSON_VERSION_MAJOR 1
#define Vs_CJSON_VERSION_MINOR 7
#define Vs_CJSON_VERSION_PATCH 15

#include <stddef.h>

/* Vs_cJSON Types: */
#define Vs_cJSON_Invalid (0)
#define Vs_cJSON_False  (1 << 0)
#define Vs_cJSON_True   (1 << 1)
#define Vs_cJSON_NULL   (1 << 2)
#define Vs_cJSON_Number (1 << 3)
#define Vs_cJSON_String (1 << 4)
#define Vs_cJSON_Array  (1 << 5)
#define Vs_cJSON_Object (1 << 6)
#define Vs_cJSON_Raw    (1 << 7) /* raw json */

#define Vs_cJSON_IsReference 256
#define Vs_cJSON_StringIsConst 512

/* The Vs_cJSON structure: */
typedef struct Vs_cJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct Vs_cJSON *next;
    struct Vs_cJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct Vs_cJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==Vs_cJSON_String  and type == Vs_cJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use Vs_cJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==Vs_cJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} Vs_cJSON;

typedef struct Vs_cJSON_Hooks
{
      /* malloc/free are CDECL on Windows regardless of the default calling convention of the compiler, so ensure the hooks allow passing those functions directly. */
      void *(Vs_CJSON_CDECL *malloc_fn)(size_t sz);
      void (Vs_CJSON_CDECL *free_fn)(void *ptr);
} Vs_cJSON_Hooks;

typedef int Vs_cJSON_bool;

/* Limits how deeply nested arrays/objects can be before Vs_cJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef Vs_CJSON_NESTING_LIMIT
#define Vs_CJSON_NESTING_LIMIT 1000
#endif

/* returns the version of Vs_cJSON as a string */
Vs_CJSON_PUBLIC(const char*) Vs_cJSON_Version(void);

/* Supply malloc, realloc and free functions to Vs_cJSON */
Vs_CJSON_PUBLIC(void) Vs_cJSON_InitHooks(Vs_cJSON_Hooks* hooks);

/* Memory Management: the caller is always responsible to free the results from all variants of Vs_cJSON_Parse (with Vs_cJSON_Delete) and Vs_cJSON_Print (with stdlib free, Vs_cJSON_Hooks.free_fn, or Vs_cJSON_free as appropriate). The exception is Vs_cJSON_PrintPreallocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a Vs_cJSON object you can interrogate. */
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_Parse(const char *value);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_ParseWithLength(const char *value, size_t buffer_length);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match Vs_cJSON_GetErrorPtr(). */
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_ParseWithOpts(const char *value, const char **return_parse_end, Vs_cJSON_bool require_null_terminated);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, Vs_cJSON_bool require_null_terminated);

/* Render a Vs_cJSON entity to text for transfer/storage. */
Vs_CJSON_PUBLIC(char *) Vs_cJSON_Print(const Vs_cJSON *item);
/* Render a Vs_cJSON entity to text for transfer/storage without any formatting. */
Vs_CJSON_PUBLIC(char *) Vs_cJSON_PrintUnformatted(const Vs_cJSON *item);
/* Render a Vs_cJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
Vs_CJSON_PUBLIC(char *) Vs_cJSON_PrintBuffered(const Vs_cJSON *item, int prebuffer, Vs_cJSON_bool fmt);
/* Render a Vs_cJSON entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
/* NOTE: Vs_cJSON is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_PrintPreallocated(Vs_cJSON *item, char *buffer, const int length, const Vs_cJSON_bool format);
/* Delete a Vs_cJSON entity and all subentities. */
Vs_CJSON_PUBLIC(void) Vs_cJSON_Delete(Vs_cJSON *item);

/* Returns the number of items in an array (or object). */
Vs_CJSON_PUBLIC(int) Vs_cJSON_GetArraySize(const Vs_cJSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_GetArrayItem(const Vs_cJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_GetObjectItem(const Vs_cJSON * const object, const char * const string);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_GetObjectItemCaseSensitive(const Vs_cJSON * const object, const char * const string);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_HasObjectItem(const Vs_cJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when Vs_cJSON_Parse() returns 0. 0 when Vs_cJSON_Parse() succeeds. */
Vs_CJSON_PUBLIC(const char *) Vs_cJSON_GetErrorPtr(void);

/* Check item type and return its value */
Vs_CJSON_PUBLIC(char *) Vs_cJSON_GetStringValue(const Vs_cJSON * const item);
Vs_CJSON_PUBLIC(double) Vs_cJSON_GetNumberValue(const Vs_cJSON * const item);

/* These functions check the type of an item */
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_IsInvalid(const Vs_cJSON * const item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_IsFalse(const Vs_cJSON * const item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_IsTrue(const Vs_cJSON * const item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_IsBool(const Vs_cJSON * const item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_IsNull(const Vs_cJSON * const item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_IsNumber(const Vs_cJSON * const item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_IsString(const Vs_cJSON * const item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_IsArray(const Vs_cJSON * const item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_IsObject(const Vs_cJSON * const item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_IsRaw(const Vs_cJSON * const item);

/* These calls create a Vs_cJSON item of the appropriate type. */
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateNull(void);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateTrue(void);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateFalse(void);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateBool(Vs_cJSON_bool boolean);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateNumber(double num);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateString(const char *string);
/* raw json */
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateRaw(const char *raw);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateArray(void);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateObject(void);

/* Create a string where valuestring references a string so
 * it will not be freed by Vs_cJSON_Delete */
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateStringReference(const char *string);
/* Create an object/array that only references it's elements so
 * they will not be freed by Vs_cJSON_Delete */
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateObjectReference(const Vs_cJSON *child);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateArrayReference(const Vs_cJSON *child);

/* These utilities create an Array of count items.
 * The parameter count cannot be greater than the number of elements in the number array, otherwise array access will be out of bounds.*/
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateIntArray(const int *numbers, int count);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateFloatArray(const float *numbers, int count);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateDoubleArray(const double *numbers, int count);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_CreateStringArray(const char *const *strings, int count);

/* Append item to the specified array/object. */
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_AddItemToArray(Vs_cJSON *array, Vs_cJSON *item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_AddItemToObject(Vs_cJSON *object, const char *string, Vs_cJSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the Vs_cJSON object.
 * WARNING: When this function was used, make sure to always check that (item->type & Vs_cJSON_StringIsConst) is zero before
 * writing to `item->string` */
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_AddItemToObjectCS(Vs_cJSON *object, const char *string, Vs_cJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing Vs_cJSON to a new Vs_cJSON, but don't want to corrupt your existing Vs_cJSON. */
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_AddItemReferenceToArray(Vs_cJSON *array, Vs_cJSON *item);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_AddItemReferenceToObject(Vs_cJSON *object, const char *string, Vs_cJSON *item);

/* Remove/Detach items from Arrays/Objects. */
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_DetachItemViaPointer(Vs_cJSON *parent, Vs_cJSON * const item);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_DetachItemFromArray(Vs_cJSON *array, int which);
Vs_CJSON_PUBLIC(void) Vs_cJSON_DeleteItemFromArray(Vs_cJSON *array, int which);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_DetachItemFromObject(Vs_cJSON *object, const char *string);
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_DetachItemFromObjectCaseSensitive(Vs_cJSON *object, const char *string);
Vs_CJSON_PUBLIC(void) Vs_cJSON_DeleteItemFromObject(Vs_cJSON *object, const char *string);
Vs_CJSON_PUBLIC(void) Vs_cJSON_DeleteItemFromObjectCaseSensitive(Vs_cJSON *object, const char *string);

/* Update array items. */
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_InsertItemInArray(Vs_cJSON *array, int which, Vs_cJSON *newitem); /* Shifts pre-existing items to the right. */
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_ReplaceItemViaPointer(Vs_cJSON * const parent, Vs_cJSON * const item, Vs_cJSON * replacement);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_ReplaceItemInArray(Vs_cJSON *array, int which, Vs_cJSON *newitem);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_ReplaceItemInObject(Vs_cJSON *object,const char *string,Vs_cJSON *newitem);
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_ReplaceItemInObjectCaseSensitive(Vs_cJSON *object,const char *string,Vs_cJSON *newitem);

/* Duplicate a Vs_cJSON item */
Vs_CJSON_PUBLIC(Vs_cJSON *) Vs_cJSON_Duplicate(const Vs_cJSON *item, Vs_cJSON_bool recurse);
/* Duplicate will create a new, identical Vs_cJSON item to the one you pass, in new memory that will
 * need to be released. With recurse!=0, it will duplicate any children connected to the item.
 * The item->next and ->prev pointers are always zero on return from Duplicate. */
/* Recursively compare two Vs_cJSON items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
Vs_CJSON_PUBLIC(Vs_cJSON_bool) Vs_cJSON_Compare(const Vs_cJSON * const a, const Vs_cJSON * const b, const Vs_cJSON_bool case_sensitive);

/* Minify a strings, remove blank characters(such as ' ', '\t', '\r', '\n') from strings.
 * The input pointer json cannot point to a read-only address area, such as a string constant, 
 * but should point to a readable and writable address area. */
Vs_CJSON_PUBLIC(void) Vs_cJSON_Minify(char *json);

/* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
Vs_CJSON_PUBLIC(Vs_cJSON*) Vs_cJSON_AddNullToObject(Vs_cJSON * const object, const char * const name);
Vs_CJSON_PUBLIC(Vs_cJSON*) Vs_cJSON_AddTrueToObject(Vs_cJSON * const object, const char * const name);
Vs_CJSON_PUBLIC(Vs_cJSON*) Vs_cJSON_AddFalseToObject(Vs_cJSON * const object, const char * const name);
Vs_CJSON_PUBLIC(Vs_cJSON*) Vs_cJSON_AddBoolToObject(Vs_cJSON * const object, const char * const name, const Vs_cJSON_bool boolean);
Vs_CJSON_PUBLIC(Vs_cJSON*) Vs_cJSON_AddNumberToObject(Vs_cJSON * const object, const char * const name, const double number);
Vs_CJSON_PUBLIC(Vs_cJSON*) Vs_cJSON_AddStringToObject(Vs_cJSON * const object, const char * const name, const char * const string);
Vs_CJSON_PUBLIC(Vs_cJSON*) Vs_cJSON_AddRawToObject(Vs_cJSON * const object, const char * const name, const char * const raw);
Vs_CJSON_PUBLIC(Vs_cJSON*) Vs_cJSON_AddObjectToObject(Vs_cJSON * const object, const char * const name);
Vs_CJSON_PUBLIC(Vs_cJSON*) Vs_cJSON_AddArrayToObject(Vs_cJSON * const object, const char * const name);

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define Vs_cJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
/* helper for the Vs_cJSON_SetNumberValue macro */
Vs_CJSON_PUBLIC(double) Vs_cJSON_SetNumberHelper(Vs_cJSON *object, double number);
#define Vs_cJSON_SetNumberValue(object, number) ((object != NULL) ? Vs_cJSON_SetNumberHelper(object, (double)number) : (number))
/* Change the valuestring of a Vs_cJSON_String object, only takes effect when type of object is Vs_cJSON_String */
Vs_CJSON_PUBLIC(char*) Vs_cJSON_SetValuestring(Vs_cJSON *object, const char *valuestring);

/* Macro for iterating over an array or object */
#define Vs_cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* malloc/free objects using the malloc/free functions that have been set with Vs_cJSON_InitHooks */
Vs_CJSON_PUBLIC(void *) Vs_cJSON_malloc(size_t size);
Vs_CJSON_PUBLIC(void) Vs_cJSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif
