#include <cstdint>
#include <iostream>
#include <functional>
#include <queue>
#include <string>
#include <map>
#include <thread>
//, PositionHash, PositionEqual

//thread for timer
//thread for main_loop

std::queue<std::string> messageQueue;

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
	uintmax_t occupant_id;
	Position position;
	std::thread thread;

	Seat(int x, int y, uintmax_t occupant_id) : position(x,y), occupant_id(occupant_id) {}

	void take_break()
	{
		std::this_thread::sleep_for(std::chrono::seconds(15)); messageQueue.push("Owner of seat at (" + std::to_string(position.get_x()) + "," + std::to_string(position.get_y()) + ") has not yet returned.");
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


void main_loop(std::map<Position, Seat> &Library)
{
	while(true)
	{
		while(!messageQueue.empty()){
			std::cout << messageQueue.front() << std::endl;
			messageQueue.pop();
		};
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
			Library.at(Position(x, y)).thread = std::thread(&Seat::take_break, &Library.at(Position(x, y)));
			
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

int main(){
	std::map<Position, Seat> Library;

	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			Library.emplace(Position(i,j), Seat(i,j,0));
		}
	}

	main_loop(Library);
}
