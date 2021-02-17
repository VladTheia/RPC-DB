#include <iostream>
#include <bits/stdc++.h>
#include <sstream>
#include "sensor.h"
#include <rpc/rpc.h>

#define RMACHINE "localhost"
using namespace std;

CLIENT *handle;
istringstream iss; // used to read tokens from stdin
// keeps the user id received from the server
unsigned long *user_id = (unsigned long*)calloc(1, sizeof(unsigned long)); 
int logged; // flag for login state
int loaded; // flag that checks if the 'load' command was first after login

int check_flags() {
    if (!logged) {
        cout << "You need to login first" << endl;
        return 0;
    }
    if (!loaded) {
        cout << "You need to load the database!" << endl;
        return 0;
    }

    return 1;
}

// create a data_sensor object with data from stdin
sensor_data* init_sensor_data() {
    sensor_data *data = (sensor_data*)malloc(sizeof(sensor_data));
    string in;
    iss >> in;
    data->data_id = stoi(in);
    iss >> in;
    data->no_values = stoi(in);
    data->values.values_len = data->no_values;
    data->values.values_val = new float[data->values.values_len];
    for (int i = 0; i < data->values.values_len; i++) {
        iss >> in;
        data->values.values_val[i] = stof(in);
    }
    data->user_id = *user_id;

    return data;
}

void print_data(sensor_data *data) {
    if (data->data_id) {
        cout << "user id: " << data->user_id << endl;
        cout << "id: " << data->data_id << endl;
        cout << "no_values: " << data->no_values << endl;
        cout << "values:" << endl;
        for (int i = 0; i < data->values.values_len; i++) {
            cout << data->values.values_val[i] << endl;
        }   
    } else {
        cout << "No data with that id " << endl;
    }
}

void print_stat(stat *stat_res) {
    if (stat_res->data_id) {
        cout << "Id: " << stat_res->data_id << endl;
        cout << "Min: " << stat_res->min << endl;
        cout << "Max: " << stat_res->max << endl;
        cout << "Medie: " << stat_res->mean << endl;
        cout << "Mediana: " << stat_res->median << endl;
    } else {
        cout << "No data with that id" << endl;
    }
}

void login() {
    string str;
    iss >> str;
    char* username = new char[str.size() + 1];
    copy(str.begin(), str.end(), username);
    username[str.size()] = '\0';

    if (logged) {
        cout << "You are already logged in!" << endl;
        return;
    }
    // servers sends a non-zero user_id if there is no error while logging in
    user_id = login_1(&username, handle);

    if (*user_id) {
        logged = 1;
        cout << "Logged in " << username << " with session id: " 
             << *user_id << endl;
    } else {
        cout << "User " << username << " is already logged in!" << endl;
    }
}

void logout() {
    if (!check_flags()) return;

    // send logout call to the server and resets local data
    logout_1(user_id, handle);
    logged = 0;  
    loaded = 0; 
    *user_id = 0;
    cout << "Logout successful!" << endl;
}

void add() {
    // get data from stdin and create a new sensor_data struct
    sensor_data *data = init_sensor_data();

    if (!check_flags()) return;

    // send the data to the server
    int *res = add_1(data, handle);

    // the server responds with 1 if it added the data or 0 if it already exists
    if (*res) {
        cout << "Added data" << endl;
    } else {
        cout << "Data with id " << data->data_id << " already exists" << endl;
    }
    free(data);
}

void update() {
    /*
        the update command can't fail since it either updates existing data 
        or adds it if there is no data with that id
    */
    sensor_data *data = init_sensor_data(); 

    if (!check_flags()) return;

    update_1(data, handle);

    cout << "Updated" << endl;

    free(data);
}

void del() {
    // read from stdin and create a struct containing the user_id and data_id
    string id_s;
    iss >> id_s;
    ids *del_ids = (ids*)malloc(sizeof(ids));
    del_ids->data_id = stoi(id_s);
    del_ids->user_id = *user_id;

    if (!check_flags()) return;

    int *res = del_1(del_ids, handle);

    /* 
        the server respons with 1 if the data was deleted 
        or 0 if there wasn't any data with that id 
    */
    if(*res) {
        cout << "Deleted " << del_ids->data_id << endl;
    } else {
        cout << "No data with id " << del_ids->data_id << endl;
    }

}

void read() {
    // read from stdin and create a struct containing the user_id and data_id
    string id_s;
    iss >> id_s;
    ids *read_ids = (ids*)malloc(sizeof(ids));
    read_ids->data_id = stoi(id_s);
    read_ids->user_id = *user_id;

    if (!check_flags()) return;

    sensor_data *data = read_1(read_ids, handle);

    // if there's no that with that id, server responds with 0 initialized struct
    print_data(data);
}

void get_stat() {
    // read from stdin and create a struct containing the user_id and data_id
    string id_s;
    iss >> id_s;
    ids *stat_ids = (ids*)malloc(sizeof(ids));
    stat_ids->data_id = stoi(id_s);
    stat_ids->user_id = *user_id;

    if (!check_flags()) return;

    stat *res = get_stat_1(stat_ids, handle);

    // if there's no that with that id, server responds with 0 initialized struct
    print_stat(res);
}

void get_stat_all() {
    if (!check_flags()) return;

    stats *res = get_stat_all_1(user_id, handle);

    // the server responds an array of stat structs and the client prints them
    for (int i = 0; i < res->stats_len; i++) { 
        print_stat(&res->stats_val[i]);
    }

    if (!res->stats_len) {
        cout << "No data" << endl;
    }
}

void load() {
    if (!logged) {
        cout << "You need to login first!" << endl;
        return;
    }

    // can't load 2 times
    if (loaded) {
        cout << "Already loaded!" << endl;
        return;
    }
    
    int *res = load_1(user_id, handle);\
    // the server loads the data if the database exists or it creates it
    loaded = 1;
    if (*res) {
        cout << "Data loaded" << endl;
    } else {
        cout << "Database created" << endl;
    }
}

void store() {
    if (!check_flags()) return;

    // make the server store in memory this user's data from his database
    store_1(user_id, handle);
}

void execute_command(string command) {
    // check what command was sent from stdin and execute it accordingly
    if (command == "login") {
        login();
    } else if (command == "load") {
        load();
    } else if (command == "logout") {
        logout();
    } else if (command == "add") {
        add();
    } else if (command == "update") {
        update();
    } else if (command == "del") {
        del();
    } else if (command == "read") {
        read();
    } else if (command == "get_stat") {
        get_stat();
    } else if (command == "get_stat_all") {
        get_stat_all();
    } else if (command == "store") {
        store();
    }
}

int main(int argc, char*argv[]) {
    string filename;
    string command;

    handle = clnt_create(
        RMACHINE,
        SENSOR_PROG,
        SENSOR_VERS,
        "tcp"
    );

    if (handle == NULL) {
        perror("Null handle");
        return -1;
    }

    int reading = 1;
    string line;
    while (getline(cin, line) && !line.empty()) {
        iss = istringstream(line);
        while (iss >> command) {
            execute_command(command);
        }
    }

    return 0;
}