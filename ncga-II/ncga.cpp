#include <iostream>
#include <string>
#include <random>
#include "GraphReader.h"
#include "Model.h"
#include "Greedy.h"

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

    Model model(reader);

    std::random_device rd;
    unsigned int seed = rd();
    Greedy greedy(model, seed);

    int num_solutions = 5;
    auto solutions = greedy.generateSolutions(num_solutions);

    std::string output_file = "solutions.txt";
    greedy.saveSolutions(solutions, output_file);

    std::cout << "Soluciones generadas: " << solutions.size() << std::endl;
    std::cout << "Resultados guardados en " << output_file << std::endl;

    return 0;
}