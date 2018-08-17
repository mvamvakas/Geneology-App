#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "GEDCOMutilities.h"
#include "GEDCOMparser.h"
#include "ctype.h"
int checkValid(char *fileName){
	GEDCOMobject *obj = NULL;
	GEDCOMerror error = createGEDCOM(fileName, &obj);
	if (error.type != OK){
		return 0;
	}
	else{
		return 1;
	}
}

char *getDesStr(char *json, char* fileName, int max){
	GEDCOMobject *obj;
	createGEDCOM(fileName, &obj);
	Individual *i = JSONtoInd(json);
	Node *node = obj->individuals.head;
	Individual *i2 = NULL;
	while(node != NULL){
		i2 = (Individual*)node->data;
		if (compareIndividuals(i,i2) == 0){
			break;
		}
		node = node->next;
	}
	List list = getDescendantListN(obj, i2, max);
	char *l = gListToJSON(list);
	return l;
}

char *getAnsStr(char *json, char* fileName, int max){
	GEDCOMobject *obj;
	createGEDCOM(fileName, &obj);
	Individual *i = JSONtoInd(json);
	Node *node = obj->individuals.head;
	Individual *i2 = NULL;
	while(node != NULL){
		i2 = (Individual*)node->data;
		if (compareIndividuals(i,i2) == 0){
			break;
		}
		node = node->next;
	}
	List list = getAncestorListN(obj, i2, max);
	char *l = gListToJSON(list);
	return l;
}

void addIndiv(char *json, char *fileName){
	GEDCOMobject *obj;
	createGEDCOM(fileName, &obj);
	Individual *i = JSONtoInd(json);
	addIndividual(obj, i);
	printf("%s\n", fileName);
	writeGEDCOM(fileName, obj);
	return;
}

char *updateFileStuffs(char *json, char *fileName){
	GEDCOMobject *obj = JSONtoGEDCOM(json);
	validateGEDCOM(obj);
	writeGEDCOM(fileName, obj);
	deleteGEDCOM(obj);
	return json;
}
char *JSONAllIndivs(char *fileName){
	//printf("plsYes");
	GEDCOMobject *obj;
	GEDCOMerror error = createGEDCOM(fileName, &obj);
	//printf("%s", printError(error));
	link(&obj);
	//writeGEDCOM("file2.ged", obj);
	if (error.type != OK){
		return NULL;
	}
	char *str = malloc(sizeof(char) * 255);
	Node *node = obj->individuals.head;
	strncpy(str, "[", 2);
	while (node != NULL){
		Individual *ind = (Individual *)node->data;
		//printf("|%s|\n", toString(ind->otherFields));
		char *toAdd = JSONwithSex(ind);
		str = realloc(str, (sizeof(char) * (strlen(str) + strlen(toAdd) + 3)));
		strcat(str, toAdd);
		if (node->next != NULL){
			strcat(str, ",");
		}
		node = node->next;
	}
	str = realloc(str, (sizeof(char) * (strlen(str) + 3)));
	strcat(str, "]");
	//printf("%s\n", str);
	return str;
	
}

char *JSONwithSex(Individual *ind){
	char *str;
	int flag = 0;
	if (ind == NULL || ind->givenName == NULL || ind->surname == NULL){
		str = malloc(sizeof(char) * 1);
		strncpy(str, "\0", 1);
		return str;
	}

	str = malloc(sizeof(char) * 500);
	strncpy(str, "{\"givenName\":\"", 16);
	strcat(str, ind->givenName);
	strcat(str,"\",\"surname\":\"");
	strcat(str, ind->surname);
	Node *node = ind->otherFields.head;
	while(node != NULL){
		Field *field = (Field *)node->data;
		//printf("%s:SEX\n", field->tag);
		if (strcmp(field->tag, "SEX") == 0){
			//printf("Hi\n");
			flag = 1;
			strcat(str,"\",\"SEX\":\"");
			strcat(str, field->value);
			break;
		}
		node = node->next;
	}
	if (flag == 0){
		strcat(str,"\",\"SEX\":\"");
		strcat(str, "U");
	}
	strcat(str,"\",\"fSize\":\"");
	int famSize = 1;
	node = ind->families.head;
	while(node != NULL){
		Family *fam = (Family *)node->data;
		if (inFam(fam, ind) == 1){
			if (fam->husband != NULL && fam->wife != NULL){
				famSize++;
			}
			famSize += fam->children.length;
		}
		node = node->next;
	}
	if (famSize == 0){
		famSize = 1;
	}
	char *version = malloc(sizeof(char) * 10);
	sprintf(version, "%d", famSize);

	strcat(str,version);
	free(version);
	strcat(str,"\"}");
	return str;
	
}


char *getGEDCOMSpecs(char *fileName){
	//printf("HI");
	GEDCOMobject *obj;
	//printf("|%s|\n", fileName);
	//printf("%d", 
	createGEDCOM(fileName,&obj);
	char *str;
	if (obj == NULL || validateGEDCOM(obj) != OK){
		str = malloc(sizeof(char) * 1);
		strncpy(str, "\0", 1);
		//printf("HI");
		return str;
	}
	//printf("Hey3");
	str = malloc(sizeof(char) * 255);
	strncpy(str, "{\"source\":\"", 16);
	strcat(str, obj->header->source);
	strcat(str,"\",\"gedcVersion\":\"");
	char *version = malloc(sizeof(char) * 10);
	sprintf(version, "%1.2lf", obj->header->gedcVersion);
	strcat(str,version);
	free(version);
	//printf("Hey4");
	//printf("Hey2");
	strcat(str,"\",\"encoding\":\"");
	if (obj->header->encoding == ANSEL){
		strcat(str,"ANSEL");
	}
	else if (obj->header->encoding == UTF8){
		strcat(str, "UTF-8");
	}
	else if (obj->header->encoding == UNICODE){
		strcat(str,"UNICODE");
	}
	else if (obj->header->encoding == ASCII){
		strcat(str, "ASCII");
	}
	//printf("Hey1");
	strcat(str,"\",\"subName\":\"");
	strcat(str, obj->submitter->submitterName);
	strcat(str,"\",\"subAddress\":\"");
	strcat(str, obj->submitter->address);
	strcat(str,"\",\"numIndivs\":\"");
	version = malloc(sizeof(char) * 10);
	sprintf(version, "%d", obj->individuals.length);
	strcat(str,version);
	free(version);
	strcat(str,"\",\"numFamilies\":\"");
	version = malloc(sizeof(char) * 10);
	sprintf(version, "%d", obj->families.length);
	strcat(str,version);
	free(version);
	strcat(str,"\"}");
	//printf("%s\n", str);
	deleteGEDCOM(obj);
	//printf("Hey");
	return str;
	
}

void removeNewline(char *string){
	if(string[strlen(string) - 1] == '\n' || string[strlen(string) - 1] == '\r') {
		if(string[strlen(string) - 2] == '\n' || string[strlen(string) - 2] == '\r') {
			string[strlen(string) - 2] = '\0';
		}
		else {
			string[strlen(string) - 1] = '\0';
		}
	}
}

int inList(Individual *indiv, List list){
	Node *node = list.head;
	while(node != NULL){
		if (compareIndividuals(indiv, (Individual *)node->data) == 0){
			return 1;
		}
		node = node->next;
	}
	return 0;
}

Individual *makeCopy(const Individual *indiv){
	Individual *copy = malloc(sizeof(Individual));
	copy->givenName = malloc(sizeof(char) * (strlen(indiv->givenName)) + 1);
	strncpy(copy->givenName, indiv->givenName, strlen(indiv->givenName) + 1);
	copy->surname = malloc(sizeof(char) * (strlen(indiv->surname)) + 1);
	strncpy(copy->surname, indiv->surname, strlen(indiv->surname) + 1);
	copy->events = initializeList(&printEvent, &deleteNULL, &compareEvents); 
	copy->families = initializeList(&printFamily, &deleteNULL, &compareFamilies); 
	copy->otherFields = initializeList(&printField, &deleteNULL, &compareFields); 
	
	Node *events = indiv->events.head;
	while(events != NULL){
		insertFront(&copy->events, events->data);
		events = events->next;
	}
	Node *families = indiv->families.head;
	while(families != NULL){
		insertFront(&copy->families, families->data);
		families = families->next;
	}
	Node *otherFields = indiv->otherFields.head;
	while(otherFields != NULL){
		insertFront(&copy->otherFields, otherFields->data);
		otherFields = otherFields->next;
	}
	return copy;
}

void getDes(const GEDCOMobject* obj, List *list, const Individual *person){
	if (obj == NULL || person == NULL){
		return;
	}
	Node *currentFam = obj->families.head;
	while (currentFam != NULL){
		Family *fam = (Family *)currentFam->data;
		if ((equalsIndiv(fam->husband, person) == 1) || (equalsIndiv(fam->wife, person) == 1)){
			Node *children = fam->children.head;
			while (children != NULL){
				insertFront(list, makeCopy(children->data));
				getDes(obj, list, (Individual *)children->data);
				children = children->next;
			}
		}
		currentFam = currentFam->next;
	}
	
	/*
	Node *current = person->otherFields.head;
	while (current != NULL){
		Field *field = (Field *)current->data;
		if (strcmp(field->tag, "FAMS") == 0){
			Node *famList = person->families.head;
			while(famList != NULL){
				Family *fam = (Family *)famList->data;
				Node *famFields = fam->otherFields.head;
				while (famFields != NULL){
					Field *fField = (Field *)famFields->data;
					if (strcmp(fField->tag, "XREF") == 0){
						if (strcmp(fField->value, field->value) == 0){
							Node *children = fam->children.head;
							while (children != NULL){
								Individual *toAdd = (Individual *)children->data;
								insertFront(list, makeCopy(toAdd));
								getDes(list, children->data);
								children = children->next;
							}
						}
					}
					famFields = famFields->next;
				}
				famList = famList->next;
			}
		}
		current = current->next;
	}
	*/
}

int headCount(GEDCOMobject *obj, FILE *fp){
	fseek(fp, 0, SEEK_SET);
	char *str = malloc(sizeof(char) * 266);
	int i = 0;
	int j = 1;
	getLine(fp, str, j);
	while((strcmp(str,"0 TRLR") != 0) || (strcmp(str, "") == 0)){
		if (strcmp(str, "0 HEAD") == 0){
			i++;
		}
		free(str);
		str = malloc(sizeof(char) * 266);
		j++;
		getLine(fp, str, j);
	}
	free(str);
	return i;
}

char *submAddress(GEDCOMobject *obj, FILE *fp, char *subREF){
	fseek(fp, 0, SEEK_SET);
	char *fileString = malloc(sizeof(char) * 256);
	char *subLook = malloc(sizeof(char) * 256);
	strcpy(subLook, "0 ");
	strcat(subLook, subREF);
	int i = 1;
	getLine(fp, fileString, i);
	while(strncmp(fileString, subLook, strlen(subLook)) != 0){
		free(fileString);
		fileString = malloc(sizeof(char) * 256);
		i++;
		getLine(fp, fileString, i);
	}
	fileString[0] = '\0';
	char *address = NULL;
	while (fileString[0] != '0'){
		//fseek(fp, -1, SEEK_SET);
		//printf("%s\n", fileString);
		free(fileString);
		fileString = malloc(sizeof(char) * 256);
		i++;
		getLine(fp, fileString, i);
		while(checkContinue(fp, &fileString, i) == 1){
			i++;
		}
		//i = checkContinue(fp, fileString, i);
		if (strncmp(fileString, "1 ADDR", 6) == 0){
			char *token = strtok(fileString, " ");
			token = strtok(NULL, " ");
			token = strtok(NULL, " ");
			address = malloc(sizeof(char) * 256);
			strncpy(address, "", 1);
			while(token != NULL){
				strcat(address, token);
				strcat(address, " ");
				token = strtok(NULL, " ");
			}
			address[strlen(address)-1] = '\0';
		}
	}
	free(fileString);
	free(subLook);
	fseek(fp, 0, SEEK_CUR);
	return address;
}

void removeEnds(char *string){
	int i = 0;
	string[strlen(string) - 1] = '\0';
	for (i = 1; i < strlen(string); i ++){
		string[i - 1] = string[i];
	}
	string[strlen(string) - 1] = '\0';
}

int checkNULL(void *check){
	if (check == NULL){
		return 1;
	}
	else {
		return 0;
	}
}

Event *createEvent(char type[5], char *date, char *place){
	Event *event = malloc(sizeof(Event));
	event->date = malloc(sizeof(char) * strlen(date) + 1);
	event->place = malloc(sizeof(char) * strlen(place) + 1);
	strncpy(event->type, type, 5);
	strncpy(event->date, date, strlen(date) + 1);
	strncpy(event->place, place, strlen(place) + 1);
	
	return event;
}

Field *createField(char *tag, char *value){
	Field *field = malloc(sizeof(Field));
	field->tag = malloc(sizeof(char) * strlen(tag) + 1);
	field->value = malloc(sizeof(char) * strlen(value) + 1);
	
	strncpy(field->tag, tag, strlen(tag) + 1);
	strncpy(field->value, value, strlen(value) + 1);
	
	return field;
}

int findCarriageReturn(FILE *fp){
	int i = 0;
	int j = 0;
	char c = fgetc(fp);
	while (c != '\n' && c != '\r' && !feof(fp)){
		i++;
	    c = fgetc(fp);
	}
	while (c == '\n' || c == '\r'){
		c = fgetc(fp);
		j++;
	}
	fseek(fp, -(i + j), SEEK_CUR);
	return j;
	/*
	if(string[strlen(string) - 1] == '\n' || string[strlen(string) - 1] == '\r') {
		if(string[strlen(string) - 2] == '\n' || string[strlen(string) - 2] == '\r') {
			return 2;
		}
		else {
			return 1;
		}
	}
	* */
}
GEDCOMerror subCount(GEDCOMobject *obj, FILE *fp){
	char *line = malloc(sizeof(char) * 256);
	GEDCOMerror error;
	error.type = OK;
	error.line = 0;
	error = getLine(fp, line, 0);
	while(strcmp(line, "0 TRLR") != 0){
		char *token = strtok(line, " ");
		if (checkNULL(line) == 1){
			free(line);
			error.type = INV_RECORD;
			return error;
		}
		token = strtok(NULL, " ");
		if (checkNULL(line) == 1){
			free(line);
			error.type = INV_RECORD;
			return error;
		}
		token = strtok(NULL, " ");
		if(token != NULL){
			if (strcmp(token, "SUBM") == 0){
				free(line);
				fseek(fp, 0, SEEK_SET);
				return error;
			}	
		}
		free(line);
		line = malloc(sizeof(char) * 255);
		error = getLine(fp, line, 0);
	}
	free(line);
	error.type = INV_GEDCOM;
	error.line = -1;
	return error;
	
}
int isFamEvent(char *string){
	if (strcmp(string, "ANUL") == 0){
		return 1;
	}
	else if (strcmp(string, "CENS") == 0){
		return 1;
	}
	else if (strcmp(string, "DIV") == 0){
		return 1;
	}
	else if (strcmp(string, "DIVF") == 0){
		return 1;
	}
	else if (strcmp(string, "ENGA") == 0){
		return 1;
	}
	else if (strcmp(string, "MARR") == 0){
		return 1;
	}
	else if (strcmp(string, "MARB") == 0){
		return 1;
	}
	else if (strcmp(string, "MARC") == 0){
		return 1;
	}
	else if (strcmp(string, "MARL") == 0){
		return 1;
	}
	else if (strcmp(string, "MARS") == 0){
		return 1;
	}
	else if (strcmp(string, "EVEN") == 0){
		return 1;
	}
	else {
		return 0;
	}
}

int isEvent(char *string){
	if (strcmp(string, "ADOP") == 0){
		return 1;
	}
	else if (strcmp(string, "BIRT") == 0){
		return 1;
	}
	else if (strcmp(string, "BAPM") == 0){
		return 1;
	}
	else if (strcmp(string, "BARM") == 0){
		return 1;
	}
	else if (strcmp(string, "BASM") == 0){
		return 1;
	}
	else if (strcmp(string, "BLES") == 0){
		return 1;
	}
	else if (strcmp(string, "BURI") == 0){
		return 1;
	}
	else if (strcmp(string, "CENS") == 0){
		return 1;
	}
	else if (strcmp(string, "CHR") == 0){
		return 1;
	}
	else if (strcmp(string, "CHRA") == 0){
		return 1;
	}
	else if (strcmp(string, "CONF") == 0){
		return 1;
	}
	else if (strcmp(string, "CREM") == 0){
		return 1;
	}
	else if (strcmp(string, "DEAT") == 0){
		return 1;
	}
	else if (strcmp(string, "EMIG") == 0){
		return 1;
	}
	else if (strcmp(string, "FCOM") == 0){
		return 1;
	}
	else if (strcmp(string, "GRAD") == 0){
		return 1;
	}
	else if (strcmp(string, "IMMI") == 0){
		return 1;
	}
	else if (strcmp(string, "NATU") == 0){
		return 1;
	}
	else if (strcmp(string, "ORDN") == 0){
		return 1;
	}
	else if (strcmp(string, "RETI") == 0){
		return 1;
	}
	else if (strcmp(string, "PROB") == 0){
		return 1;
	}
	else if (strcmp(string, "WILL") == 0){
		return 1;
	}
	else if (strcmp(string, "EVEN") == 0){
		return 1;
	}
	else{
		return 0;
	}
}

int checkContinue(FILE *fp, char **string, int lineNum){
	//printf("CONTINUE - %s\n", *string);
	if (strcmp(*string, "0 TRLR") == 0){
		return 0;
	}
	if (strcmp(*string, "") == 0){
		return 0;
	}
	 
	char *fileNewString = malloc(sizeof(char) * 256);
	GEDCOMerror error = getLine(fp, fileNewString, lineNum);
	if (error.type != OK){
		//fseek(fp, -(strlen(fileNewString)), SEEK_CUR);
		/*char c = fgetc(fp);
		while((c != '\n') && (c != '\r')){
			printf("%c\n", c);
			fseek(fp, -2, SEEK_CUR);
		}
		*/ 
		free(fileNewString);
		return 0;
	}
	if (fileNewString == NULL){
		free(fileNewString);
		return 0;
    }
    if (strcmp(fileNewString, "") == 0){
		free(fileNewString);
		return 0;
	}
    else{
		if ((fileNewString[2] == 'C') && (fileNewString[3] == 'O') && (fileNewString[4] == 'N') && (fileNewString[5] == 'T')){
			*string = realloc(*string, sizeof(char) * (strlen(*string) + strlen(fileNewString) + 2));
			char *token = strtok(fileNewString, " ");
			if (checkNULL(token) == 1){
				free(fileNewString);
				return 0;
			}
			token = strtok(NULL, " ");
			if (checkNULL(token) == 1){
				free(fileNewString);
				return 0;
			}
			token = strtok(NULL, " ");
			if (checkNULL(token) == 1){
				free(fileNewString);
				return 0;
			}
			strcat(*string, "\n");
			strcat(*string, token);
			strcat(*string, " ");
			token = strtok(NULL, " ");
			while (token != NULL){
				strcat(*string, token);
				strcat(*string, " ");
				token = strtok(NULL, " ");
			}
			strcat(*string, "\0");
			//*string[strlen(*string) - 1] = '\0';
			free(fileNewString);
			return 1;
		}
		else if ((fileNewString[2] == 'C') && (fileNewString[3] == 'O') && (fileNewString[4] == 'N') && (fileNewString[5] == 'C')){
			*string = realloc(*string, sizeof(char) * (strlen(*string) + strlen(fileNewString) + 2));
			char *token = strtok(fileNewString, " ");
			if (checkNULL(token) == 1){
				free(fileNewString);
				return 0;
			}
			token = strtok(NULL, " ");
			if (checkNULL(token) == 1){
				free(fileNewString);
				return 0;
			}
			token = strtok(NULL, " ");
			if (checkNULL(token) == 1){
				free(fileNewString);
				return 0;
			}
			strcat(*string, token);
			strcat(*string, " ");
			token = strtok(NULL, " ");
			while (token != NULL){
				strcat(*string, token);
				strcat(*string, " ");
				token = strtok(NULL, " ");
			}
			strcat(*string, "\0");
			//*string[strlen(*string) - 1] = '\0';
			free(fileNewString);
			return 1;
		}
	}
	fseek(fp, -(strlen(fileNewString)), SEEK_CUR);
	char c = fgetc(fp);
	while(c != '\n' && c != '\r'){
		fseek(fp, -2, SEEK_CUR);
		c = fgetc(fp);
	}
	free(fileNewString);
	return 0;
}

GEDCOMerror getHead(FILE *fp, Header **head){
	
	fseek(fp, 0, SEEK_SET);
	int lineNum = 1;
	GEDCOMerror error;
	error.type = OK;
	error.line = lineNum;
	*head = malloc(sizeof(Header));
	(*head)->otherFields = initializeList(&printField, &deleteField, &compareFields); 
	char *fileString = malloc(sizeof(char) * 256);
	error = getLine(fp, fileString, lineNum);
	//printf("%s\n", fileString);
	if (error.type != OK){
		free(fileString);
		return error;
	}
	//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
	while(checkContinue(fp, &fileString, lineNum) == 1){
		lineNum++;
	}
	if (error.type != OK){
		return error;
	}
	free(fileString);
	/*
	 * Getting the source of the header
	 */ 
	fileString = malloc(sizeof(char) * 256);
	lineNum ++;
	error = getLine(fp, fileString, lineNum);
	//printf("|%s|\n", fileString);
	if (error.type != OK){
		free(fileString);
		return error;
	}
	while(checkContinue(fp, &fileString, lineNum) == 1){
		lineNum++;
	}
	//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
	//printf("-%s-\n",fileString);
	if (error.type != OK){
		return error;
	}
	char *token = strtok(fileString, " ");
	token = strtok(NULL, " ");
	//printf("%s\n", token);
	
	if (strcmp(token, "SOUR") != 0){
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	token = strtok(NULL, " ");
	/*
	 * Source Name
	 */ 
	strncpy((*head)->source, token, strlen(token) + 1);
	free(fileString);
	fileString = malloc(sizeof(char) * 256);
	lineNum ++;
	error = getLine(fp, fileString, lineNum);
	while(checkContinue(fp, &fileString, lineNum) == 1){
		lineNum++;
	}
	while(fileString[0] != '1' && strcmp(fileString,"") != 0 && strcmp(fileString,"0 TRLR") != 0) {
		token = strtok(fileString, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}  
		token = strtok(NULL, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}  
		Field *field = malloc(sizeof(Field));
		field->tag = malloc(sizeof(char) * 5);
		strncpy(field->tag, token, 5);
		token = strtok(NULL, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}                
	    char *str = malloc(sizeof(char) * 256);
		strncpy(str, "", 1);
		strcat(str, token);
		strcat(str, " ");
		token = strtok(NULL, " ");
		while (token != NULL){
			strcat(str, token);
			strcat(str, " ");
			token = strtok(NULL, " ");
		}
		//removeNewline(str);
		str[strlen(str) - 1] = '\0';
		field->value = malloc(sizeof(char) * strlen(str) + 1);
		strncpy(field->value, str, strlen(str) + 1);
		free(str);
	    insertFront(&(*head)->otherFields, field);
	    free(fileString);
	    fileString = malloc(sizeof(char) * 256);
	    lineNum++;
	    error = getLine(fp, fileString, lineNum);
		while(checkContinue(fp, &fileString, lineNum) == 1){
			lineNum++;
		}
		//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
	}
	
	token = strtok(fileString, " ");
	if (checkNULL(token) == 1){
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	token = strtok(NULL, " ");
	if (checkNULL(token) == 1){
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	while((strcmp(token, "GEDC") != 0) && (strcmp(token, "0 TRLR") != 0) && (strcmp(token, "") != 0)){
		strncpy(fileString, "", 1);
		while(fileString[0] != '1'){
			free(fileString);
			fileString = malloc(sizeof(char) * 256);
			lineNum ++;
			error = getLine(fp, fileString, lineNum);
			while(checkContinue(fp, &fileString, lineNum) == 1){
				lineNum++;
			}
			if (strcmp(fileString, "0 TRLR") == 0){
				error.type = INV_HEADER;
				error.line = lineNum;
				free(fileString);
				return error;
			}
		}
		if (fileString[0] == '0'){
			error.type = INV_HEADER;
			error.line = lineNum;
			free(fileString);
			return error;
		}
		token = strtok(fileString, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}
		token = strtok(NULL, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}
    }
    if (strcmp(token, "GEDC") == 0){
		free(fileString);
		fileString = malloc(sizeof(char) * 256);
		lineNum ++;
		error = getLine(fp, fileString, lineNum);
		while(checkContinue(fp, &fileString, lineNum) == 1){
			lineNum++;
		}
		//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
		token = strtok(fileString, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}
		token = strtok(NULL, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}
		if (strcmp(token, "VERS") == 0){
			token = strtok(NULL, " ");
			if (checkNULL(token) == 1){
				free(fileString);
				error.type = INV_HEADER;
				error.line = lineNum;
				return error;
			}
			(*head)->gedcVersion = atof(token);
		}
		else {
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}
		lineNum++;
		error = getLine(fp, fileString, lineNum);
		while(checkContinue(fp, &fileString, lineNum) == 1){
			lineNum++;
		}
		//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
		while(fileString[0] != '1'){
			token = strtok(fileString, " ");
			if (checkNULL(token) == 1){
				free(fileString);
				error.type = INV_HEADER;
				error.line = lineNum;
				return error;
			}  
			token = strtok(NULL, " ");
			if (checkNULL(token) == 1){
				free(fileString);
				error.type = INV_HEADER;
				error.line = lineNum;
				return error;
			}  
			Field *field = malloc(sizeof(Field));
			field->tag = malloc(sizeof(char) * 5);
			strncpy(field->tag, token, 5);
			token = strtok(NULL, " ");
			if (checkNULL(token) == 1){
				free(fileString);
				error.type = INV_HEADER;
				error.line = lineNum;
				return error;
			}                
			char *str = malloc(sizeof(char) * 256);
			strncpy(str, "", 1);
			strcat(str, token);
			strcat(str, " ");
			token = strtok(NULL, " ");
			while (token != NULL){
				strcat(str, token);
				strcat(str, " ");
				token = strtok(NULL, " ");
			}
			//removeNewline(str);
			str[strlen(str) - 1] = '\0';
			field->value = malloc(sizeof(char) * strlen(str) + 1);
			strncpy(field->value, str, strlen(str) + 1);
			free(str);
			insertFront(&(*head)->otherFields, field);
			free(fileString);
			fileString = malloc(sizeof(char) * 256);
			lineNum++;
			error = getLine(fp, fileString, lineNum);
			while(checkContinue(fp, &fileString, lineNum) == 1){
				lineNum++;
			}
			//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
		}
		
	}
	else {
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	//printf("HEY");
	token = strtok(fileString, " ");
	if (checkNULL(token) == 1){
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	token = strtok(NULL, " ");
	if (checkNULL(token) == 1){
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	while(strcmp(token, "CHAR") != 0){	
		strncpy(fileString, "", 1);
		while(fileString[0] != '1' && !feof(fp)){
			free(fileString);
			fileString = malloc(sizeof(char) * 256);
			lineNum ++;
			error = getLine(fp, fileString, lineNum);
			while(checkContinue(fp, &fileString, lineNum) == 1){
				lineNum++;
			}
			if (strcmp(fileString, "0 TRLR") == 0){
				error.type = INV_HEADER;
				error.line = lineNum;
				free(fileString);
				return error;
			}
			//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
		}
		if (fileString[0] == '0'){
			error.type = INV_HEADER;
			error.line = lineNum;
			free(fileString);
			return error;
		}
		if (strcmp(fileString, "0 TRLR") == 0){
			error.type = INV_HEADER;
			error.line = lineNum;
			free(fileString);
			return error;
		}
		token = strtok(fileString, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}
		token = strtok(NULL, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}
    }
    
    if (strcmp(token,"CHAR") != 0){
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
    token = strtok(NULL, " ");
	if (checkNULL(token) == 1){
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	if (strcmp(token, "ANSEL") == 0){
		(*head)->encoding = ANSEL;
	}
	else if (strcmp(token, "UTF-8") == 0){
		(*head)->encoding = UTF8;
	}
	else if (strcmp(token, "UNICODE") == 0){
		(*head)->encoding = UNICODE;
	}
	else if (strcmp(token, "ASCII") == 0){
		(*head)->encoding = ASCII;
	}
	else {
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	free(fileString);
	fileString = malloc(sizeof(char) * 256);
	lineNum ++;
	error = getLine(fp, fileString, lineNum);
	while(checkContinue(fp, &fileString, lineNum) == 1){
		lineNum++;
	}
	//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
	token = strtok(fileString, " ");
	if (checkNULL(token) == 1){
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	token = strtok(NULL, " ");
	if (checkNULL(token) == 1){
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	while((strcmp(token, "SUBM") != 0) && (strcmp(token, "0 TRLR") != 0) && (strcmp(token, "") != 0)){	
		free(fileString);
		fileString = malloc(sizeof(char) * 256);
		lineNum ++;
		error = getLine(fp, fileString, lineNum);
		while(checkContinue(fp, &fileString, lineNum) == 1){
			lineNum++;
		}
		//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
		if (error.type != OK){
			free(fileString);
			return error;
		}
		/* ERROR HAPPENS HERE */
		if (strcmp(fileString, "0 TRLR") == 0){
			error.type = INV_HEADER;
			error.line = lineNum;
			free(fileString);
			return error;
		}
		token = strtok(fileString, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}
		token = strtok(fileString, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}
	}
	if (strcmp(token, "SUBM") == 0){
		token = strtok(NULL, " ");
		if (checkNULL(token) == 1){
			free(fileString);
			error.type = INV_HEADER;
			error.line = lineNum;
			return error;
		}
		Field *field = createField("SUBM", token);
		insertFront(&(*head)->otherFields, field);
	}
	else {
		free(fileString);
		error.type = INV_HEADER;
		error.line = lineNum;
		return error;
	}
	/*char *name = malloc(sizeof(char) * 255);
	if (strcmp(token, "NAME") == 0){
		token = strtok(NULL, " ");
		while (token != NULL){
			strcat(name, token);
			strcat(name, " ");
			token = strtok(NULL, " ");
		}
		printf("%s", name);
	}*/
	free(fileString);
	
	
	
	return error;
}

GEDCOMerror getLine(FILE *fp, char *string, int lineNum){
	GEDCOMerror error;
	error.type = OK;
	error.line = lineNum;
	int i = 0;
	
	char c = fgetc(fp);
	string[i] = c;
	//printf("%d:%c",i, c);
	while (c != '\n' && c != '\r' && !feof(fp)){
		i++;
		if (i > 254){
			//printf("%s\n", string);
		    error.type = INV_RECORD;
	        error.line = lineNum;
	        return error;
	    }
	    c = fgetc(fp);
		string[i] = c;
		//printf("STRING: %s\n", string);	
	}
	string[i] = '\0';
	while(isalnum(c) == 0 && !feof(fp)){
		c = fgetc(fp);
	}
	//printf("%s\n", string);
	/*if (strcmp(string, "") == 0){
		error.type = INV_GEDCOM;
		error.line = lineNum;
		printf("HI");
		return error;
	}
	 */
	fseek(fp, -1, SEEK_CUR);
	 
	return error;
}

void link(GEDCOMobject **obj){
	/*
	 *linking all individuals
	 */ 
	 printf("link");
	Node *current = (*obj)->individuals.head;
	while( current != NULL){
		Individual *indiv = (Individual *)current->data;
		//printf("|%s|\n", indiv->surname);
		Node *otherFieldsIndiv = (Node *)indiv->otherFields.head;
		while (otherFieldsIndiv != NULL){
			Field *indivField = (Field *)otherFieldsIndiv->data;
			//printf("|%s|:|%s|\n", indivField->tag, indivField->value);
			if ((strcmp(indivField->tag, "FAMS") == 0) || (strcmp(indivField->tag, "FAMC") == 0)){
				Node *familyList = (Node *)(*obj)->families.head;
				while (familyList != NULL){
					Family *family = (Family *)familyList->data;
					family->husband = NULL;
					family->wife = NULL;
					Node *familyFieldList = (Node *)family->otherFields.head;
					while (familyFieldList != NULL){
						Field *famField = (Field *)familyFieldList->data;
						//printf("famField: |%s|:|%s|\n", famField->tag, famField->value);
						if (strcmp(famField->tag, "XREF") == 0){
							if (strcmp(famField->value, indivField->value) == 0){
								//printf("found %s:%s\n", famField->value, indivField->value);
							    insertFront(&indiv->families, family);
							}
							
						}
						familyFieldList = familyFieldList->next;
					}
					familyList = familyList->next;
				}
			}
			otherFieldsIndiv = otherFieldsIndiv->next;
		}
		current = current->next;
	}
	current = (*obj)->families.head;
	while (current != NULL){
		Family *family = (Family *)current->data;
		Node *famFieldsList = family->otherFields.head;
		while (famFieldsList != NULL){
			Field *famField = (Field *)famFieldsList->data;
			//printf("|%s:%s|\n", famField->tag, famField->value);
			if (strcmp(famField->tag, "WIFE") == 0){
				Node *indivList = (*obj)->individuals.head;
				while(indivList != NULL){
					Individual *indiv = (Individual *)indivList->data;
					Node *indivFieldList = (Node *)indiv->otherFields.head;
					while(indivFieldList != NULL){
						Field *indivField = (Field *)indivFieldList->data;
						if ((strcmp(indivField->tag, "XREF") == 0) && (strcmp(indivField->value, famField->value) == 0)){
							family->wife = indiv;
						}
						indivFieldList = indivFieldList->next;
					}
					indivList = indivList->next;
				}
			}
			else if (strcmp(famField->tag, "HUSB") == 0){
				Node *indivList = (*obj)->individuals.head;
				while(indivList != NULL){
					Individual *indiv = (Individual *)indivList->data;
					Node *indivFieldList = (Node *)indiv->otherFields.head;
					while(indivFieldList != NULL){
						Field *indivField = (Field *)indivFieldList->data;
						if ((strcmp(indivField->tag, "XREF") == 0) && (strcmp(indivField->value, famField->value) == 0)){
							family->husband = indiv;
						}
						indivFieldList = indivFieldList->next;
					}
					indivList = indivList->next;
				}
			}
			else if (strcmp(famField->tag, "CHIL") == 0){
				Node *indivList = (*obj)->individuals.head;
				while(indivList != NULL){
					Individual *indiv = (Individual *)indivList->data;
					Node *indivFieldList = (Node *)indiv->otherFields.head;
					while(indivFieldList != NULL){
						Field *indivField = (Field *)indivFieldList->data;
						if ((strcmp(indivField->tag, "XREF") == 0) && (strcmp(indivField->value, famField->value) == 0)){
							insertFront(&family->children, indiv);
						}
						indivFieldList = indivFieldList->next;
					}
					indivList = indivList->next;
				}
			}
			
			famFieldsList = famFieldsList->next;
		}
		
		current = current->next;
	}
	/*
	 * Removing the XREF from the list of individuals
	 */ 
	
	current = (*obj)->individuals.head;
	while(current != NULL){
		Individual *indiv = (Individual *)current->data;
		Node *fields = indiv->otherFields.head;
		while(fields != NULL){
			Field *field = (Field *)fields->data;
			if ((strcmp(field->tag, "XREF") == 0) || (strcmp(field->tag, "FAMS") == 0) || (strcmp(field->tag, "FAMC") == 0)){
				fields = fields->next;
				printf("hey");
				deleteDataFromList(&indiv->otherFields, field);
				deleteField(field);
			}
			else {
				fields = fields->next;
			}
			
			
		}
		
		current = current->next;
	}
	current = (*obj)->header->otherFields.head;
	while(current != NULL){
		Field *field = (Field *)current->data;
		if (strcmp(field->tag, "SUBM") == 0){
			current = current->next;
			deleteDataFromList(&(*obj)->header->otherFields, field);
			deleteField(field);
			break;
		}
		else{
			current = current->next;
		}
	}
	/*
	 * Removing the XREF from the list of families
	 */ 
	 
	/*current = (*obj)->families.head;
	while(current != NULL){
		Family *fam = (Family *)current->data;
		Node *fields = fam->otherFields.head;
		while(fields != NULL){
			Field *field = (Field *)fields->data;
			if ((strcmp(field->tag, "XREF") == 0) || (strcmp(field->tag, "HUSB") == 0) || (strcmp(field->tag, "WIFE") == 0)){
				
				fields = fields->next;
				deleteDataFromList(&fam->otherFields, field);
				deleteField(field);
			}
			else{
				fields = fields->next;
			}
			
		}
		
		current = current->next;
	} 
	*/ 
}
void deleteNULL(void *toBeDeleted){}

GEDCOMerror writeHead(FILE *fp, const GEDCOMobject *obj){
	GEDCOMerror error;
	error.type = OK;
	error.line = 0;
	Header *head = obj->header;
	
	fwrite("0 HEAD\n", sizeof(char), 7, fp);
	fwrite("1 SOUR ", sizeof(char), 7, fp);
	fwrite(head->source, sizeof(char), strlen(head->source), fp);
	fwrite("\n", sizeof(char), 1, fp);
	
	/*fwrite("2 NAME ", sizeof(char), 7, fp);
	fwrite(head->, sizeof(char), strlen(head->submitter->submitterName), fp);
	fwrite("\n", sizeof(char), 1, fp);
	*/
	fwrite("1 GEDC\n", sizeof(char), 7, fp);
	fwrite("2 VERS ", sizeof(char), 7, fp);
	
	fprintf(fp, "%1.1lf", head->gedcVersion);
	//fwrite(head->gedcVersion, sizeof(float), 1, fp);
	fwrite("\n", sizeof(char), 1, fp);

	fwrite("2 FORM LINEAGE-LINKED\n", sizeof(char), 22, fp);
	fwrite("1 CHAR ", sizeof(char), 7, fp);
	if (head->encoding == 0){
		//strncpy(encoding, "ANSEL", 6);
		fwrite("ANSEL\n", sizeof(char), 6, fp);
	}
	else if (head->encoding == 1){
		//strncpy(encoding, "UTF8", 5);
		fwrite("UTF-8\n", sizeof(char), 6, fp);
	}
	else if (head->encoding == 2){
		//strncpy(encoding, "UNICODE", 8);
		fwrite("UNICODE\n", sizeof(char), 7, fp);
	}
	else if (head->encoding == 3){
		//strncpy(encoding, "ASCII", 6);
		fwrite("ASCII\n", sizeof(char), 6, fp);
	}
	fwrite("1 SUBM @SUB1@\n", sizeof(char), 14, fp);
	return error;
}
GEDCOMerror writeSubmitter(FILE *fp, const GEDCOMobject *obj){
	GEDCOMerror error;
	//char ref[50] = "@I0001@";
	error.type = OK;
	error.line = 0;
	fwrite("0 @SUB1@ SUBM\n", sizeof(char), 14, fp);
	fwrite("1 NAME ", sizeof(char), 7, fp);
	fwrite(obj->submitter->submitterName, sizeof(char), strlen(obj->submitter->submitterName), fp);
	fwrite("\n", sizeof(char), 1, fp);
	if (strcmp(obj->submitter->address, "") != 0){
		fprintf(fp, "1 ADDR %s\n", obj->submitter->address);
	}
	return error;
}

GEDCOMerror writeIndivs(FILE *fp, const GEDCOMobject *obj){
	
	/*
	 * For families the individuals are a part of
	 * create a list with references to all of the families
	 * and use to establish them, do the same with all the
	 * individuals
	 */ 
	
	List iList = initializeList(&printIndividual, &deleteNULL, &compareIndividuals); 
	List fList = initializeList(&printFamily, &deleteNULL, &compareFamilies); 
	List iAdded = initializeList(&printIndividual, &deleteNULL, &compareIndividuals); 
	GEDCOMerror error;
	char ref[50] = "@I0000@";
	char fRef[50] = "@F0000@";
	error.type = OK;
	error.line = 0;
	Node *famNode = obj->families.head;
	while (famNode != NULL){
		int i = 0;
		fRef[strlen(fRef)] = '\0';
		for (i = strlen(fRef) - 2; i >= 0; i--){
			while(fRef[i] == '9'){
				fRef[i] = '0';
				i--;
			}
			fRef[i] = fRef[i] + 1;
			break;
			
		}
		fRef[strlen(fRef) - 1] = '@';
		/*printf("%s\n", fRef);
		fwrite("0 ", sizeof(char), 2, fp);
		fwrite(fRef, sizeof(char), strlen(fRef), fp);
		fwrite(" FAM\n", sizeof(char), 5, fp);
		*/
		Family *family = (Family *)famNode->data;
		//char *famP = printFamily(family);
		//printf("%s\n", famP);
		//printf("HUSB FAM NUM %d", family->husband->families.length);
		//free(famP);
		famRef *fam = malloc(sizeof(famRef));
		strcpy(fam->reference, fRef);
		fam->family = *family;
		insertBack(&fList, fam);
		famNode = famNode->next;
	}
	Node *indivNode = obj->individuals.head;
	while (indivNode != NULL){
		/*
		 * This will randomly create the reference
		 */ 
		int i = 0;
		ref[strlen(ref)] = '\0';
		for (i = strlen(fRef) - 2; i >= 0; i--){
			while(ref[i] == '9'){
				ref[i] = '0';
				i--;
			}
			ref[i] = ref[i] + 1;
			break;
			
		}
		ref[strlen(ref) - 1] = '@';
		fwrite("0 ", sizeof(char), 2, fp);
		fwrite(ref, sizeof(char), strlen(ref), fp);
		fwrite(" INDI\n", sizeof(char), 6, fp);
		fwrite("1 NAME ", sizeof(char), 7, fp);
		//fwrite("1 SEX U", sizeof(char), 7, fp);
		Individual *indiv = (Individual *)indivNode->data;
		indivRef *indi = malloc(sizeof(indivRef));
		strcpy(indi->reference, ref);
		indi->individual = *indiv;
		insertBack(&iList, indi);
		fprintf(fp, "%s /%s/", indiv->givenName, indiv->surname);
		fwrite("\n", sizeof(char), 1, fp);
		/*
		 *Other Fields
		 */
		Node *otherFields = indiv->otherFields.head;
		while (otherFields != NULL){
			Field *field = (Field *)otherFields->data;
			if ((strcmp(field->tag, "GIVN") == 0) || (strcmp(field->tag, "SURN") == 0)){
				fprintf(fp, "2 %s %s\n", field->tag, field->value);
			}
			else{
				fprintf(fp, "1 %s %s\n", field->tag, field->value);
			}
			otherFields = otherFields->next;
		}
		/*
		 * Events
		 */ 
		Node *events = indiv->events.head;
		while (events != NULL){
			Event *event = (Event *)events->data;
			fprintf(fp, "1 %s\n", event->type);
			if (strcmp(event->date, "") != 0){
				fprintf(fp, "2 DATE %s\n", event->date);
			
			}
			if (strcmp(event->place, "") != 0){
				fprintf(fp, "2 PLAC %s\n", event->place);
			}
			events = events->next;
		}
		
		Node *families = (Node *)indiv->families.head;
		while(families != NULL){
			int flag = 0;
			Node *node = fList.head;
			while(node != NULL){
				famRef *fam = (famRef *)node->data;
				if (inFam(&fam->family, indiv) == 1){
					//printf("|%s|\n", fam->reference);
					flag = 1;
					fprintf(fp, "1 FAMS %s\n", fam->reference);
				}
				else if (inFam(&fam->family, indiv) == 2){
					//printf("|%s|\n", fam->reference);
					flag = 1;
					fprintf(fp, "1 FAMC %s\n", fam->reference);
				}
				node = node->next;
			}
			if (flag == 1){
				break;
			}
			 
			families = families->next;
		}
		
		insertBack(&iAdded, indiv);
		//printf("added size %d\n", iAdded.length);
		indivNode = indivNode->next;
	}
	/*Node *famNode = obj->families.head;
	while (famNode != NULL){
		int i = 0;
		fRef[strlen(fRef)] = '\0';
		for (i = strlen(fRef) - 2; i >= 0; i--){
			while(fRef[i] == '9'){
				fRef[i] = '0';
				i--;
			}
			fRef[i] = fRef[i] + 1;
			break;
			
		}
		fRef[strlen(fRef) - 1] = '@';
		fwrite("0 ", sizeof(char), 2, fp);
		fwrite(fRef, sizeof(char), strlen(fRef), fp);
		fwrite(" FAM\n", sizeof(char), 5, fp);
		
		insertBack(&fList, &indi);
		famNode = famNode->next;
	}
	*/
	Node *familyNode = fList.head;
	while (familyNode != NULL){
		famRef *fam = (famRef *)familyNode->data;
		fprintf(fp, "0 %s FAM\n", fam->reference);
		Individual *husband = fam->family.husband;
		Node *people = iList.head;
		while (people != NULL){
			indivRef *ind = people->data;
			if (equalsIndiv(&ind->individual, husband) == 1){
				fprintf(fp, "1 HUSB %s\n", ind->reference);
			}
			people = people->next;
		}
		Individual *wife = fam->family.wife;
		people = iList.head;
		while (people != NULL){
			indivRef *ind = people->data;
			if (equalsIndiv(&ind->individual, wife) == 1){
				fprintf(fp, "1 WIFE %s\n", ind->reference);
			}
			people = people->next;
		}
		/*
		 * Events
		 */ 
		Node *events = fam->family.events.head;
		while (events != NULL){
			Event *event = (Event *)events->data;
			fprintf(fp, "1 %s\n", event->type);
			if (strcmp(event->date, "") != 0){
				fprintf(fp, "2 DATE %s\n", event->date);
			
			}
			if (strcmp(event->place, "") != 0){
				fprintf(fp, "2 PLAC %s\n", event->place);
			}
			events = events->next;
		}
		people = iList.head;
		while (people != NULL){
			indivRef *ind = people->data;
			if (inFam(&fam->family, &ind->individual) == 2){
				fprintf(fp, "1 CHIL %s\n", ind->reference);
			}
			people = people->next;
		}
		
		familyNode = familyNode->next;
	}
	fprintf(fp, "0 TRLR\n");
	clearList(&iAdded);
	Node *i = iList.head;
	while(i != NULL){
		free(i->data);
		i = i->next;
	}
	clearList(&iList);
	Node *f = fList.head;
	while(f != NULL){
		free(f->data);
		f = f->next;
	}
	clearList(&fList);
	return error;
}

/*
 * 0 if not in family, 1 if a parent, 2 if a child
 */ 
int inFam(const Family *fam, const Individual *person){
	if ((equalsIndiv(fam->husband, person) == 1) || (equalsIndiv(fam->wife, person) == 1)){
		return 1;
	}
	Node *famNode = fam->children.head;
	while(famNode != NULL){
		
		if (equalsIndiv(famNode->data, person) == 1){
			return 2;
		}
		famNode = famNode->next;
	}
	
	return 0;
}

void getDesN(const Individual *person, List *list, unsigned int max, int currLevel){
	if (person == NULL){
		return;
	}
	if(currLevel <= max || max == 0){
		List *toAdd = NULL;
		int i = 1;
		Node *node = list->head;
		while (node != NULL && i < currLevel){
			node = node->next;
			i++;
		}
		if(i != currLevel){
			return;
		}
		 
		if (node == NULL){
			toAdd = malloc(sizeof(List));
			*toAdd = initializeList(&printIndividual, &deleteIndividual, &compareGen);
			insertBack(list, toAdd);
		}
		else{
			toAdd = (List *)node->data;
		}
		Node *famNode = person->families.head;
		while (famNode != NULL){
			Family *fam = (Family *)famNode->data;
			if (inFam(fam, person) == 1){
			//if ((equalsIndiv(fam->husband, person) == 1) || (equalsIndiv(fam->wife, person) == 1)){
				Node *childNode = fam->children.head;
				while (childNode != NULL){
					insertSorted(toAdd, makeCopy(childNode->data));
					getDesN((Individual *)childNode->data, list, max, currLevel + 1);
					childNode = childNode->next;
				}
			}
			famNode = famNode->next;
		}
	}
}

void getAnsN(const Individual *person, List *list, unsigned int max, int currLevel){
	if (person == NULL){
		return;
	}
	if(currLevel <= max || max == 0){
		List *toAdd = NULL;
		int i = 1;
		Node *node = list->head;
		while (node != NULL && i < currLevel){
			node = node->next;
			i++;
		}
		if(i != currLevel){
			return;
		}
		if (node == NULL){
			toAdd = malloc(sizeof(List));
			*toAdd = initializeList(&printIndividual, &deleteIndividual, &compareGen);
			insertBack(list, toAdd);
		}
		else{
			toAdd = (List *)node->data;
		}
		Node *famNode = person->families.head;
		while (famNode != NULL){
			Family *fam = (Family *)famNode->data;
			if (inFam(fam, person) == 2){
				if (fam->husband != NULL){
					if (inList(fam->husband, *toAdd) == 0){
						insertSorted(toAdd, makeCopy(fam->husband));
						
					}
					getAnsN(fam->husband, list, max, currLevel + 1);
					
				}
				if (fam->wife != NULL){
					if (inList(fam->wife, *toAdd) == 0){
						insertSorted(toAdd, makeCopy(fam->wife));
						
					}
					getAnsN(fam->wife, list, max, currLevel + 1);
				}
			}
			famNode = famNode->next;
		}
	}
}

int equalsIndiv(const void *person1, const void *person2){
	if (person1 == NULL){
		return 0;
	}
	if (person2 == NULL){
		return 0;
	}
	Individual *p1 = (Individual *)person1;
	Individual *p2 = (Individual *)person2;
	/*FILE *out = fopen("OUTPUT", "a+");
	fprintf(out, "COMPARE%s\n%s\n", printIndividual(p1), printIndividual(p2));
	fclose(out);
	*/
	if (strcmp(p1->givenName, p2->givenName) != 0){
		return 0;
	}
	if (strcmp(p1->surname, p2->surname) != 0){
		return 0;
	}
	Node *c1 = p1->events.head;
	Node *c2 = p2->events.head;
	while (c1 != NULL || c2 != NULL){
		if (c1 == NULL){
			return 0;
		}
		if (c2 == NULL){
			return 0;
		}
		if (compareEvents(c1->data, c2->data) != 0){
			return 0;
		}
		Event *event1 = (Event *)c1->data;
		Event *event2 = (Event *)c2->data;
		if (strcmp(event1->date, event2->date) != 0){
			return 0;
		}
		if (strcmp(event1->place, event2->place) != 0){
			return 0;
		}
		c1 = c1->next;
		c2 = c2->next;
	}
	c1 = p1->otherFields.head;
	c2 = p2->otherFields.head;
	while (c1 != NULL || c2 != NULL){
		if (c1 == NULL){
			return 0;
		}
		if (c2 == NULL){
			return 0;
		}
		if (compareFields(c1->data, c2->data) != 0){
			return 0;
		}
		c1 = c1->next;
		c2 = c2->next;
	}
	c1 = p1->families.head;
	c2 = p2->families.head;
	while (c1 != NULL || c2 != NULL){
		if (c1 == NULL){
			return 0;
		}
		if (c2 == NULL){
			return 0;
		}
		if (compareFamilies(c1->data, c2->data) != 0){
			return 0;
		}
		c1 = c1->next;
		c2 = c2->next;
	}
	return 1;
}
void deleteList(void *toDelete){
	List *list = (List *)toDelete;
	clearList(list);
	free(list);
}
int compareGen(const void* first,const void* second){
	Individual *indiv1 = (Individual *)first;
	Individual *indiv2 = (Individual *)second;
	
	if (strcmp(indiv1->surname, "") == 0){
		return 1;
	}
	if (strcmp(indiv1->surname, indiv2->surname) == 0){
		return strcmp(indiv1->givenName, indiv2->givenName);
	}
	else{
		return strcmp(indiv1->surname, indiv2->surname);
	}
	
}
