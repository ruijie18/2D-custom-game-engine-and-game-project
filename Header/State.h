#ifndef STATE_H
#define STATE_H

#include <iostream>

// Abstract State class with basic functionalities
template <typename T>
class State {
public:
    virtual ~State() = default;

    virtual void Enter() = 0;
    virtual void Exit() = 0;
    virtual void Update() = 0;
    virtual T GetState() const = 0; // Return the state type
};

#endif // STATE_H
