#include <iostream> // 1. Inclui a biblioteca para 'std::cin' e 'std::cout'.
#include <string>   // 2. Inclui a biblioteca para usar 'std::string'.

int main() {
    std::string input; // 3. Declara uma string para armazenar a entrada.

    // 4. 'std::getline' lê uma linha completa de texto da entrada padrão (std::cin).
    // O sistema operacional, por baixo dos panos, está lendo os dados do pipe que foi redirecionado pelo processo pai.
    std::getline(std::cin, input);

    // 5. 'std::cout' imprime a mensagem na saída padrão.
    // O sistema operacional, por baixo dos panos, está escrevendo os dados no pipe que o processo pai está lendo.
    std::cout << "Filho: Recebi a mensagem -> " << input << "\n";

    std::string response = "Mensagem recebida com sucesso!"; // 6. Declara uma string para a resposta.
    // 7. Imprime a resposta para a saída padrão. Esta mensagem será lida pelo processo pai.
    std::cout << response << "\n";

    return 0;
}