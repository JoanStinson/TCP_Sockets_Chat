#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <mutex>
#include <thread>

using namespace std;

#define MAX_MENSAJES 30

void Server();
void Client();
void Missatges(sf::RenderWindow& windowRef);

int main()
{	
	char connectionType;

	std::cout << "Enter (s) for Server, Enter (c) for Client: ";
	std::cin >> connectionType;

	if (connectionType == 's')
	{
		Server();
	}
	else if (connectionType == 'c')
	{
		Client();
	}
	else {
		exit(0);
	}

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	Missatges(window);

}

void Server() {
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	sf::TcpListener listener;
	sf::Socket::Status status;
	char mode;
	sf::Packet packet;
	
	std::cout << "1. Blocking + Threading" << std::endl;
	std::cout << "2. NonBlocking" << std::endl;
	std::cout << "3. Blocking + SocketSelector" << std::endl;
	std::cin >> mode;

	status = listener.listen(5000);

	if (status != sf::Socket::Done) {
		//no se puede vincular al puerto 5000
		std::cout << "No se puede vincular al puerto 5000" << std::endl;
		exit(0);
	}

	sf::TcpSocket socket;

	if (listener.accept(socket) != sf::Socket::Done) {
		//error al acceptar la conexion
		std::cout << "Error al acceptar la conexion" << std::endl;
	}
	else {
		packet << mode;
		socket.send(packet);
	}

	switch (mode) {
	case '1':

		break;
	case '2':
		break;
	case '3':
		break;
	}

	

	
	
	//listener.close();
}

void Client() {
	sf::TcpSocket socket;
	sf::Packet packet;
	char*mode = 0;

	sf::Socket::Status status = socket.connect("localhost", 50000, sf::milliseconds(15.f));
	
	if (status != sf::Socket::Done)
	{
		std::cout << "Error al establecer conexion\n";
		//exit(0);
	}
	else
	{
		std::cout << "Se ha establecido conexion con el Servidor\n";
		socket.receive(packet);
		packet >> mode;

		if (!(packet >> mode)) {
			//error no hay datos
		}
		else {
			switch (*mode) {
			case '1':
				//Modo Blocking + Threading
				break;
			case '2':
				//Modo NonBlocking
				break;
			case '3':
				//Modo Blocking + SocketSelector
				break;
			}
		}

	}
}

void Missatges(sf::RenderWindow& windowRef) {
	sf::TcpSocket socket;
	sf::Packet packet;

	//part de missatges
	std::vector<std::string> aMensajes;

	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	sf::String mensaje = " >";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);


	sf::Text text(mensaje, font, 14);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	while (windowRef.isOpen())
	{
		sf::Event evento;
		while (windowRef.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				windowRef.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					windowRef.close();
				else if (evento.key.code == sf::Keyboard::Return)
				{
					aMensajes.push_back(mensaje);
					if (aMensajes.size() > 25)
					{
						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					}

					// Send
					packet << mensaje;

					//socket.send(packet);

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
		// Receive

		windowRef.draw(separator);
		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			windowRef.draw(chattingText);
		}
		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		windowRef.draw(text);


		windowRef.display();
		windowRef.clear();
	}
}