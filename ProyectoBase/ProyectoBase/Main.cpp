#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
using namespace std;

#define MAX_MESSAGES 30

std::mutex mut;
sf::TcpSocket socket;
bool isEnd;
char buffer[2000];

void MessagesThreads(string type);																			 // Mode 1: Blocking + Threading
void MessagesNonBlocking(string type);																		 // Mode 2: NonBlocking
void MessagesSelector(string type);																			 // Mode 3: Blocking + SocketSelector
void Receive(size_t received, vector<string>* messages, sf::RenderWindow* window);							 // Receive Mode 1 & 2
void ReceiveSS(size_t received, vector<string>* messages, sf::RenderWindow* window, sf::SocketSelector* ss); // Receive Mode 3

//Tocar threads solo en recursos mensajes, dentro del while lock, receive, aMensajes_pushback, unlock

int main()
{
	//Primero la conexión
	char connectionType;

	cout << "Introduce (s) para Server o (c) para Cliente:";
	cin >> connectionType;

	// Si es el servidor
	if (connectionType == 's')
	{
		sf::TcpListener listener;
		sf::Socket::Status status;
		int mode;
		std::cout << "1. Blocking + Threading" << std::endl;
		std::cout << "2. NonBlocking" << std::endl;
		std::cout << "3. Blocking + SocketSelector" << std::endl;
		std::cin >> mode;
		sf::Packet packet;

		status = listener.listen(5000);

		if (status != sf::Socket::Done) {
			// no se puede vincular al puerto 5000
			std::cout << "No se puede vincular al puerto 5000" << std::endl;
			exit(0);
		}

		if (listener.accept(socket) != sf::Socket::Done) {
			// error al acceptar la conexion
			std::cout << "Error al acceptar la conexion" << std::endl;
		}
		else {
			packet << mode;
			socket.send(packet);
			std::cout << "Enviado packet del mode";

			switch (mode) {
			case 1:
				// Modo Blocking + Threading
				std::cout << "Modo Threads\n";
				MessagesThreads("Server");
				break;
			case 2:
				// Modo NonBlocking
				std::cout << "Modo NonBlocking\n";
				MessagesNonBlocking("Server");
				break;
			case 3:
				// Modo Blocking + SocketSelector
				std::cout << "Modo Blocking + SocketSelector\n";
				MessagesSelector("Server");
				break;
			}
		}
		listener.close();
	}

	//Si es el cliente
	else if (connectionType == 'c')
	{
		// ip localhost
		sf::IpAddress ip = sf::IpAddress::getLocalAddress();
		sf::Socket::Status status = socket.connect("localhost", 5000, sf::milliseconds(15.f));

		if (status != sf::Socket::Done)
		{
			std::cout << "Error al establecer conexion\n";
			exit(0);
		}

		else
		{
			std::cout << "Se ha establecido conexion con el Servidor\n";
			int mode = 0;
			sf::Packet packet;
			socket.receive(packet);
			packet >> mode;

			switch (mode) {
			case 1:
				// Modo Blocking + Threading
				std::cout << "Modo Threads\n";
				MessagesThreads("Client");
				break;
			case 2:
				// Modo NonBlocking
				std::cout << "Modo NonBlocking\n";
				MessagesNonBlocking("Client");
				break;
			case 3:
				// Modo Blocking + SocketSelector
				std::cout << "Modo Blocking + SocketSelector\n";
				MessagesSelector("Client");
				break;
			}
		}
	}

}

// Receive Mode 1 & 2
void Receive(size_t received, vector<string>* messages, sf::RenderWindow* window) {

	while (!isEnd) { // while we don't write "exit"
		mut.lock();
		sf::Socket::Status status = socket.receive(buffer, 2000, received);

		if (status == sf::Socket::Disconnected) 
			isEnd = true;
		
		if (status == sf::Socket::Done) 
			messages->push_back(buffer);
		
		mut.unlock();
	}
}

// Receive Mode 3
void ReceiveSS(size_t received, vector<string>* messages, sf::RenderWindow* window, sf::SocketSelector* ss) {

	while (!isEnd) { // while we don't write "exit"

		while (ss->wait()){

			if (ss->isReady(socket)){
				mut.lock();
				sf::Socket::Status status = socket.receive(buffer, 2000, received);

				if (status == sf::Socket::Disconnected)
					isEnd = true;

				if (status == sf::Socket::Done)
					messages->push_back(buffer);

				mut.unlock();
			}
		}
	}
}

// Mode 1: Blocking + Threading
void MessagesThreads(string type) {

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

	string messageStr;

	// Receive
	thread thread(&Receive, received, &messages, &window);

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
					if (messages.size() > 25)
					{
						messages.erase(messages.begin(), messages.begin() + 1);
					}

					// Send
					messageStr = type + ": ";
					messageStr += message.substring(2, message.getSize());
					
					socket.send(messageStr.c_str(), messageStr.size() + 1);

					if (message == ">exit" || message == " >exit") {
						isEnd = true;
						socket.disconnect();
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
	socket.disconnect();
}

// Mode 2: NonBlocking
void MessagesNonBlocking(string type) {

	socket.setBlocking(false);

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

	string mensajeStr;

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
					if (messages.size() > 25)
					{
						messages.erase(messages.begin(), messages.begin() + 1);
					}

					// Send
					mensajeStr = type + ": ";
					mensajeStr += mensaje.substring(2, mensaje.getSize());
					
					if (mensaje == ">exit" || mensaje == " >exit") {
						isEnd = true;
						socket.disconnect();
						exit(0);
					}

					do
					{
						sStatus = socket.send(mensajeStr.c_str(), mensajeStr.size() + 1, sent);

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

		sStatus = socket.receive(buffer, sizeof(buffer), received);
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
	socket.disconnect();
}

// Mode 3: Blocking + SocketSelector
void MessagesSelector(string type) {

	sf::SocketSelector socketS;
	socketS.add(socket); // pass socket to selector
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

	string messageStr;

	// Receive
	thread thread(&ReceiveSS, received, &messages, &window, &socketS);

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
					if (messages.size() > 25)
					{
						messages.erase(messages.begin(), messages.begin() + 1);
					}

					// Send
					messageStr = type + ": ";
					messageStr += message.substring(2, message.getSize());

					socket.send(messageStr.c_str(), messageStr.size() + 1);

					if (message == ">exit" || message == " >exit") {
						isEnd = true;
						socket.disconnect();
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
	socket.disconnect();
}