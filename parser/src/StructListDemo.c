#include <stdio.h>
#include <string.h>
#include "LinkedListAPI.h"
#include "GPXHelpers.h"
#include "LinkedListAPI.h"
#include "GPXParser.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>

// typedef struct name {
// 	char* firstName;
// 	char* lastName;
// 	unsigned int age;
// }Name;

// //printFunc will return a string that contains a humanly readable copy of the list contents
// char* printFunc(void *toBePrinted){
	
// 	char* tmpStr;
// 	Name* tmpName;
// 	int len;
	
// 	if (toBePrinted == NULL){
// 		return NULL;
// 	}
	
// 	tmpName = (Name*)toBePrinted;
		
// 	/*
// 	Length of the string is:
// 	length of the first and last names+3 spaces+4 chars for "Age:"+1 character for '\0'+20 digits to represent the age
// 	While we only need 3 digits to represent the human age, if the age is uninitialised, it can be any int value
// 	An int is 4 bytes, and needs up to 20 chars to represent it
// 	If we don't do this, our code will crash if age is uninitialized	
// 	*/
	
// 	len = strlen(tmpName->firstName)+strlen(tmpName->lastName)+28;
// 	tmpStr = (char*)malloc(sizeof(char)*len);
	
// 	sprintf(tmpStr, "%s %s Age: %d", tmpName->firstName, tmpName->lastName, tmpName->age);
	
// 	return tmpStr;
// }

// //We compare names by last name
// int compareFunc(const void *first, const void *second){
	
// 	Name* tmpName1;
// 	Name* tmpName2;
	
// 	if (first == NULL || second == NULL){
// 		return 0;
// 	}
	
// 	tmpName1 = (Name*)first;
// 	tmpName2 = (Name*)second;
	
// 	return strcmp((char*)tmpName1->lastName, (char*)tmpName2->lastName);
// }

// //We need to free the first and last names, and then the Name struct itself
// void deleteFunc(void *toBeDeleted){
	
// 	Name* tmpName;
	
// 	if (toBeDeleted == NULL){
// 		return;
// 	}
	
// 	tmpName = (Name*)toBeDeleted;
	
// 	free(tmpName->firstName);
// 	free(tmpName->lastName);
// 	free(tmpName);
// }

// int main(void){
	
// 	Name* tmpName;
// 	char tmpStr[100];
// 	int memLen;
	
// 	/* 
// 	Create the list.  The list is allocated on the stack, and initializeList returns the list struct.
// 	*/
// 	List* list = initializeList(&printFunc, &deleteFunc, &compareFunc);
	
// 	//Populate the list
// 	for (int i = 0; i < 4; i++){
// 		tmpName = (Name*)malloc(sizeof(Name));
// 		tmpName->age = (i+1)*10;
		
// 		sprintf(tmpStr, "Name%d", i);
// 		memLen = strlen(tmpStr)+2;
// 		tmpName->firstName = (char*)malloc(sizeof(char)*memLen);
// 		strcpy(tmpName->firstName, tmpStr);
		
// 		sprintf(tmpStr, "Lastname%d", i);
// 		memLen = strlen(tmpStr)+2;
// 		tmpName->lastName = (char*)malloc(sizeof(char)*memLen);
// 		strcpy(tmpName->lastName, tmpStr);
	
// 		insertBack(list, (void*)tmpName);
// 	}
		
// 	char* tmp = list->printData(getFromFront(list));
// 	printf("First element in the list is: %s\n", tmp);
// 	free(tmp);
// 	tmp = list->printData(getFromBack(list));
// 	printf("Last element in the list is: %s\n", tmp);
// 	free(tmp);
		
// 	void* elem;
	
// 	//Create an iterator - again, the iterator is allocated on the stack
// 	ListIterator iter = createIterator(list);

// 	/*
// 	Traverse the list using an iterator.  
// 	nextElement() returns NULL ones we reach the end of the list
// 	*/
// 	while ((elem = nextElement(&iter)) != NULL){
// 		Name* tmpName = (Name*)elem;
		
// 		/*
// 		We use the printData function that we created to return a string representation 
// 		of the data associated with the current node
// 		*/
// 		char* str = list->printData(tmpName);
// 		printf("%s\n", str);
		
// 		//Since list.printData dynamically allocates the string, we must free it
// 		free(str);
// 	}
	
// 	printf("\n");
	
// 	/*
// 	 Use deleteDataFromList to remove an element wirh a specific last name
// 	 We use the last name since that is what compareFunc uses to compare two Names
// 	*/ 
	
// 	Name searchName;
// 	searchName.lastName = malloc(sizeof(char)*(strlen("Lastname1")+1));
// 	strcpy(searchName.lastName, "Lastname1");
	
// 	void* retVal = deleteDataFromList(list, &searchName);
// 	if (retVal != NULL){
// 		Name* foundName = (Name*)retVal;
// 		char* nameDescr = list->printData(foundName);
// 		printf("Removed %s from the list\n", nameDescr);
// 		list->deleteData(retVal);
// 		free(nameDescr);
// 	}else{
// 		printf("A value with the last name Lastname1 not found the list\n");
// 	}
// 	free(searchName.lastName);
		
// 	char* listDescr = toString(list);
// 	printf("After removal, the list is %s\n", listDescr);
// 	free(listDescr);
	
// 	/*
// 	Crear list contents - free each node, including its contents
// 	Since the list is created in the stack, we don't need to free it. 
// 	*/
// 	freeList(list);	
	
// 	return 0;		
// }




int main(int argc, char *argv[]){

    GPXdoc *doc = createGPXdoc("simple.gpx");
    char *docString = GPXdocToString(doc);
    printf("%s\n", docString);
    free(docString);
    deleteGPXdoc(doc);

    return(0);
}






