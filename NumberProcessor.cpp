#include <iostream>
#include <fstream>
#include <vector>

class NumberProcessor {
public:
    // Method to read numbers from a file and store them in a vector
    void readNumbersFromFile(const std::string& filename) {
        std::ifstream file(filename); // Open the file
        if (!file.is_open()) { // Check if the file opened successfully
            std::cerr << "Error opening file: " << filename << std::endl;
            return;
        }

        int number;
        while (file >> number) { // Read numbers one by one from the file
            numbers.push_back(number); // Store the number in the vector
        }

        file.close(); // Close the file after reading
    }

    // Method to compute the sum of the numbers
    int computeSum() const {
        int sum = 0;
        for (int number : numbers) {
            sum += number; // Add each number to sum
        }
        return sum;
    }

    // Method to compute the average of the numbers
    double computeAverage() const {
        if (numbers.empty()) {
            return 0.0; // Avoid division by zero if no numbers are available
        }
        return static_cast<double>(computeSum()) / numbers.size();
    }

    // Method to display the numbers
    void displayNumbers() const {
        std::cout << "Numbers from the file:" << std::endl;
        for (int number : numbers) {
            std::cout << number << " ";
        }
        std::cout << std::endl;
    }

private:
    std::vector<int> numbers; // Vector to store the numbers
};
