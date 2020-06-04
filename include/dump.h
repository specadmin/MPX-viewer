#ifndef DUMP_H
#define DUMP_H
//-----------------------------------------------------------------------------
#include "common.h"
//-----------------------------------------------------------------------------
class CDumpEntry
{
public:
    CDumpEntry();
    CDumpEntry(BYTE* data);
    void update(BYTE* data);
    BYTE getID();
    void* operator new(size_t count);
    void print();
private:
    BYTE m_buf[12];
    BYTE m_prevBuf[12];
    BYTE m_id;
    BYTE m_size;
    DWORD m_count;
    BYTE m_updated;
    char* m_name;
};
//-----------------------------------------------------------------------------
class CDump
{
public:
    CDump();
    CDumpEntry* operator [](int index);
    void push(BYTE* data);
    void show();
private:
    void sort();
    CDumpEntry* find(BYTE id);
    CDumpEntry** m_entries;
    BYTE m_entriesCount;
};
//-----------------------------------------------------------------------------
#endif
