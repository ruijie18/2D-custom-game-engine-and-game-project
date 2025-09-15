/**
 * @file SignalHandler.cpp
 * @brief Checks for signala sent by the system if there were any faults (eg. Segfauult).
 *
 * Prints out the backtrace for tracking of errors, from HU_Backtrace, and can allow logging of errors.
 *
 * Author: Jarren (100%)
*/

#include "SignalHandler.h"
#include "ExceptionHandler.h"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
typedef DWORD pid_t; // On Windows, use DWORD for pid_t
#define getpid() GetCurrentProcessId()
#else
#include <unistd.h> // On Linux, use unistd for getpid
#endif

#include "backward.hpp"

/*
 * @brief Logs backtrace here
 */
#include <iostream>
#include <fstream>
#include <filesystem> // For creating directories

void HU_logBackTrace() {
    using namespace backward;
    TraceResolver thisIsAWorkaround;
    StackTrace stackTrace;
    stackTrace.load_here(32); // Load up to 32 frames

    Printer printer;
    printer.print(stackTrace, std::cerr); // Print to std::cerr

    // Ask the user if they want to log the crash report
    // std::cout<< "\nDo you want to log the crash report? (y/n): ";
    char response;
    std::cin >> response;

    // Handle user input
    if (response == 'y' || response == 'Y') {
        // Check if the "error_log" directory exists, and create it if not
        std::filesystem::path logDir = "error_log";
        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directory(logDir);
        }

        // Create and open the log file
        std::ofstream logFile(logDir / "crash_report.txt", std::ios::trunc);
        if (logFile.is_open()) {
            printer.print(stackTrace, logFile); // Save stack trace to a file
            logFile.close();
            // std::cout<< "Crash report saved to 'error_log/crash_report.txt'.\n";
        }
        else {
            std::cerr << "Failed to open log file.\n";
        }
    }
    else {
        // std::cout<< "Crash report not saved. Exiting...\n";
        std::exit(0); // Close the program or return if you want to return to a caller
    }
}


/*
 * @brief Based on the type of severity and what kind of fault it is, logs the backtrace and exits after
 *
 * @param sig: Signal number
 */
void HU_SignalHandler(int sig) {
    std::string errorMessage;
    ErrorSeverity severity = ErrorSeverity::CRITICAL; // Default severity for signals

    // std::cout<< sig;

    switch (sig) {
    case SIGABRT:
        errorMessage = "Assertion Failure or Aborted.";
        severity = ErrorSeverity::CRITICAL; // Maybe less severe
        break;
    case SIGSEGV:
        errorMessage = "Segmentation Fault.";
        severity = ErrorSeverity::CRITICAL;
        break;
    case SIGFPE:
        errorMessage = "Floating Point Exception.";
        severity = ErrorSeverity::CRITICAL;
        break;
    case SIGILL:
        errorMessage = "Illegal Instruction.";
        severity = ErrorSeverity::CRITICAL;
        break;
    case SIGTERM:
        errorMessage = "Termination Request.";
        severity = ErrorSeverity::LOW;
        break;
    default:
        errorMessage = "Unknown signal received.";
        break;
    }

    std::cerr << "Error: signal " << signal << std::endl;
    HU_logBackTrace();
    exit(sig);
}

void HU_SetupSignalHandlers() {
    std::signal(SIGFPE, HU_SignalHandler);  // Handle floating-point exceptions
    std::signal(SIGSEGV, HU_SignalHandler); // Handle segmentation faults
    std::signal(SIGABRT, HU_SignalHandler); // Handle abort signals
    
    std::signal(SIGILL, HU_SignalHandler);  // Handle illegal instructions
    std::signal(SIGTERM, HU_SignalHandler); // Handle termination requests
}
