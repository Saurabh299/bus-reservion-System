#pragma once
#include <iostream>
#include <string>

class Payment {
protected:
    int amount;

public:
    explicit Payment(int amt) : amount(amt) {}
    virtual ~Payment() = default;
    virtual void makePayment() = 0;
    int getAmount() const { return amount; }
};

class CreditCardPayment : public Payment {
private:
    std::string cardNumber;
    std::string cardHolderName;
    std::string expiryDate;

public:
    CreditCardPayment(int amt, const std::string& cardNum,
        const std::string& holderName, const std::string& expiry)
        : Payment(amt), cardNumber(cardNum),
        cardHolderName(holderName), expiryDate(expiry) {
    }

    void makePayment() override {
        std::cout << "Processing credit card payment of Rs." << amount
            << " for " << cardHolderName << "\n";
    }
};

class UPI : public Payment {
private:
    std::string upiId;

public:
    UPI(int amt, const std::string& upi) : Payment(amt), upiId(upi) {}

    void makePayment() override {
        std::cout << "Processing UPI payment of Rs." << amount
            << " to UPI ID: " << upiId << "\n";
    }
};