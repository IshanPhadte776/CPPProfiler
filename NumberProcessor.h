//Ensures the file is only included once.
//Idiot Proofs the code 
#pragma once

#include <string>
#include <vector>

class NumberProcessor {
public:
    // Method to read numbers from a file and store them in a vector
    void readNumbersFromFile(const std::string& filename);

    // Method to compute the sum of the numbers
    int computeSum() const;

    // Method to compute the average of the numbers
    double computeAverage() const;

    // Method to display the numbers
    void displayNumbers() const;

private:
    std::vector<int> numbers; // Vector to store the numbers
};
