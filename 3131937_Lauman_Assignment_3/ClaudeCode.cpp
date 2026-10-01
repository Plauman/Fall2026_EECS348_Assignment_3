// CEO Email Prioritizer
// Build: g++ -std=c++17 -Wall -Wextra -o ceo_inbox ceo_inbox.cpp
// Run:   ./ceo_inbox commands.txt

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Email: one message plus the logic that decides which of two emails wins.
// ---------------------------------------------------------------------------
class Email {
public:
    Email() : rank_(0), dateKey_(0), sequence_(0) {}

    Email(const std::string& sender, const std::string& subject,
          const std::string& date, int rank, int dateKey, long sequence)
        : sender_(sender), subject_(subject), date_(date),
          rank_(rank), dateKey_(dateKey), sequence_(sequence) {}

    const std::string& sender() const { return sender_; }
    const std::string& subject() const { return subject_; }
    const std::string& date() const { return date_; }

    // True if this email should be read before `other`.
    bool hasHigherPriorityThan(const Email& other) const {
        if (rank_ != other.rank_) return rank_ > other.rank_;          // better sender first
        if (dateKey_ != other.dateKey_) return dateKey_ > other.dateKey_; // newer first
        return sequence_ < other.sequence_;                             // arrival order tie-break
    }

private:
    std::string sender_;
    std::string subject_;
    std::string date_;
    int rank_;      // higher = more important sender
    int dateKey_;   // YYYYMMDD so dates compare as integers
    long sequence_; // arrival order, used only to break exact ties
};

// ---------------------------------------------------------------------------
// MaxHeap: hand-built binary max-heap stored in a list (std::vector used purely
// as a dynamic array; no heap algorithms or priority_queue are used).
// ---------------------------------------------------------------------------
class MaxHeap {
public:
    bool isEmpty() const { return items_.empty(); }
    std::size_t size() const { return items_.size(); }

    void insert(const Email& e) {
        items_.push_back(e);
        siftUp(items_.size() - 1);
    }

    // Returns the highest-priority email without removing it. Heap must be non-empty.
    const Email& peekMax() const { return items_[0]; }

    // Removes the highest-priority email. Heap must be non-empty.
    void removeMax() {
        items_[0] = items_.back();
        items_.pop_back();
        if (!items_.empty()) siftDown(0);
    }

private:
    std::vector<Email> items_;

    static std::size_t parent(std::size_t i) { return (i - 1) / 2; }
    static std::size_t leftChild(std::size_t i) { return 2 * i + 1; }
    static std::size_t rightChild(std::size_t i) { return 2 * i + 2; }

    void swapItems(std::size_t a, std::size_t b) {
        Email tmp = items_[a];
        items_[a] = items_[b];
        items_[b] = tmp;
    }

    void siftUp(std::size_t i) {
        while (i > 0) {
            std::size_t p = parent(i);
            if (!items_[i].hasHigherPriorityThan(items_[p])) break;
            swapItems(i, p);
            i = p;
        }
    }

    void siftDown(std::size_t i) {
        const std::size_t n = items_.size();
        while (true) {
            std::size_t l = leftChild(i);
            std::size_t r = rightChild(i);
            std::size_t best = i;
            if (l < n && items_[l].hasHigherPriorityThan(items_[best])) best = l;
            if (r < n && items_[r].hasHigherPriorityThan(items_[best])) best = r;
            if (best == i) break;
            swapItems(i, best);
            i = best;
        }
    }
};

// ---------------------------------------------------------------------------
// Inbox: the CEO's inbox. Owns the heap and implements the four operations.
// ---------------------------------------------------------------------------
class Inbox {
public:
    Inbox() : nextSequence_(0) {}

    // Returns false if the sender category or date is invalid.
    bool addEmail(const std::string& sender, const std::string& subject,
                  const std::string& date) {
        int rank = senderRank(sender);
        int dateKey = dateToKey(date);
        if (rank == 0 || dateKey < 0) return false;
        heap_.insert(Email(sender, subject, date, rank, dateKey, nextSequence_++));
        return true;
    }

    void showNext(std::ostream& out) const {
        if (heap_.isEmpty()) {
            out << "No emails to read." << std::endl;
            return;
        }
        const Email& e = heap_.peekMax();
        out << "Next email:" << std::endl
            << "Sender: " << e.sender() << std::endl
            << "Subject: " << e.subject() << std::endl
            << "Date: " << e.date() << std::endl;
    }

    void readNext(std::ostream& out) {
        if (heap_.isEmpty()) {
            out << "No emails to read." << std::endl;
            return;
        }
        heap_.removeMax();
    }

    void showCount(std::ostream& out) const {
        out << "There are " << heap_.size() << " emails to read." << std::endl;
    }

private:
    MaxHeap heap_;
    long nextSequence_;

    // Boss > Subordinate > Peer > ImportantPerson > OtherPerson. 0 = unknown.
    static int senderRank(const std::string& s) {
        if (s == "Boss") return 5;
        if (s == "Subordinate") return 4;
        if (s == "Peer") return 3;
        if (s == "ImportantPerson") return 2;
        if (s == "OtherPerson") return 1;
        return 0;
    }

    // MM-DD-YYYY -> YYYYMMDD, or -1 if malformed.
    static int dateToKey(const std::string& d) {
        if (d.size() != 10 || d[2] != '-' || d[5] != '-') return -1;
        for (std::size_t i = 0; i < d.size(); ++i) {
            if (i == 2 || i == 5) continue;
            if (d[i] < '0' || d[i] > '9') return -1;
        }
        int month = std::stoi(d.substr(0, 2));
        int day = std::stoi(d.substr(3, 2));
        int year = std::stoi(d.substr(6, 4));
        if (month < 1 || month > 12 || day < 1 || day > 31) return -1;
        return year * 10000 + month * 100 + day;
    }
};

// ---------------------------------------------------------------------------
// CommandProcessor: reads the test file and dispatches commands to the Inbox.
// ---------------------------------------------------------------------------
class CommandProcessor {
public:
    explicit CommandProcessor(Inbox& inbox) : inbox_(inbox) {}

    bool processFile(const std::string& filename) {
        std::ifstream in(filename.c_str());
        if (!in) return false;
        std::string line;
        while (std::getline(in, line)) processLine(line);
        return true;
    }

private:
    Inbox& inbox_;

    static std::string trim(const std::string& s) {
        const char* ws = " \t\r\n";
        std::size_t b = s.find_first_not_of(ws);
        if (b == std::string::npos) return "";
        std::size_t e = s.find_last_not_of(ws);
        return s.substr(b, e - b + 1);
    }

    void processLine(const std::string& rawLine) {
        std::string line = trim(rawLine);
        if (line.empty()) return;

        std::size_t space = line.find_first_of(" \t");
        std::string command = (space == std::string::npos) ? line : line.substr(0, space);
        std::string rest = (space == std::string::npos) ? "" : trim(line.substr(space + 1));

        if (command == "EMAIL") {
            handleEmail(rest);
        } else if (command == "NEXT") {
            inbox_.showNext(std::cout);
        } else if (command == "READ") {
            inbox_.readNext(std::cout);
        } else if (command == "COUNT") {
            inbox_.showCount(std::cout);
        } else {
            std::cerr << "Unknown command: " << line << std::endl;
        }
    }

    // rest looks like: "Boss,Quarterly report,03-15-2024"
    // The subject may itself contain commas: sender is before the first comma,
    // date is after the last comma, everything in between is the subject.
    void handleEmail(const std::string& rest) {
        std::size_t first = rest.find(',');
        std::size_t last = rest.rfind(',');
        if (first == std::string::npos || first == last) {
            std::cerr << "Malformed EMAIL command: " << rest << std::endl;
            return;
        }
        std::string sender = trim(rest.substr(0, first));
        std::string subject = trim(rest.substr(first + 1, last - first - 1));
        std::string date = trim(rest.substr(last + 1));

        if (!inbox_.addEmail(sender, subject, date)) {
            std::cerr << "Invalid EMAIL ignored: " << rest << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <test-file>" << std::endl;
        return 1;
    }

    Inbox inbox;
    CommandProcessor processor(inbox);
    if (!processor.processFile(argv[1])) {
        std::cerr << "Error: cannot open file '" << argv[1] << "'" << std::endl;
        return 1;
    }
    return 0;
}