#include <iostream>
#include <string>
#include <utility>
#include <windows.h>
#include <regex>
#include <vector>
#include <set>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <fstream>

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
    UPDATE_RECORD = 2,
    REMOVE_RECORD = 3,
    PEEK_RECORD = 4,
    PRINT_QUEUE = 5,
    CLEAR_QUEUE = 6,
    COMPANY_LIST_BY_WASTE_TYPE_AND_DATE = 7,
    CALCULATE_PRICE_BY_WASTE_TYPE_AND_COMPANY = 8,
    SEARCH_COMPANIES_BY_WASTE_TYPE = 9,
    CALCULATE_WASTE_COUNT_BY_COMPANY_AND_RANGE_DATE = 10,
    SORT_BY_COUNT_THEN_PRICE = 11,
    SAVE_TO_FILE = 12,
    LOAD_FROM_FILE = 13
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


// --- Forward declarations ---
std::string getLineWithPrompt(const std::string& prompt);
int getIntWithPrompt(const std::string& prompt, int minVal = std::numeric_limits<int>::min(), int maxVal = std::numeric_limits<int>::max());
double getDoubleWithPrompt(const std::string& prompt, double minVal = -std::numeric_limits<double>::max(), double maxVal = std::numeric_limits<double>::max());
bool getYesNoInput(const std::string& prompt);
int inputPhysicalState(const std::string& prompt = "Введіть агрегатний стан");
std::string inputDate(const std::string& promptMessage);
void printSingleRecordDetails(const WasteRecord& record, int recordNumber = -1);
// --- End forward declarations ---


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

void printSingleRecordDetails(const WasteRecord& record, const int recordNumber) {
    if (recordNumber != -1) {
         std::cout << "Запис #" << recordNumber << "\n";
    }
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Код підприємства: " << record.companyCode << "\n";
    std::cout << "  Назва підприємства: " << record.companyName << "\n";
    std::cout << "  Адреса:      " << record.address << "\n";
    std::cout << "  Телефон:        " << record.phone << "\n";
    std::cout << "  Код відходу:   " << record.wasteCode << "\n";
    std::cout << "  Назва відходу:   " << record.wasteName << "\n";
    std::cout << "  Агрегатний стан:        " << getPhysicalStateString(record.state) << "\n";
    std::cout << "  Дата вивезення: " << record.removalDate << "\n";
    std::cout << "  Кількість:     " << record.quantity << "\n";
    std::cout << "  Вартість:         " << record.cost << " грн\n";
    std::cout << "  ---------------------" << std::endl;
}


void printQueue(const Queue& queue) {
    if (isEmpty(queue)) {
        std::cout << "Черга порожня!" << std::endl;
        return;
    }

    const WasteNode* current = queue.head;
    std::cout << "\n===== ВМІСТ ЧЕРГИ =====\n";
    int recordNumber = 1;
    while (current != nullptr) {
        printSingleRecordDetails(current->data, recordNumber++);
        current = current->next;
    }
}

void enqueue(Queue& queue, const WasteRecord& record) {
    WasteNode* newNode = new WasteNode(record);

    if (isEmpty(queue)) {
        queue.head = queue.tail = newNode;
    }
    else {
        if (queue.tail != nullptr) {
            queue.tail->next = newNode;
            queue.tail = newNode;
        }
        else {
            queue.head->next = newNode;
            queue.tail = newNode;
        }
    }
}

WasteRecord dequeue(Queue& queue) {
    if (isEmpty(queue)) {
        throw std::out_of_range("Черга порожня");
    }

    const WasteNode* tempNode = queue.head;
    WasteRecord removedData = tempNode->data;
    queue.head = tempNode->next;

    if (isEmpty(queue)) {
        queue.tail = nullptr;
    }

    delete tempNode;
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
    if (year < 1900 || year > 2025) return false;

    int daysInMonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    if (isLeapYear(year)) {
        daysInMonth[1] = 29;
    }

    return day >= 1 && day <= daysInMonth[month - 1];
}

std::string convertDateToComparableFormat(const std::string& date_ddmmyyyy) {
    if (date_ddmmyyyy.length() != 10 || date_ddmmyyyy[2] != ':' || date_ddmmyyyy[5] != ':') {
        throw std::invalid_argument("Неправильний формат дати для конвертації: " + date_ddmmyyyy);
    }
    return date_ddmmyyyy.substr(6, 4) + date_ddmmyyyy.substr(3, 2) + date_ddmmyyyy.substr(0, 2);
}


std::string getLineWithPrompt(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

int getIntWithPrompt(const std::string& prompt, const int minVal, const int maxVal) {
    std::string line;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, line);
        try {
            int value = std::stoi(line);
            if (value >= minVal && value <= maxVal) {
                return value;
            }
            std::cout << "Значення має бути в діапазоні [" << minVal << ", " << maxVal << "]. Спробуйте ще раз.\n";
        } catch (const std::invalid_argument&) {
            std::cout << "Некоректний ввід. Введіть ціле число.\n";
        } catch (const std::out_of_range&) {
            std::cout << "Число занадто велике або занадто мале. Спробуйте ще раз.\n";
        }
    }
}

double getDoubleWithPrompt(const std::string& prompt, const double minVal, const double maxVal) {
    std::string line;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, line);
        try {
            double value = std::stod(line);
             if (value >= minVal && value <= maxVal) {
                return value;
             }
            std::cout << "Значення має бути в діапазоні [" << minVal << ", " << maxVal << "]. Спробуйте ще раз.\n";
        } catch (const std::invalid_argument&) {
            std::cout << "Некоректний ввід. Введіть число (можливо, з десятковою крапкою).\n";
        } catch (const std::out_of_range&) {
            std::cout << "Число занадто велике або занадто мале. Спробуйте ще раз.\n";
        }
    }
}


bool getYesNoInput(const std::string& prompt) {
    std::string input;
    while (true) {
        std::cout << prompt << " (Y/N): ";
        std::getline(std::cin, input);
        if (input.length() == 1) {
            char choice = toupper(input[0]);
            if (choice == 'Y') {
                return true;
            }
            return false;
        }
        std::cout << "Некоректний ввід. Будь ласка, введіть Y або N.\n";
    }
}

int inputPhysicalState(const std::string& prompt) {
    int state;
    std::string line;
    do {
        std::cout << prompt <<" (" << static_cast<int>(PhysicalState::Solid) << " = " +
            getPhysicalStateString(PhysicalState::Solid) + ", " << static_cast<int>(PhysicalState::Liquid) << " = "
        + getPhysicalStateString(PhysicalState::Liquid) + ", " << static_cast<int>(PhysicalState::Gas) << " = "
        + getPhysicalStateString(PhysicalState::Gas) + "): ";
        std::getline(std::cin, line);
        try {
            state = std::stoi(line);
            if (!isValidPhysicalState(state)) {
                std::cout << "Некоректний стан. Спробуйте ще раз.\n";
            }
        } catch (const std::invalid_argument& e) {
            std::cout << "Некоректний ввід. Введіть число.\n";
            state = 0;
        } catch (const std::out_of_range& e) {
            std::cout << "Число занадто велике. Спробуйте ще раз.\n";
            state = 0;
        }
    } while (!isValidPhysicalState(state));

    return state;
}

std::string inputDate(const std::string& promptMessage) {
    std::string date;
    do {
        std::cout << promptMessage << " (ДД:ММ:РРРР): ";
        std::getline(std::cin, date);
        if (!isValidDate(date)) {
            std::cout << "Некоректна дата або формат. Спробуйте ще раз.\n";
        }
    } while (!isValidDate(date));

    return date;
}

WasteRecord inputWasteRecord() {
    const std::string companyCode = getLineWithPrompt("Введіть код підприємства: ");
    const std::string companyName = getLineWithPrompt("Введіть назву підприємства: ");
    const std::string address = getLineWithPrompt("Введіть адресу: ");
    const std::string phone = getLineWithPrompt("Введіть номер телефону: ");
    const std::string wasteCode = getLineWithPrompt("Введіть код відходу: ");
    const std::string wasteName = getLineWithPrompt("Введіть назву відходу: ");

    const PhysicalState state = static_cast<PhysicalState>(inputPhysicalState());
    const std::string removalDate = inputDate("Введіть дату вивезення");
    const int quantity = getIntWithPrompt("Введіть кількість: ", 1);
    const double cost = getDoubleWithPrompt("Введіть вартість: ", 0.01);

    return WasteRecord(companyCode, companyName, address, phone, wasteCode, wasteName,
                       state, removalDate, quantity, cost);
}

void printCompaniesByWasteTypeAndDate(const Queue& queue) {
    if (isEmpty(queue)) {
        std::cout << "Черга порожня. Немає даних для пошуку.\n";
        return;
    }

    const std::string targetWasteName = getLineWithPrompt("Введіть назву виду відходу для пошуку: ");
    const std::string targetDate = inputDate("Введіть дату вивезення для пошуку");

    std::set<std::string> foundCompanies;
    const WasteNode* current = queue.head;
    while (current != nullptr) {
        if (current->data.wasteName == targetWasteName && current->data.removalDate == targetDate) {
            foundCompanies.insert(current->data.companyName);
        }
        current = current->next;
    }

    if (foundCompanies.empty()) {
        std::cout << "Не знайдено підприємств, які вивозили '" << targetWasteName
                  << "' на дату " << targetDate << ".\n";
    } else {
        std::cout << "\nСписок підприємств, які вивозили '" << targetWasteName
                  << "' на дату " << targetDate << ":\n";
        for (const std::string& companyName : foundCompanies) {
            std::cout << "- " << companyName << std::endl;
        }
    }
}

void calculateServiceCostByWasteTypeAndCompany(const Queue& queue) {
    if (isEmpty(queue)) {
        std::cout << "Черга порожня. Немає даних для розрахунку.\n";
        return;
    }

    const std::string targetCompanyName = getLineWithPrompt("Введіть назву підприємства для розрахунку вартості: ");
    const std::string targetWasteName = getLineWithPrompt("Введіть назву виду відходу: ");

    double totalCost = 0.0;
    bool foundRecords = false;
    const WasteNode* current = queue.head;
    while (current != nullptr) {
        if (current->data.companyName == targetCompanyName && current->data.wasteName == targetWasteName) {
            totalCost += current->data.cost;
            foundRecords = true;
        }
        current = current->next;
    }

    std::cout << std::fixed << std::setprecision(2);
    if (foundRecords) {
        std::cout << "Загальна вартість вивезення відходу '" << targetWasteName
                  << "' для підприємства '" << targetCompanyName << "' складає: "
                  << totalCost << " грн.\n";
    } else {
        std::cout << "Не знайдено записів для підприємства '" << targetCompanyName
                  << "' з видом відходу '" << targetWasteName << "' для розрахунку вартості.\n";
    }
}

void findCompaniesByPhysicalState(const Queue& queue) {
    if (isEmpty(queue)) {
        std::cout << "Черга порожня. Немає даних для пошуку.\n";
        return;
    }

    std::cout << "Пошук підприємств за агрегатним станом відходів.\n";
    const PhysicalState targetState = static_cast<PhysicalState>(inputPhysicalState());
    const std::string targetStateStr = getPhysicalStateString(targetState);

    std::set<std::string> foundCompanies;
    const WasteNode* current = queue.head;
    while (current != nullptr) {
        if (current->data.state == targetState) {
            foundCompanies.insert(current->data.companyName);
        }
        current = current->next;
    }

     if (foundCompanies.empty()) {
        std::cout << "Не знайдено підприємств, які вивозять відходи в агрегатному стані: '"
                  << targetStateStr << "'.\n";
     } else {
         std::cout << "\nСписок підприємств, які вивозять відходи в агрегатному стані '"
                   << targetStateStr << "':\n";

         for (const std::string& companyName : foundCompanies) {
             std::cout << "- " << companyName << std::endl;
         }
     }
}

void calculateWasteCountByCompanyAndDateRange(const Queue& queue) {
    if (isEmpty(queue)) {
        std::cout << "Черга порожня. Немає даних для розрахунку.\n";
        return;
    }

    const std::string targetCompanyName = getLineWithPrompt("Введіть назву підприємства для розрахунку кількості відходів: ");
    const std::string startDateStr = inputDate("Введіть початкову дату діапазону");
    const std::string endDateStr = inputDate("Введіть кінцеву дату діапазону");

    std::string comparableStartDate, comparableEndDate;
    try {
        comparableStartDate = convertDateToComparableFormat(startDateStr);
        comparableEndDate = convertDateToComparableFormat(endDateStr);
    } catch (const std::invalid_argument& ex) {
        std::cout << "Помилка: " << ex.what() << std::endl;
        return;
    }

    if (comparableStartDate > comparableEndDate) {
        std::cout << "Помилка: початкова дата (" << startDateStr
                  << ") не може бути пізніше кінцевої дати (" << endDateStr << ").\n";
        return;
    }

    long long totalQuantity = 0;
    bool foundRecords = false;
    const WasteNode* current = queue.head;

    while (current != nullptr) {
        if (current->data.companyName == targetCompanyName) {
            std::string recordDateComparable;
            try {
                recordDateComparable = convertDateToComparableFormat(current->data.removalDate);
            } catch (const std::invalid_argument&) {
                std::cerr << "Попередження: некоректна дата в записі для '" << current->data.companyName << "': " << current->data.removalDate << ". Запис пропущено.\n";
                current = current->next;
                continue;
            }

            if (recordDateComparable >= comparableStartDate && recordDateComparable <= comparableEndDate) {
                totalQuantity += current->data.quantity;
                foundRecords = true;
            }
        }
        current = current->next;
    }

    if (foundRecords) {
        std::cout << "Загальна кількість відходів, вивезених підприємством '" << targetCompanyName
                  << "' з " << startDateStr << " по " << endDateStr << ", складає: "
                  << totalQuantity << " од.\n";
    } else {
        std::cout << "Не знайдено записів про вивезення відходів підприємством '" << targetCompanyName
                  << "' в заданому діапазоні дат (" << startDateStr << " - " << endDateStr << ").\n";
    }
}

void sortQueueByQuantityThenCost(Queue& queue) {
    if (isEmpty(queue) || queue.head->next == nullptr) {
        return;
    }

    std::vector<WasteRecord> records;
    while (!isEmpty(queue)) {
        records.push_back(dequeue(queue));
    }

    std::sort(records.begin(), records.end(), [](const WasteRecord& left, const WasteRecord& right) {
        if (left.quantity != right.quantity) {
            return left.quantity < right.quantity;
        }
        return left.cost < right.cost;
    });

    for (const WasteRecord& record : records) {
        enqueue(queue, record);
    }
}

void updateRecord(Queue& queue) {
    if (isEmpty(queue)) {
        std::cout << "Черга порожня. Немає записів для редагування.\n";
        return;
    }

    std::string searchCompanyName = getLineWithPrompt("Введіть назву підприємства для пошуку записів: ");

    std::vector<WasteNode*> matchingNodes;
    WasteNode* current = queue.head;
    while (current != nullptr) {
        if (current->data.companyName == searchCompanyName) {
            matchingNodes.push_back(current);
        }
        current = current->next;
    }

    if (matchingNodes.empty()) {
        std::cout << "Не знайдено записів для підприємства '" << searchCompanyName << "'.\n";
        return;
    }

    std::cout << "\nЗнайдені записи для підприємства '" << searchCompanyName << "':\n";
    for (size_t i = 0; i < matchingNodes.size(); ++i) {
        std::cout << "--- Запис #" << i + 1 << " ---\n";
        printSingleRecordDetails(matchingNodes[i]->data, -1);
    }

    int choice = getIntWithPrompt("Введіть номер запису для редагування (0 для скасування): ", 0, matchingNodes.size());

    if (choice == 0) {
        std::cout << "Редагування скасовано.\n";
        return;
    }

    WasteNode* nodeToUpdate = matchingNodes[choice - 1];

    std::string companyCode = nodeToUpdate->data.companyCode;
    std::string companyName = nodeToUpdate->data.companyName;
    std::string address = nodeToUpdate->data.address;
    std::string phone = nodeToUpdate->data.phone;
    std::string wasteCode = nodeToUpdate->data.wasteCode;
    std::string wasteName = nodeToUpdate->data.wasteName;
    PhysicalState state = nodeToUpdate->data.state;
    std::string removalDate = nodeToUpdate->data.removalDate;
    int quantity = nodeToUpdate->data.quantity;
    double cost = nodeToUpdate->data.cost;

    std::cout << "\n--- Редагування Запису --- \n";
    std::cout << "Поточні дані:\n";
    printSingleRecordDetails(nodeToUpdate->data, -1);

    std::cout << "\nВведіть нові дані (натисніть Enter, щоб не змінювати):\n";

    if (getYesNoInput("Змінити код підприємства (" + companyCode + ")?")) {
        companyCode = getLineWithPrompt("Новий код підприємства: ");
    }
    if (getYesNoInput("Змінити назву підприємства (" + companyName + ")?")) {
        companyName = getLineWithPrompt("Нова назва підприємства: ");
    }
    if (getYesNoInput("Змінити адресу (" + address + ")?")) {
        address = getLineWithPrompt("Нова адреса: ");
    }
    if (getYesNoInput("Змінити телефон (" + phone + ")?")) {
        phone = getLineWithPrompt("Новий телефон: ");
    }
    if (getYesNoInput("Змінити код відходу (" + wasteCode + ")?")) {
        wasteCode = getLineWithPrompt("Новий код відходу: ");
    }
    if (getYesNoInput("Змінити назву відходу (" + wasteName + ")?")) {
        wasteName = getLineWithPrompt("Нова назва відходу: ");
    }
    if (getYesNoInput("Змінити агрегатний стан (" + getPhysicalStateString(state) + ")?")) {
        state = static_cast<PhysicalState>(inputPhysicalState("Новий агрегатний стан"));
    }
    if (getYesNoInput("Змінити дату вивезення (" + removalDate + ")?")) {
        removalDate = inputDate("Нова дата вивезення");
    }
    if (getYesNoInput("Змінити кількість (" + std::to_string(quantity) + ")?")) {
        quantity = getIntWithPrompt("Нова кількість: ", 1);
    }
    if (getYesNoInput("Змінити вартість (" + std::to_string(cost) + ")?")) {
        cost = getDoubleWithPrompt("Нова вартість: ", 0.01);
    }

    std::cout << "\n--- Перевірка змін --- \n";
    // Створюємо тимчасовий об'єкт з новими даними для друку
    WasteRecord tempPrintRecord(companyCode, companyName, address, phone, wasteCode, wasteName, state, removalDate, quantity, cost);
    printSingleRecordDetails(tempPrintRecord, -1);

    if (getYesNoInput("Зберегти ці зміни?")) {
        // Оновлюємо дані в вузлі черги, створюючи новий WasteRecord
        nodeToUpdate->data = WasteRecord(companyCode, companyName, address, phone, wasteCode, wasteName,
                                        state, removalDate, quantity, cost);
        std::cout << "Запис успішно оновлено.\n";
    } else {
        std::cout << "Зміни скасовано.\n";
    }
}

const std::string DEFAULT_FILENAME = "waste_data.txt";
const std::string RECORD_SEPARATOR = "---END_RECORD---";

void saveQueueToFile(const Queue& queue, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Помилка: не вдалося відкрити файл для запису: " << filename << std::endl;
        return;
    }

    const WasteNode* current = queue.head;
    while (current != nullptr) {
        const WasteRecord& rec = current->data;
        outFile << rec.companyCode << std::endl;
        outFile << rec.companyName << std::endl;
        outFile << rec.address << std::endl;
        outFile << rec.phone << std::endl;
        outFile << rec.wasteCode << std::endl;
        outFile << rec.wasteName << std::endl;
        outFile << static_cast<int>(rec.state) << std::endl;
        outFile << rec.removalDate << std::endl;
        outFile << rec.quantity << std::endl;
        outFile << std::fixed << std::setprecision(2) << rec.cost << std::endl;
        outFile << RECORD_SEPARATOR << std::endl;
        current = current->next;
    }
    outFile.close();
    std::cout << "Дані успішно збережено у файл: " << filename << std::endl;
}

void loadQueueFromFile(Queue& queue, const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Попередження: не вдалося відкрити файл для читання: " << filename << std::endl;
        std::cout << "Буде використано порожню чергу." << std::endl;
        return;
    }

    clearQueue(queue);

    std::string line;
    // Тимчасові змінні для збору полів
    std::string companyCode, companyName, address, phone, wasteCode, wasteName, removalDateStr;
    int stateInt = 0, quantity = 0;
    double cost = 0.0;
    PhysicalState state = PhysicalState::Solid; // За замовчуванням

    int fieldCounter = 0;
    int recordLineNumber = 0; // Для повідомлень про помилки

    while (std::getline(inFile, line)) {
        recordLineNumber++;
        if (line == RECORD_SEPARATOR) {
            if (fieldCounter == 10) {
                if (!isValidDate(removalDateStr)) {
                    std::cerr << "Попередження (рядок " << recordLineNumber - 10 << "): Некоректна дата '" << removalDateStr << "' у записі для '" << companyName << "'. Запис пропущено.\n";
                } else if (!isValidPhysicalState(stateInt)) {
                     std::cerr << "Попередження (рядок " << recordLineNumber - 10 << "): Некоректний агрегатний стан '" << stateInt << "' у записі для '" << companyName << "'. Запис пропущено.\n";
                } else {
                    enqueue(queue, WasteRecord(companyCode, companyName, address, phone, wasteCode, wasteName,
                                               state, removalDateStr, quantity, cost));
                }
            } else {
                std::cerr << "Попередження (починаючи з рядка " << recordLineNumber - fieldCounter << "): Неповний запис у файлі перед '" << RECORD_SEPARATOR << "'. Пропущено.\n";
            }
            // Скидання змінних для наступного запису
            fieldCounter = 0;
            companyCode.clear(); companyName.clear(); address.clear(); phone.clear();
            wasteCode.clear(); wasteName.clear(); removalDateStr.clear();
            stateInt = 0; quantity = 0; cost = 0.0; state = PhysicalState::Solid;
            continue;
        }

        try {
            switch (fieldCounter) {
                case 0: companyCode = line; break;
                case 1: companyName = line; break;
                case 2: address = line; break;
                case 3: phone = line; break;
                case 4: wasteCode = line; break;
                case 5: wasteName = line; break;
                case 6: {
                    stateInt = std::stoi(line);
                    if (isValidPhysicalState(stateInt)) {
                        state = static_cast<PhysicalState>(stateInt);
                    } else {
                         std::cerr << "Попередження (рядок " << recordLineNumber << "): Некоректне значення агрегатного стану '" << line << "'. Встановлено стандартне Solid.\n";
                         state = PhysicalState::Solid;
                    }
                    break;
                }
                case 7: removalDateStr = line; break;
                case 8: quantity = std::stoi(line); break;
                case 9: cost = std::stod(line); break;
                default:
                    std::cerr << "Попередження (рядок " << recordLineNumber << "): Зайве поле у файлі: " << line << ". Пропущено.\n";
                    break;
            }
            fieldCounter++;
        } catch (const std::invalid_argument& ex) {
            std::cerr << "Помилка парсингу (рядок " << recordLineNumber << ", поле " << fieldCounter << ", значення '" << line << "'): " << ex.what() << ". Запис буде пропущено.\n";
            while(std::getline(inFile, line) && line != RECORD_SEPARATOR) {recordLineNumber++;} // Пропустити до кінця поточного запису
            fieldCounter = 0;
            companyCode.clear(); companyName.clear(); address.clear(); phone.clear();
            wasteCode.clear(); wasteName.clear(); removalDateStr.clear();
            stateInt = 0; quantity = 0; cost = 0.0; state = PhysicalState::Solid;
        } catch (const std::out_of_range& ex) {
            std::cerr << "Помилка діапазону (рядок " << recordLineNumber << ", поле " << fieldCounter << ", значення '" << line << "'): " << ex.what() << ". Запис буде пропущено.\n";
            while(std::getline(inFile, line) && line != RECORD_SEPARATOR) {recordLineNumber++;}
            fieldCounter = 0;
            companyCode.clear(); companyName.clear(); address.clear(); phone.clear();
            wasteCode.clear(); wasteName.clear(); removalDateStr.clear();
            stateInt = 0; quantity = 0; cost = 0.0; state = PhysicalState::Solid;
        }
    }

    inFile.close();
    if (fieldCounter > 0 && fieldCounter < 10) {
        std::cerr << "Попередження: Файл закінчився на неповному записі (починаючи з рядка " << recordLineNumber - fieldCounter +1 << "). Останній неповний запис пропущено.\n";
    }
    std::cout << "Дані успішно завантажено з файлу: " << filename << std::endl;
}


void menu(Queue& queue) {
    while (true) {
        std::cout << "\n===== МЕНЮ =====\n"
            << static_cast<int>(MenuChoice::ADD_RECORD) << ". Додати запис\n"
            << static_cast<int>(MenuChoice::UPDATE_RECORD) << ". Редагувати запис\n"
            << static_cast<int>(MenuChoice::REMOVE_RECORD) << ". Видалити запис\n"
            << static_cast<int>(MenuChoice::PEEK_RECORD) << ". Переглянути перший запис\n"
            << static_cast<int>(MenuChoice::PRINT_QUEUE) << ". Вивести всі записи\n"
            << static_cast<int>(MenuChoice::CLEAR_QUEUE) << ". Очистити чергу\n"
            << static_cast<int>(MenuChoice::COMPANY_LIST_BY_WASTE_TYPE_AND_DATE) << ". Список підприємств (вид відходів, дата)\n"
            << static_cast<int>(MenuChoice::CALCULATE_PRICE_BY_WASTE_TYPE_AND_COMPANY) << ". Вартість вивезення (вид відходів, підприємство)\n"
            << static_cast<int>(MenuChoice::SEARCH_COMPANIES_BY_WASTE_TYPE) << ". Пошук підприємств (агрегатний стан)\n"
            << static_cast<int>(MenuChoice::CALCULATE_WASTE_COUNT_BY_COMPANY_AND_RANGE_DATE) << ". К-сть відходів підприємства (діапазон дат)\n"
            << static_cast<int>(MenuChoice::SORT_BY_COUNT_THEN_PRICE) << ". Сортування (кількість, вартість)\n"
            << static_cast<int>(MenuChoice::SAVE_TO_FILE) << ". Зберегти дані у файл\n"
            << static_cast<int>(MenuChoice::LOAD_FROM_FILE) << ". Завантажити дані з файлу\n"
            << static_cast<int>(MenuChoice::EXIT) << ". Вихід\n"
            << "Введіть свій вибір: ";

        std::string line;
        std::getline(std::cin, line);
        int inputChoice = -1;

        try {
            inputChoice = std::stoi(line);
        } catch (const std::invalid_argument&) {
            std::cout << "Неправильний ввід. Будь ласка, введіть число.\n";
            continue;
        } catch (const std::out_of_range&) {
            std::cout << "Вибір поза допустимим діапазоном.\n";
            continue;
        }

        switch (static_cast<MenuChoice>(inputChoice)) {
        case MenuChoice::ADD_RECORD: {
            std::cout << "--- Додавання нового запису --- \n";
            enqueue(queue, inputWasteRecord());
            std::cout << "Запис додано до черги!\n";
            break;
        }
        case MenuChoice::UPDATE_RECORD: {
            updateRecord(queue);
            break;
        }
        case MenuChoice::REMOVE_RECORD: {
            try {
                WasteRecord removed = dequeue(queue);
                std::cout << "Видалено запис для підприємства: " << removed.companyName << " - " << removed.wasteName << std::endl;
            }
            catch (const std::out_of_range& ex) {
                std::cout << "Помилка під час видалення запису: "  << ex.what() << std::endl;
            }
            break;
        }
        case MenuChoice::PEEK_RECORD: {
            try {
                WasteRecord record = peek(queue);
                std::cout << "Перший запис в черзі:\n";
                printSingleRecordDetails(record, -1);
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
        case MenuChoice::CLEAR_QUEUE: {
            if (getYesNoInput("Ви впевнені, що хочете очистити ВСЮ чергу?")) {
                clearQueue(queue);
                std::cout << "Чергу очищено.\n";
            } else {
                std::cout << "Очищення скасовано.\n";
            }
            break;
        }
        case MenuChoice::COMPANY_LIST_BY_WASTE_TYPE_AND_DATE: {
            printCompaniesByWasteTypeAndDate(queue);
            break;
        }
        case MenuChoice::CALCULATE_PRICE_BY_WASTE_TYPE_AND_COMPANY: {
            calculateServiceCostByWasteTypeAndCompany(queue);
            break;
        }
        case MenuChoice::SEARCH_COMPANIES_BY_WASTE_TYPE: {
            findCompaniesByPhysicalState(queue);
            break;
        }
        case MenuChoice::CALCULATE_WASTE_COUNT_BY_COMPANY_AND_RANGE_DATE: {
            calculateWasteCountByCompanyAndDateRange(queue);
            break;
        }
        case MenuChoice::SORT_BY_COUNT_THEN_PRICE: {
            sortQueueByQuantityThenCost(queue);
            std::cout << "Чергу відсортовано за кількістю, а потім за вартістю послуги (за зростанням).\n";
            break;
        }
        case MenuChoice::SAVE_TO_FILE: {
            std::string filename = getLineWithPrompt("Введіть ім'я файлу для збереження (натисніть Enter для " + DEFAULT_FILENAME + "): ");
            if (filename.empty()) {
                filename = DEFAULT_FILENAME;
            }
            saveQueueToFile(queue, filename);
            break;
        }
        case MenuChoice::LOAD_FROM_FILE: {
             if (!isEmpty(queue)) {
                if (!getYesNoInput("Поточна черга не порожня. Завантаження перезапише її. Продовжити?")) {
                    std::cout << "Завантаження скасовано.\n";
                    break;
                }
            }
            std::string filename = getLineWithPrompt("Введіть ім'я файлу для завантаження (натисніть Enter для " + DEFAULT_FILENAME + "): ");
            if (filename.empty()) {
                filename = DEFAULT_FILENAME;
            }
            loadQueueFromFile(queue, filename);
            break;
        }
        case MenuChoice::EXIT: {
            if (getYesNoInput("Зберегти зміни перед виходом?")) {
                std::string filename = getLineWithPrompt("Введіть ім'я файлу для збереження (натисніть Enter для " + DEFAULT_FILENAME + "): ");
                if (filename.empty()) {
                    filename = DEFAULT_FILENAME;
                }
                saveQueueToFile(queue, filename);
            }
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

    enqueue(queue, WasteRecord("C001", "Рога та Копита", "м. Київ, вул. Центральна, 1", "044-123-45-67", "W01", "Побутові відходи", PhysicalState::Solid, "15:10:2023", 100, 500.00));
    enqueue(queue, WasteRecord("C002", "Чисте Місто", "м. Львів, пл. Ринок, 5", "032-987-65-43", "W02", "Будівельне сміття", PhysicalState::Solid, "15:10:2023", 250, 1200.50));
    enqueue(queue, WasteRecord("C001", "Рога та Копита", "м. Київ, вул. Центральна, 1", "044-123-45-67", "W03", "Рідкі хім. відходи", PhysicalState::Liquid, "16:10:2023", 50, 2000.75));
    enqueue(queue, WasteRecord("C003", "ЕкоСервіс", "м. Одеса, вул. Морська, 10", "048-111-22-33", "W01", "Побутові відходи", PhysicalState::Solid, "15:10:2023", 100, 550.25));
    enqueue(queue, WasteRecord("C001", "Рога та Копита", "м. Київ, вул. Центральна, 1", "044-123-45-67", "W01", "Побутові відходи", PhysicalState::Solid, "18:10:2023", 70, 350.00)); // Ще один запис для "Рога та Копита" для тестування вибору
    enqueue(queue, WasteRecord("C001", "Рога та Копита", "м. Київ, вул. Центральна, 1", "044-123-45-67", "W05", "Інші тверді", PhysicalState::Solid, "01:11:2023", 30, 150.00));
    enqueue(queue, WasteRecord("C004", "ГазТранс", "м. Харків, пр. Науки, 20", "057-222-33-44", "W04", "Промислові гази", PhysicalState::Gas, "20:10:2023", 10, 3000.00));
    enqueue(queue, WasteRecord("C002", "Чисте Місто", "м. Львів, пл. Ринок, 5", "032-987-65-43", "W05", "Відпрацьовані масла", PhysicalState::Liquid, "21:10:2023", 70, 800.00));


    menu(queue);
    return 0;
}