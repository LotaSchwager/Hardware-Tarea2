#include "lectura.h"

bool esNumero(char caracter){
    char numeros[11] = {'0','1','2','3','4','5','6','7','8','9', '\0'};
    for (int i = 0; i < strlen(numeros); i++){
        if (caracter == numeros[i]){
            return true;
        }
    }
    return false;
}

int aEntero(char str[4]){
    int numero;
    sscanf(str, "%d", &numero);
    return numero;
}

void asignarBarberia(char chr[], int numeros[3], int posicion){
    int digito = 1;
    int memoria = 0;

    char numero1[] = "   ";
    char numero2[] = "   ";
    char numero3[] = "   ";

    for (int i = 0; i < strlen(chr); i++){
        if(esNumero(chr[i])){
            switch (digito){
                case 1:
                    if (memoria == 0){
                        numero1[0] = chr[i];
                        memoria++;
                    } else {
                        numero1[memoria] = chr[i];
                        memoria++;
                    }
                    break;
                
                case 2:
                    if (memoria == 0){
                        numero2[0] = chr[i];
                        memoria++;
                    } else {
                        numero2[memoria] = chr[i];
                        memoria++;
                    }
                    break;
                
                case 3:
                    if (memoria == 0){
                        numero3[0] = chr[i];
                        memoria++;
                    } else {
                        numero3[memoria] = chr[i];
                        memoria++;
                    }
                    break;
                
                default:
                    printf("ERROR: ALGO PASO!!");
            }
            if (!esNumero(chr[i+1])){
                digito++;
                memoria = 0;
            }
        }
    }

    numeros[0] = aEntero(numero1);
    numeros[1] = aEntero(numero2);
    numeros[2] = aEntero(numero3);
}

void lectura(Barberia *b, Cliente *c, char *nombreArchivo, int *error){
    char ch[9];
    int lista = -2;
    int numeros[3];
    int posicion = 0;

    FILE *file = fopen(nombreArchivo, "r");
    if (!file)
    {
        *error = 1;
        return;
    }

    while (fgets(ch, 9, file) != NULL){
        if (lista == -2){
            asignarBarberia(ch, numeros, posicion);
            b->sillasEspera = numeros[0];
            b->barberos = numeros[1];
            b->sillasBarberos = numeros[2];
            printf("Cantidad de barberos: %i\n", b->barberos);
            lista++;
            continue;
        }
        if (lista == -1){
            lista++;
            continue;
        }
        if (esNumero(ch[0])){
            asignarBarberia(ch, numeros, posicion);
            c[posicion].tiempoLlegada = numeros[0];
            c[posicion].tiempoEspera = numeros[1];
            c[posicion].tiempoCorte = numeros[2];
            c[posicion].waiting = true;
            c[posicion].id = posicion;
            posicion++;
        }
    }
    
    fclose(file);
    return;
}