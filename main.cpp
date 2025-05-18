#include <iostream>
#include <string>
#include <utility>
#include <windows.h>
#include <regex>

enum class PhysicalState {
    Solid = 1,
    Liquid = 2,
    Gas = 3
};

bool isValidPhysicalState(const int stateInt) {
    return stateInt == static_cast<int>(PhysicalState::Solid) ||
        stateInt == static_cast<int>(PhysicalState::Liquid) ||
        stateInt == static_cast<int>(PhysicalState::Gas);
}

std::string getPhysicalStateString(const PhysicalState state) {
    switch (state) {
    case PhysicalState::Solid: return "Твердий";
    case PhysicalState::Liquid: return "Рідкий";
    case PhysicalState::Gas: return "Газоподібний";
    default: throw std::invalid_argument("Такого фізичного стану не існує.");
    }
}

enum class MenuChoice {
    EXIT = 0,
    ADD_RECORD = 1,
    REMOVE_RECORD = 2,
    PEEK_RECORD = 3,
    PRINT_QUEUE = 4,
    CLEAR_QUEUE = 5
};

struct WasteRecord {
    std::string companyCode;
    std::string companyName;
    std::string address;
    std::string phone;
    std::string wasteCode;
    std::string wasteName;
    PhysicalState state;
    std::string removalDate;
    int quantity;
    double cost;

    WasteRecord(
        std::string  companyCode,
        std::string  companyName,
        std::string  address,
        std::string  phone,
        std::string  wasteCode,
        std::string  wasteName,
        PhysicalState state,
        std::string  removalDate,
        int quantity,
        double cost
    ) :
        companyCode(std::move(companyCode)),
        companyName(std::move(companyName)),
        address(std::move(address)),
        phone(std::move(phone)),
        wasteCode(std::move(wasteCode)),
        wasteName(std::move(wasteName)),
        state(state),
        removalDate(std::move(removalDate)),
        quantity(quantity),
        cost(cost) {}
};

struct WasteNode {
    WasteRecord data;
    WasteNode* next;

    explicit WasteNode(WasteRecord record) : data(std::move(record)), next(nullptr) {}
};

struct Queue {
    WasteNode* head;
    WasteNode* tail;

    explicit Queue() : head(nullptr), tail(nullptr) {}
};

bool isEmpty(const Queue& queue) {
    return queue.head == nullptr;
}

void clearQueue(Queue& queue) {
    while (queue.head) {
        const WasteNode* temp = queue.head;
        queue.head = queue.head->next;
        delete temp;
    }
    queue.tail = nullptr;
}

WasteRecord peek(const Queue& queue) {
    if (isEmpty(queue)) {
        throw std::out_of_range("Черга порожня");
    }
    return queue.head->data;
}

void printQueue(const Queue& queue) {
    if (isEmpty(queue)) {
        std::cout << "Черга порожня!" << std::endl;
        return;
    }

    const WasteNode* current = queue.head;
    while (current != nullptr) {
        std::cout << "  Код підприємства: " << current->data.companyCode << "\n";
        std::cout << "  Назва підприємства: " << current->data.companyName << "\n";
        std::cout << "  Адреса:      " << current->data.address << "\n";
        std::cout << "  Телефон:        " << current->data.phone << "\n";
        std::cout << "  Код відходу:   " << current->data.wasteCode << "\n";
        std::cout << "  Назва відходу:   " << current->data.wasteName << "\n";
        std::cout << "  Агрегатний стан:        " << getPhysicalStateString(current->data.state) << "\n";
        std::cout << "  Дата вивезення: " << current->data.removalDate << "\n";
        std::cout << "  Кількість:     " << current->data.quantity << "\n";
        std::cout << "  Вартість:         " << current->data.cost << "\n";
        std::cout << "  ---------------------" << std::endl;

        current = current->next;
    }
}

void enqueue(Queue& queue, const WasteRecord& record) {
    const auto newNode = new WasteNode(record);

    if (isEmpty(queue)) {
        queue.head = queue.tail = newNode;
    }
    else {
        if (queue.tail != nullptr) {
            queue.tail->next = newNode;
            queue.tail = newNode;
        }
        else {
            throw std::runtime_error("Помилка: покажчик на кінець черги дорівнює null");
        }
    }
}

WasteRecord dequeue(Queue& queue) {
    if (isEmpty(queue)) {
        throw std::out_of_range("Черга порожня");
    }

    const WasteNode* temp = queue.head;

    queue.head = temp->next;

    if (isEmpty(queue)) {
        queue.tail = nullptr;
    }

    WasteRecord removedData = temp->data;

    delete temp;

    return removedData;
}

bool isLeapYear(const int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

bool isValidDate(const std::string& date) {
    const std::regex dateRegex(R"(^(\d{2}):(\d{2}):(\d{4})$)");
    std::smatch match;

    if (!std::regex_match(date, match, dateRegex)) {
        return false;
    }

    const int day = std::stoi(match[1]);
    const int month = std::stoi(match[2]);
    const int year = std::stoi(match[3]);

    if (month < 1 || month > 12) return false;

    int daysInMonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    if (isLeapYear(year)) {
        daysInMonth[1] = 29;
    }

    return day >= 1 && day <= daysInMonth[month - 1];
}

WasteRecord inputWasteRecord() {
    std::string companyCode, companyName, address, phone;
    std::string wasteCode, wasteName, removalDate;
    int stateInt, quantity;
    double cost;

    std::cout << "Введіть код підприємства: ";
    std::getline(std::cin, companyCode);
    std::cout << "Введіть назву підприємства: ";
    std::getline(std::cin, companyName);
    std::cout << "Введіть адресу: ";
    std::getline(std::cin, address);
    std::cout << "Введіть номер телефону: ";
    std::getline(std::cin, phone);
    std::cout << "Введіть код відходу: ";
    std::getline(std::cin, wasteCode);
    std::cout << "Введіть назву відходу: ";
    std::getline(std::cin, wasteName);

    do {
        std::cout << "Введіть агрегатний стан (1 = " +
            getPhysicalStateString(PhysicalState::Solid) + ", 2 = "
        + getPhysicalStateString(PhysicalState::Liquid) + ", 3 = "
        + getPhysicalStateString(PhysicalState::Gas) + "): ";
        std::cin >> stateInt;
        if (!isValidPhysicalState(stateInt)) {
            std::cout << "Некоректний стан. Спробуйте ще раз.\n";
        }
    } while (!isValidPhysicalState(stateInt));
    std::cin.ignore();

    do {
        std::cout << "Введіть дату вивезення (ДД:ММ:РРРР): ";
        std::getline(std::cin, removalDate);
        if (!isValidDate(removalDate)) {
            std::cout << "Некоректна дата\n";
        }
    } while (!isValidDate(removalDate));

    do {
        std::cout << "Введіть кількість: ";
        std::cin >> quantity;
        if (quantity < 1) {
            std::cout << "Некоректна кількість. Спробуйте ще раз.\n";
        }
    } while (quantity < 1);

    do {
        std::cout << "Введіть вартість: ";
        std::cin >> cost;
        if (cost <= 0) {
            std::cout << "Некоректна вартість. Спробуйте ще раз.\n";
        }
    } while (cost <= 0);

    std::cin.ignore();

    return {
        companyCode, companyName, address, phone, wasteCode, wasteName,
        static_cast<PhysicalState>(stateInt), removalDate, quantity, cost
    };
}

void menu(Queue& queue) {
    while (true) {
        std::cout << "\n===== МЕНЮ =====\n"
            << "1. Додати запис\n"
            << "2. Видалити запис\n"
            << "3. Переглянути перший запис\n"
            << "4. Вивести всі записи\n"
            << "0. Вихід\n"
            << "Введіть свій вибір: ";

        int input;
        std::cin >> input;
        std::cin.ignore();

        switch (static_cast<MenuChoice>(input)) {
        case MenuChoice::ADD_RECORD: {
            WasteRecord newRecord = inputWasteRecord();
            try {
                enqueue(queue, newRecord);
                std::cout << "Запис додано до черги!\n";
            }
            catch (const std::exception& ex) {
                std::cout << "Помилка під час додавання запису: " << ex.what() << std::endl;
            }
            break;
        }
        case MenuChoice::REMOVE_RECORD: {
            try {
                WasteRecord removed = dequeue(queue);
                std::cout << "Видалено: " << removed.companyName << " - " << removed.wasteName << std::endl;
            }
            catch (const std::out_of_range& ex) {
                std::cout << "Помилка під час видалення запису: "  << ex.what() << std::endl;
            }
            break;
        }
        case MenuChoice::PEEK_RECORD: {
            try {
                WasteRecord record = peek(queue);
                std::cout << "Переглянуто: " << record.companyName << " - " << record.wasteName << std::endl;
            }
            catch (const std::out_of_range& ex) {
                std::cout << "Помилка під час перегляду запису: " << ex.what() << std::endl;
            }
            break;
        }
        case MenuChoice::PRINT_QUEUE: {
            printQueue(queue);
            break;
        }
        case MenuChoice::EXIT: {
            clearQueue(queue);
            std::cout << "Вихід...\n";
            return;
        }
        default: {
            std::cout << "Неправильний вибір. Спробуйте ще раз.\n";
            break;
        }
        }
    }
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Queue queue;
    menu(queue);
    return 0;
}