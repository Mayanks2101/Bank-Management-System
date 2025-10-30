#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

#include "Structures/structures.h"

#define CUSPATH "db/customers"
#define EMPPATH "db/employees"
#define FEEDPATH "db/feedbacks"
#define LOANPATH "db/loanApplications"
#define HISTORYPATH "db/transactions"
#define COUNTERPATH "db/counters"

void printCustomers(int fd);
void printEmployees(int fd);
void printLoanDetails(int fd);
void printTransactionHistory(int fd);
void printFeedbacks(int fd);
void printCounterDB();

int main()
{
    int fd1 = open(CUSPATH, O_RDONLY);
    int fd2 = open(EMPPATH, O_RDONLY);
    int fd3 = open(LOANPATH, O_RDONLY);
    int fd4 = open(HISTORYPATH, O_RDONLY);
    int fd5 = open(FEEDPATH, O_RDONLY);

    struct Customer temp;
    struct Employee temp1;
    struct LoanApplication temp2;
    struct TransactionHistory th;
    struct FeedBack fb;

    #define options "1. View Customers\n2. View Employees\n3. View Loan Details\n4. View Transaction History\n5. View Feedbacks\n6. View CounterDB\nEnter your choice: "

    while(1){
        int choice;
        printf("%s", options);
        scanf("%d", &choice);

        switch(choice){
            case 1:
                printCustomers(fd1);
                break;
            case 2:
                printEmployees(fd2);
                break;
            case 3:
                printLoanDetails(fd3);
                break;
            case 4:
                printTransactionHistory(fd4);
                break;
            case 5:
                printFeedbacks(fd5);
                break;
            case 6:
                printCounterDB();
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}


void printCustomers(int fd)
{
    printf("================ Customer ==================\n");
    struct Customer temp;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, &temp, sizeof(temp)) > 0)
    {
        printf("Cust ID: %d\n", temp.custId);
        printf("Password: %s\n", temp.password);
        printf("Password length: %lu\n", strlen(temp.password));
        printf("Account Number: %d\n", temp.accountNumber);
        printf("Customer Name: %s\n", temp.custName);
        printf("Balance: %.2f\n", temp.balance);
        printf("Active Status: %d\n", temp.activeStatus);
        printf("=======================\n");
    }
}

void printEmployees(int fd)
{
    printf("================ Employees ==================\n");
    struct Employee temp1;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, &temp1, sizeof(temp1)) > 0)
    {
        printf("Employee ID: %d\n", temp1.empID);
        printf("Password: %s\n", temp1.password);
        printf("Employee Name: %s\n", temp1.empName);
        printf("Role: %d\n", temp1.role);
        printf("=======================\n");
    }
}

void printLoanDetails(int fd)
{
    printf("================ Loan Details ==================\n");
    struct LoanApplication temp2;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, &temp2, sizeof(temp2)) > 0)
    {
        printf("Employee ID: %d\n", temp2.empID);
        printf("Account Number: %d\n", temp2.accountNumber);
        printf("Loan ID: %d\n", temp2.loanID);
        printf("Loan Amount: %d\n", temp2.loanAmount);
        printf("Status: %d\n", temp2.status);
        printf("=======================\n");
    }
}

void printTransactionHistory(int fd)
{
    printf("================ Transaction History ==================\n");
    struct TransactionHistory th;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, &th, sizeof(th)) > 0)
    {
        printf("Account Number: %d\n", th.acc_no);
        printf("History: %s\n", th.history);
        printf("=======================\n");
    }
}

void printFeedbacks(int fd)
{
    printf("================ Feedback ==================\n");
    struct FeedBack fb;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, &fb, sizeof(fb)) > 0)
    {
        printf("Customer ID: %d\n", fb.custId);
        printf("Feedback: %s\n", fb.feedback);
        printf("=======================\n");
    }
}

void printCounterDB()
{
    int fd = open(COUNTERPATH, O_RDONLY);
    if(fd < 0){
        perror("Failed to open counter database");
        return;
    }

    struct Counter counter;
    if(read(fd, &counter, sizeof(counter)) != sizeof(counter)){
        perror("Failed to read counter database");
        close(fd);
        return;
    }

    printf("================ Counter Database ==================\n");
    printf("Last Customer ID: %d\n", counter.lastCustId);
    printf("Last Employee ID: %d\n", counter.lastEmpId);
    printf("Last Account Number: %d\n", counter.lastAccNo);
    printf("Last Loan ID: %d\n", counter.lastLoanId);
    printf("Last Transaction ID: %d\n", counter.lastTxnId);
    printf("====================================================\n");

    close(fd);
}