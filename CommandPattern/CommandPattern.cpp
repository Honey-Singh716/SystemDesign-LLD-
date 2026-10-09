#include <iostream>
using namespace std;

class Command{
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual ~Command() {}
};


//Receiver class
class Light {
public:
    void on(){
        cout<<"Light is ON"<<endl;
    }


    void off(){
        cout<<"Light is OFF"<<endl;
    }
};

class Fan{
public:
    void on(){
        cout<<"Fan is ON"<<endl;
    }

    void off(){
        cout<<"Fan is OFF"<<endl;
    }
};  


class LightCommand : public Command {
private:
    Light* light;
public:
    LightCommand(Light* l) : light(l) {}

    void execute() override {
        light->on();
    }

    void undo() override {
        light->off();
    }
};

class FanCommand : public Command {
private:
    Fan* fan;
public:
    FanCommand(Fan* f) : fan(f) {}
    
    void execute() override {
        fan->on();
    }

    void undo() override {
        fan->off();
    }
};


class RemoteController {
private:
    static const int numButtons = 4;

    Command* buttons[numButtons];
    bool buttonStates[numButtons]; // true for ON, false for OFF

public:
    RemoteController() {

        for(int i = 0;i<numButtons;i++){
            buttons[i] = nullptr;
            buttonStates[i] = false;
        }
    }

    void setCommand(int idx, Command* command) {
        if(idx >= 0 && idx < numButtons) {
            if(buttons[idx] != nullptr){
                delete buttons[idx];
            }
            buttons[idx] = command;
        }
    }

    void pressButton(int idx) {

        if(idx >= 0 && idx < numButtons && buttons[idx] != nullptr) {
            
            if(buttonStates[idx]) {
                buttons[idx]->undo();
                buttonStates[idx] = false;
            }
            else {
                buttons[idx]->execute();
                buttonStates[idx] = true;  
            }
        }

        else {
            cout << "No command assigned at button " << idx << endl;
        }

    }

    ~RemoteController() {
        for (int i = 0; i < numButtons; i++) {
            if (buttons[i] != NULL)
                delete buttons[i];
        }
    }
};


int main(){
  
    Light* light = new Light();
    Fan* fan = new Fan();

    RemoteController* remote = new RemoteController();


    remote->setCommand(0, new LightCommand(light));
    remote->setCommand(1, new FanCommand(fan));

    cout << "Pressing button 0 (Light):" << endl;
    remote->pressButton(0); // Light ON
    remote->pressButton(0); // Light OFF

    cout << "Pressing button 1 (Fan):" << endl;
    remote->pressButton(1); // Fan ON
    remote->pressButton(1); // Fan OFF

    cout<<"Unassigned button press (button 2):"<<endl;
    remote->pressButton(2); // No command assigned

    delete light;
    delete fan;
    delete remote;

    return 0;
}