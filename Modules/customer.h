#define MAX_TRANS 1024


int deposit_handler(int nsd, struct Customer *cust);
int withdraw_handler(int nsd, struct Customer *cust);
int money_transfer_handler(int nsd, struct Customer *cust);
int add_transaction_entry(int acc_no, int txnID, const char* txnType, float amount, float remainingBalance, const char* remarks);
int view_TransactionHistory(int nsd, int acc_no);
int addFeedback_handler(int nsd, int custId);
int apply_loan_handler(int nsd, int accNo);
struct Customer authenticate_customer(int nsd);


void customer_handler(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    struct Customer cust;

    cust = authenticate_customer(nsd);
    if(cust.custId == -1){
        strcpy(writeBuffer, "Customer authentication failed.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return;
    }
    printf("Customer %s (ID: %d) authenticated successfully.\n", cust.custName, cust.custId);

    int choice;
    while(1){
        // Send Customer Menu
        memset(writeBuffer, 0, BUFF_SIZE);

        strcpy(writeBuffer, CUST_MENU);
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return;
        }

        // Read Customer Choice
        memset(readBuffer, 0, BUFF_SIZE);
        readBytes = read(nsd, readBuffer, sizeof(readBuffer));
        if(readBytes < 0){
            perror("Read from client failed");
            return;
        }
        readBuffer[readBytes] = '\0'; // Null-terminate
        
        readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
        if(strlen(readBuffer) == 0){
            continue;
        }

        choice = atoi(readBuffer);

        memset(writeBuffer, 0, BUFF_SIZE);
        switch(choice){
            case 1:
                // View Balance
                sprintf(writeBuffer, "Current Balance is %.2f.\n", cust.balance);
                break;

            case 2:
                // Deposit
                if(deposit_handler(nsd, &cust) == 0){
                    strcpy(writeBuffer, "Deposit failed. Please try again.\n");
                }
                break;

            case 3:
                // Withdraw
                if(withdraw_handler(nsd, &cust) == 0){
                    strcpy(writeBuffer, "Withdraw failed. Please try again.\n");
                }
                break;

            case 4:
                // Money Transfer
                if(money_transfer_handler(nsd, &cust) == 0){
                    strcpy(writeBuffer, "Money Transfer failed. Please try again.\n");
                }
                break;

            case 5:
                // Apply for a loan
                if(apply_loan_handler(nsd, cust.accountNumber) == 0){
                    strcpy(writeBuffer, "Failed to submit Loan Application. Please try again.\n");
                }
                break;

            case 6:
                // Change Password
                if(change_password(nsd, "CUSTOMER", cust.custId)){
                    strcpy(writeBuffer, "Password changed successfully.\n");
                }
                else {
                    strcpy(writeBuffer, "Failed to change password. Please try again.\n");
                }
                break;

            case 7:
                // Add Feedback
                if(addFeedback_handler(nsd, cust.custId) == 0){
                    strcpy(writeBuffer, "Failed to add feedback. Please try again.\n");
                }
                break;

            case 8:
                // View Transaction
                if(view_TransactionHistory(nsd, cust.accountNumber) == 0){
                    strcpy(writeBuffer, "Failed to retrieve transaction history. Please try again.\n");
                }
                break;

            case 9:
                // Logout
                log_out(nsd);
                return;

            case 10:
                // Exit
                exit_client(nsd);
                return;

            default:
                strcpy(writeBuffer, "Invalid choice. Try again.\n");
                break;
        }

        if(strlen(writeBuffer) > 0){
            writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0){
                perror("Write to client failed");
                return;
            }
        }
    }
}

struct Customer authenticate_customer(int nsd) 
{
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;
    struct Customer cust;
    cust.custId = -1; // Default to invalid

    int auth = 0;

    // Authentication
    while(!auth){
        strcpy(writeBuffer, "Enter Customer ID: ");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return cust;
        }

        // Read Customer ID
        memset(readBuffer, 0, BUFF_SIZE);
        readBytes = read(nsd, readBuffer, sizeof(readBuffer));
        if(readBytes < 0){
            perror("Read from client failed");
            return cust;
        }
        readBuffer[readBytes] = '\0'; // Null-terminate
        
        readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
        if(strlen(readBuffer) == 0){
            continue;
        }

        int custId = atoi(readBuffer);

        // Prompt for Customer Password
        strcpy(writeBuffer, "Enter Customer Password: ");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return cust;
        }

        // Read Customer Password
        memset(readBuffer, 0, BUFF_SIZE);
        readBytes = read(nsd, readBuffer, sizeof(readBuffer));
        if(readBytes < 0){
            perror("Read from client failed");
            return cust;
        }
        readBuffer[readBytes] = '\0'; // Null-terminate
        
        readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
        if(strlen(readBuffer) == 0){
            continue;
        }

        // Open Customer Database
        int fd = open(CUSTOMERS_DB, O_RDONLY);
        if(fd < 0){
            perror("Failed to open customer database");
            return cust;
        }

        struct Customer tempCust;
        int found = 0;

        lockFile(fd, F_RDLCK, 0, 0);
        // Search for Customer
        while(read(fd, &tempCust, sizeof(tempCust)) == sizeof(tempCust)){
            tempCust.password[sizeof(tempCust.password) - 1] = '\0'; // Ensure null-termination
            if(tempCust.custId == custId && strcmp(tempCust.password, readBuffer) == 0 && tempCust.activeStatus == 1){
                cust = tempCust;
                found = 1;
                break;
            }
        }
        unlockFile(fd, 0, 0);
        close(fd);

        if(!found){
            strcpy(writeBuffer, "Invalid Customer ID or Password or Inactive Account. Press Enter to try again\n");
            writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0){
                perror("Write to client failed");
                return cust;
            }

            readBytes = read(nsd, readBuffer, sizeof(readBuffer));
            continue;
        }
        auth = 1;

    }
    return cust;
}

int deposit_handler(int nsd, struct Customer *cust){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Prompt for Deposit Amount
    strcpy(writeBuffer, "Enter amount to deposit: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    // Read Deposit Amount
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

    float amount = atof(readBuffer);

    if(amount <= 0){
        strcpy(writeBuffer, "Invalid amount. Deposit failed.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return 2;
    }

    // Update Customer Balance
    cust->balance += amount;

    // Update Customer Record in Database
    int fd = open(CUSTOMERS_DB, O_RDWR);
    if(fd < 0){
        perror("Failed to open customer database");
        return 0;
    }

    struct Customer tempCust;

    // Search and Update Customer Record
    while(read(fd, &tempCust, sizeof(tempCust)) == sizeof(tempCust)){
        if(tempCust.custId == cust->custId){
            tempCust.balance = cust->balance;

            // Move file pointer back to overwrite
            off_t offset = lseek(fd, -sizeof(tempCust), SEEK_CUR);
            lockFile(fd, F_WRLCK, offset, sizeof(tempCust));
            write(fd, &tempCust, sizeof(tempCust));
            unlockFile(fd, offset, sizeof(tempCust));

            add_transaction_entry(cust->accountNumber, getNextCounterValue("txnId"), "DEPOSIT", amount, cust->balance, "Customer Deposited ");
            break;
        }
    }

    close(fd);

    strcpy(writeBuffer, "Amount Deposited successfully.\n");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
    }

    return 1;
}

int withdraw_handler(int nsd, struct Customer *cust){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Prompt for Withdraw Amount
    strcpy(writeBuffer, "Enter amount to withdraw: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    // Read Withdraw Amount
    memset(readBuffer, 0, BUFF_SIZE);
    readBytes = read(nsd, readBuffer, sizeof(readBuffer));
    if(readBytes < 0){
        perror("Read from client failed");
        return 0;
    }
    readBuffer[readBytes] = '\0'; // Null-terminate
        
    readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
    if(strlen(readBuffer) == 0){
        strcpy(writeBuffer, "Invalid amount. Withdraw failed.\n");

        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return 2;
    }

    float amount = atof(readBuffer);

    if(amount <= 0 || amount > cust->balance){
        strcpy(writeBuffer, "Invalid amount or insufficient balance. Withdraw failed.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return 2;
    }

    // Update Customer Balance
    cust->balance -= amount;

    // Update Customer Record in Database
    int fd = open(CUSTOMERS_DB, O_RDWR);
    if(fd < 0){
        perror("Failed to open customer database");
        return 0;
    }

    struct Customer tempCust;

    // Search and Update Customer Record
    while(read(fd, &tempCust, sizeof(tempCust)) == sizeof(tempCust)){
        if(tempCust.custId == cust->custId){
            tempCust.balance = cust->balance;

            // Move file pointer back to overwrite
            off_t offset = lseek(fd, -sizeof(tempCust), SEEK_CUR);
            lockFile(fd, F_WRLCK, offset, sizeof(tempCust));
            write(fd, &tempCust, sizeof(tempCust));
            unlockFile(fd, offset, sizeof(tempCust));

            add_transaction_entry(cust->accountNumber, getNextCounterValue("txnId"), "WITHDRAW", amount, cust->balance, "Customer Withdrawn ");

            break;
        }
    }

    close(fd);

    strcpy(writeBuffer, "Amount Withdrawn successfully.\n");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
    }

    return 1;
}

int money_transfer_handler(int nsd, struct Customer *cust){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Prompt for Recipient Account Number
    strcpy(writeBuffer, "Enter recipient account number: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    // Read Recipient Account Number
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

    int recAccNo = atoi(readBuffer);

    // Prompt for Transfer Amount
    strcpy(writeBuffer, "Enter amount to transfer: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    // Read Transfer Amount
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

    float amount = atof(readBuffer);

    if(amount <= 0 || amount > cust->balance){
        strcpy(writeBuffer, "Invalid amount or insufficient balance. Transfer failed.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return 2;
    }

    // Open Customer Database
    int fd = open(CUSTOMERS_DB, O_RDWR);
    if(fd < 0){
        perror("Failed to open customer database");
        return 0;
    }

    struct Customer tempCust;
    int found = 0;
    // Search for Recipient Customer
    while(read(fd, &tempCust, sizeof(tempCust)) == sizeof(tempCust)){
        if(tempCust.accountNumber == recAccNo){
            // Update Recipient Balance
            tempCust.balance += amount;

            // Move file pointer back to overwrite
            off_t offset = lseek(fd, -sizeof(tempCust), SEEK_CUR);
            lockFile(fd, F_WRLCK, offset, sizeof(tempCust));
            write(fd, &tempCust, sizeof(tempCust));
            unlockFile(fd, offset, sizeof(tempCust));

            char remarks[100];
            sprintf(remarks, "Received from AccNo %d", cust->accountNumber);
            add_transaction_entry(tempCust.accountNumber, getNextCounterValue("txnId"), "DEPOSIT", amount, tempCust.balance, remarks);

            found = 1;
            break;
        }
    }

    if(!found){
        strcpy(writeBuffer, "Recipient account not found. Transfer failed.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return 2;
    }
    
    // Deduct Amount from Sender Balance
    cust->balance -= amount;

    lseek(fd, 0, SEEK_SET); // Reset file pointer to beginning

    while(read(fd, &tempCust, sizeof(tempCust)) == sizeof(tempCust)){
        if(tempCust.accountNumber == cust->accountNumber){
            tempCust.balance = cust->balance;

            // Move file pointer back to overwrite
            lseek(fd, -sizeof(tempCust), SEEK_CUR);
            write(fd, &tempCust, sizeof(tempCust));

            char remarks[100];
            sprintf(remarks, "Transferred to AccNo %d", recAccNo);
            add_transaction_entry(cust->accountNumber, getNextCounterValue("txnId"), "TRANSFER", amount, cust->balance, remarks);

            break;
        }
    }

    close(fd);

    strcpy(writeBuffer, "Amount Transferred successfully.\n");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
    }

    return 1;
}

int add_transaction_entry(int acc_no, int txnID, const char* txnType, float amount, float remainingBalance, const char* remarks)
{
    struct TransactionHistory accounts;

    // TXN1001 | 2025-10-20 14:42:17 | DEPOSIT | +5000.00 | 20500.00 | Success\n

    //current time string
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr)-1, "%Y-%m-%d %H:%M:%S", t);

    char sign = (strcmp(txnType, "WITHDRAW") == 0 || strcmp(txnType, "TRANSFER") == 0 || 
                 strcmp(txnType, "LOAN_PAYMENT") == 0) ? '-' : '+';

    // Add transaction details to history
    char entry[256];
    snprintf(entry, sizeof(entry), "TXN%d | %s | %s | %c%.2f | %.2f | %s\n",
             txnID, timeStr, txnType, sign, amount, remainingBalance, remarks);
    
    // Open Transaction History Database
    int fd = open(TRANSACTIONS_DB, O_RDWR | O_RDWR);
    if(fd < 0){
        perror("Failed to open transaction history database");
        return 0;
    }

    while(read(fd, &accounts, sizeof(accounts)) == sizeof(accounts)){
        if(accounts.acc_no == acc_no){
            // Ensure no overflow before appending
            off_t offset = lseek(fd, -sizeof(accounts), SEEK_CUR);
            lockFile(fd, F_WRLCK, offset, sizeof(accounts));
            //If someone has modified history after reading and before locking
            lseek(fd, offset, SEEK_SET);
            read(fd, &accounts, sizeof(accounts));

            while(strlen(accounts.history) + strlen(entry) >= MAX_TRANS- 1) {
                //skip 2lines of header
                int linesSkipped = 0;
                
                char *newline = strchr(accounts.history, '\n'); //Find first newline char
                if (!newline) {
                    accounts.history[0] = '\0';
                    break;
                }

                if(!linesSkipped){
                    int lines = 0;
                    while(newline && lines < 2){
                        newline = strchr(newline + 1, '\n');
                        lines++;
                    }
                    if(!newline){
                        accounts.history[0] = '\0';
                        break;
                    }
                    linesSkipped = 1;
                }

                memmove(accounts.history, newline + 1, strlen(newline + 1) + 1); //Shift string left
                //strlen(newline + 1) + 1(\0) ==> Length of remaining string after newline
                //newline + 1          ==> Pointer to the character after newline
            }
            
            strcat(accounts.history, entry);

            off_t offset = lseek(fd, offset, SEEK_SET);
            write(fd, &accounts, sizeof(accounts));
            unlockFile(fd, offset, sizeof(accounts));

            break;
        }
    }

    close(fd);
    return 1;
}

int view_TransactionHistory(int nsd, int acc_no)
{
    char writeBuffer[BUFF_SIZE];
    int writeBytes;

    struct TransactionHistory accounts;

    // Open Transaction History Database
    int fd = open(TRANSACTIONS_DB, O_RDONLY);
    if(fd < 0){
        perror("Failed to open transaction history database");
        return 0;
    }
    
    printf("Viewing transaction history for Account No: %d\n", acc_no);

    int found = 0;

    lockFile(fd, F_RDLCK, 0, 0);
    while(read(fd, &accounts, sizeof(accounts)) == sizeof(accounts)){
        if(accounts.acc_no == acc_no){
            printf("Account No : %d, History : %s", accounts.acc_no, accounts.history);
            found = 1;
            break;
        }
    }
    unlockFile(fd, 0, 0);

    close(fd);

    if(!found){
        strcpy(writeBuffer, "No transaction history found for your account.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return 2;
    }

    // Send Transaction History to Customer
    strcpy(writeBuffer, "===== Transaction History =====\n");
    strcat(writeBuffer, accounts.history);

    printf("Account No : %d, History : %s", accounts.acc_no, accounts.history);
    
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    return 1;
}

int addFeedback_handler(int nsd, int custId){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Prompt for Feedback
    strcpy(writeBuffer, "Enter your feedback: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    // Read Feedback
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

    struct FeedBack fb;
    fb.custId = custId;
    strncpy(fb.feedback, readBuffer, sizeof(fb.feedback)-1);
    fb.feedback[sizeof(fb.feedback)-1] = '\0'; // Ensure null-termination

    // Append Feedback to Database
    int fd = open(FEEDBACKS_DB, O_APPEND | O_WRONLY | O_CREAT, 0644);
    if(fd < 0){
        perror("Failed to open feedback database");
        return 0;
    }   

    lockFile(fd, F_WRLCK, 0, 0);
    write(fd, &fb, sizeof(fb));
    unlockFile(fd, 0, 0);

    close(fd);

    strcpy(writeBuffer, "Feedback Submitted successfully.\n");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
    }

    return 1;
}

int apply_loan_handler(int nsd, int accNo){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Prompt for Loan Amount
    strcpy(writeBuffer, "Enter loan amount to apply for: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    // Read Loan Amount
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
    
    float loanAmount = atof(readBuffer);

    if(loanAmount <= 0){
        strcpy(writeBuffer, "Invalid loan amount. Application failed.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return 2;
    }

    struct LoanApplication loanApp;
    loanApp.accountNumber = accNo;
    loanApp.loanAmount = loanAmount;
    loanApp.loanID = getNextCounterValue("loanId");
    loanApp.status = 0; // Pending
    loanApp.empID = -1; // Not assigned yet

    printf("Applying for loan: AccNo=%d, LoanID=%d, Amount=%.2f\n", accNo, loanApp.loanID, loanAmount);
    // Append Loan Application to Database
    int fd = open(LOANAPPLICATIONS_DB, O_RDWR | O_CREAT, 0644);
    if(fd < 0){
        perror("Failed to open loans database");
        return 0;
    }

    lseek(fd, 0, SEEK_END);
    lockFile(fd, F_WRLCK, 0, 0);
    write(fd, &loanApp, sizeof(loanApp));
    unlockFile(fd, 0, 0);
    
    strcpy(writeBuffer, "Loan Application Submitted successfully.\n");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
    }
    close(fd);

    return 1;
}


