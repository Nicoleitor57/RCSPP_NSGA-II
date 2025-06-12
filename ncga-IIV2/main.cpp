#include <iostream>
#include <string>
#include <random>
#include "GraphReader.h"



int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <archivo_instancia>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    GraphReader reader(filename);
    if (!reader.isValid()) {
        std::cerr << "Error: No se pudo leer la instancia desde " << filename << std::endl;
        return 1;
    }

}
