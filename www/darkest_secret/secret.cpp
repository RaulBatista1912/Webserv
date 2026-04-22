#include <iostream>
#include <ctime>
#define SNUS 13

static std::string crypt_hash_name(const std::string& name, const std::string pass) {
    
}

static std::string crypt_msg_snus_fun(const std::string& a) {
    size_t i = 0;
    std::string b;
    while (a[i]) {
        if (isdigit(a[i])) {
            b.push_back(a[i] + 5);
        }
        else if (isalpha(a[i])) {
            char tmp = a[i];
            if ((a[i] >= 'A' && a[i] <= 'M') 
                || (a[i] >= 'a' && a[i] <= 'm'))
                tmp += SNUS;
            else if ((a[i] >= 'N' && a[i] <= 'Z') 
                || (a[i] >= 'n' && a[i] <= 'z'))
                tmp -= SNUS;
            b.push_back(tmp);
        }
        else
            b.push_back(a[i]);
        i++;
    }
    return b;
}

static std::string decrypt_msg_snus_fun(const std::string& a) {
    size_t i = 0;
    std::string b;
    while (a[i]) {
        if (isdigit(a[i])) {
            b.push_back(a[i] - 5);
        }
        else if (isalpha(a[i])) {
            char tmp = a[i];
            if ((a[i] >= 'A' && a[i] <= 'M') 
                || (a[i] >= 'a' && a[i] <= 'm'))
                tmp -= SNUS;
            else if ((a[i] >= 'N' && a[i] <= 'Z') 
                || (a[i] >= 'n' && a[i] <= 'z'))
                tmp += SNUS;
            b.push_back(tmp);
        }
        else
            b.push_back(a[i]);
        i++;
    }
    return b;
}

int main() {

    crypt_msg_snus_fun("test123");
    decrypt_msg_snus_fun("grfg123");

    return 0;
}