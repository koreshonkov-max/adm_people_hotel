#include "guestbook.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#endif

std::string readLine(const std::string& prompt)
{
    std::cout << prompt;
    std::string value;
    if (!std::getline(std::cin, value))
        throw std::ios_base::failure("Ввод завершён.");
    return value;
}

int readId()
{
    std::istringstream input(readLine("Номер записи: "));
    int id;
    if (!(input >> id) || id < 1 || !(input >> std::ws).eof())
        throw std::invalid_argument("Введите положительное целое число.");
    return id;
}

std::string field(const std::string& title, const std::string& oldValue)
{
    std::string prompt = title;
    if (!oldValue.empty())
        prompt += " [" + oldValue + "]";
    std::string value = readLine(prompt + ": ");
    return value.empty() ? oldValue : value;
}

guest readCard(guest person = {})
{
    person.fullName = field("ФИО", person.fullName);
    person.document = field("Документ", person.document);
    person.phone = field("Телефон", person.phone);
    person.birthDate = field("Дата рождения (ДД.ММ.ГГГГ)", person.birthDate);
    person.citizenship = field("Гражданство", person.citizenship);
    if (person.id != 0 && !person.note.empty()) {
        std::string note = readLine("Примечание [" + person.note + "] (минус очистит поле): ");
        if (note == "-")
            person.note.clear();
        else if (!note.empty())
            person.note = note;
    } else {
        person.note = readLine("Примечание: ");
    }
    return person;
}

void show(const std::vector<guest>& records)
{
    std::cout << "Количество записей: " << records.size() << '\n';
    for (const auto& person : records) {
        std::cout << "№" << person.id << " " << person.fullName
                  << "\nДокумент: " << person.document << "\nТелефон: " << person.phone
                  << "\nДата рождения: " << person.birthDate
                  << "\nГражданство: " << person.citizenship
                  << "\nПримечание: " << person.note << "\n\n";
    }
}

int main()
{
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    guestbook book("guests.txt");
    try {
        book.load();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    while (true) {
        try {
            std::cout << "\n1. Добавить гостя\n2. Список гостей\n3. Найти гостя\n"
                         "4. Изменить карточку\n5. Удалить запись\n0. Выход\n";
            std::string choice = readLine("Выберите действие: ");
            if (choice == "0")
                break;
            if (choice == "1") {
                int id = book.add(readCard());
                std::cout << "Добавлен гость №" << id << '\n';
            } else if (choice == "2") {
                show(book.all());
            } else if (choice == "3") {
                show(book.search(readLine("Часть ФИО, телефона или документа: ")));
            } else if (choice == "4" || choice == "5") {
                int id = readId();
                const guest* person = book.findById(id);
                if (!person)
                    throw std::invalid_argument("Гость не найден.");
                if (choice == "4") {
                    std::cout << "Нажмите Enter, чтобы оставить прежнее значение.\n";
                    book.edit(id, readCard(*person));
                    std::cout << "Карточка сохранена.\n";
                } else if (readLine("Удалить запись «" + person->fullName + "»? (да/нет): ") == "да") {
                    book.remove(id);
                    std::cout << "Запись удалена.\n";
                }
            } else {
                std::cout << "Выберите пункт от 0 до 5.\n";
            }
        } catch (const std::ios_base::failure&) {
            break;
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
        }
    }
}
