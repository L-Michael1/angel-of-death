#ifndef GPX_PARSER_H
#define GPX_PARSER_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "LinkedListAPI.h"

//M_PI is not declared in the C standard, so we declare it manually
//We will need it for A2
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

//Represents a generic GPX element/XML node - i.e. some sort of an additinal piece of data, 
// e.g. comment, elevation, desciption, etc..
typedef struct  {
    //GPXData name.  Must not be an empty string.
	char 	name[256];

    //GPXData value.  We use a C99 flexible array member, which we will discuss in class.
	//Must not be an empty string
	char	value[]; 
} GPXData;

typedef struct {
    //Waypoint name.  Must not be NULL.  May be an empty string.
    char* name;

    //Waypoint longitude.  Must be initialized.
    double longitude;

    //Waypoint latitude.  Must be initialized.
    double latitude;

    //Additional waypoint data - i.e. children of the GPX waypoint other than <name>.  
    //We will assume that all waypoint children have no children of their own
    //This can be elevation, time, etc.. Note that while the element <name> can be a child of the waypoint node,
    //the name already has its own dedicated filed in the Waypoint sruct - so do not place the name in this list
    //All objects in the list will be of type GPXData.  It must not be NULL.  It may be empty.
    List* otherData;
} Waypoint;

typedef struct {
    //Route name.  Must not be NULL.  May be an empty string.
    char* name;

    //Waypoints that make up the route
    //All objects in the list will be of type Waypoint.  It must not be NULL.  It may be empty.
    List* waypoints;

    //Additional route data - i.e. children of the GPX route other than <name>.  
    //We will assume that all waypoint children have no children of their own
    //This can be comment, description, etc.. Note that while the element <name> can be a child of the route node,
    //the name already has its own dedicated filed in the Waypoint sruct - so do not place the name in this list
    //All objects in the list will be of type GPXData.  It must not be NULL.  It may be empty.
    List* otherData;
} Route;

typedef struct {
    //Waypoints that make up the track segment
    //All objects in the list will be of type Waypoint.  It must not be NULL.  It may be empty.
    List* waypoints;
} TrackSegment;

typedef struct {
    //Track name.  Must not be NULL.  May be an empty string.
    char* name;

    //Segments that make up the track
    //All objects in the list will be of type TrackSegment.  It must not be NULL.  It may be empty.
    List* segments;

    //Additional track data - i.e. children of the GPX track other than <name>.  
    //We will assume that all waypoint children have no children of their own
    //This can be comment, description, etc.. Note that while the element <name> can be a child of the track node,
    //the name already has its own dedicated filed in the Waypoint sruct - so do not place the name in this list
    //All objects in the list will be of type GPXData.  It must not be NULL.  It may be empty.
    List* otherData;
} Track;


typedef struct {
    
    //Namespace associated with our GPX doc.  Must not be an empty string. While a real GPX doc might have
    //multiple namespaces associated with it, we will assume there is only one
    char namespace[256];
    
    //GPX version.  Must be initialized.  Will usually be 1.1
    double version;
    
    //GPX creator. Must not be NULL. Must not be an empty string.
    char* creator;
    
    //Waypoints in the GPX file
    //All objects in the list will be of type Waypoint.  It must not be NULL.  It may be empty.
    List* waypoints;

    //Routes in the GPX file
    //All objects in the list will be of type Route.  It must not be NULL.  It may be empty.
    List* routes;
    
    //Tracks in the GPX file
    //All objects in the list will be of type Track.  It must not be NULL.  It may be empty.
    List* tracks;
} GPXdoc;



//A1

/* Public API - main */

/** Function to create an GPX object based on the contents of an GPX file.
 *@pre File name cannot be an empty string or NULL.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid GPXdoc has been created and its address was returned
		or 
		An error occurred, and NULL was returned
 *@return the pinter to the new struct or NULL
 *@param fileName - a string containing the name of the GPX file
**/
GPXdoc* createGPXdoc(char* fileName);

/** Function to create a string representation of an GPX object.
 *@pre GPX object exists, is not null, and is valid
 *@post GPX has not been modified in any way, and a string representing the GPX contents has been created
 *@return a string contaning a humanly readable representation of an GPX object
 *@param obj - a pointer to an GPX struct
**/
char* GPXdocToString(GPXdoc* doc);

/** Function to delete doc content and free all the memory.
 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object had been freed
 *@return none
 *@param obj - a pointer to an GPX struct
**/
void deleteGPXdoc(GPXdoc* doc);

/* For the five "get..." functions below, return the count of specified entities from the file.  
They all share the same format and only differ in what they have to count.
 
 *@pre GPX object exists, is not null, and has not been freed
 *@post GPX object has not been modified in any way
 *@return the number of entities in the GPXdoc object
 *@param obj - a pointer to an GPXdoc struct
 */


//Total number of waypoints in the GPX file
int getNumWaypoints(const GPXdoc* doc);

//Total number of routes in the GPX file
int getNumRoutes(const GPXdoc* doc);

//Total number of tracks in the GPX file
int getNumTracks(const GPXdoc* doc);

//Total number of segments in all tracks in the document
int getNumSegments(const GPXdoc* doc);

//Total number of GPXData elements in the document
int getNumGPXData(const GPXdoc* doc);

// Function that returns a waypoint with the given name.  If more than one exists, return the first one.  
// Return NULL if the waypoint does not exist
Waypoint* getWaypoint(const GPXdoc* doc, char* name);
// Function that returns a track with the given name.  If more than one exists, return the first one. 
// Return NULL if the track does not exist 
Track* getTrack(const GPXdoc* doc, char* name);
// Function that returns a route with the given name.  If more than one exists, return the first one.  
// Return NULL if the route does not exist
Route* getRoute(const GPXdoc* doc, char* name);



/* ******************************* A2 functions  - MUST be implemented *************************** */


//Module 1

/** Function to create an GPX object based on the contents of an GPX file.
 * This function must validate the XML tree generated by libxml against a GPX schema file
 * before attempting to traverse the tree and create an GPXdoc struct
 *@pre File name cannot be an empty string or NULL.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid GPXdoc has been created and its address was returned
		or 
		An error occurred, and NULL was returned
 *@return the pinter to the new struct or NULL
 *@param gpxSchemaFile - the name of a schema file
 *@param fileName - a string containing the name of the GPX file
**/
GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile);

/** Function to validating an existing a GPXobject object against a GPX schema file
 *@pre 
    GPXdoc object exists and is not NULL
    schema file name is not NULL/empty, and represents a valid schema file
 *@post GPXdoc has not been modified in any way
 *@return the boolean aud indicating whether the GPXdoc is valid
 *@param doc - a pointer to a GPXdoc struct
 *@param gpxSchemaFile - the name of a schema file
 **/
bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile);

/** Function to writing a GPXdoc into a file in GPX format.
 *@pre
    GPXdoc object exists, is valid, and and is not NULL.
    fileName is not NULL, has the correct extension
 *@post GPXdoc has not been modified in any way, and a file representing the
    GPXdoc contents in GPX format has been created
 *@return a boolean value indicating success or failure of the write
 *@param
    doc - a pointer to a GPXdoc struct
 	fileName - the name of the output file
 **/
bool writeGPXdoc(GPXdoc* doc, char* fileName);


//Module 2

/** Function that returns the length of a Route
 *@pre Route object exists, is not null, and has not been freed
 *@post Route object had been freed
 *@return length of the route in meters
 *@param rt - a pointer to a Route struct
**/
float getRouteLen(const Route *rt);

/** Function that returns the length of a Track
 *@pre Track object exists, is not null, and has not been freed
 *@post Track object had been freed
 *@return length of the track in meters
 *@param tr - a pointer to a Track struct
**/
float getTrackLen(const Track *tr);

/** Function that rounds the length of a track or a route to the nearest 10m
 *@pre Length is not negative
  *@return length rounded to the nearest 10m
 *@param len - length
**/
float round10(float len);

/** Function that returns the number routes with the specified length, using the provided tolerance 
 * to compare route lengths
 *@pre GPXdoc object exists, is not null
 *@post GPXdoc object exists, is not null, has not been modified
 *@return the number of routes with the specified length
 *@param doc - a pointer to a GPXdoc struct
 *@param len - search route length
 *@param delta - the tolerance used for comparing route lengths
**/
int numRoutesWithLength(const GPXdoc* doc, float len, float delta);


/** Function that returns the number tracks with the specified length, using the provided tolerance 
 * to compare track lengths
 *@pre GPXdoc object exists, is not null
 *@post GPXdoc object exists, is not null, has not been modified
 *@return the number of tracks with the specified length
 *@param doc - a pointer to a GPXdoc struct
 *@param len - search track length
 *@param delta - the tolerance used for comparing track lengths
**/
int numTracksWithLength(const GPXdoc* doc, float len, float delta);

/** Function that checks if the current route is a loop
 *@pre Route object exists, is not null
 *@post Route object exists, is not null, has not been modified
 *@return true if the route is a loop, false otherwise
 *@param route - a pointer to a Route struct
 *@param delta - the tolerance used for comparing distances between start and end points
**/
bool isLoopRoute(const Route* route, float delta);


/** Function that checks if the current track is a loop
 *@pre Track object exists, is not null
 *@post Track object exists, is not null, has not been modified
 *@return true if the track is a loop, false otherwise
 *@param track - a pointer to a Track struct
 *@param delta - the tolerance used for comparing distances between start and end points
**/
bool isLoopTrack(const Track *tr, float delta);


/** Function that returns all routes between the specified start and end locations
 *@pre GPXdoc object exists, is not null
 *@post GPXdoc object exists, is not null, has not been modified
 *@return a list of Route structs that connect the given sets of coordinates
 *@param doc - a pointer to a GPXdoc struct
 *@param sourceLat - latitude of the start location
 *@param sourceLong - longitude of the start location
 *@param destLat - latitude of the destination location
 *@param destLong - longitude of the destination location
 *@param delta - the tolerance used for comparing distances between waypoints 
*/
List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta);

/** Function that returns all Tracks between the specified start and end locations
 *@pre GPXdoc object exists, is not null
 *@post GPXdoc object exists, is not null, has not been modified
 *@return a list of Track structs that connect the given sets of coordinates
 *@param doc - a pointer to a GPXdoc struct
 *@param sourceLat - latitude of the start location
 *@param sourceLong - longitude of the start location
 *@param destLat - latitude of the destination location
 *@param destLong - longitude of the destination location
 *@param delta - the tolerance used for comparing distances between waypoints 
*/
List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta);


//Module 3


/** Function to converting a Track into a JSON string
 *@pre Track is not NULL
 *@post Track has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to a Track struct
 **/
char* trackToJSON(const Track *tr);

/** Function to converting a Route into a JSON string
 *@pre Route is not NULL
 *@post Route has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to a Route struct
 **/
char* routeToJSON(const Route *rt);

/** Function to converting a list of Route structs into a JSON string
 *@pre Route list is not NULL
 *@post Route list has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to a List struct
 **/
char* routeListToJSON(const List *list);

/** Function to converting a list of Track structs into a JSON string
 *@pre Track list is not NULL
 *@post Track list has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to a List struct
 **/
char* trackListToJSON(const List *list);

/** Function to converting a GPXdoc into a JSON string
 *@pre GPXdoc is not NULL
 *@post GPXdoc has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to a GPXdoc struct
 **/
char* GPXtoJSON(const GPXdoc* gpx);



// ***************************** Bonus A2 functions ********************************

/** Function to adding an Waypont struct to an existing Route struct
 *@pre arguments are not NULL
 *@post The new waypoint has been added to the Route's waypoint list
 *@return N/A
 *@param rt - a Route struct
 *@param pr - a Waypoint struct
 **/
void addWaypoint(Route *rt, Waypoint *pt);

/** Function to adding an Route struct to an existing GPXdoc struct
 *@pre arguments are not NULL
 *@post The new route has been added to the GPXdoc's routes list
 *@return N/A
 *@param doc - a GPXdoc struct
 *@param rt - a Route struct
 **/
void addRoute(GPXdoc* doc, Route* rt);

/** Function to converting a JSON string into an GPXdoc struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and initialized GPXdoc struct
 *@param str - a pointer to a string
 **/
GPXdoc* JSONtoGPX(const char* gpxString);

/** Function to converting a JSON string into an Waypoint struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and initialized Waypoint struct
 *@param str - a pointer to a string
 **/
Waypoint* JSONtoWaypoint(const char* gpxString);

/** Function to converting a JSON string into an Route struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and initialized Route struct
 *@param str - a pointer to a string
 **/
Route* JSONtoRoute(const char* gpxString);



/* ******************************* List helper functions  - MUST be implemented *************************** */

void deleteGpxData( void* data);
char* gpxDataToString( void* data);
int compareGpxData(const void *first, const void *second);

void deleteWaypoint(void* data);
char* waypointToString( void* data);
int compareWaypoints(const void *first, const void *second);

void deleteRoute(void* data);
char* routeToString(void* data);
int compareRoutes(const void *first, const void *second);

void deleteTrackSegment(void* data);
char* trackSegmentToString(void* data);
int compareTrackSegments(const void *first, const void *second);

void deleteTrack(void* data);
char* trackToString(void* data);
int compareTracks(const void *first, const void *second);


#endif
