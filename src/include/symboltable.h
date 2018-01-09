#pragma once

#include "name.h"

#include <map>

namespace dns {

class Name;

/**
 *  Class that represents a DNS message (a query or a response).
 */
class SymbolTable {
public:
    explicit SymbolTable() = default;

    void build(const char *packet_start, const char *end);

    auto find(int key) const noexcept -> std::map<int, Name>::const_iterator {
        return m_table.find(key);
    }
    auto end() const noexcept -> std::map<int, Name>::const_iterator { return m_table.end(); }

private:
    std::map<int, Name> m_table;
};

} // namespace dns
