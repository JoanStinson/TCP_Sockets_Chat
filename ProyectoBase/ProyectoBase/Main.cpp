#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

#define MAX_MESSAGES 30
#define DEFAULT_PORT 50000
#define BUFFER_SIZE 2000

sf::TcpSocket theSocket;
sf::TcpListener theListener;
std::mutex mutex;
bool isEnd;
char buffer[2000];

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
					if (messages.size() > 25)
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
					if (messages.size() > 25)
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
					if (messages.size() > 25)
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

//Escucha por un socket determinado y escribe el mensaje llegado.
//socket: Es el socket al cual debemos escuchar.
//recived: La longitud de los datos que recibiremos.
//aMensajes: Es donde contendra todos los mensajes del chat.
void RecivedFunction(sf::TcpSocket* socket, size_t* recived, std::vector<std::string>* aMensajes, sf::RenderWindow* window) {

	while (window->isOpen()) {
		char buffer[BUFFER_SIZE];
		sf::Socket::Status status = socket->receive(buffer, sizeof(buffer), *recived);
		std::string s = buffer;

		if (status == sf::Socket::Status::Done) {
			mutex.lock();
			aMensajes->push_back(s);
			if (aMensajes->size() > 25)
				aMensajes->erase(aMensajes->begin(), aMensajes->begin() + 1);
			mutex.unlock();
		}
	}

}

void DisconnectUser(std::vector<sf::TcpSocket*>& list, sf::SocketSelector *ss, int i) {

	//Lo eliminamos del Socket Selector
	ss->remove(*list[i]);

	//Desconectamos el socket del cliente
	list[i]->disconnect();

	delete(list[i]);

	//Eliminamos el socket de la lista
	list.erase(list.begin() + i, list.begin() + i + 1);


	//Informamos que un usuario se ha desconectado
	for (sf::TcpSocket* socket : list) {
		std::string outMsn = "A User Disconnected";
		socket->send(outMsn.c_str(), outMsn.size() + 1);
	}

	std::cout << "User Disconnected" << std::endl;
}

void ServerTaller2() {
	//ESTABLECER CONNECION
	sf::SocketSelector ss;
	std::vector<sf::TcpSocket*> socketList;
	sf::TcpSocket::Status socketStatus;

	sf::TcpListener listener;
	sf::TcpListener::Status listenerStatus;

	char buffer[BUFFER_SIZE];
	std::string msn;
	size_t recived;

	//Le indicamos a que puerto debe escuchar para el servidor
	listenerStatus = listener.listen(DEFAULT_PORT);

	//Añadimos el listener al Socket Selector
	ss.add(listener);

	while (listenerStatus != sf::TcpListener::Status::Disconnected)
	{
		//Mientras haya elementos en el Socket Selector, esperara a que algun socket le envie algo.
		while (ss.wait())
		{
			//Comprovamos si es un Listener
			if (ss.isReady(listener))
			{

				//Se añade el socket al Socket Selector
				sf::TcpSocket* socket = new sf::TcpSocket();
				//Esperamos peticion de un cliente
				sf::TcpListener::Status st = listener.accept(*socket);
				if (st == sf::TcpListener::Status::Done)
					socketList.push_back(socket);

				ss.add(*socket);

				//Les indicamos que se ha connectado un nuevo usuario
				msn = "New User Connected!";
				for (sf::TcpSocket* s : socketList)
				{
					if (s != socket)s->send(msn.c_str(), msn.size() + 1);
				}

				std::cout << "New User Connected" << std::endl;
			}

			//Si no es el listener, sera un socket
			else
			{

				//Miramos de que socket recivimos el mensaje
				for (int j = 0; j < socketList.size(); j++)
				{
					//Encontramos el socket
					if (ss.isReady(*socketList[j]))
					{
						//Recivimos el mensaje.
						socketStatus = socketList[j]->receive(buffer, sizeof(buffer), recived);

						//Pasar el contenido del buffer, al string de mensaje.
						if (socketStatus == sf::TcpSocket::Status::Done)
						{
							msn = buffer;

							//Enviamos el mensajes
							for (int i = 0; i < socketList.size(); i++)
							{
								sf::TcpSocket::Status st = socketList[i]->send(msn.c_str(), msn.size() + 1);

								if (st == sf::TcpSocket::Status::Error)
									std::cout << "Error al enviar" << std::endl;

							}
						}

						//Si se ha desconnectado
						else if (socketStatus == sf::TcpSocket::Status::Disconnected)
							DisconnectUser(socketList, &ss, j);

						break;
					}
				}
			}
		}
	}

	//Vaciar el Socket Selector
	ss.clear();

	//Paramos de escuchar el puerto, para así dejarlo libre.
	listener.close();

	//Desconnectamos todos los clientes
	for (sf::TcpSocket* &socket : socketList)
		socket->disconnect();

	//Limpiamos el vector
	socketList.clear();
}

void ClientTaller2() {
	//ESTABLECER CONNECION
	sf::TcpSocket socket;
	sf::TcpSocket::Status socketStatus;
	size_t recived;

	// Obtenemos nuestra direccion ip, y nos connectamos con el puerto indicado y nuestra ip
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	socket.connect(ip, DEFAULT_PORT);

	std::vector<std::string> aMensajes;

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

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

	std::string msn;

	//RECIVE
	//Se genera un thread (hilo), que escucha si llegan mensajes o no.
	std::thread t(RecivedFunction, &socket, &recived, &aMensajes, &window);

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
					// SEND
					// Pasamos el mensaje a std::string para hacerlo mas facil en el momento de enviarlo.
					msn = mensaje;
					socketStatus = socket.send(msn.c_str(), msn.size() + 1);
					if (socketStatus == sf::TcpSocket::Status::Disconnected)
					{
						msn = "Server Disconnected";
						aMensajes.push_back(msn);
						if (aMensajes.size() > 25)
						{
							aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
						}
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


	socket.disconnect();
	t.join();
}

// MAIN
int main() {

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
		std::cout << "Has elegido Servidor" << std::endl << std::endl;
		std::cout << "Ahora selecciona el modo con un (numero): " << std::endl;
		std::cout << "1. Blocking + Threading" << std::endl;
		std::cout << "2. NonBlocking" << std::endl;
		std::cout << "3. Blocking + SocketSelector" << std::endl;
		std::cout << "4. Taller2" << std::endl;
		std::cin >> mode;

		status = theListener.listen(5000);

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
				MessagesThreads("Server");
				break;
			case 2:
				std::cout << "Has seleccionado modo NonBlocking" << std::endl;
				MessagesNonBlocking("Server");
				break;
			case 3:
				std::cout << "Has seleccionado modo Blocking + SocketSelector" << std::endl;
				MessagesSelector("Server");
				break;
			case 4:
				std::cout << "Has seleccionado modo Taller2" << std::endl;
				ServerTaller2();
				break;
			}
		}
		theListener.close();
	}

	// Si es el cliente
	else if (connectionType == 'c') {

		// ip localhost
		sf::IpAddress ip = sf::IpAddress::getLocalAddress();
		sf::Socket::Status status = theSocket.connect("localhost", 5000, sf::milliseconds(15.f));

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
				MessagesThreads("Client");
				break;
			case 2:
				std::cout << "Has seleccionado modo NonBlocking" << std::endl;
				MessagesNonBlocking("Client");
				break;
			case 3:
				std::cout << "Has seleccionado modo Blocking + SocketSelector" << std::endl;
				MessagesSelector("Client");
				break;
			case 4:
				std::cout << "Has seleccionado modo Taller2" << std::endl;
				ClientTaller2();
				break;
			}
		}
	}
}