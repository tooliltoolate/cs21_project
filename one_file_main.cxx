#include <chrono>
#include <exception>
#include <iostream>
#include <queue>
#include <string>
#include <map>
#include <thread>
#include <random>
#include <mutex>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>

class Seat;

struct Message{
	std::string message;
	Seat *sender;

	Message(std::string message, Seat *address) : message(message), sender(address) {}
};

struct LogEntry {
    std::string action;
    std::string type; 
};

std::queue<Message> messageQueue;
std::queue<Seat*> leavingQueue;
std::mutex alarm_queue_mutex;
std::ofstream logFile("library_log.txt", std::ios::app);
std::map<std::string, std::vector<LogEntry>> logData;
std::mutex log_file_mutex;
std::mutex seat_timer_mutex;

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
	std::jthread seated_for_timer;

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

void log_action(const std::string& action, const std::string& type) {
    logFile << action << std::endl;
    logData[type].push_back({action, type}); 
}

void saveLogData(const std::map<std::string, std::vector<LogEntry>>& logData, const std::string& fileName) {
    std::ofstream outputFile(fileName);

    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not create output file." << std::endl;
        return;
    }

    outputFile << std::setw(20) << "Action" << "|" << std::setw(20) << "Type" << std::endl;
    outputFile << "--------------------------------------" << std::endl;

    for (const auto& [type, entries] : logData) {
        for (const auto& entry : entries) {
            outputFile << std::setw(20) << entry.action << "|" << std::setw(20) << entry.type << std::endl;
        }
    }

    outputFile.close();

    std::cout << "Log data saved to " << fileName << std::endl;
}

void formatLogFile(const std::string& logFileName, const std::string& outputFileName) {
    std::ifstream logFile(logFileName);

    if (!logFile.is_open()) {
        std::cerr << "Error: Could not open log file." << std::endl;
        return;
    }

    std::map<std::string, std::vector<LogEntry>> logData;

    std::string line;
    while (std::getline(logFile, line)) {
        std::istringstream iss(line);
        std::string action, type;
        if (iss >> action >> type) {
            logData[type].push_back({action, type});
        }
    }

    logFile.close();

    saveLogData(logData, outputFileName);
}

void main_loop(std::map<Position, Seat> &Library);

void displayAdminMenu() {
    std::cout << "Admin Menu\n";
    std::cout << "1. View logs\n";
    std::cout << "2. Manage users\n";
    std::cout << "3. System settings\n";
    std::cout << "4. Exit\n";
}

void displayManageUsersMenu() {
    std::cout << "Manage Users Menu\n";
    std::cout << "1. Show occupied seats\n";
    std::cout << "2. Show users on break\n";
    std::cout << "3. Kick a user\n";
    std::cout << "4. Back to main menu\n";
}

void handleManageUsersMenu(std::map<Position, Seat> &Library) {
    int choice;
    do {
        displayManageUsersMenu();
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        switch (choice) {
            case 1: {
                std::cout << "Occupied seats:\n";
                for (const auto& [pos, seat] : Library) {
                    if (seat.is_occupied()) {
                        std::cout << "Seat at (" << pos.get_x() << "," << pos.get_y() << ") is occupied by user " << seat.get_occupant_id() << std::endl;
                    }
                }
                break;
            }
            case 2: {
                std::cout << "Enter the x and y coordinates of the seat to kick the user: ";
                int x, y;
                std::cin >> x >> y;
                try {
                    auto& seat = Library.at(Position(x, y));
                    if (seat.is_occupied()) {
                        seat.register_seat() = "0";
                        std::cout << "User kicked from seat at (" << x << "," << y << ")." << std::endl;
                        log_action("User kicked from seat at (" + std::to_string(x) + "," + std::to_string(y) + ")", "kick");
                    } else {
                        std::cout << "Seat at (" << x << "," << y << ") is already empty." << std::endl;
                    }
                } catch (const std::out_of_range&) {
                    std::cout << "Invalid seat coordinates." << std::endl;
                }
                break;
            }
            case 3: {
                std::cout << "Users on break:\n";
                for (const auto& [pos, seat] : Library) {
                    if (seat.thread.joinable() && seat.is_occupied()) {
                        std::cout << "User at seat (" << pos.get_x() << "," << pos.get_y() << ") is on break.\n";
                    }
                }
                break;
            }
            case 4:
                return;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 4);
}

void handleAdminMenu(std::map<Position, Seat> &Library) {
    int choice;
    do {
        displayAdminMenu();
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        switch (choice) {
            case 1: {
                std::cout << "Logs:\n";
                for (const auto& [type, entries] : logData) {
                    for (const auto& entry : entries) {
                        std::cout << entry.action << " | " << entry.type << std::endl;
                    }
                }
                break;
            }
            case 2: {
                handleManageUsersMenu(Library);
                break;
	}
            case 3:
                std::cout << "Exiting admin menu...\n";
				main_loop(Library);
                return;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 3);
}
void main_loop(std::map<Position, Seat> &Library)
{
	while(true)
	{
		while(!messageQueue.empty()){
			std::scoped_lock lock{alarm_queue_mutex};
			std::cout << messageQueue.front().message << std::endl;
			messageQueue.front().sender->thread.join();
			messageQueue.pop();
		};
		while(!leavingQueue.empty()){
			std::scoped_lock lock{seat_timer_mutex};
			leavingQueue.front()->thread.join();
			leavingQueue.pop();
		};
		std::cout << "What do you want to do?" << std::endl;
		std::cout << "1. Show all available seats" << std::endl;
		std::cout << "2. Register a seat" << std::endl;
		std::cout << "3. Leave a seat" << std::endl;
		std::cout << "4. Take a break" << std::endl;
		std::cout << "5. Return from break" << std::endl;
		std::cout << "6. Exit" << std::endl;
		std::string choice;
		getline(std::cin, choice);
		if(choice == "1"){
			for(auto it = Library.begin(); it != Library.end(); it++){
				if(!it->second.is_occupied()){
				    std::cout << "Seat at (" << it->second.position.get_x() << "," << it->second.position.get_y() << ") is not occupied" << std::endl;
				}
			}
		        log_action("Checked availability of seats");
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
			Library.at(Position(x, y)).seated_for_timer = std::jthread([&x, &y, &Library](std::stop_token stoken) {
				std::scoped_lock lock{seat_timer_mutex};
				auto time_start = std::chrono::steady_clock::now();
				while(!stoken.stop_requested()){
				}
				auto time_end = std::chrono::steady_clock::now();
				std::string msg("Owner of seat at (" + std::to_string(x) + "," + std::to_string(y) + ") has been seated for " + std::to_string(std::chrono::floor<std::chrono::minutes>(time_end - time_start).count()) + " minutes.");
				leavingQueue.push(&Library.at(Position(x, y)));
				log_action(msg);
			});
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
				Library.at(Position(x, y)).seated_for_timer.request_stop();
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
					std::scoped_lock lock{alarm_queue_mutex};
					Message msg("Owner of seat at (" + std::to_string(x) + "," + std::to_string(y) + ") has not yet returned.", &Library.at(Position(x, y)));
					for(int i = 0; i < 10; i++){
						std::this_thread::sleep_for(std::chrono::seconds(1));
						if(stoken.stop_requested()) {
							return;
						}
					}
					messageQueue.push(msg);
					log_action(msg);
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
				std::cout << "User of seat at (" << x << "," << y << ") has returned from break." << std::endl;
				log_action("User of seat at (" + std::to_string(x) + "," + std::to_string(y) + ") has returned from break.");
			} else {
				std::cout << "Incorrect password." << std::endl;
				log_action("Failed attempt to return from break at seat (" + std::to_string(x) + "," + std::to_string(y) + ") due to incorrect password.");
			}
			
		}
		else if(choice == "admin password"){
			handleAdminMenu(Library);
		}
		else if(choice == "6") std::terminate();
	}
}

int main(){
	std::map<Position, Seat> Library;
    std::string logFileName = "library_log.txt";
    std::string formattedLogFileName = "formatted_library_log.txt";

    formatLogFile(logFileName, formattedLogFileName);

	for(int i = 0; i < 5; i++){
		for(int j = 0; j < 5; j++){
			Library.emplace(Position(i,j), Seat(i,j,"0"));
		}
	}

	main_loop(Library);
	logFile.close();
	return 0;
}
