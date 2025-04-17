#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <windows.h>

#define HOST "localhost"
#define USER "root"
#define PW "bubun@22"
#define DB "mydb"

char fname[50], lname[50], id[50];

typedef struct {
    char Tnum[10];
    char Dep[20];
    char Des[20];
    int Seat;
} Train;
int i;
// Function to display all trains
void display(MYSQL *con) {
    if (mysql_query(con, "SELECT * FROM railways")) {
        printf("ERROR: %s\n", mysql_error(con));
        return;
    }

    MYSQL_RES *res = mysql_store_result(con);
    if (res) {
        MYSQL_ROW row;
        int num = mysql_num_fields(res);
        printf("\nTrain Info:\nTnum  Departure  Destination  Seats\n");
        while ((row = mysql_fetch_row(res))) {
            for (i = 0; i < num; i++) {
                printf(" %s ", row[i]);
            }
            printf("\n");
        }
        mysql_free_result(res);
    }
    printf("\n");
}

// Function to get serial number (slno)
int getSlno(MYSQL *con, const char *tableName, const char *fname, const char *lname, const char *id) {
    int slno = -1;
    char query[256];
    sprintf(query, "SELECT slno FROM %s WHERE fname = '%s' AND lname = '%s' AND id = '%s'", tableName, fname, lname, id);

    if (mysql_query(con, query)) {
        printf("Error: %s\n", mysql_error(con));
    } else {
        MYSQL_RES *res = mysql_store_result(con);
        if (res) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                slno = atoi(row[0]);
                printf("SL No: %d\n", slno);
            }
            mysql_free_result(res);
        }
    }
    return slno;
}

// Function to cancel ticket
void cancelTicket(MYSQL *con) {
    printf("Enter fname to cancel: ");
    scanf("%s", fname);
    printf("Enter lname to cancel: ");
    scanf("%s", lname);
    printf("Enter mail ID to cancel: ");
    scanf("%s", id);

    // Get train number of the user
    char getTrain[256];
    sprintf(getTrain, "SELECT train_number FROM users WHERE fname='%s' AND lname='%s' AND id='%s'", fname, lname, id);

    if (mysql_query(con, getTrain)) {
        printf("Error: %s\n", mysql_error(con));
        return;
    }

    MYSQL_RES *res = mysql_store_result(con);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL) {
        printf("No booking found for given user.\n");
        mysql_free_result(res);
        return;
    }

    char train_number[10];
    strcpy(train_number, row[0]);
    mysql_free_result(res);

    // Increment seat count in railways table
    char getSeatQuery[256];
    sprintf(getSeatQuery, "SELECT Seat FROM railways WHERE Tnumber='%s'", train_number);
    int seat = 0;
    if (!mysql_query(con, getSeatQuery)) {
        MYSQL_RES *resSeat = mysql_store_result(con);
        if (resSeat) {
            MYSQL_ROW seatRow = mysql_fetch_row(resSeat);
            if (seatRow) seat = atoi(seatRow[0]);
            mysql_free_result(resSeat);
        }
    }
    seat++;

    char updateSeat[256];
    sprintf(updateSeat, "UPDATE railways SET Seat='%d' WHERE Tnumber='%s'", seat, train_number);
    if (mysql_query(con, updateSeat)) {
        printf("Error updating seat: %s\n", mysql_error(con));
    }

    // Delete from train-specific table
    char deleteTrain[256];
    sprintf(deleteTrain, "DELETE FROM t%s WHERE fname='%s' AND lname='%s' AND id='%s'", train_number, fname, lname, id);
    if (mysql_query(con, deleteTrain)) {
        printf("Error deleting from train table: %s\n", mysql_error(con));
    }

    // Delete from users table
    char deleteUser[256];
    sprintf(deleteUser, "DELETE FROM users WHERE fname='%s' AND lname='%s' AND id='%s'", fname, lname, id);
    if (mysql_query(con, deleteUser)) {
        printf("Error deleting user: %s\n", mysql_error(con));
    } else {
        printf("Ticket Cancelled Successfully!\n");
    }
}

int main() {
    MYSQL *con = mysql_init(NULL);
    if (!mysql_real_connect(con, HOST, USER, PW, DB, 3306, NULL, 0)) {
        printf("Error: %s\n", mysql_error(con));
        return 1;
    } else {
        printf("Database ONLINE.....!\n");
    }

    // Train Data Initialization
    Train trains[] = {
        {"101", "bbsr", "delhi", 50},
        {"102", "bbsr", "ranchi", 40},
        {"103", "bbsr", "kolkata", 1}
    };

    // Insert or Update Train Data
    for (i = 0; i < 3; i++) {
        char query[512];
        sprintf(query, "INSERT INTO railways (Tnumber,Departure,Destination,Seat) VALUES('%s','%s','%s','%d') ON DUPLICATE KEY UPDATE Seat='%d'",
                trains[i].Tnum, trains[i].Dep, trains[i].Des, trains[i].Seat, trains[i].Seat);
        if (mysql_query(con, query)) {
            printf("Error inserting train data: %s\n", mysql_error(con));
        }
    }
    printf("Train data initialized!\n");

    int exit_flag = 0;
    while (!exit_flag) {
        printf("\nINDIAN RAILWAYS\n1. Book Ticket\n2. Cancel Ticket\n3. Exit\nEnter your choice: ");
        int choice;
        scanf("%d", &choice);

        if (choice == 1) {
            display(con);
            printf("Enter fname: ");
            scanf("%s", fname);
            printf("Enter lname: ");
            scanf("%s", lname);
            printf("Enter mail ID: ");
            scanf("%s", id);

            char insertUser[256];
            sprintf(insertUser, "INSERT INTO users (fname,lname,id) VALUES('%s','%s','%s')", fname, lname, id);
            if (mysql_query(con, insertUser)) {
                printf("Error: %s\n", mysql_error(con));
            } else {
                printf("Data inserted successfully!\n");
            }

            printf("Reserve\n1.101\n2.102\n3.103\nEnter your train choice: ");
            int tn;
            scanf("%d", &tn);

            char train[10];
            switch (tn) {
                case 1: strcpy(train, "101"); break;
                case 2: strcpy(train, "102"); break;
                case 3: strcpy(train, "103"); break;
                default:
                    printf("Invalid choice\n");
                    continue;
            }

            char updateUser[256];
            sprintf(updateUser, "UPDATE users SET train_number='%s' WHERE fname='%s' AND lname='%s' AND id='%s'", train, fname, lname, id);
            if (mysql_query(con, updateUser)) {
                printf("Error: %s\n", mysql_error(con));
            }

            int total = 0;
            char checkSeat[256];
            sprintf(checkSeat, "SELECT Seat FROM railways WHERE Tnumber='%s'", train);
            if (!mysql_query(con, checkSeat)) {
                MYSQL_RES *res = mysql_store_result(con);
                if (res) {
                    MYSQL_ROW row = mysql_fetch_row(res);
                    if (row) {
                        total = atoi(row[0]);
                    }
                    mysql_free_result(res);
                }
            }

            if (total > 0) {
                total--;
                char updateSeat[256];
                sprintf(updateSeat, "UPDATE railways SET Seat='%d' WHERE Tnumber='%s'", total, train);
                if (mysql_query(con, updateSeat)) {
                    printf("Error: %s\n", mysql_error(con));
                } else {
                    printf("Seat Reserved Successfully in: %s\n", train);
                }

                char tquery[256];
                sprintf(tquery, "INSERT INTO t%s (fname,lname,id) SELECT fname,lname,id FROM users WHERE train_number='%s'", train, train);
                mysql_query(con, tquery);

                printf("\n*****TICKET*****\nName: %s %s\nMail ID: %s\nTrain: %s\n", fname, lname, id, train);
                getSlno(con, (strcmp(train, "101") == 0 ? "t101" : (strcmp(train, "102") == 0 ? "t102" : "t103")), fname, lname, id);
            } else {
                printf("No seats available in train %s\n", train);
            }

        } else if (choice == 2) {
            cancelTicket(con);
        } else if (choice == 3) {
            printf("Thank you for using Indian Railways!\n");
            exit_flag = 1;
        } else {
            printf("Invalid choice! Try again.\n");
        }
    }

    mysql_close(con);
    return 0;
}

