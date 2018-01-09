
#include "symboltable.h"

using namespace dns;

void SymbolTable::build(const char *packet_start, const char *end)
{
    int effective_length = std::min<int>(64, (end - packet_start));
    m_table.clear();
    for (int i = 0; i < effective_length; ++i) {
        const char *src = packet_start + i;
        Name attempted_name;
        try {
            src = attempted_name.decode(*this, src, end);
            if (src != nullptr) {
                // We've found a valid name encoded at this offset.
                if (i <= 63) {
                    m_table[0xC0 + i] = std::move(attempted_name);
                }
            }
        } catch (...) {
            // ignore errors, although there shouldn't be any
        }
    }
}
