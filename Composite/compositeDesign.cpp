#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

using namespace std;


class FileSystemItem {
public:
    virtual ~FileSystemItem() {}
    virtual void ls(int indent = 0) const = 0;
    virtual void openAll(int indent = 0) const = 0;
    virtual int getSize() = 0;
    virtual string getName() const = 0;
    virtual FileSystemItem* cd(const string& name) = 0;
    virtual bool isFolder() const = 0;

};


//Leaf

class File : public FileSystemItem {
    string name;
    int size;
public:

    File(const string& name,int size){
        
        this->name = name;
        this->size = size;
    }
    
    void ls(int indent = 0) const override {
        cout << string(indent, ' ') << name << "\n";
    }

    void openAll(int indent = 0) const override {
        cout << string(indent, ' ') << name << " (File opened)\n";
    }

    int getSize() override {
        return size;
    }

    FileSystemItem* cd(const string&) override {
       return nullptr; // Files cannot be navigated into
    }

    string getName() const override {
        return name;
    }

    bool isFolder() const override {
        return false;
    }

};


class Folder : public FileSystemItem {
    string name;
    vector<FileSystemItem*> items;

public:
    
    Folder(const string& name){
        this->name = name;
    }
    ~Folder(){
        for(auto item : items) delete item;
    }

     void add(FileSystemItem* item){
        items.push_back(item);
     }

    
    void ls(int indent = 0) const override {
        
        for(auto item : items){
            if(item->isFolder()){
                cout << string(indent, ' ') << "+" << item->getName() << "\n";
            }
            else{
                cout<< string(indent, ' ') << "-" << item->getName() << "\n";
            }
        }
    }


    void openAll(int indent = 0) const override {

        cout<< string(indent, ' ') << "+" << name << "\n";

        for(auto item : items){
            item->openAll(indent + 4);
        }
    }


    int getSize() override {
        int totalSize = 0;

        for(auto item : items){
            totalSize += item->getSize();
        }

        return totalSize;
    }

    FileSystemItem* cd(const string& name) override {
        for(auto item : items){
            if(item->isFolder() && item->getName() == name){
                return item;
            }
        }

        return nullptr; // Folder not found
    }
   
    string getName() const override {
        return name;
    }

    bool isFolder() const override {
        return true;
    }
  
};


int main(){

    Folder* root = new Folder("root");

    Folder* folder1 = new Folder("folder1");

    folder1->add(new File("file1.txt", 100));
    folder1->add(new File("file2.txt", 200));

    Folder* folder2 = new Folder("folder2");

    folder2->add(new File("file3.txt", 300));


    root->add(new File("file4.txt", 400));

    root->add(folder1);
    root->add(folder2);

    cout << "Listing all items in root folder:\n";

    root->ls();


    root->openAll();
    

    FileSystemItem* folder = root->cd("folder1");
    
    if(folder != nullptr){
        folder->ls();
    }

    else{
        cout<< "Folder not found\n";
    }

    cout << "Total size of root folder: " << root->getSize() << " bytes\n";
    cout<< "Total size of folder1: " << folder1->getSize() << " bytes\n";
    cout<< "Total size of folder2: " << folder2->getSize() << " bytes\n";

    delete root;
}