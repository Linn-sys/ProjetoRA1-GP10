#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string> 
#include <vector>

#pragma comment(lib, "ws2_32.lib")

int main() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cerr << "WSAStartup falhou: " << result << std::endl;
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		std::cerr << "Erro ao criar socket." << std::endl;
		WSACleanup();
		return 1;
	}

	sockaddr_in hint{};
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr);

	if (connect(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		std::cerr << "Erro ao conectar no servidor." << std::endl;
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	std::cout << "[Cliente] Conectado ao servidor em 127.0.0.1:54000.\n";

	std::string msg;
	char buf[4096];

	while (std::getline(std::cin, msg)) { // Altera para ler do stdin
		if (msg == "sair") break;

		// Escapa as aspas duplas da mensagem antes de montar o JSON
		std::string mensagemEscapada = msg;
		size_t pos = 0;
		while ((pos = mensagemEscapada.find('"', pos)) != std::string::npos) {
			mensagemEscapada.replace(pos, 1, "\\\"");
			pos += 2; // Avanca para evitar um loop infinito
		}

		// CORRIGIDO: MONTA O JSON CORRETAMENTE COM ASPAS NAS CHAVES
		std::string jsonMsg = R"({"processo":1,"mensagem":")" + mensagemEscapada + "\"}\n";

		int sendResult = send(sock, jsonMsg.c_str(), (int)jsonMsg.size(), 0);

		if (sendResult != SOCKET_ERROR) {
			// Recebe as duas respostas do Servidor C++
			int bytesReceived1 = recv(sock, buf, 4096, 0);
			if (bytesReceived1 > 0) {
				// Não fazemos nada, apenas garantimos que os dados são lidos
			}
			int bytesReceived2 = recv(sock, buf, 4096, 0);
			if (bytesReceived2 > 0) {
				// e a segunda.
			}
		}
	}
	closesocket(sock);
	WSACleanup();
	return 0;
}