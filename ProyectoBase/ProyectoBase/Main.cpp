#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#define MAX_MESSAGES 30
#define DEFAULT_PORT 5000

sf::TcpSocket theSocket;
sf::TcpListener theListener;
std::mutex mutex;
bool isEnd;
char buffer[2000];
int userNum = 1;

/// TALLER 1
// Receive Mode 1 
void Receive(size_t received, std::vector<std::string>* messages, sf::RenderWindow* window) {

	while (!isEnd) { // while we don't write "exit"
		
		sf::Socket::Status status = theSocket.receive(buffer, 2000, received);

		if (status == sf::Socket::Disconnected) 
			isEnd = true;
		
		mutex.lock();
		if (status == sf::Socket::Done) 
			messages->push_back(buffer);
		mutex.unlock();
		
	}
}

// Receive Mode 3
void ReceiveSS(size_t received, std::vector<std::string>* messages, sf::RenderWindow* window, sf::SocketSelector* ss) {

	while (!isEnd) { // while we don't write "exit"

		while (ss->wait()){

			if (ss->isReady(theSocket)){
				
				sf::Socket::Status status = theSocket.receive(buffer, 2000, received);

				if (status == sf::Socket::Disconnected)
					isEnd = true;

				mutex.lock();
				if (status == sf::Socket::Done)
					messages->push_back(buffer);
				mutex.unlock();
			}
		}
	}
}

// Mode 1: Blocking + Threading
void MessagesThreads(std::string type) {

	size_t received;
	std::vector<std::string> messages;

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), type);

	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
		std::cout << "Can't load the font file" << std::endl;

	sf::String message = " >";

	sf::Text chattingText(message, font, 18);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);

	sf::Text text(message, font, 24);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	std::string messageStr;

	// Receive
	std::thread thread(&Receive, received, &messages, &window);

	while (window.isOpen())
	{
		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
				else if (evento.key.code == sf::Keyboard::Return)
				{
					mutex.lock();
					messages.push_back(message);
					if (messages.size() > MAX_MESSAGES)
					{
						messages.erase(messages.begin(), messages.begin() + 1);
					}
					mutex.unlock();

					// Send
					messageStr = type + ": ";
					messageStr += message.substring(2, message.getSize());
					
					theSocket.send(messageStr.c_str(), messageStr.size() + 1);

					if (message == ">exit" || message == " >exit") {
						isEnd = true;
						theSocket.disconnect();
						theListener.close();
						exit(0);
					}

					message = ">";
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					message += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && message.getSize() > 0)
					message.erase(message.getSize() - 1, message.getSize());
				break;
			}
		}

		window.draw(separator);

		for (size_t i = 0; i < messages.size(); i++)
		{
			std::string chatting = messages[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}

		std::string mensaje_ = message + "_";
		text.setString(mensaje_);
		window.draw(text);

		window.display();
		window.clear();
	}
	
	// Cleanup
	thread.join();
	theSocket.disconnect();
}

// Mode 2: NonBlocking
void MessagesNonBlocking(std::string type) {

	theSocket.setBlocking(false);

	size_t received, sent;
	std::vector<std::string> messages;
	sf::Socket::Status sStatus;

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), type);

	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
		std::cout << "Can't load the font file" << std::endl;

	sf::String mensaje = " >";

	sf::Text chattingText(mensaje, font, 18);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);

	sf::Text text(mensaje, font, 24);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	std::string mensajeStr;

	while (window.isOpen())
	{
		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
				else if (evento.key.code == sf::Keyboard::Return)
				{
					messages.push_back(mensaje);
					if (messages.size() > MAX_MESSAGES)
					{
						messages.erase(messages.begin(), messages.begin() + 1);
					}

					// Send
					mensajeStr = type + ": ";
					mensajeStr += mensaje.substring(2, mensaje.getSize());
					
					if (mensaje == ">exit" || mensaje == " >exit") {
						isEnd = true;
						theSocket.disconnect();
						theListener.close();
						exit(0);
					}

					do
					{
						sStatus = theSocket.send(mensajeStr.c_str(), mensajeStr.size() + 1, sent);

						if (sStatus == sf::Socket::Status::Partial)
							mensajeStr = mensajeStr.substr(sent + 1, mensajeStr.size() - sent);

					} while (sStatus == sf::Socket::Status::Partial);

					mensaje = ">";
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
				break;
			}
		}

		sStatus = theSocket.receive(buffer, sizeof(buffer), received);
		mensajeStr = buffer;
		if (sStatus == sf::Socket::Status::Done)
		{
			messages.push_back(mensajeStr);
			if (messages.size() > 25)
				messages.erase(messages.begin(), messages.begin() + 1);
		}

		window.draw(separator);

		for (size_t i = 0; i < messages.size(); i++)
		{
			std::string chatting = messages[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}
		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		window.draw(text);


		window.display();
		window.clear();
	}

	// Cleanup
	theSocket.disconnect();
}

// Mode 3: Blocking + SocketSelector
void MessagesSelector(std::string type) {

	sf::SocketSelector socketS;
	socketS.add(theSocket); // pass socket to selector
	size_t received;
	std::vector<std::string> messages;

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), type);

	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
		std::cout << "Can't load the font file" << std::endl;

	sf::String message = " >";

	sf::Text chattingText(message, font, 18);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);

	sf::Text text(message, font, 24);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	std::string messageStr;

	// Receive
	std::thread thread(&ReceiveSS, received, &messages, &window, &socketS);

	while (window.isOpen())
	{
		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
				else if (evento.key.code == sf::Keyboard::Return)
				{
					messages.push_back(message);
					if (messages.size() > MAX_MESSAGES)
					{
						messages.erase(messages.begin(), messages.begin() + 1);
					}

					// Send
					messageStr = type + ": ";
					messageStr += message.substring(2, message.getSize());

					theSocket.send(messageStr.c_str(), messageStr.size() + 1);

					if (message == ">exit" || message == " >exit") {
						isEnd = true;
						theSocket.disconnect();
						theListener.close();
						exit(0);
					}

					message = ">";
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					message += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && message.getSize() > 0)
					message.erase(message.getSize() - 1, message.getSize());
				break;
			}
		}

		window.draw(separator);

		for (size_t i = 0; i < messages.size(); i++)
		{
			std::string chatting = messages[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}

		std::string mensaje_ = message + "_";
		text.setString(mensaje_);
		window.draw(text);

		window.display();
		window.clear();
	}

	// Cleanup
	socketS.clear();
	thread.join();
	theSocket.disconnect();
}

/// TALLER 2
void ReceiveTaller2(sf::TcpSocket* socket, size_t* received, std::vector<std::string>* messages, sf::RenderWindow* window) {

	while (window->isOpen()) {
		char buffer[2000];
		sf::Socket::Status status = socket->receive(buffer, sizeof(buffer), *received);
		std::string str = buffer;

		if (status == sf::Socket::Status::Done) {
			mutex.lock();
			messages->push_back(str);
			if (messages->size() > MAX_MESSAGES)
				messages->erase(messages->begin(), messages->begin() + 1);
			mutex.unlock();
		}
	}
}

void DisconnectTaller2(std::vector<sf::TcpSocket*>& list, sf::SocketSelector *ss, int userNum) {

	ss->remove(*list[userNum]); // remove from ss, because it has disconnected
	list[userNum]->disconnect(); // disconnect

	delete(list[userNum]);
	list.erase(list.begin() + userNum, list.begin() + userNum + 1);

	for (sf::TcpSocket* socket : list) {
		std::string outMsn = "El cliente " + std::to_string(userNum+1) + " se ha desconectado!";
		socket->send(outMsn.c_str(), outMsn.size() + 1);
	}
	std::cout << "El cliente " + std::to_string(userNum+1) + " se ha desconectado!" << std::endl;
}

void ServerTaller2() {

	sf::SocketSelector sSelector;
	std::vector<sf::TcpSocket*> sList;
	sf::TcpSocket::Status sStatus;
	sf::TcpListener listener;
	sf::TcpListener::Status lStatus;

	char buffer[2000];
	std::string message;
	size_t received;
	//int userNum = 1;

	lStatus = listener.listen(DEFAULT_PORT);
	sSelector.add(listener); // pass listener to selector

	while (lStatus != sf::TcpListener::Status::Disconnected) {

		// while not disconnected wait for a message from socket
		while (sSelector.wait()) {

			// check if it's listener
			if (sSelector.isReady(listener)) {
				sf::TcpSocket* socket = new sf::TcpSocket();
				sf::TcpListener::Status st = listener.accept(*socket);
				if (st == sf::TcpListener::Status::Done)
					sList.push_back(socket);

				sSelector.add(*socket);

				message = "Se ha conectado el cliente " + std::to_string(userNum) + "!";
				for (sf::TcpSocket* s : sList)
					if (s != socket)s->send(message.c_str(), message.size() + 1);

				std::cout << "Se ha conectado el cliente " + std::to_string(userNum) + "!" << std::endl;
				userNum++;
			}

			// if not listener, socket
			else {

				for (int i = 0; i < sList.size(); i++) {

					if (sSelector.isReady(*sList[i])) {

						// Receive
						sStatus = sList[i]->receive(buffer, sizeof(buffer), received);

						if (sStatus == sf::TcpSocket::Status::Done) {
							message = buffer;
							for (int j = 0; j < sList.size(); j++) {
								sf::TcpSocket::Status st = sList[j]->send(message.c_str(), message.size() + 1);

								if (st == sf::TcpSocket::Status::Error)
									std::cout << "Error" << std::endl;
							}
						}
						else if (sStatus == sf::TcpSocket::Status::Disconnected)
							DisconnectTaller2(sList, &sSelector, i);
						break;
					}
				}
			}
		}
	}

	// Cleanup
	sSelector.clear();
	listener.close();
	for (sf::TcpSocket* &socket : sList)
		socket->disconnect();
	sList.clear();
}

void ClientTaller2() {

	sf::TcpSocket socket;
	sf::TcpSocket::Status socketStatus;
	size_t received;
	sf::IpAddress ip = sf::IpAddress::getLocalAddress(); // ip local
	socket.connect(ip, DEFAULT_PORT);

	std::vector<std::string> messages;
	sf::Vector2i screenDimensions(800, 600);
	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Cliente");

	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
		std::cout << "Can't load the font file" << std::endl;

	sf::String mensaje = " >";

	sf::Text chattingText(mensaje, font, 24);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);

	sf::Text text(mensaje, font, 24);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	std::string messageStr;

	// Receive
	std::thread t(ReceiveTaller2, &socket, &received, &messages, &window);

	while (window.isOpen())
	{
		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
				else if (evento.key.code == sf::Keyboard::Return)
				{
					// Send
					messageStr = "Cliente:"; //+ std::to_string(userNum);
					messageStr += mensaje;
					socketStatus = socket.send(messageStr.c_str(), messageStr.size() + 1);

					if (messageStr == ">exit" || messageStr == " >exit") {
						socket.disconnect();
						exit(0);
					}

					if (socketStatus == sf::TcpSocket::Status::Disconnected) {
						messageStr = "Servidor desconectado!";
						messages.push_back(messageStr);
						if (messages.size() > MAX_MESSAGES)
							messages.erase(messages.begin(), messages.begin() + 1);
					}

					mensaje = ">";
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
				break;
			}
		}

		window.draw(separator);
		for (size_t i = 0; i < messages.size(); i++) {
			std::string chatting = messages[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}
		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		window.draw(text);

		window.display();
		window.clear();
	}

	// Cleanup
	socket.disconnect();
	t.join();
}

// MAIN
int main() {

	// Determinar el taller
	char tallerType;
	std::cout << "Introduce (1) para Taller 1 o (2) para Taller 2: ";
	std::cin >> tallerType;

	/// Taller 1
	if (tallerType == '1') {
		std::cout << "Has seleccionado Taller 1" << std::endl << std::endl;
		// Determinar si es servidor o cliente
		char connectionType;
		std::cout << "Introduce (s) para Servidor o (c) para Cliente: ";
		std::cin >> connectionType;

		// Si es el servidor
		if (connectionType == 's') {

			sf::Socket::Status status;
			sf::Packet packet;

			// Seleccionamos el modo
			int mode;
			std::cout << "Has seleccionado Servidor" << std::endl << std::endl;
			std::cout << "Ahora elige el modo con un (numero): " << std::endl;
			std::cout << "1. Blocking + Threading" << std::endl;
			std::cout << "2. NonBlocking" << std::endl;
			std::cout << "3. Blocking + SocketSelector" << std::endl;
			std::cin >> mode;

			status = theListener.listen(DEFAULT_PORT);

			if (status != sf::Socket::Done) {
				std::cout << "No se puede vincular al puerto 5000" << std::endl;
				exit(0);
			}

			if (theListener.accept(theSocket) != sf::Socket::Done) {
				std::cout << "Error al acceptar la conexion" << std::endl;
			}
			else {
				packet << mode;
				theSocket.send(packet);
				std::cout << "Enviado packet del mode" << std::endl;

				switch (mode) {
				case 1:
					std::cout << "Has seleccionado modo Blocking + Threading" << std::endl;
					MessagesThreads("Servidor");
					break;
				case 2:
					std::cout << "Has seleccionado modo NonBlocking" << std::endl;
					MessagesNonBlocking("Servidor");
					break;
				case 3:
					std::cout << "Has seleccionado modo Blocking + SocketSelector" << std::endl;
					MessagesSelector("Servidor");
					break;
				}
			}
			theListener.close();
		}

		// Si es el cliente
		else if (connectionType == 'c') {

			// ip localhost
			sf::IpAddress ip = sf::IpAddress::getLocalAddress();
			sf::Socket::Status status = theSocket.connect("localhost", DEFAULT_PORT, sf::milliseconds(15.f));

			if (status != sf::Socket::Done) {
				std::cout << "Error al establecer conexion" << std::endl;
				exit(0);
			}
			else {
				std::cout << "Se ha establecido conexion con el Servidor\n";
				int mode = 0;
				sf::Packet packet;
				theSocket.receive(packet);
				packet >> mode;

				switch (mode) {
				case 1:
					std::cout << "Has seleccionado modo Blocking + Threading" << std::endl;
					MessagesThreads("Cliente");
					break;
				case 2:
					std::cout << "Has seleccionado modo NonBlocking" << std::endl;
					MessagesNonBlocking("Cliente");
					break;
				case 3:
					std::cout << "Has seleccionado modo Blocking + SocketSelector" << std::endl;
					MessagesSelector("Cliente");
					break;
				}
			}
		}
	}
	/// Taller 2
	else if (tallerType == '2') {
		std::cout << "Has seleccionado Taller 2" << std::endl << std::endl;
		// Determinar si es servidor o cliente
		char connectionType;
		std::cout << "Introduce (s) para Servidor o (c) para Cliente: ";
		std::cin >> connectionType;

		// Si es el servidor
		if (connectionType == 's') {
			ServerTaller2();
		}
		// Si es el cliente
		else if (connectionType == 'c') {
			ClientTaller2();
		}
	}
}