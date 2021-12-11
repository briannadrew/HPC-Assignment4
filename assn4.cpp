/* Name: assn4.cpp
Author: Brianna Drew
ID: #0622446
Date Created: 2021-12-06
Last Modified: 2021-12-10
Text for copy from: http://code.google.com/p/jquery-speedtest/downloads/detail?name=100MB.txt (modified to be approx. 1.5x larger)
Texts for read/write from: https://www.gutenberg.org/ */

// Included libraries
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <chrono>
#include <string>
#include <sstream>
using namespace std;

// Global variables
void readFunc();
void writeFunc();
string fileText;
int fileTag;
float readSum, writeSum, reads_sum, writes_sum, readThrough, writeThrough;

// Timer class to track duration of a specified amount of code
class Timer
{
public:
    Timer()
    {
        // Start timer
        startTimepoint = chrono::high_resolution_clock::now();
    }
    void Stop(string funcName, int tag, uintmax_t bytesize)
    {
        // Stop timer and calculate time elapsed
        auto endTimepoint = chrono::high_resolution_clock::now();
        auto start = chrono::time_point_cast<chrono::microseconds>(startTimepoint).time_since_epoch().count();
        auto end = chrono::time_point_cast<chrono::microseconds>(endTimepoint).time_since_epoch().count();
        auto duration = end - start;
        cout << funcName << " took " << duration << " microseconds.\n";
        switch (tag)
        {
        // File copy (calculate and output throughput)
        case 0:
            cout << funcName << " throughput = " << bytesize / (duration * 0.000001) << " bytes per second.\n";
            break;
        // File read (calculate and output throughput)
        case 1:
            readSum += duration;
            readThrough += (bytesize / (duration * 0.000001));
            cout << funcName << " throughput = " << bytesize / (duration * 0.000001) << " bytes per second.\n";
            break;
        // File write (calculate and output throughput)
        case 2:
            writeSum += duration;
            writeThrough += (bytesize / (duration * 0.000001));
            cout << funcName << " throughput = " << bytesize / (duration * 0.000001) << " bytes per second.\n";
            break;
        // Overall timer (calculate and output throughput)
        default:
            cout << "Average file read throughput = " << readThrough / 50 << " bytes per second.\n";
            cout << "Average file write throughput = " << writeThrough / 50 << " bytes per second.\n";
            break;
        }
    }
private:
    chrono::time_point< chrono::high_resolution_clock> startTimepoint;
};

// Print formatted information about the filesystem space (capacity and space available in bytes)
void print_space_info() {
    cout << left;
    cout << "| " << setw(15) << "Capacity" << " ";
    cout << "| " << setw(15) << "Free" << " ";
    cout << "| " << setw(15) << "Available" << " ";
    cout << "| " << setw(15) << "Dir" << " ";
    cout << "\n";
    error_code ec;
    // Retrieve space information about the /texts directory
    const filesystem::space_info txt = filesystem::space("/texts", ec);
    cout 
        << "| " << setw(15) << txt.capacity << " "
        << "| " << setw(15) << txt.free << " "
        << "| " << setw(15) << txt.available << " "
        << "| " << "/texts" << "\n";
}

int main() {
    // Start timer (for whole program)
    Timer totalTime;

    // Paths to source and destination files for copy
    const filesystem::path src = "texts/sample.txt";
    const filesystem::path dst = "texts/sample_cpy.txt";

    {
        // Start timer (for file copy)
        Timer copyTimer;
        // Copy contents of sample.txt file to sample_cpy.txt file
        try {
        filesystem::copy_file(src, dst, filesystem::copy_options::overwrite_existing);
        } catch(filesystem::filesystem_error& e) {
            // Print error if file copy failed
            cout << "ERROR: " << e.what() << "\n";
        }
        // Output size of copied file and stop timer
        cout << filesystem::file_size(dst) << " bytes copied from " << src << " to " << dst << ".\n";
        copyTimer.Stop("File copy ", 0, filesystem::file_size(dst));
    }

    cout << "\n";
    fileTag = 1;
    // Read and write to files 50 times
    for (int i = 1; i <= 50; i++) {
        readFunc();
        writeFunc();
        cout << "\n";
    }

    // Print information about the directory's space usage
    print_space_info();

    // Print final benchmarking results
    totalTime.Stop("\nEntire program", 3, 0);
    cout << "Average read latency: " << readSum / 50 << " microseconds.\n";
    cout << "Average write latency: " << writeSum / 50 << " microseconds.\n";
    cout << "Total size of /texts/read/: " << reads_sum << " bytes.\n";
    cout << "Total size of /texts/write/: " << writes_sum << " bytes.\n";
    cout << "Average file size in /texts/read/: " << reads_sum / 50 << " bytes.\n";
    cout << "Average file size in /texts/write/: " << writes_sum / 50 << " bytes.\n";
    cout << "Total size of /texts/: " << reads_sum + writes_sum << " bytes.\n";

    // Exit program
    return 0;
}

void readFunc() {
    // Start timer (for file read)
    Timer readTimer;
    string readName = "File read #" + to_string(fileTag);
    ifstream readFile("texts/read/example" + to_string(fileTag) + ".txt");
    if(readFile.is_open()) {
        // Get file contents from buffer as string
        stringstream buffer;
        buffer << readFile.rdbuf();
        fileText = buffer.str();
        // Get size of file
        uintmax_t file_size = filesystem::file_size("texts/read/example" + to_string(fileTag) + ".txt");
        reads_sum += file_size;
        cout << "File #" << fileTag << " size = " << file_size << " bytes.\n";
        // Close file and stop timer
        readFile.close();
        readTimer.Stop(readName, 1, file_size);
    }
    else {
        // Print error if unable to open file
        cout << "ERROR: Unable to read file.\n";
    }
}

void writeFunc() {
    // Start timer (for file write)
    Timer writeTimer;
    string writeName = "File write #" + to_string(fileTag);
    ofstream writeFile("texts/write/example" + to_string(fileTag) + "_new.txt");
    if(writeFile.is_open()) {
        // Write contents from previous file read to file
        writeFile << fileText;
        // Get size of file
        uintmax_t file_size = filesystem::file_size("texts/write/example" + to_string(fileTag) + "_new.txt");
        writes_sum += file_size;
        cout << "File #" << fileTag << " size = " << file_size << " bytes.\n";
        // Close file and stop timer
        writeFile.close();
        writeTimer.Stop(writeName, 2, file_size);
    }
    else {
        // Print error if unable to open file
        cout << "ERROR: Unable to write to file.\n";
    }
    fileTag++;
}