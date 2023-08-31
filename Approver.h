#pragma once

#ifndef APPROVER_H
#define APPROVER_H

#include "User.h"
#include "Proposal.h"

class Approver : public User {
public:
    void acceptProposal(const Proposal& proposal);
    void rejectProposal(const Proposal& proposal);
};

#endif // APPROVER_H
