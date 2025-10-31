
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
        // strcpy(writeBuffer, "Manager authentication failed.\n");
        // writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        // if(writeBytes < 0){
        //     perror("Write to client failed");
        // }
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
                    strcpy(writeBuffer, "Failed to change Customer account status. \nPress 1 to continue..\n");
                }
                break;

            case 2:
                // Assign Loan Application Processes to Employees
                if(assign_loan_applications(nsd) == 0){
                    strcpy(writeBuffer, "Failed to assign Loan Applications. \nPress 1 to continue..\n");
                }
                break;

            case 3:
                // Review Customer Feedback
                if(review_customer_feedback(nsd) == 0){
                    strcpy(writeBuffer, "Failed to retrieve Customer Feedbacks. \nPress 1 to continue..\n");
                }
                break;

            case 4:
                // Change Password
                if(change_password(nsd, "EMPLOYEE", manager.empID)){
                    strcpy(writeBuffer, "Password changed successfully.\nPress 1 to continue..\n");
                }
                else {
                    strcpy(writeBuffer, "Failed to change password. \nPress 1 to continue..\n");
                }
                break;

            case 5:
                // Logout
                log_out(nsd, "MANAGER", manager.empID);
                return;

            case 6:
                // Exit
                exit_client(nsd, "MANAGER", manager.empID);
                return;

            default:
                strcpy(writeBuffer, "Invalid choice. \nPress 1 to continue..\n");
                break;
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

struct Employee authenticate_manager(int nsd)
{
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;
    struct Employee emp;
    emp.empID = -1; // Default to invalid

    // int auth = 0;

    // // Authentication
    // while(!auth){
    memset(writeBuffer, 0, BUFF_SIZE);
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
                if(tempEmp.isLoggedIn){
                    strcpy(writeBuffer, "This Manager is already logged in from another session. \nPress 1 to continue..\n");
                    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
                    if(writeBytes < 0){
                        perror("Write to client failed");
                    }
                    readBytes = read(nsd, readBuffer, sizeof(readBuffer));
                    unlockFile(fd, 0, 0);
                    close(fd);
                    // readBytes = read(nsd, readBuffer, sizeof(readBuffer));
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
            memset(writeBuffer, 0, BUFF_SIZE);
            strcpy(writeBuffer, "Invalid Manager ID or Password. \nPress 1 to continue..\n");
            writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0){
                perror("Write to client failed");
                return emp;
            }
            readBytes = read(nsd, readBuffer, sizeof(readBuffer));
            return emp;
        }

        //Mark as logged in
        fd = open(EMPLOYEES_DB, O_RDWR);
        if(fd < 0){
            perror("Failed to open employee database");
            return emp;
        }

        off_t offset = lseek(fd, 0, SEEK_SET);
        lockFile(fd, F_WRLCK, 0, 0);
        while(read(fd, &tempEmp, sizeof(tempEmp)) == sizeof(tempEmp)){
            if(tempEmp.empID == emp.empID){
                tempEmp.isLoggedIn = 1;
                lseek(fd, -sizeof(tempEmp), SEEK_CUR);
                write(fd, &tempEmp, sizeof(tempEmp));
                emp = tempEmp;
                break;
            }
        }
        unlockFile(fd, 0, 0);
        close(fd);

    //     auth = 1;

    // }
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
                strcpy(writeBuffer, "Customer account status updated successfully.\nPress 1 to continue..\n");
            }
            else {
                strcpy(writeBuffer, "Operation cancelled by manager.\nPress 1 to continue..\n");
            }

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
        }
        readBytes = read(nsd, readBuffer, sizeof(readBuffer));
        return 2;
    }

    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
    }
    readBytes = read(nsd, readBuffer, sizeof(readBuffer));
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
    strcat(writeBuffer, "==============================\nPress 1 to continue..\n");
    unlockFile(fd, 0, 0);

    close(fd);

    if(feedbackCount == 0){
        strcpy(writeBuffer, "No customer feedback available.\nPress 1 to continue..\n");
    }

    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }
    readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
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
        strcpy(writeBuffer, "No unassigned loan applications available.\nPress 1 to continue..\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
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

    // Validate Employee ID
    int fdEmp = open(EMPLOYEES_DB, O_RDONLY);
    if(fdEmp < 0){
        perror("Failed to open employee database");
        close(fdLoans);
        return 0;
    }
    struct  Employee emp;
    int empFound = 0;
    lockFile(fdEmp, F_RDLCK, 0, 0);
    while(read(fdEmp, &emp, sizeof(emp)) == sizeof(emp)){
        if(emp.empID ==  empId && emp.role == 1){
            empFound = 1;
            break;
        }
    }
    unlockFile(fdEmp, 0, 0);
    close(fdEmp);

    if(!empFound){
        strcpy(writeBuffer, "Invalid Employee ID. Operation cancelled. \nPress 1 to continue..\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
        close(fdLoans);
        return 2;
    }

    // Reset file pointer to beginning
    lockFile(fdLoans, F_WRLCK, 0, 0);

    lseek(fdLoans, 0, SEEK_SET);
    int found = 0;
    // Assign the specified Loan Application
    while(read(fdLoans, &loanApp, sizeof(loanApp)) == sizeof(loanApp)){
        if(loanApp.loanID == loanId && loanApp.empID == -1){
            loanApp.empID = empId;
            loanApp.status = 1; // Assigned to Employee
            found = 1;
            // Move file pointer back to overwrite
            lseek(fdLoans, - sizeof(loanApp), SEEK_CUR);
            write(fdLoans, &loanApp, sizeof(loanApp));
        }
    }
    unlockFile(fdLoans, 0, 0);
    close(fdLoans);

    if(!found){
        strcpy(writeBuffer, "Invalid Loan ID. Operation cancelled.\nPress 1 to continue..\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
        return 2;
    }

    sprintf(writeBuffer, "Assigned loan application %d to Employee ID %d.\nPress 1 to continue..\n", loanId, empId);
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        close(fdLoans);
    }
    readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
    return 1;
}
