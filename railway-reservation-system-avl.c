//Railway Reservation System - AVL Tree Version//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
enum Gender { FEMALE, MALE, OTHER };

typedef struct SeatNode {
    int seatNo;
    char coachType[8];
    char berthType[3];
    int available;
    int height;
    struct SeatNode *left, *right;
} Seat;

typedef struct CoachNoNode {
    char coachNo[4];
    Seat *seatRoot; /* AVL of seats */
    int height;
    struct CoachNoNode *left, *right;
} CoachNo;

typedef struct CoachTypeNode {
    char type[8];
    CoachNo *coachRoot; /* AVL of coaches */
    int height;
    struct CoachTypeNode *left, *right;
} CoachType;

typedef struct PassengerNode {
    char name[30];
    char DoB[11];
    enum Gender gender;
    int age;
    char coachNo[4];
    int seatNo;
    char berthType[3];
    int height;
    struct PassengerNode *left, *right;
} Passenger;

/* Global passenger AVL root */
static Passenger *passengerRoot = NULL;
static Passenger* pInsert(Passenger *root, char name[], char dob[], enum Gender gender, int age,char coachNo[], int seatNo, char berthType[]);
static int pCount(Passenger *root);
static void pCollect(Passenger *root, Passenger **arr, int *idx);
static CoachType* ctFind(CoachType *root, char type[]);
static CoachNo* cnFind(CoachNo *root, char coachNo[]);
static Seat* seatFind(Seat *root, int seatNo);

void saveToFile(){
    FILE *fp = fopen("p.txt", "w");
    if(fp == NULL){
        printf("Error opening file!\n");
        return;
    }
    int total = pCount(passengerRoot);
    Passenger **arr = malloc(total * sizeof(Passenger*));
    int idx = 0;
    pCollect(passengerRoot, arr, &idx);
    for(int i=0;i<total;i++){
        Passenger *p = arr[i];
        fprintf(fp, "%s %s %d %d %s %d %s\n",p->name,p->DoB,p->gender,p->age,p->coachNo,p->seatNo,p->berthType);
    }
    fclose(fp);
    free(arr);
    printf("Data saved successfully!\n");
}

void loadFromFile(CoachType *train){
    FILE *fp = fopen("p.txt", "r");
    if(fp == NULL){
        printf("No previous data found.\n");
        return;
    }
    char name[30], dob[11], coachNo[4], berthType[3];
    int gender, age, seatNo;
    while(fscanf(fp, "%s %s %d %d %s %d %s",name, dob, &gender, &age,coachNo, &seatNo, berthType) != EOF){
        passengerRoot = pInsert(passengerRoot,name, dob, (enum Gender)gender,age,coachNo, seatNo, berthType);
        // Derive coach type from coachNo prefix
        char coachType[8];
        if      (coachNo[0] == '0')            strcpy(coachType, "1AC");
        else if (coachNo[0] == '1')            strcpy(coachType, "2AC");
        else if (coachNo[0] == '2')            strcpy(coachType, "3AC");
        else if (coachNo[0] == '3')            strcpy(coachType, "Sleeper");
        else continue; // WL or unknown
        CoachType *ct = ctFind(train, coachType);
        if(ct){
            CoachNo *cn = cnFind(ct->coachRoot, coachNo);
                if(cn){
                    Seat *s = seatFind(cn->seatRoot, seatNo);
                    if(s){
                        s->available = 0;
                    }
                }
        }
    }
    fclose(fp);
    printf("Data loaded successfully!\n");
}

void toUpper(char str[]){
    for(int i = 0; str[i]; i++){
        str[i] = toupper(str[i]);
    }
}

int max2(int a, int b) { return a > b ? a : b; }

int isValidAge(int age);
int isValidGender(int g);
int isValidDate(char dob[]);

static int seatHeight(Seat *n) { return n ? n->height : 0; }
static void seatUpdateHeight(Seat *n) {
    if (n)
        n->height = 1 + max2(seatHeight(n->left), seatHeight(n->right));
}
static int seatBF(Seat *n) {
    return n ? seatHeight(n->left) - seatHeight(n->right) : 0;
}
static Seat *seatRotateRight(Seat *y) {
    Seat *x = y->left, *T = x->right;
    x->right = y; y->left = T;
    seatUpdateHeight(y); seatUpdateHeight(x);
    return x;
}
static Seat *seatRotateLeft(Seat *x) {
    Seat *y = x->right, *T = y->left;
    y->left = x; x->right = T;
    seatUpdateHeight(x); seatUpdateHeight(y);
    return y;
}
static Seat *seatBalance(Seat *n) {
    seatUpdateHeight(n);
    int bf = seatBF(n);
    /* LL */ if (bf > 1 && seatBF(n->left) >= 0) return seatRotateRight(n);
    /* LR */ if (bf > 1 && seatBF(n->left) < 0) { 
        n->left = seatRotateLeft(n->left); 
        return seatRotateRight(n); 
    }
    /* RR */ if (bf < -1 && seatBF(n->right) <= 0) return seatRotateLeft(n);
    /* RL */ if (bf < -1 && seatBF(n->right) > 0) { 
        n->right = seatRotateRight(n->right);
        return seatRotateLeft(n);
    }
    return n;
}
static Seat *seatInsert(Seat *root, int seatNo, char coachType[], char berthType[]) {
    if (!root) {
        Seat *n = (Seat *)malloc(sizeof(Seat));
        n->seatNo = seatNo;
        strcpy(n->coachType, coachType);
        strcpy(n->berthType, berthType);
        n->available = 1;
        n->height = 1;
        n->left = n->right = NULL;
        return n;
    }
    if (seatNo < root->seatNo) root->left = seatInsert(root->left, seatNo, coachType, berthType);
    else if (seatNo > root->seatNo) root->right = seatInsert(root->right, seatNo, coachType, berthType);
    return seatBalance(root);
}
/* Find seat by seatNo */
static Seat *seatFind(Seat *root, int seatNo) {
    if (!root) return NULL;
    if (seatNo == root->seatNo) return root;
    if (seatNo < root->seatNo) return seatFind(root->left, seatNo);
    return seatFind(root->right, seatNo);
}
/* Count available seats of a given berthType via inorder */

static int countAvailableSeats(Seat *root, char pe[]) {
    if (!root) return 0;
    int count = 0;
    count += countAvailableSeats(root->left, pe);
    if (root->available == 1 && strcmp(root->berthType, pe) == 0)
        count++;
    count += countAvailableSeats(root->right, pe);
    return count;
}

/* Count ALL available seats (any berth) via inorder */
static int countAllAvailable(Seat *root) {
    if (!root) return 0;
    return countAllAvailable(root->left)+(root->available == 1 ? 1 : 0)+countAllAvailable(root->right);
}

/* Free entire seat AVL tree */
static void seatFreeAll(Seat *root) {
    if (!root) return;
    seatFreeAll(root->left);
    seatFreeAll(root->right);
    free(root);
}

static int passengerCmp(const char *coachA, int seatA,const char *coachB, int seatB) {
    int wlA = (strcmp(coachA, "WL") == 0);
    int wlB = (strcmp(coachB, "WL") == 0);
    if (wlA && !wlB) return 1;
    if (!wlA && wlB) return -1;
    int c = strcmp(coachA, coachB);
    if (c != 0) return c;
    return seatA - seatB;
}

static int pHeight(Passenger *n) { return n ? n->height : 0; }
static void pUpdateHeight(Passenger *n) {
    if (n) n->height = 1 + max2(pHeight(n->left), pHeight(n->right));
}
static int pBF(Passenger *n) {
    return n ? pHeight(n->left) - pHeight(n->right) : 0;
}
static Passenger *pRotateRight(Passenger *y) {
    Passenger *x = y->left, *T = x->right;
    x->right = y; y->left = T;
    pUpdateHeight(y); pUpdateHeight(x);
    return x;
}
static Passenger *pRotateLeft(Passenger *x) {
    Passenger *y = x->right, *T = y->left;
    y->left = x; x->right = T;
    pUpdateHeight(x); pUpdateHeight(y);
    return y;
}
static Passenger *pBalance(Passenger *n) {
    pUpdateHeight(n);
    int bf = pBF(n);
    if (bf > 1 && pBF(n->left) >= 0) return pRotateRight(n);
    if (bf > 1 && pBF(n->left) < 0) { 
        n->left = pRotateLeft(n->left);
        return pRotateRight(n); 
    }
    if (bf < -1 && pBF(n->right) <= 0) return pRotateLeft(n);
    if (bf < -1 && pBF(n->right) > 0) { 
        n->right = pRotateRight(n->right); 
        return pRotateLeft(n); 
    }
    return n;
}
static Passenger *pInsert(Passenger *root,char name[], char dob[], enum Gender gender, int age,char coachNo[], int seatNo, char berthType[]) {
    if (!root) {
        Passenger *p = (Passenger *)malloc(sizeof(Passenger));
        strcpy(p->name, name);
        strcpy(p->DoB, dob);
        p->gender = gender;
        p->age = age;
        strcpy(p->coachNo, coachNo);
        p->seatNo = seatNo;
        strcpy(p->berthType, berthType);
        p->height = 1;
        p->left = p->right = NULL;
        return p;
    }
    int cmp = passengerCmp(coachNo, seatNo, root->coachNo, root->seatNo);
    if (cmp < 0) root->left = pInsert(root->left, name, dob, gender, age, coachNo, seatNo, berthType);
    else if (cmp > 0) root->right = pInsert(root->right, name, dob, gender, age, coachNo, seatNo, berthType);
/* duplicate key: update in place */
    else {
        strcpy(root->name, name);
        strcpy(root->DoB, dob);
        root->gender = gender;
        root->age = age;
        strcpy(root->berthType, berthType);
    }
    return pBalance(root);
    }
/* Find minimum node in a subtree (for deletion) */
static Passenger *pMinNode(Passenger *root) {
    while (root->left) root = root->left;
    return root;
}
/* Delete by (coachNo, seatNo) key */
static Passenger *pDelete(Passenger *root, char coachNo[], int seatNo) {
    if (!root) return NULL;
    int cmp = passengerCmp(coachNo, seatNo, root->coachNo, root->seatNo);
    if (cmp < 0)
        root->left = pDelete(root->left, coachNo, seatNo);
    else if (cmp > 0)
        root->right = pDelete(root->right, coachNo, seatNo);
    else {
        /* Found — delete it */
        if (!root->left || !root->right) {
            Passenger *tmp = root->left ? root->left : root->right;
            free(root);
            return tmp;
        }
        /* Two children: replace with inorder successor */
        Passenger *succ = pMinNode(root->right);
        strcpy(root->name, succ->name);
        strcpy(root->DoB, succ->DoB);
        root->gender = succ->gender;
        root->age = succ->age;
        strcpy(root->coachNo, succ->coachNo);
        root->seatNo = succ->seatNo;
        strcpy(root->berthType, succ->berthType);
        root->right = pDelete(root->right, succ->coachNo, succ->seatNo);
    }
    return pBalance(root);

}
/* Find a passenger by (coachNo, seatNo) */
static Passenger *pFind(Passenger *root, char coachNo[], int seatNo) {
    if (!root) return NULL;
    int cmp = passengerCmp(coachNo, seatNo, root->coachNo, root->seatNo);
    if (cmp == 0) return root;
    if (cmp < 0) return pFind(root->left, coachNo, seatNo);
    return pFind(root->right, coachNo, seatNo);
}

static int waitNo = 0;

static int pCount(Passenger *root) {
    if (!root) return 0;
    return 1 + pCount(root->left) + pCount(root->right);
}
/* collect all passengers inorder into an array */
static void pCollect(Passenger *root, Passenger **arr, int *idx) {
    if (!root) return;
    pCollect(root->left, arr, idx);
    arr[(*idx)++] = root;
    pCollect(root->right, arr, idx);
}

static void printPassenger(Passenger *p) {
    char g;
    if (p->gender == FEMALE) g = 'F';
    else if (p->gender == MALE) g = 'M';
    else g = 'O';
    if (strcmp(p->coachNo, "WL") == 0)
        printf("%s | %c | %d | WL%d\n", p->name, g, p->age, p->seatNo);
    else
        printf("%s | %c | %d | %s | Seat %d | %s\n",
    p->name, g, p->age, p->coachNo, p->seatNo, p->berthType);
}
/* inorder display (by coach+seat order - AVL inorder) */
static void displayPassengerInorder(Passenger *root) {
    if (!root) return;
    displayPassengerInorder(root->left);
    printPassenger(root);
    displayPassengerInorder(root->right);
}
//addPassenger
static void addPassenger(char name[], char dob[], enum Gender gender, int age,
char coachNo[], int seatNo, char berthType[]) {
    passengerRoot = pInsert(passengerRoot, name, dob, gender, age,coachNo, seatNo,berthType);
}

static int cnHeight(CoachNo *n) { return n ? n->height : 0; }
static void cnUpdateHeight(CoachNo *n) {
    if (n) n->height = 1 + max2(cnHeight(n->left), cnHeight(n->right));
}
static int cnBF(CoachNo *n) {
return n ? cnHeight(n->left) - cnHeight(n->right) : 0;
}
static CoachNo *cnRotateRight(CoachNo *y) {
    CoachNo *x = y->left, *T = x->right;
    x->right = y; y->left = T;
    cnUpdateHeight(y); 
    cnUpdateHeight(x);
    return x;
}
static CoachNo *cnRotateLeft(CoachNo *x) {
    CoachNo *y = x->right, *T = y->left;
    y->left = x; x->right = T;
    cnUpdateHeight(x); 
    cnUpdateHeight(y);
    return y;
}
static CoachNo *cnBalance(CoachNo *n) {
    cnUpdateHeight(n);
    int bf = cnBF(n);
    if (bf > 1 && cnBF(n->left) >= 0) return cnRotateRight(n);
    if (bf > 1 && cnBF(n->left) < 0) { 
        n->left = cnRotateLeft(n->left); 
        return cnRotateRight(n); 
    }
    if (bf < -1 && cnBF(n->right) <= 0) return cnRotateLeft(n);
    if (bf < -1 && cnBF(n->right) > 0) { 
        n->right = cnRotateRight(n->right); 
        return cnRotateLeft(n); 
    }
    return n;
}
static CoachNo *cnInsert(CoachNo *root, char coachNo[], Seat *seatRoot) {
    if (!root) {
        CoachNo *n = (CoachNo *)malloc(sizeof(CoachNo));
        strcpy(n->coachNo, coachNo);
        n->seatRoot = seatRoot;
        n->height = 1;
        n->left = n->right = NULL;
        return n;
    }
    int c = strcmp(coachNo, root->coachNo);
    if (c < 0) root->left = cnInsert(root->left, coachNo, seatRoot);
    else if (c > 0) root->right = cnInsert(root->right, coachNo, seatRoot);
    return cnBalance(root);
}
static CoachNo *cnFind(CoachNo *root, char coachNo[]) {
    if (!root) return NULL;
    int c = strcmp(coachNo, root->coachNo);
    if (c == 0) return root;
    if (c < 0) return cnFind(root->left, coachNo);
    return cnFind(root->right, coachNo);
}
static void cnFreeAll(CoachNo *root) {
    if (!root) return;
    cnFreeAll(root->left);
    cnFreeAll(root->right);
    seatFreeAll(root->seatRoot);
    free(root);
}
/* Count coaches in subtree */
static int cnCount(CoachNo *root) {
    if (!root) return 0;
    return 1 + cnCount(root->left) + cnCount(root->right);
}
/* Collect all CoachNo nodes inorder into anarray */
static void cnCollect(CoachNo *root, CoachNo **arr, int *idx) {
    if (!root) return;
    cnCollect(root->left, arr, idx);
    arr[(*idx)++] = root;
    cnCollect(root->right, arr, idx);
}

static int ctHeight(CoachType *n) { return n ? n->height : 0; }
static void ctUpdateHeight(CoachType *n) {
    if (n) n->height = 1 + max2(ctHeight(n->left), ctHeight(n->right));
}
static int ctBF(CoachType *n) {
    return n ? ctHeight(n->left) - ctHeight(n->right) : 0;
}
static CoachType *ctRotateRight(CoachType *y) {
    CoachType *x = y->left, *T = x->right;
    x->right = y; y->left = T;
    ctUpdateHeight(y); 
    ctUpdateHeight(x);
    return x;
}
static CoachType *ctRotateLeft(CoachType *x) {
    CoachType *y = x->right, *T = y->left;
    y->left = x; x->right = T;
    ctUpdateHeight(x); 
    ctUpdateHeight(y);
    return y;
}
static CoachType *ctBalance(CoachType *n) {
    ctUpdateHeight(n);
    int bf = ctBF(n);
    if (bf > 1 && ctBF(n->left) >= 0) return ctRotateRight(n);
    if (bf > 1 && ctBF(n->left) < 0) { 
        n->left = ctRotateLeft(n->left); 
        return ctRotateRight(n); 
    }
    if (bf < -1 && ctBF(n->right) <= 0) return ctRotateLeft(n);
    if (bf < -1 && ctBF(n->right) > 0) { 
        n->right = ctRotateRight(n->right); 
        return ctRotateLeft(n); 
    }
    return n;
}
static CoachType *ctInsert(CoachType *root, char type[], CoachNo *coachRoot) {
    if (!root) {
        CoachType *n = (CoachType *)malloc(sizeof(CoachType));
        strcpy(n->type, type);
        n->coachRoot = coachRoot;
        n->height = 1;
        n->left = n->right = NULL;
        return n;
    }
    int c = strcmp(type, root->type);
    if (c < 0) root->left = ctInsert(root->left, type, coachRoot);
    else if (c > 0) root->right = ctInsert(root->right, type, coachRoot);
    return ctBalance(root);
}
static CoachType *ctFind(CoachType *root, char type[]) {
    if (!root) return NULL;
    int c = strcmp(type, root->type);
    if (c == 0) return root;
    if (c < 0) return ctFind(root->left, type);
    return ctFind(root->right, type);
}
static void ctFreeAll(CoachType *root) {
    if (!root) return;
    ctFreeAll(root->left);
    ctFreeAll(root->right);
    cnFreeAll(root->coachRoot);
    free(root);
}
static int ctCount(CoachType *root) {
    if (!root) return 0;
    return 1 + ctCount(root->left) + ctCount(root->right);
}
/* Collect all CoachType nodes inorder */
static void ctCollect(CoachType *root, CoachType **arr, int *idx) {
    if (!root) return;
    ctCollect(root->left, arr, idx);
    arr[(*idx)++] = root;
    ctCollect(root->right, arr, idx);
}
static void getCoachNo(int i, char coachType[], char coachNo[]) {
    if (strcmp(coachType,"1AC") == 0) sprintf(coachNo, "%03d", i);
    else if (strcmp(coachType,"2AC") == 0) sprintf(coachNo, "%d%02d", 1, i);
    else if (strcmp(coachType,"3AC") == 0) sprintf(coachNo, "%d%02d", 2, i);
    else if (strcmp(coachType,"Sleeper")== 0) sprintf(coachNo, "%d%02d", 3, i);
    else sprintf(coachNo, "000");
}
static int getSeatsPerCoach(char type[]) {
    if (strcmp(type,"1AC") == 0) return 16;
    else if (strcmp(type,"2AC") == 0) return 48;
    else if (strcmp(type,"3AC") == 0) return 64;
    else if (strcmp(type,"Sleeper")== 0) return 72;
    return 0;
}
static char *getBerthType(int i, char coachType[]) {
    if (strcmp(coachType, "1AC") == 0) {
        return (i % 2 == 1) ? "L" : "U";
    }
    if (i%8==1 || i%8==4) return "L";
    else if (i%8==2 || i%8==5) return "M";
    else if (i%8==3 || i%8==6) return "U";
    else if (i%8==7) return "SL";
    else return "SU";
}
/* Build seat AVL for a coach (seats 1..numOfSeats) */
static Seat *createSeatAVL(int numOfSeats, char coachType[]) {
    Seat *root = NULL;
    for (int i = 1; i <= numOfSeats; i++)
    root = seatInsert(root, i, coachType, getBerthType(i, coachType));
    return root;
}
/* Build CoachNo AVL for a coach type */
static CoachNo *createCoachNoAVL(char coachType[], int numOfCoaches) {
    CoachNo *root = NULL;
    for (int i = 1; i <= numOfCoaches; i++) {
        char coachNo[4];
        getCoachNo(i, coachType, coachNo);
        Seat *seats = NULL;
        if (strcmp(coachType,"Pantry")!=0 && strcmp(coachType,"Engine")!=0)
            seats = createSeatAVL(getSeatsPerCoach(coachType), coachType);
        root = cnInsert(root, coachNo, seats);
    }
    return root;
}
/* Build entire train as CoachType AVL */
static CoachType *initTrain(void) {
    CoachType *root = NULL;
    root = ctInsert(root, "1AC", createCoachNoAVL("1AC", 2));
    root = ctInsert(root, "2AC", createCoachNoAVL("2AC", 3));
    root = ctInsert(root, "3AC", createCoachNoAVL("3AC", 8));
    root = ctInsert(root, "Engine", createCoachNoAVL("Engine", 1));
    root = ctInsert(root, "Pantry", createCoachNoAVL("Pantry", 1));
    root = ctInsert(root, "Sleeper",createCoachNoAVL("Sleeper",10));
    return root;
}

/* Find first available seat of given berthType via inorder */
static Seat *findFirstAvailable(Seat *root, char berthType[]) {
    if (!root) return NULL;
    Seat *left = findFirstAvailable(root->left, berthType);
    if (left) return left;
    if (root->available == 1 && strcmp(root->berthType, berthType) == 0)
        return root;
    return findFirstAvailable(root->right, berthType);
}
/* Find first available seat of ANY type via inorder */
static Seat *findFirstAvailableAny(Seat *root) {
    if (!root) return NULL;
    Seat *left = findFirstAvailableAny(root->left);
    if (left) return left;
    if (root->available == 1) return root;
    return findFirstAvailableAny(root->right);
}
static void readPassengerDetails(char name[], char dob[], int *age, enum Gender *gender) {
    int genderInput;
    printf("Enter name: ");
    scanf("%s", name);
    do {
        printf("Enter DoB (dd-mm-yyyy): ");
        scanf("%s", dob);
        if (!isValidDate(dob)) printf("Invalid date! Try again.\n");
    } while (!isValidDate(dob));
    do {
        printf("Enter age: ");
        scanf("%d", age);
        if (!isValidAge(*age)) printf("Invalid age! Enter between 0-120\n");
    } while (!isValidAge(*age));
    do {
        printf("Enter gender (0-F,1-M,2-O): ");
        scanf("%d", &genderInput);
        if (!isValidGender(genderInput)) printf("Invalid gender!\n");
    } while (!isValidGender(genderInput));
    *gender = (enum Gender)genderInput;
}
//book seats
void Book(CoachType *train, char coachType[], int n, char seatTypes[][3]) {
    CoachType *ct = ctFind(train, coachType);
    int foundCoach = 0;
    int totalBooked = 0;
    typedef struct { char coachNo[4]; int seatNo; char berthType[3];
    char name[30]; char g; int age; int wl; int wlNo; } TicketEntry;
    TicketEntry tickets[200];
    int ticketCount = 0;
    if (ct == NULL) {
        printf("Coach type not found\n");
        return;
    }
    int numCoaches = cnCount(ct->coachRoot);

    CoachNo **coaches = (CoachNo **)malloc(numCoaches * sizeof(CoachNo *));
    { int idx = 0; cnCollect(ct->coachRoot, coaches, &idx); }
    char name[30], dob[11];
    int age;
    enum Gender gender;
    /* Try to fit ALL seats in ONE coach --- */
    for (int ci = 0; ci < numCoaches && !foundCoach; ci++) {
        CoachNo *c = coaches[ci];
        int possible = 1;
        for (int i = 0; i < n && possible; i++) {
        if (countAvailableSeats(c->seatRoot, seatTypes[i]) == 0)
            possible = 0;
        }
        if (!possible) continue;
        printf("Booking in Coach %s\n", c->coachNo);
        for (int i = 0; i < n; i++) {
            Seat *s = findFirstAvailable(c->seatRoot, seatTypes[i]);
            if (s) {
            s->available = 0;
            readPassengerDetails(name, dob, &age, &gender);
            addPassenger(name, dob, gender, age, c->coachNo, s->seatNo, s->berthType);
            printf("Booked Seat %d (%s)\n", s->seatNo, s->berthType);
            totalBooked++;
            /* ticket record */
            strcpy(tickets[ticketCount].coachNo, c->coachNo);
            tickets[ticketCount].seatNo = s->seatNo;
            strcpy(tickets[ticketCount].berthType, s->berthType);
            strcpy(tickets[ticketCount].name, name);
            tickets[ticketCount].g = (gender==FEMALE)?'F':(gender==MALE)?'M':'O';
            tickets[ticketCount].age = age;
            tickets[ticketCount].wl = 0;
            ticketCount++;
            }
        }
        foundCoach = 1;
    }
/* Multi-coach, one seat at a time (preferred type) --- */
    if (!foundCoach) {
        for (int i = 0; i < n; i++) {
            int foundSeat = 0;
            for (int ci = 0; ci < numCoaches && !foundSeat; ci++) {
                CoachNo *c2 = coaches[ci];
                Seat *s = findFirstAvailable(c2->seatRoot, seatTypes[i]);
                if (!s) continue;
                s->available = 0;
                readPassengerDetails(name, dob, &age, &gender);
                addPassenger(name, dob, gender, age, c2->coachNo, s->seatNo, s->berthType);
                printf("Booked Seat %d (%s) in Coach %s\n", s->seatNo, s->berthType, c2->coachNo);
                totalBooked++;
                strcpy(tickets[ticketCount].coachNo, c2->coachNo);
                tickets[ticketCount].seatNo = s->seatNo;
                strcpy(tickets[ticketCount].berthType, s->berthType);
                strcpy(tickets[ticketCount].name, name);
                tickets[ticketCount].g = (gender==FEMALE)?'F':(gender==MALE)?'M':'O';
                tickets[ticketCount].age = age;
                tickets[ticketCount].wl = 0;
                ticketCount++;
                foundSeat = 1;
            }
            /* Alternative seat if preferred not found */
            if (!foundSeat) {
                printf("Preferred seat %s not available\n", seatTypes[i]);
                char choice;
                printf("Alternative? (Y/N): ");
                scanf(" %c", &choice);
                if (choice == 'Y' || choice == 'y') {
                    int altFound = 0;
                    for (int ci = 0; ci < numCoaches && !altFound; ci++) {
                        CoachNo *c3 = coaches[ci];
                        Seat *s2 = findFirstAvailableAny(c3->seatRoot);
                        if (!s2) continue;
                        s2->available = 0;
                        readPassengerDetails(name, dob, &age, &gender);
                        addPassenger(name, dob, gender, age, c3->coachNo, s2->seatNo, s2->berthType);
                        printf("Booked Seat %d (%s) in %s\n", s2->seatNo, s2->berthType, c3->coachNo);
                        totalBooked++;
                        strcpy(tickets[ticketCount].coachNo, c3->coachNo);
                        tickets[ticketCount].seatNo = s2->seatNo;
                        strcpy(tickets[ticketCount].berthType, s2->berthType);
                        strcpy(tickets[ticketCount].name, name);
                        tickets[ticketCount].g = (gender==FEMALE)?'F':(gender==MALE)?'M':'O';
                        tickets[ticketCount].age = age;
                        tickets[ticketCount].wl = 0;
                        ticketCount++;
                        altFound = 1;
                    }
                } 
                else {
                    printf("Skipping this preference\n");
                }
            }
        }
    }
/* Waiting list for whoever wasn't booked --- */
    if (totalBooked < n) {
        int remaining = n - totalBooked;
        for (int i = 0; i < remaining; i++) {
            readPassengerDetails(name, dob, &age, &gender);
            waitNo++;
            addPassenger(name, dob, gender, age, "WL", waitNo, "NA");
            printf("Added to WL%d\n", waitNo);
            strcpy(tickets[ticketCount].coachNo, "WL");
            tickets[ticketCount].seatNo = waitNo;
            strcpy(tickets[ticketCount].berthType, "NA");
            strcpy(tickets[ticketCount].name, name);
            tickets[ticketCount].g = (gender==FEMALE)?'F':(gender==MALE)?'M':'O';
            tickets[ticketCount].age = age;
            tickets[ticketCount].wl = 1;
            tickets[ticketCount].wlNo = waitNo;
            ticketCount++;
        }
    }
    /* --- TICKET SUMMARY --- */
    printf("\n--- TICKET DETAILS---\n");
    printf("Coach Type: %s\n", coachType);
    for (int i = 0; i < ticketCount; i++) {
        TicketEntry *t = &tickets[i];
        if (t->wl)
            printf("%s | %c | %d | WL%d\n", t->name, t->g, t->age, t->wlNo);
        else
            printf("%s | %c | %d | %s | Seat %d | %s\n",
        t->name, t->g, t->age, t->coachNo, t->seatNo, t->berthType);
    }
    free(coaches);
    saveToFile();
}
//display passenegers
void displayPassengers(void) {
    displayPassengerInorder(passengerRoot);
}
static int cmpByName(const void *a, const void *b) {
    return strcmp((*(Passenger **)a)->name, (*(Passenger **)b)->name);
}
void displaySortedByName(void) {
    int total = pCount(passengerRoot);
    if (total == 0) return;
    Passenger **arr = (Passenger **)malloc(total * sizeof(Passenger *));
    int idx = 0;
    pCollect(passengerRoot, arr, &idx);
    qsort(arr, total, sizeof(Passenger *), cmpByName);
    for (int i = 0; i < total; i++)
        printPassenger(arr[i]);
    free(arr);
}
// DISPLAY SORTED BY COACH
void displaySortedByCoach(void) {
    displayPassengerInorder(passengerRoot);
}
static void displayLowerBerthInorder(Passenger *root) {
    if (!root) return;
    displayLowerBerthInorder(root->left);
    if (strcmp(root->berthType,"L")==0 || strcmp(root->berthType,"SL")==0)
        printf("%s | Age: %d | %s | Seat %d (%s)\n",root->name, root->age, root->coachNo, root->seatNo, root->berthType);
    displayLowerBerthInorder(root->right);
}
void displayLowerBerth(void) {
    displayLowerBerthInorder(passengerRoot);
}
static void displaySeniorNoLowerInorder(Passenger *root,int* found) {
    if (!root) return;
    displaySeniorNoLowerInorder(root->left,found);
    if (root->age > 60 && strcmp(root->berthType,"L") != 0 && strcmp(root->berthType,"SL") != 0){
        printf("%s | Age: %d | %s | Seat %d (%s)\n",root->name, root->age, root->coachNo, root->seatNo, root->berthType);
        *found = 1;
    }
    displaySeniorNoLowerInorder(root->right,found);
}
void displaySeniorNoLower(void) {
    int found = 0;
    displaySeniorNoLowerInorder(passengerRoot, &found);
    if (!found) {
        printf("No senior passengers found without lower berth.\n");
    }
}
// DISPLAY AVAILABLE SEATS
static void displayAvailableSeatsForType(CoachNo *cnRoot) {
    if (!cnRoot) return;
    displayAvailableSeatsForType(cnRoot->left);
    int count = countAllAvailable(cnRoot->seatRoot);
    printf("Coach %s : %d seats available\n", cnRoot->coachNo, count);
    displayAvailableSeatsForType(cnRoot->right);
}
static void displayAvailableSeatsInorder(CoachType *ctRoot) {
    if (!ctRoot) return;
    displayAvailableSeatsInorder(ctRoot->left);
    printf("\nCoach Type: %s\n", ctRoot->type);
    displayAvailableSeatsForType(ctRoot->coachRoot);
    displayAvailableSeatsInorder(ctRoot->right);
}
void displayAvailableSeats(CoachType *train) {
    displayAvailableSeatsInorder(train);
}
static CoachType **trainOrder = NULL;
static int trainCount = 0;
void displayAvailableSeatsOrdered(void) {
    for (int i = 0; i < trainCount; i++) {
        CoachType *ct = trainOrder[i];
        printf("\nCoach Type: %s\n", ct->type);
        displayAvailableSeatsForType(ct->coachRoot);
    }
}
void rangeSearch(char n1[], char n2[]){
    int total = pCount(passengerRoot);
    if(total == 0){
    printf("No passengers found\n");
    return;
    }
    Passenger **arr = (Passenger**)malloc(total * sizeof(Passenger*));
    int idx = 0;
    pCollect(passengerRoot, arr, &idx);
    // sort by name
    qsort(arr, total, sizeof(Passenger*), cmpByName);

    printf("\nPassengers between %s and %s:\n", n1, n2);
    for(int i = 0; i < total; i++){
        if((strcmp(arr[i]->name, n1) >= 0 && strcmp(arr[i]->name, n2) <= 0) ||(strcmp(arr[i]->name, n2) >= 0 && strcmp(arr[i]->name, n1) <= 0)){
            printPassenger(arr[i]);
        }
    }
    free(arr);
}
/* CANCELLATION */
/* Mark a seat available again */
static void freeSeat(CoachType *train, char coachNo[], int seatNo) {
    int total = ctCount(train);
    CoachType **arr = (CoachType **)malloc(total * sizeof(CoachType *));
    int idx = 0; 
    ctCollect(train, arr, &idx);
    for (int i = 0; i < total; i++) {
        CoachNo *cn = cnFind(arr[i]->coachRoot, coachNo);
        if (cn) {
            Seat *s = seatFind(cn->seatRoot, seatNo);
            if (s) s->available = 1;
        }
    }
    free(arr);
}
/* Find lowest-numbered WL passenger (inorder ) */
static Passenger *getFirstWLInorder(Passenger *root, Passenger **best) {
    if (!root) return *best;
    getFirstWLInorder(root->left, best);
    if (strcmp(root->coachNo, "WL") == 0) {
        if (*best == NULL || root->seatNo < (*best)->seatNo)
            *best = root;
    }
    getFirstWLInorder(root->right, best);
    return *best;
}
static Passenger *getFirstWL(void) {
    Passenger *best = NULL;
    getFirstWLInorder(passengerRoot, &best);
    return best;
}
/* Re-number all WL passengers sequentially (inorder) */
typedef struct { Passenger **arr; int idx; } WLCollector;
static void collectWL(Passenger *root, Passenger **arr, int *idx) {
    if (!root) return;
    collectWL(root->left, arr, idx);
    if (strcmp(root->coachNo, "WL") == 0)
        arr[(*idx)++] = root;
    collectWL(root->right, arr, idx);
}
static void updateWaitingList(void) {
    int total = pCount(passengerRoot);
    Passenger **arr = (Passenger **)malloc(total * sizeof(Passenger *));
    int idx = 0;
    collectWL(passengerRoot, arr, &idx);
    /* Sort WL entries by current seatNo to preserve relative order */
    for (int i = 0; i < idx-1; i++)
        for (int j = i+1; j < idx; j++)
            if (arr[i]->seatNo > arr[j]->seatNo) {
                Passenger *t = arr[i]; 
                arr[i] = arr[j]; 
                arr[j] = t;
            }
    /* Re-number */
    for (int i = 0; i < idx; i++) {
        char oldCoach[4]; int oldSeat;
        strcpy(oldCoach, arr[i]->coachNo);
        oldSeat = arr[i]->seatNo;
        char name[30], dob[11]; enum Gender g; int age; char bt[3];
        strcpy(name, arr[i]->name);
        strcpy(dob, arr[i]->DoB);
        g = arr[i]->gender;
        age = arr[i]->age;
        strcpy(bt, arr[i]->berthType);
        passengerRoot = pDelete(passengerRoot, oldCoach, oldSeat);
        passengerRoot = pInsert(passengerRoot, name, dob, g, age, "WL", i+1, bt);
    }
    waitNo = idx;
    free(arr);
}
static void cancelOnePassenger(CoachType *train, char coachNo[], int seatNo) {
    Passenger *curr = pFind(passengerRoot, coachNo, seatNo);
    if (!curr) {
        printf("Passenger not found in coach %s seat %d\n", coachNo, seatNo);
        return;
    }
    /* Copy the cancelled passenger's berth before deletion */
    char cancelledBerth[3];
    strcpy(cancelledBerth, curr->berthType);
    Passenger *wl = getFirstWL();
    if (wl != NULL) {
    /* Move WL passenger into this seat */
        char wlName[30], wlDob[11]; enum Gender wlG; int wlAge;
        int wlOldSeat;
        strcpy(wlName, wl->name);
        strcpy(wlDob, wl->DoB);
        wlG = wl->gender;
        wlAge = wl->age;
        wlOldSeat = wl->seatNo;
        printf("WL passenger %s moved to Seat %d (%s)\n",wl->name, seatNo,cancelledBerth);
        /* Delete cancelled passenger */
        passengerRoot = pDelete(passengerRoot, coachNo, seatNo);
        /* Delete WL entry */
        passengerRoot = pDelete(passengerRoot, "WL", wlOldSeat);
        /* Re-insert WL passenger as booked */
        passengerRoot = pInsert(passengerRoot, wlName, wlDob, wlG, wlAge,
        coachNo, seatNo, cancelledBerth);
        updateWaitingList();
    } else {
        freeSeat(train, coachNo, seatNo);
        passengerRoot = pDelete(passengerRoot, coachNo, seatNo);
    }
    printf("Cancelled Seat %d in Coach %s\n", seatNo, coachNo);
}
void cancelTickets(CoachType *train, int n) {
    if (n <= 0) {
        printf("Invalid number of seats to cancel.\n");
        return;
    }
    int booked = pCount(passengerRoot);
    if (n > booked) {
        printf("Cannot cancel %d seats. Only %d seat(s) currently booked.\n", n, booked);
        return;
    }
    for (int i = 0; i < n; i++) {
    char coachNo[4];
        int seatNo;
        printf("Enter coach number and seat number to cancel: ");
        scanf("%s %d", coachNo, &seatNo);
        cancelOnePassenger(train, coachNo, seatNo);
    }
    saveToFile();  
}
//VALIDATION
int isValidAge(int age) { return (age >= 0 && age <= 120) ? 1 : 0; }
int isValidGender(int g) { return (g==0||g==1||g==2) ? 1 : 0; }
int isValidDate(char dob[]) {
int d, m, y;
sscanf(dob, "%d-%d-%d", &d, &m, &y);
if (m < 1 || m > 12) return 0;

int days;
if (m==1||m==3||m==5||m==7||m==8||m==10||m==12) days = 31;
else if (m==4||m==6||m==9||m==11) days = 30;
else days = ((y%4==0 && y%100!=0)||(y%400==0)) ? 29 : 28;
return (d >= 1 && d <= days) ? 1 : 0;
}
void initTrainOrder(CoachType *train){
}
/* =========================================================
* MAIN
* ========================================================= */
int main(void) {
    CoachType *train = initTrain();
    initTrainOrder(train);
    loadFromFile(train);
    int choice = 1;
    while (choice) {
        printf("\n===== TRAIN RESERVATION SYSTEM =====\n");
        printf("1. Book Tickets\n");
        printf("2. Cancel Tickets\n");
        printf("3. Display All Passengers\n");
        printf("4. Display Sorted by Name\n");
        printf("5. Display Sorted by Coach\n");
        printf("6. Display Lower Berth Passengers\n");
        printf("7. Display Senior Citizens without Lower Berth\n");
        printf("8. Display Available Seats\n");
        printf("9. Range Search by Name\n");
        printf("10. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        switch (choice) {
        case 1: {
            char coachType[10];
            int n;
            printf("Enter coach type (1AC/2AC/3AC/Sleeper): ");
            scanf("%s", coachType);
            toUpper(coachType);
            printf("Enter number of seats: ");
            scanf("%d", &n);
            char (*seatTypes)[3] = malloc(n * 3);
            for (int i = 0; i < n; i++) {
                if(strcmp(coachType,"1AC")==0)
                    printf("Enter seat type (L/U): ");
                else{
                    printf("Enter seat type (L/M/U/SL/SU): ");
                }
            scanf("%s", seatTypes[i]);
            toUpper(seatTypes[i]);
            }
            Book(train, coachType, n, seatTypes);
            free(seatTypes);
            break;
        }
        case 2: {
            int n;
            printf("Enter number of seats to cancel: ");
            scanf("%d", &n);
            cancelTickets(train, n);
            break;
        }
        case 3:
            displayPassengers();
            break;
        case 4:
            displaySortedByName();
            break;
        case 5:
            displaySortedByCoach();
            break;
        case 6:
            displayLowerBerth();
            break;
        case 7:
            displaySeniorNoLower();
            break;
        case 8:
            displayAvailableSeats(train);
            break;
        case 9:{
            char n1[30], n2[30];
            printf("Enter first name: ");
            scanf("%s", n1);
            printf("Enter second name: ");
            scanf("%s", n2);
            rangeSearch(n1, n2);
            break;
        }
        case 10:
            printf("Exiting...\n");
            choice = 0;
            break;
            default:
            printf("Invalid choice\n");
        }
    }
    /* Free all memory */
    ctFreeAll(train);
    free(trainOrder);
    /* Free passenger AVL */
    int total = pCount(passengerRoot);
    if (total > 0) {
        Passenger **arr = (Passenger **)malloc(total * sizeof(Passenger *));
        int idx = 0;
        pCollect(passengerRoot, arr, &idx);
        for (int i = 0; i < total; i++) free(arr[i]);
        free(arr);
    }
    return 0;
}