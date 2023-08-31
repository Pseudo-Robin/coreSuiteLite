#pragma once

#ifndef CLIENT_H
#define CLIENT_H

#include <string>

using namespace std;

class Client {
public:
    int ClientID;
    string firstName;
    string lastName;
    string contactNumber;
    int agentS_id;

    Client() = default;

    Client(int client_id, string first_name, string last_name, string contact_number, int agent_id ) {
        ClientID = client_id;
        firstName = first_name;
        lastName = last_name;
        contactNumber = contact_number;
        agentS_id = agent_id;
    }

    void CreateClient(int client_id, string first_name, string last_name, string contact_number, int agent_id) {
        ClientID = client_id;
        firstName = first_name;
        lastName = last_name;
        contactNumber = contact_number;
        agentS_id = agent_id;
    }
};

#endif // CLIENT_H
