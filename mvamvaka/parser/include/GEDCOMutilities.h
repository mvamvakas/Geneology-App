#include "GEDCOMparser.h"

typedef struct indivRef{
	char reference[15];
	Individual individual;
}indivRef;

typedef struct famRef{
	char reference[15];
	Family family;
}famRef;

/**
 * Function: checks to see if a void pointer is NULL
 * @param: check, the pointer to check
 * @return: returns 1 if null, returns 0 if not null
 */ 
int checkNULL(void *check);
char *getGEDCOMSpecs(char *fileName);
char *updateFileStuffs(char *json, char *fileName);
Individual *makeCopy(const Individual *indiv);
void deleteNULL(void *toBeDeleted);
void removeNewline(char *string);
Event *createEvent(char type[5], char *date, char *place);
Field *createField(char *tag, char *value);
int findCarriageReturn(FILE *fp);
void removeEnds(char *string);
int isEvent(char *string);
int checkContinue(FILE *fp, char **string, int lineNum);
GEDCOMerror getHead(FILE *fp, Header **head);
GEDCOMerror getLine(FILE *fp, char *string, int lineNum);
int isFamEvent(char *string);
void link(GEDCOMobject **obj);
int headCount(GEDCOMobject *obj, FILE *fp);
char *submAddress(GEDCOMobject *obj, FILE *fp, char *subREF);
void getDes(const GEDCOMobject* obj, List *list, const Individual *person);
int equalsIndiv(const void *person1, const void *person2);
GEDCOMerror writeHead(FILE *fp, const GEDCOMobject *obj);
GEDCOMerror writeSubmitter(FILE *fp, const GEDCOMobject *obj);
GEDCOMerror writeIndivs(FILE *fp, const GEDCOMobject *obj);
GEDCOMerror subCount(GEDCOMobject *obj, FILE *fp);
int inFam(const Family *fam, const Individual *person);
void getDesN(const Individual *person, List *list, unsigned int max, int currLevel);
void getAnsN(const Individual *person, List *list, unsigned int max, int currLevel);
void deleteList(void *toDelete);
int compareGen(const void* first,const void* second);
char *JSONAllIndivs(char *fileName);
char *JSONwithSex(Individual *ind);
void addIndiv(char *json, char *fileName);
char *getDesStr(char *json, char* fileName, int max);
char *getAnsStr(char *json, char* fileName, int max);
int checkValid(char *fileName);
