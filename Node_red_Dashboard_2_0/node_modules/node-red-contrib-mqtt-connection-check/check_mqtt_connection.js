const mqtt = require('mqtt');

module.exports = function (RED) {
    //called whenever a new instance of the node is created.
    function CheckMQTTConnection(config) {
        //initialise the features shared by all nodes
        RED.nodes.createNode(this, config);
        let node = this;

        //node-specific code goes here
        node.on('input', function (msg, send, done) {
            if (msg == null) msg = {};
            msg.mqttCheck = {};

            send = send || function () { node.send.apply(node, arguments) };

            let options = {
                username: config.username,
                password: config.password,
                connectTimeout: 7 * 1000
            };

            let client = mqtt.connect(`mqtt://${config.host}:${config.port}`, options);

            client.on('connect', function (connack) {
                msg.mqttCheck['connected'] = true;
                msg.mqttCheck['code'] = 0;
                client.end();
            });

            client.on('error', function (error) {
                msg.mqttCheck['connected'] = false;
                msg.mqttCheck['code'] = error.code;
                client.end();
            });

            client.on('close', function () {
                if (!msg.mqttCheck['connected']) msg.mqttCheck['connected'] = false;
                send(msg);
                client.end();
                if (done) {
                    done();
                }
            });
        });
    }
    RED.nodes.registerType("mqtt-check", CheckMQTTConnection);
}

