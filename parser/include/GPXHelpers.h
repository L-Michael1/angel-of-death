#ifndef GPX_HELPERS_H
#define GPX_HELPERS_H

#include <ctype.h>
#include "GPXParser.h"

/** Function to trim the spaces of a string 
 *@post A valid trimmed string
 *@param str - a pointer to a string
**/
void trimSpaces(char *str);

/** Function to parse a wpt, trkpt, or rtept
 * @pre A valid wpt, trkpt, or rtept node, and a valid initialized Waypoint structure
 * @post The wpt struct will be filled if there is data available
 * @param waypoint_node - a node that points to a wpt, trkpt, or rtept node
 * @param wpt - an initialized Waypoint structure
**/
void parseWpt(xmlNode *waypoint_node, Waypoint *wpt);

/** Function to parse a rte node
 * @pre A valid rte node, and a valid initialized Route structure
 * @post The rte struct will be filled if there is data available
 * @param route_node - a node that points to a rte node
 * @param rte - an initialized Route structure
**/
void parseRte(xmlNode *route_node, Route *rte);

/** Function to parse a trkseg node
 * @pre A valid trkseg node, and a valid initialized list of Waypoint structures
 * @post The list of Waypoint structures will be filled if there is data available
 * @param trackSeg_node - a node that points to a trkseg node
 * @param waypoints - an initialized list of Waypoint structures
**/
void parseTrkSeg(xmlNode *trackSeg_node, List *waypoints);

/** Function to parse a trk node
 * @pre A valid trk node, and a valid initialized Track structure
 * @post The trk structure will be filled if there is data available
 * @param track_node - a node that points to a trkseg node
 * @param trk - an initialized Track structure
**/
void parseTrk(xmlNode *track_node, Track *trk);

/** Function to parse a GPX document
 * @pre A valid xmlNode root node (GPX), and a valid GPX document
 * @post A parsed gpx doc with valid nodes
 * @param root_node - a node that points to the root node of a GPX document
 * @param doc - a valid GPX document 
**/
void parseGPX(xmlNode *root, GPXdoc *doc);

/** Function to convert a GPXdoc into an xmlDoc struct (libxml tree)
 * @pre A valid GPXdoc
 * @param doc - A valid GPX document
 * @return A valid xmlDoc struct
**/
xmlDoc *gpxToTree(GPXdoc *doc);

/** Function to validate a libxml tree
 * @pre A valid xmlDoc struct
 * @param tree - A valid xmlDoc struct that represents a tree
 * @param gpxSchemaFile - A valid GPX schema file
 * @return a boolean value indicating success or failure of the validation for the libxml tree
**/
bool validateXMLTree(xmlDoc *tree, char *gpxSchemaFile);

/** Function to turn a Waypoint to a xmlDoc struct node
 * @pre A valid xmlNodePtr child_node to turn into a Waypoint node, and a pointer to a Waypoint struct
 * @param child_node - A valid xmlNodePtr child_node
 * @param wpt - A valid pointer to a Waypoint struct
**/
void waypointToNode(xmlNodePtr child_node, Waypoint *wpt);

/** Function to check the validity of GPXData
 * @param data - A pointer to a GPXData struct
 * @return a boolean value indicating whether the GPXData is valid or not
**/
bool isValidGPXData(GPXData *data);

/** Function to check the validity of a Waypoint
 * @param wpt - A pointer to a Waypoint struct
 * @return a boolean value indicating whether the Waypoint is valid or not
**/
bool isValidWaypoint(Waypoint *wpt);

/** Function to get the distance between two waypoints using the Haversine formula
 * @param wpt1 - A pointer to a Waypoint struct
 * @param wpt2 - A pointer to a Waypoint struct
 * @return the distance between wpt1 and wpt2 
**/
float distWaypoints(Waypoint *wpt1, Waypoint *wpt2);

/** Dummy delete function **/
void dummyDelete(void *data);

#endif