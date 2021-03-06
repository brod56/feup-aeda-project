#include "dashboard.h"

Dashboard::Dashboard(Store &store, Person *person) : UI(store), _person(person){
}

void Dashboard::show() {
    printLogo({});
    std::cout << SEPARATOR;
    print(_person->getName(),util::BLUE);

    std::cout << "\nUsername: " << _person->getCredential().username
    << "\nPassword: " << std::string(_person->getCredential().password.size(),'*')
    << "\n\nTax ID: " << ((_person->getTaxId() == Person::DEFAULT_TAX_ID) ?
    "Not provided" : std::to_string(_person->getTaxId()));
}

void Dashboard::manageOrders(Client *client, Worker* worker) {
    printLogo("Manage orders");
    std::cout << SEPARATOR;
    bool hasOrders = _store.orderManager.print(std::cout,client,worker);
    std::cout << SEPARATOR << "\n";

    if (hasOrders) {
        std::vector<std::string> options = {
                "expand <index> - view order details",
                "edit <index> - edit order details"
        };
        if (client){
            options.emplace_back("deliver <index> <evaluation> - mark order as delivered and evaluate it");
            options.emplace_back("remove <index> - cancel requested order");
        }
        printOptions(options);
    }

    for(;;){
        try {
            std::string input = readCommand();
            if (input == BACK) return;
            if (hasOrders && client != nullptr && validInput1Cmd2ArgsDigit(input, "deliver")) {
                unsigned long idx = std::stoul(to_words(input).at(1)) - 1;
                int eval = std::stoi(to_words(input).at(2));
                _store.orderManager.deliver(_store.orderManager.get(idx, client), eval, true);
                break;
            } else if (hasOrders && client != nullptr && validInput1Cmd1ArgDigit(input,"remove")){
                unsigned long idx = std::stoul(to_words(input).at(1)) - 1;
                _store.orderManager.remove(idx);
                break;
            } else if (hasOrders && validInput1Cmd1ArgDigit(input, "expand")) {
                unsigned long idx = std::stoul(to_words(input).at(1)) - 1;
                expandOrder(_store.orderManager.get(idx, client));
                break;
            } else if (hasOrders && validInput1Cmd1ArgDigit(input,"edit")){
                unsigned long idx = std::stoul(to_words(input).at(1)) - 1;
                editOrder(_store.orderManager.get(idx, client));
                break;
            }
            else printError();
        }
        catch (std::exception& e){
            std::cout << e.what() << "\n";
        }
    }
    manageOrders(client, worker);
}

void Dashboard::changeCredential(Person *person){
    std::cout << "\n" << SEPARATOR;
    std::string username, password;

    for (;;) { // while credential is not valid
        try {
            for (;;) {
                std::cout << "New username: ";
                std::string input1 = readCommand();
                if (input1 == BACK) return;
                std::cout << "Confirm new username: ";
                std::string input2 = readCommand();
                if (input2 != input1) std::cout << "Usernames do not match! Try again.\n";
                else if (to_words(input1).size() != 1) std::cout << "Must have exactly one word! Try again.\n";
                else {
                    username = input1;
                    break;
                }
            }
            for (;;) {
                std::cout << "\nNew password: ";
                std::string input1 = readCommand();
                std::cout << "Confirm new password: ";
                std::string input2 = readCommand();
                if (input2 != input1) std::cout << "Passwords do not match! Try again.\n";
                else if (to_words(input1).size() != 1) std::cout << "Must have exactly one word! Try again.\n";
                else {
                    password = input1;
                    break;
                }
            }
            person->setCredential({username,password});
            break;
        }
        catch(std::exception& e){
            std::cout << "\n" << e.what() << "\n\n";
        }
    }
}

void Dashboard::expandOrder(Order *order) const {
    printLogo("Order details");
    std::cout << SEPARATOR;
    order->print(std::cout);
    std::cout << SEPARATOR << "\n";

    for (;;) {
        std::string input = readCommand();
        if (input == BACK) return;
        else printError();
    }
}

void Dashboard::managePersonalData(Person *person) {
    printLogo("Edit account details");
    bool isClient = person->getRole() == PersonRole::CLIENT;

    std::vector<std::string> options = {
            "change credential - set new username or password",
            "change name - change your name",
            "change taxid - change tributary number"
    };
    if (isClient){
        options.emplace_back("set premium - make myself a premium client");
        options.emplace_back("set basic - make myself a basic client");
    }

    printOptions(options);
    for (;;){
        try {
            std::string input = readCommand();
            if (input == BACK) return;
            else if (validInput1Cmd1Arg(input, "change", "credential")) {
                changeCredential(person);
                break;
            } else if (validInput1Cmd1Arg(input, "change", "name")) {
                changeName(person);
                break;
            } else if (isClient && validInput1Cmd1Arg(input, "set", "basic")) {
                _store.clientManager.getClient(person->getTaxId())->setPremium(false);
                break;
            }
            else if (isClient && validInput1Cmd1Arg(input, "set", "premium")) {
                _store.clientManager.getClient(person->getTaxId())->setPremium(true);
                break;
            }
            else if (validInput1Cmd1Arg(input,"change","taxid")){
                changeTaxID(person);
                break;
            }
            else printError();
        }
        catch (std::exception& e){
            std::cout << e.what() << "\n";
        }
    }
}

void Dashboard::changeName(Person *person) {
    std::cout << "\n" << SEPARATOR;
    for(;;){
        try {
            std::cout << "New name: ";
            std::string input1 = readCommand(false);
            if (input1 == BACK) return;
            std::cout << "Confirm new name: ";
            std::string input2 = readCommand(false);
            if (input2 != input1) std::cout << "Names do not match! Try again.\n";
            else {
                person->setName(input1);
                break;
            }
        }
        catch (std::exception& e){
            std::cout << e.what() << "\n\n";
        }
    }
}

void Dashboard::printLogo(const std::string &detail) const {
    util::clearScreen();
    std::string title = _store.getName() + " - " + _person->getName() + "'s personal area";
    if (!detail.empty()) title += " - " + detail;
    uppercase(title);
    util::print(title + "\n\n",util::BLUE);
}

void Dashboard::editOrder(Order* order) {
    if (!_store.orderManager.has(order)) throw std::invalid_argument("Couldn't find order to edit");
    for(;;){
        printLogo("Edit order");
        std::cout << "AVAILABLE STOCK\n" << SEPARATOR;
        _store.productManager.print(std::cout, false);
        std::cout << SEPARATOR << "\n"
                  << "ORDER DETAILS\n" << SEPARATOR;
        order->print(std::cout);
        std::cout << SEPARATOR << "\n";

        const std::vector<std::string> options = {
                "add <product_number> <quantity> - add product from stock",
                "remove <product_number> - remove product from order",
                "change location - set new deliver location"
        };
        printOptions(options);

        for(;;) {
            try {
                std::string input = readCommand();
                if (input == BACK){
                    if (order->getProducts().empty()) _store.orderManager.remove(order);
                    return;
                }
                else if (validInput1Cmd2ArgsDigit(input, "add")) {
                    unsigned long idx = std::stoul(to_words(input).at(1)) - 1;
                    unsigned int quantity = (unsigned) std::stoi(to_words(input).at(2));
                    _store.orderManager.addProduct(order,_store.productManager.get(idx), quantity);
                    break;
                } else if (validInput1Cmd1ArgDigit(input, "remove")) {
                    unsigned long idx = std::stoul(to_words(input).at(1)) - 1;
                    _store.orderManager.removeProduct(order,idx);
                    break;
                } else if (validInput1Cmd1Arg(input,"change","location")){
                    setOrderLocation(order);
                    break;
                }
                else printError();
            }
            catch (std::exception &e) {
                std::cout << e.what() << "\n";
            }
        }
    }
}

void Dashboard::manageStock() {
    for (;;){
        printLogo("Manage stock");
        std::cout << SEPARATOR;
        _store.productManager.print(std::cout);
        std::cout << SEPARATOR << "\n";

        const std::vector<std::string> options = {
                "remove <number> - remove product from existing stock",
                "add cake - add a new cake to the stock",
                "add bread - add a new bread to the store"
        };
        printOptions(options);

        for (;;){
            try {
                std::string input = readCommand();
                if (input == BACK) return;
                else if (validInput1Cmd1Arg(input, "add", "cake")) {
                    addCake();
                    break;
                } else if (validInput1Cmd1Arg(input, "add", "bread")) {
                    addBread();
                    break;
                } else if (validInput1Cmd1ArgDigit(input, "remove")){
                    unsigned long idx = std::stoul(to_words(input).at(1)) - 1;
                    _store.productManager.remove(idx);
                    break;
                } else printError();
            }
            catch (std::exception& e){
                std::cout << e.what() << "\n";
            }
        }
    }
}

void Dashboard::addBread() {
    std::cout << "\n" << SEPARATOR;
    std::string name, input;
    float price;
    bool small;

    std::cout << "Bread name: ";
    name = readCommand(false);
    if (name == BACK) return;

    for(;;){
        std::cout << "Unit price: ";
        input = readCommand();
        if (isdigit(input,true)) break;
        else std::cout << "The price must be a float. Do not include the coin symbol.\n";
    }
    price = std::stof(input);

    for (;;){
        std::cout << "Size (small/big): ";
        input = readCommand();
        if (input == "small"){
            small = true;
            break;
        }
        else if (input == "big"){
            small = false;
            break;
        }
        else std::cout << "Small/big are the only accepted inputs.\n";
    }
    _store.productManager.addBread(name,price,small);
}

void Dashboard::addCake() {
    std::cout << "\n" << SEPARATOR;
    std::string name, input;
    float price;
    CakeCategory category = CakeCategory::GENERAL;

    std::cout << "Cake name: ";
    name = readCommand(false);
    if (name == BACK) return;

    for(;;){
        std::cout << "Unit price: ";
        input = readCommand();
        if (isdigit(input,true)) break;
        else std::cout << "The price must be a float. Do not include the coin symbol.\n";
    }
    price = std::stof(input);

    const std::vector<std::string> categories = Cake::getCategories();
    for (;;){
        std::cout << "Category (";
        for (unsigned long i = 0; i < categories.size(); ++i){
            std::cout << categories.at(i);
            if (i != categories.size() - 1) std::cout << ", ";
        }
        std::cout << "): ";

        input = readCommand();
        bool found = false;
        for (unsigned long i = 0; i < categories.size(); ++i) {
            std::string catName = categories.at(i);
            lowercase(catName);
            if (input == catName) {
                category = static_cast<CakeCategory>(i);
                found = true;
                break;
            }
        }
        if (!found) std::cout << "Unrecognized category. Read carefully!\n";
        else break;
    }
    _store.productManager.addCake(name,price,category);
}

void Dashboard::changeTaxID(Person *person) {
    std::cout << "\n" << SEPARATOR;
    for(;;){
        try {
            std::cout << "New tax ID: ";
            std::string input1 = readCommand(false);
            if (input1 == BACK) return;
            std::cout << "Confirm new tax ID: ";
            std::string input2 = readCommand(false);
            if (input2 != input1) std::cout << "IDs did not match! Try again.\n";
            else if (isdigit(input1)){
                person->setTaxID(std::stoul(input1));
                break;
            }
            else std::cout << "Please enter a reasonable sized integer number!\n";
        }
        catch (std::exception& e){
            std::cout << e.what() << "\n\n";
        }
    }
}

void Dashboard::manageClients() {
    for (;;){
        printLogo("Manage clients");
        std::cout << SEPARATOR;
        bool hasClients = _store.clientManager.print(std::cout);
        std::cout << SEPARATOR << "\n";

        std::vector<std::string> options = {
                "add client - register account"
        };
        if (hasClients) options.emplace_back("kick <index> - remove client account");
        printOptions(options);

        for (;;){
            try {
                std::string input = readCommand();
                if (input == BACK) return;
                else if (hasClients && validInput1Cmd1ArgDigit(input, "kick")) {
                    unsigned long idx = std::stoul(to_words(input).at(1)) - 1;
                    _store.clientManager.remove(idx);
                    break;
                } else if (validInput1Cmd1Arg(input, "add", "client")) {
                    addClient();
                    break;
                } else printError();
            }
            catch (std::exception& e){
                std::cout << e.what() << "\n";
            }
        }
    }
}

void Dashboard::addClient() {
    std::cout << "\n" << SEPARATOR;
    std::string name, input;
    unsigned long taxID;
    bool premium;

    std::cout << "Client name: ";
    name = readCommand(false);
    if (name == BACK) return;

    for(;;){
        std::cout << "Tax ID: ";
        input = readCommand();
        if (isdigit(input)) break;
        else std::cout << "The ID must be a reasonable sized integer!\n";
    }
    taxID = std::stoul(input);

    for (;;){
        std::cout << "Status (basic/premium): ";
        input = readCommand();
        if (input == "basic"){
            premium = false;
            break;
        }
        else if (input == "premium"){
            premium = true;
            break;
        }
        else std::cout << "Basic/premium are the only accepted inputs.\n";
    }
    _store.clientManager.add(name,taxID,premium);
}

void Dashboard::setOrderLocation(Order *order) {
    std::cout << "\n" << SEPARATOR
    << "New location (";

    std::string locations;
    for (const auto& l: _store.locationManager.getAll()){
        locations += l + ", ";
    }
    if (!locations.empty()) locations = locations.substr(0,locations.size()-2);

    std::cout << locations << "): ";
    for(;;){
        try{
            std::string input = readCommand(false);
            if (input == BACK) return;
            if (!_store.locationManager.has(input)) throw LocationDoesNotExist(input);
            _store.orderManager.setDeliveryLocation(order,input);
            break;
        }
        catch(std::exception& e){
            std::cout << e.what() << "\n";
        }
    }
}
