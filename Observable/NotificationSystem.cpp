#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

//Notification build and add some decorators to it

class INotification {
    public:
        virtual string getContent() const = 0;

        virtual ~INotification() {}
};

//concrete notification class

class SimpleNotification : public INotification {
private:
    string text;
public:
    
    SimpleNotification(const string & content) : text(content) {}
    
    string getContent() const override {
        return text;
    }

};

//Abstract of Notification Decorator

class INotificationDecorator : public INotification {
protected:
    INotification* notification;
public:

   INotificationDecorator(INotification* n) : notification(n) {}

   virtual ~INotificationDecorator() {
    delete notification;
   }
};


//decorator builders
class TimeStampDecorator : public INotificationDecorator {
    public:
        TimeStampDecorator(INotification* n) : INotificationDecorator(n) {}

        string getContent() const override {
            return "[2025-04-13 14:22:00]" + notification->getContent();
        }
};

class SignatureDecorator : public INotificationDecorator {
private:
    string signature;
public:
       SignatureDecorator(INotification* n, const string& sig) : INotificationDecorator(n), signature(sig) {}

       string getContent() const override {
        return notification->getContent() + "\n-- " + signature + "\n\n";
       }
};



//Observer pattern implementation


//Abstract Observer interface
class IObserver {
    public:
        virtual void update() = 0;
        virtual ~IObserver() {}
};

//Abstract Observable interface
class IObservable {
    public:
        virtual void addObserver(IObserver* observer) = 0;
        virtual void removeObserver(IObserver* observer) = 0;
        virtual void notifyObservers() = 0;
};


//Concrete Observable class
class NotificationObservable : public IObservable {
private:
    vector<IObserver*> observers;
    INotification* currentNotification;
public:
    NotificationObservable() : currentNotification(nullptr) {}
    

    void addObserver(IObserver* observer) override {
        observers.push_back(observer);
    }

    void removeObserver(IObserver* observer) override {
        observers.erase(remove(observers.begin(), observers.end(), observer), observers.end());
    }

    
    void notifyObservers() override {        
        for (unsigned int i = 0; i < observers.size(); i++) {
            observers[i]->update();
        }
    }

    void setNotification(INotification* notification) {
        if (currentNotification != nullptr) {
            delete currentNotification;
        }
        currentNotification = notification;
        notifyObservers();
    }

    INotification* getNotification() {
        return currentNotification;
    }

    string getNotificationContent() {
        return currentNotification->getContent();
    }

    ~NotificationObservable() {
        if (currentNotification != NULL) {
            delete currentNotification;
        }
    }

};


//concrete Observer class
class Logger : public IObserver {
private:
    NotificationObservable* notificationObservable;
public:
    Logger(NotificationObservable* obs) : notificationObservable(obs) {}

    void update(){
        cout <<"[Logger] New Notification: " << notificationObservable->getNotificationContent() << endl;
    }
};


//Strategy pattern implementation for notification sending

//abstract strategy interface
class INotificationStrategy {
public:
    virtual void sendNotification(string content) = 0;
};

class EmailStrategy : public INotificationStrategy {
private:
    string emailAddress;
public:
    EmailStrategy(const string& email) : emailAddress(email) {}

    void sendNotification(string content) override {
        cout << "[Email] Sending notification to " << emailAddress << ": " << content << endl;
    }
};

class SMSStrategy : public INotificationStrategy {
private:
    string mobileNumber;
public:

    SMSStrategy(string mobileNumber) {
        this->mobileNumber = mobileNumber;
    }

    void sendNotification(string content) override {
        // Simulate the process of sending an SMS notification, 
        // representing the dispatch of messages to users via SMS.​
        cout << "Sending SMS Notification to: " << mobileNumber << "\n" << content;
    }
};

class PopUpStrategy : public INotificationStrategy {
public:
    void sendNotification(string content) override {
        // Simulate the process of sending popup notification.
        cout << "Sending Popup Notification: \n" << content;
    }
};




class NotificationEngine : public IObserver {
private:
    NotificationObservable* notificationObservable;
    vector<INotificationStrategy*> notificationStrategies;

public:
    NotificationEngine(NotificationObservable* observable) {
        this->notificationObservable = observable;
    }

    void addNotificationStrategy(INotificationStrategy* ns) {
        this->notificationStrategies.push_back(ns);
    }

    // Can have RemoveNotificationStrategy as well.

    void update() {
        string notificationContent = notificationObservable->getNotificationContent();
        for(const auto notificationStrategy : notificationStrategies) {
            notificationStrategy->sendNotification(notificationContent);
        }
    }
};


// The NotificationService manages notifications. It keeps track of notifications. 
// Any client code will interact with this service.

// Singleton class
class NotificationService {
private:
    NotificationObservable* observable;
    static NotificationService* instance;
    vector<INotification*> notifications;

    NotificationService() {
        // private constructor
        observable = new NotificationObservable();
    }

public:
    static NotificationService* getInstance() {
        if(instance == nullptr) {
            instance = new NotificationService();
        }
        return instance;
    }

    // Expose the observable so observers can attach.
    NotificationObservable* getObservable() {
        return observable;
    }

    // Creates a new Notification and notifies observers.
    void sendNotification(INotification* notification) {
        notifications.push_back(notification); // history
        observable->setNotification(notification);
    }

    ~NotificationService() {
        delete observable;
    }
};

NotificationService* NotificationService::instance = nullptr;



int main() {
    // Create NotificationService.
    NotificationService* notificationService = NotificationService::getInstance();

    // Get Observable
    NotificationObservable* notificationObservable = notificationService->getObservable();
   
    // Create Logger Observer
    Logger* logger = new Logger(notificationObservable);

    // Create NotificationEngine observers.
    NotificationEngine* notificationEngine = new NotificationEngine(notificationObservable);

    notificationEngine->addNotificationStrategy(new EmailStrategy("random.person@gmail.com"));
    notificationEngine->addNotificationStrategy(new SMSStrategy("+91 9876543210"));
    notificationEngine->addNotificationStrategy(new PopUpStrategy());

    // Attach these observers.
    notificationObservable->addObserver(logger);
    notificationObservable->addObserver(notificationEngine);

    // Create a notification with decorators.
    INotification* notification = new SimpleNotification("Your order has been shipped!");
    notification = new TimeStampDecorator(notification);
    notification = new SignatureDecorator(notification, "Customer Care");
    
    notificationService->sendNotification(notification);

    delete logger;
    delete notificationEngine;
    return 0;
}