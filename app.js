'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app = express();
const path = require('path');
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname + '/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// SQL
const mysql = require('mysql2/promise')

// Send HTML at root, do not change
app.get('/', function (req, res) {
  res.sendFile(path.join(__dirname + '/public/index.html'));
});

// Send Style, do not change
app.get('/style.css', function (req, res) {
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname + '/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js', function (req, res) {
  fs.readFile(path.join(__dirname + '/public/index.js'), 'utf8', function (err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, { compact: true, controlFlowFlattening: true });
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function (req, res) {
  if (!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function (err) {
    if (err) {
      return res.status(500).send(err);
    }

    console.log(`uploading: ${uploadFile.name}`);
    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function (req, res) {
  fs.stat('uploads/' + req.params.name, function (err, stat) {
    if (err == null) {
      res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: ' + err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

/**** Set up functions from shared library ****/
const sharedLib = ffi.Library('./libgpxparser', {
  'GPXFiletoJSON': ['string', ['string', 'string']],
  'GPXtoJSON': ['string', ['string']],
  'GPXFileToRouteList': ['string', ['string', 'string']],
  'GPXFileToTrackList': ['string', ['string', 'string']],
  'rteDataToJSON': ['string', ['string', 'string', 'string']],
  'trkDataToJSON': ['string', ['string', 'string', 'string']],
  'renameComponent': ['bool', ['string', 'string', 'string', 'string']],
  'createGPXFromInput': ['bool', ['string', 'string']],
  'addRouteFromInput': ['bool', ['string', 'string', 'string']],
  'addWptToRoute': ['bool', ['string', 'string', 'string', 'string']],
  'pathGetRoutesBetween': ['string', ['string', 'string', 'float', 'float', 'float', 'float', 'float']],
  'pathGetTracksBetween': ['string', ['string', 'string', 'float', 'float', 'float', 'float', 'float']],
  'numComponents': ['int', ['string', 'string', 'string', 'float']],
  'GPXFileToWaypointList': ['string', ['string', 'string', 'string']],
});

// Endpoint for uploads on-page load, load all files into table
app.get('/uploads', (req, res) => {
  fs.readdir('uploads/', (err, files) => {

    if (err) {
      res.status(500).send(err);
    }

    let JSONfiles = [];

    files.forEach(file => {
      let fileToJSON = sharedLib.GPXFiletoJSON(`uploads/${file}`, 'gpx.xsd');

      // Delete invalid files
      if (fileToJSON === '{}') {
        fs.unlink(`uploads/${file}`, (err) => {
          if (err) {
            console.error(err);
          }
        })
        console.log(`Deleted ${file}`);
      }

      let JSONobj = JSON.parse(fileToJSON);
      JSONobj.name = file;
      JSONfiles.push(JSONobj);
    })
    res.send(JSONfiles);
  });
});

// Endpoint for selecting files in the GPX view panel
app.get('/select', (req, res) => {

  let lists = [];
  let routeList = sharedLib.GPXFileToRouteList(`uploads/${req.query.fileName}`, 'gpx.xsd');
  let trackList = sharedLib.GPXFileToTrackList(`uploads/${req.query.fileName}`, 'gpx.xsd');
  let routeListObj = JSON.parse(routeList);
  let trackListObj = JSON.parse(trackList);

  routeListObj.forEach((route, index) => {
    route.component = `Route ${index + 1}`
  })

  if (routeListObj.length !== 0) {
    lists.push(routeListObj);
  }

  trackListObj.forEach((track, index) => {
    track.component = `Track ${index + 1}`
  })

  if (trackListObj.length !== 0) {
    lists.push(trackListObj);
  }
  res.send(lists);
});

// Endpoint for showing other data of a component
app.get('/otherdata', (req, res) => {
  let component = sharedLib.trkDataToJSON(`uploads/${req.query.fileName}`, `gpx.xsd`, `${req.query.componentName}`);

  if (Object.keys(JSON.parse(component)).length === 0) {
    component = sharedLib.rteDataToJSON(`uploads/${req.query.fileName}`, `gpx.xsd`, `${req.query.componentName}`);
  }
  console.log(component);
  component = JSON.parse(component);

  if (component.length === 0) {
    res.send([]);
  } else {
    component[0].component = req.query.componentName;
    console.log(component);
    res.send(component);
  }
});

// Endpoint to rename a component (route/track)
app.get('/rename', async (req, res) => {
  let valid = sharedLib.renameComponent(`uploads/${req.query.fileName}`, `gpx.xsd`, `${req.query.oldName}`, `${req.query.newName}`);
  let lists = [];
  let routeList = sharedLib.GPXFileToRouteList(`uploads/${req.query.fileName}`, 'gpx.xsd');
  let trackList = sharedLib.GPXFileToTrackList(`uploads/${req.query.fileName}`, 'gpx.xsd');
  let routeListObj = JSON.parse(routeList);
  let trackListObj = JSON.parse(trackList);

  routeListObj.forEach((route, index) => {
    route.component = `Route ${index + 1}`
  })

  if (routeListObj.length !== 0) {
    lists.push(routeListObj);
  }

  trackListObj.forEach((track, index) => {
    track.component = `Track ${index + 1}`
  })

  if (trackListObj.length !== 0) {
    lists.push(trackListObj);
  }

  // Update route in db
  if (connection) {
    await connection.execute(`UPDATE ROUTE SET route_name = '${req.query.newName}' WHERE ROUTE.route_name = '${req.query.oldName}'`);
  }

  if (valid) {
    res.send({
      oldName: req.query.oldName,
      newName: req.query.newName,
      components: lists
    });
  } else {
    res.send('');
  }
});

// Endpoint to CreateGPX
app.get('/create', (req, res) => {
  let JSONstr = {};
  JSONstr.version = parseFloat(req.query.version);
  JSONstr.creator = req.query.creator;

  let valid = sharedLib.createGPXFromInput(`uploads/${req.query.fileName}`, `${JSON.stringify(JSONstr)}`);

  if (valid) {
    res.send({
      fileName: `${req.query.fileName}.gpx`
    });
  } else {
    res.send({
      fileName: `${req.query.fileName}.gpx`
    });
  }
});

app.get('/addRte', (req, res) => {

  let wptObjs = req.query.waypoints;
  let jsonRteName = { name: req.query.routeName };
  let lists = [];

  // Add route whether empty or not
  sharedLib.addRouteFromInput(`uploads/${req.query.fileName}`, 'gpx.xsd', JSON.stringify(jsonRteName));

  // Append waypoints to route if there exists waypoints
  if (wptObjs !== undefined) {
    wptObjs.forEach((wpt) => {
      let JSONwpt = JSON.stringify({
        "lat": parseFloat(wpt.lat),
        "lon": parseFloat(wpt.lon)
      });
      sharedLib.addWptToRoute(`uploads/${req.query.fileName}`, 'gpx.xsd', req.query.routeName, JSONwpt);
    });
  }

  let routeList = sharedLib.GPXFileToRouteList(`uploads/${req.query.fileName}`, 'gpx.xsd');
  let trackList = sharedLib.GPXFileToTrackList(`uploads/${req.query.fileName}`, 'gpx.xsd');
  let routeListObj = JSON.parse(routeList);
  let trackListObj = JSON.parse(trackList);

  routeListObj.forEach((route, index) => {
    route.component = `Route ${index + 1}`
  })

  if (routeListObj.length !== 0) {
    lists.push(routeListObj);
  }

  trackListObj.forEach((track, index) => {
    track.component = `Track ${index + 1}`
  })

  if (trackListObj.length !== 0) {
    lists.push(trackListObj);
  }

  fs.readdir('uploads/', (err, files) => {

    if (err) {
      res.status(500).send(err);
    }

    let JSONfiles = [];

    files.forEach(file => {
      let fileToJSON = sharedLib.GPXFiletoJSON(`uploads/${file}`, 'gpx.xsd');

      // Delete invalid files
      if (fileToJSON === '{}') {
        fs.unlink(`uploads/${file}`, (err) => {
          if (err) {
            console.error(err);
          }
        })
        console.log(`Deleted ${file}`);
      }

      let JSONobj = JSON.parse(fileToJSON);
      JSONobj.name = file;
      JSONfiles.push(JSONobj);
    })

    if (connection) {
      routeListObj.forEach(async (route) => {
        if (route.name === req.query.routeName) {

          // Query most recent row's gpx_id
          const [data, fields] = await connection.execute(`SELECT * FROM FILE WHERE file_name='${req.query.fileName}'`);

          if (data.length !== 0) {
            // Add route to db
            await connection.execute(`INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('${route.name}', '${route.len}', '${data[0].gpx_id}')`);

            // Query most recent route's data
            const [routeData, routeFields] = await connection.execute(`SELECT * FROM ROUTE WHERE route_name='${route.name}'`);

            // Get route's waypoints
            let wptList = JSON.parse(sharedLib.GPXFileToWaypointList(`uploads/${req.query.fileName}`, 'gpx.xsd', route.name));
            let ptIdx = 0;

            // Insert point data into db
            await wptList.forEach(async (wpt) => {
              await connection.execute(`INSERT INTO POINT (point_index, latitude, longitude, point_name, route_len, route_id) VALUES ('${ptIdx++}', '${wpt.latitude}', '${wpt.longitude}', '${wpt.name}', '${route.len}', '${routeData[0].route_id}')`);
            })
          }
        }
      })
    }

    res.send({
      routeName: req.query.routeName,
      wpts: req.query.waypoints,
      components: lists,
      files: JSONfiles
    })
  });
});

app.get('/paths', (req, res) => {
  let paths = [];
  let parsedPaths = [];
  let files = req.query.files;
  console.log(req.query.files);

  files.forEach(file => {
    paths.push(sharedLib.pathGetRoutesBetween(
      `uploads/${file}`,
      'gpx.xsd',
      parseFloat(req.query.startLat),
      parseFloat(req.query.startLon),
      parseFloat(req.query.endLat),
      parseFloat(req.query.endLon),
      req.query.delta));

    paths.push(sharedLib.pathGetTracksBetween(
      `uploads/${file}`,
      'gpx.xsd',
      parseFloat(req.query.startLat),
      parseFloat(req.query.startLon),
      parseFloat(req.query.endLat),
      parseFloat(req.query.endLon),
      req.query.delta));
  });

  paths.forEach(path => {
    parsedPaths.push(JSON.parse(path));
  })

  console.log(parsedPaths);

  res.send(parsedPaths);
})

app.get('/len', (req, res) => {
  let numComponents = 0;
  let files = req.query.files;

  files.forEach(file => {
    console.log(file);
    numComponents += sharedLib.numComponents(`uploads/${file}`, 'gpx.xsd', req.query.component, parseFloat(req.query.length));
  })

  res.send(JSON.stringify(numComponents));
})

let connection;

app.get('/loginDb', async (req, res) => {

  let dbConf = {
    host: 'dursley.socs.uoguelph.ca',
    user: req.query.username,
    password: req.query.password,
    database: req.query.dbName,
  };

  let routeNames = [];

  try {
    connection = await mysql.createConnection(dbConf);

    connection.execute('CREATE TABLE IF NOT EXISTS FILE (gpx_id INT auto_increment,  file_name VARCHAR(60) NOT NULL, ver DECIMAL(2,1), creator VARCHAR(256) NOT NULL, PRIMARY KEY(gpx_id))');

    // route_name, route_len, gpx_id
    connection.execute('CREATE TABLE IF NOT EXISTS ROUTE (route_id INT auto_increment,  route_name VARCHAR(256), route_len FLOAT(15,7) NOT NULL, gpx_id INT NOT NULL, PRIMARY KEY(route_id), FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE)');

    // point_index, latitude, longitude, point name, route len, route id,
    connection.execute('CREATE TABLE IF NOT EXISTS POINT (point_id INT auto_increment,  point_index INT NOT NULL, latitude DECIMAL(11,7) NOT NULL, longitude DECIMAL(11,7) NOT NULL, point_name VARCHAR(256), route_len FLOAT(15,7) NOT NULL, route_id INT NOT NULL, PRIMARY KEY(point_id), FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE)');

  } catch (err) {
    return res.status(400).send(err);
  }

  let [routeData, routeColData] = await connection.execute(`SELECT * FROM ROUTE`);
  await routeData.forEach((route) => {
    routeNames.push(route.route_name);
  })

  let numFiles = await connection.execute(`SELECT COUNT(*) FROM FILE`);
  let numRoutes = await connection.execute(`SELECT COUNT(*) FROM ROUTE`);
  let numPts = await connection.execute(`SELECT COUNT(*) FROM POINT`);

  await res.send(JSON.stringify(
    {
      files: numFiles[0][0]["COUNT(*)"],
      routes: numRoutes[0][0]["COUNT(*)"],
      points: numPts[0][0]["COUNT(*)"],
      routeNames: routeNames,
    }
  ))
})

app.get('/storeFiles', async (req, res) => {
  let files = req.query.files;
  let routeNames = [];
  // Query from FILE table
  const [fileRows, fields] = await connection.execute('SELECT * FROM `FILE`')

  await files.forEach(async (file) => {
    let uniqueFile = true;

    if (fileRows.length !== 0) {
      fileRows.forEach((dbFile) => {
        if (dbFile.file_name === file) {
          uniqueFile = false;
        }
      })
    }

    // If a unique file, add to FILE table
    if (uniqueFile) {
      let fileToJSON = JSON.parse(sharedLib.GPXFiletoJSON(`uploads/${file}`, 'gpx.xsd'));
      await connection.execute(`INSERT INTO FILE (file_name, ver, creator) VALUES ('${file}', '${fileToJSON.version}', '${fileToJSON.creator}')`);

      // Query most recent row's gpx_id
      const [data, fields] = await connection.execute(`SELECT * FROM FILE WHERE file_name='${file}'`);

      // Insert route data into db
      let routeList = JSON.parse(sharedLib.GPXFileToRouteList(`uploads/${file}`, 'gpx.xsd'));

      routeList.forEach(async (route) => {
        await connection.execute(`INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('${route.name}', '${route.len}', '${data[0].gpx_id}')`);

        // Query most recent route's data
        const [routeData, routeFields] = await connection.execute(`SELECT * FROM ROUTE WHERE route_name='${route.name}'`);

        // Get route's waypoints
        let wptList = JSON.parse(sharedLib.GPXFileToWaypointList(`uploads/${file}`, 'gpx.xsd', route.name));
        let ptIdx = 0;

        // Insert point data into db
        await wptList.forEach(async (wpt) => {
          await connection.execute(`INSERT INTO POINT (point_index, latitude, longitude, point_name, route_len, route_id) VALUES ('${ptIdx++}', '${wpt.latitude}', '${wpt.longitude}', '${wpt.name}', '${route.len}', '${routeData[0].route_id}')`);
        })
      })
    }
  })

  setTimeout(async () => {

    let [routeData, routeColData] = await connection.execute(`SELECT * FROM ROUTE`);
    await routeData.forEach((route) => {
      routeNames.push(route.route_name);
    })

    let numFiles = await connection.execute(`SELECT COUNT(*) FROM FILE`);
    let numRoutes = await connection.execute(`SELECT COUNT(*) FROM ROUTE`);
    let numPts = await connection.execute(`SELECT COUNT(*) FROM POINT`);

    await res.send(JSON.stringify(
      {
        files: numFiles[0][0]["COUNT(*)"],
        routes: numRoutes[0][0]["COUNT(*)"],
        points: numPts[0][0]["COUNT(*)"],
        routeNames: routeNames,
      }
    ))
  }, 150)
})

app.get('/clearData', async (req, res) => {
  await connection.execute(`DELETE FROM FILE WHERE ver='1.1'`);
  res.send(JSON.stringify({ isSuccess: true }));
})

app.get('/dbStatus', async (req, res) => {

  let numFiles = await connection.execute(`SELECT COUNT(*) FROM FILE`);
  let numRoutes = await connection.execute(`SELECT COUNT(*) FROM ROUTE`);
  let numPts = await connection.execute(`SELECT COUNT(*) FROM POINT`);

  res.send(JSON.stringify(
    {
      files: numFiles[0][0]["COUNT(*)"],
      routes: numRoutes[0][0]["COUNT(*)"],
      points: numPts[0][0]["COUNT(*)"],
    }
  ))
})

app.get('/queryAllRtes', async (req, res) => {
  let routes = [];

  if (connection) {
    const [rowData, colData] = await connection.execute(`SELECT * FROM ROUTE ORDER BY ${req.query.order}`)
    rowData.forEach((route) => {
      let rteObj = {};
      rteObj.route_name = route.route_name;
      rteObj.route_len = route.route_len;
      routes.push(rteObj);
    })
  }

  res.send(JSON.stringify(routes));
})

app.get('/queryFileAllRtes', async (req, res) => {
  let routes = [];
  let fileName = req.query.fileName;
  let order = req.query.order;

  if (connection) {
    const [fileData, fileColData] = await connection.execute(`SELECT * FROM FILE WHERE file_name='${fileName}'`);

    if (fileData.length !== 0) {
      console.log(fileData[0]);

      const [rowData, colData] = await connection.execute(`SELECT * FROM ROUTE WHERE gpx_id='${fileData[0].gpx_id}' ORDER BY ${order}`);
      rowData.forEach((route) => {
        let rteObj = {};
        rteObj.route_name = route.route_name;
        rteObj.route_len = route.route_len;
        routes.push(rteObj);
      })
    }
  }
  res.send(JSON.stringify(routes));
})

app.get('/getPtsFromRte', async (req, res) => {
  let pts = [];
  const [rteData, rteColData] = await connection.execute(`SELECT * FROM ROUTE WHERE route_name='${req.query.rteName}'`);

  if (rteData.length !== 0) {
    const [ptData, ptColData] = await connection.execute(`SELECT * FROM POINT WHERE route_id='${rteData[0].route_id}' ORDER BY point_index`);
    ptData.forEach((pt) => {
      let ptObj =
      {
        point_id: pt.point_id,
        point_index: pt.point_index,
        point_name: pt.point_name,
        point_lat: pt.latitude,
        point_lon: pt.longitude,
        point_routeLen: pt.route_len,
        point_routeName: rteData[0].route_name,
      };
      pts.push(ptObj);
    })
  }
  res.send(pts);
})

app.get('/queryFileAllPts', async (req, res) => {
  let points = [];
  let fileName = req.query.fileName;
  let order = req.query.order;

  if (connection) {
    const [fileData, fileColData] = await connection.execute(`SELECT * FROM FILE WHERE file_name='${fileName}'`);

    if (fileData.length !== 0) {

      const [rowData, colData] = await connection.execute(`SELECT * FROM ROUTE WHERE gpx_id='${fileData[0].gpx_id}' ORDER BY ${order}`);

      if (rowData.length !== 0) {

        rowData.forEach(async (route, index) => {
          let [tempPtData, tempPtColData] = await connection.execute(`SELECT * FROM POINT WHERE route_id='${rowData[index].route_id}'`);
          // console.log(tempPtData);
          await tempPtData.forEach((pt) => {
            // console.log(pt);
            let ptObj =
            {
              point_id: pt.point_id,
              point_index: pt.point_index,
              point_name: pt.point_name,
              point_lat: pt.latitude,
              point_lon: pt.longitude,
              point_routeLen: pt.route_len,
              point_routeName: rowData[index].route_name,
            };
            console.log(ptObj);
            points.push(ptObj);
            console.log(points);
          })
        })
      }
    }
  }
  setTimeout(() => {
    res.send(points);
  }, 100)
})

app.get('/logoutDb', (req, res) => {
  if (connection && connection.end) connection.end();
  res.send(JSON.stringify({ isSuccess: true }));
})

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);