#include <iostream>
#include <bits/stdc++.h>
#include <map>
#include "sensor.h"
#include <rpc/rpc.h>

#define RMACHINE "localhost"
using namespace std;

unsigned long session_id = 1; // keeps track of each login
map<string, unsigned long> users; // map for username - session_id (user_id)
map<unsigned long, vector<sensor_data*>> data_map; // map for user_id - user data
map<unsigned long, string> databases; // map for user_id - user database name

// create and store in memory a new struct with the data received from the client
sensor_data* init_sensor_data(sensor_data* data) {
    sensor_data *new_data = (sensor_data*)malloc(sizeof(sensor_data));
    new_data->data_id = data->data_id;
    new_data->no_values = data->no_values;
    new_data->values.values_len = new_data->no_values;
    new_data->values.values_val = new float[new_data->values.values_len];
    for (int i = 0; i < data->values.values_len; i++) {
        new_data->values.values_val[i] = data->values.values_val[i];
    }
    new_data->user_id = data->user_id;

    return new_data;
}

void print_data(sensor_data *data) {
    cout << "user id: " << data->user_id << endl;
    cout << "id: " << data->data_id << endl;
    cout << "no_values: " << data->no_values << endl;
    cout << "values:" << endl;
    for (int i = 0; i < data->values.values_len; i++) {
        cout << data->values.values_val[i] << endl;
    }   
}

// functions used for get_stat command
int get_min(sensor_data *data) {
    int len = data->values.values_len;
    float min = data->values.values_val[0];

    for (int i = 1; i < len; i++) {
        float aux = data->values.values_val[i];
        min = min < aux ? min : aux;
    }

    return min;
}

float get_max(sensor_data *data) {
    int len = data->values.values_len;
    float max = data->values.values_val[0];

    for (int i = 1; i < len; i++) {
        float aux = data->values.values_val[i];
        max = max > aux ? max : aux;
    }

    return max;
}

float get_mean(sensor_data *data) {
    int len = data->values.values_len;
    float sum = data->values.values_val[0];

    for (int i = 1; i < len; i++)
        sum += data->values.values_val[i];

    return sum / len;
}

float get_median(sensor_data *data) {
    int len = data->values.values_len;
    float *aux = (float*)malloc(sizeof(float) * len);

    for (int i = 0; i < len; i++)
        aux[i] = data->values.values_val[i];

    sort(aux, aux + len);

    if (len % 2 != 0)
        return aux[len / 2];

    return (aux[(len - 1) / 2] + aux[len / 2]) / 2;
}

// store the stats of the sensor_data in the stat struct
void get_stat_res(stat *stat_res, sensor_data *data) {
    stat_res->data_id = data->data_id;
    stat_res->min = get_min(data);
    stat_res->max = get_max(data);
    stat_res->mean = get_mean(data);
    stat_res->median = get_median(data);   
}

unsigned long * login_1_svc(char **username, struct svc_req *cl) {
    unsigned long *id = (unsigned long*)malloc(sizeof(unsigned long));

    // if there's no user already logged with that username
    if (users.find(*username) == users.end()) {
        // give him the current session_id
        *id = session_id;
        string name(*username);
        // add him to the maps
        users[name] = session_id;
        data_map[session_id] = vector<sensor_data*>();
        string db_name(name + ".rpcdb");
        databases[session_id] = db_name;

        cout << "Logged in " << *username << " with id: " << *id << endl;

        // increase session_id for next login
        session_id++;
    } else {
        // there's already someone logged with that username, so login fails
        *id = 0;
        cout << "Couldn't login" << endl;
    }

    // send the user his session_id (user_id)
    return id;
}

int * logout_1_svc(u_long *user_id, struct svc_req *cl) {
    int* res = (int*)malloc(sizeof(int));
    *res = 1;
    
    // delete him from the maps and free the memory
    for (auto &i : users) {
        if (i.second == *user_id) {
            cout << "Logged out " << i.first << endl;
            users.erase(i.first);
            break;
        }
    }

    map<unsigned long, vector<sensor_data *>>::iterator it = data_map.find(*user_id);
    for (auto &i : it->second) {
        free(i);
    }
    data_map.erase(it);

    map<unsigned long, string>::iterator db = databases.find(*user_id);
    databases.erase(db);

    return res;
}

int * add_1_svc(sensor_data *data, struct svc_req *cl) {
    int* res = (int*)malloc(sizeof(int));
    *res = 1;

    // select this user's sensor_data array
    map<unsigned long, vector<sensor_data *>>::iterator it = data_map.find(data->user_id);

    for (auto &i : it->second) {
        // check if there is data with that id
        if (i->data_id == data->data_id) {
            *res = 0;
            cout << "Already added " << i->data_id << endl;
            return res;
        }
    }
    // it there isn't, create a new struct and add it
    it->second.push_back(init_sensor_data(data));
    cout << "Added: " << data->data_id << endl;

    return res;
}

int * update_1_svc(sensor_data *data, struct svc_req *cl) {
    int* res = (int*)malloc(sizeof(int));
    *res = 1;
    
    map<unsigned long, vector<sensor_data *>>::iterator it = data_map.find(data->user_id);
    for (int i = 0; i < it->second.size(); i++) {
        // if there is data with that id, update it
        if (it->second.at(i)->data_id == data->data_id) {
            print_data(it->second.at(i));
            it->second.at(i) = init_sensor_data(data);
            print_data(it->second.at(i));
            
            return res;
        }
    }
    // else, add it to the sensor_data array
    it->second.push_back(init_sensor_data(data));
    cout << "Added: " << data->data_id << endl;

    return res;
}

int * del_1_svc(ids *del_ids, struct svc_req *cl) {
    int* res = (int*)malloc(sizeof(int));
    *res = 1;
    map<unsigned long, vector<sensor_data *>>::iterator it = data_map.find(del_ids->user_id);
    for (int i = 0; i < it->second.size(); i++) {
        // search for the sensor_data with the wanted id and erase it from the array
        if (it->second.at(i)->data_id == del_ids->data_id) {
            it->second.erase(it->second.begin() + i);
            cout << "Deleted" << del_ids->data_id << endl;
            return res;
        }
    }
    // if there's no data with that id, send an error code
    cout << "Couldn't delete" << endl;
    *res = 0;
    return res;
}

sensor_data * read_1_svc(ids *read_ids, struct svc_req *cl) {
    map<unsigned long, vector<sensor_data *>>::iterator it = data_map.find(read_ids->user_id);

    for (int i = 0; i < it->second.size(); i++) {
        // if there is data with that id, send the struct
        if (it->second.at(i)->data_id == read_ids->data_id) {
            return it->second.at(i);
        }
    }

    // else send a zero initialized struct
    sensor_data *empty_data = (sensor_data*)calloc(1, sizeof(sensor_data));
    return empty_data;
}

stat * get_stat_1_svc(ids *stat_ids, struct svc_req *cl) {
    stat* stat_res = (stat*)calloc(1, sizeof(stat));
    map<unsigned long, vector<sensor_data *>>::iterator it = data_map.find(stat_ids->user_id);
    sensor_data *data;

    for (int i = 0; i < it->second.size(); i++) {
        // if there is data with that id, calculate the stats and send them
        if (it->second.at(i)->data_id == stat_ids->data_id) {
            data = it->second.at(i);
            get_stat_res(stat_res, data);
            break;
        }
    }

    return stat_res;
}

stats * get_stat_all_1_svc(u_long *user_id, struct svc_req *cl) {
    map<unsigned long, vector<sensor_data *>>::iterator it = data_map.find(*user_id);
    stats *stat_arr = (stats*)calloc(1, sizeof(stats));
    stat_arr->stats_len = it->second.size();
    stat_arr->stats_val = (stat*)calloc(it->second.size(), sizeof(stat));
    
    // calculate the stats for each of that user's sensor_data and send the results
    for (int i = 0; i < it->second.size(); i++) {
        get_stat_res(&stat_arr->stats_val[i], it->second.at(i));
    }

    return stat_arr;
}

int * load_1_svc(u_long *user_id, struct svc_req *cl) {
    int *res = (int*)malloc(sizeof(int));
    *res = 1;
    
    // get user's database name
    map<unsigned long, string>::iterator it = databases.find(*user_id);
    // get user's sensor_data array
    map<unsigned long, vector<sensor_data *>>::iterator it2 = data_map.find(*user_id);
    fstream db;
    db.open(it->second, fstream::in | fstream::out | fstream::app);

    string id_s;
    string len_s;
    string val_s;

    // if there wasn't a database, create it
    if (!db) {
        db.open(it->second,  fstream::in | fstream::out | fstream::trunc);
        *res = 0;
        db.close();
    } else {
        // read from database token by token and init the sensor_data array
        while (db >> id_s) {
            unsigned long id = stoul(id_s);
            db >> len_s;
            int len = stoi(len_s);
            float *vals = (float*)malloc(sizeof(float)*len);
            for (int i = 0; i < len; i++) {
                db >> val_s;
                vals[i] = stof(val_s);
            }
            
            sensor_data *data = (sensor_data*)malloc(sizeof(sensor_data));
            data->user_id = *user_id;
            data->data_id = id;
            data->no_values = len;
            data->values.values_len = len;
            data->values.values_val = vals;
            it2->second.push_back(data);
        }
        db.close();
    }

    return res;
}

int * store_1_svc(u_long *user_id, struct svc_req *cl) {
    int *res = (int*)malloc(sizeof(int));
    *res = 1;

    map<unsigned long, string>::iterator it = databases.find(*user_id);
    map<unsigned long, vector<sensor_data *>>::iterator it2 = data_map.find(*user_id);
    fstream db;
    db.open(it->second, fstream::out);

    // loop through user's sensor_data array and write it in his database
    for (int i = 0; i < it2->second.size(); i++) {
        sensor_data *data = it2->second.at(i);
        db << data->data_id;
        db << " ";
        db << data->no_values;
        db << " ";
        for (int j = 0; j < data->no_values; j++) {
            db << data->values.values_val[j];
            db << " ";
        }
        db << "\n";
    }

    db.close();

    return res;
}