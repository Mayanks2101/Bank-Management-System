int authenticate_admin(int nsd);
int addEmployee(int nsd);
int promoteEmployeeToManager(int nsd);
int demoteManagerToEmployee(int nsd);
int manage_user_roles(int nsd);
int modify_customer_details(int nsd);
int modify_employee_details(int nsd);
int modify_customer_employee_details(int nsd);


void admin_handler(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    if(!authenticate_admin(nsd)){
        memset(writeBuffer, 0, BUFF_SIZE);
        strcpy(writeBuffer, "Admin authentication failed.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return; // Authentication failed
    }
    printf("Admin authenticated successfully.\n");


    // Admin Menu Loop
    int choice;
    while(1){
        memset(writeBuffer, 0, BUFF_SIZE);

        // Send Admin Menu
        strcpy(writeBuffer, ADMIN_MENU);
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return;
        }

        // Read Admin Choice
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
        printf("Client Entered Choice: %d\n", choice);
        
        memset(writeBuffer, 0, BUFF_SIZE);
        switch(choice){
            case 1:
                // Add New Bank Employee
                if(addEmployee(nsd) == 0){
                    strcpy(writeBuffer, "Failed to add New Employee.  Please try Again.\n");
                }
                break;

            case 2:
                // Modify Customer/Employee Details
                if(modify_customer_employee_details(nsd) == 0){
                    strcpy(writeBuffer, "Failed to modify Customer details. Please Try Again.\n");
                }
                break;

            case 3:
                // Manage User Roles
                if(manage_user_roles(nsd) == 0){
                    strcpy(writeBuffer, "User roles not Changed. Please Try Again.\n");
                }
                break;

            case 4:
                // Logout
                log_out(nsd);
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
        memset(writeBuffer, 0, BUFF_SIZE);

    }
}

int authenticate_admin(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;
    int auth = 0;

    // Authentication
    while(!auth){
        memset(writeBuffer, 0, BUFF_SIZE);
        strcpy(writeBuffer, "Enter Admin Name: ");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return 0;
        }

        // Read Admin ID
        memset(readBuffer, 0, BUFF_SIZE);
        readBytes = read(nsd, readBuffer, BUFF_SIZE);
        if(readBytes < 0){
            perror("Read from client failed");
            return 0;
        }
        readBuffer[readBytes] = '\0'; // Null-terminate
    
        readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
        if(strlen(readBuffer) == 0){
            return 0;
        }

        // Check Admin Name
        if(strcmp(readBuffer, ADMINNAME) != 0){
            memset(writeBuffer, 0, BUFF_SIZE);
            strcpy(writeBuffer, "Invalid Admin Name. Try again.\n");
            writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0){
                perror("Write to client failed");
                return 0;
            }
            readBytes = read(nsd, readBuffer, BUFF_SIZE); // Consume input before retry
            continue;
        }

        // Prompt for Admin Password
        memset(writeBuffer, 0, BUFF_SIZE);
        strcpy(writeBuffer, "Enter Admin Password: ");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return 0;
        }

        // Read Admin Password
        memset(readBuffer, 0, BUFF_SIZE);
        readBytes = read(nsd, readBuffer, BUFF_SIZE);
        if(readBytes < 0){
            perror("Read from client failed");
            return 0;
        }
        readBuffer[readBytes] = '\0'; // Null-terminate
    
        readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
        if(strlen(readBuffer) == 0){
            return 0;
        }

        // Check Admin Password
        if(strcmp(readBuffer, ADMINPASS) != 0){
            memset(writeBuffer, 0, BUFF_SIZE);
            strcpy(writeBuffer, "Invalid Admin Password. Try again.\n");
            writeBytes = write(nsd, writeBuffer, BUFF_SIZE);
            if(writeBytes < 0){
                perror("Write to client failed");
                return 0;
            }
            continue;
        }

        auth = 1; // Successful authentication
    }
    return 1;
}

int addEmployee(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    struct Employee newEmp;
    memset(&newEmp, 0, sizeof(newEmp));

    // Get Employee Name
    memset(writeBuffer, 0, BUFF_SIZE);
    strcpy(writeBuffer, "Enter New Employee Name: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    memset(readBuffer, 0, BUFF_SIZE);
    readBytes = read(nsd, readBuffer, sizeof(readBuffer) - 1);
    if(readBytes < 0){
        perror("Read from client failed");
        return 0;
    }
    readBuffer[readBytes] = '\0'; // Null-terminate
    
    readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
    if(strlen(readBuffer) == 0){
        return 0;
    }

    strncpy(newEmp.empName, readBuffer, sizeof(newEmp.empName) - 1);

    // Get Employee Password
    strcpy(writeBuffer, "Enter New Employee Password: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    memset(readBuffer, 0, BUFF_SIZE);
    readBytes = read(nsd, readBuffer, sizeof(readBuffer) - 1);
    if(readBytes <= 0){
        perror("Read from client failed");
        return 0;
    }
    readBuffer[readBytes] = '\0'; // Null-terminate
    
    readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
    if(strlen(readBuffer) == 0){
        return 0;
    }

    strncpy(newEmp.password, readBuffer, sizeof(newEmp.password) - 1);

    newEmp.role = 1; // Default role as Employee
    newEmp.empID = getNextCounterValue("empId");

    int fd = open(EMPLOYEES_DB, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd < 0){
        perror("Failed to open employee database");
        return 0;
    }

    lockFile(fd, F_WRLCK, 0, 0);
    write(fd, &newEmp, sizeof(newEmp));
    unlockFile(fd, 0, 0);

    close(fd);

    sprintf(writeBuffer, "Employee added successfully.\nPlease Note Employee ID for future references.\nEmployee ID: %d\n", newEmp.empID);
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));

    if(writeBytes < 0){
        perror("Write to client failed");
    }
    return 1;
}

int manage_user_roles(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;
    int choice;

    while(1){
        int result = 0;
        // Send Manage User Roles Menu
        memset(writeBuffer, 0, BUFF_SIZE);

        strcpy(writeBuffer, MANAGE_USERROLES_MENU);
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            return 0;
        }

        // Read Choice
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

        choice = atoi(readBuffer);

        memset(writeBuffer, 0, BUFF_SIZE);
        switch(choice){
            case 1:
                // Promote Employee to Manager
                result =  promoteEmployeeToManager(nsd);
                break;

            case 2:
                // Demote Manager to Employee
                result = demoteManagerToEmployee(nsd);
                break;

            default:
                strcpy(writeBuffer, "Invalid choice. Try again.\n");
                writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
                continue;
        }

        //Handle This Problem
        if(result != 2){
            return result;
        }
    }

    return 1;
}

int promoteEmployeeToManager(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Get Employee ID to Promote
    strcpy(writeBuffer, "Enter Employee ID to Promote to Manager: ");
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
    
    int empId = atoi(readBuffer);

    // Open Employee Database
    int fd = open(EMPLOYEES_DB, O_RDWR);
    if(fd < 0){
        perror("Failed to open employee database");
        return 0;
    }

    struct Employee emp;
    int found = 0;

    // Search for Employee
    while(read(fd, &emp, sizeof(emp)) == sizeof(emp)){
        if(emp.empID == empId && emp.role == 1){
            found = 1;

            emp.role = 0; // Promote to Manager
            
            // Move file pointer back to overwrite
            off_t offset = lseek(fd, - sizeof(emp), SEEK_CUR);

            lockFile(fd, F_WRLCK, offset, sizeof(emp));
            write(fd, &emp, sizeof(emp));
            unlockFile(fd, offset, sizeof(emp));

            break;
        }
    }

    close(fd);

    if(!found){
        strcpy(writeBuffer, "Employee ID not found. Press Enter to continue.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        readBytes = read(nsd, readBuffer, sizeof(readBuffer));
        return 2;
    }

    strcpy(writeBuffer, "Employee promoted to Manager successfully.\n");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
    }
    return 1;
}

int demoteManagerToEmployee(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Get Manager ID to Demote
    strcpy(writeBuffer, "Enter Manager ID to Demote to Employee: ");
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

    int empId = atoi(readBuffer);

    // Open Employee Database
    int fd = open(EMPLOYEES_DB, O_RDWR);
    if(fd < 0){
        perror("Failed to open employee database");
        return 0;
    }

    struct Employee emp;
    int found = 0;

    // Search for Manager
    while(read(fd, &emp, sizeof(emp)) == sizeof(emp)){
        if(emp.empID == empId && emp.role == 0){
            found = 1;
            emp.role = 1; // Demote to Employee

            // Move file pointer back to overwrite
            off_t offset = lseek(fd, -sizeof(emp), SEEK_CUR);
            lockFile(fd, F_WRLCK, offset, sizeof(emp));
            write(fd, &emp, sizeof(emp));
            unlockFile(fd, offset, sizeof(emp));

            break;
        }
    }

    close(fd);

    if(!found){
        strcpy(writeBuffer, "Manager ID not found. Press Enter to continue.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        readBytes = read(nsd, readBuffer, sizeof(readBuffer));
        return 2;
    }

    strcpy(writeBuffer, "Manager demoted to Employee successfully.\n");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){ 
        perror("Write to client failed");
    }
    
    return 1;
}

int modify_customer_employee_details(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Prompt for Choice
    strcpy(writeBuffer, "Modify Details Menu:\n1. Customer Details\n2. Employee Details\nEnter choice: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    // Read Choice
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


    int choice = atoi(readBuffer);

    if(choice == 1){
        // Modify Customer Details
        return modify_customer_details(nsd);
    }
    else if(choice == 2){
        // Modify Employee Details
        return modify_employee_details(nsd);
    }
    else{
        strcpy(writeBuffer, "Invalid choice. Operation cancelled.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
        }
        return 2;
    }
}

int modify_employee_details(int nsd){
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    // Get Employee ID to Modify
    strcpy(writeBuffer, "Enter Employee ID to Modify: ");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }

    memset(readBuffer, 0, BUFF_SIZE);
    readBytes = read(nsd, readBuffer, sizeof(readBuffer) - 1);
    if(readBytes < 0){
        perror("Read from client failed");
        return 0;
    }
    readBuffer[readBytes] = '\0'; // Null-terminate
    
    readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
    if(strlen(readBuffer) == 0){
        return 0;
    }
    
    int empId = atoi(readBuffer);

    // Open Employee Database
    int fd = open(EMPLOYEES_DB, O_RDWR);
    if(fd < 0){
        perror("Failed to open employee database");
        return 0;
    }

    struct Employee emp;
    int found = 0;

    // Search for Employee
    while(read(fd, &emp, sizeof(emp)) == sizeof(emp)){
        if(emp.empID == empId){
            found = 1;

            // Print Current Details
            snprintf(writeBuffer, sizeof(writeBuffer), "Current Employee Name is %s\nEnter New Employee Name is ", emp.empName);
            
            writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0){
                perror("Write to client failed");
                close(fd);
                return 0;
            }

            // Read New Employee Name
            memset(readBuffer, 0, BUFF_SIZE);
            readBytes = read(nsd, readBuffer, sizeof(readBuffer) - 1);
            if(readBytes <= 0){
                perror("Read from client failed");
                close(fd);
                return 0;
            }
            readBuffer[readBytes] = '\0'; // Null-terminate
    
            readBuffer[strcspn(readBuffer, "\n")] = '\0'; // Remove newline
            if(strlen(readBuffer) == 0){
                return 0;
            }

            strncpy(emp.empName, readBuffer, sizeof(emp.empName) - 1);

            // Move file pointer back to overwrite
            off_t offset = lseek(fd, - sizeof(emp), SEEK_CUR);
            lockFile(fd, F_WRLCK, offset, sizeof(emp));
            write(fd, &emp, sizeof(emp));
            unlockFile(fd, offset, sizeof(emp));
            
            break;
        }
    }

    if(!found){
        strcpy(writeBuffer, "Employee ID not found.\n");
        writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
        if(writeBytes < 0){
            perror("Write to client failed");
            close(fd);
        }
        return 2;
    }

    close(fd);

    strcpy(writeBuffer, "Employee details updated successfully.\n");
    writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    if(writeBytes < 0){
        perror("Write to client failed");
        return 0;
    }
    
    return 1;
}

