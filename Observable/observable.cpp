#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

class ISubscriber {
    public:
       virtual void update() = 0;
       virtual ~ISubscriber() {}
};

class Ichannel{
public:
    virtual void subscribe(ISubscriber* subscriber) =0;
    virtual void unsubscribe(ISubscriber* subscriber) =0;
    virtual void notifySubscribers() =0;
    virtual ~Ichannel() {}
};

class Channel : public Ichannel {
    private:
    vector<ISubscriber*> subscribers;
    string name;
    string latestVideo;
    public:

    Channel(string name){
        this->name = name;
    }

    //Add a subscriber to the list
    void subscribe(ISubscriber* subscriber) override {
        if (find(subscribers.begin(), subscribers.end(), subscriber) == subscribers.end()) {
            subscribers.push_back(subscriber);
        }
    }

    // remove a subscriber from the list
    void unsubscribe(ISubscriber* subscriber) override {
        auto it = find(subscribers.begin(), subscribers.end(), subscriber);
        if (it != subscribers.end()) {
            subscribers.erase(it);
        }
    }

    // notify all subscribers
    void notifySubscribers() override {
        for (auto subscriber : subscribers) {
            subscriber->update();
        }
    }

    // Upload a new video and notify all subscribers
    void uploadVideo(const string& title) {
        latestVideo = title;
        cout << "\n[" << name << " uploaded \"" << title << "\"]\n";
        notifySubscribers();
    }

    string getVideoData() {
        return "New video uploaded: " + latestVideo + "\n";
    }



};

//Concrete Observer class
class Subscriber : public ISubscriber {
    private:
    string name;
    Channel* channel;

    public:
     
    Subscriber(string name, Channel* channel) {
        this->name = name;
        this->channel = channel;
        channel->subscribe(this);
    }
    
    void update() override {
        cout << "Hey " << name << "," << this->channel->getVideoData();
    }

};


int main(){

    Channel* channel = new Channel("Code Buddy");

    Subscriber* subscriber1 = new Subscriber("Alice", channel);
    Subscriber* subscriber2 = new Subscriber("Bob", channel);

    channel->uploadVideo("Observer Design Pattern in C++");

    channel->unsubscribe(subscriber2);
    
    channel->uploadVideo("Factory Design Pattern in C++");

    return 0;


}