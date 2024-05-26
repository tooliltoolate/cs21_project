//have a global queue
//give every seat a timer argument
//after the async wait, push something to the queue
//in the for loop, it goes through the queue first
#include <cstdint>
#include <iostream>
#include <functional>
#include <asio.hpp>
#include <queue>
#include <string>
#include <unordered_map>

class Position{
public:
	int x;
	int y;
	Position() : x(0), y(0) {};
	Position(int x, int y) : x(x), y(y) {}
	int get_x() const { return x; }
	int get_y() const { return y; }
};

class Seat {
public:
	uintmax_t occupant_id;
	Position position;
	asio::steady_timer timer;
	std::queue<std::string> queue;

	Seat(int x, int y, uintmax_t occupant_id, std::queue<std::string> queue, asio::io_context& io) : position(x,y), occupant_id(occupant_id), timer(io, asio::chrono::minutes(15)), queue(queue) {}

	void take_break()
	{
		timer.async_wait([this](const asio::error_code& ec) {
			if (!ec) {
				queue.push("Owner of seat at (" + std::to_string(position.get_x()) + "," + std::to_string(position.get_y()) + ") has not yet returned.");
			}
		});
	}

	bool is_occupied() const {
		if(occupant_id == 0){
			return false;
		}
		else{
			return true;
		}
	}
	uintmax_t get_occupant_id() const { return occupant_id; }
	uintmax_t & register_seat() { return occupant_id; }
	const uintmax_t & register_seat() const { return occupant_id; }
	Position get_position() const { return position; }
};

struct PositionHash {
    std::size_t operator()(const Position& foo) const {
        return std::hash<int>()(foo.x) ^ std::hash<int>()(foo.y);
    }
};

// Define the equality function for Position
struct PositionEqual {
    bool operator()(const Position& lhs, const Position& rhs) const {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
};

int main(){
	std::queue<std::string> messageQueue;
	asio::io_context io;
	std::unordered_map<Position, Seat, PositionHash, PositionEqual> Library;
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			Library.emplace(Position(i,j), Seat(i,j,0,messageQueue,io));
		}
	}
	while(true){
		std::cout << "What do you want to do?" << std::endl;
		std::cout << "1. Show all available seats" << std::endl;
		std::cout << "2. Register a seat" << std::endl;
		std::cout << "3. Leave a seat" << std::endl;
		std::cout << "4. Take a break" << std::endl;
		std::cout << "5. Exit" << std::endl;
		std::string choice;
		std::cin >> choice;
		if(choice == "1"){
			for(auto it = Library.begin(); it != Library.end(); it++){
					if(it->second.is_occupied()){
					}
					else{
						std::cout << "Seat at (" << std::to_string(it->second.position.get_x()) << "," << std::to_string(it->second.position.get_y()) << ") is not occupied" << std::endl;
					}
			}
		}
		else if(choice == "2"){
			std::cout << "Enter the x and y coordinates of the seat you want to register" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the occupant id" << std::endl;
			uint16_t occupant_id;
			std::cin >> occupant_id;
			Library.at(Position(x, y)).register_seat() = occupant_id;
		}
		else if(choice == "3") {
			std::cout << "Enter the x and y coordinates of your seat" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the occupant id" << std::endl;
			uint16_t occupant_id;
			std::cin >> occupant_id;
			if(Library.at(Position(x, y)).get_occupant_id() == occupant_id) {
				Library.at(Position(x, y)).register_seat() = 0;
				std::cout << "Seat at (" << x << "," << y << ") has been left." << std::endl;
			} else {
				std::cout << "Incorrect occupant id. Unable to leave the seat." << std::endl;
			}
		}
		else if(choice == "4"){
			std::cout << "Enter the x and y coordinates of your seat" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the occupant id" << std::endl;
			uint16_t occupant_id;
			std::cin >> occupant_id;
			Library.at(Position(x, y)).take_break();
			
		}
		else if(choice == "admin password"){
			std::cout << "Enter the x and y coordinates of the seat you want to register" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the occupant id" << std::endl;
			uint16_t occupant_id;
			std::cin >> occupant_id;
			Library.at(Position(x, y)).register_seat() = occupant_id;
		}
		else if(choice == "5") break;
	}
}
