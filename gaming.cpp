#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <limits>
#include <cctype>

using namespace std;

// ============================================================
// REQUIRED PRODUCT STRUCTURE
// ============================================================
struct Product {
    int id;
    string name;
    string category;
    string brand;
    double price;
    int stockQuantity;
};

struct CartLine {
    Product product;
    int quantity;
};

struct Order {
    int orderId;
    vector<CartLine> items;
    double total;
};

struct SalesRecord {
    string productName;
    int unitsSold;
};

// ============================================================
// LAB 1: DYNAMIC ARRAY + EXACT pop_back_dynamic OPERATION
// Inventory is stored in a heap-allocated dynamic array.
// ============================================================
template <typename T>
void pop_back_dynamic(T*& arr, int& size) {
    if (size <= 0) return;

    int newSize = size - 1;
    T* smaller = new T[newSize]; // allocate a new, smaller heap block

    for (int i = 0; i < newSize; ++i) {
        smaller[i] = arr[i];     // copy every element except the last one
    }

    delete[] arr;                // release the old heap block
    arr = smaller;               // point to the new block
    size = newSize;              // update the logical and physical size
}

template <typename T>
class DynamicArray {
private:
    T* data;
    int count;
    int capacity;

    void grow() {
        int newCapacity = (capacity == 0) ? 1 : capacity * 2;
        T* bigger = new T[newCapacity];
        for (int i = 0; i < count; ++i) bigger[i] = data[i];
        delete[] data;
        data = bigger;
        capacity = newCapacity;
    }

public:
    DynamicArray() : data(nullptr), count(0), capacity(0) {}
    ~DynamicArray() { delete[] data; }

    DynamicArray(const DynamicArray&) = delete;
    DynamicArray& operator=(const DynamicArray&) = delete;

    int size() const { return count; }
    bool empty() const { return count == 0; }

    T& operator[](int index) { return data[index]; }
    const T& operator[](int index) const { return data[index]; }

    void push_back(const T& value) {
        if (count == capacity) grow();
        data[count++] = value;
    }

    // Remove an item at any index, then shrink the heap array by one slot.
    void removeAt(int index) {
        if (index < 0 || index >= count) return;
        for (int i = index; i < count - 1; ++i) data[i] = data[i + 1];
        pop_back_dynamic(data, count);
        capacity = count; // after shrinking, capacity exactly matches count
    }

    int findIndexById(int id) const {
        for (int i = 0; i < count; ++i)
            if (data[i].id == id) return i;
        return -1;
    }
};

// ============================================================
// LAB 2: SINGLY LINKED LIST + getLastElement()
// Stores completed order IDs in insertion order.
// ============================================================
class OrderHistoryList {
private:
    struct Node {
        int data;
        Node* next;
        explicit Node(int value) : data(value), next(nullptr) {}
    };
    Node* head;

public:
    OrderHistoryList() : head(nullptr) {}
    ~OrderHistoryList() {
        while (head) {
            Node* old = head;
            head = head->next;
            delete old;
        }
    }

    void append(int orderId) {
        Node* node = new Node(orderId);
        if (!head) { head = node; return; }
        Node* current = head;
        while (current->next) current = current->next;
        current->next = node;
    }

    // Lab 2 task: traverse from head to the node immediately before NULL.
    int getLastElement() const {
        if (head == nullptr) return -1;
        Node* current = head;
        while (current->next != nullptr) current = current->next;
        return current->data;
    }

    void display() const {
        if (!head) { cout << "No completed orders yet.\n"; return; }
        Node* current = head;
        while (current) {
            cout << "ORD-" << current->data;
            if (current->next) cout << " -> ";
            current = current->next;
        }
        cout << " -> NULL\n";
    }
};

// ============================================================
// LAB 3A: DOUBLY LINKED LIST
// Shopping cart supports forward and backward traversal.
// ============================================================
class DoublyLinkedCart {
private:
    struct Node {
        CartLine data;
        Node* next;
        Node* prev;
        explicit Node(const CartLine& line) : data(line), next(nullptr), prev(nullptr) {}
    };
    Node* head;
    Node* tail;

public:
    DoublyLinkedCart() : head(nullptr), tail(nullptr) {}
    ~DoublyLinkedCart() { clear(); }

    bool empty() const { return head == nullptr; }

    void add(const Product& product, int quantity) {
        Node* current = head;
        while (current) {
            if (current->data.product.id == product.id) {
                current->data.quantity += quantity;
                return;
            }
            current = current->next;
        }
        // Insert in ascending product-ID order. The new node may be placed
        // between two existing nodes, requiring four link updates.
        CartLine line{product, quantity};
        Node* node = new Node(line);
        current = head;
        while (current && current->data.product.id < product.id) current = current->next;
        if (!current) { // insert at tail
            node->prev = tail;
            if (tail) tail->next = node;
            else head = node;
            tail = node;
        } else { // insert before current (possibly at head or in the middle)
            node->next = current;
            node->prev = current->prev;
            if (current->prev) current->prev->next = node;
            else head = node;
            current->prev = node;
        }
    }

    bool reduceQuantity(int productId, int quantity) {
        Node* current = head;
        while (current && current->data.product.id != productId) current = current->next;
        if (!current) return false;
        current->data.quantity -= quantity;
        if (current->data.quantity <= 0) {
            if (current->prev) current->prev->next = current->next;
            else head = current->next;
            if (current->next) current->next->prev = current->prev;
            else tail = current->prev;
            delete current;
        }
        return true;
    }

    bool removeProduct(int productId) {
        Node* current = head;
        while (current && current->data.product.id != productId) current = current->next;
        if (!current) return false;
        if (current->prev) current->prev->next = current->next;
        else head = current->next;
        if (current->next) current->next->prev = current->prev;
        else tail = current->prev;
        delete current;
        return true;
    }

    vector<CartLine> toVector() const {
        vector<CartLine> result;
        for (Node* current = head; current; current = current->next)
            result.push_back(current->data);
        return result;
    }

    double subtotal() const {
        double total = 0;
        for (Node* current = head; current; current = current->next)
            total += current->data.product.price * current->data.quantity;
        return total;
    }

    void displayForward() const {
        if (!head) { cout << "Your cart is empty.\n"; return; }
        cout << "\n--- CART: FRONT TO BACK ---\n";
        for (Node* current = head; current; current = current->next)
            cout << current->data.product.name << " x" << current->data.quantity << "\n";
    }

    void displayBackward() const {
        if (!tail) { cout << "Your cart is empty.\n"; return; }
        cout << "--- CART: BACK TO FRONT ---\n";
        for (Node* current = tail; current; current = current->prev)
            cout << current->data.product.name << " x" << current->data.quantity << "\n";
    }

    void clear() {
        Node* current = head;
        while (current) {
            Node* nextNode = current->next;
            delete current;
            current = nextNode;
        }
        head = tail = nullptr;
    }
};

// ============================================================
// LAB 3B: CIRCULAR LINKED LIST
// Cycles through partner brands for a rotating featured-brand panel.
// ============================================================
class CircularBrandList {
private:
    struct Node {
        string brand;
        Node* next;
        explicit Node(const string& value) : brand(value), next(nullptr) {}
    };
    Node* tail;
    Node* current;

public:
    CircularBrandList() : tail(nullptr), current(nullptr) {}
    ~CircularBrandList() {
        if (!tail) return;
        Node* start = tail->next;
        tail->next = nullptr;
        while (start) {
            Node* nextNode = start->next;
            delete start;
            start = nextNode;
        }
    }

    void add(const string& brand) {
        Node* node = new Node(brand);
        if (!tail) {
            tail = node;
            tail->next = tail;
            current = tail;
        } else {
            node->next = tail->next;
            tail->next = node;
            tail = node;
        }
    }

    string nextBrand() {
        if (!current) return "No featured brands";
        string result = current->brand;
        current = current->next;
        return result;
    }
};

// ============================================================
// LAB 4: STACK + BALANCED PARENTHESES CHECKER
// Also used to undo the most recent cart-add action (product ID).
// ============================================================
bool isBalanced(const string& expression) {
    stack<char> brackets;
    for (char ch : expression) {
        if (ch == '(') brackets.push(ch);
        else if (ch == ')') {
            if (brackets.empty()) return false;
            brackets.pop();
        }
    }
    return brackets.empty();
}

// ============================================================
// LABS 5 & 6: BINARY SEARCH TREE + RECURSIVE BST SEARCH
// Product IDs are keys. Inorder traversal displays IDs in ascending order.
// ============================================================
class ProductBST {
private:
    struct Node {
        int id;
        Node* left;
        Node* right;
        explicit Node(int value) : id(value), left(nullptr), right(nullptr) {}
    };
    Node* root;

    Node* insertRecursive(Node* node, int id) {
        if (!node) return new Node(id);
        if (id < node->id) node->left = insertRecursive(node->left, id);
        else if (id > node->id) node->right = insertRecursive(node->right, id);
        return node;
    }

    bool searchRecursive(Node* node, int target) const {
        if (!node) return false;
        if (node->id == target) return true;
        if (target < node->id) return searchRecursive(node->left, target);
        return searchRecursive(node->right, target);
    }

    void inorder(Node* node) const {
        if (!node) return;
        inorder(node->left);
        cout << node->id << " ";
        inorder(node->right);
    }

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    ProductBST() : root(nullptr) {}
    ~ProductBST() { destroy(root); }

    void clear() { destroy(root); root = nullptr; }
    void insert(int id) { root = insertRecursive(root, id); }
    bool searchBST(int target) const { return searchRecursive(root, target); }
    void displaySortedIDs() const {
        cout << "BST product IDs (inorder): ";
        inorder(root);
        cout << "\n";
    }
};

// ============================================================
// LAB 7: BINARY SEARCH THAT RETURNS THE NUMBER OF COMPARISONS
// Input IDs must be sorted in ascending order before this function.
// ============================================================
int countBinarySteps(const vector<int>& arr, int target) {
    int low = 0;
    int high = static_cast<int>(arr.size()) - 1;
    int steps = 0;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        ++steps; // count the comparison against arr[mid]
        if (arr[mid] == target) return steps;
        if (target < arr[mid]) high = mid - 1;
        else low = mid + 1;
    }
    return steps; // comparisons performed when target is absent
}

// ============================================================
// LAB 8: INSERTION SORT IN DESCENDING ORDER
// Sales leaderboard ranks products by units sold (highest first).
// ============================================================
void rankLeaderboard(vector<SalesRecord>& scores) {
    int n = static_cast<int>(scores.size());
    for (int i = 1; i < n; ++i) {
        SalesRecord key = scores[i];
        int j = i - 1;
        while (j >= 0 && scores[j].unitsSold < key.unitsSold) {
            scores[j + 1] = scores[j];
            --j;
        }
        scores[j + 1] = key;
    }
}

// ============================================================
// SYSTEM CLASS
// ============================================================
class NexoraGaming {
private:
    DynamicArray<Product> inventory;
    ProductBST productTree;
    DoublyLinkedCart cart;
    CircularBrandList featuredBrands;
    OrderHistoryList orderHistory;
    queue<Order> checkoutQueue;      // FIFO order processing
    stack<pair<int, int>> cartUndoStack;         // product IDs added to cart
    vector<SalesRecord> sales;
    int nextOrderId;

    static string lower(string value) {
        for (char& ch : value) ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
        return value;
    }

    int nextProductId() const {
        int highest = 0;
        for (int i = 0; i < inventory.size(); ++i)
            highest = max(highest, inventory[i].id);
        return highest + 1;
    }

    void rebuildTree() {
        productTree.clear();
        for (int i = 0; i < inventory.size(); ++i) productTree.insert(inventory[i].id);
    }

    void addSales(const string& name, int quantity) {
        for (auto& record : sales) {
            if (record.productName == name) { record.unitsSold += quantity; return; }
        }
        sales.push_back({name, quantity});
    }

    void printProduct(const Product& p) const {
        cout << left << setw(5) << p.id << setw(34) << p.name
             << setw(15) << p.brand << setw(17) << p.category
             << right << setw(10) << fixed << setprecision(2) << p.price
             << setw(8) << p.stockQuantity << "\n";
    }

    void printInventoryHeader() const {
        cout << left << setw(5) << "ID" << setw(34) << "PRODUCT"
             << setw(15) << "BRAND" << setw(17) << "CATEGORY"
             << right << setw(10) << "PRICE" << setw(8) << "STOCK" << "\n";
        cout << string(89, '-') << "\n";
    }

    void seedInventory() {
        const Product samples[] = {
            {101,"G502 HERO Gaming Mouse","Mouse","Logitech G",2495,18},
            {102,"G Pro X Superlight Mouse","Mouse","Logitech G",6995,9},
            {103,"G Pro Mechanical Keyboard","Keyboard","Logitech G",5995,12},
            {104,"G733 Wireless Headset","Headset","Logitech G",7495,7},
            {105,"G305 Lightspeed Mouse","Mouse","Logitech G",1995,25},
            {106,"G435 Wireless Headset","Headset","Logitech G",3495,14},
            {107,"DeathAdder V3 Mouse","Mouse","Razer",3995,16},
            {108,"BlackWidow V4 Keyboard","Keyboard","Razer",8995,6},
            {109,"Kraken V3 Headset","Headset","Razer",4995,11},
            {110,"Viper Mini Mouse","Mouse","Razer",1495,20},
            {111,"Ornata V3 Keyboard","Keyboard","Razer",3495,13},
            {112,"Barracuda X Headset","Headset","Razer",5995,8},
            {113,"Cloud II Gaming Headset","Headset","HyperX",4295,17},
            {114,"Cloud III Wireless Headset","Headset","HyperX",7995,5},
            {115,"Alloy Origins Keyboard","Keyboard","HyperX",5495,10},
            {116,"Pulsefire Haste Mouse","Mouse","HyperX",2495,19},
            {117,"QuadCast Microphone","Microphone","HyperX",6995,4},
            {118,"Alloy Core RGB Keyboard","Keyboard","HyperX",2995,15},
            {119,"Rival 5 Gaming Mouse","Mouse","SteelSeries",3995,12},
            {120,"Arctis Nova 7 Headset","Headset","SteelSeries",10995,6},
            {121,"Apex 3 Keyboard","Keyboard","SteelSeries",3495,14},
            {122,"Qck Gaming Mousepad","Mousepad","SteelSeries",995,30}
        };
        for (const Product& p : samples) inventory.push_back(p);
        rebuildTree();
    }

    bool getInt(const string& prompt, int& value) const {
        cout << prompt;
        if (!(cin >> value)) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a whole number.\n";
            return false;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return true;
    }

    bool getPositivePrice(const string& prompt, double& value) const {
        cout << prompt;
        if (!(cin >> value) || value <= 0) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Price must be a valid number greater than zero.\n";
            return false;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return true;
    }

    string getLine(const string& prompt) const {
        string value;
        cout << prompt;
        getline(cin, value);
        return value;
    }

    void displayProducts() const {
        if (inventory.empty()) { cout << "Inventory is empty.\n"; return; }
        printInventoryHeader();
        for (int i = 0; i < inventory.size(); ++i) printProduct(inventory[i]);
    }

    void searchByName() const {
        string query = lower(getLine("Enter product name keyword: "));
        bool found = false;
        for (int i = 0; i < inventory.size(); ++i) {
            if (lower(inventory[i].name).find(query) != string::npos) {
                if (!found) printInventoryHeader();
                printProduct(inventory[i]); found = true;
            }
        }
        if (!found) cout << "No matching product found.\n";
    }

    void searchById() const {
        int id;
        if (!getInt("Enter product ID: ", id)) return;
        bool found = false;
        for (int i = 0; i < inventory.size(); ++i) {
            if (inventory[i].id == id) { printInventoryHeader(); printProduct(inventory[i]); found = true; break; }
        }
        cout << (found ? "BST check: product ID exists.\n" : "Product ID not found.\n");
        cout << "BST result: " << (productTree.searchBST(id) ? "Found" : "Not found") << "\n";
        vector<int> ids;
        for (int i = 0; i < inventory.size(); ++i) ids.push_back(inventory[i].id);
        sort(ids.begin(), ids.end());
        cout << "Binary-search comparisons: " << countBinarySteps(ids, id) << "\n";
    }

    void filterByBrand() const {
        string brand = lower(getLine("Brand (Logitech G, Razer, HyperX, SteelSeries): "));
        bool found = false;
        for (int i = 0; i < inventory.size(); ++i) {
            if (lower(inventory[i].brand) == brand) {
                if (!found) printInventoryHeader();
                printProduct(inventory[i]); found = true;
            }
        }
        if (!found) cout << "No products found for that brand.\n";
    }

    void filterByCategory() const {
        string category = lower(getLine("Category (Mouse, Keyboard, Headset, Microphone, Mousepad): "));
        bool found = false;
        for (int i = 0; i < inventory.size(); ++i) {
            if (lower(inventory[i].category) == category) {
                if (!found) printInventoryHeader();
                printProduct(inventory[i]); found = true;
            }
        }
        if (!found) cout << "No products found in that category.\n";
    }

    void sortByPrice() const {
        vector<Product> sorted;
        for (int i = 0; i < inventory.size(); ++i) sorted.push_back(inventory[i]);
        // Insertion Sort ascending by price for the customer price-sort feature.
        for (int i = 1; i < static_cast<int>(sorted.size()); ++i) {
            Product key = sorted[i];
            int j = i - 1;
            while (j >= 0 && sorted[j].price > key.price) {
                sorted[j + 1] = sorted[j]; --j;
            }
            sorted[j + 1] = key;
        }
        cout << "\nProducts sorted by price (lowest to highest):\n";
        printInventoryHeader();
        for (const Product& p : sorted) printProduct(p);
    }

    void addToCart() {
        int id, quantity;
        if (!getInt("Product ID to add: ", id) || !getInt("Quantity: ", quantity)) return;
        if (quantity <= 0) { cout << "Quantity must be greater than zero.\n"; return; }
        int index = inventory.findIndexById(id);
        if (index == -1) { cout << "Product not found.\n"; return; }
        if (quantity > inventory[index].stockQuantity) { cout << "Insufficient stock. Available: " << inventory[index].stockQuantity << "\n"; return; }
        cart.add(inventory[index], quantity);
        cartUndoStack.push({id, quantity});
        cout << "Added to cart successfully.\n";
    }

    void viewCart() const {
        cart.displayForward();
        cout << "Cart subtotal: PHP " << fixed << setprecision(2) << cart.subtotal() << "\n";
        cart.displayBackward();
    }

    void undoCartAdd() {
        if (cartUndoStack.empty()) { cout << "No cart action to undo.\n"; return; }
        auto action = cartUndoStack.top(); cartUndoStack.pop();
        if (cart.reduceQuantity(action.first, action.second)) cout << "Undid the most recent cart addition.\n";
        else cout << "That product is no longer in the cart.\n";
    }

    void checkout() {
        if (cart.empty()) { cout << "Cannot checkout: cart is empty.\n"; return; }
        vector<CartLine> lines = cart.toVector();
        // Validate all lines before creating/enqueuing the order.
        for (const CartLine& line : lines) {
            int index = inventory.findIndexById(line.product.id);
            if (index == -1 || line.quantity > inventory[index].stockQuantity) {
                cout << "Checkout rejected. Insufficient or unavailable stock for " << line.product.name << ".\n";
                return;
            }
        }
        Order order{nextOrderId++, lines, cart.subtotal()};
        checkoutQueue.push(order);
        cout << "Order placed in checkout queue. Processing FIFO...\n";
        processCheckoutQueue();
    }

    void processCheckoutQueue() {
        while (!checkoutQueue.empty()) {
            Order order = checkoutQueue.front();
            checkoutQueue.pop(); // FIFO: earliest submitted order is processed first
            cout << "\n==============================================\n";
            cout << "             NEXORA GAMING RECEIPT\n";
            cout << "             Order: ORD-" << order.orderId << "\n";
            cout << "==============================================\n";
            cout << left << setw(34) << "PRODUCT" << setw(8) << "QTY" << setw(12) << "PRICE" << "AMOUNT\n";
            for (const CartLine& line : order.items) {
                int index = inventory.findIndexById(line.product.id);
                // Stock was validated before enqueue; deduct only after successful processing.
                inventory[index].stockQuantity -= line.quantity;
                addSales(line.product.name, line.quantity);
                double amount = line.product.price * line.quantity;
                cout << left << setw(34) << line.product.name << setw(8) << line.quantity
                     << "PHP " << setw(8) << fixed << setprecision(2) << line.product.price
                     << "PHP " << fixed << setprecision(2) << amount << "\n";
            }
            cout << string(65, '-') << "\n";
            cout << "TOTAL: PHP " << fixed << setprecision(2) << order.total << "\n";
            cout << "Payment status: SUCCESSFUL\n";
            cout << "Thank you for shopping at NEXORA GAMING!\n";
            orderHistory.append(order.orderId);
        }
        cart.clear();
        while (!cartUndoStack.empty()) cartUndoStack.pop();
    }

    void addProduct() {
        Product p{};
        p.id = nextProductId();
        p.name = getLine("Product name: ");
        p.category = getLine("Category: ");
        p.brand = getLine("Brand: ");
        if (p.name.empty() || p.category.empty() || p.brand.empty()) { cout << "Fields cannot be empty.\n"; return; }
        if (!getPositivePrice("Price (PHP): ", p.price)) return;
        if (!getInt("Stock quantity: ", p.stockQuantity)) return;
        if (p.stockQuantity < 0) { cout << "Stock cannot be negative.\n"; return; }
        inventory.push_back(p);
        productTree.insert(p.id);
        cout << "Product added. Assigned ID: " << p.id << "\n";
    }

    void updateProduct() {
        int id;
        if (!getInt("Product ID to update: ", id)) return;
        int index = inventory.findIndexById(id);
        if (index == -1) { cout << "Product not found.\n"; return; }
        cout << "Leave a text field blank to keep its current value.\n";
        string value = getLine("New name [" + inventory[index].name + "]: ");
        if (!value.empty()) inventory[index].name = value;
        value = getLine("New category [" + inventory[index].category + "]: ");
        if (!value.empty()) inventory[index].category = value;
        value = getLine("New brand [" + inventory[index].brand + "]: ");
        if (!value.empty()) inventory[index].brand = value;
        value = getLine("New price (PHP), blank to keep: ");
        if (!value.empty()) {
            try {
                size_t used = 0; double price = stod(value, &used);
                if (used != value.size() || price <= 0) throw invalid_argument("price");
                inventory[index].price = price;
            } catch (...) { cout << "Invalid price; old price retained.\n"; }
        }
        value = getLine("New stock quantity, blank to keep: ");
        if (!value.empty()) {
            try {
                size_t used = 0; int stock = stoi(value, &used);
                if (used != value.size() || stock < 0) throw invalid_argument("stock");
                inventory[index].stockQuantity = stock;
            } catch (...) { cout << "Invalid stock; old stock retained.\n"; }
        }
        cout << "Product updated.\n";
    }

    void deleteProduct() {
        int id;
        if (!getInt("Product ID to delete: ", id)) return;
        int index = inventory.findIndexById(id);
        if (index == -1) { cout << "Product not found.\n"; return; }
        inventory.removeAt(index); // invokes Lab 1 heap-array shrinking operation
        rebuildTree();             // rebuild BST so deleted ID is no longer searchable
        cout << "Product deleted and dynamic inventory array shrunk.\n";
    }

    void lowStock() const {
        int threshold;
        if (!getInt("Low-stock threshold: ", threshold) || threshold < 0) { cout << "Invalid threshold.\n"; return; }
        bool found = false;
        for (int i = 0; i < inventory.size(); ++i) {
            if (inventory[i].stockQuantity <= threshold) {
                if (!found) printInventoryHeader();
                printProduct(inventory[i]); found = true;
            }
        }
        if (!found) cout << "No products at or below that stock threshold.\n";
    }

    void binarySearchDemo() const {
        vector<int> ids;
        for (int i = 0; i < inventory.size(); ++i) ids.push_back(inventory[i].id);
        sort(ids.begin(), ids.end());
        int target;
        if (!getInt("Product ID for binary search: ", target)) return;
        int steps = countBinarySteps(ids, target);
        cout << "Binary search comparisons: " << steps << "\n";
        cout << (productTree.searchBST(target) ? "BST: Found\n" : "BST: Not found\n");
    }

    void showLeaderboard() {
        if (sales.empty()) { cout << "No completed sales yet; leaderboard is empty.\n"; return; }
        vector<SalesRecord> ranking = sales;
        rankLeaderboard(ranking);
        cout << "\n====== NEXORA BEST-SELLING PRODUCTS ======\n";
        for (int i = 0; i < static_cast<int>(ranking.size()); ++i)
            cout << "Rank #" << i + 1 << ": " << ranking[i].productName
                 << " - " << ranking[i].unitsSold << " units sold\n";
    }

    void customerMenu() {
        int choice;
        do {
            cout << "\n========== CUSTOMER SECTION ==========\n"
                 << "[1] View all products\n[2] Search by product name\n[3] Search by product ID\n"
                 << "[4] Filter by brand\n[5] Filter by category\n[6] Sort by price\n"
                 << "[7] Add product to cart\n[8] View cart\n[9] Undo last cart addition\n"
                 << "[10] Checkout\n[0] Back to main menu\n";
            if (!getInt("Choice: ", choice)) continue;
            switch (choice) {
                case 1: displayProducts(); break;
                case 2: searchByName(); break;
                case 3: searchById(); break;
                case 4: filterByBrand(); break;
                case 5: filterByCategory(); break;
                case 6: sortByPrice(); break;
                case 7: addToCart(); break;
                case 8: viewCart(); break;
                case 9: undoCartAdd(); break;
                case 10: checkout(); break;
                case 0: break;
                default: cout << "Invalid menu choice.\n";
            }
        } while (choice != 0);
    }

    void adminMenu() {
        int choice;
        do {
            cout << "\n========== ADMINISTRATOR PANEL ==========\n"
                 << "[1] View inventory\n[2] Add product\n[3] Update product\n[4] Delete product\n"
                 << "[5] Search product by ID\n[6] Display low-stock products\n"
                 << "[7] View completed order IDs\n[8] View latest order ID (Lab 2)\n"
                 << "[9] Display BST product IDs\n[10] Binary-search comparison demo\n"
                 << "[11] Best-selling products leaderboard\n[12] Check balanced parentheses (Lab 4)\n"
                 << "[0] Back to main menu\n";
            if (!getInt("Choice: ", choice)) continue;
            switch (choice) {
                case 1: displayProducts(); break;
                case 2: addProduct(); break;
                case 3: updateProduct(); break;
                case 4: deleteProduct(); break;
                case 5: searchById(); break;
                case 6: lowStock(); break;
                case 7: orderHistory.display(); break;
                case 8: {
                    int last = orderHistory.getLastElement();
                    if (last == -1) cout << "No completed orders yet.\n";
                    else cout << "Latest completed order: ORD-" << last << "\n";
                    break;
                }
                case 9: productTree.displaySortedIDs(); break;
                case 10: binarySearchDemo(); break;
                case 11: showLeaderboard(); break;
                case 12: {
                    string expr = getLine("Enter parentheses expression: ");
                    cout << (isBalanced(expr) ? "Valid: balanced parentheses.\n" : "Invalid: unbalanced parentheses.\n");
                    break;
                }
                case 0: break;
                default: cout << "Invalid menu choice.\n";
            }
        } while (choice != 0);
    }

public:
    NexoraGaming() : nextOrderId(1001) {
        seedInventory();
        featuredBrands.add("Logitech G");
        featuredBrands.add("Razer");
        featuredBrands.add("HyperX");
        featuredBrands.add("SteelSeries");
    }

    void run() {
        int choice;
        do {
            cout << "\n===============================================\n"
                 << "               NEXORA GAMING\n"
                 << "             GEAR UP. PLAY BETTER.\n"
                 << "          GAMING ACCESSORIES STORE\n"
                 << "===============================================\n"
                 << "Featured partner: " << featuredBrands.nextBrand() << "\n"
                 << "[1] Customer section\n[2] Administrator panel\n"
                 << "[3] View next featured partner brand\n[0] Exit\n";
            if (!getInt("Enter your choice: ", choice)) continue;
            switch (choice) {
                case 1: customerMenu(); break;
                case 2: adminMenu(); break;
                case 3: cout << "Featured brand: " << featuredBrands.nextBrand() << "\n"; break;
                case 0: cout << "Thank you for visiting NEXORA GAMING!\n"; break;
                default: cout << "Invalid menu choice. Please try again.\n";
            }
        } while (choice != 0);
    }
};

int main() {
    NexoraGaming store;
    store.run();
    return 0;
}
