'use strict'
var connection;
const mysql = require('mysql');

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

//Sample endpoint

let records = [ 
    "INSERT INTO student (last_name, first_name, mark) VALUES ('Hugo','Victor','B+')", 
    "INSERT INTO student (last_name, first_name, mark) VALUES ('Rudin','Walter','A-')", 
    "INSERT INTO student (last_name, first_name, mark) VALUES ('Stevens','Richard','C')"
];

app.get('/someendpoint', function(req , res){
  res.send({
    foo: "bar"
  });
});

var sharedLib = ffi.Library('./myLibrary', {
  'updateFileStuffs': [ 'string', [ 'string', 'string'] ],
  'getGEDCOMSpecs': [ 'string', [ 'string'] ],
  'JSONAllIndivs': [ 'string', [ 'string'] ],
  'addIndiv': [ 'void', ['string', 'string' ] ],		//return type first, argument list second
  'getDesStr': [ 'string', ['string', 'string', 'int' ] ],		//return type first, argument list second
  'getAnsStr': [ 'string', ['string', 'string', 'int' ] ],		//return type first, argument list second
								//for void input type, leave argumrnt list empty
  'checkValid': [ 'int', [ 'string' ] ],	//int return, int argument
/*  'getDesc' : [ 'string', [] ],
  'putDesc' : [ 'void', [ 'string' ] ],
  */ 
});

app.get('/somenewendpoint', function(req , res){
	let JSON = sharedLib.updateFileStuffs(req.query.str, req.query.fileName);
	res.send();
	//console.log(req.query.fileName);
});

app.get('/upDateTable', function(req , res){
	
	let JSON = sharedLib.getGEDCOMSpecs(req.query.fileName);
	//console.log(JSON);
	res.send(JSON);
	
});

app.get('/upDateFileTable', function(req , res){
	console.log(req.query.fileName);
	let JSON = sharedLib.JSONAllIndivs(req.query.fileName);
	console.log(JSON);
	console.log("HEY");
	res.send(JSON);
	
});

app.get('/upFile', function(req , res){
	let fs = require('fs');
	fs.readdir("./uploads", function(err, items){
		
		let i = 0;
		let array = [];
		for (i in items){
			let val = sharedLib.checkValid("uploads/" + items[i]);
			if (val == 1){
				console.log(items[i]);
				array.push(items[i]);
			}
		}
		console.log(array);
	
		res.send(array);
	});

});

app.get('/addIndiv', function(req , res){
	//console.log("hey");
	sharedLib.addIndiv(req.query.json, req.query.fileName);
	res.send();
});

app.get('/getDes', function(req , res){
	//console.log("hey");
	let x = sharedLib.getDesStr(req.query.json, req.query.file, req.query.max);
	res.send(x);
});

app.get('/getAns', function(req , res){
	//console.log("hey");
	let x = sharedLib.getAnsStr(req.query.json, req.query.file, req.query.max);
	res.send(x);
});

app.get('/getDatabase', function(req , res){
	//console.log("hey");
	connection = mysql.createConnection({ 
		host     : 'dursley.socs.uoguelph.ca',  
		user     : req.query.username,
		password : req.query.password,  
		database : req.query.database, 
	});
	connection.connect(function(err) {
		let obj = "";
		if (err){
			obj = "fail";
		}
		if (err){
			console.log("Not Connected");
		}
		else{
			console.log("Connected!");
			var sql = "CREATE TABLE FILE (file_id INT AUTO_INCREMENT PRIMARY KEY, file_Name VARCHAR(60) NOT NULL, source VARCHAR(250) NOT NULL, version VARCHAR(10) NOT NULL, encoding VARCHAR(10) NOT NULL, sub_name VARCHAR(62) NOT NULL, sub_addr VARCHAR(256), num_individials INT, num_families INT)";
			var sql2 = "CREATE TABLE INDIVIDUAL (ind_id INT AUTO_INCREMENT PRIMARY KEY, surname VARCHAR(256) NOT NULL, given_name VARCHAR(256) NOT NULL, sex VARCHAR(1), fam_size INT, source_file INT, FOREIGN KEY(source_file) REFERENCES FILE(file_id) ON DELETE CASCADE)";

			connection.query(sql, function (err, result) {
				console.log("1 record inserted");
			});
			
			connection.query(sql2, function (err, result) {
				console.log("1 record inserted");
			});
		}
		res.send(obj);
	});
});
app.get('/addFileDatabase', function(req , res){
	console.log(req.query);
	let lqs = "INSERT INTO FILE (file_id, file_Name, source, version, encoding, sub_name, sub_addr, num_individials, num_families) ";
	let sql = "VALUES (" + "\"null\"" + ", \"" + req.query.fileName + "\", \"" + req.query.source + "\", \"" + req.query.version + "\", \"" + req.query.encoding + "\", \"" + req.query.subName + "\", \"" + req.query.subAddress + "\", \"" + req.query.numIndiv + "\", \"" + req.query.numFams + "\")";
	let qsl = lqs + sql;
	console.log(sql);
	connection.query(qsl, function (err, result) {
		if (err){
			console.log(err);
		}
		console.log("File inserted");
	});
	 res.send();
});

app.get('/addIndDatabase', function(req , res){
	console.log("YO");
	let file = req.query.fileName;//.replace(/<uploads\//g,"");
	console.log(file);
	let look = "SELECT file_id FROM FILE WHERE file_name=\"" + file + "\"";
	console.log(look);
	//res.send();
	connection.query(look, function (err, result) {
		if (err){
			console.log(err);
		}
		else{
			console.log(result);
			let val = JSON.stringify(result[0]).split(":");
			let val2 = val[1].split("}");
			
			let lqs = "INSERT INTO INDIVIDUAL (ind_id, surname, given_name, sex, fam_size, source_file) ";
			let sql = "VALUES (" + "\"null\"" + ", \"" + req.query.surname + "\", \"" + req.query.givenName + "\", \"" + req.query.sex + "\", \"" + req.query.fSize + "\", \"" + val2[0] + "\")";
			let qsl = lqs + sql;
			console.log(qsl);
			
			connection.query(qsl, function (err, result) {
				if (err){
					console.log(err);
				}
			});
			 
		}
		//console.log("File inserted");
	});
	res.send();
	  
});

app.get('/deleteFromTables', function(req , res){
	connection.query("DELETE FROM FILE", function (err, result) {
		if (err){
			console.log(err);
		}
		console.log("File inserted");
	});
	connection.query("DELETE FROM INDIVIDUAL", function (err, result) {
		if (err){
			console.log(err);
		}
		console.log("File inserted");
	});
	res.send();
});

app.get('/numFileTable', function(req , res){
	connection.query("SELECT COUNT (DISTINCT file_name) FROM FILE", function (err, result) {
		result = JSON.parse(JSON.stringify(result[0]));
		res.send(result);
	});
});
app.get('/numIndTable', function(req , res){
	connection.query("SELECT COUNT (DISTINCT ind_id) FROM INDIVIDUAL", function (err, result) {
		result = JSON.parse(JSON.stringify(result[0]));
		res.send(result);
	}); 
});
app.get('/executeQ', function(req , res){
	connection.query(req.query.str, function (err, result) {
		//result = JSON.parse(JSON.stringify(result[0]));
		console.log(result);
		res.send(result);
	}); 
	
});
app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
