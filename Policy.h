#ifndef POLICY_H
#define POLICY_H

#include <string>

class Policy {
public:
    std::string policyName;
    // Other policy attributes
};

class PolicyController {
public:
    void createNewPolicy(Policy& policy);
};

#endif // POLICY_H
