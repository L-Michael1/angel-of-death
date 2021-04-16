#include "GPXHelpers.h"
#include "GPXParser.h"

/*
 * Author:      Michael Lam
 * Student ID:  1079127
 * Email:       mlam11@uoguelph.ca
 * Professor:   Denis Nikitenko
 * Program:     GPXHelpers.c
 * References:  Snippets of the following code were implemented into this file 
 *              from the instructor: Professor Denis Nikitenko's example code
 *              Snippets of this sample code from http://www.xmlsoft.org/examples/tree1.c
 *              were used in the following GPXParser API
 */

/** Function to trim the spaces of a string 
 *@post A valid trimmed string
 *@param str - a pointer to a string
**/
void trimSpaces(char *str){
    int i, x = 0;

    // iterate through the string and trim excess spaces
    for(i = 0; str[i]; i++){
        if(!isspace(str[i]) || (i > 0 && !isspace(str[i-1]))) str[x++] = str[i];
    }

    str[x] = '\0';
}

/** Function to parse a wpt, trkpt, or rtept
 * @pre A valid wpt, trkpt, or rtept node, and a valid initialized Waypoint structure
 * @post The wpt struct will be filled if there is data available
 * @param waypoint_node - a node that points to a wpt, trkpt, or rtept node
 * @param wpt - an initialized Waypoint structure
**/
void parseWpt(xmlNode *waypoint_node, Waypoint *wpt){

    xmlNode *sibling = NULL;
    xmlAttr *attribute = NULL;
    xmlNode *value = NULL;
    char *attrName;
    char *content;

    /* Iterate through the children of a wpt node **/
    for(sibling = waypoint_node->children; sibling != NULL; sibling = sibling->next){

        char *siblingContent = (char *)xmlNodeGetContent(sibling);

        if(sibling->type == XML_ELEMENT_NODE){

            char *childName = (char *)sibling->parent->name; 
            char *siblingName = (char *)sibling->name;                       
            trimSpaces(siblingContent);

            if(siblingContent[0] != '\0'){

                /* Parse data for the wpt node */
                if((strcmp(childName, "wpt") == 0 || strcmp(childName, "rtept") == 0 || strcmp(childName, "trkpt") == 0) && strcmp(siblingName, "name") == 0){
                    /* Parse wpt name */
                    wpt->name = (char *) realloc(wpt->name, strlen(siblingContent) + 1);
                    strcpy(wpt->name, siblingContent);

                /* Parse data for any extra data that the wpt node has */
                }else if((strcmp(childName, "wpt") == 0 || strcmp(childName, "rtept") == 0) || strcmp(childName, "trkpt") == 0){
                
                    GPXData *extraData = malloc(sizeof(GPXData) + strlen(siblingContent) + 1);
                    strcpy(extraData->name , siblingName);
                    strcpy(extraData->value, siblingContent);
                    insertBack(wpt->otherData, (void*)extraData);
                }
            }
        }   
        free(siblingContent);
    }

    /* Iterate through xml attributes for a wpt node */
    for(attribute = waypoint_node -> properties; attribute != NULL; attribute = attribute -> next){

        value = attribute->children;
        attrName = (char *)attribute->name;
        content = (char *)value->content;

        /* Retrieve WPT attributes (lat and lon) */
        if(strcmp(((char *)waypoint_node->name), "wpt") == 0 || strcmp((char *)waypoint_node->name, "rtept") == 0 || strcmp((char *)waypoint_node->name, "trkpt") == 0){
            if(strcmp(attrName, "lat") == 0){
                wpt->latitude = atof(content);
            }else if(strcmp(attrName, "lon") == 0){
                wpt->longitude = atof(content);
            }
        }
    }
}

/** Function to parse a rte node
 * @pre A valid rte node, and a valid initialized Route structure
 * @post The rte struct will be filled if there is data available
 * @param route_node - a node that points to a rte node
 * @param rte - an initialized Route structure
**/
void parseRte(xmlNode *route_node, Route *rte){

    xmlNode *sibling = NULL;

    /* Iterate through the children of a rte node */
    for(sibling = route_node->children; sibling != NULL; sibling = sibling->next){

        char *siblingContent = (char *)xmlNodeGetContent(sibling);

        if(sibling->type == XML_ELEMENT_NODE){
            char *childName = (char *)sibling->parent->name; 
            char *siblingName = (char *)sibling->name;                       
            trimSpaces(siblingContent);

            /* Parse the name for a Route structure */
            if(strcmp(childName, "rte") == 0 && strcmp(siblingName, "name") == 0){

                rte->name = (char *) realloc(rte->name, strlen(siblingContent)+1);
                strcpy(rte->name, siblingContent);
            
            /* Parse any rtept nodes */
            }else if(strcmp(childName, "rte") == 0 && strcmp(siblingName, "rtept") == 0){

                Waypoint *wpt = (Waypoint *) malloc(sizeof(Waypoint));
                wpt->name = malloc(1);
                *wpt->name = '\0';
                wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                parseWpt(sibling, wpt);
                insertBack(rte->waypoints, (void *)wpt);

            /* Parse any other data the rte node has */
            }else if(strcmp(childName, "rte") == 0){
                
                GPXData *extraData = malloc(sizeof(GPXData) + strlen(siblingContent) + 1);
                strcpy(extraData->name , siblingName);
                strcpy(extraData->value, siblingContent);
                insertBack(rte->otherData, (void*)extraData);

            }
        }   
        free(siblingContent);
    }
}

/** Function to parse a trkseg node
 * @pre A valid trkseg node, and a valid initialized list of Waypoint structures
 * @post The list of Waypoint structures will be filled if there is data available
 * @param trackSeg_node - a node that points to a trkseg node
 * @param waypoints - an initialized list of Waypoint structures
**/
void parseTrkSeg(xmlNode *trackSeg_node, List *waypoints){

    xmlNode *sibling = NULL;

    /* Iterate through the trkseg children nodes */
    for(sibling = trackSeg_node->children; sibling != NULL; sibling = sibling->next){

        if(sibling->type == XML_ELEMENT_NODE){
            char *childName = (char *)sibling->name; 
            
            /* Parse the trkpts for a trkseg node */
            if(strcmp(childName, "trkpt") == 0){
                Waypoint *wpt = (Waypoint *) malloc(sizeof(Waypoint));
                wpt->name = malloc(1);
                *wpt->name = '\0';
                wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                parseWpt(sibling, wpt);
                insertBack(waypoints, (void *)wpt);
            }
        }   
    }
}

/** Function to parse a trk node
 * @pre A valid trk node, and a valid initialized Track structure
 * @post The trk structure will be filled if there is data available
 * @param track_node - a node that points to a trkseg node
 * @param trk - an initialized Track structure
**/
void parseTrk(xmlNode *track_node, Track *trk){

    xmlNode *sibling = NULL;
    TrackSegment *trkseg = NULL;

    /* Iterate through the children of a trk node */
    for(sibling = track_node->children; sibling != NULL; sibling = sibling->next){

        char *siblingContent = (char *)xmlNodeGetContent(sibling);

        if(sibling->type == XML_ELEMENT_NODE){
            char *childName = (char *)sibling->parent->name; 
            char *siblingName = (char *)sibling->name;                       
            trimSpaces(siblingContent);
                
            /* Parse name for the trk node */
            if(strcmp(childName, "trk") == 0 && strcmp(siblingName, "name") == 0){

                trk->name = (char *) realloc(trk->name, strlen(siblingContent)+1);
                strcpy(trk->name, siblingContent);
            
            /* Parse trkseg nodes */
            }else if(strcmp(childName, "trk") == 0 && strcmp(siblingName, "trkseg") == 0){

                trkseg = (TrackSegment *) malloc(sizeof(TrackSegment));
                List *waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
                parseTrkSeg(sibling, waypoints);
                trkseg->waypoints = waypoints;
                insertBack(trk->segments, (void*)trkseg);

            /* Parse any other data for the trk node */
            }else if(strcmp(childName, "trk") == 0){
                
                GPXData *extraData = malloc(sizeof(GPXData) + strlen(siblingContent) + 1);
                strcpy(extraData->name , siblingName);
                strcpy(extraData->value, siblingContent);
                insertBack(trk->otherData, (void*)extraData);
            }
        }   
        free(siblingContent);
    }
}

/** Function to parse a GPX document
 * @pre A valid xmlNode root node (GPX), and a valid GPX document
 * @post A parsed gpx doc with valid nodes
 * @param root_node - a node that points to the root node of a GPX document
 * @param doc - a valid GPX document 
**/
void parseGPX(xmlNode *root_node, GPXdoc *doc){
    xmlNode *current_node = NULL;
    xmlAttr *attribute = NULL;
    xmlNode *value = NULL;
    char *elementName;
    char *attrName;
    char *content;
    Waypoint *wpt;
    Route *rte;
    Track *trk;

    /* Iterate through all nodes in the GPX document */
    for(current_node = root_node; current_node != NULL; current_node = current_node -> next){
        
        elementName = (char *)current_node->name;

        /* Get namespace for the GPXdoc */
        if(strcmp(elementName, "gpx") == 0){
            strcpy(doc->namespace , (char *)current_node->ns->href);
        }

        if(strcmp(elementName, "gpx") != 0 && current_node->type == XML_ELEMENT_NODE){

            /* Parse a wpt node */
            if(strcmp(elementName, "wpt") == 0){
                wpt = (Waypoint *) malloc(sizeof(Waypoint));
                wpt->name = malloc(1);
                *wpt->name = '\0';
                wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                parseWpt(current_node, wpt);
            }

            /* Parse a rte node */
            if(strcmp(elementName, "rte") == 0){
                rte = (Route *) malloc(sizeof(Route));
                rte->name = malloc(1);
                *rte->name = '\0';
                rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
                rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                parseRte(current_node, rte);
            }

            /* Parse a trk node */
            if(strcmp(elementName, "trk") == 0){
                trk = (Track *) malloc(sizeof(Track));
                trk->name = malloc(1);
                *trk->name = '\0';
                trk->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
                trk->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                parseTrk(current_node, trk);
            }
        }

        /* Iterate through xml attributes for current node */
        for(attribute = current_node -> properties; attribute != NULL; attribute = attribute -> next){

            value = attribute->children;
            attrName = (char *)attribute->name;
            content = (char *)value->content;

            /* Get the version and creator for the GPX node */
            if(strcmp(elementName, "gpx") == 0){
                if(strcmp(attrName, "version") == 0){
                    doc -> version = atof(content); 
                }else if(strcmp(attrName, "creator") == 0){
                    doc -> creator =  (char *) malloc(strlen(content) + 1);
                    strcpy(doc->creator, content);
                }
            }
        }

        /* Insert the parsed wpt to the GPXdoc's waypoints list */
        if(strcmp(elementName, "wpt") == 0){
            insertBack(doc->waypoints, (void*)wpt);

        /* Insert the parsed rte to the GPXdoc's routes list */
        }else if(strcmp(elementName, "rte") == 0){
            insertBack(doc->routes, (void*)rte);

        /* Insert the parsed trk to the GPXdoc's tracks list */
        } else if(strcmp(elementName, "trk") == 0){
            insertBack(doc->tracks, (void*)trk);
        }
        
        /* Recursively call this function for the current node's child */
        parseGPX(current_node->children, doc);
    }
}

#if defined(LIBXML_TREE_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)
/** Function to convert a GPXdoc into an xmlDoc struct (libxml tree)
 * @pre A valid GPXdoc
 * @param doc - A valid GPX document
 * @return A valid xmlDoc struct
**/
xmlDoc *gpxToTree(GPXdoc *doc){

    if(doc == NULL){
        fprintf(stderr, "error: invalid GPXdoc\n");
        return NULL;
    }

    /* Document pointer */
    xmlDocPtr document = NULL;       

    /* Node pointers */
    xmlNodePtr root_node = NULL;
    xmlNodePtr waypointNode = NULL;
    xmlNodePtr routeNode = NULL;
    xmlNodePtr rteptNode = NULL;
    xmlNodePtr trackNode = NULL;
    xmlNodePtr trksegNode = NULL;
    xmlNodePtr trkptNode = NULL;

    /* Variable declarations */
    char version[256]; 

    LIBXML_TEST_VERSION;

    /* 
     * Creates a new document, a gpx node and set it as a root node
     */
    document = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlDocSetRootElement(document, root_node);

    /*** GPX root node attributes ***/

    /* Convert version to a string */
    sprintf(version, "%.1f", doc->version);

    /* Add props to GPX node */
    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST version);
    xmlNewProp(root_node, BAD_CAST "creator", BAD_CAST doc->creator);

    /* Set namespace */
    xmlNsPtr ns = xmlNewNs(root_node, BAD_CAST doc->namespace , NULL);
    xmlSetNs(root_node, ns);

    /*** Waypoint children ***/
    Waypoint *wpt;
    ListIterator wptIter = createIterator(doc->waypoints);
    
    // Iterate through waypoints
    while ((wpt = nextElement(&wptIter)) != NULL){
        waypointNode = xmlNewChild(root_node, NULL, BAD_CAST "wpt", BAD_CAST "");
        waypointToNode(waypointNode, wpt);
    }

    /*** Route children ***/
    Route *rte;
    ListIterator rteIter = createIterator(doc->routes);
    
    // Iterate through routes
    while ((rte = nextElement(&rteIter)) != NULL){
        
        // Create rte node
        routeNode = xmlNewChild(root_node, NULL, BAD_CAST "rte", BAD_CAST "");

        // Create name node if not empty
        if(rte->name[0] != '\0'){
            xmlNewChild(routeNode, NULL, BAD_CAST "name", BAD_CAST rte->name);
        }

        // Create other data children for rte node
        GPXData *data;
        ListIterator dataIter = createIterator(rte->otherData);
        
        while ((data = nextElement(&dataIter)) != NULL){
            xmlNewChild(routeNode, NULL, BAD_CAST data->name, BAD_CAST data->value);
        }

        // Iterate through rtepts and create rtept nodes
        Waypoint *rtept;
        ListIterator rteptIter = createIterator(rte->waypoints);

        while ((rtept = nextElement(&rteptIter)) != NULL){
            rteptNode = xmlNewChild(routeNode, NULL, BAD_CAST "rtept", BAD_CAST "");
            waypointToNode(rteptNode, rtept);
        }
    }

    /*** Track children ***/
    Track *trk;
    ListIterator trkIter = createIterator(doc->tracks);
    
    while ((trk = nextElement(&trkIter)) != NULL){
        
        // Create trk node
        trackNode = xmlNewChild(root_node, NULL, BAD_CAST "trk", BAD_CAST "");

        // Create name node if not empty
        if(trk->name[0] != '\0'){
            xmlNewChild(trackNode, NULL, BAD_CAST "name", BAD_CAST trk->name);
        }

        // Create other data children for trk node
        GPXData *data;
        ListIterator dataIter = createIterator(trk->otherData);
        
        while ((data = nextElement(&dataIter)) != NULL){
            xmlNewChild(trackNode, NULL, BAD_CAST data->name, BAD_CAST data->value);
        }

        // Iterate through trksegs
        TrackSegment *trkseg;
        ListIterator trksegIter = createIterator(trk->segments);

        while ((trkseg = nextElement(&trksegIter)) != NULL){

            // Create trkseg node
            trksegNode = xmlNewChild(trackNode, NULL, BAD_CAST "trkseg", BAD_CAST "");

            // Iterate through trkpts
            Waypoint *trkpt;
            ListIterator trkptIter = createIterator(trkseg->waypoints);

            while ((trkpt = nextElement(&trkptIter)) != NULL){
                trkptNode = xmlNewChild(trksegNode, NULL, BAD_CAST "trkpt", BAD_CAST "");
                waypointToNode(trkptNode, trkpt);
            }
        }
    }
    return document;
}
#else
/** Function to convert a GPXdoc into an xmlDoc struct (libxml tree)
 * @pre A valid GPXdoc
 * @param doc - A valid GPX document
 * @return A valid xmlDoc struct
**/
xmlDoc *gpxToTree(GPXdoc *doc){
    fprintf(stderr, "tree support not compiled in\n");
    return NULL;
}
#endif

/** Function to validate a libxml tree
 * @pre A valid xmlDoc struct
 * @param tree - A valid xmlDoc struct that represents a tree
 * @param gpxSchemaFile - A valid GPX schema file
 * @return a boolean value indicating success or failure of the validation for the libxml tree
**/
bool validateXMLTree(xmlDoc *tree, char *gpxSchemaFile){

    /* If gpxSchemaFile is NULL or empty, return NULL */
    if(gpxSchemaFile == NULL || gpxSchemaFile[0] == '\0'){
        fprintf(stderr, "error: invalid schema file\n");
        return false;
    }

	xmlSchemaPtr schema = NULL;
	xmlSchemaParserCtxtPtr xsdContext;

    xmlLineNumbersDefault(1);

    //returns the parser context of xsd file
    xsdContext = xmlSchemaNewParserCtxt(gpxSchemaFile);

    //set structured error callback and parse xsd file to build an internal XML schema structure to validate
    xmlSchemaSetParserErrors(xsdContext, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(xsdContext);
    xmlSchemaFreeParserCtxt(xsdContext);

	if (tree == NULL){
        fprintf(stderr, "error: failed to parse\n");

        // free the resource
        if(schema != NULL)
            xmlSchemaFree(schema);

        xmlCleanupParser();

        return false;

    /* validate xml file with xsd file */
	}else{
		xmlSchemaValidCtxtPtr xsdContext;
		int ret;

        //Create an XML Schemas validation context based on the given schema
		xsdContext = xmlSchemaNewValidCtxt(schema);
		xmlSchemaSetValidErrors(xsdContext, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);

        //Validate the document tree
		ret = xmlSchemaValidateDoc(xsdContext, tree);

        //if valid, (ret == 0)
		if (ret != 0){
            //Free contents
		    xmlSchemaFreeValidCtxt(xsdContext);

            // free the resource
            if(schema != NULL)
                xmlSchemaFree(schema);

            xmlCleanupParser();

            return false;
        }
        //Free contents
		xmlSchemaFreeValidCtxt(xsdContext);
	}

	// free the resource
    if(schema != NULL)
        xmlSchemaFree(schema);

    return true;
}

/** Function to turn a Waypoint to a xmlDoc struct node
 * @pre A valid xmlNodePtr child_node to turn into a Waypoint node, and a pointer to a Waypoint struct
 * @param child_node - A valid xmlNodePtr child_node
 * @param wpt - A valid pointer to a Waypoint struct
**/
void waypointToNode(xmlNodePtr child_node, Waypoint *wpt){
    char lat[256];
    char lon[256];

    sprintf(lat, "%f", wpt->latitude);
    sprintf(lon, "%f", wpt->longitude);
    
    // Create wpt node and add attributes
    xmlNewProp(child_node, BAD_CAST "lat", BAD_CAST lat);
    xmlNewProp(child_node, BAD_CAST "lon", BAD_CAST lon);

    // Create name node if not empty
    if(wpt->name[0] != '\0'){
        xmlNewChild(child_node, NULL, BAD_CAST "name", BAD_CAST wpt->name);
    }

    // Create other data children
    GPXData *data;
    ListIterator dataIter = createIterator(wpt->otherData);
    
    while ((data = nextElement(&dataIter)) != NULL){
        xmlNewChild(child_node, NULL, BAD_CAST data->name, BAD_CAST data->value);
    }
}

/** Function to check the validity of GPXData
 * @param data - A pointer to a GPXData struct
 * @return a boolean value indicating whether the GPXData is valid or not
**/
bool isValidGPXData(GPXData *data){

    if(data->name == NULL || data->name[0] == '\0'){
        fprintf(stderr, "error: empty GPXData name, failed to validate GPXDoc\n");
        return false;
    }

    if(data->value == NULL || data->value[0] == '\0'){
        fprintf(stderr, "error: empty GPXData value, failed to validate GPXDoc\n");
        return false;
    }

    return true;
}

/** Function to check the validity of a Waypoint
 * @param wpt - A pointer to a Waypoint struct
 * @return a boolean value indicating whether the Waypoint is valid or not
**/
bool isValidWaypoint(Waypoint *wpt){

     if(wpt->name == NULL){
        fprintf(stderr, "error: NULL waypoint name, failed to validate GPXDoc\n");
        return false;
    }

    if(wpt->otherData == NULL){
        fprintf(stderr, "error: empty otherData list failed to validate GPXDoc\n");
        return false;
    }

    GPXData *data;
    ListIterator dataIter = createIterator(wpt->otherData);
    
    while ((data = nextElement(&dataIter)) != NULL){
        if(!isValidGPXData(data)) return false;
    }

    return true;
}

/** Function to get the distance between two waypoints using the Haversine formula
 * @param wpt1 - A pointer to a Waypoint struct
 * @param wpt2 - A pointer to a Waypoint struct
 * @return the distance between wpt1 and wpt2 
**/
float distWaypoints(Waypoint *wpt1, Waypoint *wpt2){

    int radius = 6371e3;

    float rLat1 = wpt1->latitude * (M_PI/180);
    float rLat2 = wpt2->latitude * (M_PI/180);

    float rLon1 = wpt1->longitude * (M_PI/180);
    float rLon2 = wpt2->longitude * (M_PI/180);

    float changeInLat = sin((rLat2 - rLat1)/2);
    float changeInLon = sin((rLon2 - rLon1)/2);

    float a = pow(changeInLat,2) + (cos(rLat1) * cos(rLat2) * pow(changeInLon,2));
    float c = 2 * (atan2(sqrt(a), sqrt(1-a)));
    float d = radius * c;

    return d;
}

/** Dummy delete function **/
void dummyDelete(void *data){
    return;
}