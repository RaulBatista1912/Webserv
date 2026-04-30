#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <fstream>
#include <ostream>

static std::string generate(long gen, long range) {
    std::string r;
    std::ifstream urandom("/dev/urandom");
    if (!urandom)
        return NULL;
    
    bool have_print = false;
    unsigned int num = 0;
    int old_pr = 0;

    for (long i = 0; i < gen; ++i)
    {
        long pr = (i * 100) / gen;
        if ((old_pr + 2) == pr) {
            have_print = false;
        }
        if ((pr % 2) == 0 && !have_print) {
            std::cout << "\r" << "Generating... " << pr << "%";
            std::cout.flush();
            have_print = true;
            old_pr = pr;
        }
        if (!urandom.read(reinterpret_cast<char *>(&num), sizeof(num)))
            return NULL;
        long res = static_cast<long>(num % static_cast<unsigned long>(range));
        r += std::to_string(res);
        if (i < gen - 1)
            r += ", ";
    }
    std::cout << "\r" << "Generating... " << 100 << "%" << std::endl;
    return r;
}

int main(int ac, char **av) {
    if (ac < 4) {
        std::cout << "Usage: ./name [many num] [range max] [in file or not 1/0]";
        return 1;
    }

    std::string r = generate(atol(av[1]), atol(av[2]));

    if (av[3][0] == '1') {
        std::string gen = av[1];
        std::ofstream out("result_gen" + gen + ".txt" );
        out << r;
        out.close();
    }
    else {
        std::cout << r << std::endl;
    }
    return 0;
}