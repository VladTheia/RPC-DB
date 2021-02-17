struct sensor_data {
    unsigned long user_id;
    int data_id;
    int no_values;
    float values<>;
};

struct ids {
    unsigned long user_id;
    int data_id;
};

struct stat {
    unsigned long user_id;
    int data_id;
    float min;
    float max;
    float mean;
    float median;
};

typedef stat stats<>;

program SENSOR_PROG {
    version SENSOR_VERS {
        unsigned long LOGIN(string)       = 1;
        int LOGOUT(unsigned long)         = 2;
        int ADD(sensor_data)              = 3;
        int UPDATE(sensor_data)           = 4;
        int DEL(ids)                      = 5;
        sensor_data READ(ids)             = 6;
        stat GET_STAT(ids)                = 7;
        stats GET_STAT_ALL(unsigned long) = 8;
        int LOAD(unsigned long)           = 9;
        int STORE(unsigned long)          = 10;
    } = 1;
} = 123456789;