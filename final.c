#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *file_command;
FILE *file_customer;
FILE *file_result;

int error;

int count_lines(FILE *file_line);
int digits(char a);
void reading_customers(FILE *f, int **matrix, int number_customers, int number_resources);
int requests(FILE *f, int **allocation, int number_customers, int number_resources, int *duplis);
void check_safestate(int **need, int *resources, int number_customers, int number_resources);
void check_maximum(int **allocation, int **maximum, int number_customers, int number_resources);
int releases(FILE *f, int **allocation, int number_customers, int number_resources, int *duplis);
void printMatriz(int number_resources, int number_customers, int **maximum, int **allocation, int **need);


int main(int argc, char *argv[]){

    
    char c;
    error = 0;

    int **maximum;
    int **allocation;
    int **need;
    int **auxalloc;

    int curr_customer;
    int resources[argc-1];
    int flags[argc-1];
    int duplis[argc-1];

    int number_resources = argc;
    int number_customers;

    for (int i = 1; i < number_resources; i++){
        resources[i-1] = atoi(argv[i]);
        flags[i-1] = resources[i-1];
    }
    number_resources = argc - 1;

    //abrir arquivos
    file_command = fopen("commands.txt", "r");
    if (file_command == NULL){
        printf("Não foi possível abrir o arquivo com os comandos.");
        return 0;
    }

    file_customer = fopen("customer.txt", "r");
    if (file_customer == NULL){
        printf("Não foi possível abrir o arquivo com os clientes.");
        return 0;
    }
    
    number_customers = count_lines(file_customer);
    fclose(file_customer);
    
    file_result = fopen("result.txt", "w");
    if (file_result == NULL){
        printf("Não foi possível abrir o arquivo com os resultados.");
        return 0;
    }

    

    maximum = malloc(number_customers * sizeof * maximum);
    for (int i = 0; i < number_customers; i++){
        maximum[i] = malloc(number_customers * sizeof * maximum[i]); 
    }

    allocation = malloc(number_customers * sizeof * allocation);
    for (int i = 0; i < number_customers; i++){
        allocation[i] = malloc(number_customers * sizeof* allocation[i]);
    }

    need = malloc(number_customers * sizeof * need);
    for (int i = 0; i < number_customers; i++){
        need[i] = malloc(number_customers * sizeof* need[i]);
    }

    auxalloc = malloc(number_customers * sizeof * auxalloc);
    for (int i = 0; i < number_customers; i++){
        auxalloc[i] = malloc(number_customers * sizeof* auxalloc[i]);
    }

    file_customer = fopen("customer.txt", "r");
    reading_customers(file_customer, maximum, number_customers, number_resources);
    fclose(file_customer);
    
    updateneed(maximum, allocation, need, number_customers, number_resources);

    updateavailable(allocation, resources, number_customers, number_resources);

    int request = 0;
    int release = 0;

    while((c = getc(file_command)) != EOF){

            //update need++++++++++++++++++++++++++
        for (int i = 0; i < number_customers; i++){
            for (int j = 0; j < number_resources; j++){
                need[i][j] = maximum[i][j] - allocation[i][j];
            }
        }

         if (c == 'R'){
            request++;
            release++;
        }
        else if ((c == 'Q') && (request != 0)){
            request = 2;
            release = 0;
        }
        else if ((c == 'L') && (release != 0)){
            request = 0;
            release = 2;
        }
        else if (request == 2){

            for (int i = 0; i < number_customers; i++){
                for (int j = 0; j < number_resources; j++){
                    auxalloc[i][j] = allocation[i][j];
                }
            }

            curr_customer = requests(file_command, allocation, number_customers, number_resources, duplis);

            verifyavailable(allocation, flags, number_customers, number_resources);
            //+++++++++++

            if(error == 0){
                for(int i = 0; i < number_resources; i++){
                    resources[i] = flags[i];
                }

                updateavailable(allocation, resources, number_customers, number_resources);
                //++++++++++++++++++++++++++++++++
                updateneed(maximum, allocation, need, number_customers, number_resources);
                //+++++++++++++++++++++++++++++++++++++

                check_safestate(need, resources, number_customers, number_resources);

                if(error == 5){
                    fprintf(file_result, "The customer %d resquest ", curr_customer);
                    for (int i = 0; i < number_resources; i++){
                        fprintf(file_result, "%d ", duplis[i]);
                    }
                    fprintf(file_result,"was denied because result in an unsafe state\n");

                    for (int i = 0; i < number_customers; i++){
                        for (int j = 0; j < number_resources; j++){
                            allocation[i][j] = auxalloc[i][j];
                        }
                    }
                } else {
                    check_maximum(allocation, maximum, number_customers, number_resources);

                    if (error == 7){
                        fprintf(file_result, "The customer %d resquest ", curr_customer);
                        for (int i = 0; i < number_resources; i++){
                            fprintf(file_result, "%d ", duplis[i]);
                        }
                        fprintf(file_result, "was denied because exceed its maximum allocation\n");
                        for (int i = 0; i < number_customers; i++){
                            for (int j = 0; j < number_resources; j++){
                                allocation[i][j] = auxalloc[i][j];
                            }
                        }
                    } else {
                        fprintf(file_result, "Allocated to customer %d the resources ", curr_customer);
                        for (int i = 0; i < number_resources; i++){
                            fprintf(file_result, "%d ", duplis[i]);
                        }
                        fprintf(file_result,"\n");
                    }

                }

            }else {
                if(error == 2){
                    fprintf(file_result, "The resources ");

                    for(int i = 0; i < number_resources; i++){
                        fprintf(file_result, "%d ", resources[i]);
                    }
                    fprintf(file_result, "are not enough to customer %d request ", curr_customer);

                    for(int i = 0; i < number_resources; i++){
                        fprintf(file_result, "%d ", duplis[i]);
                    }
                    fprintf(file_result, "\n");

                }
                for (int i = 0; i < number_customers; i++){
                    for (int j = 0; j < number_resources; j++){
                        allocation[i][j] = auxalloc[i][j];
                    }

                }
            }
            request = 0;
            error = 0;

        } else if (release == 2){
            curr_customer = releases(file_command, allocation, number_customers, number_resources, duplis);
            if(error == 0){
                fprintf(file_result, "Release from customer %d the resources ", curr_customer);
                for(int i = 0; i < number_resources; i++){
                    fprintf(file_result, "%d ", duplis[i]);
                }

                fprintf(file_result, "\n");
            }
            release = 0;
            error = 0;
        }
        else if (c =='*'){
            printMatriz(number_resources, number_customers, maximum, allocation, need);
            fprintf(file_result, "AVAILABLE");
            for (int a = 0; a < number_resources; a++){
                fprintf(file_result, "%d ", resources[a]); 
            }
            fprintf(file_result, "\n");
        }
    }
    fclose(file_command);
    fclose(file_result);
}

int count_lines(FILE *file_line){
    char c;
    int count = 0;
    //int aux = 0;

    while (c != EOF){
        c = getc(file_line);
        if (c == '\n'){
            count++;
        }
    }
    int aux = count + 1;
    return aux;

    // for (c = getc(file_line); c != EOF; c = getc(file_line)){
    //     if (c == '\n'){
    //         count++;
    //         aux = 0;
    //     } else {
    //         aux = 0;
    //     } if (aux){
    //         count++;
    //     }
    // }
    // return count;
}
int digits(char a){
    if ((a == '0') || (a == '1') || (a == '2') || (a == '3') || (a == '4') || (a == '5') || (a == '6') || (a == '7') || (a == '8') || (a == '9'))
        return 1;
    else
        return 0;
}

void reading_customers(FILE *f, int **matrix, int number_customers, int number_resources){

    int curr_customer = 0, lim = 0, dig = 0;
    char c;

    for (c = getc(f); c != EOF; c = getc(f)){
        if (c == '\n'){
            matrix[curr_customer][lim] = dig;
            dig = 0;
            curr_customer++;
            lim = 0;
        } else if (digits(c)) {
            dig = (dig * 10) + (c - '0');
        } else if (c == ','){
            matrix[curr_customer][lim] = dig;
            dig = 0;
            lim++;
        } else {
            exit(1);
        }
    }
    matrix[curr_customer][lim] = dig;
}

int requests(FILE *f, int **allocation, int number_customers, int number_resources, int *duplis){
    int curr_customer = 0;
    int lim = 0;
    int dig = 0;
    int next = 0;
    char c;

    for(c = getc(f); c != EOF; c = getc(f)){
        if(c == '\n'){
            allocation[curr_customer][lim] += dig;
            duplis[next-1] = dig;

            return curr_customer;
        }

        else if (digits(c)){
            if(next == 0){
                curr_customer = (curr_customer * 10) + (c - '0');
            } else {
                dig = (dig * 10) + (c - '0');
            }
        } else if (c == ' '){
            if(next == 0){
                next++;
            } else {
                allocation[curr_customer][lim] += dig;
                duplis[next-1] = dig;

                dig = 0;
                lim++;
                next++;
            }
        } else {
            return 0;
        }
    }
    allocation[curr_customer][lim] += dig;
    duplis[next-1] = dig;

    return curr_customer;
}

void check_safestate(int **need, int *resources, int number_customers, int number_resources){
    int counter = 0;
    for (int i = 0; i < number_customers; i++){
        for (int j = 0; j < number_resources; j++){
            if((need[i][j] - resources[j]) <= 0){
                counter++;
            }
        }
         if (counter == number_resources){
            return;
        }
        counter = 0;
    }
    error = 5;
}

void check_maximum(int **allocation, int **maximum, int number_customers, int number_resources){
    for (int i = 0; i < number_customers; i++){
        for (int j = 0; j < number_resources; j++){
            if(allocation[i][j] > maximum[i][j]){
                error = 7;
                return;
            }
        }
    
    }
}

int releases(FILE *f, int **allocation, int number_customers, int number_resources, int *duplis){
    int cus = 0;
    int lim = 0;
    int dig = 0;
    int next = 0;
    char c;
    for(c = getc(f); c != EOF; c = getc(f)){
        if(c == '\n'){
            allocation[cus][lim] -= dig;
            duplis[next-1] = dig;

            return cus;
        } else if (digits(c)){
            if(next == 0){
                cus = (cus * 10) + (c - '0');
            } 
            else {
                dig = (dig * 10) + (c - '0');
            }
        } else if (c == ' '){
            if(next == 0){
                next++;
            } else if (next > number_resources){
                error = 1;
                return 0;

            }
            else {
                allocation[cus][lim] -= dig;
                duplis[next-1] = dig;

                dig = 0;
                lim++;
                next++;
            }
        }else{
            return 0;
        }
    }
}


void printMatriz(int number_resources, int number_customers, int **maximum, int **allocation, int **need){
    

    fprintf(file_result, "MAXIMUM");
    
    
    for (int i = 0; i < (number_resources * 2) - 7; i++){
        fprintf(file_result, " ");
    }
    fprintf(file_result, "|");
/////////////

    fprintf(file_result, "ALLOCATION");
    for (int i = 0; i < (number_resources * 2) - 10; i++){
        fprintf(file_result, " ");
    }
    fprintf(file_result, "| ");
////////////
    int aux = number_resources * 2;
    int aux2 = number_resources * 2;
    fprintf(file_result, "NEED\n");
    for(int a = 0; a < number_customers; a++){
        for(int b = 0; b < number_resources; b++){
            fprintf(file_result, "%d ", maximum[a][b]);
        }

        while(aux < 7){
            fprintf(file_result, " ");
            aux++;
        }
        aux = number_resources * 2;
        fprintf(file_result, "|");
        for(int b = 0; b < number_resources; b++){
            fprintf(file_result, "%d ", allocation[a][b]);
        }
        while (aux2 < 10){
            fprintf(file_result, " ");
            aux2++;
        }
        aux2 = number_resources * 2;
        fprintf(file_result, "|");

        for(int b = 0; b < number_resources; b++){
            fprintf(file_result, "%d ", need[a][b]);
        }

        fprintf(file_result, "\n");
    }
}   


//update need
void updateneed(int **maximum, int **allocation, int **need, int number_customers, int number_resources){

for (int i = 0; i < number_customers; i++){
    for (int j = 0; j < number_resources; j++){
        need[i][j] = maximum[i][j] - allocation[i][j];
    }
}
}

void updateavailable(int **allocation, int *resources, int number_customers, int number_resources){
    //update available
int acaux[number_resources];
for(int i = 0; i < number_resources; i++){
    acaux[i] = 0;
}
for (int i = 0; i < number_resources; i++){
    for(int j = 0; j < number_customers; j++){
        acaux[i] = acaux[i] + allocation[j][i];
    }
}
for(int i = 0; i < number_resources; i++){
    resources[i] -= acaux[i];
}
}

void verifyavailable(int **allocation, int *res, int number_customers, int number_resources){
    //verify available
    int acaux[number_resources];
    for(int i = 0; i < number_resources; i++){
        acaux[i] = 0;
    }
    for (int i = 0; i < number_resources; i++){
        for (int j = 0; j < number_customers; j++){
            acaux[i] += allocation[j][i];
        }
    }
    for(int i = 0; i < number_resources; i++){
        if((res[i] - acaux[i]) < 0){
            error = 2;
        }
    }
}