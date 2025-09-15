/**
 * @file messageSystem.h
 * @brief message handling system.
 *
 * This file contains the implementation of a message broker system that allows
 * different components (observers) to subscribe to messages sent by other components.
 * The system uses the Observer design pattern to facilitate communication
 * between different parts of the application.
 * 
 * 
 * RuiJie 100%
 */

#pragma once
#include <map>
#include <string>
#include <vector>
#include <iostream>

namespace CoreEngine {

	enum MessageID
	{
		//add more when more stuff is completed
		Quit,
		Pause,
		CollisionDetected,
		RenderObject,
		UpdatePhysics,
		LoadGraphics,
	};
		
	class IMessage {
	public:
		std::string sender;
		std::string receiver;
		MessageID M_ID;

		/// @brief Constructor for IMessage.
		/// @param id The message ID.
		/// @param sender The name of the sender.
		IMessage(MessageID id, const std::string& sender) : M_ID(id), sender(sender),receiver("") {}
		virtual ~IMessage() {}

		
		//get the message
		MessageID GetMessageID() const { return M_ID; }
	};


	//observer class to handle subscriptions
	//each system can subscript to messages they want to see
	class Observer {
	public:

		virtual ~Observer() = default;

		virtual const char* getName() const = 0;


		/// attach a handler to a specific message ID,register the handler 
		void AttachHandler(MessageID messageId, void(*handler)(IMessage*)) {
			M_Handlers[messageId] = handler;
		}

		//get the handler for the specific message ID
		void(*GetHandler(MessageID messageId))(IMessage*) {
			return M_Handlers[messageId];
		}

		//classes will provide their own messages
		virtual void HandleMessage(CoreEngine::IMessage* message) = 0;

	private:
		//takes in 
		std::map<MessageID, void(*)(IMessage*)> M_Handlers;
	};

	// MessageBroker class to manage observers and send messages
	class MessageBroker {
	public:
		static MessageBroker& Instance() {
			static MessageBroker instance;
			return instance;
		}

		/// register the observer to message ID
		void Register(MessageID messageId, Observer* observer) {
			observers[messageId].push_back(observer);
		}

		void Notify(IMessage* message) {
			// Log the sender of the message
			//// std::cout<< "[MessageBroker] Message Sent by: " << message->sender << " -> MessageID: " << MessageIDToString(message->M_ID) << std::endl;

			MessageID messageId = message->GetMessageID();
			auto it = observers.find(messageId); // Find observers registered to this message ID

			if (it != observers.end()) {
				for (auto* observer : it->second) {
					// Log the receiver (observer)
					//// std::cout<< "[MessageBroker] Message Received by: " << observer->getName()
					//	<< " for MessageID: " << MessageIDToString(message->M_ID) << std::endl;

					// Call the observer's registered handler for this message ID
					auto handler = observer->GetHandler(messageId);
					if (handler) {
						handler(message); // Call the handler and pass the message
					}
				}
			}
		}


		/**
		* @brief Convert a MessageID to its string representation.
		*
		* @param id The message ID to convert.
		* @return A string representing the message ID.
		*/
		std::string MessageIDToString(CoreEngine::MessageID id) {
			switch (id) {
			case CoreEngine::MessageID::Quit: return "Quit";
			case CoreEngine::MessageID::Pause: return "Pause";
			case CoreEngine::MessageID::CollisionDetected: return "CollisionDetected";
			case CoreEngine::MessageID::RenderObject: return "RenderObject";
			case CoreEngine::MessageID::UpdatePhysics: return "UpdatePhysics";
			case CoreEngine::MessageID::LoadGraphics: return "LoadGraphics";
			default: return "Unknown MessageID";
			}
		}


	private:
		MessageBroker() = default;
		std::map<MessageID, std::vector<Observer*>> observers; 
	};


	
}
