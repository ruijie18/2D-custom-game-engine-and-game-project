///////////////////////////////////////////////////////////////////////////////////////
///
///	\file Core.h
///	Defines the CoreEngine that encapsulate all the various systems
///	Authors: Ruijie 100%
///
///
///////////////////////////////////////////////////////////////////////////////////////
#ifndef  HUSTLERS_ENGINE_H
#define HUSTLERS_ENGINE_H

#include "GlobalVariables.h"
#include <GLFW/glfw3.h>
#include "MessageSystem.h"

extern bool isFullscreen;

class HustlersEngine: public CoreEngine::Observer{
private:
	//FPS
	double targetFPS = 60.0;
	double minFrameTime = 1.0/targetFPS;
	
public:
    HustlersEngine() {}
    HustlersEngine(GLFWwindow* window);
    ~HustlersEngine();


	void setTargetFPS(double fps) {
		targetFPS=fps;
		minFrameTime = 1.0/targetFPS;
	}

	void run(GLFWwindow* window);	

    //used to get the system process with imgui
	void CheckSystemProcess(double deltaTime, std::string& output);


    const char* getName() const override {
        return "HustlersEngine";
    }


	void HandleMessage(CoreEngine::IMessage* message) override {
		switch (message->GetMessageID()) {
		case CoreEngine::Quit:
			//HandleQuit(message);
			break;
		default:
			break;
		}
	}

    static void HandleQuit(CoreEngine::IMessage* message) {
		(void)message;
		 //std::cout<< "Sender: " << message->sender << std::endl;
		 //std::cout<< "Game is quitting..." << std::endl;
    }
};

#endif // ! HUSTLERS_ENGINE_H