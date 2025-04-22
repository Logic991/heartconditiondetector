#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <sstream>

using namespace std;

// to_string function for older compilers (converts double to string)
string toString(double value) {
    stringstream ss;
    ss << value;
    return ss.str();
}

// ECGData class definition that will hold and process ECG data
class ECG {
private:
    vector<double> times;       // Vector to store times of ECG data points
    vector<double> amp;         // Vector to store ECG amplitudes
    vector<int> peakData;       // Vector to store number of detected peaks
    vector<double> peakTime;    // Vector to store times of detected peaks

public:
    // Method to load ECG data from a file
    void loadData( string& fileName);

    // Method to detect peaks in the ECG signal
    void detectPeaks();

    // Method to classify heart rate based on the time intervals between peaks
    void classifyHeartRate(queue<string>& tachycardiaResults, queue<string>& bradycardiaResults, queue<string>& normalResults);

    // Method to write results (tachycardia, bradycardia, or normal) to files
    void writeDetectionResults( string& filename, queue<string>& results);

    // Method to read previously saved results from files
    void readDetectionResults( string& filename, queue<string>& results);
};

// Load ECG data from a file into the times and amp vectors
void ECG::loadData( string& fileName) {
    ifstream file(fileName.c_str());  // Open the file
    if (!file.is_open()) {            // Check if the file opened successfully
        cerr << "ERROR: Failed to open file: " << fileName << endl;
        return;
    }

    double time, amplitude;
    while (file >> time >> amplitude) {    // Read time and amplitude values
        times.push_back(time);              // Store time in times vector
        amp.push_back(amplitude);           // Store amplitude in amp vector
    }
    file.close();    // Close the file
    cout << "Data loaded from " << fileName << ". Total records: " << times.size() << endl;
}

// Detect peaks in the ECG and store their data and time
void ECG::detectPeaks() {
     double threshold = 0.1;  // Minimum threshold for peak detection

    // Loop through the ECG data to find peaks
    for (size_t i = 1; i < amp.size() - 1; i++) {
        if (amp[i] > threshold) {    // If the amplitude is greater than the threshold, check the current value for a value greater than both the previous and next values.
            if (amp[i] > amp[i - 1] && amp[i] > amp[i + 1]) {
                peakData.push_back(i);       // Store peak data
                peakTime.push_back(times[i]); // Store peak time
            }
        }
    }
}

// Classify heart rate based on the time intervals between detected peaks
void ECG::classifyHeartRate(queue<string>& tachycardiaResults, queue<string>& bradycardiaResults, queue<string>& normalResults) {
    for (size_t i = 1; i < peakTime.size(); i++) {
        double interval = peakTime[i] - peakTime[i - 1]; // Calculate time difference between peaks

        // Bradycardia: heart rate < 60 bpm (interval > 0.85s)
        // If the interval between peaks is greater than 0.85 seconds, the heart rate is slow (Bradycardia)
        if (interval > 1) { 
            bradycardiaResults.push("Bradycardia detected between peaks at " + toString(peakTime[i - 1]) + " and " + toString(peakTime[i]));
        }
        // Tachycardia: heart rate > 100 bpm (interval < 0.3s)
        // If the interval between peaks is less than 0.3 seconds, the heart rate is very fast (Tachycardia)
        else if (interval < 0.6) { 
            tachycardiaResults.push("Tachycardia detected between peaks at " + toString(peakTime[i - 1]) + " and " + toString(peakTime[i]));
        }
        // Normal heart rate: heart rate between 60 and 100 bpm (interval between 0.3s and 0.85s)
        // If the interval between peaks is between 0.3 and 0.85 seconds, the heart rate is considered normal (Normal)
        else { 
            normalResults.push("Normal heart rate detected between peaks at " + toString(peakTime[i - 1]) + " and " + toString(peakTime[i]));
        }
    }
}

// Write results (tachycardia, bradycardia, or normal heart rate) to a specified file
void ECG::writeDetectionResults( string& filename, queue<string>& results) {
    ofstream outFile(filename.c_str());  // Open file for appending results

    if (!outFile) {    // Check if the file opened successfully
        cerr << "ERROR: Failed to open output file: " << filename << endl;
        return;
    }

    // Write each result from the queue to the file
    while (!results.empty()) {
        outFile << results.front() << endl;  // Write the front result from the queue
        results.pop();  // Remove the result from the queue
    }

    outFile.close();  // Close the output file
    cout << "Results written to " << filename << endl;
}

// Read saved results from a file and store them in a queue
void ECG::readDetectionResults( string& filename, queue<string>& results) {
    ifstream file(filename.c_str());  // Open the file for reading

    if (!file.is_open()) {    // Check if the file opened successfully
        cerr << "ERROR: Failed to open file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {  // Read each line from the file
        results.push(line);    // Store each line in the results queue
    }

    file.close();  // Close the file
}

// Combine results from two people and write them to a combined file
void combineFiles( string& resultFile,  string& person1File,  string& person2File) {
    queue<string> results;
    ECG person1, person2;

    // Read results from both person files and store them in the results queue
    person1.readDetectionResults(person1File, results);
    results.push("**************"); // Add separator between the two people's results
    person2.readDetectionResults(person2File, results);

    // Write combined results to the result file
    person1.writeDetectionResults(resultFile, results);
}

// Process ECG data for a person (detect peaks, classify heart rate, and save results)
void processPersonData(ECG& person,  string& personName) {
    person.detectPeaks();  // Detect peaks in the ECG data

    queue<string> tachycardiaResults, bradycardiaResults, normalResults;
    person.classifyHeartRate(tachycardiaResults, bradycardiaResults, normalResults);  // Classify heart rate

    // Write detection results to separate files for each condition (Normal, Tachycardia, and Bradycardia)
    person.writeDetectionResults(personName + "-Normal.txt", normalResults);
    person.writeDetectionResults(personName + "-Tachycardia.txt", tachycardiaResults);
    person.writeDetectionResults(personName + "-Bradycardia.txt", bradycardiaResults);
}

// Main function to load data, process it, and combine results
int main() {
    ECG person1, person2;

    // Load ECG data from files for two people
    //will have to change according to location of files
    person1.loadData("C:\\Users\\metin\\Downloads\\a\\person1.txt");
    person2.loadData("C:\\Users\\metin\\Downloads\\a\\person2.txt");

    // Process the ECG data for both persons and save individual results
    processPersonData(person1, "Person-1");
    processPersonData(person2, "Person-2");

    // Combine results for each condition (Normal, Tachycardia, and Bradycardia) for both persons
    combineFiles("Normal-Person-1-2.txt", "Person-1-Normal.txt", "Person-2-Normal.txt");
    combineFiles("Tachycardia-Person-1-2.txt", "Person-1-Tachycardia.txt", "Person-2-Tachycardia.txt");
    combineFiles("Bradycardia-Person-1-2.txt", "Person-1-Bradycardia.txt", "Person-2-Bradycardia.txt");

    cout << "Processing completed!" << endl;

    return 0;
}

