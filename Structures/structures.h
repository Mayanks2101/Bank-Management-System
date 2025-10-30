struct TransactionHistory
{
    int acc_no;
    char history[1024];
};

struct FeedBack
{
    int custId;
    char feedback[1024];
};

struct Employee{
    int empID;
    char empName[50];
    char password[256];
    int role; // 0 -> Manager, 1 -> Employee
};

struct LoanApplication {
    int empID;
    int accountNumber;
    int loanID;
    int loanAmount;
    int status; // 0 -> requested, 1 -> Assigned to Empl, 2 -> approved, 3 -> rejected
};

struct Customer {
    int custId;
    int accountNumber;
    float balance;
    char custName[20];
    char password[256];
    int activeStatus; // 0 -> deactivate, 1 -> activate
};

struct Counter {
    int lastCustId;
    int lastEmpId;
    int lastAccNo;
    int lastLoanId;
    int lastTxnId;
};