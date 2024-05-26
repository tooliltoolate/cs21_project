#include <cstdint>
#include <iostream>

class Position{
public:
	int x;
	int y;
	Position() {};
	Position(int x, int y) : x(x), y(y) {}
	int get_x() const { return x; }
	int get_y() const { return y; }
};

class Seat {
public:
	uintmax_t occupant_id;
	Position position;
	Seat() {};
	Seat(int x, int y, uintmax_t occupant_id) : position(x,y), occupant_id(occupant_id) {}
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

int main(){
	Seat Library[5][5];
	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			Library[i][j] = Seat(i,j,0);
		}
	}
	while(true){
		std::cout << "What do you want to do?" << std::endl;
		std::cout << "1. Show all available seats" << std::endl;
		std::cout << "2. Register a seat" << std::endl;
		std::cout << "3. Leave a seat" << std::endl;
		std::cout << "4. Exit" << std::endl;
		int choice;
		std::cin >> choice;
		if(choice == 1){
			for(int i = 0; i < 5; i++){
				for(int j = 0; j < 5; j++){
					if(Library[i][j].is_occupied()){
					}
					else{
						std::cout << "Seat at (" << i << "," << j << ") is not occupied" << std::endl;
					}
				}
			}
		}
		else if(choice == 2){
			std::cout << "Enter the x and y coordinates of the seat you want to register" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the occupant id" << std::endl;
			uint16_t occupant_id;
			std::cin >> occupant_id;
			Library[x][y].register_seat() = occupant_id;
		}
		else if(choice == 3) {
			std::cout << "Enter the x and y coordinates of the seat you want to register" << std::endl;
			int x, y;
			std::cin >> x >> y;
			std::cout << "Enter the occupant id" << std::endl;
			uint16_t occupant_id;
			std::cin >> occupant_id;
			if(Library[x][y].get_occupant_id() == occupant_id) {
				Library[x][y].register_seat() = 0;
				std::cout << "Seat at (" << x << "," << y << ") has been left." << std::endl;
			} else {
				std::cout << "Incorrect occupant id. Unable to leave the seat." << std::endl;
			}
		}else if(choice == 4){
			break;
		}
	}
}
