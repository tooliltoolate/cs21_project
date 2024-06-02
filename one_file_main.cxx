#include <iostream>
#include <queue>
#include <string>
#include <map>
#include <thread>
#include <random>
#include <mutex>
#include <fstream>


class Seat;

struct Message{
	std::string message;
	Seat *sender;

	Message(std::string message, Seat *address) : message(message), sender(address) {}
};

std::queue<Message> messageQueue;
std::mutex mtx;
std::ofstream logFile("library_log.txt", std::ios::app);

class Rand_int {
	public:
	Rand_int(int low, int high) : dist{low,high} { }
	int operator()() { return dist(re); }
	void seed(int s) { re.seed(s); }
	private:
		std::default_random_engine re;
		std::uniform_int_distribution<> dist;
};

class Position{
public:
	int x;
	int y;
	Position(int x, int y) : x(x), y(y) {}
	int get_x() const { return x; }
	int get_y() const { return y; }
	bool operator<(const Position& other) const {
		if (x < other.x) return true;
		if (x > other.x) return false;
		return y < other.y;
	}
};

class Seat {
public:
	std::string occupant_id;
	std::string password;
	Position position;
	std::jthread thread;

	Seat(int x, int y, std::string occupant_id) : position(x,y), occupant_id(occupant_id) { 
		Rand_int rand_int {1000, 9999};
		rand_int.seed(std::chrono::system_clock::now().time_since_epoch().count());
		password = std::to_string(rand_int());
	}

	void print_password() {
		std::cout << "Password for seat at (" << position.get_x() << "," << position.get_y() << ") is " << password << std::endl;
	}

	bool is_occupied() const {
		if(occupant_id == "0"){
			return false;
		}
		else{
			return true;
		}
	}
	std::string get_occupant_id() const { return occupant_id; }
	std::string & register_seat() { return occupant_id; }
	const std::string & register_seat() const { return occupant_id; }
	Position get_position() const { return position; }
};

void log_action(const std::string &action) {
	std::scoped_lock lock{mtx};
	logFile << action << std::endl;
}

void main_loop(std::map<Position, Seat> &Library)
{
	while(true)
	{
		while(!messageQueue.empty()){
			std::scoped_lock lock{mtx};
			std::cout << messageQueue.front().message << std::endl;
			messageQueue.front().sender->thread.join();
			messageQueue.pop();
		};
		std::cout << "What do you want to do?" << std::endl;
		std::cout << "1. Show all available seats" << std::endl;
		std::cout << "2. Register a seat" << std::endl;
		std::cout << "3. Leave a seat" << std::endl;
		std::cout << "4. Take a break" << std::endl;
		std::cout << "5. Return from break" << std::endl;
		std::cout << "6. Exit" << std::endl;
		std::string choice;
		std::cin >> choice;
		if(choice == "1"){
			for(auto it = Library.begin(); it != Library.end(); it++){
					if(!it->second.is_occupied()){
                    std::cout << "Seat at (" << it->second.position.get_x() << "," << it->second.position.get_y() << ") is not occupied" << std::endl;
                    log_action("Checked availability of seat at (" + std::to_string(it->second.position.get_x()) + "," + std::to_string(it->second.position.get_y()) + ")");
                }
            }
		}
		else if(choice == "2"){
			std::cout << "Enter the x and y coordinates of the seat you want to register" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the occupant id" << std::endl;
			std::string occupant_id;
			std::cin >> occupant_id;
			Library.at(Position(x, y)).register_seat() = occupant_id;
			Library.at(Position(x, y)).print_password();
			log_action("Registered seat at (" + std::to_string(x) + "," + std::to_string(y) + ") with occupant id " + occupant_id);
		}
		else if(choice == "3") {
			std::cout << "Enter the x and y coordinates of your seat" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the password" << std::endl;
			std::string password;
			std::cin >>password;
			if(Library.at(Position(x, y)).password ==password) {
				Library.at(Position(x, y)).register_seat() = "0";
				std::cout << "Seat at (" << x << "," << y << ") has been left." << std::endl;
				log_action("Seat at (" + std::to_string(x) + "," + std::to_string(y) + ") has been left.");
			} else {
				std::cout << "Incorrect password. Unable to leave the seat." << std::endl;
				log_action("Failed attempt to leave seat at (" + std::to_string(x) + "," + std::to_string(y) + ") due to incorrect password.");
			}
		}
		else if(choice == "4"){
			std::cout << "Enter the x and y coordinates of your seat" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the password " << std::endl;
			std::string password;
			std::cin >> password;
			if(Library.at(Position(x, y)).password ==password) {
				Library.at(Position(x, y)).thread = std::jthread([&x, &y, &Library](std::stop_token stoken) {
					std::scoped_lock lock{mtx};
					Message msg("Owner of seat at (" + std::to_string(x) + "," + std::to_string(y) + ") has not yet returned.", &Library.at(Position(x, y)));
					Message msg2("Owner of seat at (" + std::to_string(x) + "," + std::to_string(y) + ") has returned.", &Library.at(Position(x, y)));
					for(int i = 0; i < 15; i++){
						std::this_thread::sleep_for(std::chrono::seconds(1));
						if(stoken.stop_requested()) {
							messageQueue.push(msg2);
							return;
						}
					}
					messageQueue.push(msg);
				});
				std::cout << "User of seat at (" << x << "," << y << ") has taken a break." << std::endl;
				log_action("User of seat at (" + std::to_string(x) + "," + std::to_string(y) + ") has taken a break.");
			} else {
				std::cout << "Incorrect password." << std::endl;
				log_action("Failed attempt to take a break at seat (" + std::to_string(x) + "," + std::to_string(y) + ") due to incorrect password.");
			}
			
		}
		else if(choice == "5"){
			std::cout << "Enter the x and y coordinates of your seat" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the password " << std::endl;
			std::string password;
			std::cin >> password;
			if(Library.at(Position(x, y)).password ==password) {
				Library.at(Position(x, y)).thread.request_stop();
				log_action("User of seat at (" + std::to_string(x) + "," + std::to_string(y) + ") has returned from break.");
			} else {
				std::cout << "Incorrect password." << std::endl;
				log_action("Failed attempt to return from break at seat (" + std::to_string(x) + "," + std::to_string(y) + ") due to incorrect password.");
			}
			
		}
		else if(choice == "admin password"){
			std::cout << "Enter the x and y coordinates of the seat you want to register" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the occupant id" << std::endl;
			std::string occupant_id;
			std::cin >> occupant_id;
			Library.at(Position(x, y)).register_seat() = occupant_id;
			log_action("Admin registered seat at (" + std::to_string(x) + "," + std::to_string(y) + ") with occupant id " + occupant_id);
        }
		}
		else if(choice == "6") break;
	}
}

int main(){
	std::map<Position, Seat> Library;

	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			Library.emplace(Position(i,j), Seat(i,j,"0"));
		}
	}

	main_loop(Library);
	logFile.close();
}
