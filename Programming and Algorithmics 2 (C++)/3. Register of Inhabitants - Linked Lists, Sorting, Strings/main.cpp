#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#endif /* __PROGTEST__ */

/* 
    CString class
        very basic string implementation to keep program simple
*/
class CString
{
private:

    unsigned size;  
    char* data;

public:

    /* Default constructor not needed -> deleted */    
    CString() = delete;

    /* Constructor(const char* str) */
    CString(const char* str) : size(strlen(str) + 1), data(new char[size])
    {
        memcpy(data, str, size);
    }

    /* Constructor(const CString& str) */
    CString(const CString& other) : size(other.size), data(new char[size])
    {
        memcpy(data, other.data, size);
    }

    /* Destructor */
    ~CString()
    {
        delete[] data;
    }

    /*
        operator<
            uses strcmp (lexicographical comparison)
            needed to be used with SortedLinkedList
    */
    bool operator<(const CString& other) const
    {
        return strcmp(data, other.data) < 0;
    }

    /*
        operator==
            uses strcmp (lexicographical comparison)
            return true if identical
    */
    bool operator==(const CString& other) const
    {
        return strcmp(data, other.data) == 0;
    }

    /*
        friend operator<<
            feeds ostream with data
    */
    friend std::ostream& operator<<(std::ostream& os, const CString& str);
};

std::ostream& operator<<(std::ostream& os, const CString& str)
{
    return (os << str.data);
}

/* 
    SortedLinkedList class
        remains sorted
            requires operator< for datatype used
        contains no duplicates
        variables: 
            head - first element (Node) in linked list
            FLAG_DUPLICATE - is set true if found exact match in insert method
*/
template <typename T>
class SortedLinkedList
{
public:

    struct Node;
    bool FLAG_DUPLICATE = false;

private:
    
    Node* head;

public:

    /* Constructor */
    SortedLinkedList() : head(nullptr) {}

    /* Copy constructor */
    SortedLinkedList(const SortedLinkedList& other) : head(nullptr)
    {
        Node** currPtr = &head; 
        Node* currOther = other.head;
        for ( ; currOther; currOther = currOther->next, currPtr = &((*currPtr)->next))
            *currPtr = new Node(currOther->data);
    }

    /* Destructor */
    ~SortedLinkedList()
    {
        while (head)
        {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }

    /* 
        getHead method
            getter method -> returns pointer to first element of linked list
    */
    const Node* getHead() const
    {
        return head;
    }
    
    /* 
        getT Method    
            returns pointer to <T> value from parameter 
    */
    const T* getT(const T& value) const
    {
        const Node* current = head;
        while (current && current->data < value)
            current = current->next;
        if (current && !(value < current->data))
            return &current->data;
        return nullptr;
    }

    /* 
        insert Method
            -> if duplicate id (person exists), returns pointer to the node containing the match
            -> else if unique id (person doesn't exist), return pointer to the newly created person
    */
    Node* insert(const T& value)
    {
        Node** current = &head;
        while (*current && ((*current)->data < value))
            current = &((*current)->next);
        
        /* Duplicity check */
        if (*current && !((value < (*current)->data) || ((*current)->data < value)))
        {
            FLAG_DUPLICATE = true;
            return *current;
        }

        Node* newNode = new Node(value);
        newNode->next = *current;
        *current = newNode;
        FLAG_DUPLICATE = false;
        return newNode;
    }

    /* 
        operator=
            deletes contents of linked list
            replaces with contests for other parameter
    */
    SortedLinkedList& operator=(const SortedLinkedList& other)
    {
        if (this != &other)
        {
            /* delete data in current linked list */
            while (head)
            {
                Node* temp = head;
                head = head->next;
                delete temp;
            }
            head = nullptr;
            Node** currPtr = &head;
            for (const Node* currOther = other.head; currOther; currOther = currOther->next, currPtr = &((*currPtr)->next))
                *currPtr = new Node(currOther->data);
        }
        return *this;
    }

    /* 
        Node struct
            element of linked list
    */
    struct Node
    {
        T data;
        Node* next;
        Node(const T& val) : data(val), next(nullptr) {}
    };
};

/* Shared database of cities and addresses for all CRegister instances */
static SortedLinkedList<CString> cityDB;
static SortedLinkedList<CString> addressDB;

/* 
    CRegister call
        keeps track persons' current and past settlements
*/
class CRegister
{
private:
    
    /* forward declaration */
    struct Person;
    
    /* data */
    SortedLinkedList<Person> personDB;

public:
    
    /* Default constructor */
    CRegister() = default;

    /* Copy constructor */
    CRegister(const CRegister& other) : personDB(other.personDB){}

    /* Destructor */
    ~CRegister() = default;

    /* 
        Operator=
            copies it's contents from other CRegister instance
            heavily relies on operator= from SortedLinkedList 
    */
    CRegister& operator=(const CRegister& other)
    {
        if (this != &other)
            personDB = other.personDB;

        return *this;
    }

    /*
        add method
            inserts new person into personDB
                person needs to have unique ID, otherwise is not added
            adds/searches for address and city in corresponding databases
            adds new record into person's records
    */
    bool add(const char id[],
             const char name[],
             const char surname[],
             const char date[],
             const char street[],
             const char city[])
    {
        /* Create new person and try to insert into person database */
        Person newPerson = Person(id, name, surname);
        auto* pNode = personDB.insert(newPerson);

        /* Check if person with same ID exists */
        if (personDB.FLAG_DUPLICATE)
            return false;

        /* Find or add city and address into database */
        auto* cityNode = cityDB.insert(city);
        const CString* cityPtr = &cityNode->data;

        auto* addrNode = addressDB.insert(street);
        const CString* addrPtr = &addrNode->data;

        /* Add into new person's records */
        pNode->data.records.insert(Record(addrPtr, cityPtr, CString(date)));
        return true;
    }

    /*
        resettle method
            finds person by ID in personDB
                if ID is not found, functions returns false
            checks there isn't date collision (1 record per date rule)
            adds/searches for address and city in corresponding databases
            adds new record into person's records
    */
    bool resettle(const char id[],
                  const char date[],
                  const char street[],
                  const char city[])
    {
        /* Find person by id */
        Person newPerson = Person(id, "", "");
        Person* p = const_cast<Person*>(personDB.getT(newPerson));
        if (!p) // if this fails, tak je to v pytli 
            return false; 

        /* Check if person has assigned a resettlement on the specific date */
        Record tempRec(nullptr, nullptr, CString(date));
        if (p->records.getT(tempRec))
            return false;
        
        /* Find or add city and address into database */
        auto* cityNode = cityDB.insert(city);
        const CString* cityPtr = &cityNode->data;

        auto* addrNode = addressDB.insert(street);
        const CString* addrPtr = &addrNode->data;
        
        /* Add record to person */
        p->records.insert(Record(addrPtr, cityPtr, CString(date)));
        return true;
    }
    /*
        print method
            finds person by ID in personDB
                if ID is not found, functions returns false
            prints person's info and records
    */
    bool print(std::ostream & os, const char id[]) const
    {
        /* Find person by id */
        Person newPerson = Person(id, "", "");
        Person* p = const_cast<Person*>(personDB.getT(newPerson));
        if (!p) // if this fails, tak je to v pytli 
            return false; 

        /* Print his id, name, surname */
        os << p->id << " " << p->name << " " << p->surname << '\n';

        /* Iterate through person's records are sending them on output stream */
        const auto* recNode = p->records.getHead();
        while (recNode)
        {
            os << recNode->data.date << " " 
               << *(recNode->data.address) << " " 
               << *(recNode->data.city) << '\n';
            recNode = recNode->next;
        }
        return true;
    }

private:

    /* 
        Record struct
            each Person has linked list of Records (address, city, date)
    */
    struct Record
    {
        const CString* address;
        const CString* city;
        const CString date;

        Record(const CString* addr_, const CString* city_, CString date_) : address(addr_), city(city_), date(date_) {}
        
        bool operator<(const Record& other) const
        {
            return date < other.date;
        }
    };
    
    /* 
        Person struct
            element of personDB
    */
    struct Person
    {
        const CString id;
        const CString name;
        const CString surname;
        SortedLinkedList<Record> records;

        Person(const char* id_, const char* name_, const char* surname_) : id(id_), name(name_), surname(surname_) {}

        bool operator<(const Person & other) const 
        {
            return id < other.id;
        }
    };
};

#ifndef __PROGTEST__
int main()
{
  char   lID[12], lDate[12], lName[50], lSurname[50], lStreet[50], lCity[50];
  std::ostringstream oss;
  CRegister  a;
  assert ( a . add ( "123456/7890", "John", "Smith", "2000-01-01", "Main street", "Seattle" ) == true );
  assert ( a . add ( "987654/3210", "Freddy", "Kruger", "2001-02-03", "Elm street", "Sacramento" ) == true );
  assert ( a . resettle ( "123456/7890", "2003-05-12", "Elm street", "Atlanta" ) == true );
  assert ( a . resettle ( "123456/7890", "2002-12-05", "Sunset boulevard", "Los Angeles" ) == true );
  oss . str ( "" );
  assert ( a . print ( oss, "123456/7890" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(123456/7890 John Smith
2000-01-01 Main street Seattle
2002-12-05 Sunset boulevard Los Angeles
2003-05-12 Elm street Atlanta
)###" ) );
  oss . str ( "" );
  assert ( a . print ( oss, "987654/3210" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
)###" ) );
  CRegister b ( a );
  assert ( b . resettle ( "987654/3210", "2008-04-12", "Elm street", "Cinccinati" ) == true );
  assert ( a . resettle ( "987654/3210", "2007-02-11", "Elm street", "Indianapolis" ) == true );
  oss . str ( "" );
  assert ( a . print ( oss, "987654/3210" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2007-02-11 Elm street Indianapolis
)###" ) );
  oss . str ( "" );
  assert ( b . print ( oss, "987654/3210" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
)###" ) );
  a = b;
  assert ( a . resettle ( "987654/3210", "2011-05-05", "Elm street", "Salt Lake City" ) == true );
  oss . str ( "" );
  assert ( a . print ( oss, "987654/3210" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
2011-05-05 Elm street Salt Lake City
)###" ) );
  oss . str ( "" );
  assert ( b . print ( oss, "987654/3210" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm street Sacramento
2008-04-12 Elm street Cinccinati
)###" ) );
  assert ( b . add ( "987654/3210", "Joe", "Lee", "2010-03-17", "Abbey road", "London" ) == false );
  assert ( a . resettle ( "987654/3210", "2001-02-03", "Second street", "Milwaukee" ) == false );
  oss . str ( "" );
  assert ( a . print ( oss, "666666/6666" ) == false );

  CRegister  c;
  strncpy ( lID, "123456/7890", sizeof ( lID ) );
  strncpy ( lName, "John", sizeof ( lName ) );
  strncpy ( lSurname, "Smith", sizeof ( lSurname ) );
  strncpy ( lDate, "2000-01-01", sizeof ( lDate) );
  strncpy ( lStreet, "Main street", sizeof ( lStreet ) );
  strncpy ( lCity, "Seattle", sizeof ( lCity ) );
  assert (  c . add ( lID, lName, lSurname, lDate, lStreet, lCity ) == true );
  strncpy ( lID, "987654/3210", sizeof ( lID ) );
  strncpy ( lName, "Freddy", sizeof ( lName ) );
  strncpy ( lSurname, "Kruger", sizeof ( lSurname ) );
  strncpy ( lDate, "2001-02-03", sizeof ( lDate) );
  strncpy ( lStreet, "Elm street", sizeof ( lStreet ) );
  strncpy ( lCity, "Sacramento", sizeof ( lCity ) );
  assert (  c . add ( lID, lName, lSurname, lDate, lStreet, lCity ) == true );
  strncpy ( lID, "123456/7890", sizeof ( lID ) );
  strncpy ( lDate, "2003-05-12", sizeof ( lDate) );
  strncpy ( lStreet, "Elm street", sizeof ( lStreet ) );
  strncpy ( lCity, "Atlanta", sizeof ( lCity ) );
  assert ( c . resettle ( lID, lDate, lStreet, lCity ) == true );
  strncpy ( lID, "123456/7890", sizeof ( lID ) );
  strncpy ( lDate, "2002-12-05", sizeof ( lDate) );
  strncpy ( lStreet, "Sunset boulevard", sizeof ( lStreet ) );
  strncpy ( lCity, "Los Angeles", sizeof ( lCity ) );
  assert ( c . resettle ( lID, lDate, lStreet, lCity ) == true );
  oss . str ( "" );
  assert ( c . print ( oss, "123456/7890" ) == true );
  assert ( ! strcmp ( oss . str () . c_str (), R"###(123456/7890 John Smith
2000-01-01 Main street Seattle
2002-12-05 Sunset boulevard Los Angeles
2003-05-12 Elm street Atlanta
)###" ) );

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
