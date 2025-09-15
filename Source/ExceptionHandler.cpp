/**
 * @file ExceptionHandler.cpp
 * @brief Handles exceptions and allows logging of errors, with their own severity.
 *
 * HU_Exception() will help allow better logging of errors and error handling
 *
 * To be used in a try-catch:
 * try {
 * } catch {
 *  HU_Exception("error occured", error_file_name, line, severity, handler);
 * }
 *
 * Have our own safe open/read file
 *
 * Author: Jarren (100%)
*/

#include "ExceptionHandler.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <csignal>
#include <cstdlib> 

// Create the error_log folder if it doesn't exist
void createErrorLogFolder() {
    std::filesystem::create_directory("error_log");
}

/**
 * @brief Constructs an `HU_Exception` object with detailed error information.
 *
 * @param msg A descriptive error message providing details about the exception.
 * @param file The name of the file where the exception occurred.
 * @param line The line number in the file where the exception occurred.
 * @param severity The severity level of the error, defined as `ErrorSeverity`.
 * @param customHandler An optional custom error handler of type `ErrorHandler`.
 */
 /**
  * @brief Constructs an `HU_Exception` object with detailed error information.
  */
HU_Exception::HU_Exception(const std::string& msg,
    const std::string& file,
    int line,
    ErrorSeverity severity,
    ErrorHandler customHandler)
    : message(msg), file(file), line(line), severity(severity), handler(customHandler) {

    // If a custom handler is provided, call it, otherwise log the error using the default handler
    if (handler) {
        try {
            handler(*this); // Attempt to invoke the handler
        }
        catch (const std::exception& e) {
            std::cerr << "Error in custom handler: " << e.what() << "\n";
            logError(); // Log error if the handler fails
        }
    }
    else {
        logError(); // Default logging if no handler
    }
}

/**
 * @brief Gets the file name
 */
const std::string& HU_Exception::getFileName() const {
    return file;
}

/**
 * @brief Getter for the line number
 */
int HU_Exception::getLineName() const {
    return line;
}

/**
 * @brief Getter for the severity
 */
ErrorSeverity HU_Exception::getSeverity() const {
    return severity;
}


/**
 * @brief Gets the what() message sent
 */
const char* HU_Exception::what() const noexcept {
    return message.c_str();
}


/**
 * @brief Logs the error in a error_log.
 */
void HU_Exception::logError() const {
    createErrorLogFolder();  // Ensure the error_log folder exists

    std::ifstream logFile("error_log/error_log.txt");  // Read existing log entries
    std::string logContents;

    if (logFile.is_open()) {
        std::string line_1;
        while (std::getline(logFile, line_1)) {
            logContents += line + "\n"; // Read all lines into a string
        }
        logFile.close();
    }

    // Now write everything back including the 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    //  entry at the top
    std::ofstream outFile("error_log/error_log.txt", std::ios_base::trunc); // Open in truncation mode
    if (outFile.is_open()) {
        // Get the current time and format it
        std::time_t now = std::time(nullptr);
        char timeBuffer[20];
        struct tm timeInfo;

        if (localtime_s(&timeInfo, &now) == 0) {
            std::strftime(timeBuffer, sizeof(timeBuffer), "%m-%d-%y %H:%M:%S", &timeInfo);
        }
        else {
            fprintf(stderr, "Error getting local time\n");
        }

        // Write the new log entry at the top
        outFile << timeBuffer << "\n"
            << "Error: " << message << "\n"
            << "Location: " << file << " at line " << line << "\n";

        // Log the severity level
        switch (severity) {
        case ErrorSeverity::LOW:
            outFile << "Severity: LOW\n";
            break;
        case ErrorSeverity::MIDDLE:
            outFile << "Severity: ERROR\n";
            break;
        case ErrorSeverity::CRITICAL:
            outFile << "Severity: CRITICAL\n";
            break;
        }

        outFile << "-----------------------------\n";
        outFile << logContents; // Append existing log content

        outFile.close();
    }
    else {
        std::cerr << "Unable to open error log file.\n";
    }
}



/*
 * @brief Writes to std::cerr on the error, file name and line the error originates from
 *
 * @param ex (The exception it's supposed to handle)
 */
void HU_ConsoleLogHandler(const HU_Exception& ex) {
    std::cerr << "HU_ConsoleLogHandler\n";

    // Check severity
    switch (ex.getSeverity()) {
    case ErrorSeverity::LOW:
        std::cerr << "[LOW] ";
        break;
    case ErrorSeverity::MIDDLE:
        std::cerr << "[MIDDLE] ";
        break;
    case ErrorSeverity::CRITICAL:
        std::cerr << "[CRITICAL] ";
        break;
    }

    std::cerr << "Error: " << ex.what() << "\n"
        << "File: " << ex.getFileName()
        << ", Line: " << ex.getLineName() << "\n";

    if (ex.getSeverity() == ErrorSeverity::CRITICAL) {
        std::cerr.flush();  // Ensure the logs are printed to the terminal
        std::system("pause");  // Wait for user input (if running in a terminal)
        ex.logError();
        exit(EXIT_FAILURE);
    }
}

//Helper functions

/*
 * @brief Opens file for reading
 *
 * @param filename (The file name you want to open for reading)
 * 
 * @return std::ifstream
 * @throws ExceptionType std::ios_base::failure
 */
std::ifstream HU_ReadFile(const std::string& filename) {
    std::ifstream file;
    try {
        file.open(filename);
    }
    catch (const std::ios_base::failure& e) {
        // Log the error and throw a custom exception
        std::cerr << "Error: " << e.what() << "\n";
        throw HU_Exception(
            "File I/O error: " + std::string(e.what()),
            __FILE__,
            __LINE__,
            ErrorSeverity::CRITICAL,
            HU_ConsoleLogHandler
        );
    }
    return file; // Return the successfully opened file
}

/*
 * @brief Opens file for writing
 *
 * @param filename (The file name you want to open for writing)
 * @param clear (Put 1 if you want the file to be truncated/cleared every time it is opened. Else put 0)
 * 
 * @return std::ofstream
 * @throws ExceptionType std::ios_base::failure
 */
std::ofstream HU_OpenFile(const std::string& filename, bool clear) {
    try {
        // Check the mode and open the file accordingly
        if (clear) {
            std::ofstream file(filename, std::ios_base::trunc); // Open in truncate mode
            // Optionally, check if the file was successfully opened
            if (!file.is_open()) {
                throw std::ios_base::failure("Failed to open file: " + filename);
            }
            return file; // Return the successfully opened file
        }
        else {
            std::ofstream file(filename, std::ios_base::app); // Open in append mode
            // Optionally, check if the file was successfully opened
            if (!file.is_open()) {
                throw std::ios_base::failure("Failed to open file: " + filename);
            }
            return file; // Return the successfully opened file
        }        

    }
    catch (const std::ios_base::failure& e) {
        // Log the error and throw a custom exception
        std::cerr << "Error: " << e.what() << "\n";
        throw HU_Exception(
            "File I/O error: " + std::string(e.what()),
            __FILE__,
            __LINE__,
            ErrorSeverity::CRITICAL,
            HU_ConsoleLogHandler
        );
    }
}

