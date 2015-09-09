
.pragma library

var captionWidth = 80,
    refreshWidth = 80;
switch (Qt.locale().name.substr(0, 2))
{
case "zh":
    captionWidth = 64;
    refreshWidth = 52;
    break;
}
console.log("locale:[" + Qt.locale() + "]")
console.log("caption:" + captionWidth)
console.log("caption:" + refreshWidth)

function trim(str)
{
    console.log(performance);
    return str.replace(/^\s+/g, "").replace(/\s+$/g, "");
}

function getServerList(url, callback)
{
    var request = new XMLHttpRequest;
    request.onreadystatechange = function() {
        if (request.readyState === XMLHttpRequest.DONE) {
            if (request.status === 200) {
                if (request.responseText !== null) {
                    var serverList;
                    try {
                        serverList = JSON.parse(request.responseText);
                    } catch (e) {
//                        return callback(-3, "response is not JSON");
                    }
                    serverList = [{name: "192.168.1.9"}, {name: "192.168.1.8"}, {name: "127.0.0.1"}];
                    if (serverList !== null) {
                        return callback(0, serverList);
                    }
                    return callback(-3, i18n.tr("response is not JSON"));
                }
                return callback(-2, "response null");
            }
            return callback(-1);
        }
        console.log("xml-http-request state: [" + request.readyState + "]");
    }
    request.open("GET", url, true);
    request.send();
}

function pickServer(serverList)
{
    return serverList[0];
}

function populateServers(model)
{
    getServerList("http://www.baidu.com/", function(code, serverList) {
        console.log(code);
        console.log(serverList);
        addServerList(model, serverList)
    });
}

function addServerList(model, serverList)
{
    if (serverList) {
        model.clear();
        for (var i = 0; i < serverList.length; ++i) {
            console.log(serverList[i]);
            addServer(model, serverList[i]);
        }
    }
}

function addServer(model, server)
{
    model.append({text: server.name});
}
