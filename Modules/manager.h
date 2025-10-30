
int activate_deactivate_customer_accounts(int nsd);
int review_customer_feedback(int nsd);
int assign_loan_applications(int nsd);
struct Employee authenticate_manager(int nsd);


void manager_handler(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    struct Employee manager;

    manager = authenticate_manager(nsd);
    if(manager.empID == -1){
        strcpy(writeBuffer, "Manager authentication failed.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return;
    }
    // Manager Menu Loop
    int choice;
    while(1){
        // Send Manager Menu
        memset(writeBuffer, 0, BUFF_SIZE);
        
        strcpy(writeBuffer, MANAGER_MENU);
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return;
        }

        // Read Manager Choice
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
                // Activate/Deactivate Customer Accounts
                if(activate_deactivate_customer_accounts(nsd) == 0){
                    strcpy(writeBuffer, "Failed to change Customer account status. Please try again.\n");
                }
                break;

            case 2:
                // Assign Loan Application Processes to Employees
                if(assign_loan_applications(nsd) == 0){
                    strcpy(writeBuffer, "Failed to assign Loan Applications. Please try again.\n");
                }
                break;

            case 3:
                // Review Customer Feedback
                if(review_customer_feedback(nsd) == 0){
                    strcpy(writeBuffer, "Failed to retrieve Customer Feedbacks.\n");
                }
                break;

            case 4:
                // Change Password
                if(change_password(nsd, "EMPLOYEE", manager.empID)){
                    strcpy(writeBuffer, "Password changed successfully.\n");
                }
                else {
                    strcpy(writeBuffer, "Failed to change password. Please try again.\n");
                }
                break;

            case 5:
                // Logout
                log_out(nsd);
                return;

            case 6:
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

struct Employee authenticate_manager(int nsd)
{
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;
    struct Employee emp;
    emp.empID = -1; // Default to invalid

    int auth = 0;

    // Authentication
    while(!auth){
        strcpy(writeBuffer, "Enter Manager ID: ");
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
            return emp;
        }

        int empId = atoi(readBuffer);

        // Prompt for Employee Password
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

        // Open Employee Database
        int fd = open(EMPLOYEES_DB, O_RDONLY);
        if(fd < 0){
            perror("Failed to open employee database");
            return emp;
        }

        struct Employee tempEmp;
        int found = 0;

        // Search for Employee
        lockFile(fd, F_RDLCK, 0, 0);
        while(read(fd, &tempEmp, sizeof(tempEmp)) == sizeof(tempEmp)){
            if(tempEmp.empID == empId && strcmp(tempEmp.password, readBuffer) == 0 && tempEmp.role == 0){
                emp = tempEmp;
                found = 1;
                break;
            }
        }
        unlockFile(fd, 0, 0);

        close(fd);

        if(!found){
            strcpy(writeBuffer, "Invalid Manager ID or Password. Try Again.\n");
            writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0){
                perror("Write to client failed");
                return emp;
            }
            continue;
        }
        auth = 1;

    }
    return emp;
}

int activate_deactivate_customer_accounts(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Get Customer ID
    strcpy(writeBuffer, "Enter Customer ID to Activate/Deactivate: ");
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

            char statusMsg[100];
            if(cust.activeStatus == 1){
                sprintf(statusMsg, "Customer is currently ACTIVE.\n\nAre you sure you want to deactivate the account? (y/n): ");
            } 
            else {
                sprintf(statusMsg, "Customer is currently INACTIVE.\n\nAre you sure you want to activate the account? (y/n): ");
            }
            writeBytes = write(nsd, statusMsg, strlen(statusMsg));
            if(writeBytes < 0){
                perror("Write to client failed");
                unlockFile(fd, offset, sizeof(cust));
                close(fd);
                return 0;
            }


            // Read Confirmation
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

            // Check Confirmation
            if(strcmp(readBuffer, "y") == 0){
                cust.activeStatus = !cust.activeStatus;

                lseek(fd, - sizeof(cust), SEEK_CUR);
                write(fd, &cust, sizeof(cust));
                strcpy(writeBuffer, "Customer account status updated successfully.\n");
            }
            else {
                strcpy(writeBuffer, "Operation cancelled by manager.\n");
            }

            unlockFile(fd, offset, sizeof(cust));
            break;
        }
    }

    close(fd);

    if(!found){
        strcpy(writeBuffer, "Customer ID not found.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return 2;
    }

    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
    }

    return 1;
}

int review_customer_feedback(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Open Feedback Database
    int fd = open(FEEDBACKS_DB, O_RDONLY);
    if(fd < 0){
        perror("Failed to open feedback database");
        return 0;
    }

    struct FeedBack feedback;
    int feedbackCount = 0;

    // Read and Send Feedbacks
    lockFile(fd, F_RDLCK, 0, 0);
    strcpy(writeBuffer, "===== Customer Feedbacks =====\n");
    while(read(fd, &feedback, sizeof(feedback)) == sizeof(feedback)){
        char feedbackEntry[BUFF_SIZE];
        snprintf(feedbackEntry, sizeof(feedbackEntry), "Customer ID - %d\nFeedback - %s\n\n", feedback.custId, feedback.feedback);
        strcat(writeBuffer, feedbackEntry);
        feedbackCount++;
    }
    unlockFile(fd, 0, 0);

    close(fd);

    if(feedbackCount == 0){
        strcpy(writeBuffer, "No customer feedback available.\n");
    }

    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    return 1;
}

int assign_loan_applications(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Open Loan Applications Database
    int fdLoans = open(LOANAPPLICATIONS_DB, O_RDWR);
    if(fdLoans < 0){
        perror("Failed to open loan applications database");
        return 0;
    }

    struct LoanApplication loanApp;
    char unassignedLoanIds[1024] = {0}; // Store unassigned loan IDs
    

    // Count Unassigned Loan Applications
    lockFile(fdLoans, F_RDLCK, 0, 0);
    while(read(fdLoans, &loanApp, sizeof(loanApp)) == sizeof(loanApp)){
        if(loanApp.empID == -1){
            char loanIdEntry[1024];
            sprintf(loanIdEntry, "Loan ID - %d, Account Number - %d, Loan Amount - %.2f\n", loanApp.loanID, loanApp.accountNumber, (float)loanApp.loanAmount);
            strcat(unassignedLoanIds, loanIdEntry);
        }
    }
    unlockFile(fdLoans, 0, 0);

    if(strlen(unassignedLoanIds) == 0){
        strcpy(writeBuffer, "No unassigned loan applications available.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        close(fdLoans);
        return 2;
    }

    strcpy(writeBuffer, "Unassigned Loan Applications are - \n");
    strcat(writeBuffer, unassignedLoanIds);
    strcat(writeBuffer, "Enter Loan ID to assign: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        close(fdLoans);
        return 0;
    }
    

    //Enter Loan Id to assign
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

    // Get Employee ID to Assign Applications To
    strcpy(writeBuffer, "Enter Employee ID to assign this loan application: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        close(fdLoans);
        return 0;
    }

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

    int empId = atoi(readBuffer);

    // Reset file pointer to beginning
    lockFile(fdLoans, F_WRLCK, 0, 0);

    lseek(fdLoans, 0, SEEK_SET);

    while(read(fdLoans, &loanApp, sizeof(loanApp)) == sizeof(loanApp)){
        if(loanApp.loanID == loanId && loanApp.empID == -1){
            loanApp.empID = empId;
            loanApp.status = 1; // Assigned to Employee

            // Move file pointer back to overwrite
            lseek(fdLoans, - sizeof(loanApp), SEEK_CUR);
            write(fdLoans, &loanApp, sizeof(loanApp));
        }
    }

    unlockFile(fdLoans, 0, 0);
    close(fdLoans);

    sprintf(writeBuffer, "Assigned loan application %d to Employee ID %d.\n", loanId, empId);
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        close(fdLoans);
    }
    return 1;
}
