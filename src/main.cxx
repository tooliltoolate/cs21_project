import Seat;
#include <iostream>

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
		std::cout << "3. Exit" << std::endl;
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
		else if(choice == 3){
			break;
		}
	}
}
