#include "GPXParser.h"
#include "GPXHelpers.h"
#include "LinkedListAPI.h"

/*
 * Author:      Michael Lam
 * Student ID:  1079127
 * Email:       mlam11@uoguelph.ca
 * Professor:   Denis Nikitenko
 * Program:     GPXParser.c
 * References:  Snippets of the following code were implemented into this file 
 *              from the instructor: Professor Denis Nikitenko's example code
 *              Snippets of this sample code from http://www.xmlsoft.org/examples/tree1.c
 *              were used in the following GPXParser API
 */

GPXdoc *createGPXdoc(char *filename){
    
    /* Variable declarations */
    GPXdoc *parsedGPXdoc;
    xmlDoc *document = NULL;
    xmlNode *root = NULL;

    // allocate memory for required structs
    parsedGPXdoc = (GPXdoc *) malloc(sizeof(GPXdoc)); 
    
    //check if file is valid
    if(filename == NULL){
        fprintf(stderr, "error: invalid file\n");
        free(parsedGPXdoc);
        return NULL;
    }

    // initialize the library and check potential ABI mismatches
    LIBXML_TEST_VERSION

    // read the gpx file
    document = xmlReadFile(filename, NULL, 0);

    if(document == NULL){
        fprintf(stderr, "Invalid GPX file\n");
        free(parsedGPXdoc);
        return NULL;
    }
    
    //Initialize GPXdoc variables to empty, not NULL
    parsedGPXdoc -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    parsedGPXdoc -> routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    parsedGPXdoc -> tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

    root = xmlDocGetRootElement(document);

    parseGPX(root, parsedGPXdoc);

    /*free the document */
    xmlFreeDoc(document);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    return parsedGPXdoc;
}

char* GPXdocToString(GPXdoc* doc){

    char* tempStr;
    char* waypoints;
    char* routes;
    char* tracks;
    char version[256];
	int length = 0;

    if (doc == NULL){
		return NULL;
	}

    /* Get version from doc */
    sprintf(version, "%f", doc->version);

    /* Get waypoints and routes list as a string */
    waypoints = toString(doc->waypoints);
    routes = toString(doc->routes);
    tracks = toString(doc->tracks);

    length =  strlen(doc->namespace) + strlen(version) + strlen(doc->creator) + strlen(waypoints) + strlen(routes) + strlen(tracks) + 61;
    tempStr = (char *) malloc(sizeof(char)*length);
    sprintf(tempStr, "Version: %s\nCreator: %s\nNamespace: %s\n\nWaypoints\n%s\nRoutes\n%s\n\nTracks\n%s", version, doc->creator, doc->namespace, waypoints, routes, tracks);
    
    free(waypoints);
    free(routes);
    free(tracks);
    
    return tempStr;
}

void deleteGPXdoc(GPXdoc* doc){

    if(doc != NULL){

        /* Free creator char pointer */
        free(doc->creator);

        /* Deletes the entire linked list, freeing all memory */
        freeList(doc->waypoints);
        freeList(doc->routes);
        freeList(doc->tracks);

        /* Free GPXdoc structure */
        free(doc); 
    }
}

//Total number of waypoints in the GPX file
int getNumWaypoints(const GPXdoc* doc){
    
    if(doc == NULL){
        return 0;
    }

    return getLength(doc->waypoints);
}

//Total number of routes in the GPX file
int getNumRoutes(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }

    return getLength(doc->routes);
}

//Total number of tracks in the GPX file
int getNumTracks(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }

    return getLength(doc->tracks);
}

//Total number of segments in all tracks in the document
int getNumSegments(const GPXdoc* doc){

    if(doc == NULL){
        return 0;
    }
    
    Track *trk;
    ListIterator iter = createIterator(doc->tracks);
    int numSegments = 0;

    // Iterate through all tracks, and get lengths of segments list
	while ((trk = nextElement(&iter)) != NULL){
        numSegments += getLength(trk->segments);
    }

    return numSegments;
}

//Total number of GPXData elements in the document
int getNumGPXData(const GPXdoc* doc){
    
    if(doc == NULL){
        return 0;
    }

    int numGpxData = 0;
    Waypoint *wpt;
    Route *rte;
    Track *trk;

    // Initialize iterators for waypoints, routes, and tracks
    ListIterator wptIter = createIterator(doc->waypoints);
    ListIterator rteIter = createIterator(doc->routes);
    ListIterator trkIter = createIterator(doc->tracks);

    // Iterate through all tracks, add 1 if name exists and get lengths of otherData list 
	while ((wpt = nextElement(&wptIter)) != NULL){
        if(strcmp(wpt->name, "\0") != 0){
            numGpxData++;
        }
        numGpxData += getLength(wpt->otherData);
    }

    // Iterate through all routes, add 1 if name exists and get lengths of otherData list 
	while ((rte = nextElement(&rteIter)) != NULL){

        if(strcmp(rte->name, "\0") != 0){
            numGpxData++;
        }
        
        Waypoint *rtept;
        ListIterator rteptIter = createIterator(rte->waypoints);

        // Iterate through all rtepts and increment if name found or other data found
        while ((rtept = nextElement(&rteptIter)) != NULL){
            if(strcmp(rtept->name, "\0") != 0){
                numGpxData++;
            }
            numGpxData += getLength(rtept->otherData);
        }

        numGpxData += getLength(rte->otherData);
    }

    // Iterate through all tracks, add 1 if name exists and get lengths of otherData list 
	while ((trk = nextElement(&trkIter)) != NULL){

        if(strcmp(trk->name, "\0") != 0){
            numGpxData++;
        }

        TrackSegment *seg;
        ListIterator segIter = createIterator(trk->segments);

        // Iterate through all trksegs to get all trkpts
        while ((seg = nextElement(&segIter)) != NULL){

            Waypoint *trkpt;
            ListIterator trkptIter = createIterator(seg->waypoints);

            // Iterate through all trkpts for the current segments list           
            while((trkpt = nextElement(&trkptIter)) != NULL){
                if(strcmp(trkpt->name, "\0") != 0){
                    numGpxData++;
                }
                numGpxData += getLength(trkpt->otherData);
            }
        }
        numGpxData += getLength(trk->otherData);
    }
    return numGpxData;
}

/* Compare Waypoints by name */
bool compareWaypoint(const void *first, const void *second){
    
    Waypoint *wpt1;
    Waypoint *wpt2;

    if (first == NULL || second == NULL){
        return false;
    }
    
    wpt1 = (Waypoint *)first;
    wpt2 = (Waypoint *)second;

    //Waypoints are "equal" if their names are equal
    if (strcmp(wpt1->name, wpt2->name) == 0){
        return true;
    }else{
        return false;
    }
}

// Function that returns a waypoint with the given name.  If more than one exists, return the first one.  
// Return NULL if the waypoint does not exist
Waypoint* getWaypoint(const GPXdoc* doc, char* name){

    if((doc == NULL) || (name == NULL)){
        return NULL;
    }

    /* Allocate memory for the waypoint being searched for and store the given search name */
    Waypoint *searchWpt = (Waypoint *) malloc(sizeof(Waypoint));
    searchWpt->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(searchWpt->name, name);

    /* Search for the waypoint */
    Waypoint *foundWpt = findElement(doc->waypoints, &compareWaypoint, searchWpt);

    /* Free any allocated data */
    free(searchWpt->name);
    free(searchWpt);

    if(foundWpt == NULL){
        return NULL;
    }

    return foundWpt;
}

/* Compare Routes by name */
bool compareRoute(const void *first, const void *second){
    
    Route *rte1;
    Route *rte2;

    if (first == NULL || second == NULL){
        return false;
    }
    
    rte1 = (Route *)first;
    rte2 = (Route *)second;

    //Waypoints are "equal" if their names are equal
    if (strcmp(rte1->name, rte2->name) == 0){
        return true;
    }else{
        return false;
    }
}

// Function that returns a route with the given name.  If more than one exists, return the first one.  
// Return NULL if the route does not exist
Route* getRoute(const GPXdoc* doc, char* name){
    
    if((doc == NULL) || (name == NULL)){
        return NULL;
    }

    /* Allocate memory for the waypoint being searched for and store the given search name */
    Route *searchRte = (Route *) malloc(sizeof(Route));
    searchRte->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(searchRte->name, name);

    /* Search for the waypoint */
    Route *foundRte = findElement(doc->routes, &compareRoute, searchRte);

    /* Free any allocated data */
    free(searchRte->name);
    free(searchRte);

    if(foundRte == NULL){
        return NULL;
    }

    return foundRte;
}

/* Compare Tracks by name */
bool compareTrack(const void *first, const void *second){
    
    Track *trk1;
    Track *trk2;

    if (first == NULL || second == NULL){
        return false;
    }
    
    trk1 = (Track *)first;
    trk2 = (Track *)second;

    //Waypoints are "equal" if their names are equal
    if (strcmp(trk1->name, trk2->name) == 0){
        return true;
    }else{
        return false;
    }
}

// Function that returns a track with the given name.  If more than one exists, return the first one. 
// Return NULL if the track does not exist 
Track* getTrack(const GPXdoc* doc, char* name){

    if((doc == NULL) || (name == NULL)){
        return NULL;
    }

    /* Allocate memory for the waypoint being searched for and store the given search name */
    Track *searchTrk = (Track *) malloc(sizeof(Track));
    searchTrk->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(searchTrk->name, name);

    /* Search for the waypoint */
    Track *foundTrk = findElement(doc->tracks, &compareTrack, searchTrk);

    /* Free any allocated data */
    free(searchTrk->name);
    free(searchTrk);

    if(foundTrk == NULL){
        return NULL;
    }

    return foundTrk;
}

/* ******************************* A2 functions  - MUST be implemented *************************** */

/* *** Module 1 *** */

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile){

    GPXdoc *validGPXdoc;

    /* if fileName is NULL or empty, if gpxSchemaFile is NULL or empty, return NULL */
    if((fileName == NULL || gpxSchemaFile == NULL) || (fileName[0] == '\0' || gpxSchemaFile[0] == '\0')){
        fprintf(stderr, "Invalid file\n");
        return NULL;
    }

    xmlDocPtr doc;

    //parse xml file
    doc = xmlReadFile(fileName, NULL, 0);
    
    //Validate the XML Tree
    if(!validateXMLTree(doc, gpxSchemaFile)){

        fprintf(stderr, "Could not parse \"%s\"\n", fileName);
        xmlFreeDoc(doc);
        return NULL;

    }else{
        /* Variable declarations */
        xmlNode *root = NULL;

        // allocate memory for required struct
        validGPXdoc = (GPXdoc *) malloc(sizeof(GPXdoc)); 
        
        // initialize the library and check potential ABI mismatches
        LIBXML_TEST_VERSION
        
        //Initialize GPXdoc variables to empty, not NULL
        validGPXdoc -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
        validGPXdoc -> routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
        validGPXdoc -> tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

        root = xmlDocGetRootElement(doc);

        parseGPX(root, validGPXdoc);

        printf("%s validates\n", fileName);
		xmlFreeDoc(doc);
	}

    xmlCleanupParser();

    return validGPXdoc;
}

bool writeGPXdoc(GPXdoc* doc, char* fileName){
    
    //Check for any NULL pointers
    if(doc == NULL || fileName == NULL || fileName[0] == '\0'){
        fprintf(stderr, "error: failed to write\n");
        return false;
    }
    
    //Check if fileName has correct extension
    char *ext = strrchr(fileName, '.');
    if((ext == NULL) || (strcmp(ext, ".gpx") != 0)){
        fprintf(stderr, "error: invalid file extension\n");
        return false;
    }

    xmlDoc *document = gpxToTree(doc);

    if(document == NULL){
        return false;
    }

    xmlSaveFormatFileEnc(fileName, document, "UTF-8", 1);

    /*free the document */
    xmlFreeDoc(document);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();

    return true;
}

bool validateGPXDoc(GPXdoc* gpxDoc, char* gpxSchemaFile){

    // Check for NULL pointers
    if(gpxDoc == NULL || gpxSchemaFile == NULL || gpxSchemaFile[0] == '\0'){
        fprintf(stderr, "error: failed to validate GPXDoc\n");
        return false;
    }

    // Convert the gpxDoc to an XML tree
    xmlDoc *tree = gpxToTree(gpxDoc);

    if(validateXMLTree(tree, gpxSchemaFile) == false){
        fprintf(stderr, "error: failed to validate GPXDoc\n");
        return false;
    }

    // Iterate through gpxDoc and check constraints with GPXParser.h

    /*** Validate GPXdoc attributes ***/
    if(gpxDoc->namespace[0] == '\0' || gpxDoc->creator == NULL || gpxDoc->creator[0] == '\0'){
        fprintf(stderr, "error: invalid attributes, failed to validate GPXDoc\n");
        return false;
    }

    /*** Validate Waypoints ***/

    if(gpxDoc->waypoints == NULL){
        fprintf(stderr, "error: empty waypoints list, failed to validate GPXDoc\n");
        return false;
    }

    Waypoint *wpt;
    ListIterator wptIter = createIterator(gpxDoc->waypoints);

    // Iterate through list of waypoints
    while ((wpt = nextElement(&wptIter)) != NULL){
        if(!isValidWaypoint(wpt)) return false;
    }

    /*** Validate Routes ***/

    if(gpxDoc->routes == NULL){
        fprintf(stderr, "error: empty routes list, failed to validate GPXDoc\n");
        return false;
    }

    Route *rte;
    ListIterator rteIter = createIterator(gpxDoc->routes);

    // Iterate through list of routes
    while ((rte = nextElement(&rteIter)) != NULL){

        if(rte->name == NULL){
            fprintf(stderr, "error: NULL route name, failed to validate GPXDoc\n");
            return false;
        }

        Waypoint *rtept;
        ListIterator rteptIter = createIterator(rte->waypoints);

        // Iterate through list of rtepts
        while ((rtept = nextElement(&rteptIter)) != NULL){
            if(!isValidWaypoint(rtept)) return false;
        }

        GPXData *data;
        ListIterator dataIter = createIterator(rte->otherData);
        
        // Iterate through otherData for current route
        while ((data = nextElement(&dataIter)) != NULL){
            if(!isValidGPXData(data)) return false;
        }
    }

    /*** Validate Tracks ***/

    if(gpxDoc->tracks == NULL){
        fprintf(stderr, "error: empty tracks list, failed to validate GPXDoc\n");
        return false;
    }

    Track *trk;
    ListIterator trkIter = createIterator(gpxDoc->tracks);

    // Iterate through list of tracks
    while ((trk = nextElement(&trkIter)) != NULL){

        if(trk->name == NULL){
            fprintf(stderr, "error: NULL track name, failed to validate GPXDoc\n");
            return false;
        }

        TrackSegment *trkseg;
        ListIterator trksegIter = createIterator(trk->segments);

        // Iterate through track segments
        while ((trkseg = nextElement(&trksegIter)) != NULL){
            // if(!isValidWaypoint(rtept)) return false;
            
            if(trkseg->waypoints == NULL){
                fprintf(stderr, "error: empty waypoints list, failed to validate GPXDoc\n");
                return false;
            }

            Waypoint *trkpt;
            ListIterator trkptIter = createIterator(trkseg->waypoints);
            
            // Iterate through trkpts for current trkseg
            while ((trkpt = nextElement(&trkptIter)) != NULL){
                if(!isValidWaypoint(trkpt)) return false;
            }
        }

        GPXData *data;
        ListIterator dataIter = createIterator(trk->otherData);
        
        // Iterate through otherData for current track
        while ((data = nextElement(&dataIter)) != NULL){
            if(!isValidGPXData(data)) return false;
        }
    }

    /*free the document */
    xmlFreeDoc(tree);

    /*
     *Free the global variables that may
     *have been allocated by the parser.
     */
    xmlCleanupParser();

    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();

    return true;
}

/* *** Module 2 *** */

float round10(float len){
    int rounded = (int) len;
    int tens = rounded % 10;
    if(rounded > 0){
        return (tens < 5 ? rounded - tens : rounded + 10 - tens);
    }else{
        return (abs(tens) < 5 ? rounded - tens : rounded - 10 - tens);
    }
}  

float getRouteLen(const Route *rt){

    if(rt == NULL){
        return 0;
    }

    float len = 0;

    //Iterate through waypoints for rt and calculate distance
    ListIterator rteptIter = createIterator(rt->waypoints);
    Waypoint *rtept1; 
    Waypoint *rtept2;

    if((rtept1 = nextElement(&rteptIter)) != NULL){
        while ((rtept2 = nextElement(&rteptIter)) != NULL){
            len += distWaypoints(rtept1, rtept2);
            rtept1 = rtept2;
        }
    }
    return len;
}

float getTrackLen(const Track *tr){

    if(tr == NULL){
        return 0;
    }

    float len = 0;
    bool first = true;
    Waypoint *lastPt = NULL;

    // Iterate through track segments
    TrackSegment *trkseg;
    ListIterator trksegIter = createIterator(tr->segments);

    while((trkseg = nextElement(&trksegIter)) != NULL){

        // Iterate through waypoints for each track segment and calculate distance
        ListIterator trkptIter = createIterator(trkseg->waypoints);
        Waypoint *trkpt1 = nextElement(&trkptIter); 
        Waypoint *trkpt2 = NULL;
        
        while ((trkpt2 = nextElement(&trkptIter)) != NULL){

            // If there exists extra track segments, add distance between last point 
            // of previous track segement, and first point for new track segment
            if(first == true){
                if(lastPt != NULL){
                    len += distWaypoints(trkpt1, lastPt);
                }
                first = false;
            }
            len += distWaypoints(trkpt1, trkpt2);
            trkpt1 = trkpt2;
            lastPt = trkpt2;
        }
        first = true;
    }
    return len;
}

int numRoutesWithLength(const GPXdoc* doc, float len, float delta){

    if(doc == NULL || len < 0 || delta < 0){
        return 0;
    }

    int numRoutes = 0;

    // Iterate through routes and find lengths when compared with len within the delta
    Route *rte;
    ListIterator rteIterator = createIterator(doc->routes);

    while((rte = nextElement(&rteIterator)) != NULL){
        
        float differenceLen = abs(getRouteLen(rte) - len);
        
        if(differenceLen <= delta)
            numRoutes++;
    }
    return numRoutes;
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta){
    
    if(doc == NULL || len < 0 || delta < 0){
        return 0;
    }

    int numTracks = 0;

    // Iterate through routes and find lengths when compared with len within the delta
    Track *trk;
    ListIterator trkIterator = createIterator(doc->tracks);

    while((trk = nextElement(&trkIterator)) != NULL){
        
        float differenceLen = abs(getTrackLen(trk) - len);
        
        if(differenceLen <= delta){
            numTracks++;
        }
    }
    return numTracks;
}

bool isLoopRoute(const Route* route, float delta){

    if(route == NULL || delta < 0){
        return false;
    }

    // Variable declarations
    int numRtepts = 0;
    Waypoint *lastPt;

    Waypoint *rtept;
    ListIterator rteptIterator = createIterator(route->waypoints); 

    // Get first rtept
    Waypoint *firstRtept = nextElement(&rteptIterator);
    numRtepts++;

    // Iterate through waypoints of route to get last waypoint
    while((rtept = nextElement(&rteptIterator)) != NULL) {
        numRtepts++;
        lastPt = rtept;    
    }

    // Check if valid loop
    if(numRtepts >= 4 && distWaypoints(firstRtept, lastPt) <= delta) 
        return true;

    return false;
}

bool isLoopTrack(const Track* tr, float delta){

    if(tr == NULL || delta < 0){
        return false;
    }

    // Variable declaration
    Waypoint *firstTrkpt = NULL;
    Waypoint *lastPt = NULL;
    Waypoint *trkpt = NULL;
    bool isValidNumTrkpts = false;
    bool first = true;
    int numTrkpts = 0;

    // Iterate through track segments
    TrackSegment *trkseg;
    ListIterator trksegIter = createIterator(tr->segments);

    while((trkseg = nextElement(&trksegIter)) != NULL){

        // Iterate through waypoints for each track segment and get the last waypoint
        ListIterator trkptIter = createIterator(trkseg->waypoints);

        while ((trkpt = nextElement(&trkptIter)) != NULL){

            // Hold first waypoint of the track
            if(first == true){
                firstTrkpt = trkpt;
                first = false;
            }
            
            // Increment number of waypoints
            numTrkpts++;

            // Check if at least 4 waypoints in the segment
            if(numTrkpts >= 4) isValidNumTrkpts = true; 

            // Hold last waypoint
            lastPt = trkpt;
        }
        numTrkpts = 0;
    }

    // Check if valid loop
    if(isValidNumTrkpts && distWaypoints(firstTrkpt, lastPt) <= delta)
        return true;

    return false;
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){

    if(doc == NULL)
        return NULL;
    
    List *routesBetween = initializeList(&routeToString, &dummyDelete, &compareRoutes);
    
    Waypoint *src = malloc(sizeof(Waypoint));
    Waypoint *dest = malloc(sizeof(Waypoint));
    Waypoint *firstRtept;
    Waypoint *lastRtept;

    src->latitude = sourceLat;
    src->longitude = sourceLong;

    dest->latitude = destLat;
    dest->longitude = destLong;

    Route *rte;
    ListIterator rteIter = createIterator(doc->routes);

    while((rte = nextElement(&rteIter)) != NULL){

        firstRtept = getFromFront(rte->waypoints);
        lastRtept = getFromBack(rte->waypoints);

        // Calculate if route is between
        float srcDist = distWaypoints(src, firstRtept);
        float destDist = distWaypoints(dest, lastRtept);

        if(srcDist <= delta && destDist <= delta)
            insertBack(routesBetween, rte);
    }

    free(src);
    free(dest);

    if(getLength(routesBetween) == 0){
        freeList(routesBetween);
        return NULL;
    }

    return routesBetween;
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    
    if(doc == NULL)
        return NULL;
    
    List *tracksBetween = initializeList(&trackToString, &dummyDelete, &compareTracks);
    
    Waypoint *src = malloc(sizeof(Waypoint));
    Waypoint *dest = malloc(sizeof(Waypoint));
    Waypoint *firstTrkpt;
    Waypoint *lastTrkpt;
    bool first = true;

    src->latitude = sourceLat;
    src->longitude = sourceLong;

    dest->latitude = destLat;
    dest->longitude = destLong;

    Track *trk;
    ListIterator trkIter = createIterator(doc->tracks);

    while((trk = nextElement(&trkIter)) != NULL){

        TrackSegment *trkseg;
        ListIterator trksegIter = createIterator(trk->segments);

        while((trkseg = nextElement(&trksegIter)) != NULL){

            Waypoint *trkpt;
            ListIterator trkptIter = createIterator(trkseg->waypoints);

            while((trkpt = nextElement(&trkptIter)) != NULL){

                if(first == true){
                    firstTrkpt = trkpt;
                    first = false;
                }
                lastTrkpt = trkpt;
            }
        }

        // Calculate if track is between
        float srcDist = distWaypoints(src, firstTrkpt);
        float destDist = distWaypoints(dest, lastTrkpt);

        if(srcDist <= delta && destDist <= delta)
            insertBack(tracksBetween, trk);

        first = true;
    }

    free(src);
    free(dest);

    if(getLength(tracksBetween) == 0){
        freeList(tracksBetween);
        return NULL;
    }

    return tracksBetween;
}

char *pathGetRoutesBetween(char *filename, char *gpxSchemaFile, float sourceLat, float sourceLong, float destLat, float destLong, float delta){

    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);

    char *empty = malloc(3 * sizeof(char));
    strcpy(empty, "[]");

    if (gpx == NULL){
        return empty;
	}

    List *rtes = getRoutesBetween(gpx, sourceLat, sourceLong, destLat, destLong, delta);
    if(rtes != NULL){
        return routeListToJSON(rtes);
    }else{  
        deleteGPXdoc(gpx);
        return empty;
    }

    deleteGPXdoc(gpx);
    return empty;
}

char *pathGetTracksBetween(char *filename, char *gpxSchemaFile, float sourceLat, float sourceLong, float destLat, float destLong, float delta){

    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);

    char *empty = malloc(3 * sizeof(char));
    strcpy(empty, "[]");

    if (gpx == NULL){
        return empty;
	}

    List *trks = getTracksBetween(gpx, sourceLat, sourceLong, destLat, destLong, delta);
    if(trks != NULL){
        return trackListToJSON(trks);
    }else{  
        deleteGPXdoc(gpx);
        return empty;
    }
    
    deleteGPXdoc(gpx);
    return empty;
}

/* *** Module 3 *** */

int getNumTrkpts(const Track *tr){

    int numTrkpts = 0;

    TrackSegment *trkseg;
    ListIterator trksegIter = createIterator(tr->segments);

    while((trkseg = nextElement(&trksegIter)) != NULL){
        numTrkpts += getLength(trkseg->waypoints);
    }

    return numTrkpts;
}

char *wptToJSON(const Waypoint *wpt){

    if (wpt == NULL){
		char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "{}");
        return empty;
	}

    char* JSONstr;
    char *name = (wpt->name[0] == '\0' ? "None" : wpt->name);
    char strLat[1024];
    char strLon[1024];
    int length;

    sprintf(strLat, "%f", wpt->latitude);
    sprintf(strLon, "%f", wpt->longitude);

    /* Allocate enough memory for the string, +40 for extra characters and '\0' */
    length = strlen(name) + strlen(strLat) + strlen(strLon) + 40;
    JSONstr = (char*) malloc(sizeof(char)*length);
    sprintf(JSONstr, "{\"name\":\"%s\",\"latitude\":%s,\"longitude\":%s}", name, strLat, strLon);
    
    return JSONstr;
}

char* routeToJSON(const Route *rt){

    if (rt == NULL){
		char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "{}");
        return empty;
	}

    char* JSONstr;
    char *name = (rt->name[0] == '\0' ? "None" : rt->name);
    char *loopStat = (isLoopRoute(rt, 10) ? "true" : "false");
    char strNumPoints[1024];
    char strRouteLen[1024];
	int length;

    sprintf(strNumPoints, "%d", getLength(rt->waypoints));
    sprintf(strRouteLen, "%.1f", round10(getRouteLen(rt)));

    /* Allocate enough memory for the string, +40 for extra characters and '\0' */
    length = strlen(name) + strlen(loopStat) + strlen(strNumPoints) + strlen(strRouteLen) + 40;
    JSONstr = (char*) malloc(sizeof(char)*length);
    sprintf(JSONstr, "{\"name\":\"%s\",\"numPoints\":%s,\"len\":%s,\"loop\":%s}", name, strNumPoints, strRouteLen, loopStat);
    
    return JSONstr;
}

char* trackToJSON(const Track *tr){

    if (tr == NULL){
        char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "{}");
        return empty;
	}

    char* JSONstr;
    char *name = (tr->name[0] == '\0' ? "None" : tr->name);
    char *loopStat = (isLoopTrack(tr, 10) ? "true" : "false");
    char strNumPoints[1024];
    char trackLen[1024];
	int length;

    sprintf(strNumPoints, "%d", getNumTrkpts(tr));
    sprintf(trackLen, "%.1f", round10(getTrackLen(tr)));

    /* Allocate enough memory for the string, +27 for extra characters and '\0' */
    length = strlen(name) + strlen(loopStat) + strlen(strNumPoints) + strlen(trackLen) + 40;
    JSONstr = (char*) malloc(sizeof(char) * length);
    sprintf(JSONstr, "{\"name\":\"%s\",\"numPoints\":%s,\"len\":%s,\"loop\":%s}", name, strNumPoints, trackLen, loopStat);
    
    return JSONstr;
}

char* dataToJSON(const GPXData *data){

    if (data == NULL){
		char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "{}");
        return empty;
	}

    char *JSONstr;
    char *name = (char *) data->name;
    char *value = (char *) data->value;
	int length;

    if(value[strlen(value) -1] == '\n'){
        value[strlen(value)-1] = '\0';
    }

    length = strlen(name) + strlen(value) + 23;
    JSONstr = (char*) malloc(sizeof(char) * length);
    sprintf(JSONstr, "{\"name\":\"%s\",\"value\":\"%s\"}", name, value);
    
    return JSONstr;
}

char *wptListToJSON(const List *list){
    if(list == NULL || getLength((List *) list) == 0){
        char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "[]");
        return empty;
    }

    char *listJSON;
    int listSz = 0;
    int length = 0;
    int commaCounter = 0;
    int numCommas = getLength((List *) list) - 1;

    Waypoint *tempWpt;
    ListIterator tempIter = createIterator((List *) list);

    // Get size of list to allocate enough memory for JSON string
    while((tempWpt = nextElement(&tempIter)) != NULL){
        char *tempWptStr = wptToJSON(tempWpt);
        listSz += strlen(tempWptStr);
        free(tempWptStr);
    }

    // Additional memory for commas and '\0'
    listSz += numCommas + 1;

    char *waypoints = (char*) calloc(sizeof(char) * listSz, 1);
    Waypoint *wpt = NULL;
    ListIterator wptIter = createIterator((List *) list);

    // Concat all JSON route strings together
    while((wpt = nextElement(&wptIter)) != NULL){
        char *wptStr = wptToJSON(wpt);
        strcat(waypoints, wptStr);
        
        if(commaCounter++ != numCommas)
            strcat(waypoints, ",");

        free(wptStr);
    }

    length = strlen(waypoints) + 3;
    listJSON = (char*) malloc(sizeof(char) * length);
    sprintf(listJSON, "[%s]", waypoints);

    free(waypoints);

    return listJSON;
}

char* routeListToJSON(const List *list){

    if(list == NULL || getLength((List *) list) == 0){
        char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "[]");
        return empty;
    }

    char *listJSON;
    int listSz = 0;
    int length = 0;
    int commaCounter = 0;
    int numCommas = getLength((List *) list) - 1;

    Route *tempRte;
    ListIterator tempIter = createIterator((List *) list);

    // Get size of list to allocate enough memory for JSON string
    while((tempRte = nextElement(&tempIter)) != NULL){
        char *tempRteStr = routeToJSON(tempRte);
        listSz += strlen(tempRteStr);
        free(tempRteStr);
    }

    // Additional memory for commas and '\0'
    listSz += numCommas + 1;

    char *routes = (char*) calloc(sizeof(char) * listSz, 1);
    Route *rte = NULL;
    ListIterator rteIter = createIterator((List *) list);

    // Concat all JSON route strings together
    while((rte = nextElement(&rteIter)) != NULL){
        char *rteStr = routeToJSON(rte);
        strcat(routes, rteStr);
        
        if(commaCounter++ != numCommas)
            strcat(routes, ",");

        free(rteStr);
    }

    length = strlen(routes) + 3;
    listJSON = (char*) malloc(sizeof(char) * length);
    sprintf(listJSON, "[%s]", routes);

    free(routes);

    return listJSON;
}

char* trackListToJSON(const List *list){

    if(list == NULL || getLength((List *) list) == 0){
        char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "[]");
        return empty;
    }

    char *listJSON;
    int listSz = 0;
    int length = 0;
    int commaCounter = 0;
    int numCommas = getLength((List *) list) - 1;

    Track *tempTrk;
    ListIterator tempIter = createIterator((List *) list);

    // Get size of list to allocate enough memory for JSON string
    while((tempTrk = nextElement(&tempIter)) != NULL){
        char *tempTrkStr = trackToJSON(tempTrk);
        listSz += strlen(tempTrkStr);
        free(tempTrkStr);
    }

    // Additional memory for commas and '\0'
    listSz += numCommas + 1;

    char *tracks = (char*) calloc(sizeof(char) * listSz, 1);
    Track *trk;
    ListIterator trkIter = createIterator((List *) list);

    // Concat all JSON route strings together
    while((trk = nextElement(&trkIter)) != NULL){
        char *trkStr = trackToJSON(trk);
        strcat(tracks, trkStr);
        
        if(commaCounter++ != numCommas)
            strcat(tracks, ",");

        free(trkStr);
    }

    length = strlen(tracks) + 3;
    listJSON = (char*) malloc(sizeof(char) * length);
    sprintf(listJSON, "[%s]", tracks);

    free(tracks);

    return listJSON;
}

char *dataListToJSON(const List *list){

    if(list == NULL || getLength((List *) list) == 0){
        char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "[]");
        return empty;
    }

    char *listJSON;
    int listSz = 0;
    int length = 0;
    int commaCounter = 0;
    int numCommas = getLength((List *) list) - 1;

    GPXData *tempData;
    ListIterator tempIter = createIterator((List *) list);

    // Get size of list to allocate enough memory for JSON string
    while((tempData = nextElement(&tempIter)) != NULL){
        char *tempDataStr = dataToJSON(tempData);
        listSz += strlen(tempDataStr);
        free(tempDataStr);
    }

    // Additional memory for commas and '\0'
    listSz += numCommas + 1;

    char *datas = (char*) calloc(sizeof(char) * listSz, 1);
    GPXData *data = NULL;
    ListIterator dataIter = createIterator((List *) list);

    // Concat all JSON route strings together
    while((data = nextElement(&dataIter)) != NULL){
        char *dataStr = dataToJSON(data);
        strcat(datas, dataStr);
        
        if(commaCounter++ != numCommas)
            strcat(datas, ",");

        free(dataStr);
    }

    length = strlen(datas) + 3;
    listJSON = (char*) malloc(sizeof(char) * length);
    sprintf(listJSON, "[%s]", datas);

    free(datas);

    return listJSON;
}

char* GPXFileToRouteList(char *filename, char *gpxSchemaFile){
    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);

    if(gpx == NULL){
        char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "[]");
        return empty;
    }

    char *JSONstr = routeListToJSON(gpx->routes);

    deleteGPXdoc(gpx);

    return JSONstr;
}

char* GPXFileToTrackList(char *filename, char *gpxSchemaFile){
    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);

    if(gpx == NULL){
        char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "[]");
        return empty;
    }

    char *JSONstr = trackListToJSON(gpx->tracks);

    deleteGPXdoc(gpx);

    return JSONstr;
}

char* GPXFileToWaypointList(char *filename, char *gpxSchemaFile, char *rteName){
    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);

    if(gpx == NULL){
        char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "[]");
        return empty;
    }

    Route *rte = getRoute(gpx, rteName);
    char *JSONstr;

    if(rte != NULL){
        JSONstr = wptListToJSON(rte->waypoints);
    }

    deleteGPXdoc(gpx);

    return JSONstr;
}

char* GPXtoJSON(const GPXdoc *gpx){

    if (gpx == NULL){
		char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "{}");
        return empty;
	}

    char* JSONstr;
    char *crVal = gpx->creator;
    char numW[1024];
    char numR[1024];
    char numT[1024];
    char ver[256];
	int length;

    sprintf(numW, "%d", getNumWaypoints(gpx));
    sprintf(numR, "%d", getNumRoutes(gpx));
    sprintf(numT, "%d", getNumTracks(gpx));
    sprintf(ver, "%.1f", gpx->version);

    /* Allocate enough memory for the string, +40 for extra characters and '\0' */
    length = strlen(ver) + strlen(crVal) + strlen(numW) + strlen(numR) + strlen(numT) + 68;
    JSONstr = (char*) malloc(sizeof(char)*length);
    sprintf(JSONstr, "{\"version\":%s,\"creator\":\"%s\",\"numWaypoints\":%s,\"numRoutes\":%s,\"numTracks\":%s}", ver, crVal, numW, numR, numT);
    
    return JSONstr;
}

char* GPXFiletoJSON(char *filename, char *gpxSchemaFile){

    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);

    if (gpx == NULL){
		char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "{}");
        return empty;
	}

    char* JSONstr;
    char *crVal = gpx->creator;
    char numW[1024];
    char numR[1024];
    char numT[1024];
    char ver[256];
	int length;

    sprintf(numW, "%d", getNumWaypoints(gpx));
    sprintf(numR, "%d", getNumRoutes(gpx));
    sprintf(numT, "%d", getNumTracks(gpx));
    sprintf(ver, "%.1f", gpx->version);

    /* Allocate enough memory for the string, +40 for extra characters and '\0' */
    length = strlen(ver) + strlen(crVal) + strlen(numW) + strlen(numR) + strlen(numT) + 68;
    JSONstr = (char*) malloc(sizeof(char)*length);
    sprintf(JSONstr, "{\"version\":%s,\"creator\":\"%s\",\"numWaypoints\":%s,\"numRoutes\":%s,\"numTracks\":%s}", ver, crVal, numW, numR, numT);
    
    deleteGPXdoc(gpx);

    return JSONstr;
}

char *rteDataToJSON(char *filename, char *gpxSchemaFile, char *rteName){
    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);
    Route *rte = getRoute(gpx, rteName);

    if (gpx == NULL || rte == NULL){
		char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "[]");
        return empty;
	}

    char *JSONstr = dataListToJSON(rte->otherData);

    deleteGPXdoc(gpx);

    return JSONstr;
}

char *trkDataToJSON(char *filename, char *gpxSchemaFile, char *trkName){
    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);
    Track *trk = getTrack(gpx, trkName);

    if (gpx == NULL || trk == NULL){
		char *empty = malloc(3 * sizeof(char));
        strcpy(empty, "[]");
        return empty;
	}

    char *JSONstr = dataListToJSON(trk->otherData);

    deleteGPXdoc(gpx);

    return JSONstr;
}

bool renameComponent(char *filename, char *gpxSchemaFile, char *compName, char *newCompName){
    
    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);

    if (gpx == NULL){
        return false;
	}

    Route *rte = getRoute(gpx, compName);
    Track *trk = getTrack(gpx, compName);

    if(rte != NULL){
        rte->name = newCompName;
        if(validateGPXDoc(gpx, gpxSchemaFile)){
            if(writeGPXdoc(gpx, filename)){
                return true;
            }
        }
    }else if(trk != NULL){
        trk->name = newCompName;
        if(validateGPXDoc(gpx, gpxSchemaFile)){
            if(writeGPXdoc(gpx, filename)){
                return true;
            }
        }
    }

    deleteGPXdoc(gpx);
    return false;
}

bool createGPXFromInput(char *filename, char *gpxString){

    GPXdoc *newGPX = (GPXdoc *) malloc(sizeof(GPXdoc));   
    newGPX = JSONtoGPX(gpxString);

    char *newFileName = malloc(strlen(filename) + 6);
    newFileName = filename;
    strcat(newFileName, ".gpx");

    if(writeGPXdoc(newGPX, newFileName)){
        return true;
    }

    deleteGPXdoc(newGPX);

    return false;
}

bool addRouteFromInput(char *filename, char *gpxSchemaFile, char *rteJSONStr){

    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);

    if (gpx == NULL){
        return false;
	}

    Route *rte = JSONtoRoute(rteJSONStr);

    if(rte != NULL){
        addRoute(gpx, rte);
        if(validateGPXDoc(gpx, gpxSchemaFile)){
            if(writeGPXdoc(gpx, filename)){
                return true;
            }
        }
    }

    deleteGPXdoc(gpx);

    return false;
}

bool addWptToRoute(char *filename, char *gpxSchemaFile, char *rteName, char *wptJSONStr){

    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);

    if (gpx == NULL){
        return false;
	}

    Route *rte = getRoute(gpx, rteName);

    if(rte != NULL){
        Waypoint *wpt = JSONtoWaypoint(wptJSONStr);
        addWaypoint(rte, wpt);
        if(validateGPXDoc(gpx, gpxSchemaFile)){
            if(writeGPXdoc(gpx, filename)){
                return true;
            }
        }
    }

    deleteGPXdoc(gpx);

    return false;
}

int numComponents(char *filename, char *gpxSchemaFile, char *type, float len){
    GPXdoc *gpx = createValidGPXdoc(filename, gpxSchemaFile);

    if (gpx == NULL){
        return false;
	}

    int numComps = 0;

    if(strcmp(type, "track") == 0){
        numComps = numTracksWithLength(gpx, len, 10);
    }else{
        numComps = numRoutesWithLength(gpx, len, 10);
    }

    deleteGPXdoc(gpx);

    return numComps;
}



/* *** Bonus *** */

void addWaypoint(Route *rt, Waypoint *pt){
    if(rt == NULL || pt == NULL)
        return;

    insertBack(rt->waypoints, pt);
}

void addRoute(GPXdoc* doc, Route* rt){
    if(doc == NULL || rt == NULL)
        return;

    insertBack(doc->routes, rt);
}

GPXdoc* JSONtoGPX(const char* gpxString){

    if(gpxString == NULL)
        return NULL;

    GPXdoc *gpxDoc = (GPXdoc *) malloc(sizeof(GPXdoc)); 
    char *version = malloc(1);
    char *creator = malloc(1);

    // Convert JSON string to readable GPX values
    char *tempVer = strchr(gpxString, ':');
    char *tempCre = strrchr(tempVer, ':');

    // Allocate enough memory and copy parsed string for version 
    int sz = 0;
    for(int i = 1; tempVer[i] != ','; i++){
        sz++;
        version = (char *) realloc(version, sz+1);
        version[sz-1] = tempVer[i];
    }
    version[sz] = '\0';

    // Allocate enough memory and copy parsed string for creator 
    sz = 0;
    for(int i = 2; tempCre[i] != '\"'; i++){
        sz++;
        creator = (char *) realloc(creator, sz+1);
        creator[sz-1] = tempCre[i];
    }
    creator[sz] = '\0';

    // Initialize gpxDoc
    strcpy(gpxDoc->namespace, "http://www.topografix.com/GPX/1/1");
    gpxDoc -> version = atof(version); 
    gpxDoc -> creator = creator;
    gpxDoc -> waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    gpxDoc -> routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    gpxDoc -> tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

    free(version);

    return gpxDoc;
}

Waypoint* JSONtoWaypoint(const char* gpxString){

    if(gpxString == NULL)
        return NULL;

    Waypoint *pt = (Waypoint *) malloc(sizeof(Waypoint)); 
    char *lat = malloc(1);
    char *lon = malloc(1);

    // Convert JSON string to readable GPX values
    char *tempLat = strchr(gpxString, ':');
    char *tempLon = strrchr(tempLat, ':');

    // Allocate enough memory and copy parsed string for latitude 
    int sz = 0;
    for(int i = 1; tempLat[i] != ','; i++){
        sz++;
        lat = (char *) realloc(lat, sz+1);
        lat[sz-1] = tempLat[i];
    }
    lat[sz] = '\0';

    // Allocate enough memory and copy parsed string for longitude 
    sz = 0;
    for(int i = 1; tempLon[i] != '}'; i++){
        sz++;
        lon = (char *) realloc(lon, sz+1);
        lon[sz-1] = tempLon[i];
    }
    lon[sz] = '\0';

    // Initialize Waypoint
    pt->name = malloc(1);
    *pt->name = '\0';
    pt->latitude = atof(lat);
    pt->longitude = atof(lon);
    pt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    free(lat);
    free(lon);

    return pt;
}

Route* JSONtoRoute(const char* gpxString){

    if(gpxString == NULL)
        return NULL;

    Route *rt = (Route *) malloc(sizeof(Route)); 
    char *name = malloc(1);

    // Convert JSON string to readable GPX values
    char *tempName = strchr(gpxString, ':');
    printf("%s\n", tempName);
    
    // Allocate enough memory and copy parsed string for route name 
    int sz = 0;
    for(int i = 2; tempName[i] != '\"'; i++){
        sz++;
        name = (char *) realloc(name, sz+1);
        name[sz-1] = tempName[i];
    }
    name[sz] = '\0';
    
    // Initialize route
    rt->name = name;
    rt->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);

    return rt;
}

/* ******************************* List helper functions *************************** */

/* deleteGpxData will free the data inside the GPXData struct */
void deleteGpxData(void* data){
    
    if (data == NULL){
		return;
	}
    
    free((GPXData *)data);
}

/* Return GPXData structure in a readable format, if data does not exist, return NULL */
char* gpxDataToString(void* data){

    char* tempStr;
	GPXData* tempName;
	int length;

    if (data == NULL){
		return NULL;
	}

    tempName = (GPXData*)data;

    /* Allocate enough memory for the string, +3 for extra characters and '\0' */
    length = strlen(tempName->name)+strlen(tempName->value)+4;
    tempStr = (char*)malloc(sizeof(char)*length);
    sprintf(tempStr, "%s: %s ", tempName->name, tempName->value);
    
    return tempStr;
}

/* compareGpxData compares two GPXData structs looking at their name and value
*  returns 0 if the names and values are the same
*/
int compareGpxData(const void *first, const void *second){
    return 0;   
}

/* deleteWaypoint will free the data inside the Waypoint and then free the Waypoint struct itself*/
void deleteWaypoint(void* data){

    if (data == NULL){
		return;
	}

    /* Free the data inside the Waypoint struct */
    free(((Waypoint *)data)->name);
    freeList(((Waypoint *)data)->otherData);

    /* Free the Waypoint structure */
    free((Waypoint *)data);
}

/* waypointToString will return a string that contains the list contents in a readable format */
char* waypointToString(void* data){

    char* tempStr;
    char* otherData;
    char latitude[256];
    char longitude[256];
	int length;
	Waypoint* wpt;

    if (data == NULL){
		return NULL;
	}

    /* Cast data as a Waypoint */
    wpt = (Waypoint*)data;

    /* Get GPXdata list as a string */
    otherData = toString((wpt->otherData));

    /* Get longitude and latitude from Waypoint */
    sprintf(latitude, "%f", wpt->latitude);
    sprintf(longitude, "%f", wpt->longitude);

    /* Allocate enough memory for the string, +58 for extra characters including '\0' */
    length = strlen(wpt->name) + strlen(latitude) + strlen(longitude) + strlen(otherData) + 58;

    tempStr = (char*)malloc(sizeof(char)*length);
    sprintf(tempStr, "Waypoint name: %s\nlatitude: %s\nlongitude: %s\nWaypoint data: %s\n", wpt->name, latitude, longitude, otherData);

    /* Free any mallocd data */
    free(otherData);

    return tempStr;
}

/* compareWaypoints will return an integer relating to if two waypoints are the same */
int compareWaypoints(const void *first, const void *second){
    return 0;
}

/* deleteRoute will free the Route itself, and then free the Route struct */
void deleteRoute(void* data){

    if (data == NULL){
		return;
	}

    /* Free the data inside the Route struct */
    free(((Route *)data)->name);
    freeList(((Route *)data)->waypoints);
    freeList(((Route *)data)->otherData);

    /* Free the Route structure */
    free((Route *)data);
}

/* routeToString will return a string that contains the list contents in a readable format */
char* routeToString( void* data){
	Route* rte;
    char* tempStr;
    char* waypoints;
    char* otherData;
	int length;

    if (data == NULL){
		return NULL;
	}

    rte = (Route*)data;

    /* Get Waypoints list as a string */
    waypoints = toString(rte->waypoints);

    /* Get GPXdata list as a string */
    otherData = toString(rte->otherData);

    /* Allocate enough memory for the string, +44 for extra characters including '\0' */
    length = strlen(rte->name) + strlen(waypoints) + strlen(otherData) + 44;
    tempStr = (char*)malloc(sizeof(char)*length);
    sprintf(tempStr, "Route name: %s\nRoute Waypoints:\n%s\nRoute Data: %s", rte->name, waypoints, otherData);

    /* Free any mallocd data */
    free(waypoints);
    free(otherData);

    return tempStr;
}

/* compareRoutes will return an integer relating to if two routes are the same */
int compareRoutes(const void *first, const void *second){
    return 0;
}

/* deleteTrackSegment will free the contents of the TrackSegment itself, and free the TrackSegment struct */
void deleteTrackSegment(void* data){

    if (data == NULL){
		return;
	}

    freeList(((TrackSegment *)data)->waypoints); 
    free((TrackSegment *)data);
}

char* trackSegmentToString(void* data){
    char* tempStr;
    char* waypoints;
	TrackSegment* trkseg;
	int length;

    if (data == NULL){
		return NULL;
	}

    trkseg = (TrackSegment*)data;
    waypoints = toString(trkseg->waypoints);

    /* Allocate enough memory for the string, +17jj for extra characters and '\0' */
    length = strlen(waypoints) + 17;
    tempStr = (char*)malloc(sizeof(char)*length);
    sprintf(tempStr, "Track segments: %s", waypoints);
    
    free(waypoints);

    return tempStr;
}

int compareTrackSegments(const void *first, const void *second){
    return 0;
}

/* deleteTrack will free the Track itself, and then free the Route struct */
void deleteTrack(void* data){
    
    if (data == NULL){
		return;
	}

    free(((Track *)data)->name);
    freeList(((Track *)data)->segments);
    freeList(((Track *)data)->otherData);
    free((Track *)data);
}   

/* trackToString will return a string that contains the list contents in a readable format */
char* trackToString(void* data){
    char* tempStr;
    char* segments;
    char* otherData;
	Track* trk;
	int length;

    if (data == NULL){
		return NULL;
	}

    trk = (Track*)data;
    segments = toString(trk->segments);
    otherData = toString(trk->otherData);

    /* Allocate enough memory for the string, +3 for extra characters and '\0' */
    length = strlen(trk->name) + strlen(segments) + strlen(otherData) + 26;
    tempStr = (char*)malloc(sizeof(char)*length);
    sprintf(tempStr, "Track name: %s%sTrack data: %s\n", trk->name, segments, otherData);
    
    free(segments);
    free(otherData);

    return tempStr;
}

/* compareTracks will return an integer relating to if two tracks are the same */
int compareTracks(const void *first, const void *second){
    return 0;
}
