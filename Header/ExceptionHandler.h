/**
 * @file ExceptionHandler.h
 * @brief Declares the HU_Exception class for the cpp file.
 *
 * Author: Jarren (100%)
*/

#ifndef HU_EXCEPTION_H
#define HU_EXCEPTION_H

#include <exception>
#include <string>
#include <functional>

enum class ErrorSeverity {
    LOW,
    MIDDLE,
    CRITICAL
};

class HU_Exception : public std::exception {
public:
    // Define a type for the custom error handler
    using ErrorHandler = std::function<void(const HU_Exception&)>;

    // Constructor accepting message, file, line, and an optional custom handler
    HU_Exception(const std::string& msg, const std::string& file, int line, ErrorSeverity severity, ErrorHandler customHandler = nullptr);

    // Override the what() function to return the error message
    virtual const char* what() const noexcept override;

    // Accessor methods for file and line
    const std::string& getFileName() const;
    int getLineName() const;
    ErrorSeverity getSeverity() const;

private:
    std::string message;
    std::string file;
    int line;
    ErrorSeverity severity;
    ErrorHandler handler;

    // Default error logging function
public: 
    void logError() const;
};

// Declaring custom log handler
void HU_ConsoleLogHandler(const HU_Exception& ex);
std::ifstream HU_ReadFile(const std::string& filename);
std::ofstream HU_OpenFile(const std::string& filename, bool clear);

#endif // HU_EXCEPTION_H
