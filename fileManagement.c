# include <stdlib.h>
# include <stdio.h>
# include "fileManagement.h"

void openFile(char filename[], char mode[], FILE **filePointer) {
	*filePointer = fopen(filename, mode);
	if (*filePointer == NULL) {
		printf("Errore nell'apertura del file");
		exit(1);
	}
}
