#pragma once

#ifndef PROPOSAL_H
#define PROPOSAL_H

#include "Client.h"
#include "Policy.h"
#include <string>

using namespace std;

class Proposal {
public:
    string AgentName;
    int ClientID;
    string ClientFirstName;
    string ClientLastName;
    string policyType;
    int Age;
    string ClientEmail;
    string phoneNumber;
    char Gender;
    string tobaccoStatus;
    double annualIncome;
    double lifeCoverAmt;
    int lifeCoverUpAge;

    Proposal() = default;  //default constructor

    Proposal(string agent_name,int client_id, string client_first_name, string client_last_name, string polType, 
        int age, string client_email, string phone_number, char gender, string tobacco_status, 
        double annual_income, double life_cover_amt, int life_cover_up_age) {

        AgentName = agent_name;
        ClientID = client_id;
        ClientFirstName = client_first_name;
        ClientLastName = client_last_name;
        policyType = polType;
        Age = age;
        ClientEmail = client_email;
        phoneNumber = phone_number;
        Gender = gender;
        tobaccoStatus = tobacco_status;
        annualIncome = annual_income;
        lifeCoverAmt = life_cover_amt;
        lifeCoverUpAge = life_cover_up_age;
    }

    //static void createProposal(const Proposal& proposal);
};

#endif // PROPOSAL_H
