let selectedFile = '';
let routeFile = '';
let displayRouteFile = ''
let displayPointFile = '';
let selectedComp = '';
let pathFile = '';
let pathComp = '';
let routeSort = 'route_name';
let pointSort = 'route_name';
let numUploads = 0;
let dbRouteNames = [];

function updateGPXView(component, isLoop) {
    $('#gpxView').append(`
        <tr>
            <th scope = 'col'>${component.component}</th >
            <th scope = 'col'>${component.name}</th>
            <th scope = 'col'>${component.numPoints}</th>
            <th scope = 'col'>${component.len}m</th>
            <th scope = 'col'>${isLoop}</th>
        </tr >`);
}

function updateBtnData(component) {
    $('#otherDataBody').append(`<div class = "row pt-2" align='center'><div class = "col-sm"><button class = 'btn btn-info'>${component.name}</button></div></div>`);
    $('#renameBody').append(`<div class = "row pt-2" align='center'><div class = "col-sm"><button class = 'btn btn-warning'>${component.name}</button></div></div>`);
}

updateFileLog = (file) => {
    $('#fileLog').append(`
    <tr>
        <th scope = 'col'><a href='../uploads/${file.name}' download'</a>${file.name}</th>
        <th scope = 'col'>${file.version}</th>
        <th scope = 'col'>${file.creator}</th>
        <th scope = 'col'>${file.numWaypoints}</th>
        <th scope = 'col'>${file.numRoutes}</th>
        <th scope = 'col'>${file.numTracks}</th>
    </tr>`);
}

// Put all onload AJAX calls here, and event listeners
$(document).ready(function () {
    let fileList = [];
    window.scrollTo(0, 0);
    $.ajax({
        dataType: 'json',
        url: '/logoutDb',
        success: (data) => {
            $('#login').show();
        },
        error: (err) => {
            // console.error(err);
        }
    })

    // AJAX call to load all files into file log
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Files will be in JSON format
        url: '/uploads',        //The uploads endpoint
        success: function (files) {

            // If files do not exist
            if (files.length === 0 || ((files.length === 1) && (files[0].name === '.DS_Store'))) {
                $('#fileLogSec').append(`<h2 style = 'text-align:center;padding-top:30px'>No valid files found...upload a file!</h2>`);
                $('#addBody').append(`<p>No valid files found...upload a file!</p>`)
                $('#allRteFileBody').append(`<p>No valid files found...upload a file!</p>`)
                $('#status').append(`<p class = 'h6'>No files to load...</p>`)
            }

            // For each file, add <th>
            files.forEach(file => {
                if (Object.keys(file).length === 6) {
                    $('#status').append(`<p class = 'h6'>Loaded: ${file.name}</p>`)
                    updateFileLog(file);
                    $('#addBody').append(`<div class = "row pt-2" align='center'><div class = "col-sm"><button class = 'btn addButton'>${file.name}</button></div></div>`);
                    $('#allRteFileBody').append(`<div class = "row pt-2" align='center'><div class = "col-sm"><button class = 'btn allRteFileBtn'>${file.name}</button></div></div>`);
                    $('#allPtFileBody').append(`<div class = "row pt-2" align='center'><div class = "col-sm"><button class = 'btn allPtFileBtn'>${file.name}</button></div></div>`);
                    fileList.push(file.name);
                } else {
                    console.log(`Invalid GPX file: ${file.name}`)
                    alert(`Invalid GPX file: ${file.name}`)
                    location.reload();
                }
            });
            console.log('Loaded files successfully');
        },
        fail: function (error) {
            console.log(error);
            $('#status').append(`<p class = 'h6'>Failed to load files</p>`);
        }
    });

    // AJAX call to load all files into 'select file menu' for GPX view
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/uploads',
        success: function (files) {
            console.log(files.length);
            if (files.length !== 0) {
                files.forEach(file => {
                    console.log(file);
                    if (file !== '.DS_Store' && Object.keys(file).length === 6) {
                        $('#selectMenu').append(`<option value='${file.name}'>${file.name}</option>`);
                    }
                });
                console.log('Loaded files for GPX view panel');
            }
        },
        fail: function (error) {
            console.log(error);
            $('#status').append(`<p class = 'h6'>Failed to load files</p>`);
        },
    });

    $('#gpxSecView').append(`<h2 style='text-align:center;padding-top:60px'>No components found...</h2>`)

    $('select').change(() => {

        $('#gpxView').empty();

        let file = '';
        $('select option:selected').each(function () {
            file += $(this).text();
        });
        selectedFile = file;

        console.log(`Selected file: ${selectedFile}`);

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/select',
            data: {
                fileName: file,
            },
            success: (data) => {

                data.forEach(obj => {
                    obj.forEach(component => {
                        let isLoop = (component.loop ? 'TRUE' : 'FALSE');
                        updateGPXView(component, isLoop);
                    })
                })

                $('#otherDataBody').empty();
                $('#renameBody').empty();

                // If no data, show user "No components found"
                if (data.length === 0) {
                    $('#gpxSecView').empty();
                    $('#gpxSecView').append(`<h2 style='text-align:center;padding-top:60px'>No components found...</h2>`)
                    $('#otherDataBody').append(`<div class='row pt-2'>No components found...select a file!</div>`);
                    $('#renameBody').append(`<div class='row pt-2'>No components to rename...select a file!</div>`)
                } else {
                    $('#gpxSecView').empty();
                }

                let index = 0;
                data.forEach(obj => {
                    obj.forEach(component => {
                        updateBtnData(component);
                    })
                })
                console.log('Loaded GPX view panel data successfully');
            },
            fail: (error) => {
                console.log(error);
                $('#status').append(`<p class = 'h6'>Failed to load GPX info</p> `);
            },
        });
    });

    // Add event handlers for appended "other data" buttons
    $(document).on('click', '.btn-info', (e) => {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/otherdata',
            data: {
                fileName: selectedFile,
                componentName: e.target.innerHTML,
            },
            success: (data) => {
                if (data.length !== 0) {
                    console.log(`Showing other data for \"${e.target.innerHTML}\"`);
                    let componentStr = `Other data for \"${data[0].component}\"\n`;
                    data.forEach(component => {
                        componentStr += `Name: ${component.name}\n`;
                        componentStr += `Value: ${component.value}\n`;
                        componentStr += `\n`;
                    });
                    alert(componentStr);
                } else {
                    alert('No other data found...');
                }
            },
            fail: (error) => {
                console.log(error);
                $('#status').append(`<p class='h6'>Failed to load other data for \"${e.target.innerHTML}\"</p>`);
            }
        });
    });

    // Add event handlers for appended "rename" buttons
    $(document).on('click', '.btn-warning', (e) => {
        $('#renameForm').show();
        $('#renameBox').val('');
        $('#renameSelectedText').text(`Selected component: ${e.target.innerHTML}`)
        selectedComp = e.target.innerHTML;
    });

    // On submit to rename
    $('#renameForm').submit((e) => {
        console.log($('#renameBox').val());
        if ($('#renameBox').val().trim() !== '') {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/rename',
                data: {
                    fileName: selectedFile,
                    oldName: selectedComp,
                    newName: $('#renameBox').val().trim(),
                },
                success: (data) => {
                    if (data != '') {
                        alert(`Successfully renamed \"${data.oldName}\" to \"${data.newName}\"`);
                        console.log(`Renamed \"${data.oldName}\" to \"${data.newName}\"`);

                        $('#gpxView').empty();
                        $('#otherDataBody').empty();
                        $('#renameBody').empty();

                        data.components.forEach(obj => {
                            obj.forEach(component => {
                                let isLoop = (component.loop ? 'TRUE' : 'FALSE');
                                updateGPXView(component, isLoop);
                                updateBtnData(component);
                            })
                        })
                    } else {
                        alert(`Failed to rename \"${data.oldName}\" to \"${data.newName}\"`);
                        console.log(`Failed to rename \"${data.oldName}\" to \"${data.newName}\"`);
                    }
                },
                fail: (err) => {
                    console.log(err);
                    console.log(`Failed to rename \"${data.oldName}\" to \"${data.newName}\"`);
                },
            });
            $('#renameModal').modal('toggle');
        } else {
            alert('Component name cannot be whitespaces or empty!');
        }
        e.preventDefault();
    });

    $('#renameModalBtn').on('click', () => {
        $('#renameForm').hide();
    })

    $('#createForm').submit((e) => {
        let versionVal = parseFloat($('#versionIn').val());
        let creatorVal = $('#creatorIn').val().trim();
        let fileVal = $('#fileIn').val().replace(/\s+/g, '').trim();

        if (creatorVal === '' || fileVal === '') {
            alert('Fields cannot be whitespaces or empty..');
            e.preventDefault();
        } else if (fileVal.includes('.') === true) {
            alert('Do not add file extensions or the \".\" character to your filename..');
            e.preventDefault();
        } else {
            e.preventDefault();
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/create',
                data: {
                    fileName: fileVal,
                    creator: creatorVal,
                    version: versionVal,
                },
                success: (data) => {
                    alert(`Successfully created GPX: ${data.fileName}`);
                    location.reload();
                },
                fail: (err) => {
                    alert(`Failed to create GPX: ${data.fileName}`)
                    console.log(err);
                    console.log(`Failed to create GPX`);
                }
            })
        }
    });

    $(document).on('click', '.addButton', (e) => {
        $('#wptDiv').empty();
        $('#routeForm').show();
        $('#addWptBtn').show();
        $('#resetRouteBtn').show();
        $('#routeBox').val('');
        $('#routeSelectedFile').text(`Selected file: ${e.target.innerHTML}`)
        routeFile = e.target.innerHTML;
        e.preventDefault();
    })

    $('#addRouteBtn').on('click', () => {
        $('#routeForm').hide();
        $('#addWptBtn').hide();
        $('#resetRouteBtn').hide();
        $('#wptDiv').empty();
    })

    // Append input fields for new route waypoints
    $('#addWptBtn').on('click', () => {
        $('#wptDiv').append(`
        <div class='pl-2 ml-4 pt-2 row' align='center'>
            <input type='text' class='col-md-4 wptData' placeholder='Latitude'>
                <div class='pl-2'>
                    <input type='text' class='col-md-8 wptData' placeholder='Longitude'>
                </div>
        </div>`)
    })

    $('#resetRouteBtn').on('click', () => {
        console.log('Resetting add route form');
        $('#wptDiv').empty();
        $('#routeBox').val('');
    })

    $('#routeForm').submit((e) => {
        let valid = true;
        let wpts = [];
        let routeName = $('#routeBox').val();
        let wptData = $('input.wptData');

        // Do not allow user to enter 0 waypoints
        if (wptData.length === 0) {
            alert(`Add at least one waypoint!`);
            valid = false;
        }

        // Iterate waypoint data
        for (let i = 0; i < wptData.length; i += 2) {
            let wpt = {};
            for (let j = i; j < i + 2; j++) {

                // Error check
                if (Number.isNaN(parseFloat($(wptData[j]).val()))) {
                    alert(`Longitudes and latitudes must be number!`);
                    valid = false;
                    i = wptData.length;
                    j = i + 2;
                }

                // Store latitude and longitude into one JSON string
                if (j % 2 == 0) {
                    wpt.lat = parseFloat($(wptData[j]).val());

                    if (wpt.lat < -90 || wpt.lat > 90) {
                        alert(`Latitudes have range of '-90.0 to 90.0'`);
                        valid = false;
                        i = wptData.length;
                        j = i + 2;
                    }
                } else {
                    wpt.lon = parseFloat($(wptData[j]).val());
                    if (wpt.lon < -180 || wpt.lon > 180) {
                        alert(`Longitudes have a range of '-180.0 to 180.0'`);
                        valid = false;
                        i = wptData.length;
                        j = i + 2;
                    }
                }
            }
            wpts.push(wpt);
        }

        console.log(valid);
        // If all user inputs are valid, continue with AJAX call
        if (valid) {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/addRte',
                data: {
                    routeName: routeName,
                    fileName: routeFile,
                    waypoints: wpts,
                },
                success: (data) => {
                    alert(`Successfully added "${data.routeName}" to "${routeFile}"!`);

                    $('#gpxSecView').empty();
                    $('#gpxView').empty();
                    $('#otherDataBody').empty();
                    $('#renameBody').empty();

                    data.components.forEach(obj => {
                        console.log(obj);
                        obj.forEach(component => {
                            let isLoop = (component.loop ? 'TRUE' : 'FALSE');
                            updateGPXView(component, isLoop);
                            updateBtnData(component);
                        })
                    })

                    $('#fileLog').empty();

                    data.files.forEach(file => {
                        if (Object.keys(file).length === 6) {
                            updateFileLog(file);
                        }
                    });

                },
                fail: (err) => {
                    alert(`Failed to add route...`)
                    console.log(`Failed to add route...`);
                    console.log(err);
                }
            });
            $('#addModal').modal('toggle');
        } else {
            wpts = [];
        }
        e.preventDefault();
    });

    // Reset user's find path info
    $('#findPathBtn').on('click', () => {
        $('.pathData').each(function (i, data) {
            $(data).val('');
        });
        pathFile = '';
        pathComp = '';
    })

    $(document).on('click', '.pathFile', (e) => {
        $('#compMenu').show();
        $('.pathData').each(function (i, data) {
            $(data).val('');
        });
        pathFile = e.target.innerHTML;
        pathComp = '';
    });

    $(document).on('click', '.pathComponent', (e) => {
        $('.pathData').each(function (i, data) {
            $(data).val('');
        });
        $('#pathForm').show();
        pathComp = e.target.innerText;
    })

    $('#pathForm').submit((e) => {
        let startLat = parseFloat($('#startLat').val());
        let endLat = parseFloat($('#endLat').val());
        let startLon = parseFloat($('#startLon').val());
        let endLon = parseFloat($('#endLon').val());
        let delta = parseFloat($('#delta').val());
        let valid = true;

        // Check every input if number
        $('.pathData').each(function (i, data) {
            if (Number.isNaN(parseFloat($(this).val()))) {
                alert(`All fields must be a number!`);
                valid = false;
                return false;
            }
        });

        // Check latitudes + longitudes if within range
        if (startLat < -90 || startLat > 90 || endLat < -90 || endLat > 90) {
            alert(`Latitudes have range of '-90.0 to 90.0'`);
            valid = false;
        }

        if (startLon < -180 || startLon > 180 || endLon < -180 || endLon > 180) {
            alert(`Longitudes have a range of '-180.0 to 180.0'`);
            valid = false;
        }

        if (valid) {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/paths',
                data: {
                    startLat: startLat,
                    startLon: startLon,
                    endLat: endLat,
                    endLon: endLon,
                    delta: delta,
                    pathType: pathComp,
                    files: fileList,
                },
                success: (data) => {
                    let empty = true;
                    $('#pathSec').empty();
                    $('#pathsBody').empty();
                    data.forEach(components => {
                        if (components.length !== 0) {
                            empty = false;
                        }
                        components.forEach(component => {
                            console.log(`Found path: ${component.name}`)
                            let isLoop = (component.loop ? 'TRUE' : 'FALSE');
                            $('#pathsBody').append(`
                                <tr>
                                    <th scope = 'col'>${component.name}</th>
                                    <th scope = 'col'>${component.numPoints}</th>
                                    <th scope = 'col'>${component.len}m</th>
                                    <th scope = 'col'>${isLoop}</th>
                                </tr >`);
                        });
                    });

                    if (empty) {
                        $('#pathSec').append(`<h2 style='text-align:center;padding-top:30px'>No paths found...</h2>`);
                        console.log('No paths between...')
                        alert('No paths...');
                    } else {
                        console.log('Found paths!');
                        alert('Found paths!');
                    }
                    $('#findModal').modal('toggle');
                    e.preventDefault();
                },
                fail: (err) => {
                    console.log(err);
                },
            });
            e.preventDefault();
        } else {
            e.preventDefault();
        }
    });

    $('#rteWLen').on('click', () => {
        let valid = true;
        let rteLen = $('#routeLen').val();
        if (isNaN(rteLen) || rteLen === '') {
            alert('Length must be a number!');
            valid = false;
        }

        if (valid) {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/len',
                data: {
                    length: parseFloat(rteLen),
                    files: fileList,
                    component: 'route'
                },
                success: (data) => {
                    alert(`Number of routes with length \"${rteLen}\": ${data}`)
                },
                fail: (err) => {
                    console.log(err);
                },
            })
        }
    })

    $('#trkWLen').on('click', () => {
        let valid = true;
        let trackLen = $('#trackLen').val();
        if (isNaN(trackLen) || trackLen === '') {
            alert('Length must be a number!');
            valid = false;
        }

        if (valid) {
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/len',
                data: {
                    length: parseFloat(trackLen),
                    files: fileList,
                    component: 'track'
                },
                success: (data) => {
                    alert(`Number of tracks with length \"${trackLen}\": ${data}`)
                },
                fail: (err) => {
                    console.log(err);
                },
            })
        }
    })

    $('#loginForm').submit((e) => {
        dbRouteNames = [];
        $('#pointBody').empty();
        let user = $('#userIn').val();
        let password = $('#passwordIn').val();
        let databaseName = $('#databaseIn').val();

        if (user === '' || password === '' || databaseName === '') {
            alert('Fields cannot be whitespaces or empty..');
            e.preventDefault();
        } else {
            e.preventDefault();
            $.ajax({
                type: 'get',
                dataType: 'json',
                url: '/loginDb',
                data: {
                    username: user,
                    password: password,
                    dbName: databaseName,
                },
                success: (data) => {
                    console.log(data);

                    data.routeNames.forEach((rte) => {
                        dbRouteNames.push(rte);
                        $('#pointBody').append(`<div class = "row pt-2" align='center'><div class = "col-sm"><button class = 'btn rteBtn'>${rte}</button></div></div>`);
                    })

                    alert(`Successfully logged in!\nDatabase has ${data.files} files, ${data.routes} routes, and ${data.points} points`);
                    $('#loginModal').modal('toggle');
                    $('#login').hide();
                    $('#logout').show();
                    $('#clearDataBtn').show();
                    $('#displayDbBtn').show();
                    $('#queryDiv').show();
                    $('#allRtesBtn').show();
                    $('#allRtesFileBtn').show();
                    $('#allPtsRteBtn').show();
                    $('#allPtsFileBtn').show();
                    window.scrollTo(5000, 5000)
                    if (fileList.length !== 0) {
                        $('#storeFileBtn').show();
                    }
                },
                error: (err) => {
                    alert(`Access denied for \"${user}\"\n`);
                }
            })
        }
    });

    $('#storeFileBtn').on('click', () => {
        dbRouteNames = [];
        $('#pointBody').empty();
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/storeFiles',
            data: {
                files: fileList,
            },
            success: (data) => {

                data.routeNames.forEach((rte) => {
                    dbRouteNames.push(rte);
                    $('#pointBody').append(`<div class = "row pt-2" align='center'><div class = "col-sm"><button class = 'btn rteBtn'>${rte}</button></div></div>`);
                })

                alert(`Database has ${data.files} files, ${data.routes} routes, and ${data.points} points`);
            },
            fail: (err) => {
                console.error(err);
            }
        })
    });

    $('#clearDataBtn').on('click', () => {
        $('#pointBody').empty();
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/clearData',
            success: (data) => {
                alert(`Database has 0 files, 0 routes, and 0 points`);
            },
            fail: (err) => {
                console.error(err);
            }
        })
    });

    $('#displayDbBtn').on('click', () => {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/dbStatus',
            success: (data) => {
                alert(`Database has ${data.files} files, ${data.routes} routes, and ${data.points} points`);
            },
            fail: (err) => {
                console.error(err);
            }
        })
    })

    $('#rteSortByLenBtn').on('click', () => {
        $('#selectedFile').empty();
        $('#rteQueryBody').empty();
        $('#ptQueryBody').empty();
        $('#rteTable').empty();
        $('#ptTable').empty();
        $.ajax({
            dataType: 'json',
            url: '/queryAllRtes',
            data: {
                order: 'route_len'
            },
            success: (data) => {

                if (data.length === 0) {
                    $('#rteTable').append(`<h2 class='mt-3' align='center'>No routes...</h2>`)
                }

                data.forEach((route) => {
                    $('#rteQueryBody').append(`
                    <tr>
                        <th scope = 'col'>${route.route_name}</th>
                        <th scope = 'col'>${route.route_len}</th>
                    </tr>`);
                })
                $('#allRtes').modal('toggle');
                $('#rteTableQuery').show();
                $('#ptTableQuery').hide();
                window.scrollTo(5000, 5000);
            },
            error: (err) => {
                console.error(err);
            }
        })
    })

    $('#rteSortByNameBtn').on('click', () => {
        $('#selectedFile').empty();
        $('#rteQueryBody').empty();
        $('#ptQueryBody').empty();
        $('#rteTable').empty();
        $('#ptTable').empty();
        $.ajax({
            dataType: 'json',
            url: '/queryAllRtes',
            data: {
                order: 'route_name'
            },
            success: (data) => {

                if (data.length === 0) {
                    $('#rteTable').append(`<h2 class='mt-3' align='center'>No routes...</h2>`)
                }

                data.forEach((route) => {
                    $('#rteQueryBody').append(`
                    <tr>
                        <th scope = 'col'>${route.route_name}</th>
                        <th scope = 'col'>${route.route_len}</th>
                    </tr>`);
                })
                $('#allRtes').modal('toggle');
                $('#rteTableQuery').show();
                $('#ptTableQuery').hide();
                window.scrollTo(5000, 5000);
            },
            error: (err) => {
                console.error(err);
            }
        })
    })

    $(document).on('click', '.allRteFileBtn', (e) => {
        $('#selectedFile').empty();
        displayRouteFile = e.target.innerHTML;
        $('#selectedFile').append(`<h3 class='mb-1'><b>Selected file: ${displayRouteFile}</b></h3>`)
        $.ajax({
            dataType: 'json',
            url: '/queryFileAllRtes',
            data: {
                fileName: displayRouteFile,
                order: routeSort,
            },
            success: (data) => {
                $('#rteQueryBody').empty();
                $('#rteTable').empty();

                if (data.length === 0) {
                    $('#rteTable').append(`<h2 class='mt-3' align='center'>No routes...</h2>`)
                }

                data.forEach((route) => {
                    $('#rteQueryBody').append(`
                    <tr>
                        <th scope = 'col'>${route.route_name}</th>
                        <th scope = 'col'>${route.route_len}</th>
                    </tr>`);
                })
                $('#allRtesFile').modal('toggle');
                $('#displayRteSortDiv').empty();
                $('#ptTableQuery').hide();
                $('#rteTableQuery').show();
                window.scrollTo(5000, 5000);
            },
            error: (err) => {
                console.error(err);
            }
        })
    })


    $('#routeLenSortBtn').on('click', () => {
        routeSort = 'route_len'
        $('#displayRteSortDiv').empty().append(`<h4>Sorting by: route length</h4>`);
    })

    $('#routeNameSortBtn').on('click', () => {
        routeSort = 'route_name'
        $('#displayRteSortDiv').empty().append(`<h4>Sorting by: route name</h4>`)
    })

    $(document).on('click', '.allPtFileBtn', (e) => {
        $('#selectedFile').empty();
        displayPointFile = e.target.innerHTML;
        $('#selectedFile').append(`<h3 class='mb-1'><b>Selected file: ${displayPointFile}</b></h3>`)
        $.ajax({
            dataType: 'json',
            url: '/queryFileAllPts',
            data: {
                fileName: displayPointFile,
                order: pointSort,
            },
            success: (data) => {
                console.log(data);
                $('#ptQueryBody').empty();
                $('#ptTable').empty();

                if (data.length === 0) {
                    $('#ptTable').append(`<h2 class='mt-3' align='center'>No points...</h2>`)
                }

                data.forEach((point) => {
                    if (point.point_name === 'None') {
                        $('#ptQueryBody').append(`
                        <tr>
                            <th scope = 'col'>${point.point_index}</th>
                            <th scope = 'col'>Unnamed point ${point.point_id}</th>
                            <th scope = 'col'>${point.point_lat}</th>
                            <th scope = 'col'>${point.point_lon}</th>
                            <th scope = 'col'>${point.point_routeName}</th>
                            <th scope = 'col'>${point.point_routeLen}</th>
                        </tr>`);
                    } else {
                        $('#ptQueryBody').append(`
                        <tr>
                            <th scope = 'col'>${point.point_index}</th>
                            <th scope = 'col'>${point.point_name}</th>
                            <th scope = 'col'>${point.point_lat}</th>
                            <th scope = 'col'>${point.point_lon}</th>
                            <th scope = 'col'>${point.point_routeName}</th>
                            <th scope = 'col'>${point.point_routeLen}</th>
                        </tr>`);
                    }
                })

                $('#allPtsFile').modal('toggle');
                $('#displayPtSortDiv').empty();
                $('#rteTableQuery').hide();
                $('#ptTableQuery').show();
                window.scrollTo(5000, 5000);
            },
            error: (err) => {
                console.error(err);
            }
        })
    })

    $('#ptLenSortBtn').on('click', () => {
        pointSort = 'route_len'
        $('#displayPtSortDiv').empty().append(`<h4>Sorting by: route length</h4>`);
    })

    $('#ptNameSortBtn').on('click', () => {
        pointSort = 'route_name'
        $('#displayPtSortDiv').empty().append(`<h4>Sorting by: route name</h4>`)
    })

    $(document).on('click', '.rteBtn', (e) => {
        console.log(e.target.innerHTML);
        $.ajax({
            dataType: 'json',
            url: '/getPtsFromRte',
            data: {
                rteName: e.target.innerHTML,
            },
            success: (data) => {
                $('#ptQueryBody').empty();
                $('#ptTable').empty();

                if (data.length === 0) {
                    $('#ptTable').append(`<h2 class='mt-3' align='center'>No points...</h2>`)
                }

                data.forEach((point) => {
                    if (point.point_name === 'None') {
                        $('#ptQueryBody').append(`
                        <tr>
                            <th scope = 'col'>${point.point_index}</th>
                            <th scope = 'col'>Unnamed point ${point.point_id}</th>
                            <th scope = 'col'>${point.point_lat}</th>
                            <th scope = 'col'>${point.point_lon}</th>
                            <th scope = 'col'>${point.point_routeName}</th>
                            <th scope = 'col'>${point.point_routeLen}</th>
                        </tr>`);
                    } else {
                        $('#ptQueryBody').append(`
                        <tr>
                            <th scope = 'col'>${point.point_index}</th>
                            <th scope = 'col'>${point.point_name}</th>
                            <th scope = 'col'>${point.point_lat}</th>
                            <th scope = 'col'>${point.point_lon}</th>
                            <th scope = 'col'>${point.point_routeName}</th>
                            <th scope = 'col'>${point.point_routeLen}</th>
                        </tr>`);
                    }

                })

                $('#allPtsRte').modal('toggle');
                $('#selectedFile').empty();
                $('#rteTableQuery').hide();
                $('#ptTableQuery').show();
                window.scrollTo(5000, 5000);
            },
            error: (err) => {
                console.error(err);
            }
        })

    })

    $('#logout').on('click', () => {
        $.ajax({
            dataType: 'json',
            url: '/logoutDb',
            success: (data) => {
                alert('Successfully logged out!');
                $('#login').show();
                $('#logout').hide();
                $('#clearDataBtn').hide();
                $('#displayDbBtn').hide();
                $('#queryDiv').hide();
                $('#allRtesBtn').hide();
                $('#allRtesFileBtn').hide();
                $('#allPtsRteBtn').hide();
                $('#storeFileBtn').hide();
                $('#rteTableQuery').hide();
                $('#ptTableQuery').hide();
                $('#allPtsFileBtn').hide();
            },
            error: (err) => {
                console.error(err);
            }
        })
    });
});
