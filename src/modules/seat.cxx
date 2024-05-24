module;
#include <cstdint>

export module Seat;

class Position{
public:
	int x;
	int y;
	Position() {};
	Position(int x, int y) : x(x), y(y) {}
	int get_x() const { return x; }
	int get_y() const { return y; }
};

export class Seat {
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
