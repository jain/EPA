var express = require('express');
var ejs = require('ejs');
var app = express();
app.use(express.static('public'));
//app.set('view engine', 'pug');
var base64 = require('base64-arraybuffer');
var getRawBody = require('raw-body');
var fs = require('fs');
var glob = require("glob");
var counter = 0;
var timeIn = [];
var timeOut = [];
//var twilio = require('twilio');

// Create a new REST API client to make authenticated requests against the
// twilio back end
//var client = new twilio.RestClient('ACcbe3db49a64427e547a466c94744470d', 'a4f5beea1e5370ccb538a6caf9a5661e');

var accountSid = 'ACcbe3db49a64427e547a466c94744470d'; // Your Account SID from www.twilio.com/console
var authToken = 'a4f5beea1e5370ccb538a6caf9a5661e';

//var twilio = require('twilio');
//var client = new twilio.RestClient(accountSid, authToken);
var client = require('twilio')(accountSid, authToken);
var phone = '+16787024172';


const StringDecoder = require('string_decoder').StringDecoder;
const decoder = new StringDecoder('utf8');
rec = '';
arr = {};
id_k = '';
img = '';

function message(body) {
    client.messages.create({
        to: phone,
        from: "+14046204583",
        body: body
    }, function (err, message) {
        console.log(err);
        console.log(message);
    });
}

app.get('/phone/:phone', function (req, res) {
    phone = req.params.phone;
    message('Sending Test Message!', 'http://epa4012.herokuapp.com/see_image/image.jpg');
    res.send(phone + ' has been set, sending test sms!');
});


app.get('/msg/:msg', function (req, res) {
    var bdy = req.params.msg;
    message(bdy + ' url: http://epa4012.herokuapp.com/see_image/image.jpg');
    res.send('sent!');
});

app.get('/timeout/:id/:time/:type', function (req, res) {
    var bdy = req.params.id + ' sent ' + req.params.time + ' time ' + req.params.type;
    res.send('sent!');
    message(bdy + 'image url: http://epa4012.herokuapp.com/img');
});

app.get('/sms', function (req, res) {
    res.send('sent');
    message('test sms: http://epa4012.herokuapp.com/img');
});

app.get('/', function (req, res) {
    res.send('Welcome to EPA, once we get a micro controller with wifi module we can proceed with next step!');
});

app.get('/web', function (req, res) {
    res.sendfile('public/index.html');
});
app.get('/dashboard', function (req, res) {
    res.sendfile('public/dashboard.html');
});

app.set('port', (process.env.PORT || 5000));

app.listen(app.get('port'), function () {
    console.log('Node app is running on port', app.get('port'));
});

app.get('/oct', function (req, res) {
    res.send(rec);
});

app.get('/img', function (req, res) {
    fs.readFile('image.jpg', function (err, data) {
        res.set('Content-Type', 'image/jpeg');
        res.send(data); // Send the file data to the browser.
    });
});

app.get('/img_name/:name', function(req, res){
    console.log(req.params.name);
    fs.readFile('imgs/'+req.params.name, function (err, data) {
        console.log('hi');
        res.set('Content-Type', 'image/jpeg');
        res.send(data); // Send the file data to the browser.
    });
});

app.get('/images', function(req, res){
    //res.send("halelihu");
    //res.render('index', { title: 'Hey' });
    res.writeHead(200, {'Content-Type': 'text/html'});
    var pics = [];
    glob("imgs/*.jpg", function (er, files) {
        console.log(files);
        pics = files;
        console.log('pics' + pics);
        fs.readFile('tab.html', 'utf-8', function(err, content) {
            if (err) {
                res.end('error occurred');
                return;
            }
            var renderedHtml = ejs.render(content, {temp: pics, time_in:timeIn, time_out:timeOut});  //get redered HTML code
            res.end(renderedHtml);
        });
    });
    //since we are in a request handler function
    //we're using readFile instead of readFileSync
});

app.post('/oct', function (req, res) {
    console.log(req.headers);
    console.log(req.get('time'));
    var time = req.get('time');
    timeIn.push(new Date());
    var time2 = new Date();
    var time2 = new Date(time2.setSeconds(time2.getSeconds() + parseInt(time)));
    timeOut.push(time2);
    console.log(req.connection.remoteAddress);
    console.log('n00b\n');
    var body = [];
    console.log('reee   ')
    console.log('why');
    var body = [];
    req.on('data', function (chunk) {
        body.push(chunk);
    }).on('end', function () {
        console.log(body);
        //body = Buffer.concat(body).toString();
        body = Buffer.concat(body);
        console.log(body.toString())
        rec = body;
        fs.writeFile('image.jpg', rec, function (err) {
            if (err) return console.log(err);
            console.log('n00b');
            message('See your image at: http://epa4012.herokuapp.com/img');
        });
        fs.writeFile('imgs/image' + counter + '.jpg', rec, function (err) {
            counter++;
            if (err) return console.log(err);
            console.log('n00b');
            //message('See your image at: http://epa4012.herokuapp.com/img');
        });
        console.log(typeof(rec));
        console.log(body);
        res.send('yay');
        // at this point, `body` has the entire request body stored in it as a string
    });
});
///see_image/image.jpg
app.get('/see_image/:path', function (req, res) {
    fs.readFile(req.params.path, function (err, data) {
        res.send(data);
    });
});