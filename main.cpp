#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>

using namespace std;

// Data structures
struct User {
    string username;
    string password;
    string name;
    string mailAddr;
    int privilege;
};

struct Train {
    string trainID;
    int stationNum;
    vector<string> stations;
    int seatNum;
    vector<int> prices;
    string startTime;
    vector<int> travelTimes;
    vector<int> stopoverTimes;
    string saleDateStart;
    string saleDateEnd;
    char type;
    bool released;

    // Seat availability for each date and station segment
    map<string, vector<int>> seatAvailability;
};

struct Order {
    string username;
    string trainID;
    string date;
    string fromStation;
    string toStation;
    int num;
    int price;
    string status; // success, pending, refunded
    time_t timestamp;
};

// Global data
map<string, User> users;
set<string> loggedInUsers;
map<string, Train> trains;
vector<Order> orders;

// Helper functions
vector<string> split(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream iss(str);
    while (getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

string formatTime(int month, int day, int hour, int min) {
    char buffer[20];
    sprintf(buffer, "%02d-%02d %02d:%02d", month, day, hour, min);
    return string(buffer);
}

// Command implementations
int add_user(const string& cur_username, const string& username, const string& password,
             const string& name, const string& mailAddr, int privilege) {
    // Check if creating first user
    if (users.empty()) {
        User newUser;
        newUser.username = username;
        newUser.password = password;
        newUser.name = name;
        newUser.mailAddr = mailAddr;
        newUser.privilege = 10;
        users[username] = newUser;
        return 0;
    }

    // Check if cur_user is logged in
    if (loggedInUsers.find(cur_username) == loggedInUsers.end()) {
        return -1;
    }

    // Check if user already exists
    if (users.find(username) != users.end()) {
        return -1;
    }

    // Check privilege
    User& curUser = users[cur_username];
    if (curUser.privilege <= privilege) {
        return -1;
    }

    User newUser;
    newUser.username = username;
    newUser.password = password;
    newUser.name = name;
    newUser.mailAddr = mailAddr;
    newUser.privilege = privilege;
    users[username] = newUser;

    return 0;
}

int login(const string& username, const string& password) {
    if (users.find(username) == users.end()) {
        return -1;
    }

    if (loggedInUsers.find(username) != loggedInUsers.end()) {
        return -1;
    }

    if (users[username].password != password) {
        return -1;
    }

    loggedInUsers.insert(username);
    return 0;
}

int logout(const string& username) {
    if (loggedInUsers.find(username) == loggedInUsers.end()) {
        return -1;
    }

    loggedInUsers.erase(username);
    return 0;
}

string query_profile(const string& cur_username, const string& username) {
    if (loggedInUsers.find(cur_username) == loggedInUsers.end()) {
        return "-1";
    }

    if (users.find(username) == users.end()) {
        return "-1";
    }

    User& curUser = users[cur_username];
    User& targetUser = users[username];

    if (curUser.privilege <= targetUser.privilege && cur_username != username) {
        return "-1";
    }

    return targetUser.username + " " + targetUser.name + " " + targetUser.mailAddr + " " + to_string(targetUser.privilege);
}

string modify_profile(const string& cur_username, const string& username,
                     const string& password, const string& name,
                     const string& mailAddr, int privilege) {
    if (loggedInUsers.find(cur_username) == loggedInUsers.end()) {
        return "-1";
    }

    if (users.find(username) == users.end()) {
        return "-1";
    }

    User& curUser = users[cur_username];
    User& targetUser = users[username];

    if (curUser.privilege <= targetUser.privilege && cur_username != username) {
        return "-1";
    }

    if (privilege != -1) {
        if (curUser.privilege <= privilege) {
            return "-1";
        }
        targetUser.privilege = privilege;
    }

    if (!password.empty()) {
        targetUser.password = password;
    }
    if (!name.empty()) {
        targetUser.name = name;
    }
    if (!mailAddr.empty()) {
        targetUser.mailAddr = mailAddr;
    }

    return targetUser.username + " " + targetUser.name + " " + targetUser.mailAddr + " " + to_string(targetUser.privilege);
}

int add_train(const string& trainID, int stationNum, int seatNum,
              const vector<string>& stations, const vector<int>& prices,
              const string& startTime, const vector<int>& travelTimes,
              const vector<int>& stopoverTimes, const string& saleDateStart,
              const string& saleDateEnd, char type) {

    if (trains.find(trainID) != trains.end()) {
        return -1;
    }

    Train newTrain;
    newTrain.trainID = trainID;
    newTrain.stationNum = stationNum;
    newTrain.stations = stations;
    newTrain.seatNum = seatNum;
    newTrain.prices = prices;
    newTrain.startTime = startTime;
    newTrain.travelTimes = travelTimes;
    newTrain.stopoverTimes = stopoverTimes;
    newTrain.saleDateStart = saleDateStart;
    newTrain.saleDateEnd = saleDateEnd;
    newTrain.type = type;
    newTrain.released = false;

    trains[trainID] = newTrain;
    return 0;
}

int release_train(const string& trainID) {
    if (trains.find(trainID) == trains.end()) {
        return -1;
    }

    if (trains[trainID].released) {
        return -1;
    }

    trains[trainID].released = true;
    return 0;
}

string query_train(const string& trainID, const string& date) {
    if (trains.find(trainID) == trains.end()) {
        return "-1";
    }

    Train& train = trains[trainID];

    // Parse date
    int month, day;
    sscanf(date.c_str(), "%d-%d", &month, &day);

    // Parse sale dates
    int saleMonthStart, saleDayStart, saleMonthEnd, saleDayEnd;
    sscanf(train.saleDateStart.c_str(), "%d-%d", &saleMonthStart, &saleDayStart);
    sscanf(train.saleDateEnd.c_str(), "%d-%d", &saleMonthEnd, &saleDayEnd);

    // Check if date is in sale range
    int dateValue = month * 100 + day;
    int saleStartValue = saleMonthStart * 100 + saleDayStart;
    int saleEndValue = saleMonthEnd * 100 + saleDayEnd;

    if (dateValue < saleStartValue || dateValue > saleEndValue) {
        return "-1";
    }

    // Parse start time
    int startHour, startMin;
    sscanf(train.startTime.c_str(), "%d:%d", &startHour, &startMin);

    // Calculate times for each station
    vector<string> arrivalTimes;
    vector<string> departureTimes;
    vector<int> cumulativePrices(train.stationNum, 0);

    // Starting station
    arrivalTimes.push_back("xx-xx xx:xx");
    departureTimes.push_back(formatTime(month, day, startHour, startMin));

    int currentMonth = month;
    int currentDay = day;
    int currentHour = startHour;
    int currentMin = startMin;

    // Calculate for each station
    for (int i = 1; i < train.stationNum; i++) {
        // Add travel time
        currentMin += train.travelTimes[i-1];
        currentHour += currentMin / 60;
        currentMin %= 60;
        currentDay += currentHour / 24;
        currentHour %= 24;

        // Handle month changes (simplified for June-August 2021)
        if (currentMonth == 6 && currentDay > 30) {
            currentMonth = 7;
            currentDay -= 30;
        } else if (currentMonth == 7 && currentDay > 31) {
            currentMonth = 8;
            currentDay -= 31;
        } else if (currentMonth == 8 && currentDay > 31) {
            // Shouldn't happen based on constraints
        }

        arrivalTimes.push_back(formatTime(currentMonth, currentDay, currentHour, currentMin));

        // Add stopover time (except for last station)
        if (i < train.stationNum - 1) {
            currentMin += train.stopoverTimes[i-1];
            currentHour += currentMin / 60;
            currentMin %= 60;
            currentDay += currentHour / 24;
            currentHour %= 24;

            // Handle month changes again
            if (currentMonth == 6 && currentDay > 30) {
                currentMonth = 7;
                currentDay -= 30;
            } else if (currentMonth == 7 && currentDay > 31) {
                currentMonth = 8;
                currentDay -= 31;
            }

            departureTimes.push_back(formatTime(currentMonth, currentDay, currentHour, currentMin));
        } else {
            departureTimes.push_back("xx-xx xx:xx");
        }

        // Calculate cumulative price
        if (i > 0) {
            cumulativePrices[i] = cumulativePrices[i-1] + train.prices[i-1];
        }
    }

    // Format output
    string result = trainID + " " + train.type + "\n";
    for (int i = 0; i < train.stationNum; i++) {
        result += train.stations[i] + " " + arrivalTimes[i] + " -> " + departureTimes[i] + " " + to_string(cumulativePrices[i]) + " ";
        if (i == train.stationNum - 1) {
            result += "x";
        } else {
            result += to_string(train.seatNum);
        }
        if (i < train.stationNum - 1) {
            result += "\n";
        }
    }

    return result;
}

int delete_train(const string& trainID) {
    if (trains.find(trainID) == trains.end()) {
        return -1;
    }

    if (trains[trainID].released) {
        return -1;
    }

    trains.erase(trainID);
    return 0;
}

string query_ticket(const string& fromStation, const string& toStation, const string& date, const string& sortBy) {
    vector<string> results;

    for (auto& trainPair : trains) {
        Train& train = trainPair.second;
        if (!train.released) continue;

        // Parse sale dates
        int saleMonthStart, saleDayStart, saleMonthEnd, saleDayEnd;
        sscanf(train.saleDateStart.c_str(), "%d-%d", &saleMonthStart, &saleDayStart);
        sscanf(train.saleDateEnd.c_str(), "%d-%d", &saleMonthEnd, &saleDayEnd);

        // Check if date is in sale range
        int queryMonth, queryDay;
        sscanf(date.c_str(), "%d-%d", &queryMonth, &queryDay);
        int dateValue = queryMonth * 100 + queryDay;
        int saleStartValue = saleMonthStart * 100 + saleDayStart;
        int saleEndValue = saleMonthEnd * 100 + saleDayEnd;

        if (dateValue < saleStartValue || dateValue > saleEndValue) {
            continue;
        }

        // Find stations
        int fromIndex = -1, toIndex = -1;
        for (int i = 0; i < train.stationNum; i++) {
            if (train.stations[i] == fromStation) {
                fromIndex = i;
            }
            if (train.stations[i] == toStation) {
                toIndex = i;
            }
        }

        if (fromIndex == -1 || toIndex == -1 || fromIndex >= toIndex) {
            continue;
        }

        // Calculate price and time
        int totalPrice = 0;
        for (int i = fromIndex; i < toIndex; i++) {
            totalPrice += train.prices[i];
        }

        // Parse start time
        int startHour, startMin;
        sscanf(train.startTime.c_str(), "%d:%d", &startHour, &startMin);

        // Calculate departure time from fromStation
        int currentMonth = queryMonth;
        int currentDay = queryDay;
        int currentHour = startHour;
        int currentMin = startMin;

        // Add days to reach departure date from starting station
        int departureMonth = queryMonth;
        int departureDay = queryDay;

        // Calculate arrival time at fromStation
        for (int i = 0; i < fromIndex; i++) {
            // Add travel time
            currentMin += train.travelTimes[i];
            currentHour += currentMin / 60;
            currentMin %= 60;
            currentDay += currentHour / 24;
            currentHour %= 24;

            // Handle month changes
            if (currentMonth == 6 && currentDay > 30) {
                currentMonth = 7;
                currentDay -= 30;
            } else if (currentMonth == 7 && currentDay > 31) {
                currentMonth = 8;
                currentDay -= 31;
            }

            // Add stopover time (except for last station)
            if (i < train.stationNum - 1 && fromIndex > 0) {
                currentMin += train.stopoverTimes[i];
                currentHour += currentMin / 60;
                currentMin %= 60;
                currentDay += currentHour / 24;
                currentHour %= 24;

                // Handle month changes again
                if (currentMonth == 6 && currentDay > 30) {
                    currentMonth = 7;
                    currentDay -= 30;
                } else if (currentMonth == 7 && currentDay > 31) {
                    currentMonth = 8;
                    currentDay -= 31;
                }
            }
        }

        string departureTime = formatTime(currentMonth, currentDay, currentHour, currentMin);

        // Calculate arrival time at toStation
        for (int i = fromIndex; i < toIndex; i++) {
            currentMin += train.travelTimes[i];
            currentHour += currentMin / 60;
            currentMin %= 60;
            currentDay += currentHour / 24;
            currentHour %= 24;

            // Handle month changes
            if (currentMonth == 6 && currentDay > 30) {
                currentMonth = 7;
                currentDay -= 30;
            } else if (currentMonth == 7 && currentDay > 31) {
                currentMonth = 8;
                currentDay -= 31;
            }
        }

        string arrivalTime = formatTime(currentMonth, currentDay, currentHour, currentMin);

        // Format result
        string result = train.trainID + " " + fromStation + " " + departureTime + " -> " + toStation + " " + arrivalTime + " " + to_string(totalPrice) + " " + to_string(train.seatNum);
        results.push_back(result);
    }

    // Sort results
    if (sortBy == "time") {
        // Sort by time (lexicographic order of time strings should work)
        sort(results.begin(), results.end());
    } else if (sortBy == "cost") {
        // Sort by price - extract price and sort
        vector<pair<int, string>> pricedResults;
        for (const string& result : results) {
            istringstream iss(result);
            string trainID, from, depTime, arrow, to, arrTime;
            int price, seats;
            iss >> trainID >> from >> depTime >> arrow >> to >> arrTime >> price >> seats;
            pricedResults.push_back({price, result});
        }
        sort(pricedResults.begin(), pricedResults.end());
        results.clear();
        for (const auto& p : pricedResults) {
            results.push_back(p.second);
        }
    }

    // Format final output
    string output = to_string(results.size()) + "\n";
    for (int i = 0; i < results.size(); i++) {
        output += results[i];
        if (i < results.size() - 1) output += "\n";
    }

    return output;
}

string query_transfer(const string& fromStation, const string& toStation, const string& date, const string& sortBy) {
    // Find all possible transfer combinations
    vector<pair<string, string>> transfers;

    for (auto& train1Pair : trains) {
        Train& train1 = train1Pair.second;
        if (!train1.released) continue;

        // Check if train1 passes through fromStation
        int fromIndex1 = -1;
        for (int i = 0; i < train1.stationNum; i++) {
            if (train1.stations[i] == fromStation) {
                fromIndex1 = i;
                break;
            }
        }
        if (fromIndex1 == -1) continue;

        // For each possible transfer station
        for (int transferIndex = fromIndex1 + 1; transferIndex < train1.stationNum; transferIndex++) {
            string transferStation = train1.stations[transferIndex];

            // Find trains that go from transferStation to toStation
            for (auto& train2Pair : trains) {
                Train& train2 = train2Pair.second;
                if (!train2.released || train2.trainID == train1.trainID) continue;

                // Check if train2 passes through transferStation and toStation
                int transferIndex2 = -1, toIndex2 = -1;
                for (int i = 0; i < train2.stationNum; i++) {
                    if (train2.stations[i] == transferStation) {
                        transferIndex2 = i;
                    }
                    if (train2.stations[i] == toStation) {
                        toIndex2 = i;
                    }
                }
                if (transferIndex2 == -1 || toIndex2 == -1 || transferIndex2 >= toIndex2) continue;

                // Calculate if transfer is possible (simplified - just check if it's the same date)
                // In reality, would need to check if arrival at transfer is before departure from transfer

                // Add this transfer combination
                string train1Info = train1.trainID + " " + fromStation + " -> " + transferStation;
                string train2Info = train2.trainID + " " + transferStation + " -> " + toStation;
                transfers.push_back({train1Info, train2Info});
            }
        }
    }

    if (transfers.empty()) {
        return "0";
    }

    // For simplicity, just return the first transfer found
    // In a real implementation, would sort by time or cost
    string result = transfers[0].first + "\n" + transfers[0].second;
    return result;
}

int buy_ticket(const string& username, const string& trainID, const string& date,
               int num, const string& fromStation, const string& toStation, bool queue) {

    if (loggedInUsers.find(username) == loggedInUsers.end()) {
        return -1;
    }

    if (trains.find(trainID) == trains.end()) {
        return -1;
    }

    Train& train = trains[trainID];
    if (!train.released) {
        return -1;
    }

    if (num <= 0 || num > train.seatNum) {
        return -1;
    }

    // Find stations
    int fromIndex = -1, toIndex = -1;
    for (int i = 0; i < train.stationNum; i++) {
        if (train.stations[i] == fromStation) {
            fromIndex = i;
        }
        if (train.stations[i] == toStation) {
            toIndex = i;
        }
    }

    if (fromIndex == -1 || toIndex == -1 || fromIndex >= toIndex) {
        return -1;
    }

    // Check seat availability
    bool hasEnoughSeats = true;
    for (int i = fromIndex; i < toIndex; i++) {
        if (train.seatAvailability.find(date) != train.seatAvailability.end()) {
            if (train.seatAvailability[date][i] < num) {
                hasEnoughSeats = false;
                break;
            }
        }
    }

    if (!hasEnoughSeats && !queue) {
        return -1;
    }

    if (!hasEnoughSeats && queue) {
        // Add to queue
        Order newOrder;
        newOrder.username = username;
        newOrder.trainID = trainID;
        newOrder.date = date;
        newOrder.fromStation = fromStation;
        newOrder.toStation = toStation;
        newOrder.num = num;
        newOrder.price = 0; // Will calculate when fulfilled
        newOrder.status = "pending";
        newOrder.timestamp = time(nullptr);
        orders.push_back(newOrder);

        return -2; // Special value for "queue"
    }

    // Calculate price
    int totalPrice = 0;
    for (int i = fromIndex; i < toIndex; i++) {
        totalPrice += train.prices[i];
    }
    totalPrice *= num;

    // Update seat availability
    if (train.seatAvailability.find(date) == train.seatAvailability.end()) {
        train.seatAvailability[date] = vector<int>(train.stationNum - 1, train.seatNum);
    }

    for (int i = fromIndex; i < toIndex; i++) {
        train.seatAvailability[date][i] -= num;
    }

    // Add order
    Order newOrder;
    newOrder.username = username;
    newOrder.trainID = trainID;
    newOrder.date = date;
    newOrder.fromStation = fromStation;
    newOrder.toStation = toStation;
    newOrder.num = num;
    newOrder.price = totalPrice;
    newOrder.status = "success";
    newOrder.timestamp = time(nullptr);
    orders.push_back(newOrder);

    return totalPrice;
}

string query_order(const string& username) {
    if (loggedInUsers.find(username) == loggedInUsers.end()) {
        return "-1";
    }

    // Collect user's orders
    vector<Order> userOrders;
    for (const Order& order : orders) {
        if (order.username == username) {
            userOrders.push_back(order);
        }
    }

    // Sort by timestamp (newest first)
    sort(userOrders.begin(), userOrders.end(), [](const Order& a, const Order& b) {
        return a.timestamp > b.timestamp;
    });

    // Format output
    string result = to_string(userOrders.size()) + "\n";
    for (int i = 0; i < userOrders.size(); i++) {
        const Order& order = userOrders[i];

        if (order.status == "pending") {
            result += "[pending] ";
        } else if (order.status == "success") {
            result += "[success] ";
        } else if (order.status == "refunded") {
            result += "[refunded] ";
        }

        // Get train info for times
        Train& train = trains[order.trainID];

        // Parse start time
        int startHour, startMin;
        sscanf(train.startTime.c_str(), "%d:%d", &startHour, &startMin);

        // Calculate departure time from fromStation
        int currentMonth, currentDay;
        sscanf(order.date.c_str(), "%d-%d", &currentMonth, &currentDay);
        int currentHour = startHour;
        int currentMin = startMin;

        int fromIndex = -1, toIndex = -1;
        for (int j = 0; j < train.stationNum; j++) {
            if (train.stations[j] == order.fromStation) {
                fromIndex = j;
            }
            if (train.stations[j] == order.toStation) {
                toIndex = j;
            }
        }

        // Calculate departure time
        for (int j = 0; j < fromIndex; j++) {
            currentMin += train.travelTimes[j];
            currentHour += currentMin / 60;
            currentMin %= 60;
            currentDay += currentHour / 24;
            currentHour %= 24;

            if (currentMonth == 6 && currentDay > 30) {
                currentMonth = 7;
                currentDay -= 30;
            } else if (currentMonth == 7 && currentDay > 31) {
                currentMonth = 8;
                currentDay -= 31;
            }

            if (j < train.stationNum - 1 && fromIndex > 0) {
                currentMin += train.stopoverTimes[j];
                currentHour += currentMin / 60;
                currentMin %= 60;
                currentDay += currentHour / 24;
                currentHour %= 24;

                if (currentMonth == 6 && currentDay > 30) {
                    currentMonth = 7;
                    currentDay -= 30;
                } else if (currentMonth == 7 && currentDay > 31) {
                    currentMonth = 8;
                    currentDay -= 31;
                }
            }
        }

        string departureTime = formatTime(currentMonth, currentDay, currentHour, currentMin);

        // Calculate arrival time
        for (int j = fromIndex; j < toIndex; j++) {
            currentMin += train.travelTimes[j];
            currentHour += currentMin / 60;
            currentMin %= 60;
            currentDay += currentHour / 24;
            currentHour %= 24;

            if (currentMonth == 6 && currentDay > 30) {
                currentMonth = 7;
                currentDay -= 30;
            } else if (currentMonth == 7 && currentDay > 31) {
                currentMonth = 8;
                currentDay -= 31;
            }
        }

        string arrivalTime = formatTime(currentMonth, currentDay, currentHour, currentMin);

        result += order.trainID + " " + order.fromStation + " " + departureTime + " -> " + order.toStation + " " + arrivalTime + " " + to_string(order.price) + " " + to_string(order.num);

        if (i < userOrders.size() - 1) {
            result += "\n";
        }
    }

    return result;
}

int refund_ticket(const string& username, int n) {
    if (loggedInUsers.find(username) == loggedInUsers.end()) {
        return -1;
    }

    // Collect user's orders (non-refunded only)
    vector<Order*> userOrders;
    for (Order& order : orders) {
        if (order.username == username && order.status != "refunded") {
            userOrders.push_back(&order);
        }
    }

    // Sort by timestamp (newest first)
    sort(userOrders.begin(), userOrders.end(), [](Order* a, Order* b) {
        return a->timestamp > b->timestamp;
    });

    if (n <= 0 || n > userOrders.size()) {
        return -1;
    }

    Order* orderToRefund = userOrders[n-1];

    if (orderToRefund->status == "pending") {
        // Just cancel the order
        orderToRefund->status = "refunded";
        return 0;
    }

    if (orderToRefund->status == "success") {
        // Restore seats
        Train& train = trains[orderToRefund->trainID];

        int fromIndex = -1, toIndex = -1;
        for (int i = 0; i < train.stationNum; i++) {
            if (train.stations[i] == orderToRefund->fromStation) {
                fromIndex = i;
            }
            if (train.stations[i] == orderToRefund->toStation) {
                toIndex = i;
            }
        }

        if (train.seatAvailability.find(orderToRefund->date) == train.seatAvailability.end()) {
            train.seatAvailability[orderToRefund->date] = vector<int>(train.stationNum - 1, train.seatNum);
        }

        for (int i = fromIndex; i < toIndex; i++) {
            train.seatAvailability[orderToRefund->date][i] += orderToRefund->num;
        }

        orderToRefund->status = "refunded";
        return 0;
    }

    return -1;
}

// Main function
int main() {
    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;

        istringstream iss(line);
        string command;
        iss >> command;

        if (command == "add_user") {
            string cur_username, username, password, name, mailAddr;
            int privilege = -1;
            string arg;

            while (iss >> arg) {
                if (arg == "-c") iss >> cur_username;
                else if (arg == "-u") iss >> username;
                else if (arg == "-p") iss >> password;
                else if (arg == "-n") iss >> name;
                else if (arg == "-m") iss >> mailAddr;
                else if (arg == "-g") iss >> privilege;
            }

            int result = add_user(cur_username, username, password, name, mailAddr, privilege);
            cout << result << endl;

        } else if (command == "login") {
            string username, password;
            string arg;

            while (iss >> arg) {
                if (arg == "-u") iss >> username;
                else if (arg == "-p") iss >> password;
            }

            int result = login(username, password);
            cout << result << endl;

        } else if (command == "logout") {
            string username;
            string arg;

            while (iss >> arg) {
                if (arg == "-u") iss >> username;
            }

            int result = logout(username);
            cout << result << endl;

        } else if (command == "query_profile") {
            string cur_username, username;
            string arg;

            while (iss >> arg) {
                if (arg == "-c") iss >> cur_username;
                else if (arg == "-u") iss >> username;
            }

            string result = query_profile(cur_username, username);
            cout << result << endl;

        } else if (command == "modify_profile") {
            string cur_username, username, password, name, mailAddr;
            int privilege = -1;
            string arg;

            while (iss >> arg) {
                if (arg == "-c") iss >> cur_username;
                else if (arg == "-u") iss >> username;
                else if (arg == "-p") iss >> password;
                else if (arg == "-n") iss >> name;
                else if (arg == "-m") iss >> mailAddr;
                else if (arg == "-g") iss >> privilege;
            }

            string result = modify_profile(cur_username, username, password, name, mailAddr, privilege);
            cout << result << endl;

        } else if (command == "add_train") {
            string trainID, stationsStr, pricesStr, startTime, travelTimesStr, stopoverTimesStr, saleDateStr, type;
            int stationNum, seatNum;
            string arg;

            while (iss >> arg) {
                if (arg == "-i") iss >> trainID;
                else if (arg == "-n") iss >> stationNum;
                else if (arg == "-m") iss >> seatNum;
                else if (arg == "-s") iss >> stationsStr;
                else if (arg == "-p") iss >> pricesStr;
                else if (arg == "-x") iss >> startTime;
                else if (arg == "-t") iss >> travelTimesStr;
                else if (arg == "-o") iss >> stopoverTimesStr;
                else if (arg == "-d") iss >> saleDateStr;
                else if (arg == "-y") iss >> type;
            }

            vector<string> stations = split(stationsStr, '|');
            vector<int> prices;
            vector<string> priceStrs = split(pricesStr, '|');
            for (const string& p : priceStrs) {
                prices.push_back(atoi(p.c_str()));
            }

            vector<int> travelTimes;
            vector<string> travelStrs = split(travelTimesStr, '|');
            for (const string& t : travelStrs) {
                travelTimes.push_back(atoi(t.c_str()));
            }

            vector<int> stopoverTimes;
            if (stationNum > 2 && stopoverTimesStr != "_") {
                vector<string> stopoverStrs = split(stopoverTimesStr, '|');
                for (const string& s : stopoverStrs) {
                    stopoverTimes.push_back(atoi(s.c_str()));
                }
            }

            vector<string> saleDates = split(saleDateStr, '|');

            int result = add_train(trainID, stationNum, seatNum, stations, prices,
                                 startTime, travelTimes, stopoverTimes, saleDates[0], saleDates[1], type[0]);
            cout << result << endl;

        } else if (command == "release_train") {
            string trainID;
            string arg;

            while (iss >> arg) {
                if (arg == "-i") iss >> trainID;
            }

            int result = release_train(trainID);
            cout << result << endl;

        } else if (command == "query_train") {
            string trainID, date;
            string arg;

            while (iss >> arg) {
                if (arg == "-i") iss >> trainID;
                else if (arg == "-d") iss >> date;
            }

            string result = query_train(trainID, date);
            cout << result << endl;

        } else if (command == "delete_train") {
            string trainID;
            string arg;

            while (iss >> arg) {
                if (arg == "-i") iss >> trainID;
            }

            int result = delete_train(trainID);
            cout << result << endl;

        } else if (command == "query_ticket") {
            string fromStation, toStation, date, sortBy = "time";
            string arg;

            while (iss >> arg) {
                if (arg == "-s") iss >> fromStation;
                else if (arg == "-t") iss >> toStation;
                else if (arg == "-d") iss >> date;
                else if (arg == "-p") iss >> sortBy;
            }

            string result = query_ticket(fromStation, toStation, date, sortBy);
            cout << result << endl;

        } else if (command == "query_transfer") {
            string fromStation, toStation, date, sortBy = "time";
            string arg;

            while (iss >> arg) {
                if (arg == "-s") iss >> fromStation;
                else if (arg == "-t") iss >> toStation;
                else if (arg == "-d") iss >> date;
                else if (arg == "-p") iss >> sortBy;
            }

            string result = query_transfer(fromStation, toStation, date, sortBy);
            cout << result << endl;

        } else if (command == "buy_ticket") {
            string username, trainID, date, fromStation, toStation, queueStr = "false";
            int num;
            string arg;

            while (iss >> arg) {
                if (arg == "-u") iss >> username;
                else if (arg == "-i") iss >> trainID;
                else if (arg == "-d") iss >> date;
                else if (arg == "-n") iss >> num;
                else if (arg == "-f") iss >> fromStation;
                else if (arg == "-t") iss >> toStation;
                else if (arg == "-q") iss >> queueStr;
            }

            bool queue = (queueStr == "true");
            int result = buy_ticket(username, trainID, date, num, fromStation, toStation, queue);

            if (result == -2) {
                cout << "queue" << endl;
            } else {
                cout << result << endl;
            }

        } else if (command == "query_order") {
            string username;
            string arg;

            while (iss >> arg) {
                if (arg == "-u") iss >> username;
            }

            string result = query_order(username);
            cout << result << endl;

        } else if (command == "refund_ticket") {
            string username;
            int n = 1;
            string arg;

            while (iss >> arg) {
                if (arg == "-u") iss >> username;
                else if (arg == "-n") iss >> n;
            }

            int result = refund_ticket(username, n);
            cout << result << endl;

        } else if (command == "clean") {
            users.clear();
            loggedInUsers.clear();
            trains.clear();
            orders.clear();
            cout << "0" << endl;

        } else if (command == "exit") {
            cout << "bye" << endl;
            break;

        } else {
            // Unknown command
            cout << "-1" << endl;
        }
    }

    return 0;
}