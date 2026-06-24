#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

constexpr int PHONE_DIGITS = 10;

typedef struct TNode
{
  	char* m_Name;
  	struct TNode* m_Child[PHONE_DIGITS];
}TNODE;

typedef struct
{
  	TNODE* m_Root;
  	int m_Size;
}TPHONEBOOK;

#endif /* __PROGTEST__ */
#include <stdbool.h>
#define VEC_INIT_CAP 10


bool CheckPhoneNumber(const char* phone, size_t* phoneLen)
{
	*phoneLen = strlen(phone);
	for (size_t i = 0; i < *phoneLen; i++)
		if (!isdigit(phone[i]))
			return false;
	return true;
}

void TNODE_setname(TNODE* node, const char* name)
{
	if (node->m_Name)
		free(node->m_Name);
	
	if (!name)
		node->m_Name = NULL;
	else
		node->m_Name = strdup(name);
}

TNODE* TNODE_init(const char* name)
{
	TNODE* node = (TNODE*)malloc(sizeof(TNODE));
	node->m_Name = NULL;
	
	TNODE_setname(node, name);
	
	for (size_t i = 0; i < PHONE_DIGITS; i++)
		node->m_Child[i] = NULL;
	
	return node;
}

bool addPhone(TPHONEBOOK* book, const char* phone, const char* name)
{
	size_t phoneLen;
	if (!CheckPhoneNumber(phone, &phoneLen))
		return false;

	if (!book->m_Root)
	{
    	book->m_Root = TNODE_init(NULL);
    	book->m_Size = 0;
	}

	TNODE* currNode = book->m_Root;
	for	(size_t i = 0; i < phoneLen; i++)
	{
		int index = phone[i] - '0';
		if (currNode->m_Child[index] == NULL)
			currNode->m_Child[index] = TNODE_init(NULL);
		currNode = currNode->m_Child[index];
	}
	
	if (!currNode->m_Name)
		book->m_Size++;
	TNODE_setname(currNode, name);
	
	return true;
}

void delNode(TNODE* node)
{
	if (!node)
		return;
	
	for (size_t i = 0; i < PHONE_DIGITS; i++)
	{
		delNode(node->m_Child[i]);
	}

	if (node->m_Name)
		free(node->m_Name);
	free(node);
}

void delBook(TPHONEBOOK * book)
{
	delNode(book->m_Root);
}

typedef struct {
    TNODE* node;
    int index;
} VecElement;

typedef struct
{
	VecElement* arr;
	size_t count;
	size_t capacity;
}Vec;

void Vec_init(Vec* v)
{
    v->capacity = VEC_INIT_CAP;
    v->count = 0;
    v->arr = (VecElement*)malloc(v->capacity * sizeof(VecElement));
}

void Vec_resize(Vec* v)
{
    if (v->count == v->capacity)
	{
        v->capacity *= 2;
        v->arr = (VecElement*)realloc(v->arr, v->capacity * sizeof(VecElement));
	}
}

void Vec_free(Vec* v)
{
	free(v->arr);
}

void Vec_pushback(Vec* v, VecElement item)
{
    Vec_resize(v);
    v->arr[v->count++] = item;
}

void Vec_popback(Vec* v)
{
	if (v->count != 0)
		v->count--;
	else
		printf("Called popback on empty vec\n");
}

bool delPhone(TPHONEBOOK* book, const char* phone)
{
    size_t phoneLen;
    if (!CheckPhoneNumber(phone, &phoneLen) || !book->m_Root)
        return false;

    Vec path;
    Vec_init(&path);

    TNODE* currNode = book->m_Root;
    VecElement root = {currNode, -1};
    Vec_pushback(&path, root);

    for (size_t i = 0; i < phoneLen; i++)
    {
        int index = phone[i] - '0';
        if (currNode->m_Child[index] == NULL)
        {
            Vec_free(&path);
            return false;
        }
        currNode = currNode->m_Child[index];
        VecElement currentItem = {currNode, index};
        Vec_pushback(&path, currentItem);
    }

    if (!currNode->m_Name)
    {
        Vec_free(&path);
        return false;
    }

    TNODE_setname(currNode, NULL);
    book->m_Size--;

    bool hasChild = false;
    for (size_t i = 0; i < PHONE_DIGITS; i++)
        if (currNode->m_Child[i])
        {
            hasChild = true;
            break;
        }

    if (!hasChild)
        while (path.count > 0)
        {
            VecElement current = path.arr[path.count - 1];
            TNODE* node = current.node;

            hasChild = false;
            for (size_t i = 0; i < PHONE_DIGITS; i++)
                if (node->m_Child[i] != NULL)
                {
                    hasChild = true; break;
                }

            if (node->m_Name || hasChild)
                break;

            Vec_popback(&path);

            if (path.count == 0)
            {
                delNode(node);
                book->m_Root = NULL;
                break;
            }

            VecElement parent = path.arr[path.count - 1];
            TNODE* parentNode = parent.node;
            int childIndex = current.index;
            parentNode->m_Child[childIndex] = NULL;
            delNode(node);
        }
    Vec_free(&path);
    return true;
}


const char* findPhone(TPHONEBOOK* book, const char* phone)
{
	size_t phoneLen;
	if (!CheckPhoneNumber(phone, &phoneLen) || !book->m_Root)
		return NULL;

	const char* mostPrecise = NULL;

	TNODE* currNode = book->m_Root;
	for	(size_t i = 0; i < phoneLen; i++)
	{
		currNode = currNode->m_Child[phone[i] - '0'];
		if (currNode == NULL || i == phoneLen)
			break;
		if (currNode->m_Name)
		{
			mostPrecise = currNode->m_Name;
		}
	}
	return mostPrecise;
}

#ifndef __PROGTEST__
int main ()
{
  	TPHONEBOOK b = { nullptr, 0 };
  	char tmpStr[100];
  	const char * name;
  	assert ( addPhone ( &b, "420", "Czech Republic" ) );
  	assert ( addPhone ( &b, "42022435", "Czech Republic CVUT" ) );
  	assert ( addPhone ( &b, "421", "Slovak Republic" ) );
  	assert ( addPhone ( &b, "44", "Great Britain" ) );
  	strncpy ( tmpStr, "USA", sizeof ( tmpStr ) - 1 );
  	assert ( addPhone ( &b, "1", tmpStr ) );
  	strncpy ( tmpStr, "Guam", sizeof ( tmpStr ) - 1 );
  	assert ( addPhone ( &b, "1671", tmpStr ) );
  	assert ( addPhone ( &b, "44", "United Kingdom" ) );
  	assert ( b . m_Size == 6 );
  	assert ( ! b . m_Root -> m_Name );
  	assert ( ! b . m_Root -> m_Child[0] );
  	assert ( ! strcmp ( b . m_Root -> m_Child[1] -> m_Name, "USA" ) );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[0] );
  	assert ( ! strcmp ( b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Name, "Guam" ) );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[1] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[7] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Name );
  	assert ( ! strcmp ( b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Name, "Czech Republic" ) );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[4] );
  	assert ( ! strcmp ( b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Name, "Czech Republic CVUT" ) );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[9] );
  	assert ( ! strcmp ( b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Name, "Slovak Republic" ) );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[3] );
  	assert ( ! strcmp ( b . m_Root -> m_Child[4] -> m_Child[4] -> m_Name, "United Kingdom" ) );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[9] );
  	name = findPhone ( &b, "420800123456" );  assert ( !strcmp ( name, "Czech Republic" ) );
  	name = findPhone ( &b, "420224351111" );  assert ( !strcmp ( name, "Czech Republic CVUT" ) );
  	name = findPhone ( &b, "42022435" );  assert ( !strcmp ( name, "Czech Republic CVUT" ) );
  	name = findPhone ( &b, "4202243" );  assert ( !strcmp ( name, "Czech Republic" ) );
  	name = findPhone ( &b, "420224343258985224" );  assert ( !strcmp ( name, "Czech Republic" ) );
  	name = findPhone ( &b, "42" );  assert ( !name );
  	name = findPhone ( &b, "422" );  assert ( !name );
  	name = findPhone ( &b, "4422" );  assert ( !strcmp ( name, "United Kingdom" ) );
  	name = findPhone ( &b, "16713425245763" );  assert ( !strcmp ( name, "Guam" ) );
  	name = findPhone ( &b, "123456789123456789" );  assert ( !strcmp ( name, "USA" ) );
  	assert ( delPhone ( &b, "420" ) );
  	assert ( delPhone ( &b, "421" ) );
  	assert ( delPhone ( &b, "44" ) );
  	assert ( delPhone ( &b, "1671" ) );
  	assert ( !delPhone ( &b, "1672" ) );
  	assert ( !delPhone ( &b, "1671" ) );
  	name = findPhone ( &b, "16713425245763" );  assert ( !strcmp ( name, "USA" ) );
  	name = findPhone ( &b, "4422" );  assert ( !name );
  	name = findPhone ( &b, "420800123456" );  assert ( !name );
  	name = findPhone ( &b, "420224351111" );  assert ( !strcmp ( name, "Czech Republic CVUT" ) );
  	assert ( b . m_Size == 2 );
  	assert ( ! b . m_Root -> m_Name );
  	assert ( ! b . m_Root -> m_Child[0] );
  	assert ( ! strcmp ( b . m_Root -> m_Child[1] -> m_Name, "USA" ) );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[1] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Name );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[4] );
  	assert ( ! strcmp ( b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Name, "Czech Republic CVUT" ) );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[0] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[5] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[3] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[4] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[2] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[2] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[0] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[1] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[2] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[2] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[3] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[4] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[4] -> m_Child[9] );
  	assert ( ! b . m_Root -> m_Child[5] );
  	assert ( ! b . m_Root -> m_Child[6] );
  	assert ( ! b . m_Root -> m_Child[7] );
  	assert ( ! b . m_Root -> m_Child[8] );
  	assert ( ! b . m_Root -> m_Child[9] );
  	assert ( delPhone ( &b, "1" ) );
  	assert ( delPhone ( &b, "42022435" ) );
  	assert ( !addPhone ( &b, "12345XYZ", "test" ) );
  	assert ( b . m_Size == 0 );
  	assert ( ! b . m_Root );

  	delBook ( &b );
  	return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
