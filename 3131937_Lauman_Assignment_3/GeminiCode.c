#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

// Helper function to map sender category to priority rank
int getSenderPriority(const std::string& category) {
    if (category == "Boss") return 5;
    if (category == "Subordinate") return 4;
    if (category == "Peer") return 3;
    if (category == "ImportantPerson") return 2;
    if (category == "OtherPerson") return 1;
    return 0;
}

// Helper function to convert MM-DD-YYYY to YYYYMMDD integer for chronological comparison
int parseDate(const std::string& dateStr) {
    if (dateStr.length() < 10) return 0;
    int month = std::stoi(dateStr.substr(0, 2));
    int day = std::stoi(dateStr.substr(3, 2));
    int year = std::stoi(dateStr.substr(6, 4));
    return year * 10000 + month * 100 + day;
}

class Email {
private:
    std::string senderCategory;
    std::string subject;
    std::string dateStr;
    int priorityRank;
    int dateValue;
    int arrivalOrder;

public:
    Email(std::string sender, std::string subj, std::string date, int arrival)
        : senderCategory(sender), subject(subj), dateStr(date), arrivalOrder(arrival) {
        priorityRank = getSenderPriority(senderCategory);
        dateValue = parseDate(dateStr);
    }

    // Determines if this email has higher priority than 'other'
    bool isHigherPriorityThan(const Email& other) const {
        if (priorityRank != other.priorityRank) {
            return priorityRank > other.priorityRank;
        }
        if (dateValue != other.dateValue) {
            return dateValue > other.dateValue; // Newest email has higher priority
        }
        return arrivalOrder < other.arrivalOrder; // FIFO tie-breaker for identical category & date
    }

    std::string getSender() const { return senderCategory; }
    std::string getSubject() const { return subject; }
    std::string getDate() const { return dateStr; }
};

class MaxHeap {
private:
    std::vector<Email> heap;

    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[index].isHigherPriorityThan(heap[parent])) {
                std::swap(heap[index], heap[parent]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void heapifyDown(int index) {
        int size = heap.size();
        while (true) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int largest = index;

            if (left < size && heap[left].isHigherPriorityThan(heap[largest])) {
                largest = left;
            }
            if (right < size && heap[right].isHigherPriorityThan(heap[largest])) {
                largest = right;
            }

            if (largest != index) {
                std::swap(heap[index], heap[largest]);
                index = largest;
            } else {
                break;
            }
        }
    }

public:
    void insert(const Email& email) {
        heap.push_back(email);
        heapifyUp(heap.size() - 1);
    }

    Email getMax() const {
        return heap[0];
    }

    void extractMax() {
        if (heap.empty()) return;
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty()) {
            heapifyDown(0);
        }
    }

    bool isEmpty() const {
        return heap.empty();
    }

    int size() const {
        return heap.size();
    }
};

class EmailSystem {
private:
    MaxHeap inboxHeap;
    int arrivalCounter;

public:
    EmailSystem() : arrivalCounter(0) {}

    void addEmail(const std::string& sender, const std::string& subject, const std::string& date) {
        Email newEmail(sender, subject, date, arrivalCounter++);
        inboxHeap.insert(newEmail);
    }

    void handleNext() const {
        if (inboxHeap.isEmpty()) {
            std::cout << "Inbox is empty.\n";
        } else {
            Email topEmail = inboxHeap.getMax();
            std::cout << "Next email:\n";
            std::cout << "Sender: " << topEmail.getSender() << "\n";
            std::cout << "Subject: " << topEmail.getSubject() << "\n";
            std::cout << "Date: " << topEmail.getDate() << "\n";
        }
    }

    void handleRead() {
        if (inboxHeap.isEmpty()) {
            std::cout << "Inbox is empty. No email to read.\n";
        } else {
            inboxHeap.extractMax();
        }
    }

    void handleCount() const {
        std::cout << "There are " << inboxHeap.size() << " emails to read.\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <test_file>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << argv[1] << "\n";
        return 1;
    }

    EmailSystem system;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Handle Windows carriage returns if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        size_t firstSpace = line.find(' ');
        std::string cmd = (firstSpace == std::string::npos) ? line : line.substr(0, firstSpace);

        if (cmd == "EMAIL") {
            size_t comma1 = line.find(',', firstSpace);
            size_t comma2 = line.find(',', comma1 + 1);
            if (comma1 != std::string::npos && comma2 != std::string::npos) {
                std::string sender = line.substr(firstSpace + 1, comma1 - (firstSpace + 1));
                std::string subject = line.substr(comma1 + 1, comma2 - (comma1 + 1));
                std::string date = line.substr(comma2 + 1);
                system.addEmail(sender, subject, date);
            }
        } else if (cmd == "NEXT") {
            system.handleNext();
        } else if (cmd == "READ") {
            system.handleRead();
        } else if (cmd == "COUNT") {
            system.handleCount();
        }
    }

    file.close();
    return 0;
}