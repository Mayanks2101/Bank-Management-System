

int log_out(int nsd, const char* userType, int userId){
    char writeBuffer[BUFF_SIZE], readBuffer[BUFF_SIZE];
    int writeBytes, readBytes;

    if(strcmp(userType, "CUSTOMER") == 0){ // Customer
        //Mark Customer as Logged Out
        int fd = open(CUSTOMERS_DB, O_RDWR);
        if(fd < 0){
            perror("Failed to open customer database");
            return 0;
        }

        struct Customer tempCust;
        off_t offset = lseek(fd, 0, SEEK_SET);
        lockFile(fd, F_WRLCK, 0, 0);
        while(read(fd, &tempCust, sizeof(tempCust)) == sizeof(tempCust)){
            if(tempCust.custId == userId){
                tempCust.isLoggedIn = 0; // Mark as logged out
                lseek(fd, -sizeof(tempCust), SEEK_CUR);
                write(fd, &tempCust, sizeof(tempCust));
                break;
            }
        }
        unlockFile(fd, 0, 0);
        close(fd);
    }
    else if(strcmp(userType, "EMPLOYEE") == 0){ // Employee
        //Mark Employee as Logged Out
        int fd = open(EMPLOYEES_DB, O_RDWR);
        if(fd < 0){
            perror("Failed to open employee database");
            return 0;
        }

        struct Employee tempEmp;
        off_t offset = lseek(fd, 0, SEEK_SET);
        lockFile(fd, F_WRLCK, 0, 0);
        while(read(fd, &tempEmp, sizeof(tempEmp)) == sizeof(tempEmp)){
            if(tempEmp.empID == userId){
                tempEmp.isLoggedIn = 0; // Mark as logged out
                lseek(fd, -sizeof(tempEmp), SEEK_CUR);
                write(fd, &tempEmp, sizeof(tempEmp));
                break;
            }
        }
        unlockFile(fd, 0, 0);
        close(fd);
    }
    else if(strcmp(userType, "MANAGER") == 0){ // Manager
        //Mark Manager as Logged Out
        int fd = open(EMPLOYEES_DB, O_RDWR);
        if(fd < 0){
            perror("Failed to open employee database");
            return 0;
        }

        struct Employee tempEmp;
        off_t offset = lseek(fd, 0, SEEK_SET);
        lockFile(fd, F_WRLCK, 0, 0);
        while(read(fd, &tempEmp, sizeof(tempEmp)) == sizeof(tempEmp)){
            if(tempEmp.empID == userId){
                tempEmp.isLoggedIn = 0; // Mark as logged out
                lseek(fd, -sizeof(tempEmp), SEEK_CUR);
                write(fd, &tempEmp, sizeof(tempEmp));
                break;
            }
        }
        unlockFile(fd, 0, 0);
        close(fd);
    }
    else if(strcmp(userType, "ADMIN") == 0){ // Admin
        //Mark Admin as Logged Out
        int fd = open(ADMIN_LOCK_DB, O_RDWR | O_CREAT, 0644);
        if(fd < 0){
            perror("Failed to open admin lock database");
            return 0;
        }

        int isLoggedIn;
        lockFile(fd, F_WRLCK, 0, 0);
        
        lseek(fd, 0, SEEK_SET);
        read(fd, &isLoggedIn, sizeof(isLoggedIn));
        isLoggedIn = 0; // Mark as logged out
        lseek(fd, 0, SEEK_SET);
        write(fd, &isLoggedIn, sizeof(isLoggedIn));

        unlockFile(fd, 0, 0);
        close(fd);
    }
    
    // Exit
    // memset(writeBuffer, 0, BUFF_SIZE);
    // strcpy(writeBuffer, "Logging out...\n");
    // writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    // if(writeBytes < 0){
    //     perror("Write to client failed");
    //     return 0;
    // }
    // readBytes = read(nsd, readBuffer, sizeof(readBuffer));
    return 1;
}

int exit_client(int nsd, const char* userType, int userId){
    char writeBuffer[BUFF_SIZE];
    int writeBytes;

    // Perform logout operations
    log_out(nsd, userType, userId);

    // Exit
    // memset(writeBuffer, 0, BUFF_SIZE);
    // strcpy(writeBuffer, "Exiting...\n");
    // writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    // if(writeBytes < 0){
    //     perror("Write to client failed");
    //     return 0;
    // }

    close(nsd);
    exit(0);
}