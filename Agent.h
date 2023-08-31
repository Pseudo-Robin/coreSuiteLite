#pragma once

#ifndef AGENT_H
#define AGENT_H

/*
void createNewProposal(string agentName, int agentId, string policyType);
void addNewClient(int agent_id)
void cancelPolicy(int policy_id);
void viewAllClients(int agent_id);
void viewAllProposals(string agent_name);
*/



#include <string>
#include <vector>
#include "User.h"
#include "Client.h"
#include "Proposal.h"
#include "Policy.h"

class Agent : public User {
public:
    std::string Agentname; // Agent's name
    std::string AgentPassword; // Agent's password
};

#endif // AGENT_H

