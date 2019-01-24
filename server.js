var server = require('ws').Server;
var s = new server({port:3000});

s.on('connection',function(ws){

    ws.on('message',function(message){
        console.log("Received: "+message);
        
        s.clients.forEach(function(client){
            client.send(message);
        });
    });

    ws.on('close',function(){
        console.log('I lost a client');
    });

});
