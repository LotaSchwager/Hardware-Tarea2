#include "lectura.h"

// Sirve para saber si el char entregado es un numero o no
bool comparadorString(char chr)
{
	char numeros[11] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '\0'};
	for (int i = 0; i < strlen(numeros); i++)
	{
		if (chr == numeros[i])
		{
			return true;
		}
	}
	return false;
}

// Convierte el array string en un valor tipo int y lo retorna
int stringTOint(char str[4])
{
	int numero;
	sscanf(str, "%d", &numero);
	return numero;
}

// Lee la linea entregada por el archivo y entrega los valores deseados a las variables correspondientes
void barberiaInfo(char chr[], int numeros[3], int posicion)
{
	int digito = 1, memoria = 0;
	char numero1[] = "   ", numero2[] = "   ", numero3[] = "   ";
	for (int i = 0; i < strlen(chr); i++)
	{
		if (comparadorString(chr[i]))
		{
			switch (digito)
			{
			case 1:
				if (memoria == 0)
				{
					numero1[0] = chr[i];
					memoria++;
				}
				else
				{
					numero1[memoria] = chr[i];
					memoria++;
				}
				break;
			case 2:
				if (memoria == 0)
				{
					numero2[0] = chr[i];
					memoria++;
				}
				else
				{
					numero2[memoria] = chr[i];
					memoria++;
				}
				break;
			case 3:
				if (memoria == 0)
				{
					numero3[0] = chr[i];
					memoria++;
				}
				else
				{
					numero3[memoria] = chr[i];
					memoria++;
				}
				break;
			default:
				printf("ERROR: ALGO PASO!!");
			}
			if (!comparadorString(chr[i + 1]))
			{
				digito++;
				memoria = 0;
			}
		}
	}
	numeros[0] = stringTOint(numero1);
	numeros[1] = stringTOint(numero2);
	numeros[2] = stringTOint(numero3);
}

// Esta función sirve para saber cuantos clientes tiene el archivo entregado
int cantidadClientes()
{
	int size = 0;
	FILE *f = fopen("file0.data", "r");
	if (!f)
	{
		return 0;
	}

	for (char c = getc(f); c != EOF; c = getc(f))
	{
		if (c == '\n')
		{
			size += 1;
		}
	}
	fclose(f);
	return size;
}

// Se rellena con los valorea deseados en array struct creados para almacenarlos
void lectura(Barberia *b, Cliente *c)
{
	char ch[9];
	int lista = -2, numeros[3], posicion = 0;
	FILE *file = fopen("file0.data", "r");
	if (!file)
	{
		exit(-1);
	}
	while (fgets(ch, 9, file) != NULL)
	{
		if (lista == -2)
		{
			barberiaInfo(ch, numeros, posicion);
			b->sillas_espera = numeros[0];
			b->barberos = numeros[1];
			b->sillas_barberos = numeros[2];
			lista++;
			continue;
		}
		if (lista == -1)
		{
			lista++;
			continue;
		}
		if (comparadorString(ch[0]))
		{
			barberiaInfo(ch, numeros, posicion);
			c[posicion].tiempo_llegada = numeros[0];
			c[posicion].tiempo_espera = numeros[1];
			c[posicion].tiempo_corte = numeros[2];
			c[posicion].id = posicion;
			c[posicion].cortando = false;
			posicion++;
		}
	}
	fclose(file);
	return;
}