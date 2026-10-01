/*
Program: EECS 348 Assignment 3 - CEO Email Prioritizer

Description:
C++ object-oriented program that reads CEO inbox commands from a test
file and prioritizes unread emails using a hand-built MaxHeap.
Sender category determines the main priority. If two emails have the
same sender category, the newest email has the higher priority.

Inputs:
A command-line test file containing EMAIL, NEXT, READ, and COUNT commands.

Outputs:
COUNT displays the number of unread emails.
NEXT displays the highest-priority unread email.
READ removes the highest-priority unread email.

Collaborators:
None.

Other Sources:
Google Gemini and Claude were used as GenAI sources for the required
Assignment 3 comparison. Claudes generated program was selected as
the baseline. Modified the baseline for correctness,
maintainability, output behavior, and documentation.

Author: Peter Lauman
Creation Date: October 1, 2026
Revision Date: October 1, 2026

Revisions:
Improved the Claude baseline, kept priority logic inside objects, removed unnecessary
empty-inbox output, and added detailed comments.
*/

#include <fstream>   // Provides file input.
#include <iostream>  // Provides terminal output.
#include <string>    // Provides the string class.
#include <vector>    // Used as the list storage for the MaxHeap.


class Email
{
private:
    // Stores the sender category.
    std::string senderCategory;

    // Stores the email subject.
    std::string subject;

    // Stores the original date for output.
    std::string date;

    // Stores the numerical sender priority.
    int senderPriority;

    // Stores the date as YYYYMMDD.
    int dateValue;

    // Stores arrival order for exact ties.
    long arrivalOrder;

    /*
    Converts a sender category into a numerical priority.

    Boss has the highest priority.
    OtherPerson has the lowest priority.
    */
    static int categoryToPriority(const std::string& sender)
    {
        if (sender == "Boss")
        {
            return 5;
        }

        if (sender == "Subordinate")
        {
            return 4;
        }

        if (sender == "Peer")
        {
            return 3;
        }

        if (sender == "ImportantPerson")
        {
            return 2;
        }

        if (sender == "OtherPerson")
        {
            return 1;
        }

        return 0;
    }

    /*
    Converts a date from MM-DD-YYYY into YYYYMMDD.

    This makes newer dates have larger integer values.
    */
    static int dateToNumber(const std::string& emailDate)
    {
        // Extract the month.
        int month = std::stoi(emailDate.substr(0, 2));

        // Extract the day.
        int day = std::stoi(emailDate.substr(3, 2));

        // Extract the year.
        int year = std::stoi(emailDate.substr(6, 4));

        // Return the date in YYYYMMDD form.
        return (year * 10000) + (month * 100) + day;
    }

public:
    /*
    Default constructor.

    Initializes numerical values to zero.
    */
    Email()
        : senderPriority(0),
          dateValue(0),
          arrivalOrder(0)
    {
    }

    /*
    Main Email constructor.

    Stores the email information and calculates its priority values.
    */
    Email(
        const std::string& sender,
        const std::string& emailSubject,
        const std::string& emailDate,
        long order)
        : senderCategory(sender),
          subject(emailSubject),
          date(emailDate),
          senderPriority(categoryToPriority(sender)),
          dateValue(dateToNumber(emailDate)),
          arrivalOrder(order)
    {
    }

    /*
    Determines whether this email should appear before another email.

    Sender priority is checked first.

    If sender priorities are equal, the newest date is checked.

    Arrival order is used only if both priority and date are identical.
    */
    bool hasHigherPriorityThan(const Email& other) const
    {
        // Compare sender categories first.
        if (senderPriority != other.senderPriority)
        {
            return senderPriority > other.senderPriority;
        }

        // If sender categories match, compare dates.
        if (dateValue != other.dateValue)
        {
            return dateValue > other.dateValue;
        }

        // Preserve arrival order for exact ties.
        return arrivalOrder < other.arrivalOrder;
    }

    // Returns the sender category.
    const std::string& getSender() const
    {
        return senderCategory;
    }

    // Returns the email subject.
    const std::string& getSubject() const
    {
        return subject;
    }

    // Returns the original email date.
    const std::string& getDate() const
    {
        return date;
    }
};


/*
MaxHeap Class
Implements the priority queue using a binary MaxHeap.
A vector is used only as list-based storage.
No std::priority_queue or existing heap implementation is used.
*/
class MaxHeap
{
private:
    // Stores all unread emails in heap order.
    std::vector<Email> items;

    // Returns the index of a node's parent.
    static std::size_t parent(std::size_t index)
    {
        return (index - 1) / 2;
    }

    // Returns the index of the left child.
    static std::size_t leftChild(std::size_t index)
    {
        return (2 * index) + 1;
    }

    // Returns the index of the right child.
    static std::size_t rightChild(std::size_t index)
    {
        return (2 * index) + 2;
    }

    /*
    Swaps two Email objects inside the heap.
    */
    void swapItems(std::size_t first, std::size_t second)
    {
        // Temporarily save the first email.
        Email temporary = items[first];

        // Move the second email into the first position.
        items[first] = items[second];

        // Move the saved email into the second position.
        items[second] = temporary;
    }

    /*
    Moves a newly inserted email upward until the MaxHeap
    property is restored.
    */
    void siftUp(std::size_t index)
    {
        // Continue while the email is not the root.
        while (index > 0)
        {
            // Find the parent.
            std::size_t parentIndex = parent(index);

            // Stop if the email does not outrank its parent.
            if (!items[index].hasHigherPriorityThan(
                    items[parentIndex]))
            {
                break;
            }

            // Swap the email with its parent.
            swapItems(index, parentIndex);

            // Continue from the parent's old position.
            index = parentIndex;
        }
    }

    /*
    Moves the root downward after removal until the MaxHeap
    property is restored.
    */
    void siftDown(std::size_t index)
    {
        // Save the number of items currently in the heap.
        const std::size_t count = items.size();

        while (true)
        {
            // Find the child indexes.
            std::size_t left = leftChild(index);
            std::size_t right = rightChild(index);

            // Assume the current email has the highest priority.
            std::size_t highest = index;

            // Compare the left child when it exists.
            if (left < count &&
                items[left].hasHigherPriorityThan(items[highest]))
            {
                highest = left;
            }

            // Compare the right child when it exists.
            if (right < count &&
                items[right].hasHigherPriorityThan(items[highest]))
            {
                highest = right;
            }

            // Stop if the current email is already highest.
            if (highest == index)
            {
                break;
            }

            // Swap with the higher-priority child.
            swapItems(index, highest);

            // Continue checking from the new position.
            index = highest;
        }
    }

public:
    /*
    Returns true when no unread emails are stored.
    */
    bool isEmpty() const
    {
        return items.empty();
    }

    /*
    Returns the number of unread emails.
    */
    std::size_t size() const
    {
        return items.size();
    }

    /*
    Inserts a new email.

    Complexity: O(log n)
    */
    void insert(const Email& email)
    {
        // Add the email to the end of the list.
        items.push_back(email);

        // Move it upward to its proper position.
        siftUp(items.size() - 1);
    }

    /*
    Returns the highest-priority email without removing it.

    Complexity: O(1)
    */
    const Email& getMax() const
    {
        return items[0];
    }

    /*
    Removes the highest-priority email.

    Complexity: O(log n)
    */
    void removeMax()
    {
        // Replace the root with the last email.
        items[0] = items.back();

        // Remove the old final position.
        items.pop_back();

        // Restore heap order if emails remain.
        if (!items.empty())
        {
            siftDown(0);
        }
    }
};


/*
Inbox Class
Owns the MaxHeap and implements the required CEO inbox operations.
*/
class Inbox
{
private:
    // Stores unread emails.
    MaxHeap heap;

    // Tracks arrival order for exact priority ties.
    long nextArrivalOrder;

public:
    /*
    Constructs an empty inbox.
    */
    Inbox()
        : nextArrivalOrder(0)
    {
    }

    /*
    EMAIL operation.

    Creates an Email object and inserts it into the MaxHeap.
    */
    void addEmail(
        const std::string& sender,
        const std::string& subject,
        const std::string& date)
    {
        // Create the new Email object.
        Email email(
            sender,
            subject,
            date,
            nextArrivalOrder);

        // Increment the sequence for the next email.
        nextArrivalOrder++;

        // Insert the email into the priority queue.
        heap.insert(email);
    }

    /*
    NEXT operation.

    Displays the highest-priority unread email without removing it.
    */
    void showNext() const
    {
        // Safely handle NEXT when there are no emails.
        if (heap.isEmpty())
        {
            return;
        }

        // Obtain the highest-priority email.
        const Email& email = heap.getMax();

        // Display the required heading.
        std::cout << "Next email:\n";

        // Display the sender.
        std::cout
            << "Sender: "
            << email.getSender()
            << "\n";

        // Display the subject.
        std::cout
            << "Subject: "
            << email.getSubject()
            << "\n";

        // Display the date.
        std::cout
            << "Date: "
            << email.getDate()
            << "\n";
    }

    /*
    READ operation.

    Removes the highest-priority email without displaying it.
    */
    void readNext()
    {
        // Safely handle READ when no emails exist.
        if (heap.isEmpty())
        {
            return;
        }

        // Remove the highest-priority email.
        heap.removeMax();
    }

    /*
    COUNT operation.

    Displays the number of unread emails.
    */
    void showCount() const
    {
        std::cout
            << "There are "
            << heap.size()
            << " emails to read.\n";
    }
};


/*
CommandProcessor Class
Reads the test file and determines which Inbox operation
should be performed.
*/
class CommandProcessor
{
private:
    // Reference to the CEO's inbox.
    Inbox& inbox;

    /*
    Removes a Windows carriage-return character when necessary.

    This allows a Windows-created input file to work correctly
    when the program runs on the Linux Cycle Server.
    */
    static void removeCarriageReturn(std::string& line)
    {
        // Check for a carriage return at the end.
        if (!line.empty() && line.back() == '\r')
        {
            // Remove the carriage return.
            line.pop_back();
        }
    }

    /*
    Processes an EMAIL command.

    Expected format:
    EMAIL Sender,Subject,MM-DD-YYYY
    */
    void processEmail(const std::string& line)
    {
        // Find the comma after the sender.
        std::size_t firstComma =
            line.find(',');

        // Find the comma after the subject.
        std::size_t secondComma =
            line.find(',', firstComma + 1);

        // Extract the sender after "EMAIL ".
        std::string sender =
            line.substr(
                6,
                firstComma - 6);

        // Extract the subject between the commas.
        std::string subject =
            line.substr(
                firstComma + 1,
                secondComma - firstComma - 1);

        // Extract the date.
        std::string date =
            line.substr(secondComma + 1);

        // Add the parsed email to the inbox.
        inbox.addEmail(
            sender,
            subject,
            date);
    }

public:
    /*
    Constructor.

    Connects the command processor to the Inbox object.
    */
    explicit CommandProcessor(Inbox& inboxReference)
        : inbox(inboxReference)
    {
    }

    /*
    Opens and processes the test file.

    Returns true if the file was successfully processed.
    */
    bool processFile(const std::string& filename)
    {
        // Open the requested input file.
        std::ifstream inputFile(filename);

        // Check whether the file opened successfully.
        if (!inputFile.is_open())
        {
            return false;
        }

        // Stores one command from the file.
        std::string line;

        // Read commands until the end of the file.
        while (std::getline(inputFile, line))
        {
            // Remove a Windows carriage return if present.
            removeCarriageReturn(line);

            // Ignore blank lines.
            if (line.empty())
            {
                continue;
            }

            /*
            EMAIL contains additional information after the
            command name.
            */
            if (line.rfind("EMAIL ", 0) == 0)
            {
                processEmail(line);
            }

            /*
            NEXT displays the root but does not remove it.
            */
            else if (line == "NEXT")
            {
                inbox.showNext();
            }

            /*
            READ removes the root.
            */
            else if (line == "READ")
            {
                inbox.readNext();
            }

            /*
            COUNT displays the heap size.
            */
            else if (line == "COUNT")
            {
                inbox.showCount();
            }
        }

        // File processing completed successfully.
        return true;
    }
};


/*
Main Program
Creates the required objects and starts processing the test file.
*/
int main(int argc, char* argv[])
{
    /*
    The program expects exactly one argument after the
    executable name: the test-file name.
    */
    if (argc != 2)
    {
        // Display proper program usage.
        std::cerr
            << "Usage: "
            << argv[0]
            << " <test_file>\n";

        // End the program with an error.
        return 1;
    }

    // Create the CEO's inbox.
    Inbox inbox;

    // Create the object that processes commands.
    CommandProcessor processor(inbox);

    // Attempt to process the requested test file.
    if (!processor.processFile(argv[1]))
    {
        // Display an error if the file cannot be opened.
        std::cerr
            << "Error opening file: "
            << argv[1]
            << "\n";

        // End the program with an error.
        return 1;
    }

    // End the program successfully.
    return 0;
}