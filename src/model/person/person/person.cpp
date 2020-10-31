//
// Created by bdmendes on 29/10/20.
//

#include "person.h"

#include <utility>

Person::Person(std::string name, int tributaryNumber, Credential credential) :
        _name{std::move(name)}, _tributaryNumber{tributaryNumber}, _credential{ std::move(credential) } {
}

std::string Person::getName() const {
    return _name;
}

int Person::getTributaryNumber() const {
    return _tributaryNumber;
}

Credential Person::getCredential() const {
    return _credential;
}

void Person::changeName(const std::string& name){
    if(_name == name) throw InvalidPersonNameChange();
    _name = name;
}

void Person::changeCredential(const Credential &credential) {
    if (_credential == credential) throw InvalidPersonCredentialChange();
    _credential = credential;
}

bool Person::operator<(const Person &p2) const {
    return _name < p2.getName();
}

bool Person::operator==(const Person &p2) const {
    return _name == p2.getName() && _tributaryNumber == p2.getTributaryNumber() &&
    _credential == p2.getCredential();
}