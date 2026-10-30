#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

using namespace std;

/////////////////////////////////////////////
// Product & Factory
/////////////////////////////////////////////

class Product {
private:
    int sku;
    string name;
    double price;

public:
    Product(int id, string nm, double pr) {
        sku   = id;
        name  = nm;
        price = pr;
    }

    //Getters & Setters
    int getSku() {
        return this->sku;
    }

    string getName() {
        return this->name;
    }
    double getPrice() {
        return this->price;
    }
};

class ProductFactory {
public:
    static Product* createProduct(int sku) {
        // In reality product comes from DB
        string name;
        double price;

        if (sku == 101) {
            name  = "Apple";
            price = 20;
        }
        else if (sku == 102) {
            name  = "Banana";
            price = 10;
        }
        else if (sku == 103) {
            name  = "Chocolate";
            price = 50;
        }
        else if (sku == 201) {
            name  = "T-Shirt";
            price = 500;
        }
        else if (sku == 202) {
            name  = "Jeans";
            price = 1000;
        }
        else {
            name  = "Item" + to_string(sku);
            price = 100;
        }
        return new Product(sku, name, price);
    }
};

/////////////////////////////////////////////
// InventoryStore (Abstract) & DbInventoryStore
/////////////////////////////////////////////

class InventoryStore {
public:
    virtual ~InventoryStore() {}
    virtual void addProduct(Product* prod, int qty) = 0;
    virtual void removeProduct(int sku, int qty) = 0;
    virtual int checkStock(int sku) = 0;
    virtual vector<Product*> listAvailableProducts() = 0;
};

class DbInventoryStore : public InventoryStore {
private:
    map<int,int>* stock;             // SKU -> quantity
    map<int,Product*>* products;     // SKU -> Product*
public:
    DbInventoryStore() {
        stock = new map<int,int>();
        products = new map<int,Product*>();
    }
    ~DbInventoryStore() {
        for (auto it : *products) {
            delete it.second;
        }
        delete products;
        delete stock;
    }

    void addProduct(Product* prod, int qty) override {
        int sku = prod->getSku();
        if (products->count(sku) == 0) {
            // Does not exist
            (*products)[sku] = prod;
        } else {
            delete prod;
        }
        (*stock)[sku] += qty;
    }

    void removeProduct(int sku, int qty) override {
        if (stock->count(sku) == 0) 
            return;

        int currentQuantity = (*stock)[sku];
        int remainingQuantity = currentQuantity - qty;
        if (remainingQuantity > 0) {
            (*stock)[sku] = remainingQuantity;
        } else {
            stock->erase(sku);
        }
    }

    int checkStock(int sku) override {
        if (stock->count(sku) == 0) 
            return 0;

        return (*stock)[sku];
    }

    vector<Product*> listAvailableProducts() override {
        vector<Product*> available;
        for (auto it : *stock) {
            int sku = it.first;
            int qty = it.second;
            if (qty > 0 && products->count(sku)) {
                available.push_back((*products)[sku]);
            }
        }
        return available;
    }
};

/////////////////////////////////////////////
// InventoryManager 
/////////////////////////////////////////////

class InventoryManager {
    InventoryStore* store;

public:
    InventoryManager(InventoryStore* store) {
        this->store = store;
    }

    void addStock(int sku, int qty) {
        Product* prod = ProductFactory::createProduct(sku);
        store->addProduct(prod, qty);
        cout << "[InventoryManager] Added SKU " << sku << " Qty " << qty << endl;
    }

    void removeStock(int sku, int qty) {
        store->removeProduct(sku, qty); 
    }

    int checkStock(int sku) {
        return store->checkStock(sku);
    }

    vector<Product*> getAvailableProducts() {
        return store->listAvailableProducts();
    }
};

/////////////////////////////////////////////
// Replenishment Strategy (Strategy Pattern)
/////////////////////////////////////////////

class ReplenishStrategy {
public:
    virtual void replenish(InventoryManager* manager, map<int,int> itemsToReplenish) = 0;
    virtual ~ReplenishStrategy() {}
};

class ThresholdReplenishStrategy : public ReplenishStrategy {
private:
    int threshold;
public:
    ThresholdReplenishStrategy(int threshold) {
        this->threshold = threshold;
    }
    void replenish(InventoryManager* manager, map<int,int> itemsToReplenish) override {
        cout << "[ThresholdReplenish] Checking threshold... \n";
        for (auto it : itemsToReplenish) {
            int sku = it.first;
            int qtyToAdd = it.second;
            int current  = manager->checkStock(sku);
            if (current < threshold) {
                manager->addStock(sku, qtyToAdd);
                cout << "  -> SKU " << sku << " was " << current 
                     << ", replenished by " << qtyToAdd << endl;
            }
        }
    }
};

class WeeklyReplenishStrategy : public ReplenishStrategy {
public:
    WeeklyReplenishStrategy() {}
    void replenish(InventoryManager* manager, map<int,int> itemsToReplenish) override {
        cout << "[WeeklyReplenish] Weekly replenishment triggered for inventory.\n";
    }
};

/////////////////////////////////////////////
// DarkStore (formerly Warehouse)
/////////////////////////////////////////////

class DarkStore {
private:
    string name;
    double x, y;                       // location coordinates
    InventoryManager* inventoryManager;
    ReplenishStrategy* replenishStrategy;

public:
    DarkStore(string n, double x_coord, double y_coord) {
        name = n;
        x = x_coord;
        y = y_coord;
        
        // We could have made another factory called InventoryStoreFactory to get
        // DbInventoryStore by enum and hence make it loosely coupled.
        inventoryManager = new InventoryManager(new DbInventoryStore);

    }
    ~DarkStore() {
        delete inventoryManager;
        if (replenishStrategy) delete replenishStrategy;
    }

    double distanceTo(double ux, double uy) {
        return sqrt((x - ux)*(x - ux) + (y - uy)*(y - uy));
    }

    void runReplenishment(map<int,int> itemsToReplenish) {
        if (replenishStrategy) {
            replenishStrategy->replenish(inventoryManager, itemsToReplenish);
        }
    }

    // Delegation Methods
    vector<Product*> getAllProducts() {
        return inventoryManager->getAvailableProducts();
    }

    int checkStock(int sku) {
        return inventoryManager->checkStock(sku);
    }

    void removeStock(int sku, int qty) {
        inventoryManager->removeStock(sku, qty); 
    }

    void addStock(int sku, int qty) {
        Product* prod = ProductFactory::createProduct(sku);
        inventoryManager->addStock(sku, qty);
    }

    // Getters & Setters
    void setReplenishStrategy(ReplenishStrategy* strategy) {
        this->replenishStrategy = strategy;
    }

    string getName() {
        return this->name;
    }

    double getXCoordinate() {
        return this->x;
    }

    double getYCoordinate() {
        return this->y;
    }

    InventoryManager* getInventoryManager() {
        return this->inventoryManager;
    }
};

class DarkStoreManager {
    vector<DarkStore*>* darkStores;
    static DarkStoreManager* instance;

    DarkStoreManager() {
        darkStores = new vector<DarkStore*>();
    }

public:
    static DarkStoreManager* getInstance() {
        if (!instance) {
            instance = new DarkStoreManager();
        }
        return instance;
    }

    void registerDarkStore(DarkStore* db){
        darkStores->push_back(db);
    }

    vector<DarkStore*> getNearByDarkStores(double x, double y, double radius) {
        vector<pair<double,DarkStore*>> distList;
        for (auto ds : *darkStores) {
            double d = ds->distanceTo(x, y);
            if (d <= radius) {
                distList.push_back(make_pair(d, ds));
            }
        }
        sort(distList.begin(), distList.end(),
             [](auto &a, auto &b){ return a.first < b.first; });

        vector<DarkStore*> result;
        for (auto &p : distList) {
            result.push_back(p.second);
        }
        return result;
    }

    ~DarkStoreManager() {
        for (auto ds : *darkStores) {
            delete ds;
        }
        delete darkStores;
    }

};

DarkStoreManager* DarkStoreManager::instance = nullptr;


class Cart {
public:
    vector<pair<Product*, int>> items; // Product and quantity
    
    void addItem(int sku, int qty) {
        Product* prod = ProductFactory::createProduct(sku);
        items.push_back(make_pair(prod, qty));
        cout << "[Cart] Added SKU " << sku << " (" << prod->getName() 
             << ") x" << qty << endl;
    }

    double getTotalAmount(){
        double total = 0.0;

        for(auto & item : items){
            total += item.first->getPrice() * item.second;
        }
        return total;
    }

    vector<pair<Product*,int>> getItems() {
        return items;
    }

    ~Cart() {
        for (auto &it : items){
            delete it.first;
        }
    }
};


class User {
public:
    string name;
    double x,y;
    Cart* cart;

    User(string n,double x_coord,double y_coord){
        name = n;
        x = x_coord;
        y = y_coord;
        cart = new Cart();
    }
    ~User() {
        delete cart;
    }

    Cart* getCart() {
        return cart;
    }

};



class DeliveryPartner {
private:
    string name;
    double x,y;

public:
    
    DeliveryPartner(string n,double x_coord,double y_coord){
        name = n;
        x = x_coord;
        y = y_coord;
    }

    double distanceTo(double ux, double uy) {
        return sqrt((x - ux)*(x - ux) + (y - uy)*(y - uy));
    }

    bool isAvailable() {
        // For simulation, we can randomly decide availability
        return rand() % 2 == 0; // 50% chance of being available
    }

    string getName() {
        return this->name;
    }

    double getXCoordinate() {
        return this->x;
    }

    double getYCoordinate() {
        return this->y;
    }

    ~DeliveryPartner() {}

};

class DeliveryPartnerManager {
    vector<DeliveryPartner*>* partners;
    static DeliveryPartnerManager* instance;

    DeliveryPartnerManager() {
        partners = new vector<DeliveryPartner*>();
    }

    public:

    static DeliveryPartnerManager* getInstance() {
        if (!instance) {
            instance = new DeliveryPartnerManager();
        }
        return instance;
    }

    void registerDeliveryPartner(DeliveryPartner* dp){
        partners->push_back(dp);
    }

    vector<DeliveryPartner*> getNearDeliveryPartners(double x, double y, double radius){
        vector<pair<int,DeliveryPartner*>> distlist;

        for(auto dp : *partners){
            double d = dp->distanceTo(x,y);
            if(d <= radius){
                distlist.push_back(make_pair(d,dp));
            }
        }
        
        sort(distlist.begin(), distlist.end(),
             [](auto &a, auto &b){ return a.first < b.first; });
        
        vector<DeliveryPartner*> result;

        for(auto &p : distlist){
            result.push_back(p.second);
        }

        return result;
    }
};


DeliveryPartnerManager* DeliveryPartnerManager::instance = nullptr;



class Order {
public:
    static int orderCounter;
    
    int orderId;
    User* user;
    vector<pair<Product*, int>> items; // Product and quantity
    vector<DeliveryPartner*> deliveryPartners;
    double totalAmount;

    Order(User* u){
        orderId = ++orderCounter;
        user = u;
        totalAmount = 0.0;
    }
};

int Order::orderCounter = 1;


class OrderManager {
private:
    vector<Order*>* orders;
    static OrderManager* instance;
    
    OrderManager(){
        orders = new vector<Order*>();
    }

public:
    static OrderManager* getInstance() {
        if (!instance) {
            instance = new OrderManager();
        }
        return instance;
    }

    void placeOrder(User* user, Cart* cart){
        cout<< "\n[OrderManager] Placing order for user: " << user->name << endl;

        vector<pair<Product*,int>> requestedItems = cart->getItems();

        if(requestedItems.empty()){
            cout << "[OrderManager] Cart is empty. Cannot place order.\n";
            return;
        }

        double radius = 10.0; // Define a radius to search for nearby dark stores

        vector<DarkStore*> nearbyStores = DarkStoreManager::getInstance()->getNearByDarkStores(user->x, user->y, radius);

        if(nearbyStores.empty()){
            cout << "[OrderManager] No nearby dark stores found.\n";
            return;
        }

        // Check stock availability in nearby stores
        if (requestedItems.empty()) {
            cout << "[OrderManager] No items in the cart.\n";
            return;
        }
        DarkStore* firstStore = nearbyStores[0]; // For simplicity, we take the first store
        
        bool allInFirst = true;
        for (pair<Product*,int>& item : requestedItems) {

            int sku = item.first->getSku();
            int qty = item.second;

            if (firstStore->checkStock(sku) < qty) {
                allInFirst = false;
                break;
            }
        }
        
        Order* order = new Order(user);

        if (allInFirst) {

            cout << "  All items at: " << firstStore->getName() << "\n";

            // Remove the products from store
            for (pair<Product*,int>& item : requestedItems) {
                int sku = item.first->getSku();
                int qty = item.second;
                firstStore->removeStock(sku, qty);
                order->items.push_back({ item.first, qty });
            }

            order->totalAmount = cart->getTotalAmount();
            orders->push_back(order);
            
            for(auto dp : DeliveryPartnerManager::getInstance()->getNearDeliveryPartners(user->x, user->y, radius)){
                if(dp->isAvailable()){
                    order->deliveryPartners.push_back(dp);
                    break; // Assign the first available partner
                }
            }

            if(order->deliveryPartners.empty()){
                cout << "  No available delivery partners found.\n";
            } else {
                cout << "  Assigned Delivery Partner: " << order->deliveryPartners[0]->getName() << "\n";
            }


            cout << "  Assigned Delivery Partner: Partner1\n";
        } 
        
        //Multiple stores and multiple delivery partners can be handled here.
        else{

            cout << "  Splitting order across stores...\n";
             map<int,int> allItems; //SKU --> Qty

             for (pair<Product*,int>& item : requestedItems) {
                allItems[item.first->getSku()] = item.second;
            }
            
            //first check the first partner;

            for(auto dp : DeliveryPartnerManager::getInstance()->getNearDeliveryPartners(user->x, user->y, radius)){
                if(dp->isAvailable()){
                    order->deliveryPartners.push_back(dp);
                    break; // Assign the first available partner
                }
            }

            if(order->deliveryPartners.empty()){
                cout << "  No available delivery partners found.\n";
            } else {
                cout << "  Assigned Delivery Partner: " << order->deliveryPartners[0]->getName() << "\n";
            }

            for (DarkStore* store : nearbyStores) {

                // If allItems becomes empty, we break early (all SKUs have been assigned)
                if (allItems.empty()) break;

                cout << "   Checking: " << store->getName() << "\n";
    
                bool assigned = false;
                vector<int> toErase;
                
                for (auto& [sku, qtyNeeded] : allItems) {

                    int availableQty = store->checkStock(sku);
                    if (availableQty <= 0) continue;
    
                    //take whichever is smaller: available or qtyNeeded.
                    int takenQty = min(availableQty, qtyNeeded);
                    store->removeStock(sku, takenQty);

                    cout << "     " << store->getName() << " supplies SKU " << sku 
                         << " x" << takenQty << "\n";

                    order->items.push_back({ ProductFactory::createProduct(sku), takenQty });
    
                    // Adjust the Quantity
                    if (qtyNeeded > takenQty) {
                        allItems[sku] = qtyNeeded - takenQty;
                    } else {
                        toErase.push_back(sku);
                    }
                    assigned = true;
                }

                // Remove items that have been fully assigned
                for (int sku : toErase) {
                    allItems.erase(sku);
                }

                // If at least one SKU was taken from this store, we assign 
                // a new DeliveryPartner.
                if (assigned) {
                    for(auto dp : DeliveryPartnerManager::getInstance()->getNearDeliveryPartners(user->x, user->y, radius)){
                        if(dp->isAvailable()){
                            order->deliveryPartners.push_back(dp);
                            cout << "  Assigned Delivery Partner: " << dp->getName() << "\n";
                            break; // Assign the first available partner
                        }
                    }
                }
            }

             //  if remaining still has entries, we print which SKUs/quantities could not be fulfilled.
            if (!allItems.empty()) {
                cout << "  Could not fulfill:\n";
                for (auto& [sku, qty] : allItems) {
                    cout << "    SKU " << sku << " x" << qty << "\n";
                }
            }


            double sum = 0;
            for (auto& item : order->items) {
                sum += item.first->getPrice() * item.second;
            }
            order->totalAmount = sum;
        }

         // Printing Order Summary
        cout << "\n[OrderManager] Order #" << order->orderId << " Summary:\n";
        cout << "  User: " << user->name << "\n  Items:\n";
        for (auto& item : order->items) {
            cout << "    SKU " << item.first->getSku()
                 << " (" << item.first->getName() << ") x" << item.second
                 << " @ ₹" << item.first->getPrice() << "\n";
        }
        cout << "  Total: ₹" << order->totalAmount << "\n  Partners:\n";
        for (auto* dp : order->deliveryPartners) {
            cout << "    " << dp->getName() << "\n";
        }
        cout << endl;
    
        orders->push_back(order);
        // Cleanups
        for (auto* dp : order->deliveryPartners) delete dp;
        for (auto& item : order->items) delete item.first;
    }

    vector<Order*> getAllOrders() {
        return *orders;
    }

    ~OrderManager() {
        for (auto ord : *orders) {
            delete ord;
        }
        delete orders;
    }
    
};


OrderManager* OrderManager::instance = nullptr;


class ZeptoHelper {
public:
    static void showAllItems(User* user) {
        
        double radius = 10.0; // Define a radius to search for nearby dark stores

        vector<DarkStore*> nearbyStores = DarkStoreManager::getInstance()->getNearByDarkStores(user->x, user->y, radius);

        if (nearbyStores.empty()) {
            cout << "[ZeptoHelper] No nearby dark stores found.\n";
            return;
        }

        // Collect each SKU → price (so we only display each product once)
        map<int, double> skuToPrice;
        map<int, string> skuToName;

        cout << "[ZeptoHelper] Available items in nearby dark stores:\n";
        for (DarkStore* store : nearbyStores) {
            cout << "  Store: " << store->getName() << "\n";

            vector<Product*> products = store->getAllProducts();

            for (Product* prod : products) {

                int sku = prod->getSku();

                if (skuToPrice.count(sku) == 0) {
                    skuToPrice[sku] = prod->getPrice();
                    skuToName[sku]  = prod->getName();
                }
            }
        }
        
        for (auto& entry : skuToPrice) {
            int sku = entry.first;
            double price = entry.second;
            cout << "  SKU " << sku << " - " << skuToName[sku] << " @ ₹" << price << "\n";
        }
    }

    static void initilize(){
        auto dsManager = DarkStoreManager::getInstance();

        // Initialize the dark store manager
        DarkStore* ds1 = new DarkStore("DarkStore1", 0, 0);
        ds1->addStock(101, 50); // Apple

        ds1->addStock(102, 100); // Banana
        ds1->addStock(103, 30); // Chocolate

        dsManager->registerDarkStore(ds1);

        DarkStore* ds2 = new DarkStore("DarkStore2", 5, 5);
        ds2->addStock(201, 20); // T-Shirt
        ds2->addStock(202, 15); // Jeans

        dsManager->registerDarkStore(ds2);
        

        DarkStore* ds3 = new DarkStore("DarkStore3", 8, 8);
        ds3->addStock(101, 30); // Apple
        ds3->addStock(202, 10); // Jeans

        dsManager->registerDarkStore(ds3);



        ds1->setReplenishStrategy(new ThresholdReplenishStrategy(3));
        ds2->setReplenishStrategy(new WeeklyReplenishStrategy());
        ds3->setReplenishStrategy(new ThresholdReplenishStrategy(2));

    }
};


int main() {
    // 1. Initialize dark stores and their inventory
    ZeptoHelper::initilize();

    // 2. Register delivery partners
    DeliveryPartnerManager* dpManager =
        DeliveryPartnerManager::getInstance();

    dpManager->registerDeliveryPartner(
        new DeliveryPartner("Partner1", 1, 1)
    );

    dpManager->registerDeliveryPartner(
        new DeliveryPartner("Partner2", 6, 6)
    );

    dpManager->registerDeliveryPartner(
        new DeliveryPartner("Partner3", 9, 9)
    );

    // 3. Create a user
    User* user = new User("Honey", 2, 2);

    cout << "\n========== AVAILABLE PRODUCTS ==========\n";

    // 4. Display products available in nearby stores
    ZeptoHelper::showAllItems(user);

    // 5. Add products to cart
    cout << "\n========== ADD TO CART ==========\n";

    user->getCart()->addItem(101, 2);  // 2 Apples
    user->getCart()->addItem(102, 3);  // 3 Bananas
    user->getCart()->addItem(201, 1);  // 1 T-Shirt

    cout << "\nCart Total: Rs. "
         << user->getCart()->getTotalAmount() << "\n";

    // 6. Run replenishment for DarkStore1
    cout << "\n========== INVENTORY REPLENISHMENT ==========\n";

    DarkStore* store1 =
        DarkStoreManager::getInstance()->getNearByDarkStores(
            user->x, user->y, 10.0
        )[0];

    map<int, int> itemsToReplenish = {
        {101, 20},
        {102, 30},
        {103, 10}
    };

    store1->runReplenishment(itemsToReplenish);

    // 7. Place order
    cout << "\n========== PLACE ORDER ==========\n";

    OrderManager* orderManager = OrderManager::getInstance();

    orderManager->placeOrder(user, user->getCart());

    // 8. Display remaining inventory
    cout << "\n========== REMAINING INVENTORY ==========\n";

    cout << "Apple stock in DarkStore1: "
         << store1->checkStock(101) << "\n";

    cout << "Banana stock in DarkStore1: "
         << store1->checkStock(102) << "\n";

    // 9. Clean up user
    delete user;

    cout << "\n========== SIMULATION COMPLETE ==========\n";

    return 0;
}