#pragma once

#include "guest.h"

#include <filesystem>
#include <vector>

class guestbook
{
public:
    explicit guestbook(std::filesystem::path fileName);

    void load();
    const std::vector<guest>& all() const;
    const guest* findById(int id) const;
    std::vector<guest> search(const std::string& query) const;

    int add(guest person);
    void edit(int id, guest person);
    void remove(int id);

private:
    std::filesystem::path fileName;
    std::vector<guest> guests;
    int nextId = 1;

    void save(const std::vector<guest>& records, int next) const;
};
