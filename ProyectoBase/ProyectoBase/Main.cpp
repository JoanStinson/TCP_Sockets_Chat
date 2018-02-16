#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#define MAX_MENSAJES 30

using namespace std;
std::mutex mut;
sf::TcpSocket socket;
bool isEnd;
char buffer[2000];

void ReciveF(size_t recived, vector<string>* aMensajes, sf::RenderWindow* window)
{
	while (!isEnd) {
		mut.lock();
		sf::Socket::Status status = socket.receive(buffer, 2000, recived);
		if (status == sf::Socket::Disconnected) {
			isEnd = true;
		}
		if (status == sf::Socket::Done) {
			aMensajes->push_back(buffer);
		}
		mut.unlock();
	}
}

void MensajesThreads(string type) {
	size_t recived;
	std::vector<std::string> aMensajes;

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), type);

	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

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

	//RECIVE
	thread thread(&ReciveF, recived, &aMensajes, &window);

	
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
					aMensajes.push_back(mensaje);
					if (aMensajes.size() > 25)
					{
						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					}
					// SEND
					mensajeStr = type + ": ";
					mensajeStr += mensaje.substring(2, mensaje.getSize());
					
					socket.send(mensajeStr.c_str(), mensajeStr.size() + 1);

					if (mensaje == ">exit" || mensaje == " >exit") {
						isEnd = true;
						socket.disconnect();
						exit(0);
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
		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
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
	
	thread.join();

	socket.disconnect();
}

void MensajesNonBlocking(string type) {
	size_t recived, sended;
	std::vector<std::string> aMensajes;
	sf::Socket::Status sStatus;

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), type);

	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

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
					aMensajes.push_back(mensaje);
					if (aMensajes.size() > 25)
					{
						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					}
					// SEND
					mensajeStr = type + ": ";
					mensajeStr += mensaje.substring(2, mensaje.getSize());
					
					do
					{
						sStatus = socket.send(mensajeStr.c_str(), mensajeStr.size() + 1, sended);
						if (sStatus == sf::Socket::Status::Partial)
						{
							mensajeStr = mensajeStr.substr(sended + 1, mensajeStr.size() - sended);
							if (mensaje == ">exit" || mensaje == " >exit") {
								isEnd = true;
								socket.disconnect();
								exit(0);
							}
						}
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


		window.draw(separator);
		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
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

	socket.disconnect();
}

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
			//no se puede vincular al puerto 5000
			std::cout << "No se puede vincular al puerto 5000" << std::endl;
			exit(0);
		}

		if (listener.accept(socket) != sf::Socket::Done) {
			//error al acceptar la conexion
			std::cout << "Error al acceptar la conexion" << std::endl;
		}
		else {
			packet << mode;
			socket.send(packet);
			std::cout << "Enviado packet del mode";

			switch (mode) {
			case 1:
				//Modo Blocking + Threading
				std::cout << "Modo Threads\n";
				MensajesThreads("Server");

				break;
			case 2:
				//Modo NonBlocking
				std::cout << "Modo NonBlocking\n";
				MensajesNonBlocking("Server");
				break;
			case 3:
				//Modo Blocking + SocketSelector
				break;
			}
		}
		listener.close();
	}

	//Si es el cliente
	else if (connectionType == 'c')
	{
		//ip localhost
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
				//Modo Blocking + Threading
				std::cout << "Modo Threads\n";
				MensajesThreads("Client");

				break;
			case 2:
				//Modo NonBlocking
				std::cout << "Modo NonBlocking\n";
				MensajesNonBlocking("Client");
				break;
			case 3:
				//Modo Blocking + SocketSelector
				break;
			}
		}
	}

	

}