#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include<ncurses.h>


// Definimos los registros como un arreglo global
int registros[4] = {0, 0, 0, 0};

//  obtener el índice del registro basado en su nombre
int obtener_indice_registro(char *nombre) {
    if (strcmp(nombre, "EAX") == 0) return 0;
    if (strcmp(nombre, "EBX") == 0) return 1;
    if (strcmp(nombre, "ECX") == 0) return 2;
    if (strcmp(nombre, "EDX") == 0) return 3;
    return -1; // Error o no es registro
}

// Función para verificar si un token es un número
int es_numero(char *str) {
    if (str == NULL || *str == '\0') return 0;
    //desdivos si es negativo o positivo
    int i = 0;
    if (str[0] != '-') i=1;
    //recorremos el texto
    for ( ;  str[i] != '\0'; i++)
    {
        //verificamos si hay al que nos e un digito
        if(!isdigit(str[i])) return 0;
    }
    return 1; 
}

int es_registro_valido(char *str){
    if (str == NULL) return 0;
    return (
      strcmp(str, "EAX") == 0 ||   
      strcmp(str, "EBX") == 0 ||   
      strcmp(str, "ECX") == 0 ||         
      strcmp(str, "EDX") == 0);
    
}

int main() {
    FILE *archivo;
    char linea[100]; // Buffer para guardar cada renglón leído
    char *token;
    char instruccion[10];
    char operando1[10];
    char operando2[10];
    int PC = 0; // Program Counter 

    //NCURSES
    initscr();             
    cbreak();              
    noecho();               
    scrollok(stdscr, TRUE);

    printw("Iniciando simulacion con NCURSES...\n");
    printw("\n------------------------------------------------------------\n");
    printw("PC\tIR\tEAX\tEBX\tECX\tEDX\tMENSAJE\n");
    printw("\n------------------------------------------------------------\n");
    refresh();

    // fopen
    // Asegúrate de crear un archivo "codigo.txt" con las instrucciones
    //archivo = fopen("codigos_de_error.txt", "r"); 
    archivo = fopen("codigo.txt","r"); 

    if (archivo == NULL) {
        printw("\nError: No se pudo abrir el archivo 'codigo.txt'.\n");
        printw("\nPresiona cualquier tecla para salir...\n");
        refresh();
        getch();
        endwin();
        return 1;
    }



    // Lectura del archivo
    // fgets lee hasta encontrar un salto de linea
    while (fgets(linea, sizeof(linea), archivo)) {
        

        // Eliminar el salto de línea
        linea[strcspn(linea, "\n")] = 0;
        
        // Limpiar buffers 
        strcpy(operando1, ""); 
        strcpy(operando2, "");

        //ontines instrccion
        token = strtok(linea, " ,");
        if (token == NULL) continue; // Línea vacía
        strcpy(instruccion, token);

        // detectando arreglso desconocidos
        // Si no es ninguno de los conocidos, es error.
        if (strcmp(instruccion, "MOV") != 0 && strcmp(instruccion, "ADD") != 0 &&
            strcmp(instruccion, "MUL") != 0 && strcmp(instruccion, "DIV") != 0 &&
            strcmp(instruccion, "INC") != 0 && strcmp(instruccion, "DEC") != 0 &&
            strcmp(instruccion, "END") != 0) {
            
            printw("%d\t%s\tERROR: Instruccion desconocida\n", PC, instruccion);
            refresh(); PC++; continue; // Saltamos a la siguiente línea
        }

        // Caso especial END
        if (strcmp(instruccion, "END") == 0) {
            token = strtok(NULL, " , ");
            if (token != NULL)
            {
                //si hay un nuemro  despues del end hay un error
                printw("%d\tEND\tError: la instrcciin end no lleva parametros\n",PC);
            }else{
                // si es NUll  es corecto
               printw("\n%d\tEND\t%d\t%d\t%d\t%d\n", PC, registros[0], registros[1], registros[2], registros[3]);
               
            }
            refresh();
            break;
        }

        // Obtenemos el primer operando
        token = strtok(NULL, " ,");
        if (token != NULL) strcpy(operando1, token);
        else {  
            // Error si falta el primer operando (Ej: 
            printw("%d\t%s\tERROR: Faltan operandos\n", PC, instruccion);
           refresh(); PC++; continue;
        }

        // REGLA: EL DESTINO DEBE SER REGISTRO 
        if (!es_registro_valido(operando1)) {
            printw("%d\t%s\tERROR: '%s' no es un registro destino valido\n", PC, instruccion, operando1);
            refresh(); PC++; continue;
        }

        // Obtener índice del registro destino (ya sabemos que es válido)
        int idx1 = obtener_indice_registro(operando1);

        // OBTENER SEGUNDO OPERANDO (Si aplica)
        token = strtok(NULL, " ,");
        if (token != NULL) strcpy(operando2, token);

        // Identificar si la instrucción necesita 2 operandos 
        int necesita_dos = (strcmp(instruccion, "INC") != 0 && strcmp(instruccion, "DEC") != 0);

        // --- REGLA: INC/DEC SOLO LLEVAN 1 OPERANDO
        if (!necesita_dos && strlen(operando2) > 0) {
            printf("%d\t%s\tERROR: '%s' solo acepta 1 operando\n", PC, instruccion, instruccion);
            refresh(); PC++; continue;
        }

        // REGLA: MOV/ADD NECESITAN 2 OPERANDOS
        if (necesita_dos && strlen(operando2) == 0) {
             printf("%d\t%s\tERROR: '%s' requiere 2 operandos\n", PC, instruccion, instruccion);
            refresh(); PC++; continue;
        }

        //  PREPARAR VALORES PARA EJECUTAR
        int valor2 = 0;
        if (necesita_dos) {
            if (es_registro_valido(operando2)) {
                valor2 = registros[obtener_indice_registro(operando2)];
            } else if (es_numero(operando2)) {
                valor2 = atoi(operando2);
            } else {
                //  REGLA: OPERANDO 2 INVÁLIDO 
                printf("%d\t%s\tERROR: Segundo operando '%s' invalido\n", PC, instruccion, operando2);
                refresh(); PC++; continue;
            }
        }

        // EJECUCION
        if (strcmp(instruccion, "MOV") == 0) registros[idx1] = valor2;
        else if (strcmp(instruccion, "ADD") == 0) registros[idx1] += valor2;
        else if (strcmp(instruccion, "MUL") == 0) registros[idx1] *= valor2;
        else if (strcmp(instruccion, "DIV") == 0) {
            // --- REGLA: DIVISIÓN POR CERO 
            if (valor2 == 0) {
                printw("%d\tDIV\tERROR: Division por CERO\n", PC);
                refresh(); PC++; continue;
            }
            registros[idx1] /= valor2;
        }
        else if (strcmp(instruccion, "INC") == 0) registros[idx1]++;
        else if (strcmp(instruccion, "DEC") == 0) registros[idx1]--;

        // Imprimir estado actual
        printw("\n%d\t%s\t%d\t%d\t%d\t%d\tok\n", PC, instruccion, registros[0], registros[1], registros[2], registros[3]);
        refresh();
        PC++; 
    }

    // fclose
    fclose(archivo);
 // cerramos NCURSES
    printw("\n------------------------------------------------------------\n");
    printw("\n Simulacion terminada. Presiona cualquier tecla para salir...\n");
    refresh(); 
    
    getch();   // esta isntruccion espera al precionar cualquier teclas
    endwin();

    return 0;
}   