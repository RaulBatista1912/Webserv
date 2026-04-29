#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <fstream>

static std::string generate(int gen, int range) {
    std::string r;
    std::ifstream urandom("/dev/urandom");
    if (!urandom)
        return NULL;
    unsigned int num = 0;
    for (size_t i = 0; i < gen; i++)
    {
        if (!urandom.read(reinterpret_cast<char *>(&num), sizeof(num)))
            return NULL;
        int res = (int)(num % range);
        r += std::to_string(res);
        if (i < gen - 1)
            r += ", ";
    }
    return r;
}

int main(int ac, char **av) {
    if (ac < 3) {
        std::cout << "Usage: ./name [many num] [range max]";
        return 1;
    }

    std::string r = generate(atoi(av[1]), atoi(av[2]));
    std::cout << r << std::endl;

    return 0;
}