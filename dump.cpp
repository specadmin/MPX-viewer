//-----------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <cstdlib>
#include "dump.h"
//-----------------------------------------------------------------------------
#define FG_GREEN        "\033[1;32m"
#define FG_YELLOW       "\033[1;33m"
#define FG_BLUE         "\033[1;34m"
#define FG_MAGENTA      "\033[1;35m"
#define CL_INVERSE      "\033[1;7m"
//-----------------------------------------------------------------------------
#define HIGHLIGHT_TIME          25
//-----------------------------------------------------------------------------
struct sKnown
{
    BYTE id;
    char name[8];
} known[] =
{
    0x15,   "CP_BTNS",
    0x20,   "FRATE_T",
    0x24,   "SPD+IGN",
    0x25,   "AVG_SPD",
    0x26,   " TACHO ",
    0x40,   "TRC_SEL",
    0x90,   "FRATE_D",
    0x1A,   "CP_ENCS",
    0x1B,   "BLOWSPD",
    0x2C,   "ENGTEMP",
    0x76,   "TRIPDIS",
    0xA3,   "AVG_F/R",
    0xCC,   "AMBTEMP",
    0xCD,   "AMBTEMP",
    0xD2,   "TRC_MOD",
    0xD4,   "ENGSTAT",
    0xD7,   "MCU_CMD",
    0xE4,   "B_MISC ",
    0xFC,   "CLIMATE",
    0xFD,   "TGTTEMP"
};
//-----------------------------------------------------------------------------
static char line[64];
//-----------------------------------------------------------------------------
CDumpEntry::CDumpEntry()
{
    m_id = 0;
    m_size = 0;
    m_count = 0;
    m_updated = false;
}
//-----------------------------------------------------------------------------
void CDumpEntry::update(BYTE* data)
{
    m_size = (data[0] & 0x0F) + 3;
    memcpy(m_prevBuf, m_buf, m_size);
    memcpy(m_buf, data, m_size);
    m_count++;
    if(memcmp(m_prevBuf + 3, m_buf + 3, m_size - 5))
    {
        m_updated = HIGHLIGHT_TIME;
    }
}
//-----------------------------------------------------------------------------
CDumpEntry::CDumpEntry(BYTE* data)
{
    m_id = data[2];
    update(data);
    m_updated = true;
    m_name = NULL;
    for(BYTE i = 0; i < sizeof(known) / sizeof(sKnown); i++)
    {
        if(known[i].id == m_id)
        {
            m_name = known[i].name;
            break;
        }
    }
}
//-----------------------------------------------------------------------------
void* CDumpEntry::operator new(size_t count)
{
    return malloc(sizeof(CDumpEntry) * count);
}
//-----------------------------------------------------------------------------
BYTE CDumpEntry::getID()
{
    return m_id;
}
//-----------------------------------------------------------------------------
void CDumpEntry::print()
{
    line[0] = 0;
    for(BYTE i = 0; i < m_size; i++)
    {
        if(i == 2)
        {
            // MSG-ID
            sprintf(line + strlen(line), FG_MAGENTA);
        }
        else if(i == (m_size - 2))
        {
            // CRC
            sprintf(line + strlen(line), FG_BLUE);
        }
        else if(i > 2 && i < (m_size - 2))
        {
            // data
            if(m_updated)
            {
                sprintf(line + strlen(line), CL_INVERSE);
            }
            else
            {
                sprintf(line + strlen(line), FG_GREEN);
            }
        }
        else
        {
            // other
            sprintf(line + strlen(line), FG_BLUE);
        }
        sprintf(line + strlen(line), "%02X\033[1;0m ", m_buf[i]);
    }
    printf(" %6ld", m_count);
    if(m_name)
    {
        printf("   %s   ", m_name);
    }
    else
    {
        printf("             ");
    }
    printf("%s\n", line);
    m_updated = (m_updated > 0) ? m_updated - 1 : 0;
}
//-----------------------------------------------------------------------------
CDump::CDump()
{
    m_entries = NULL;
    m_entriesCount = 0;
}
//-----------------------------------------------------------------------------
CDumpEntry* CDump::find(BYTE id)
{
    for(BYTE i = 0; i < m_entriesCount; i++)
    {
        if(m_entries[i]->getID() == id)
        {
            return m_entries[i];
        }
    }
    return NULL;
}
//-----------------------------------------------------------------------------
void CDump::push(BYTE* data)
{
    CDumpEntry* entry = find(data[2]);
    if(entry)
    {
        entry->update(data);
    }
    else
    {
        m_entries = (CDumpEntry**) realloc(m_entries, sizeof(CDumpEntry*) * (m_entriesCount + 1));
        m_entries[m_entriesCount++] = (CDumpEntry*) new CDumpEntry(data);
    }
}
//-----------------------------------------------------------------------------
void CDump::sort()
{
    CDumpEntry* tmp;
    bool sorted = false;
    if(m_entriesCount < 2)
    {
        // nothing to sort
        return;
    }
    while(!sorted)
    {
        sorted = true;
        for(BYTE i = 0; i < (m_entriesCount - 1); i++)
        {
            if(m_entries[i]->getID() > m_entries[i + 1]->getID())
            {
                // swap entries
                tmp = m_entries[i];
                m_entries[i] = m_entries[i + 1];
                m_entries[i + 1] = tmp;

                sorted = false;
            }
        }
    }
}
//-----------------------------------------------------------------------------
void CDump::show()
{
    sort();
    std::system ("clear");
    printf("\n");
    for(BYTE i = 0; i < m_entriesCount; i++)
    {
        m_entries[i]->print();
    }
}
//-----------------------------------------------------------------------------
