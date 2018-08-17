#include "GEDCOMparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "string.h"
#include "GEDCOMutilities.h"

GEDCOMerror createGEDCOM(char* fileName, GEDCOMobject** obj){
	GEDCOMerror error;
	if (fileName == NULL){
		error.type = INV_FILE;
		error.line = -1;
		return error;
	}
	if (strcmp(fileName, "") == 0){
		error.type = INV_FILE;
		error.line = -1;
		return error;
	}
	*obj = malloc(sizeof(GEDCOMobject));
	if (*obj == NULL){
		error.type = OTHER_ERROR;
		error.line = -1;
		return error;
	}
	 
	/*
	 * Creating the Gedcom object 
	 */ 
	(*obj)->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
	(*obj)->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
	(*obj)->submitter = NULL;
	(*obj)->header = NULL;
	
	int prevLine = 0;
	int i = 0;
	int lineNum = 0;
	int currLine = 0;
	char *gedPointer = NULL;
	char *fileString = NULL;
	char *token = NULL;
	const char delim[2] = " ";
	error.type = OK;
	error.line = -1;
	int fileNameLength = strlen(fileName);
	char newFileName[fileNameLength + 1];
	for (i = 0; i < strlen(fileName) - 1; i ++){
		newFileName[i] = tolower(fileName[i]);
	}
	
	if ((newFileName[fileNameLength - 4] != '.') || (newFileName[fileNameLength - 3] != 'g') || (newFileName[fileNameLength - 2] != 'e') || (fileName[fileNameLength - 1] != 'd')){
		deleteGEDCOM(*obj);
		*obj = NULL;
		error.type = INV_FILE;
		error.line = -1;
		return error;
	}
	  
	
	/*
	 * Opening the file and checking if it's NULL or 
	 * if there is problems with it
	 */ 
	FILE *fp = fopen(fileName, "r'");
	if (fp == NULL){
		//printf("NO FILE");
		deleteGEDCOM(*obj);
		*obj = NULL;
		error.type = INV_FILE;
		error.line = -1;
		
		return error;
	}
	fileString = malloc(sizeof(char) * 256);
	error = getLine(fp, fileString, 0);
	if (strcmp(fileString,"0 HEAD") != 0){
		char *token = strtok(fileString, " ");
		token = strtok(NULL, " ");
		if (strcmp(token, "HEAD") == 0){
			error.type = INV_HEADER;
			
		}
		else{
			error.type = INV_GEDCOM;
			//printf("Hey");
		}
		error.line = -1;
		free(fileString);
		deleteGEDCOM(*obj);
		fclose(fp);
		*obj = NULL;
		return error;
	}
	free(fileString);
	fseek(fp, 0, SEEK_SET);
	error = subCount(*obj, fp);
	if (error.type != OK){
		deleteGEDCOM(*obj);
		*obj = NULL;
		fclose(fp);
		return error;
	}
	/*
	 * FIND 0 TRLR
	 */
	lineNum = 0; 
	fileString = malloc(sizeof(char) * 256);
	error = getLine(fp, fileString, lineNum);
	lineNum++;
	while (strcmp(fileString, "0 TRLR") != 0){
		int prev = fileString[0] - '0';
		//printf("%s\n", fileString);
		if (error.type != OK){
			fclose(fp);
			free(fileString);
			deleteGEDCOM(*obj);
			*obj = NULL;
			//printf("oh NO\b");
			return error;
		}
		//printf("|%s|\n", fileString);
		char c = fgetc(fp);
		while(c == '\n' || c == '\r'){
			c = fgetc(fp);
		}
		if (fgetc(fp) == EOF){
			error.type = INV_GEDCOM;
			error.line = -1;
			deleteGEDCOM(*obj);
			free(fileString);
			fclose(fp);
			*obj = NULL;
			return error;
		}
		fseek(fp, -2, SEEK_CUR);
		lineNum++;
		free(fileString);
		fileString = malloc(sizeof(char) * 256);
		error = getLine(fp, fileString, lineNum);
		if (((fileString[0] - '0') - prev) > 1){
			error.type = INV_RECORD;
			error.line = lineNum;
			deleteGEDCOM(*obj);
			free(fileString);
			fclose(fp);
			*obj = NULL;
			return error;
		}
	}

	free(fileString);
	fseek(fp, 0, SEEK_SET);
	const int carriage = findCarriageReturn(fp);
	/*
	 * Getting the head, if the head is invalid
	 * free the memory and return
	 */ 
	 //printf("HEY");
	 	
	error = getHead(fp, &(*obj)->header);
	if (error.type != OK){
		deleteGEDCOM(*obj);
		fclose(fp);
		*obj = NULL;
		return error;
	}
	char *address = NULL;
	int subFind = 0;
	//malloc(sizeof(char) * 255);
	/*
	 * This will get the submitter address and then malloc
	 * the struct for the space for it
	 */ 
	Node *headOther = (*obj)->header->otherFields.head;
	while(headOther != NULL){
		Field *field = (Field *)headOther->data;
		if (strcmp(field->tag, "SUBM") == 0){
			address = malloc(sizeof(char) * (strlen(field->value) + 1));
			if (address == NULL){
				deleteGEDCOM(*obj);
				fclose(fp);
				error.type = OTHER_ERROR;
				error.line = -1;
				*obj = NULL;
				return error;
			}
			strcpy(address, field->value);
			subFind = 1;
			break;
		}
		headOther = headOther->next;
	}
	if (subFind == 0){
		deleteGEDCOM(*obj);
		fclose(fp);
		error.type = INV_HEADER;
		error.line = -1;
		*obj = NULL;
		return error;
	}
	char *subA = submAddress(*obj, fp, address);
	if (subA == NULL){
		(*obj)->submitter = malloc(sizeof(Submitter) + 1);
		if ((*obj)->submitter == NULL){
			deleteGEDCOM(*obj);
			fclose(fp);
			error.type = OTHER_ERROR;
			error.line = -1;
			*obj = NULL;
			return error;
		}
		(*obj)->submitter->otherFields = initializeList(&printField, &deleteField, &compareFields);
		strncpy((*obj)->submitter->address, "", 1);
	}
	else{
		(*obj)->submitter = malloc(sizeof(Submitter) + (sizeof(char) * strlen(subA) + 1));
		if ((*obj)->submitter == NULL){
			deleteGEDCOM(*obj);
			fclose(fp);
			error.type = OTHER_ERROR;
			error.line = -1;
			*obj = NULL;
			return error;
		}
		(*obj)->submitter->otherFields = initializeList(&printField, &deleteField, &compareFields);
		strncpy((*obj)->submitter->address, subA, strlen(subA) + 1);
	}
	free(address);
	free(subA);
	(*obj)->header->submitter = (*obj)->submitter;
	//return error;
	if (error.type != OK){
		deleteGEDCOM(*obj);
		fclose(fp);
		*obj = NULL;
		return error;
	}
	fseek(fp, 0, SEEK_END);
	int fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    //return error;
	if (fileSize == 0 || fileSize == 1){
		deleteGEDCOM(*obj);
		fclose(fp);
		error.type = INV_FILE;
		error.line = -1;
		*obj = NULL;
		return error;
	}
	fileString = malloc(sizeof(char) * 256);
	if (fileString == NULL){
		deleteGEDCOM(*obj);
		fclose(fp);
		error.type = OTHER_ERROR;
		error.line = -1;
		*obj = NULL;
		return error;
	}
	error = getLine(fp, fileString, lineNum);
	if (error.type != OK){
		deleteGEDCOM(*obj);
		free(fileString);
		*obj = NULL;
		return error;
	}
	while(checkContinue(fp, &fileString, lineNum) == 1){
		lineNum++;
	}
	if (error.type != OK){
		free(fileString);
		deleteGEDCOM(*obj);
		*obj = NULL;
		fclose(fp);
		return error;
	}
	//fseek(fp, carriage - 1, SEEK_CUR);

	/*
	 * Getting the number of 0 HEAD's found in the file
	 * if the number is not one then an error is returned
	 */ 
	int hCount = headCount(*obj, fp);
	if (strcmp(fileString,"0 HEAD") != 0){
		if (hCount == 1){
			error.type = INV_GEDCOM;
			error.line = -1;
		}
		else if (hCount == 0 || hCount > 1){
			error.type = INV_GEDCOM;
			error.line = -1;
		}
		free(fileString);
		deleteGEDCOM(*obj);
		fclose(fp);
		*obj = NULL;
		return error;
	}
	if (hCount > 1){
		error.type = INV_GEDCOM;
		error.line = -1;
		free(fileString);
		deleteGEDCOM(*obj);
		fclose(fp);
		*obj = NULL;
		return error;
	}
	free(fileString);
	fseek(fp, 0, SEEK_SET);
	lineNum = 0;
	/*
	 * Parsing the Individuals
	 */ 
	lineNum = 1;
	while (!feof(fp)){
		lineNum++;
		fileString = malloc(sizeof(char) * 256);
		if (fileString == NULL){
			deleteGEDCOM(*obj);
			fclose(fp);
			error.type = OTHER_ERROR;
			error.line = -1;
			*obj = NULL;
			return error;
		}
        error = getLine(fp, fileString, lineNum);
        while(checkContinue(fp, &fileString, lineNum) == 1){
			lineNum++;
		}
	    if (error.type != OK){
			free(fileString);
			deleteGEDCOM(*obj);
			fclose(fp);
			*obj = NULL;
		    return error;
	    }
	    /*
	     * Checking to see if its at the end of the file
	     */ 
	    if ((strcmp(fileString, "0 TRLR") == 0) || (strcmp(fileString, "") == 0)){
			/*if (fgetc(fp) != EOF){
				printf("HEY\n");
				free(fileString);
				deleteGEDCOM(*obj);
				fclose(fp);
				error.type = INV_GEDCOM;
				error.type = -1;
				return error;
			}
			*/ 
			lineNum++;
			free(fileString);
			break;
		}
        /*
         * Checking to see if the first character in the line
         * is a digit, then verifying that it is not more than
         * 1 higher than the previous line
         */ 
        token = strtok(fileString, delim); 
        if (isdigit(token[0]) == 0){
			free(fileString);
			deleteGEDCOM(*obj);
			fclose(fp);
			error.type = INV_RECORD;
			error.line = lineNum;
			*obj = NULL;
			return error;
		}
		currLine = token[0] - '0';
		if (currLine > prevLine + 2){
			free(fileString);
			deleteGEDCOM(*obj);
			fclose(fp);
			error.type = INV_RECORD;
			error.line = lineNum;
			*obj = NULL;
			return error;
		}
		
		/*
		 * Checking if line value is 0, if it is then its a new
		 * record type
		 */ 
		token = strtok(NULL, delim);
		if (checkNULL(token) == 1){
			free(fileString);
			deleteGEDCOM(*obj);
			fclose(fp);
			error.type = INV_RECORD;
			error.line = lineNum;
			*obj = NULL;
			//printf("HI");
			return error;
		}
        if (currLine == 0){
			if (token[0] == '@' && token[strlen(token) - 1] == '@'){
				Field *refField = NULL;
				gedPointer = malloc(sizeof(char) * strlen(token) + 1);
				if (gedPointer == NULL){
					deleteGEDCOM(*obj);
					fclose(fp);
					free(fileString);
					error.type = OTHER_ERROR;
					error.line = -1;
					*obj = NULL;
					return error;
				}
				strncpy(gedPointer, token, strlen(token) + 1);
				// Getting the next token and checking if NULL
				token = strtok(NULL, delim);
				if (checkNULL(token) == 1){
					free(gedPointer);
					free(fileString);
					deleteGEDCOM(*obj);
					fclose(fp);
			        error.type = INV_RECORD;
			        error.line = lineNum;
			        *obj = NULL;
			        return error;
	    	    }
	    	    // Checking if individual
	    	    if (strcmp(token, "INDI") == 0){
				    //Creating the individual
				    refField = createField("XREF", gedPointer);
				    free(gedPointer);
				    strcpy(fileString, "");
				    Individual *indiv = malloc(sizeof(Individual));
				    if (indiv == NULL){
						deleteGEDCOM(*obj);
						fclose(fp);
						free(fileString);
						error.type = OTHER_ERROR;
						error.line = -1;
						*obj = NULL;
						return error;
					}
				    indiv->events = initializeList(&printEvent, &deleteEvent, &compareEvents); 
				    indiv->families = initializeList(&printFamily, &deleteFamily, &compareFamilies); 
				    indiv->otherFields = initializeList(&printField, &deleteField, &compareFields); 
			    	insertFront(&indiv->otherFields, refField);
			    	insertBack(&(*obj)->individuals, indiv);
			    	while ((fileString[0] != '0') && (!feof(fp))){
		    			/*
		    			 * Gets a line from the file and checks validity
		    			 */ 
		    			free(fileString);
	    			    fileString = malloc(sizeof(char) * 256);
    	                if (fileString == NULL){
							deleteGEDCOM(*obj);
							fclose(fp);
							error.type = OTHER_ERROR;
							error.line = -1;
							*obj = NULL;
							return error;
						}
    	                error = getLine(fp, fileString, lineNum);
						while(checkContinue(fp, &fileString, lineNum) == 1){
							lineNum++;
						}
						if (error.type != OK){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
							*obj = NULL;
							return error;
						}
	                    if (fileString[0] == '0'){
							fseek(fp, -(strlen(fileString) + carriage), SEEK_CUR);
						    break;
					    }
					    lineNum++;
	                    token = strtok(fileString, delim);
		                if (isdigit(token[0]) == 0){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
		                 	error.type = INV_RECORD;
	    	        	    error.line = lineNum;
	    	        	    *obj = NULL;
    		        	    return error;
		                }
	            	    currLine = token[0] - '0';
	            	    if (currLine > prevLine + 2){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
	            		    error.type = INV_RECORD;
    			            error.line = lineNum;
    			            *obj = NULL;
			                return error;
		                }
		                token = strtok(NULL, delim);
		                if (checkNULL(token) == 1){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
			                error.type = INV_RECORD;
			                error.line = lineNum;
			                *obj = NULL;
			                return error;
	    	            }
	    	            /*
	    	             * Gets the name of the individual
	    	             */
	    	            int flag = 0;
		                if (strcmp(token,"NAME") == 0){
		    				token = strtok(NULL, delim);
		    				if (strcmp(token, "Not") == 0){
								token = strtok(NULL, delim);
								token = strtok(NULL, delim);
								flag = 1;
							}
		    				if (token[0] == '/' && token[strlen(token) - 1] == '/'){
		    					indiv->givenName = malloc(sizeof(char) * 10);
		    					if (indiv->givenName == NULL){
									deleteGEDCOM(*obj);
									fclose(fp);
									free(fileString);
									error.type = OTHER_ERROR;
									error.line = -1;
									*obj = NULL;
									return error;
								}
								if (flag == 1){
									strncpy(indiv->givenName,"Not named", 10);
									flag = 0;
								}
								else{
									strncpy(indiv->givenName,"", 1);
								}
		    					
		    					indiv->surname = malloc(sizeof(char) * strlen(token) + 1);
		    					if (indiv->surname == NULL){
									deleteGEDCOM(*obj);
									fclose(fp);
									free(fileString);
									error.type = OTHER_ERROR;
									error.line = -1;
									*obj = NULL;
									return error;
								}
								
		    					strncpy(indiv->surname, token, strlen(token) + 1);
		    					removeEnds(indiv->surname);
		    					
		    				}
		    				else {
								//printf("%s\n", token);
		    					indiv->givenName = malloc(sizeof(char) * strlen(token) + 1);
		    					if (indiv->givenName == NULL){
									deleteGEDCOM(*obj);
									fclose(fp);
									free(fileString);
									error.type = OTHER_ERROR;
									error.line = -1;
									*obj = NULL;
									return error;
								}
		    					strncpy(indiv->givenName, token, strlen(token) + 1);
		    					token = strtok(NULL, delim);
		    					//printf("%s\n", token);
		    					if (strlen(token) != 2){
									while(token != NULL && token[0] != '/'){
										token = strtok(NULL, " ");
										indiv->givenName = realloc(indiv->givenName, sizeof(char) * (strlen(indiv->givenName) + strlen(token) + 2));
										strcat(indiv->givenName, token);
										strcat(indiv->givenName," ");
									}
									if ((token[0] == '/' && token[strlen(token) - 1] == '/') && token != NULL){
										indiv->surname = malloc(sizeof(char) * strlen(token) + 1);
									    if (indiv->surname == NULL){
											deleteGEDCOM(*obj);
											fclose(fp);
											free(fileString);
											error.type = OTHER_ERROR;
											error.line = -1;
											*obj = NULL;
											return error;
										}
									    //printf("5%s5", token);
		    					        strncpy(indiv->surname, token, strlen(token) + 1);
										removeEnds(indiv->surname);
										//printf("%s==\n", indiv->surname);
								    }
								}
								else {
									indiv->surname = malloc(sizeof(char) * 1);
									if (indiv->surname == NULL){
										deleteGEDCOM(*obj);
										fclose(fp);
										free(fileString);
										error.type = OTHER_ERROR;
										error.line = -1;
										*obj = NULL;
										return error;
									}
									indiv->surname[0] = '\0';
								}
		    					
		    				}
		    			}
		    			/*
		    			 * Gets Sex of the individual
		    			 */ 
		    			//printf("%s\n", token);
    		    	    else if (strcmp(token, "SEX") == 0){
						//else if (isEvent(token) == 0){	
							//printf("%s: Not an Event\n", token);
							token = strtok(NULL, delim);
							if (checkNULL(token) == 1){
								//clearList(&(*obj)->individuals);
								free(fileString);
								fclose(fp);
								deleteGEDCOM(*obj);
			                    error.type = INV_RECORD;
			                    error.line = lineNum;
			                    *obj = NULL;
			                    return error;
	    	                }
						    if (strcmp(token, "M") == 0 || strcmp(token, "F") == 0 || strcmp(token, "U") == 0){
								Field *field = createField("SEX", token);
								insertBack(&indiv->otherFields, field);
							}
							else {
			                    free(fileString);
								deleteGEDCOM(*obj);
								fclose(fp);
								error.type = INV_RECORD;
			                    error.line = lineNum;
			                    *obj = NULL;
			                    return error;
							}
						}
						/*
						 * Date of Birth or any other event
						 */ 
						else if (isEvent(token) == 1){
							strcpy(fileString, "");
						    Event *birthEvent = malloc(sizeof(Event));
						    if (birthEvent == NULL){
								deleteGEDCOM(*obj);
								fclose(fp);
								free(fileString);
								error.type = OTHER_ERROR;
								error.line = -1;
								*obj = NULL;
								return error;
							}
						    strncpy(birthEvent->type, token, 5);
						    birthEvent->date = NULL;
						    birthEvent->place = NULL;
						    birthEvent->otherFields = initializeList(&printField, &deleteField, &compareFields); 
						    insertFront(&indiv->events, birthEvent);
						    //printf("|%s|\n", token);
							while ((fileString[0] != '1') && (!feof(fp))){
						    	free(fileString);
	    			            fileString = malloc(sizeof(char) * 256);
	    			            if (fileString == NULL){
									deleteGEDCOM(*obj);
									fclose(fp);
									error.type = OTHER_ERROR;
									error.line = -1;
									*obj = NULL;
									return error;
								}
    	                        error = getLine(fp, fileString, lineNum);
						
								//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
								while(checkContinue(fp, &fileString, lineNum) == 1){
									lineNum++;
								}
    	                        //printf("++%s++\n", fileString);
	                            if (fileString[0] == '1' || fileString[0] == '0'){
									//printf("HI\n");
									fseek(fp, -(strlen(fileString) + carriage), SEEK_CUR);
	                                //printf("%s\n", fileString);
									break;
								}
	                            lineNum++;
	                            //lineNum = lineNum + checkContinue(fp, fileString);
		                        token = strtok(fileString, delim);
		                        if (checkNULL(token) == 1){
									//clearList(&(*obj)->individuals);
									free(fileString);
									fclose(fp);
									deleteGEDCOM(*obj);
									error.type = INV_RECORD;
									error.line = lineNum;
									*obj = NULL;
									return error;
								}
		                        if (isdigit(token[0]) == 0){
									free(fileString);
									deleteGEDCOM(*obj);
									fclose(fp);
		                         	error.type = INV_RECORD;
	    	            	        error.line = lineNum;
	    	            	        *obj = NULL;
    		        	            return error;
		                        }
	                	        currLine = token[0] - '0';
	                	        if (currLine > prevLine + 2){
									free(fileString);
									deleteGEDCOM(*obj);
									fclose(fp);
    	                		    error.type = INV_RECORD;
            			            error.line = lineNum;
            			            *obj = NULL;
		        	                return error;
		                        }
		                        if (currLine == 1){
									break;
								}
		                        token = strtok(NULL, delim);
		                        if (checkNULL(token) == 1){
									free(fileString);
									deleteGEDCOM(*obj);
									fclose(fp);
			                        error.type = INV_RECORD;
			                        error.line = lineNum;
			                        *obj = NULL;
	    	    	                return error;
	        	                }
	        	                /*
	        	                 * Collecting place of the event
	        	                 */ 
	        	                if (strcmp(token, "PLAC") == 0){
									token = strtok(NULL, delim);
									if (checkNULL(token) == 1){
										free(fileString);
										deleteGEDCOM(*obj);
										fclose(fp);
			                            error.type = INV_RECORD;
			                            error.line = lineNum;
			                            *obj = NULL;
	    	    	                    return error;
	        	                    }
	        	                    birthEvent->place = malloc(sizeof(char) * 256);
	        	                    if (birthEvent->place == NULL){
										deleteGEDCOM(*obj);
										fclose(fp);
										free(fileString);
										error.type = OTHER_ERROR;
										error.line = -1;
										*obj = NULL;
										return error;
									}
	        	                    strncpy(birthEvent->place, token, strlen(token) + 1);
	        	                    strcat(birthEvent->place, " ");
	        	                    token = strtok(NULL, delim);
									while (token != NULL){
										strcat(birthEvent->place, token);
										strcat(birthEvent->place, " ");
										token = strtok(NULL, delim);
									}
									//clebirthEvent->place[strlen(birthEvent->place) - 1] 
									birthEvent->place[strlen(birthEvent->place) - 1] = '\0';
								}
								/*
								 * Getting the date of an event, saving an 
								 * empty string if nothing
								 */ 
								else if (strcmp(token, "DATE") == 0){
									char *birth = malloc(sizeof(char) * 256);
									if (birth == NULL){
										deleteGEDCOM(*obj);
										fclose(fp);
										free(fileString);
										error.type = OTHER_ERROR;
										error.line = -1;
										*obj = NULL;
										return error;
									}
									strncpy(birth, "", 1);
									token = strtok(NULL, delim);
									while (token != NULL){
										strcat(birth, token);
										strcat(birth, " ");
										token = strtok(NULL, delim);
									}
									birth[strlen(birth) - 1] = '\0';
									//removeNewline(birth);
									birthEvent->date = malloc(sizeof(char) * strlen(birth) + 1);
									if (birthEvent->date == NULL){
										deleteGEDCOM(*obj);
										fclose(fp);
										free(fileString);
										error.type = OTHER_ERROR;
										error.line = -1;
										*obj = NULL;
										return error;
									}
									strncpy(birthEvent->date, birth, strlen(birth) + 1);
									free(birth);
								}
							}
							if (birthEvent->place == NULL){
								birthEvent->place = malloc(sizeof(char) * 1);
								if (birthEvent->place == NULL){
									deleteGEDCOM(*obj);
									fclose(fp);
									free(fileString);
									error.type = OTHER_ERROR;
									error.line = -1;
									*obj = NULL;
									return error;
								}
								strncpy(birthEvent->place, "", 1);
							}
							if (birthEvent->date == NULL){
								birthEvent->date = malloc(sizeof(char) * 1);
								
								if (birthEvent->date == NULL){
									deleteGEDCOM(*obj);
									fclose(fp);
									free(fileString);
									error.type = OTHER_ERROR;
									error.line = -1;
									*obj = NULL;
									return error;
								}
								strncpy(birthEvent->date, "", 1);
							}
						}
						/*
						 * If not in the list of events then it is
						 * thrown into otherfields
						 */ 
						else {
							Field *field = malloc(sizeof(Field));
							if (field == NULL){
								deleteGEDCOM(*obj);
								fclose(fp);
								free(fileString);
								error.type = OTHER_ERROR;
								error.line = -1;
								*obj = NULL;
								return error;
							}
							field->tag = malloc(sizeof(char) * 5);
							if (field->tag == NULL){
								deleteGEDCOM(*obj);
								fclose(fp);
								free(fileString);
								error.type = OTHER_ERROR;
								error.line = -1;
								*obj = NULL;
								return error;
							}
							field->value = NULL;
							strncpy(field->tag, token, 5);
							token = strtok(NULL, delim);
							if (checkNULL(token) == 1){
								deleteField(field);
								free(fileString);
								fclose(fp);
								deleteGEDCOM(*obj);
			                    error.type = INV_RECORD;
			                    error.line = lineNum;
			                    *obj = NULL;
			                    return error;
	    	                }
	    	                char *str = malloc(sizeof(char) * 256);
							if (str == NULL){
								deleteGEDCOM(*obj);
								fclose(fp);
								free(fileString);
								error.type = OTHER_ERROR;
								error.line = -1;
								*obj = NULL;
								return error;
							}
							strncpy(str, "", 1);
							strcat(str, token);
							strcat(str, " ");
							token = strtok(NULL, delim);
							while (token != NULL){
								strcat(str, token);
								strcat(str, " ");
								token = strtok(NULL, delim);
							}
							//removeNewline(str);
							str[strlen(str) - 1] = '\0';
							field->value = malloc(sizeof(char) * strlen(str) + 1);
							if (field->value == NULL){
								deleteGEDCOM(*obj);
								free(str);
								fclose(fp);
								free(fileString);
								error.type = OTHER_ERROR;
								error.line = -1;
								*obj = NULL;
								return error;
							}
							strncpy(field->value, str, strlen(str) + 1);
							free(str);
	    	                insertFront(&indiv->otherFields, field);
						} 
	                }
	                /*if (strcmp(fileString,"0 TRLR") == 0){
						break;
					}
					*/ 
			    }
			    else{
					free(gedPointer);
				}
			}
		} 
		if (strcmp(fileString,"0 TRLR") == 0){
			free(fileString);
			fileString = NULL;
			break;
		}
	    free(fileString);
	    fileString = NULL;
	}
	fseek(fp, 0, SEEK_SET);
	lineNum = 1;
	subFind = 0;
	
	/*
	 * Families
	 */ 
	while (!feof(fp)){
		
		fileString = malloc(sizeof(char) * 256);
		if (fileString == NULL){
			deleteGEDCOM(*obj);
			fclose(fp);
			error.type = OTHER_ERROR;
			error.line = -1;
			*obj = NULL;
			return error;
		}
        error = getLine(fp, fileString, lineNum);
        while(checkContinue(fp, &fileString, lineNum) == 1){
			lineNum++;
		}
        if (error.type != OK){
			free(fileString);
			deleteGEDCOM(*obj);
			fclose(fp);
			*obj = NULL;
		    return error;
	    }
	    if ((strcmp(fileString, "0 TRLR") == 0) || (strcmp(fileString, "") == 0)){
			free(fileString);
			break;
			/*fclose(fp);
			deleteGEDCOM(*obj);
			*obj = NULL;
			return error;
			*/ 
		}
		//printf("%s\n",fileString);
		lineNum++;
        //lineNum = lineNum + checkContinue(fp, fileString);
        /*
         * Checking to see if the first character in the line
         * is a digit, then verifying that it is not more than
         * 1 higher than the previous line
         */ 
        token = strtok(fileString, delim); 
        if (isdigit(token[0]) == 0){
			free(fileString);
			deleteGEDCOM(*obj);
			fclose(fp);
			error.type = INV_RECORD;
			error.line = lineNum;
			*obj = NULL;
			return error;
		}
		currLine = token[0] - '0';
		if (currLine > prevLine + 2){
			free(fileString);
			deleteGEDCOM(*obj);
			fclose(fp);
			error.type = INV_RECORD;
			error.line = lineNum;
			*obj = NULL;
			return error;
		}
		
		/*
		 * Checking if line value is 0, if it is then its a new
		 * record type
		 */ 
		token = strtok(NULL, delim);
		if (checkNULL(token) == 1){
			free(fileString);
			deleteGEDCOM(*obj);
			fclose(fp);
			error.type = INV_RECORD;
			error.line = lineNum;
			*obj = NULL;
			return error;
		}
        if (currLine == 0){
			/*
			 * Collecting the X Reference of the individual
			 */ 
			if (token[0] == '@' && token[strlen(token) - 1] == '@'){
				Field *refField = NULL;
				gedPointer = malloc(sizeof(char) * strlen(token) + 1);
				if (gedPointer == NULL){
					deleteGEDCOM(*obj);
					fclose(fp);
					free(fileString);
					error.type = OTHER_ERROR;
					error.line = -1;
					*obj = NULL;
					return error;
				}
				strncpy(gedPointer, token, strlen(token) + 1);
				// Getting the next token and checking if NULL
				token = strtok(NULL, delim);
				if (checkNULL(token) == 1){
					free(fileString);
					free(gedPointer);
					deleteGEDCOM(*obj);
					fclose(fp);
			        error.type = INV_RECORD;
			        error.line = lineNum;
			        *obj = NULL;
			        return error;
	    	    }
	    	    // Checking if fam
	    	    if (strcmp(token, "FAM") == 0){
					
					//Creating the fam
				    refField = createField("XREF", gedPointer);
				    free(gedPointer);
				    strcpy(fileString, "");
				    Family *fam = malloc(sizeof(Family));
				    if (fam == NULL){
						deleteGEDCOM(*obj);
						fclose(fp);
						free(fileString);
						error.type = OTHER_ERROR;
						error.line = -1;
						*obj = NULL;
						return error;
					}
					fam->husband = NULL;
					fam->wife = NULL;
				    fam->children = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals); 
				    fam->events = initializeList(&printEvent, &deleteEvent, &compareEvents); 
				    fam->otherFields = initializeList(&printField, &deleteField, &compareFields); 
			    	insertFront(&fam->otherFields, refField);
			    	insertFront(&(*obj)->families, fam);
			    	while ((fileString[0] != '0') && (!feof(fp))){
		    			/*
		    			 * Gets a line from the file and checks validity
		    			 */
		    			free(fileString);
	    			    fileString = malloc(sizeof(char) * 256);
	    			    
    	                error = getLine(fp, fileString, lineNum);
    	                
						while(checkContinue(fp, &fileString, lineNum) == 1){
							lineNum++;
						}
						if (error.type != OK){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
							*obj = NULL;
							return error;
						}
	                    if (fileString[0] == '0'){
							fseek(fp, -(strlen(fileString) + carriage), SEEK_CUR);
	                        //printf("%s\n", fileString);
						    break;
					    }
	                    //printf("%s\n", fileString);
	                    lineNum++;
	                    //lineNum = lineNum + checkContinue(fp, fileString);
		                token = strtok(fileString, delim);
		                if (isdigit(token[0]) == 0){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
		                 	error.type = INV_RECORD;
	    	        	    error.line = lineNum;
	    	        	    *obj = NULL;
    		        	    return error;
		                }
	            	    currLine = token[0] - '0';
	            	    if (currLine > prevLine + 2){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
	            		    error.type = INV_RECORD;
    			            error.line = lineNum;
    			            *obj = NULL;
			                return error;
		                }
		                token = strtok(NULL, delim);
		                if (checkNULL(token) == 1){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
			                error.type = INV_RECORD;
			                error.line = lineNum;
			                *obj = NULL;
			                return error;
	    	            }
	    	            if (isFamEvent(token) == 1){
						    strcpy(fileString, "");
						    Event *event = malloc(sizeof(Event));
						    strncpy(event->type, token, 5);
						    event->date = NULL;
						    event->place = NULL;
						    event->otherFields = initializeList(&printField, &deleteField, &compareFields); 
						    insertFront(&fam->events, event);
							while ((fileString[0] != '1') && (!feof(fp))){
						    	free(fileString);
	    			            fileString = malloc(sizeof(char) * 256);
	    			            
    	                        error = getLine(fp, fileString, lineNum);
								//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
								while(checkContinue(fp, &fileString, lineNum) == 1){
									lineNum++;
								}
    	                        //printf("++%s++\n", fileString);
	                            if (fileString[0] == '1' || fileString[0] == '0'){
									//printf("HI\n");
									fseek(fp, -(strlen(fileString) + carriage), SEEK_CUR);
	                                //printf("%s\n", fileString);
									break;
								}
	                            
	                            //lineNum = lineNum + checkContinue(fp, fileString);
		                        token = strtok(fileString, delim);
		                        if (isdigit(token[0]) == 0){
									free(fileString);
									deleteGEDCOM(*obj);
									fclose(fp);
		                         	error.type = INV_RECORD;
	    	            	        error.line = lineNum;
	    	            	        *obj = NULL;
    		        	            return error;
		                        }
	                	        currLine = token[0] - '0';
	                	        if (currLine > prevLine + 2){
									free(fileString);
									deleteGEDCOM(*obj);
									fclose(fp);
    	                		    error.type = INV_RECORD;
            			            error.line = lineNum;
            			            *obj = NULL;
		        	                return error;
		                        }
		                        if (currLine == 1){
									break;
								}
		                        lineNum++;
		                        token = strtok(NULL, delim);
		                        if (checkNULL(token) == 1){
									free(fileString);
									deleteGEDCOM(*obj);
									fclose(fp);
			                        error.type = INV_RECORD;
			                        error.line = lineNum;
			                        *obj = NULL;
	    	    	                return error;
	        	                }
	        	                if (strcmp(token, "PLAC") == 0){
									token = strtok(NULL, delim);
									if (checkNULL(token) == 1){
										free(fileString);
										deleteGEDCOM(*obj);
										fclose(fp);
			                            error.type = INV_RECORD;
			                            error.line = lineNum;
			                            *obj = NULL;
	    	    	                    return error;
	        	                    }
	        	                    char *str = malloc(sizeof(char) * 256);
									strncpy(str, "", 1);
									strcat(str, token);
									strcat(str, " ");
									token = strtok(NULL, delim);
									while (token != NULL){
										strcat(str, token);
										strcat(str, " ");
										token = strtok(NULL, delim);
									}
									str[strlen(str) - 1] = '\0';
	        	                    event->place = malloc(sizeof(char) * strlen(str) + 1);
	        	                    strncpy(event->place, str, strlen(str) + 1);
									free(str);
								}
								else if (strcmp(token, "DATE") == 0){
									char *birth = malloc(sizeof(char) * 256);
									strncpy(birth, "", 1);
									token = strtok(NULL, delim);
									while (token != NULL){
										strcat(birth, token);
										strcat(birth, " ");
										token = strtok(NULL, delim);
									}
									birth[strlen(birth) - 1] = '\0';
									//removeNewline(birth);
									event->date = malloc(sizeof(char) * strlen(birth) + 1);
									strncpy(event->date, birth, strlen(birth) + 1);
									free(birth);
									/*free(fileString);
									fclose(fp);
									return error;*/
								}
							}
							if (event->place == NULL){
								event->place = malloc(sizeof(char) * 1);
								strncpy(event->place, "", 1);
							}
							if (event->date == NULL){
								event->date = malloc(sizeof(char) * 1);
								strncpy(event->date, "", 1);
							}
						}
						/*
						 * otherFields
						 */ 
						else {
							Field *field = malloc(sizeof(Field));
							field->tag = malloc(sizeof(char) * 5);
							strncpy(field->tag, token, 5);
							token = strtok(NULL, delim);
							if (checkNULL(token) == 1){
								free(fileString);
								fclose(fp);
								deleteGEDCOM(*obj);
			                    error.type = INV_RECORD;
			                    error.line = lineNum;
			                    *obj = NULL;
			                    return error;
	    	                }
	    	                
	    	                char *str = malloc(sizeof(char) * 256);
							strncpy(str, "", 1);
							strcat(str, token);
							strcat(str, " ");
							token = strtok(NULL, delim);
							while (token != NULL){
								strcat(str, token);
								strcat(str, " ");
								token = strtok(NULL, delim);
							}
							str[strlen(str) - 1] = '\0';
							field->value = malloc(sizeof(char) * strlen(str) + 1);
							strncpy(field->value, str, strlen(str) + 1);
							free(str);
	    	                insertFront(&fam->otherFields, field);
						}
					}
				}
				/*
				 * Submitter
				 */ 
				else if (strcmp(token, "SUBM") == 0){
					
					subFind = 1;
					refField = createField("XREF", gedPointer);
				    free(gedPointer);
				    strcpy(fileString, "");
				    //(*obj)->submitter = malloc(sizeof(Submitter));
				    (*obj)->submitter->otherFields = initializeList(&printField, &deleteField, &compareFields); 
					(*obj)->header->submitter = (*obj)->submitter;
					insertFront(&(*obj)->submitter->otherFields, refField);
					while ((fileString[0] != '0') && (!feof(fp))){
		    			/*
		    			 * Gets a line from the file and checks validity
		    			 */
		    			free(fileString);
	    			    fileString = malloc(sizeof(char) * 256);
    	                error = getLine(fp, fileString, lineNum);
						while(checkContinue(fp, &fileString, lineNum) == 1){
							lineNum++;
						}
						//lineNum = lineNum + checkContinue(fp, fileString, lineNum);
						//fseek(fp, carriage - 1, SEEK_CUR);
						if (error.type != OK){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
							*obj = NULL;
							return error;
						}
	                    if (fileString[0] == '0'){
							fseek(fp, -(strlen(fileString) + carriage), SEEK_CUR);
	                        //printf("%s\n", fileString);
						    break;
					    }
	                    //printf("%s\n", fileString);
	                    
	                    //lineNum = lineNum + checkContinue(fp, fileString);
		                token = strtok(fileString, delim);
		                if (isdigit(token[0]) == 0){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
		                 	error.type = INV_RECORD;
	    	        	    error.line = lineNum;
	    	        	    *obj = NULL;
    		        	    return error;
		                }
	            	    currLine = token[0] - '0';
	            	    if (currLine > prevLine + 2){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
	            		    error.type = INV_RECORD;
    			            error.line = lineNum;
    			            *obj = NULL;
			                return error;
		                }
		                token = strtok(NULL, delim);
		                if (checkNULL(token) == 1){
							free(fileString);
							deleteGEDCOM(*obj);
							fclose(fp);
			                error.type = INV_RECORD;
			                error.line = lineNum;
			                *obj = NULL;
			                return error;
	    	            }
	    	            lineNum++;
	    	            if (strcmp(token, "NAME") == 0){
							token = strtok(NULL, delim);
							if (checkNULL(token) == 1){
								free(fileString);
								deleteGEDCOM(*obj);
								fclose(fp);
								error.type = INV_RECORD;
								error.line = lineNum;
								*obj = NULL;
								return error;
							}
							strncpy((*obj)->submitter->submitterName, token, strlen(token) + 1);
						}
					}
				}
				else {
					free(gedPointer);
				}
			}
		}
		if (strcmp(fileString, "0 TRLR") == 0){
			lineNum++;
			free(fileString);
			break;
		}
		free(fileString);
	    fileString = NULL;
	}
	if (subFind == 0){
		deleteGEDCOM(*obj);
		fclose(fp);
		error.type = INV_GEDCOM;
		error.line = -1;
		*obj = NULL;
		return error;
	}
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
				//printf("hey");
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
	Node *curr = (*obj)->submitter->otherFields.head;
	while (curr != NULL){
		Field *field = (Field *)curr->data;
		if (strcmp(field->tag,"SUBM") == 0){
			deleteDataFromList(&(*obj)->submitter->otherFields, field);
			deleteField(field);
			break;
		}
		curr = curr->next;
	}
	fclose(fp);
	return error;
}

Individual* findPerson(const GEDCOMobject* familyRecord, bool (*compare)(const void* first, const void* second), const void* person){
	if (familyRecord == NULL){
		return NULL;
	}
	if (compare == NULL){
		return NULL;
	}
	if (person == NULL){
		return NULL;
	}
	Individual *indiv = (Individual *)findElement(familyRecord->individuals, compare, person);
	if (indiv == NULL){
		return NULL;
	}
	else{
		return indiv;
	}
}

List getDescendants(const GEDCOMobject* familyRecord, const Individual* person){
	//List list;// = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals); 
	/*if (person == NULL){
		//return NULL;
	}*/
	List list = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals); 
	if (familyRecord != NULL && person != NULL){
		getDes(familyRecord, &list, person);
	}
	else{
		list.head = NULL;
	}
	return list;
}

void deleteGEDCOM(GEDCOMobject* obj){
	if (obj != NULL){
		if (obj->header != NULL){
			clearList(&obj->header->otherFields);
			free(obj->header);
		}
		if (obj->submitter != NULL){
			clearList(&obj->submitter->otherFields);
			free(obj->submitter);
		}
		if (&obj->individuals.head != NULL){
			clearList(&obj->individuals);
		}
		if (&obj->families.head != NULL){
			clearList(&obj->families);
		}
		free(obj);
	}
	obj = NULL;
}

char* printError(GEDCOMerror err){
	char *string = malloc(sizeof(char) * 256);
	strncpy(string, "========ERROR========\nType: ", 29);
	if (err.type == 0){
		strcat(string, "OK\n");
	}
	else if (err.type == 1){
		strcat(string, "Invalid File\n");
	}
	else if (err.type == 2){
		strcat(string, "Invalid GEDCOM\n");
	}
	else if (err.type == 3){
		strcat(string, "Invalid Header\n");
	}
	else if (err.type == 4){
		strcat(string, "Invalid Record\n");
	}
	else if (err.type == 5){
		strcat(string, "Other Error\n");
	}
	else{
		strcat(string, "\n");
	}
	string = realloc(string, sizeof(char) * (strlen(string) + 14));
	strcat(string,"line Number: ");
	char *lineNum = malloc(sizeof(int));
	sprintf(lineNum, "%d", err.line);
	string = realloc(string, sizeof(char) * (strlen(string) + strlen(lineNum) + 2));
	strcat(string, lineNum);
	strcat(string, "\n");
	free(lineNum);
	return string;
}

GEDCOMerror writeGEDCOM(char *fileName, const GEDCOMobject *obj){
	GEDCOMerror error;
	error.type = OK;
	error.line = -1;
	if (obj == NULL){
		error.type = WRITE_ERROR;
		return error;
	}
	//error.type = validateGEDCOM(obj);
	if (fileName == NULL){
		error.type = WRITE_ERROR;
		return error;
	}
	if (strcmp(fileName, "") == 0){
		error.type = WRITE_ERROR;
		return error;
	}
	if(error.type != OK){
		error.type = WRITE_ERROR;
		return error;
	}
	int i = 0;
	int fileNameLength = strlen(fileName);
	char newFileName[fileNameLength + 1];
	for (i = 0; i < strlen(fileName) - 1; i ++){
		newFileName[i] = tolower(fileName[i]);
	}
	
	if ((newFileName[fileNameLength - 4] != '.') || (newFileName[fileNameLength - 3] != 'g') || (newFileName[fileNameLength - 2] != 'e') || (fileName[fileNameLength - 1] != 'd')){
		error.type = INV_FILE;
		error.line = -1;
		return error;
	}
	FILE *fp = fopen(fileName, "w");
	if (fp == NULL){
		error.type = INV_FILE;
		error.line = -1;
		return error;
	}
	error = writeHead(fp, obj);
	if (error.type != OK){
		//printf("HI?\n");
		return error;
	}
	error = writeSubmitter(fp, obj);
	if (error.type != OK){
		//printf("HI!\n");
		return error;
	}
	error = writeIndivs(fp, obj);
	fclose(fp);
	//error.type = WRITE_ERROR;
	return error;
}

ErrorCode validateGEDCOM(const GEDCOMobject* obj){
	if (obj == NULL){
		return INV_GEDCOM;
	}
	if (obj->header == NULL){
		return INV_GEDCOM;
	}
	if (obj->submitter == NULL){
		return INV_GEDCOM;
	}
	if (strcmp(obj->header->source,"") == 0){
		return INV_HEADER;
	}
	if (obj->header->submitter == NULL){
		return INV_HEADER;
	}
	if (obj->submitter->submitterName == NULL){
		return INV_RECORD;
	}
	if (strcmp(obj->submitter->submitterName, "") == 0){
		return INV_RECORD;
	}
	Node *indivs = obj->individuals.head;
	while(indivs != NULL){
		if (indivs->data == NULL){
			return INV_RECORD;
		}
		Individual *person = (Individual *)indivs->data;
		if (strlen(person->givenName) > 200){
			return INV_RECORD;
		}
		if (strlen(person->surname) > 200){
			return INV_RECORD;
		}
		Node *events = person->events.head;
		while (events != NULL){
			if (events->data == NULL){
				return INV_RECORD;
			}
			Event *event = (Event *)events->data;
			if (strcmp(event->type, "") == 0){
				return INV_RECORD;
			}
			if (event->place == NULL){
				return INV_RECORD;
			}
			if (strlen(event->place) > 200){
				return INV_RECORD;
			}
			if (event->date == NULL){
				return INV_RECORD;
			}
			if (strlen(event->date) > 200){
				return INV_RECORD;
			}
			events = events->next;
		}
		Node *fields = person->otherFields.head;
		while (fields != NULL){
			if (fields->data == NULL){
				return INV_RECORD;
			}
			Field *field = (Field *)fields->data;
			if (field->tag == NULL){
				return INV_RECORD;
			}
			if (strlen(field->tag) > 200){
				return INV_RECORD;
			}
			if (field->value == NULL){
				return INV_RECORD;
			}
			if (strlen(field->value) > 200){
				return INV_RECORD;
			}
			fields = fields->next;
		}
		indivs = indivs->next;
	}
	
	indivs = obj->families.head;
	while(indivs != NULL){
		if (indivs->data == NULL){
			return INV_RECORD;
		}
		Family *person = (Family *)indivs->data;
		Node *events = person->events.head;
		while (events != NULL){
			if (events->data == NULL){
				return INV_RECORD;
			}
			Event *event = (Event *)events->data;
			if (strcmp(event->type, "") == 0){
				return INV_RECORD;
			}
			if (event->place == NULL){
				return INV_RECORD;
			}
			if (strlen(event->place) > 200){
				return INV_RECORD;
			}
			if (event->date == NULL){
				return INV_RECORD;
			}
			if (strlen(event->date) > 200){
				return INV_RECORD;
			}
			events = events->next;
		}
		Node *fields = person->otherFields.head;
		while (fields != NULL){
			if (fields->data == NULL){
				return INV_RECORD;
			}
			Field *field = (Field *)fields->data;
			if (field->tag == NULL){
				return INV_RECORD;
			}
			if (strlen(field->tag) > 200){
				return INV_RECORD;
			}
			if (field->value == NULL){
				return INV_RECORD;
			}
			if (strlen(field->value) > 200){
				return INV_RECORD;
			}
			fields = fields->next;
		}
		indivs = indivs->next;
	}
	
	return OK;
}

List getDescendantListN(const GEDCOMobject* familyRecord, const Individual* person, unsigned int maxGen){
	List list = initializeList(&printGeneration, &deleteList, &compareGenerations); 
	if (familyRecord != NULL && person != NULL){
		//maxGen++;
		getDesN(person, &list, maxGen, 1);
	}
	else{
		list.head = NULL;
	}
	Node *node = list.head;
	while(node != NULL){
		List *list2 = (List *)node->data;
		//Node *node2 = list2->head;
		if (list2->head == NULL){
			deleteDataFromList(&list, list2);
			clearList(list2);
			free(list2);
			break;
		}
		
		node = node->next;
	}
	return list;
}
List getAncestorListN(const GEDCOMobject* familyRecord, const Individual* person, int maxGen){
	List list = initializeList(&printIndividual, &deleteList, &compareGenerations); 
	if (familyRecord != NULL && person != NULL){
		getAnsN(person, &list, maxGen, 1);
	}
	else{
		list.head = NULL;
	}
	Node *node = list.head;
	while(node != NULL){
		List *list2 = (List *)node->data;
		//Node *node2 = list2->head;
		if (list2->head == NULL){
			deleteDataFromList(&list, list2);
			clearList(list2);
			free(list2);
			break;
		}
		
		node = node->next;
	}
	
	return list;
}

char* indToJSON(const Individual* ind){
	char *str;
	if (ind == NULL || ind->givenName == NULL || ind->surname == NULL){
		str = malloc(sizeof(char) * 1);
		strncpy(str, "\0", 1);
		return str;
	}
	str = malloc(sizeof(char) * 255);
	strncpy(str, "{\"givenName\":\"", 16);
	strcat(str, ind->givenName);
	strcat(str,"\",\"surname\":\"");
	strcat(str, ind->surname);
	strcat(str,"\"}");
	return str;
}

Individual* JSONtoInd(const char* str){
	if (str == NULL){
		return NULL;
	}
	int flag = 0;
	Individual *indiv = malloc(sizeof(Individual));
	char *string = malloc(sizeof(char) * (strlen(str) + 1));
	strncpy(string, str, strlen(str) + 1);
	char givenName[200];
	char surname[200];
	char *token = strtok(string, "\"");
	if (token == NULL){
		deleteIndividual(indiv);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteIndividual(indiv);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteIndividual(indiv);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteIndividual(indiv);
		free(string);
		return NULL;
	}
	if (strcmp(token, ",") == 0){
		strcpy(token, "");
		flag = 1;
	}
	strcpy(givenName, token);
	if (flag == 0){
		token = strtok(NULL, "\"");
	}
	if (token == NULL){
		deleteIndividual(indiv);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteIndividual(indiv);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteIndividual(indiv);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteIndividual(indiv);
		free(string);
		return NULL;
	}
	if (strcmp(token, "}") == 0){
		strcpy(token, "");
		flag = 1;
	}
	strcpy(surname, token);
	free(string);
	indiv->givenName = malloc(sizeof(char) * (strlen(givenName) + 1));
	strncpy(indiv->givenName, givenName, strlen(givenName) + 1);
	indiv->surname = malloc(sizeof(char) * (strlen(surname) + 1));
	strncpy(indiv->surname, surname, strlen(surname) + 1);
	indiv->events = initializeList(&printEvent, &deleteEvent, &compareEvents); 
	indiv->families = initializeList(&printFamily, &deleteFamily, &compareFamilies); 
	indiv->otherFields = initializeList(&printField, &deleteField, &compareFields);
	printf("created\n");
	return indiv;
}

GEDCOMobject* JSONtoGEDCOM(const char* str){
	int flag = 0;
	if (str == NULL){
		return NULL;
	}
	GEDCOMobject *obj = malloc(sizeof(GEDCOMobject));
	(obj)->individuals = initializeList(&printIndividual, &deleteIndividual, &compareIndividuals);
	(obj)->families = initializeList(&printFamily, &deleteFamily, &compareFamilies);
	(obj)->submitter = NULL;
	(obj)->header = NULL;
	char subName[200];
	char subAddress[200];
	//printf("%s\n", str);
	char *string = malloc(sizeof(char) * (strlen(str) + 1));
	strncpy(string, str, strlen(str) + 1);
	char *token = strtok(string, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL || strcmp(token, "source") != 0){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	//source
	//printf("%s:", token);
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	if (strcmp(token, ",") == 0){
		strcpy(token,"");
		flag = 1;
	}
	//printf("%s\n", token);
	obj->header = malloc(sizeof(Header));
	obj->header->submitter = NULL;
	obj->header->otherFields = initializeList(&printField, &deleteField, &compareFields);
	strcpy(obj->header->source, token);
	if (flag == 0){
		token = strtok(NULL, "\"");
	}
	//token = strtok(NULL, "\"");
	flag = 0;
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL || strcmp(token, "gedcVersion") != 0){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	//gedcVersion
	//printf("%s:", token);
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	if (strcmp(token, ",") == 0){
		strcpy(token,"");
		flag = 1;
	}
	obj->header->gedcVersion = atof(token);
	
	//printf("%s\n", token);
	if (flag == 0){
		token = strtok(NULL, "\"");
	}
	flag = 0;
	
	//token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	//printf("%s:", token);
	if (token == NULL || strcmp(token,"encoding") != 0){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	//encoding
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	if (strcmp(token,"ANSEL") == 0){
		obj->header->encoding = ANSEL;
	}
	else if (strcmp(token,"UTF-8") == 0){
		obj->header->encoding = UTF8;
	}
	else if (strcmp(token,"UNICODE") == 0){
		obj->header->encoding = UNICODE;
	}
	else if (strcmp(token,"ASCII") == 0){
		obj->header->encoding = ASCII;
	}
	else {
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	//printf("%s\n", token);
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL || strcmp(token,"subName") != 0){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	//subName
	//printf("%s:", token);
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	if (strcmp(token, ",") == 0){
		strcpy(token,"");
		flag = 1;
	}
	strcpy(subName, token);
	if (flag == 0){
		token = strtok(NULL, "\"");
	}
	flag = 0;
	
	//printf("%s\n", token);
	//token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL || strcmp(token,"subAddress") != 0){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	//subAddress
	//printf("%s:", token);
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	token = strtok(NULL, "\"");
	if (token == NULL){
		deleteGEDCOM(obj);
		free(string);
		return NULL;
	}
	if (strcmp(token, "}") == 0){
		strcpy(token,"");
		flag = 1;
	}
	strcpy(subAddress, token);
	obj->submitter = malloc(sizeof(Submitter) + (sizeof(char) * strlen(subAddress) + 1));
	obj->submitter->otherFields = initializeList(&printField, &deleteField, &compareFields);
	//obj->submitter->submitterName = malloc(sizeof(char) * strlen(subName) + 1);
	strcpy(obj->submitter->submitterName, subName);
	strcpy(obj->submitter->address, subAddress);
	obj->header->submitter = obj->submitter;
	//printf("%s\n", token);
	free(string);
	//deleteGEDCOM(obj);
	return obj;
}

void addIndividual(GEDCOMobject* obj, const Individual* toBeAdded){
	if (obj == NULL){
		return;
	}
	if (toBeAdded == NULL){
		return;
	}
	insertBack(&obj->individuals, (void *)toBeAdded);
}

char* iListToJSON(List iList){
	char *str = malloc(sizeof(char) * 3);
	char *indiS;// = malloc(sizeof(char) * 400);
	if (iList.head == NULL){
		strncpy(str,"[]", 3);
		return str;
	}
	strncpy(str,"[", 2);
	Node *indivs = iList.head;
	while(indivs != NULL){
		Individual *ind = (Individual *)indivs->data;
		//indiS = malloc(sizeof(char) * 400);
		indiS = indToJSON(ind);
		str = realloc(str, ((sizeof(char) * strlen(str)) + (sizeof(char) * strlen(indiS) + 5)));
		strcat(str, indiS);
		free(indiS);
		if (indivs->next != NULL){
			strcat(str,",");
		}
		indivs = indivs->next;
	}
	str = realloc(str, sizeof(char) * strlen(str) + 2);
	strcat(str,"]");

	return str;
}

char* gListToJSON(List gList){
	char *str = malloc(sizeof(char) * 4);
	char *gen;// = malloc(sizeof(char) * 400);
	if (gList.head == NULL){
		strncpy(str,"[]", 3);
		return str;
	}
	strncpy(str,"[", 2);
	Node *genNode = gList.head;
	while(genNode != NULL){
		List *list = (List *)genNode->data;
		gen = iListToJSON(*list);
		str = realloc(str, ((sizeof(char) * strlen(str)) + (sizeof(char) * strlen(gen) + 5)));
		strcat(str, gen);
		if (genNode->next != NULL){
			strcat(str,",");
		}
		
		free(gen);
		
		genNode = genNode->next;
	}
	str = realloc(str, (sizeof(char) * strlen(str) + 3));
	strcat(str,"]");
	return str;
}


void deleteGeneration(void* toBeDeleted){
	clearList((List *)toBeDeleted);
}
int compareGenerations(const void* first,const void* second){
	List *list1 = (List *)first;
	List *list2 = (List *)second;
	return (list1->length - list2->length);
	
}
char* printGeneration(void* toBePrinted){
	char *generation = malloc(sizeof(char) * 100);
	strcpy(generation, "========GENERATION========\n");
	if (toBePrinted == NULL){
		return generation;
	}
	List *list = (List *)toBePrinted;
	Node *node = list->head;
	while(node != NULL){
		char *toAdd = printIndividual(node->data);
		generation = realloc(generation, ((sizeof(char) * strlen(generation)) + (sizeof(char) * strlen(toAdd) + 5)));
		strcat(generation, toAdd);
		free(toAdd);
		
		node = node->next;
	}
	return generation;
}

void deleteEvent(void* toBeDeleted){
	Event *event = (Event *)toBeDeleted;
	if(event->date != NULL){
		free(event->date);
	}
	if (event->place != NULL){
		free(event->place);
	}
	clearList(&event->otherFields);
	free(event);
	event = NULL;  
	return; 
}
int compareEvents(const void* first,const void* second){
	Event *event1 = (Event *)first;
	Event *event2 = (Event *)second;
	
	return (strcmp(event1->type, event2->type));
}
char* printEvent(void* toBePrinted){
	char *str = malloc(sizeof(char) * 255);
	Event *event = (Event *)toBePrinted;
	strcpy(str, "==========Event==========\nType: ");
	str = realloc(str, strlen(str) + strlen(event->type) + 9);
	strcat(str, event->type);
	strcat(str, "\nDate: |");
	str = realloc(str, strlen(str) + strlen(event->date) + 11);
	strcat(str, event->date);
	strcat(str, "|\nPlace: |");
	str = realloc(str, strlen(str) + strlen(event->place) + 3);
	strcat(str, event->place);
	strcat(str, "|\n");
	return str;
}
char* printGEDCOM(const GEDCOMobject* obj){
	char *gedCom = malloc(sizeof(char) * 255);
	strcpy(gedCom, "=======GEDCOM========\n");
	if (obj == NULL){
		printf("NO");
		return gedCom;
	}
	strcat(gedCom, "Source: |");
	strcat(gedCom, obj->header->source);
	gedCom = realloc(gedCom, sizeof(char) * strlen(gedCom) + 19);
	strcat(gedCom, "|\nGedcom Version: ");
	char *vers = malloc(sizeof(char) * 100);
	sprintf(vers, "%1.2f", obj->header->gedcVersion);
	gedCom = realloc(gedCom, sizeof(char) * (strlen(gedCom) + strlen(vers) + 1));
	strcat(gedCom, vers);
	free(vers);
	gedCom = realloc(gedCom, sizeof(char) * strlen(gedCom) + 12);
	strcat(gedCom, "\nEncoding: ");
	char *encoding = malloc(sizeof(char) * 50);
	if (obj->header->encoding == 0){
		strncpy(encoding, "ANSEL", 6);
	}
	else if (obj->header->encoding == 1){
		strncpy(encoding, "UTF8", 5);
	}
	else if (obj->header->encoding == 2){
		strncpy(encoding, "UNICODE", 8);
	}
	else if (obj->header->encoding == 3){
		strncpy(encoding, "ASCII", 6);
	}
	gedCom = realloc(gedCom, sizeof(char) * (strlen(gedCom) + strlen(encoding) + 1));
	strcat(gedCom, encoding);
	free(encoding);
	gedCom = realloc(gedCom, sizeof(char) * (strlen(gedCom) + 19));
	strcat(gedCom, "\nSubmitter Name: |");
	gedCom = realloc(gedCom, sizeof(char) * (strlen(gedCom) + strlen(obj->header->submitter->submitterName) + 1));
	strcat(gedCom, obj->submitter->submitterName);
	
	gedCom = realloc(gedCom, sizeof(char) * (strlen(gedCom) + 22));
	strcat(gedCom, "|\nSubmitter Address: ");
	
	gedCom = realloc(gedCom, sizeof(char) * (strlen(gedCom) + strlen(obj->submitter->address) + 2));
	strcat(gedCom, obj->submitter->address);
	strcat(gedCom, "\n");

	return gedCom;
	if (obj->header->otherFields.head != NULL){
		char *fields = toString(obj->header->otherFields);
		gedCom = realloc(gedCom, sizeof(char) * (strlen(gedCom) + strlen(fields) + 1));
		strcat(gedCom, fields);
		free(fields);
	}
	
	if(obj->families.head != NULL){
		char *families = toString(obj->families);
		gedCom = realloc(gedCom, sizeof(char) * (strlen(gedCom) + strlen(families) + 1));
		strcat(gedCom, families);
		free(families);	
	}
	
	if(obj->individuals.head != NULL){
		char *individuals = toString(obj->individuals);
		gedCom = realloc(gedCom, sizeof(char) * (strlen(gedCom) + strlen(individuals) + 1));
		strcat(gedCom, individuals);
		free(individuals);	
	}
	
	return gedCom;
}

void deleteFamily(void* toBeDeleted){
	Family *fam = (Family *)toBeDeleted;
	clearList(&fam->events);
	Node *current = fam->children.head;
	Node *temp;
	while (current != NULL){
		temp = current;
		current = current->next;
		free(temp);
	}
	clearList(&fam->otherFields);
	free(fam);
	fam = NULL;
	return;
}
int compareFamilies(const void* first,const void* second){
	int numFam1 = 0;
	int numFam2 = 0;
	Family *fam1 = (Family *)first;
	Family *fam2 = (Family *)second;
	
	if (fam1->husband != NULL){
		numFam1++;
	}
	if (fam1->wife != NULL){
		numFam1++;
	}
	
	Node *curr1 = fam1->children.head;
	while (curr1 != NULL){
		numFam1++;
		curr1 = curr1->next;
	}
	
	if (fam2->husband != NULL){
		numFam2++;
	}
	if (fam2->wife != NULL){
		numFam2++;
	}
	
	Node *curr2 = fam2->children.head;
	while (curr2 != NULL){
		numFam2++;
		curr2 = curr2->next;
	}
	
	if (numFam1 < numFam2){
		return -1;
	}
	else if (numFam1 > numFam2){
		return 1;
	}
	else{
		return 0;
	}
}
char* printFamily(void* toBePrinted){
	char *str = malloc(sizeof(char) * 255);
	Family *fam = (Family *)toBePrinted;
	strncpy(str, "======FAMILY-", 14);
	Node *current = fam->otherFields.head;
	while(current != NULL){
		Field *field = (Field *)current->data;
		if (strcmp(field->tag, "XREF") == 0){
			strcat(str, field->value);
			break;
		}
		current = current->next;
	}
	strcat(str,"======\n");
	str = realloc(str, sizeof(char) * (strlen(str) + 27));
	strcat(str, "=========HUSBAND=========\n");
	char *husband = NULL;
	if (fam->husband != NULL){
		husband = printIndividual(fam->husband);
		str = realloc(str, sizeof(char) * (strlen(str) + strlen(husband) + 1));
		strcat(str, husband);
		free(husband);
	}
	str = realloc(str, sizeof(char) * (strlen(str) + 27));
	strcat(str, "==========WIFE===========\n");
	char *wife = NULL;
	if (fam->wife != NULL){
		wife = printIndividual(fam->wife);
		str = realloc(str, sizeof(char) * (strlen(str) + strlen(wife) + 1));
		strcat(str, wife);
		free(wife);
	}
	Node *curChild = fam->children.head;
	while(curChild != NULL){
		str = realloc(str, sizeof(char) * (strlen(str) + 26));
		strcat(str, "==========CHILD==========");
		char *child = printIndividual(curChild->data);
		str = realloc(str, sizeof(char) * (strlen(str) + strlen(child) + 1));
		strcat(str, child);
		free(child);
		curChild = curChild->next;
	}
	Node *fields = fam->otherFields.head;
	str = realloc(str, sizeof(char) * (strlen(str) + 27));
	strcat(str, "_______FAM-Fields________\n");
	while(fields != NULL){
		char *fieldChar = printField(fields->data);
		str = realloc(str, sizeof(char) * (strlen(str) + strlen(fieldChar) + 1));
		strcat(str, fieldChar);
		free(fieldChar);
		fields = fields->next;
	}
	Node *events = fam->events.head;
	str = realloc(str, sizeof(char) * (strlen(str) + 27));
	strcat(str, "_______FAM-EVENTS________\n");
	while(events != NULL){
		char *eventChar = printEvent(events->data);
		str = realloc(str, sizeof(char) * (strlen(str) + strlen(eventChar) + 1));
		strcat(str, eventChar);
		free(eventChar);
		events = events->next;
	} 
	str = realloc(str, sizeof(char) * (strlen(str) + 27));
	strcat(str, "=======END=FAMILY========\n");
	return str;
}

void deleteField(void* toBeDeleted){
	Field *field = (Field *)toBeDeleted;
	if (field->tag != NULL){
		free(field->tag);
	}
	if (field->value != NULL){
		free(field->value);
	}
	free(field);
	field = NULL;
	return;
}
int compareFields(const void* first,const void* second){
	Field *field1 = (Field *)first;
	Field *field2 = (Field *)second;
	
	char *name1 = malloc(sizeof(char) * (strlen(field1->tag) + strlen(field1->value) + 3));
	char *name2 = malloc(sizeof(char) * (strlen(field2->tag) + strlen(field2->value) + 3));
	
	strncpy(name1, field1->tag, strlen(field1->tag) + 1);
	strcat(name1, " ");
	strcat(name1, field1->value);
	
	strncpy(name2, field2->tag, strlen(field2->tag) + 1);
	strcat(name2, " ");
	strcat(name2, field2->value);
	
	int val = strcmp(name1, name2);
	free(name1);
	free(name2);
	
	return val;
}
char* printField(void* toBePrinted){
	char *str = malloc(sizeof(char) * 255);
	Field *field = (Field *)toBePrinted;
	strcpy(str, "==========Field==========\nTag: |");
	str = realloc(str, strlen(str) + strlen(field->tag) + 11);
	strcat(str, field->tag);
	strcat(str, "|\nValue: |");
	str = realloc(str, strlen(str) + strlen(field->value) + 3);
	strcat(str, field->value);
	strcat(str, "|\n");
	return str;
}

void deleteIndividual(void* toBeDeleted){
	Individual *indiv = (Individual *)toBeDeleted;
	free(indiv->givenName);
	free(indiv->surname);
	clearList(&indiv->events);
	//clearList(&indiv->families);
	Node *current = indiv->families.head;
	Node *temp;
	while (current != NULL){
		temp = current;
		current = current->next;
		free(temp);
	}
	clearList(&indiv->otherFields);
	free(indiv);
}
int compareIndividuals(const void* first,const void* second){
	Individual *indiv1 = (Individual *)first;
	Individual *indiv2 = (Individual *)second;
	
	char *name1 = malloc(sizeof(char) * (strlen(indiv1->givenName) + strlen(indiv1->surname) + 3));
	char *name2 = malloc(sizeof(char) * (strlen(indiv2->givenName) + strlen(indiv2->surname) + 3));
	
	strncpy(name1, indiv1->surname, strlen(indiv1->surname) + 1);
	strcat(name1, ",");
	strcat(name1, indiv1->givenName);
	
	strncpy(name2, indiv2->surname, strlen(indiv2->surname) + 1);
	strcat(name2, ",");
	strcat(name2, indiv2->givenName);
	
	int val = strcmp(name1, name2);
	free(name1);
	free(name2);
	
	return val;
	
}
char* printIndividual(void* toBePrinted){
	char *str = malloc(sizeof(char) * 1000);
	if (toBePrinted == NULL){
		strcpy(str, "\nNone\n");
		return str;
	}
	Individual *indiv = (Individual *)toBePrinted;
	strcpy(str, "\n_______Individual________\nName: |");
	strcat(str, indiv->givenName);
	strcat(str, "|\nLast Name: |");
	if (indiv->surname != NULL){
		strcat(str, indiv->surname);
	}
	strcat(str, "|\n");
	str = realloc(str, sizeof(char) * (strlen(str) + 1));
	Node *oFields = indiv->otherFields.head;
	while (oFields != NULL){
		Field *field = (Field *)oFields->data;
		char *fieldChar = printField(field);
		str = realloc(str, sizeof(char) * (strlen(str) + strlen(fieldChar) + 1));
		strcat(str, fieldChar);
		free(fieldChar);
		oFields = oFields->next;
	}
	Node *eventsList = indiv->events.head;
	while (eventsList != NULL){
		Event *event = (Event *)eventsList->data;
		char *eventChar = printEvent(event);
		str = realloc(str, sizeof(char) * (strlen(str) + strlen(eventChar) + 1));
		strcat(str, eventChar);
		free(eventChar);
		eventsList = eventsList->next;
	}
	str = realloc(str, sizeof(char) * strlen(str) + 27);
	strcat(str, "___________END___________\n");
	return str;
}
