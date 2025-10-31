#define TRANS_HEADER "TxnID    | Timestamp           | Type       | Amount   | Balance  | Remarks\n" \
                     "--------------------------------------------------------------------------\n"

int add_transaction_header(int nsd);
int add_new_customer(int nsd);
int add_transaction_entry(int acc_no, int txnID, const char* txnType, float amount, float remainingBalance, const char* remarks);
int approve_reject_loans(int nsd, int empID);
int view_assigned_loan_applications(int nsd, int empID);
int view_customer_transactions(int nsd);

int apply_loan_handler(int nsd, int acc_no);
int view_TransactionHistory(int nsd, int acc_no);
int addFeedback_handler(int nsd, int custId);
struct Employee authenticate_employee(int nsd);


void employee_handler(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    struct Employee emp;

    emp = authenticate_employee(nsd);
    if(emp.empID == -1){
        // strcpy(writeBuffer, "Authentication Failed. Disconnecting...\n");
        // writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        // if(writeBytes < 0){
        //     perror("Write to client failed");
        // }
        // readBytes = read(nsd, readBuffer, sizeof(readBuffer));
        return;
    }

    printf("Employee %s (ID: %d) authenticated successfully.\n", emp.empName, emp.empID);

    int choice;
    while(1){
        // Send Employee Menu
        memset(writeBuffer, 0, BUFF_SIZE);

        strcpy(writeBuffer, EMP_MENU);
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return;
        }
        // Read Employee Choice
        memset(readBuffer, 0, BUFF_SIZE);
        readBytes = read(nsd, readBuffer, sizeof(readBuffer));
        if(readBytes < 0){
            perror("Read from client failed");
            return;
        }
        readBuffer[readBytes] = '\0'; // Null-terminate
        
        readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
        if(strlen(readBuffer) == 0){
            return;
        }
        
        choice = atoi(readBuffer);

        memset(writeBuffer, 0, BUFF_SIZE);
        switch(choice){
            case 1:
                // Add New Customer
                if(add_new_customer(nsd) == 0){
                    strcpy(writeBuffer, "Failed to add New Customer. \nPress 1 to continue..\n");
                }
                break;

            case 2:
                // Modify Customer Details
                if(modify_customer_details(nsd) == 0){
                    strcpy(writeBuffer, "Failed to modify Customer details. \nPress 1 to continue..\n");
                }
                break;

            case 3:
                // Approve/Reject Loans
                if(approve_reject_loans(nsd, emp.empID) == 0){
                    strcpy(writeBuffer, "Failed to process Loan Application. \nPress 1 to continue..\n");
                }
                break;

            case 4:
                // View Assigned Loan Applications
                if(view_assigned_loan_applications(nsd, emp.empID) == 0){
                    strcpy(writeBuffer, "Failed to retrieve Assigned Loan Applications. Please \nPress 1 to continue..\n");
                }
                break;

            case 5:
                // View Customer Transactions
                if(view_customer_transactions(nsd) == 0){
                    strcpy(writeBuffer, "Failed to retrieve Customer Transactions. Please \nPress 1 to continue..\n");
                }
                break;

            case 6:
                // Change Password
                if(change_password(nsd, "EMPLOYEE", emp.empID)){
                    strcpy(writeBuffer, "Password changed successfully.\n");
                }
                else {
                    strcpy(writeBuffer, "Failed to change password. Please \nPress 1 to continue..\n");
                }
                break;

            case 7:
                // Logout
                log_out(nsd, "EMPLOYEE", emp.empID);
                return;

            case 8:
                // Exit
                exit_client(nsd, "EMPLOYEE", emp.empID);
                return;

            default:
                strcpy(writeBuffer, "Invalid choice. \nPress 1 to continue..\n");
        }
        
        if(strlen(writeBuffer) > 0){
            writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0){
                perror("Write to client failed");
                return;
            }
            readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
        }

    }
}

struct Employee authenticate_employee(int nsd)
{
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;
    struct Employee emp;
    emp.empID = -1; // Default to invalid

    int auth = 0;

    // Authentication
    // while(!auth){
    memset(writeBuffer, 0, BUFF_SIZE);
    strcpy(writeBuffer, "Enter Employee ID: ");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return emp;
        }

        // Read Employee ID
        memset(readBuffer, 0, BUFF_SIZE);
        readBytes = read(nsd, readBuffer, sizeof(readBuffer));
        if(readBytes < 0){
            perror("Read from client failed");
            return emp;
        }
        readBuffer[readBytes] = '\0'; // Null-terminate
        
        readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
        if(strlen(readBuffer) == 0){
            printf("Read buffer empty\n");
            return emp;
        }

        int empId = atoi(readBuffer);

        // Prompt for Employee Password
        memset(writeBuffer, 0, BUFF_SIZE);
        strcpy(writeBuffer, "Enter Employee Password: ");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return emp;
        }

        // Read Employee Password
        memset(readBuffer, 0, BUFF_SIZE);
        readBytes = read(nsd, readBuffer, sizeof(readBuffer));
        if(readBytes < 0){
            perror("Read from client failed");
            return emp;
        }
        readBuffer[readBytes] = '\0'; // Null-terminate
        
        readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
        if(strlen(readBuffer) == 0){
            return emp;
        }

        // Validate Credentials
        int fd = open(EMPLOYEES_DB, O_RDONLY);
        if(fd < 0){
            perror("Failed to open employee database");
            return emp;
        }

        struct Employee tempEmp;
        int found = 0;
        lockFile(fd, F_RDLCK, 0, 0);
        while(read(fd, &tempEmp, sizeof(tempEmp)) == sizeof(tempEmp)){
            if(tempEmp.empID == empId && strcmp(tempEmp.password, readBuffer) == 0 && tempEmp.role == 1){
                if(tempEmp.isLoggedIn){
                    // Already logged in
                    strcpy(writeBuffer, "This Employee is already logged in from another session. \nPress 1 to continue..\n");
                    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
                    if(writeBytes < 0){
                        perror("Write to client failed");
                        close(fd);
                        return emp;
                    }
                    readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
                    unlockFile(fd, 0, 0);
                    close(fd);
                    // read(nsd, readBuffer, sizeof(readBuffer)); // Wait for Enter
                    return emp;
                }
                emp = tempEmp;
                found = 1;
                break;
            }
        }
        unlockFile(fd, 0, 0);
        close(fd);

        if(!found){
            strcpy(writeBuffer, "Invalid Employee ID or Password. \nPress 1 to continue..\n");
            writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0){
                perror("Write to client failed");
                return emp;
            }
            readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
            return emp;
        }

        //Mark Employee as Logged In
        fd = open(EMPLOYEES_DB, O_RDWR);
        if(fd < 0){
            perror("Failed to open employee database");
            return emp;
        }

        // Mark Employee as Logged In
        off_t offset = lseek(fd, 0, SEEK_SET);
        lockFile(fd, F_WRLCK, 0, 0);

        while(read(fd, &tempEmp, sizeof(tempEmp)) == sizeof(tempEmp)){
            if(tempEmp.empID == emp.empID){
                tempEmp.isLoggedIn = 1; // Mark as logged in
                emp = tempEmp;
                // Move file pointer back to overwrite
                lseek(fd, - sizeof(tempEmp), SEEK_CUR);
                write(fd, &tempEmp, sizeof(tempEmp));
                break;
            }
        }
        unlockFile(fd, offset, sizeof(emp));
        close(fd);

    //     auth = 1; // Successful authentication
    // }
    return emp;
}

int add_new_customer(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    struct Customer newCust;

    // Get Customer Name
    strcpy(writeBuffer, "Enter New Customer Name: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    memset(readBuffer, 0, BUFF_SIZE);
    readBytes = read(nsd, readBuffer, sizeof(readBuffer));
    if(readBytes < 0){
        perror("Read from client failed");
        return 0;
    }
    readBuffer[readBytes] = '\0'; // Null-terminate
        
    readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
    if(strlen(readBuffer) == 0){
        return 0;
    }

    strncpy(newCust.custName, readBuffer, sizeof(newCust.custName) - 1);
    newCust.custName[sizeof(newCust.custName) - 1] = '\0'; // Ensure null-termination 

    // Get Customer Password
    strcpy(writeBuffer, "Enter New Customer Password: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    memset(readBuffer, 0, BUFF_SIZE);
    readBytes = read(nsd, readBuffer, sizeof(readBuffer));
    if(readBytes < 0){
        perror("Read from client failed");
        return 0;
    }   
    readBuffer[readBytes] = '\0'; // Null-terminate
        
    readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
    if(strlen(readBuffer) == 0){
        return 0;
    }

    strncpy(newCust.password, readBuffer, sizeof(newCust.password) - 1);
    newCust.password[sizeof(newCust.password) - 1] = '\0'; // Ensure null-termination

    newCust.activeStatus = 1; // Default status as Active
    newCust.balance = 0.0f; // Initial balance
    newCust.isLoggedIn = 0; // Default to logged out

    int custId = getNextCounterValue("custId");
    int accNo = getNextCounterValue("accNo");
    newCust.custId = custId;
    newCust.accountNumber = accNo;

    int fd = open(CUSTOMERS_DB, O_APPEND | O_WRONLY | O_CREAT, 0644);

    if(fd < 0){
        perror("Failed to open customer database");
        return 0;
    }
    lseek(fd, 0, SEEK_END);
    lockFile(fd, F_WRLCK, 0, 0);
    if(write(fd, &newCust, sizeof(newCust)) != sizeof(newCust)){
        perror("Failed to add new customer");
        close(fd);
        return 0;
    }
    unlockFile(fd, 0, 0);

    add_transaction_header(newCust.accountNumber);

    close(fd);

    sprintf(writeBuffer, "New Customer added successfully. Customer ID is %d, Account Number is %d \nPress 1 to continue..\n", custId, accNo);
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
    }
    readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry

    return 1;
}

int add_transaction_header(int accNo){
    int fd = open(TRANSACTIONS_DB, O_APPEND | O_WRONLY | O_CREAT, 0644);
    if(fd < 0){
        perror("Failed to open transactions database");
        return 0;
    }

    struct TransactionHistory accounts;
    accounts.acc_no = accNo;
    strcpy(accounts.history, TRANS_HEADER);
    lseek(fd, 0, SEEK_END);
    lockFile(fd, F_WRLCK, 0, 0);
    if(write(fd, &accounts, sizeof(accounts)) != sizeof(accounts)){
        perror("Failed to add transaction header");
        close(fd);
        return 0;
    }
    unlockFile(fd, 0, 0);

    close(fd);
    return 1;
}

int approve_reject_loans(int nsd, int empId){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Open Loan Applications Database
    int fdLoans = open(LOANAPPLICATIONS_DB, O_RDWR);
    if(fdLoans < 0){
        perror("Failed to open loan applications database");
        return 0;
    }

    struct LoanApplication loanApp;
    char pendingLoanIds[1024] = {0}; // Store pending loan IDs

    // Collect Pending Loan Applications
    lockFile(fdLoans, F_RDLCK, 0, 0);
    while(read(fdLoans, &loanApp, sizeof(loanApp)) == sizeof(loanApp)){
        if(loanApp.empID == empId && loanApp.status == 1){
            char loanIdEntry[1024];
            sprintf(loanIdEntry, "Loan ID - %d, Account Number - %d, Loan Amount - %.2f\n", loanApp.loanID, loanApp.accountNumber, (float)loanApp.loanAmount);
            strcat(pendingLoanIds, loanIdEntry);
        }
    }
    unlockFile(fdLoans, 0, 0);


    if(strlen(pendingLoanIds) == 0){
        strcpy(writeBuffer, "No pending loan applications assigned to you.\nPress 1 to continue..\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
        close(fdLoans);
        return 2;
    }

    // Send Pending Loan Applications to Employee
    strcpy(writeBuffer, "Pending Loan Applications assigned to you are - \n");
    strcat(writeBuffer, pendingLoanIds);
    strcat(writeBuffer, "Enter Loan ID to Approve/Reject: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        close(fdLoans);
        return 0;
    }

    // Read Loan ID to Process
    memset(readBuffer, 0, BUFF_SIZE);
    readBytes = read(nsd, readBuffer, sizeof(readBuffer));
    if(readBytes < 0){
        perror("Read from client failed");
        close(fdLoans);
        return 0;
    }
    readBuffer[readBytes] = '\0'; // Null-terminate
        
    readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
    if(strlen(readBuffer) == 0){
        return 0;
    }

    int loanId = atoi(readBuffer);
    if(loanId <= 0){
        strcpy(writeBuffer, "Invalid Loan ID. Operation cancelled. \nPress 1 to continue..\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
        close(fdLoans);
        return 2;
    }
    // Reset file pointer to beginning
    lseek(fdLoans, 0, SEEK_SET);

    int found = 0;
    // Process the specified Loan Application
    while(read(fdLoans, &loanApp, sizeof(loanApp)) == sizeof(loanApp)){
        if(loanApp.loanID == loanId && loanApp.empID == empId && loanApp.status == 1){
            found = 1;
            memset(writeBuffer, 0, BUFF_SIZE);
            // Prompt for Approval/Rejection
            strcpy(writeBuffer, "\n1 .Approve\n2. Reject the loan\n3. Go Back to Main Menu\nEnter your choice: ");
            writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0){
                perror("Write to client failed");
                close(fdLoans);
                return 0;
            }

            // Read Decision
            memset(readBuffer, 0, BUFF_SIZE);
            readBytes = read(nsd, readBuffer, sizeof(readBuffer));
            if(readBytes < 0){
                perror("Read from client failed");
                close(fdLoans);
                return 0;
            }
            readBuffer[readBytes] = '\0'; // Null-terminate
        
            readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
            if(strlen(readBuffer) == 0){
                return 0;
            }

            int decision = atoi(readBuffer);

            off_t offset = lseek(fdLoans, -sizeof(loanApp), SEEK_CUR);
            lockFile(fdLoans, F_WRLCK, offset, sizeof(loanApp));

            // Update Loan Application Status
            if(decision == 1){
                loanApp.status = 2; // Approved

            }
            else if(decision == 2){
                loanApp.status = 3; // Rejected
            }
            else{
                memset(writeBuffer, 0, BUFF_SIZE);
                strcpy(writeBuffer, "Invalid choice. Operation cancelled. \nPress 1 to continue..\n");
                writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
                if(writeBytes < 0){
                    perror("Write to client failed");
                }
                readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
                unlockFile(fdLoans, 0, 0);
                close(fdLoans);
                return 2;
            }

            write(fdLoans, &loanApp, sizeof(loanApp));
            unlockFile(fdLoans, offset, sizeof(loanApp));
            
            if(decision == 1){
                // If approved, update customer balance
                int fdCust = open(CUSTOMERS_DB, O_RDWR);
                if(fdCust < 0){
                    perror("Failed to open customer database");
                    close(fdLoans);
                    return 0;
                }

                struct Customer cust;
                // Search for Customer by Account Number
                while(read(fdCust, &cust, sizeof(cust)) == sizeof(cust)){
                    if(cust.accountNumber == loanApp.accountNumber){
                        off_t offset = lseek(fdCust, -sizeof(cust), SEEK_CUR);
                        lockFile(fdCust, F_WRLCK, offset, sizeof(cust));

                        cust.balance += loanApp.loanAmount;

                        // Move file pointer back to overwrite
                        write(fdCust, &cust, sizeof(cust));
                        unlockFile(fdCust, offset, sizeof(cust));

                        add_transaction_entry(cust.accountNumber, getNextCounterValue("txnId"), "DEPOSIT", loanApp.loanAmount, cust.balance, "Loan Approved ");

                        break;
                    }
                }
                close(fdCust);
            }

            close(fdLoans);

            memset(writeBuffer, 0, BUFF_SIZE);
            strcpy(writeBuffer, "Loan application processed successfully.\nPress 1 to continue..\n");
            writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0){
                perror("Write to client failed");
            }
            readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
            
            return 1;
        }
    }

    close(fdLoans);
    if(!found){
        memset(writeBuffer, 0, BUFF_SIZE);
        strcpy(writeBuffer, "No matching pending loan application found. Operation cancelled. \nPress 1 to continue..\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
        return 2;
    }

    return 1;
}

int view_assigned_loan_applications(int nsd, int empId){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Open Loan Applications Database
    int fdLoans = open(LOANAPPLICATIONS_DB, O_RDONLY);
    if(fdLoans < 0){
        perror("Failed to open loan applications database");
        return 0;
    }

    struct LoanApplication loanApp;
    char assignedLoanIds[1024] = {0}; // Store assigned loan IDs

    // Collect Assigned Loan Applications
    lockFile(fdLoans, F_RDLCK, 0, 0);
    while(read(fdLoans, &loanApp, sizeof(loanApp)) == sizeof(loanApp)){
        if(loanApp.empID == empId){
            char loanIdEntry[128];
            
            //snprintf has larger buffer than sprintf to avoid overflow
            snprintf(loanIdEntry, sizeof(loanIdEntry), "Loan ID - %d, Account Number - %d, Loan Amount - %.2f, Status - %s\n", 
                    loanApp.loanID, 
                    loanApp.accountNumber, 
                    (float)loanApp.loanAmount,
                    (loanApp.status == 1) ? "Pending" : (loanApp.status == 2) ? "Approved" : "Rejected");
            strcat(assignedLoanIds, loanIdEntry);
        }
    }
    unlockFile(fdLoans, 0, 0);
    close(fdLoans);

    if(strlen(assignedLoanIds) == 0){
        strcpy(writeBuffer, "No loan applications assigned to you.\nPress 1 to continue..\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
        return 2;
    }

    // Send Assigned Loan Applications to Employee
    strcpy(writeBuffer, "All Loan Applications assigned to you are - \n");
    strcat(writeBuffer, assignedLoanIds);
    strcat(writeBuffer, "\nPress 1 to continue..\n");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }
    readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
    
    return 1;
}

int view_customer_transactions(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    //Enter Account Number of Customer whose transactions are to be viewed
    strcpy(writeBuffer, "Enter Customer Account Number to view transactions: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    // Read Customer Account Number
    memset(readBuffer, 0, BUFF_SIZE);
    readBytes = read(nsd, readBuffer, sizeof(readBuffer));
    if(readBytes < 0){
        perror("Read from client failed");
        return 0;
    }
    readBuffer[readBytes] = '\0'; // Null-terminate
        
    readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
    if(strlen(readBuffer) == 0){
        return 0;
    }

    int custAccNo = atoi(readBuffer);

    // Open Transaction History Database
    int fdTrans = open(TRANSACTIONS_DB, O_RDONLY);
    if(fdTrans < 0){
        perror("Failed to open transaction history database");
        return 0;
    }

    struct TransactionHistory transHist;
    char allTransactions[4096] = "===== Customer Transactions =====\n";

    // Read all transaction histories
    int found = 0;
    lockFile(fdTrans, F_RDLCK, 0, 0);
    while(read(fdTrans, &transHist, sizeof(transHist)) == sizeof(transHist)){
        if(transHist.acc_no == custAccNo){
            strcat(allTransactions, transHist.history);
            found = 1;
        }
    }
    strcat(allTransactions, "=================================\nPress 1 to continue..\n");

    unlockFile(fdTrans, 0, 0);

    if(!found){
        strcat(allTransactions, "No transactions found for this account number.\nPress 1 to continue..\n");
    }

    // Send All Transactions to Employee
    writeBytes = write(nsd, allTransactions, strlen(allTransactions));
    if(writeBytes < 0){
        perror("Write to client failed");
        close(fdTrans);
        return 0;
    }
    readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
    close(fdTrans);
    return 1;
}

int modify_customer_details(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Get Customer ID to Modify
    strcpy(writeBuffer, "Enter Customer ID to Modify: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    memset(readBuffer, 0, BUFF_SIZE);
    readBytes = read(nsd, readBuffer, sizeof(readBuffer));
    if(readBytes < 0){
        perror("Read from client failed");
        return 0;
    }
    readBuffer[readBytes] = '\0'; // Null-terminate
        
    readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
    if(strlen(readBuffer) == 0){
        return 0;
    }

    int custId = atoi(readBuffer);

    // Open Customer Database
    int fd = open(CUSTOMERS_DB, O_RDWR);
    if(fd < 0){
        perror("Failed to open customer database");
        return 0;
    }

    struct Customer cust;
    int found = 0;

    // Search for Customer
    while(read(fd, &cust, sizeof(cust)) == sizeof(cust)){
        if(cust.custId == custId){
            found = 1;

            off_t offset = lseek(fd, - sizeof(cust), SEEK_CUR);
            lockFile(fd, F_WRLCK, offset, sizeof(cust));

            // Print Current Details
            sprintf(writeBuffer, "Current Customer Name - %s\nEnter New Customer Name - ", cust.custName);
            write(nsd, writeBuffer, strlen(writeBuffer));

            // Read New Customer Name
            memset(readBuffer, 0, BUFF_SIZE);
            readBytes = read(nsd, readBuffer, sizeof(readBuffer));
            if(readBytes < 0){
                perror("Read from client failed");
                close(fd);
                return 0;
            }
            readBuffer[readBytes] = '\0'; // Null-terminate
        
            readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
            if(strlen(readBuffer) == 0){
                return 0;
            }
        
            strncpy(cust.custName, readBuffer, strlen(cust.custName));

            // Move file pointer back to overwrite
            lseek(fd, offset, SEEK_SET);
            write(fd, &cust, sizeof(cust));

            unlockFile(fd, offset, sizeof(cust));
            break;
        }
    }

    close(fd);

    if(!found){
        strcpy(writeBuffer, "Customer ID not found.\nPress 1 to continue..\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return 0;
        }
        readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
        return 2;
    }

    strcpy(writeBuffer, "Customer details updated successfully.\nPress 1 to continue..\n");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }
    readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry

    return 1;
}


