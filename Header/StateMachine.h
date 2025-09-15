#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <unordered_map>
#include <memory>
#include <iostream>

template <typename T>
class StateMachine {
protected:
    std::unordered_map<T, std::unique_ptr<State<T>>> states;
    State<T>* currentState;

public:
    StateMachine() : currentState(nullptr) {}

    void AddState(std::unique_ptr<State<T>> state) {
        T stateEnum = state->GetState();
        states[stateEnum] = std::move(state);
    }

    void TransitionTo(T newState) {
        if (states.find(newState) == states.end()) {
            std::cerr << "State " << static_cast<int>(newState) << " not found!" << std::endl;
            return;
        }

        if (currentState != nullptr) {
            currentState->Exit();
        }

        currentState = states[newState].get();

        if (currentState != nullptr) {
            currentState->Enter();
        }
    }

    void UpdateState() {
        if (currentState != nullptr) {
            currentState->Update();
        }
    }

    T GetCurrentState() const {
        return (currentState != nullptr) ? currentState->GetState() : T{};
    }
};

#endif // STATEMACHINE_H
