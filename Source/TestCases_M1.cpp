#pragma

/*

Ruijie 50
jarren 50
*/

#include "GlobalVariables.h"
#include "CommonIncludes.h"
//#include "AudioEngine.h"
#include "AssetsManager.h"
#include "ExceptionHandler.h"
#include "TestCases_M1.h"

//TEST CASES FOR M1 

//FAULTS
void simulateSegmentationFault() {
    int* ptr = nullptr; // Null pointer
    *ptr = 42; // Dereference null pointer to cause SIGSEGV
}

// Function to simulate illegal instruction
void simulateAbort() {
    std::cout << "Simulating abort...\n";
    std::abort(); // Causes the program to terminate
}

void throwCustomException() {
    // Throwing Custom Exceptions
    try {
        // Attempt to open a file in a non-existent directory to force an exception
        std::ofstream outFile = HU_OpenFile("non_existent_directory/output.txt", true);
    }
    catch (const HU_Exception& e) {
        // Catch the custom exception and handle it
        std::cerr << "Caught HU_Exception: " << e.what() << std::endl;
        std::cerr << "In file: " << e.getFileName() << " at line: " << e.getLineName() << std::endl;

        // You can also trigger custom logging, or take further actions based on severity
        if (e.getSeverity() == ErrorSeverity::CRITICAL) {
            std::cerr << "Critical error occurred, exiting..." << std::endl;
            exit(EXIT_FAILURE);  // Exit the program on critical errors
        }
    }
}


void music() {
    // Play a sound
    //CAudioEngine audioEngine;
    //audioEngine.Init();
    //float volume = 5.0f;  // Set volume in decibels
    //assets will load using the audiolibrary.
    //AudioLibrary.LoadAssets("./Assets/Audio/WhooshCartoon CTE02_89.1.wav", "Whoosh");
    //AudioLibrary.LoadAssets("./Assets/Audio/BAG-SAND_GEN-HDF-02864.wav", "Bag");
    //audioEngine.LoadSound(AudioLibrary.GetFileName("Whoosh"), false, true, false);
    //audioEngine.LoadSound(AudioLibrary.GetFileName("Bag"), false, true, false);

    // Add key to play here:
    // UNCOMMENT EACH ONE TO PLAY SEPARATELY, ITS LOUD
   // audioEngine.PlaySound(AudioLibrary.GetFileName("Whoosh"), 0, volume);
}

/*
*   Uncomment any line to test the error/music 
*/
void testcases() {


    /*
    *   Test for debugging errors
    *   Uncomment the three test cases to check
    */

    // ThrowingCustomException
    //throwCustomException();
    
    // Segfaults
    //simulateAbort();
    //simulateSegmentationFault();

    // Playing Music
    music();

}
