#include <iostream>
#include <string>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <fstream>

#include "Agent.h"
#include "Approver.h"
#include "Payment.h"
#include "mysql_connection.h"

using namespace std;

struct PremiumInfo {
    double extraMonthlyPremium;
    bool optedForADP;
    bool optedForCC;
};
struct PaymentInfo {
    int payUpTo;
    string city;
    string education;
    string occupation;
    string paymentMode;
    string modeOfPayment;
    bool TnC;
    double paymentAfterTaxes;
};


// Function declarations
void agentDashboardMenu(string,int);
void approverDashboardMenu(string,int);
void showMainMenu();
void createNewProposal(string,int,string);
void addNewClient(int);
void viewAllClients(int);
void viewAllProposals(string);
void sendProposalForApproval(int);
void cancelExistingPolicy(int);

double retInitialAnnualPremium(int, char, string, double, double);

PremiumInfo addRider(double);
PaymentInfo addPayment(double);

// DB helpers
const string server = "localhost:3306";
const string username = "root";
const string password = "Rs112233";
const string schemaName = "miniprojectschema";


// Global Variables
int cAgentID;
int newClientID = 1;
string Cfname, Clname;
string CcontactNumber;
string inputUsername, inputPassword;
string inputApproverUsername, inputApproverPassword;
string selectQuery;

Client newClient;

sql::ResultSet* resLastID;
sql::Statement* stmt;
sql::ResultSet* res;
sql::Connection* con;


// Function definitions

// Final Payment and Riders
PaymentInfo addPayment(double totalPremiumAfterTaxes) {

    PaymentInfo results;
    
    results.payUpTo = 0;

    cout << "\n\nEnter your payment details\n\n";

    cout << "\nEnter payUpToYears : "; cin >> results.payUpTo;
    cout << "\nEnter city : "; cin >> results.city;
    cout << "\nEnter education : "; cin >> results.education;
    cout << "\nEnter occupation : "; cin >> results.occupation;
    cout << "\nEnter payment mode : "; cin >> results.paymentMode;
    cout << "\nEnter mode of payment : "; cin >> results.modeOfPayment;
    cout << "\nAccept Terms and Conditions : "; cin >> results.TnC;

    if (results.paymentMode == "monthly") {
        results.paymentAfterTaxes = totalPremiumAfterTaxes;
    }
    else if (results.paymentMode == "half_yearly") {
        double total = totalPremiumAfterTaxes * 12;
        total = total - total * 0.10;    // 10% discount
        results.paymentAfterTaxes = total / 2;
    }
    else if (results.paymentMode == "annually") {
        double total = totalPremiumAfterTaxes * 12;
        total = total - total * 0.15;    // 15% discount
        results.paymentAfterTaxes = total;
    }
    else {
        cout << "\nInvalid payment mode! ";
    }

    return results;
}
PremiumInfo addRider(double lifeCoverAmt) {

    PremiumInfo results;

    results.extraMonthlyPremium = 0;
    results.optedForCC = false;
    results.optedForADP = false;

    int riderChoice;
    double adpCoverageAmt, ccCoverageAmt;
    double extraADPmonthlyPremium=0, extraCCmonthlyPremium=0;


    do {
        cout << "\nChoose an option:" << endl;
        cout << "1. Accidental Death Benefit" << endl;
        cout << "2. Comprehensive Care" << endl;
        cout << "3. Confirm" << endl;
        cout << "Enter your choice (1-3):\n ";
        cin >> riderChoice;

        switch (riderChoice) {
        case 1:
            cout << "\nYou chose Accidental Death Benefit." << endl;
            cout << "Enter coverage amount : "; cin >> adpCoverageAmt;

            if (adpCoverageAmt < 25000) {
                cout << "25k is the minimum coverage offered\n";
            }
            else {
                extraADPmonthlyPremium = 10 + ((adpCoverageAmt - 25000) / 50000) * 10;
                results.optedForADP = true;
            }
            break;

        case 2:
            cout << "\nYou chose Comprehensive Care." << endl;
            cout << "Enter coverage amount : "; cin >> ccCoverageAmt;

            if (ccCoverageAmt < 200000) {
                cout << "200k is the minimum coverage offered\n";
            }
            else {
                extraCCmonthlyPremium = 80 + ((ccCoverageAmt - 200000) / 300000) * 400;
                results.optedForCC = true;
            }
            break;

        case 3:
            results.extraMonthlyPremium = extraADPmonthlyPremium + extraCCmonthlyPremium;
            cout << extraADPmonthlyPremium << " xxx " << extraCCmonthlyPremium << endl;
            break;

        default:
            cout << "Invalid choice. Please enter a valid option (1-3)." << endl;
        }
    } while (riderChoice != 3);

    return results;
}

// Function for returning intial premium
double retInitialAnnualPremium(int age, char gender, string tobaccoStatus, double annualIncome, double lifeCoverAmt) {
    
    double ageFact = 0;
    double genderFact = 0;
    double tobaccFact = 0;
    double incomeFact = 0;
    
    //Age Factor
    if (age < 10) {
        ageFact = 0;
    }
    else if (age >= 10 && age < 20) {
        ageFact = 0.02;
    }
    else if (age >= 20 && age < 40) {
        ageFact = 0.035;
    }
    else if (age >= 40 && age < 60) {
        ageFact = 0.040;
    }
    else {
        ageFact = 0.055;
    }

    //Gender Factor
    if (gender == 'M') {
        genderFact = 0.02;
    }
    else {
        genderFact = 0.01;
    }

    //Tobacco Factor
    if (tobaccoStatus == "Yes") {
        tobaccFact = 0.05;
    }
    else
        tobaccFact = 0;

    //Income Factor
    if (annualIncome < 100000) {
        incomeFact = 0.05;
    }
    else if (annualIncome >= 100000 && annualIncome < 500000) {
        incomeFact = 0.04;
    }
    else if (annualIncome >= 500000 && annualIncome < 1000000) {
        incomeFact = 0.03;
    }
    else if (annualIncome >= 1000000 && annualIncome < 2000000) {
        incomeFact = 0.02;
    }
    else
        incomeFact = 0.01;


    double initPrem = lifeCoverAmt / 100 + lifeCoverAmt * ageFact + lifeCoverAmt * genderFact +
        lifeCoverAmt * tobaccFact + lifeCoverAmt * incomeFact;

    return initPrem;
}

// Agent Dashboard Functions
void viewAllProposals(string agent_name) {

    cout << "\nPending Proposals: \n\n";

    sql::Driver* driver;
    sql::Connection* con;

    try {
        driver = get_driver_instance();
        con = driver->connect(server, username, password);
        con->setSchema(schemaName);
    }
    catch (sql::SQLException e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        system("pause");
        exit(1);
    }

    // Prepare the SQL statement to retrieve clients belonging to a specific agent_id
    sql::PreparedStatement* pstmt;
    pstmt = con->prepareStatement("SELECT proposal_id, policyType, client_id, firstName, lastName FROM Proposals WHERE agentName = ?");
    pstmt->setString(1, agent_name);

    // Execute the query and retrieve the result set
    sql::ResultSet* res = pstmt->executeQuery();

    // Iterate through the result set and display client information
    while (res->next()) {
        int proposal_id = res->getInt("proposal_id");
        int client_id = res->getInt("client_id");
        string firstName = res->getString("firstName");
        string lastName = res->getString("lastName");
        string policyType = res->getString("policyType");

        cout << "Proposal ID: " << proposal_id <<" - "<< firstName <<" "<<lastName<<" - "<<policyType<<endl;
        cout << "--------------" << endl;
    }

    // Clean up resources
    delete res;
    delete pstmt;
    delete con;

}
void viewAllClients(int agent_id) {

    cout << "\nA list of your clients: \n\n";

    sql::Driver* driver;
    sql::Connection* con;

    try {
        driver = get_driver_instance();
        con = driver->connect(server, username, password);
        con->setSchema(schemaName);
    }
    catch (sql::SQLException e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        system("pause");
        exit(1);
    }

    // Prepare the SQL statement to retrieve clients belonging to a specific agent_id
    sql::PreparedStatement* pstmt;
    pstmt = con->prepareStatement("SELECT client_id, firstName, lastName, contactNumber FROM Clients WHERE agent_id = ?");
    pstmt->setInt(1, agent_id);

    // Execute the query and retrieve the result set
    sql::ResultSet* res = pstmt->executeQuery();

    // Iterate through the result set and display client information
    while (res->next()) {
        int clientID = res->getInt("client_id");
        string firstName = res->getString("firstName");
        string lastName = res->getString("lastName");
        string contactNumber = res->getString("contactNumber");

        cout << "Client ID: " << clientID << endl;
        cout << "Name: " << firstName << " " << lastName << endl;
        cout << "Contact Number: " << contactNumber << endl;
        cout << "-----------------------------" << endl;
    }

    // Clean up resources
    delete res;
    delete pstmt;
    delete con;
}
void addNewClient(int agent_id) {

    std::cout << "------------------------------------------------------------------------------ \n";
    std::cout << "Add a new client \n\n";

    std::cout << "Enter first name and last name: \n";
    std::cin >> Cfname >> Clname;
    std::cout << "Enter contactNumber: \n";
    std::cin >> CcontactNumber;

    cAgentID = agent_id;

    // CODE TO WRITE THIS CLIENT TO THE CLIENTS TABLE
    sql::Driver* driver;
    sql::Connection* con;

    try {
        driver = get_driver_instance();
        con = driver->connect(server, username, password);
    }
    catch (sql::SQLException e) {
        std::cout << "Could not connect to server. Error message: " << e.what() << endl;
        system("pause");
        exit(1);
    }

    con->setSchema(schemaName);

    // Prepare the SQL statement
    sql::PreparedStatement* pstmt;
    pstmt = con->prepareStatement("INSERT INTO Clients (firstName, lastName, contactNumber,agent_id) VALUES (?, ?, ?, ?)");
    pstmt->setString(1, Cfname);
    pstmt->setString(2, Clname);
    pstmt->setString(3, CcontactNumber);
    pstmt->setInt(4, cAgentID);

    pstmt->execute();


    sql::PreparedStatement* pstmtLastID;
    pstmtLastID = con->prepareStatement("SELECT LAST_INSERT_ID()");
    resLastID = pstmtLastID->executeQuery();


    if (resLastID->next()) {
        newClientID = resLastID->getInt(1);
        std::cout << "\nClient saved with client_id: " << newClientID << endl << endl;
    }

    // CREATE A NEW CLIENT OBJECT
    newClient.CreateClient(newClientID, Cfname, Clname, CcontactNumber, cAgentID);

    // Clean up resources
    delete pstmt;
    delete con;
}
void createNewProposal(string agentName,int agent_id, string policyType) {

    int choice = 0;
    sql::ResultSet* resLastID;

    do {
        std::cout << "\nSelect policy type:" << endl;
        std::cout << "1. Term Life Policy" << endl;
        std::cout << "2. Whole Life Policy" << endl;
        std::cout << "3. Endowment Policy" << endl;
        std::cout << "4. MoneyBack Policy" << endl;
        std::cout << "5. Go Back" << endl;
        std::cout << "Enter your choice (1-5): ";
        std::cin >> choice;

        // Clear newline character from input buffer
        std::cin.ignore();

        switch (choice) {
        case 1:
            policyType = "Term Life Policy";
            break;
        case 2:
            policyType = "Whole Life Policy";
            break;
        case 3:
            policyType = "Endowment Policy";
            break;
        case 4:
            policyType = "MoneyBack Policy";
            break;
        case 5:
            std::cout << "Going back to previous menu." << endl;
            agentDashboardMenu(agentName, agent_id);
            break;
        default:
            std::cout << "Incorrect choice. Please enter a valid option." << endl;
        }

        if (choice >= 1 && choice <= 4) {

            int clientID;
            string fname, lname, emailID, phoneNumber;
            char gender;
            int age, lifeCoverUpAge;
            double lifeCoverAmt, annualIncome;
            string tobaccoStatus;

            std::cout << "\nEnter Client's ID: \n";
            std::cin >> clientID;

            std::cout << "Enter Client's First Name and Last's Name: \n";
            std::cin >> fname >> lname;

            std::cout << "Enter Client's email and phone number: \n";
            std::cin >> emailID >> phoneNumber;

            std::cout << "Enter Gender('M','F'): \n"; std::cin >> gender;
            std::cout << "Enter Age: \n"; std::cin >> age;
            std::cout << "Tobacco('Yes'/'No'): \n"; std::cin >> tobaccoStatus;
            std::cout << "Enter Annual Income \n"; std::cin >> annualIncome;
            std::cout << "Enter Cover Amount: \n"; std::cin >> lifeCoverAmt;
            std::cout << "Enter CoverUpAge: \n"; std::cin >> lifeCoverUpAge;

            // Create a new proposal object
            Proposal initialProposal(agentName,clientID, fname, lname, policyType, age, emailID, phoneNumber, gender
                , tobaccoStatus, annualIncome, lifeCoverAmt, lifeCoverUpAge);

            // Funtion to return initial premium
            double initialAnnualPremium;
            initialAnnualPremium = retInitialAnnualPremium(age, gender,tobaccoStatus, annualIncome, lifeCoverAmt);

            double initialMonthlyPremium = initialAnnualPremium/12;

            cout << "\nInitial Premium is (per month) : " << initialMonthlyPremium << endl;


            // Function for Riders
            char riderChoice;
            cout << "\nDo you want to add rider(s)? (Y/N) : ";
            cin >> riderChoice;

            double extraMonthlyPremiumRiders = 0;
            PremiumInfo premiumInfo{};

            if (riderChoice == 'Y') {
                premiumInfo = addRider(initialProposal.lifeCoverAmt);
            }
            else if (riderChoice == 'N')
                cout << "\nNo rider(s) selected.";
            else
                cout << "\nInvalid Choice!";

            double totalPremiumAfterRiders = initialMonthlyPremium + premiumInfo.extraMonthlyPremium;
            cout << "\n After Rider(s), your monthly premium is: " << totalPremiumAfterRiders;
            double totalPremiumAfterTaxes = totalPremiumAfterRiders + totalPremiumAfterRiders * 0.18;
            cout << "\n After taxes: " << totalPremiumAfterTaxes;

            // Opted for which rides (if any)
            bool optedForADP = premiumInfo.optedForADP;
            bool optedForCC = premiumInfo.optedForCC;

            // Function for Payment Tenure and details
            PaymentInfo paymentInfo{};
            paymentInfo=addPayment(totalPremiumAfterTaxes);


            // Perform operations with the newProposal object as needed
            std::cout << endl << endl << "Proposal created:" << endl;
            std::cout << "Client ID: " << initialProposal.ClientID<<endl;
            std::cout << "Client's name: " << initialProposal.ClientFirstName << " " << initialProposal.ClientLastName << endl;
            std::cout << "Policy type: " << initialProposal.policyType << endl << endl << endl;

            // CODE to save the new proposal details to my "Proposals" TABLE

            sql::Driver* driver;
            sql::Connection* con;

            try {
                driver = get_driver_instance();
                con = driver->connect(server, username, password);
            }
            catch (sql::SQLException e) {
                std::cout << "Could not connect to server. Error message: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            con->setSchema(schemaName);

            // Prepare the SQL statement
            sql::PreparedStatement* pstmt;
            pstmt = con->prepareStatement("INSERT INTO Proposals (agentName, policyType, client_id, firstName, lastName, age, email_ID, phoneNumber, gender, tobaccoStatus, annualIncome, lifeCoverAmt, lifeCoverUpAge, agentID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            pstmt->setString(1, agentName);
            pstmt->setString(2, policyType);
            pstmt->setInt(3, clientID);
            pstmt->setString(4, fname);
            pstmt->setString(5, lname);
            pstmt->setInt(6, age);
            pstmt->setString(7, emailID);
            pstmt->setString(8, phoneNumber);
            pstmt->setString(9, string(1, gender));
            pstmt->setString(10, tobaccoStatus);
            pstmt->setDouble(11, annualIncome);
            pstmt->setDouble(12, lifeCoverAmt);
            pstmt->setInt(13, lifeCoverUpAge);
            pstmt->setInt(14, agent_id);
            pstmt->execute();



            sql::PreparedStatement* pstmtLastID;
            pstmtLastID = con->prepareStatement("SELECT LAST_INSERT_ID()");
            resLastID = pstmtLastID->executeQuery();

            int newProposalID;
            if (resLastID->next()) {
                newProposalID = resLastID->getInt(1);
                std::cout << "\nProposal saved with proposal_id: " << newProposalID << endl << endl;
            }

            // Code to save new payment details to the PAYMENTS table

            pstmt = con->prepareStatement("INSERT INTO PAYMENTS (proposal_id, clientID, payUpTo, education, occupation, paymentMode, modeOfPayment, TnC, paymentAfterTaxes,optedForADB,optedForCC) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            pstmt->setInt(1, newProposalID);
            pstmt->setInt(2, initialProposal.ClientID);
            pstmt->setInt(3, paymentInfo.payUpTo);
            pstmt->setString(4, paymentInfo.education);
            pstmt->setString(5, paymentInfo.occupation);
            pstmt->setString(6, paymentInfo.paymentMode);
            pstmt->setString(7, paymentInfo.modeOfPayment);
            pstmt->setBoolean(8, paymentInfo.TnC);
            pstmt->setDouble(9, paymentInfo.paymentAfterTaxes);
            pstmt->setBoolean(10,premiumInfo.optedForADP);
            pstmt->setBoolean(11,premiumInfo.optedForCC);
            pstmt->execute();

            cout << "\nPayment details saved." << endl;

            // Clean up resources
            delete pstmt;
            delete con;

            // ##### END OF WRITING NEW PROPOSAL TO MY TABLE "PROPOSALS" AND PAYMENTS #####

        }
    } while (choice != 5);

}
void sendProposalForApproval(int proposal_id) {
    sql::Driver* driver;
    sql::Connection* con;

    try {
        driver = get_driver_instance();
        con = driver->connect(server, username, password);
        con->setSchema(schemaName);
    }
    catch (sql::SQLException e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        system("pause");
        exit(1);
    }

    // Retrieve data from PROPOSALS and PAYMENTS tables
    sql::Statement* stmt = con->createStatement();
    sql::ResultSet* result = stmt->executeQuery("SELECT * FROM PROPOSALS "
        "JOIN PAYMENTS ON PROPOSALS.proposal_id = PAYMENTS.proposal_id "
        "WHERE PROPOSALS.proposal_id = " + std::to_string(proposal_id));

    // Insert data into POLICIES table
    sql::PreparedStatement* insertStmt = con->prepareStatement("INSERT INTO POLICIES "
        "(proposal_id, payID, agentName, agentID, policyType, client_id, firstName,"
        "lastName, age, email_ID, phoneNumber, gender, tobaccoStatus,annualIncome,"
        "lifeCoverAmt, lifeCoverUpAge, payUpTo, education, occupation, paymentMode,"
        "modeOfPayment, TnC, paymentAfterTaxes) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");

    while (result->next()) {
        insertStmt->setInt(1, result->getInt("proposal_id"));
        insertStmt->setString(2, result->getString("payID"));
        insertStmt->setString(3, result->getString("agentName"));
        insertStmt->setInt(4, result->getInt("agentID"));
        insertStmt->setString(5, result->getString("policyType"));
        insertStmt->setInt(6, result->getInt("client_id"));
        insertStmt->setString(7, result->getString("firstName"));
        insertStmt->setString(8, result->getString("lastName"));
        insertStmt->setInt(9, result->getInt("age"));
        insertStmt->setString(10, result->getString("email_ID"));
        insertStmt->setString(11, result->getString("phoneNumber"));
        insertStmt->setString(12, result->getString("gender"));
        insertStmt->setString(13, result->getString("tobaccoStatus"));
        insertStmt->setDouble(14, result->getDouble("annualIncome"));
        insertStmt->setDouble(15, result->getDouble("lifeCoverAmt"));
        insertStmt->setInt(16, result->getInt("lifeCoverUpAge"));
        insertStmt->setInt(17, result->getInt("payUpTo"));
        insertStmt->setString(18, result->getString("education"));
        insertStmt->setString(19, result->getString("occupation"));
        insertStmt->setString(20, result->getString("paymentMode"));
        insertStmt->setString(21, result->getString("modeOfPayment"));
        insertStmt->setBoolean(22, result->getBoolean("TnC"));
        insertStmt->setDouble(23, result->getDouble("paymentAfterTaxes"));


        insertStmt->execute();
    }

    // Change proposal status to "sent"

    stmt = con->createStatement();
    std::string updateQuery = "UPDATE proposals SET proposalStatus = 'sent' WHERE proposal_id = " + std::to_string(proposal_id);
    
    int rowsAffected = stmt->executeUpdate(updateQuery);

    if (rowsAffected > 0) {
        std::cout << "Proposal status updated for proposal ID: " << proposal_id << std::endl;
    }
    else {
        std::cout << "No rows updated for proposal ID: " << proposal_id << std::endl;
    }


    // Clean up
    delete result;
    delete stmt;
    delete insertStmt;
    con->close();
    delete con;
}
void cancelExistingPolicy(int policy_id) {

    sql::Driver* driver;
    sql::Connection* con;

    try {
        driver = get_driver_instance();
        con = driver->connect(server, username, password);
        con->setSchema(schemaName);
    }
    catch (sql::SQLException e) {
        cout << "Could not connect to server. Error message: " << e.what() << endl;
        system("pause");
        exit(1);
    }
    
    // Update the policyStatus as "cancelled"
    stmt = con->createStatement();
    std::string updateQuery = "UPDATE policies SET policyStatus = 'cancelled' WHERE policy_id = " + std::to_string(policy_id);

    int rowsAffected = stmt->executeUpdate(updateQuery);

    if (rowsAffected > 0) {
        std::cout << "Proposal status updated for proposal ID: " << policy_id << std::endl;
    }
    else {
        std::cout << "No rows updated for proposal ID: " << policy_id << std::endl;
    }


    // Clean up
    delete stmt;
    con->close();
    delete con;


}

// Approver Dashboard Functions


// Menus
void showMainMenu() {

    int ch;

    while (true) {

        std::cout << "\nCoreSuiteLite\n";
        std::cout << "----------------------------------\n";
        std::cout << "Menu:" << endl;
        std::cout << "1.  Agent Log in" << endl;
        std::cout << "2.  Agent Sign up" << endl;
        std::cout << "3.  Approver Log in" << endl;
        std::cout << "4.  Approver Sign up" << endl;
        std::cout << "5.  Exit" << endl;
        std::cout << "----------------------------------";
        std::cout << "\nEnter your choice: ";
        std::cin >> ch;
        std::cout << "\n";


        switch (ch) {

        case 1:
        {
            // Log in the Agent with the correct credentials
            // if true continue flow else exit the flow

            sql::Driver* driver;
            sql::Connection* con;

            try {
                driver = get_driver_instance();
                con = driver->connect(server, username, password);
            }
            catch (sql::SQLException e) {
                std::cout << "Could not connect to server. Error message: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            con->setSchema(schemaName);

           
            std::cout << "Enter username: ";
            std::cin >> inputUsername;
            std::cout << "Enter password: ";
            std::cin >> inputPassword;

            // Retrieve user's password from the database
            try {
                sql::PreparedStatement* pstmt;
                pstmt = con->prepareStatement("SELECT agent_ID,Password FROM agents WHERE userName = ?");
                pstmt->setString(1, inputUsername);
                sql::ResultSet* res = pstmt->executeQuery();

                if (res->next()) {
                    string PasswordFromDB = res->getString("Password");
                    int AgentID = res->getInt("agent_ID");

                    if (PasswordFromDB == inputPassword) {
                        std::cout << "Sign-in successful." << endl<<endl;
                        string UserName = inputUsername;
                        agentDashboardMenu(UserName,AgentID);
                    }
                    else {
                        std::cout << "Incorrect password." << endl;
                    }
                }
                else {
                    std::cout << "User not found." << endl;
                    showMainMenu();
                }

                delete res;
                delete pstmt;
            }
            catch (sql::SQLException e) {
                std::cout << "Error while querying database: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            delete con;
            std::cout << "Agent Logged in..." << endl;

            break;
        }

        case 2:
        {
            top_of_case:

            //Agent newAgent;
            std::cout << "Agent Signing up..." << endl<<endl;

            sql::Driver* driver;
            sql::Connection* con;

            try {
                driver = get_driver_instance();;
                con = driver->connect(server, username, password);
            }
            catch (sql::SQLException e) {
                std::cout << "Could not connect to server. Error message: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            // Set the schema
            con->setSchema(schemaName);

            // Rest of the code will go here
            string signupUsername, signupPassword, signupEmail,signupContact;
            string signupFname, signupLname;

            std::cout << "Enter First Name : " << endl; std::cin >> signupFname;
            std::cout << "Enter Last Name : " << endl; std::cin >> signupLname;

            std::cout << "Enter username for signup: ";
            std::cin >> signupUsername;

            // Check that it does not exist in our db //

            try {
                sql::PreparedStatement* pstmt;
                pstmt = con->prepareStatement("SELECT userName from agents");
                sql::ResultSet* res = pstmt->executeQuery();

                bool usernameExists = false;
                while (res->next()) {
                    if (signupUsername == res->getString("userName")) {
                        usernameExists = true;
                        break;
                    }
                }

                if (usernameExists) {
                    std::cout << "Username already exists." << endl;
                    goto top_of_case;

                }
                else {
                    std::cout << "Username is available." << endl;

                    std::cout << "Enter password for signup: ";
                    std::cin >> signupPassword;
                    std::cout << "Enter email for signup: ";
                    std::cin >> signupEmail;
                    std::cout << "Enter Contact Number for signup: ";
                    std::cin >> signupContact;


                    sql::PreparedStatement* pstmt;
                    pstmt = con->prepareStatement("INSERT INTO agents (userName, Password, fName, Lname, Email, contactNumber) VALUES (?, ?, ?, ?, ?, ?)");
                    pstmt->setString(1, signupUsername);
                    pstmt->setString(2, signupPassword);
                    pstmt->setString(3, signupFname);
                    pstmt->setString(4, signupLname);
                    pstmt->setString(5, signupEmail);
                    pstmt->setString(6, signupContact);

                    pstmt->execute();
                    std::cout << "\nUser signed up and details saved.\n" << endl;
                }
                delete res;
                delete pstmt;
            }
            catch (sql::SQLException e) {
                std::cout << "\nError while querying database: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            delete con;
            system("pause");
      
            break;
        }

        case 3:
            std::cout << "Approver Logging in..." << endl;

            sql::Driver* driver;
            sql::Connection* con;

            try {
                driver = get_driver_instance();
                con = driver->connect(server, username, password);
            }
            catch (sql::SQLException e) {
                std::cout << "Could not connect to server. Error message: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            con->setSchema(schemaName);

            
            std::cout << "Enter username: ";
            std::cin >> inputApproverUsername;
            std::cout << "Enter password: ";
            std::cin >> inputApproverPassword;

            // Retrieve user's password from the database
            try {
                sql::PreparedStatement* pstmt;
                pstmt = con->prepareStatement("SELECT approverID,Password FROM approvers WHERE approverUserName = ?");
                pstmt->setString(1, inputApproverUsername);
                sql::ResultSet* res = pstmt->executeQuery();

                if (res->next()) {
                    string PasswordFromDB = res->getString("Password");
                    int ApproverID = res->getInt("approverID");

                    if (PasswordFromDB == inputApproverPassword) {
                        std::cout << "Sign-in successful." << endl << endl;
                        string approverUserName = inputApproverUsername;
                        approverDashboardMenu(approverUserName, ApproverID);
                    }
                    else {
                        std::cout << "Incorrect password." << endl;
                    }
                }
                else {
                    std::cout << "User not found." << endl;
                    showMainMenu();
                }

                delete res;
                delete pstmt;
            }
            catch (sql::SQLException e) {
                std::cout << "Error while querying database: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            delete con;
            std::cout << "Approver Logged in..." << endl;


            // send notif

            break;

        case 4:
        {   
       
            std::cout << "Approver Signing up..." << endl;

            sql::Driver* driver;
            sql::Connection* con;

            try {
                driver = get_driver_instance();;
                con = driver->connect(server, username, password);
            }
            catch (sql::SQLException e) {
                std::cout << "Could not connect to server. Error message: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            // Set the schema
            con->setSchema(schemaName);

            // Rest of the code will go here
            string signupApproverUsername, signupApproverPassword;


            std::cout << "Enter username for signup: ";
            std::cin >> signupApproverUsername;

            // Check that it does not exist in our db //

            try {
                sql::PreparedStatement* pstmt;
                pstmt = con->prepareStatement("SELECT approverUserName from approvers");
                sql::ResultSet* res = pstmt->executeQuery();

                bool usernameExists = false;
                while (res->next()) {
                    if (signupApproverUsername == res->getString("approverUserName")) {
                        usernameExists = true;
                        break;
                    }
                }

                if (usernameExists) {
                    std::cout << "Username already exists." << endl;

                }
                else {
                    std::cout << "Username is available." << endl;

                    std::cout << "Enter password for signup: ";
                    std::cin >> signupApproverPassword;
                 
                    sql::PreparedStatement* pstmt;
                    pstmt = con->prepareStatement("INSERT INTO approvers (approverUserName, Password) VALUES (?, ?)");
                    pstmt->setString(1, signupApproverUsername);
                    pstmt->setString(2, signupApproverPassword);
                    

                    pstmt->execute();
                    std::cout << "\nApprover signed up and details saved.\n\n" << endl;
                }
                delete res;
                delete pstmt;
            }
            catch (sql::SQLException e) {
                std::cout << "Error while querying database: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            delete con;
            system("pause");

            break;
        }
        case 5:
        {
            std::cout << "Exiting..." << endl;
            exit(0);
            break;
        }
        default:
            std::cout << "Invalid choice. Please enter a valid option." << endl;
        }
    }
}
void agentDashboardMenu(string agent_name, int agent_id) {

    while (true) {
        int choice;

        std::cout << "\nWelcome : " <<agent_name<<"(Agent) ("<<agent_id<<")"<<endl<<endl;

        std::cout << "Menu:" << endl;
        std::cout << "1. Create a new proposal" << endl;
        std::cout << "2. Add a new Client" << endl;
        std::cout << "3. View all Clients" << endl;
        std::cout << "4. View all Proposals" << endl;
        std::cout << "5. Send a proposal for approval" << endl;
        std::cout << "6. Cancel an existing policy" << endl;
        std::cout << "7. Back" << endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;


        switch (choice) {
        case 1: {
            // Create a new proposal
            string agentName = agent_name;
            string policyType;

            createNewProposal(agentName, agent_id, policyType);
            break;
        }

        case 2: {
            // Just add a new client
            addNewClient(agent_id);
            break;
        }
              
        case 3: {
            // View all clients
            viewAllClients(agent_id);
            break;
        }
        
        case 4: {
            // View all Proposals
            viewAllProposals(agent_name);
            break;
        }

        case 5: {
            // Send a proposal for approval
            int proposalIDforApproval;
            cout << "\n\nEnter a proposal ID: "; cin >> proposalIDforApproval;

            sendProposalForApproval(proposalIDforApproval);

            break;
        }
        
        case 6: {
            // Cancel an existing policy
            int policyIdtoBeCancelled;
            cout << "\nEnter the policy id that needs to be cancelled: " << endl;
            cin >> policyIdtoBeCancelled;
            cancelExistingPolicy(policyIdtoBeCancelled);
            break;
        }

        case 7: {
            std::cout << "Going Back..." << endl;
            showMainMenu();
            break;
        }
            
        default:
            std::cout << "Invalid choice. Please enter a valid option." << endl;
        }
    }

}
void approverDashboardMenu(string approver_name, int approver_id) {

    std::cout << "\nWelcome : " << approver_name << "(Approver) (" << approver_id << ")" << endl << endl;

    int choice;
    bool exitLoop = false;

    do {
        std::cout << "Choose an option:" << std::endl;
        std::cout << "1. View all pending policies" << std::endl;
        std::cout << "2. Accept or reject policy" << std::endl;
        std::cout << "3. Export a Policy" << std::endl;
        std::cout << "4. Go back" << std::endl;
        std::cout << "Enter your choice (1-4): ";
        std::cin >> choice;

        switch (choice) {
        case 1: {
            // View all pending policies logic
            std::cout << "\nPending Policies: \n\n" << std::endl;

            sql::Driver* driver;
            sql::Connection* con;

            try {
                driver = get_driver_instance();
                con = driver->connect(server, username, password);
                con->setSchema(schemaName);
            }
            catch (sql::SQLException e) {
                cout << "Could not connect to server. Error message: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            stmt = con->createStatement();
            selectQuery = "SELECT * FROM policies WHERE policyStatus = 'pending'";
            res = stmt->executeQuery(selectQuery);

            while (res->next()) {
                int policyID = res->getInt("policy_id");
                std::string proposalID = res->getString("proposal_id");
                int clientID = res->getInt("client_id");
                std::string firstName = res->getString("firstName");

                std::cout << "Policy ID : " << policyID << ", Proposal ID : " << proposalID << ", Client ID : " << clientID << ", First Name : " << firstName << endl;
                std::cout << "-------------------------------------------------------";
            }

            cout << "\n";
            break;
        }

        case 2: {
            // Accept or reject policy logic
            sql::Driver* driver;
            sql::Connection* con;

            try {
                driver = get_driver_instance();
                con = driver->connect(server, username, password);
                con->setSchema(schemaName);
            }
            catch (sql::SQLException e) {
                cout << "Could not connect to server. Error message: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            int policyIDforStatusChange;
            string newPolicyStatus;

            cout << "\nEnter Policy ID: " <<endl; cin >> policyIDforStatusChange;
            cout << "\nEnter whether to change status to (approved,cancelled)"; cin >> newPolicyStatus;

            // Update the policyStatus in the policies table

            std::string updateQuery = "UPDATE policies SET policyStatus = ? WHERE policy_id = ?";
            sql::PreparedStatement* updateStmt = con->prepareStatement(updateQuery);
            
            updateStmt->setString(1, newPolicyStatus);
            updateStmt->setInt(2, policyIDforStatusChange);
            updateStmt->executeUpdate();

            std::cout << "Policy " << policyIDforStatusChange << " has been marked as " << newPolicyStatus << "." << std::endl;

            // Clean up
            delete updateStmt;
            con->close();
            delete con;



            break;
        }

        case 3: {
            // Export a policy
            sql::Driver* driver;
            sql::Connection* con;

            try {
                driver = get_driver_instance();
                con = driver->connect(server, username, password);
                con->setSchema(schemaName);
            }
            catch (sql::SQLException e) {
                cout << "Could not connect to server. Error message: " << e.what() << endl;
                system("pause");
                exit(1);
            }

            //Take input for generating a policy txt
            int policyID;
            std::cout << "Enter the Policy ID: ";
            std::cin >> policyID;

            // Retrieve policy details from the POLICIES table
            std::string selectQuery = "SELECT * FROM POLICIES WHERE policy_id = ?";
            sql::PreparedStatement* selectStmt = con->prepareStatement(selectQuery);
            selectStmt->setInt(1, policyID);
            sql::ResultSet* resultSet = selectStmt->executeQuery();

            // Check if the policy exists
            if (resultSet->next()) {
                // Get column names from the result set's metadata
                sql::ResultSetMetaData* metaData = resultSet->getMetaData();
                int numColumns = metaData->getColumnCount();

                // Create and write to a text file
                std::ofstream outputFile("policy_details.txt");
                // Loop through columns, retrieve names and values, and write to file
                for (int columnIndex = 1; columnIndex <= numColumns; ++columnIndex) {
                    std::string columnName = metaData->getColumnName(columnIndex);
                    std::string columnValue = resultSet->getString(columnIndex);
                    outputFile << columnName << ": " << columnValue << "\n";
                }
                outputFile.close();

                std::cout << "\nPolicy details written to policy_details.txt\n\n" << std::endl;
            }
            else {
                std::cout << "\nPolicy not found.\n\n" << std::endl;
            }

            // Clean up
            delete resultSet;
            delete selectStmt;
            con->close();
            delete con;

            
            break;
        }

        case 4: {
            exitLoop = true;
            std::cout << "Going back..." << std::endl;
            break;
        }

        default: {
            std::cout << "Invalid choice. Please enter a valid option (1-4)." << std::endl;
            break;
        }
        }

    } while (!exitLoop);
}


int main() {
    showMainMenu();
    return 0;
}
