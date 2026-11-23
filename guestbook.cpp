#include "guestbook.h"

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace
{
bool validDate(const std::string& date)
{
    if (date.size() != 10 || date[2] != '.' || date[5] != '.')
        return false;

    for (size_t i = 0; i < date.size(); ++i) {
        if (i != 2 && i != 5 && (date[i] < '0' || date[i] > '9'))
            return false;
    }

    int day = std::stoi(date.substr(0, 2));
    int month = std::stoi(date.substr(3, 2));
    int year = std::stoi(date.substr(6));
    if (year == 0 || month < 1 || month > 12)
        return false;

    bool leap = year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
    int days[] = {31, leap ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return day >= 1 && day <= days[month - 1];
}

void validate(const guest& person, const std::vector<guest>& records)
{
    for (const auto* field : {&person.fullName, &person.document, &person.phone,
                              &person.birthDate, &person.citizenship, &person.note}) {
        if (field->find_first_of("\r\n") != std::string::npos)
            throw std::invalid_argument("Каждое поле должно занимать одну строку.");
        if (field != &person.note && field->find_first_not_of(" \t") == std::string::npos)
            throw std::invalid_argument("Заполните все поля, кроме примечания.");
    }

    if (!validDate(person.birthDate))
        throw std::invalid_argument("Проверьте дату рождения. Формат ДД.ММ.ГГГГ.");

    for (const auto& other : records) {
        if (other.id != person.id && other.document == person.document)
            throw std::invalid_argument("Гость с таким документом уже есть.");
    }
}
}

guestbook::guestbook(std::filesystem::path fileName)
    : fileName(std::move(fileName))
{
}

void guestbook::load()
{
    std::vector<guest> records;
    int next = 1;
    if (std::filesystem::exists(fileName)) {
        std::ifstream input(fileName);
        std::string line;
        if (!std::getline(input, line))
            throw std::runtime_error("Не удалось прочитать файл гостей.");

        std::istringstream header(line);
        if (!(header >> next) || next < 1 || !(header >> std::ws).eof())
            throw std::runtime_error("Повреждён заголовок файла гостей.");

        while (std::getline(input, line)) {
            guest person;
            std::istringstream row(line);
            if (!(row >> person.id >> std::quoted(person.fullName) >> std::quoted(person.document)
                      >> std::quoted(person.phone) >> std::quoted(person.birthDate)
                      >> std::quoted(person.citizenship) >> std::quoted(person.note))
                || !(row >> std::ws).eof() || person.id <= 0 || person.id >= next)
                throw std::runtime_error("Повреждена строка в файле гостей.");

            for (const auto& other : records) {
                if (other.id == person.id)
                    throw std::runtime_error("В файле повторяется номер записи.");
            }
            validate(person, records);
            records.push_back(person);
        }
        if (!input.eof() || input.bad())
            throw std::runtime_error("Ошибка чтения файла гостей.");
    }
    guests.swap(records);
    nextId = next;
}

void guestbook::save(const std::vector<guest>& records, int next) const
{
    auto temp = fileName;
    temp += ".tmp";
    std::ofstream output(temp);
    output << next << '\n';
    for (const auto& person : records) {
        output << person.id << ' ' << std::quoted(person.fullName) << ' '
               << std::quoted(person.document) << ' ' << std::quoted(person.phone) << ' '
               << std::quoted(person.birthDate) << ' ' << std::quoted(person.citizenship)
               << ' ' << std::quoted(person.note) << '\n';
    }
    output.close();
    if (!output)
        throw std::runtime_error("Не удалось записать файл гостей.");

    std::filesystem::rename(temp, fileName);
}

const std::vector<guest>& guestbook::all() const
{
    return guests;
}

const guest* guestbook::findById(int id) const
{
    for (const auto& person : guests) {
        if (person.id == id)
            return &person;
    }
    return nullptr;
}

std::vector<guest> guestbook::search(const std::string& query) const
{
    std::vector<guest> result;
    for (const auto& person : guests) {
        if (person.fullName.find(query) != std::string::npos ||
            person.document.find(query) != std::string::npos ||
            person.phone.find(query) != std::string::npos)
            result.push_back(person);
    }
    return result;
}

int guestbook::add(guest person)
{
    if (nextId == std::numeric_limits<int>::max())
        throw std::runtime_error("Закончились номера записей.");
    person.id = nextId;
    validate(person, guests);
    auto records = guests;
    records.push_back(person);
    save(records, nextId + 1);
    guests.swap(records);
    ++nextId;
    return person.id;
}

void guestbook::edit(int id, guest person)
{
    auto records = guests;
    for (auto& entry : records) {
        if (entry.id == id) {
            person.id = id;
            validate(person, guests);
            entry = person;
            save(records, nextId);
            guests.swap(records);
            return;
        }
    }
    throw std::invalid_argument("Гость не найден.");
}

void guestbook::remove(int id)
{
    auto records = guests;
    for (auto it = records.begin(); it != records.end(); ++it) {
        if (it->id == id) {
            records.erase(it);
            save(records, nextId);
            guests.swap(records);
            return;
        }
    }
    throw std::invalid_argument("Гость не найден.");
}
